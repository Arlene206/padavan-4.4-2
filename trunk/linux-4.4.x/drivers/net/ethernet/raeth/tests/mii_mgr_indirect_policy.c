#include <stdio.h>
#include <string.h>

#include "mii_mgr_indirect_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

struct mock_bus {
	char operation[4];
	unsigned int phy[4];
	unsigned int reg[4];
	unsigned int value[4];
	unsigned int count;
	unsigned int fail_at;
	unsigned int locked;
};

static unsigned int
mock_write(void *context, unsigned int phy, unsigned int reg,
	   unsigned int value)
{
	struct mock_bus *bus = (struct mock_bus *)context;
	unsigned int index = bus->count++;

	if (!bus->locked || index >= 4U)
		return 0;
	bus->operation[index] = 'W';
	bus->phy[index] = phy;
	bus->reg[index] = reg;
	bus->value[index] = value;
	return bus->count != bus->fail_at;
}

static unsigned int
mock_read(void *context, unsigned int phy, unsigned int reg,
	  unsigned int *value)
{
	struct mock_bus *bus = (struct mock_bus *)context;
	unsigned int index = bus->count++;

	if (!bus->locked || index >= 4U || !value)
		return 0;
	bus->operation[index] = 'R';
	bus->phy[index] = phy;
	bus->reg[index] = reg;
	if (reg == 0x0dU)
		*value = 0x4567U;
	else if (reg == 0x10U)
		*value = 0x89abU;
	else
		*value = 0xdeadU;
	bus->value[index] = *value;
	return bus->count != bus->fail_at;
}

static void
reset_bus(struct mock_bus *bus, unsigned int fail_at)
{
	memset(bus, 0, sizeof(*bus));
	bus->locked = 1;
	bus->fail_at = fail_at;
}

static int
test_read_sequence_is_one_locked_transaction(void)
{
	struct mock_bus bus;
	unsigned int value = 0;

	reset_bus(&bus, 0);
	CHECK(mii_mgr_indirect_read(31U, 0x1234U, &bus, mock_write,
				   mock_read, &value) == 1U);
	CHECK(value == 0x89ab4567U);
	CHECK(bus.count == 3U);
	CHECK(bus.operation[0] == 'W' && bus.phy[0] == 31U &&
	      bus.reg[0] == 0x1fU && bus.value[0] == 0x48U);
	CHECK(bus.operation[1] == 'R' && bus.phy[1] == 31U &&
	      bus.reg[1] == 0x0dU);
	CHECK(bus.operation[2] == 'R' && bus.phy[2] == 31U &&
	      bus.reg[2] == 0x10U);
	return 0;
}

static int
test_read_failures_clear_output_and_stop_sequence(void)
{
	struct mock_bus bus;
	unsigned int fail_at;
	unsigned int value;

	for (fail_at = 1U; fail_at <= 3U; ++fail_at) {
		reset_bus(&bus, fail_at);
		value = 0xffffffffU;
		CHECK(mii_mgr_indirect_read(31U, 0x1234U, &bus, mock_write,
					   mock_read, &value) == 0U);
		CHECK(value == 0U);
		CHECK(bus.count == fail_at);
	}
	return 0;
}

static int
test_write_sequence_uses_low_and_high_words(void)
{
	struct mock_bus bus;

	reset_bus(&bus, 0);
	CHECK(mii_mgr_indirect_write(31U, 0x1234U, 0x89abcdefU,
				     &bus, mock_write) == 1U);
	CHECK(bus.count == 3U);
	CHECK(bus.operation[0] == 'W' && bus.reg[0] == 0x1fU &&
	      bus.value[0] == 0x48U);
	CHECK(bus.operation[1] == 'W' && bus.reg[1] == 0x0dU &&
	      bus.value[1] == 0xcdefU);
	CHECK(bus.operation[2] == 'W' && bus.reg[2] == 0x10U &&
	      bus.value[2] == 0x89abU);
	return 0;
}

static int
test_write_failures_stop_before_later_registers(void)
{
	struct mock_bus bus;
	unsigned int fail_at;

	for (fail_at = 1U; fail_at <= 3U; ++fail_at) {
		reset_bus(&bus, fail_at);
		CHECK(mii_mgr_indirect_write(31U, 0x1234U, 0x89abcdefU,
					     &bus, mock_write) == 0U);
		CHECK(bus.count == fail_at);
	}
	return 0;
}

static int
test_invalid_callbacks_do_not_issue_operations(void)
{
	struct mock_bus bus;
	unsigned int value = 0xffffffffU;

	reset_bus(&bus, 0);
	CHECK(mii_mgr_indirect_read(31U, 0x1234U, &bus, NULL,
				   mock_read, &value) == 0U);
	CHECK(value == 0U && bus.count == 0U);
	value = 0xffffffffU;
	CHECK(mii_mgr_indirect_read(31U, 0x1234U, &bus, mock_write,
				   mock_read, NULL) == 0U);
	CHECK(bus.count == 0U);
	CHECK(mii_mgr_indirect_write(31U, 0x1234U, 1U,
				     &bus, NULL) == 0U);
	CHECK(bus.count == 0U);
	return 0;
}

int
main(void)
{
	CHECK(test_read_sequence_is_one_locked_transaction() == 0);
	CHECK(test_read_failures_clear_output_and_stop_sequence() == 0);
	CHECK(test_write_sequence_uses_low_and_high_words() == 0);
	CHECK(test_write_failures_stop_before_later_registers() == 0);
	CHECK(test_invalid_callbacks_do_not_issue_operations() == 0);
	puts("RAETH indirect MMD transaction policy: PASS");
	return 0;
}
