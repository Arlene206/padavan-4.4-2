#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../switch_wait_policy.h"

struct wait_mock {
	unsigned int values[8], value_count, reads, sleeps, expected_reg, last_delay;
	unsigned int fail_on_read;
	int read_error;
};

static void
wait_mock_init(struct wait_mock *mock, unsigned int reg)
{
	memset(mock, 0, sizeof(*mock));
	mock->expected_reg = reg;
}

static int
mock_read(void *opaque, unsigned int reg, unsigned int *value)
{
	struct wait_mock *mock = (struct wait_mock *)opaque;
	unsigned int index;
	mock->reads++;
	if (reg != mock->expected_reg)
		return EINVAL;
	if (mock->fail_on_read != 0 && mock->reads == mock->fail_on_read)
		return mock->read_error;
	if (mock->value_count == 0)
		return EIO;
	index = mock->reads - 1;
	if (index >= mock->value_count)
		index = mock->value_count - 1;
	*value = mock->values[index];
	return 0;
}

static void
mock_delay(void *opaque, unsigned int usec)
{
	struct wait_mock *mock = (struct wait_mock *)opaque;
	mock->sleeps++;
	mock->last_delay = usec;
}

static int
wait_for_clear(struct wait_mock *mock, unsigned int attempts,
               unsigned int *last_value)
{
	return switch_wait_until_clear(mock_read, mock_delay, mock,
	                               mock->expected_reg, 0x80000000U,
	                               attempts, 1000U, last_value);
}

static int
wait_for_table(struct wait_mock *mock, unsigned int attempts,
               unsigned int *last_value)
{
	return switch_wait_for_table_result(mock_read, mock_delay, mock,
	                                    mock->expected_reg, 0x8000U,
	                                    0x6000U, attempts, 5000U, last_value);
}

static void
test_idle_register_returns_immediately(void)
{
	struct wait_mock mock;
	unsigned int last = 0xffffffffU;
	wait_mock_init(&mock, 0x90U);
	mock.values[0] = 0x1234U;
	mock.value_count = 1;
	assert(wait_for_clear(&mock, 20U, &last) == 0);
	assert(mock.reads == 1U && mock.sleeps == 0U);
	assert(mock.last_delay == 0U && last == 0x1234U);
}

static void
test_busy_register_retries_until_idle(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x90U);
	mock.values[0] = 0x80000001U;
	mock.values[1] = 0x80000000U;
	mock.values[2] = 0x00000001U;
	mock.value_count = 3;
	assert(wait_for_clear(&mock, 20U, &last) == 0);
	assert(mock.reads == 3U && mock.sleeps == 2U);
	assert(mock.last_delay == 1000U && last == 1U);
}

static void
test_busy_timeout_is_reported(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x90U);
	mock.values[0] = 0x80000000U;
	mock.value_count = 1;
	assert(wait_for_clear(&mock, 4U, &last) == ETIMEDOUT);
	assert(mock.reads == 4U && mock.sleeps == 4U);
	assert(mock.last_delay == 1000U && last == 0x80000000U);
}

static void
test_register_read_error_is_propagated(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x90U);
	mock.values[0] = 0x80000000U;
	mock.value_count = 1;
	mock.fail_on_read = 2U;
	mock.read_error = EIO;
	assert(wait_for_clear(&mock, 20U, &last) == EIO);
	assert(mock.reads == 2U && mock.sleeps == 1U);
	assert(last == 0x80000000U);
}

static void
test_search_waits_for_idle_and_ready_or_end(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x80U);
	mock.values[0] = 0xa000U;
	mock.values[1] = 0x8000U;
	mock.values[2] = 0x2000U;
	mock.value_count = 3;
	assert(wait_for_table(&mock, 20U, &last) == 0);
	assert(mock.reads == 3U && mock.sleeps == 2U);
	assert(mock.last_delay == 5000U && last == 0x2000U);
	wait_mock_init(&mock, 0x80U);
	mock.values[0] = 0x4000U;
	mock.value_count = 1;
	assert(wait_for_table(&mock, 20U, &last) == 0);
	assert(mock.reads == 1U && mock.sleeps == 0U && last == 0x4000U);
}

static void
test_search_timeout_is_bounded(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x80U);
	mock.values[0] = 0x8000U;
	mock.value_count = 1;
	assert(wait_for_table(&mock, 3U, &last) == ETIMEDOUT);
	assert(mock.reads == 3U && mock.sleeps == 3U);
	assert(mock.last_delay == 5000U && last == 0x8000U);
}

static void
test_invalid_wait_inputs_fail(void)
{
	struct wait_mock mock;
	unsigned int last = 0;
	wait_mock_init(&mock, 0x90U);
	assert(switch_wait_until_clear(NULL, mock_delay, &mock, 0x90U,
	                               0x80000000U, 1U, 1000U, &last) == EINVAL);
	assert(switch_wait_until_clear(mock_read, NULL, &mock, 0x90U,
	                               0x80000000U, 1U, 1000U, &last) == EINVAL);
	assert(switch_wait_until_clear(mock_read, mock_delay, &mock, 0x90U,
	                               0, 1U, 1000U, &last) == EINVAL);
	assert(switch_wait_until_clear(mock_read, mock_delay, &mock, 0x90U,
	                               0x80000000U, 0, 1000U, NULL) == EINVAL);
	assert(switch_wait_for_table_result(mock_read, mock_delay, &mock, 0x90U,
	                                    0, 0, 1U, 5000U, &last) == EINVAL);
	assert(mock.reads == 0U && mock.sleeps == 0U);
}

int
main(void)
{
	test_idle_register_returns_immediately();
	test_busy_register_retries_until_idle();
	test_busy_timeout_is_reported();
	test_register_read_error_is_propagated();
	test_search_waits_for_idle_and_ready_or_end();
	test_search_timeout_is_bounded();
	test_invalid_wait_inputs_fail();
	puts("switch table-engine bounded wait policy: PASS");
	return 0;
}
