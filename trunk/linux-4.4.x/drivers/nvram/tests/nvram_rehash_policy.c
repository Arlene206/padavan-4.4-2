#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "nvram_rehash_policy.h"

struct rehash_test_state {
	unsigned int inserted;
	unsigned int rollback_calls;
	unsigned int fail_on_insert;
	char entries[4][32];
};

static int
test_insert(void *context, const char *name, const char *value)
{
	struct rehash_test_state *state = context;
	int written;

	++state->inserted;
	if (state->fail_on_insert == state->inserted)
		return -ENOMEM;
	if (state->inserted > 4)
		return -ENOMEM;

	written = snprintf(state->entries[state->inserted - 1],
			   sizeof(state->entries[0]), "%s=%s", name, value);
	if (written < 0 || (size_t)written >= sizeof(state->entries[0]))
		return -ENOMEM;
	return 0;
}

static void
test_rollback(void *context)
{
	struct rehash_test_state *state = context;

	++state->rollback_calls;
	state->inserted = 0;
	memset(state->entries, 0, sizeof(state->entries));
}

static void test_valid_records_and_zero_padding(void)
{
	char data[] = "lan_ipaddr=192.168.1.1\0empty=\0\0\0";
	char original[sizeof(data)];
	struct rehash_test_state state = { 0 };

	memcpy(original, data, sizeof(data));
	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == 0);
	assert(state.inserted == 2);
	assert(state.rollback_calls == 0);
	assert(strcmp(state.entries[0], "lan_ipaddr=192.168.1.1") == 0);
	assert(strcmp(state.entries[1], "empty=") == 0);
	assert(memcmp(data, original, sizeof(data)) == 0);
}

static void test_empty_image_is_valid(void)
{
	char data[] = { '\0', '\0', '\0', '\0' };
	struct rehash_test_state state = { 0 };

	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == 0);
	assert(state.inserted == 0);
	assert(state.rollback_calls == 0);
}

static void test_malformed_record_rolls_back_prior_entries(void)
{
	char data[] = "first=ok\0malformed\0\0";
	struct rehash_test_state state = { 0 };

	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == NVRAM_REHASH_INVALID);
	assert(state.rollback_calls == 1);
	assert(state.inserted == 0);
	assert(state.entries[0][0] == '\0');
}

static void test_empty_name_rolls_back(void)
{
	char data[] = "=value\0\0";
	struct rehash_test_state state = { 0 };

	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == NVRAM_REHASH_INVALID);
	assert(state.rollback_calls == 1);
	assert(state.inserted == 0);
}

static void test_missing_double_nul_is_rejected(void)
{
	char data[] = "single=terminator\0";
	struct rehash_test_state state = { 0 };

	assert(nvram_rehash_records(data, sizeof(data) - 1, test_insert,
				    test_rollback, &state) == NVRAM_REHASH_INVALID);
	assert(state.rollback_calls == 1);
	assert(state.inserted == 0);
}

static void test_allocation_failure_rolls_back(void)
{
	char data[] = "first=ok\0second=fails\0\0";
	struct rehash_test_state state = { 0 };

	state.fail_on_insert = 2;
	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == -ENOMEM);
	assert(state.rollback_calls == 1);
	assert(state.inserted == 0);
	assert(state.entries[0][0] == '\0');
}

static void test_unterminated_record_is_rejected(void)
{
	char data[] = { 'a', '=', 'b', 'c', 'd', 0, 0 };
	struct rehash_test_state state = { 0 };

	data[5] = 'x';
	assert(nvram_rehash_records(data, sizeof(data), test_insert,
				    test_rollback, &state) == NVRAM_REHASH_INVALID);
	assert(state.rollback_calls == 1);
	assert(state.inserted == 0);
}

int main(void)
{
	test_valid_records_and_zero_padding();
	test_empty_image_is_valid();
	test_malformed_record_rolls_back_prior_entries();
	test_empty_name_rolls_back();
	test_missing_double_nul_is_rejected();
	test_allocation_failure_rolls_back();
	test_unterminated_record_is_rejected();
	puts("NVRAM flash rehash validation and rollback policy: PASS");
	return 0;
}
