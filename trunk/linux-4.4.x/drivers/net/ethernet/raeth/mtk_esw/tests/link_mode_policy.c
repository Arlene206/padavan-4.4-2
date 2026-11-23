#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "../link_mode_policy.h"

enum fake_event_type { FAKE_READ = 1, FAKE_WRITE, FAKE_DELAY };

struct fake_event {
	enum fake_event_type type;
	unsigned int phy, reg, value;
};

struct fake_mdio {
	struct fake_event events[8];
	unsigned int event_count, mdio_calls;
	int fail_at, fail_status;
	unsigned int read_value;
};

static void
fake_record(struct fake_mdio *fake, enum fake_event_type type,
	    unsigned int phy, unsigned int reg, unsigned int value)
{
	assert(fake->event_count < sizeof(fake->events) / sizeof(fake->events[0]));
	fake->events[fake->event_count].type = type;
	fake->events[fake->event_count].phy = phy;
	fake->events[fake->event_count].reg = reg;
	fake->events[fake->event_count].value = value;
	fake->event_count++;
}

static int
fake_mdio_should_fail(struct fake_mdio *fake)
{
	unsigned int call = fake->mdio_calls++;

	return fake->fail_at >= 0 && (int)call == fake->fail_at;
}

static int
fake_read(void *context, unsigned int phy, unsigned int reg,
	  unsigned int *value)
{
	struct fake_mdio *fake = (struct fake_mdio *)context;

	fake_record(fake, FAKE_READ, phy, reg, 0U);
	if (fake_mdio_should_fail(fake))
		return fake->fail_status ? fake->fail_status : -EIO;
	*value = fake->read_value;
	return 0;
}

static int
fake_write(void *context, unsigned int phy, unsigned int reg,
	   unsigned int value)
{
	struct fake_mdio *fake = (struct fake_mdio *)context;

	fake_record(fake, FAKE_WRITE, phy, reg, value);
	if (fake_mdio_should_fail(fake))
		return fake->fail_status ? fake->fail_status : -EIO;
	return 0;
}

static void
fake_delay(void *context, unsigned int milliseconds)
{
	struct fake_mdio *fake = (struct fake_mdio *)context;

	fake_record(fake, FAKE_DELAY, 0U, 0U, milliseconds);
}

static const struct mtk_esw_link_mode_io fake_ops = {
	fake_read, fake_write, fake_delay
};

static void
fake_init(struct fake_mdio *fake)
{
	unsigned int i;

	fake->event_count = 0;
	fake->mdio_calls = 0;
	fake->fail_at = -1;
	fake->fail_status = 0;
	fake->read_value = 0U;
	for (i = 0; i < sizeof(fake->events) / sizeof(fake->events[0]); ++i) {
		fake->events[i].type = 0;
		fake->events[i].phy = 0;
		fake->events[i].reg = 0;
		fake->events[i].value = 0;
	}
}

static int
program_fake(struct fake_mdio *fake, unsigned int advertisement,
	     unsigned int control, int gigabit_enabled, int power_enabled)
{
	return mtk_esw_link_mode_program(3U, advertisement, control,
					gigabit_enabled, power_enabled,
					&fake_ops, fake);
}

static void
test_autoneg_register_sequence_and_cache_ready_result(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.read_value = 0x0055U;
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, 1) == 0);
	assert(fake.event_count == 4U && fake.mdio_calls == 4U);
	assert(fake.events[0].type == FAKE_READ &&
	       fake.events[0].phy == 3U && fake.events[0].reg == 9U);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].reg == 9U && fake.events[1].value == 0x0355U);
	assert(fake.events[2].type == FAKE_WRITE &&
	       fake.events[2].reg == 4U && fake.events[2].value == 0x05e1U);
	assert(fake.events[3].type == FAKE_WRITE &&
	       fake.events[3].reg == 0U && fake.events[3].value == 0x3300U);
}

static void
test_forced_mode_power_cycles_before_final_control_write(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.read_value = 0x0300U;
	assert(program_fake(&fake, 0x04a1U, 0x2100U, 0, 1) == 0);
	assert(fake.event_count == 6U && fake.mdio_calls == 5U);
	assert(fake.events[1].type == FAKE_WRITE && fake.events[1].reg == 9U &&
	       fake.events[1].value == 0U);
	assert(fake.events[2].type == FAKE_WRITE && fake.events[2].reg == 4U &&
	       fake.events[2].value == 0x04a1U);
	assert(fake.events[3].type == FAKE_WRITE && fake.events[3].reg == 0U &&
	       fake.events[3].value == 0x2900U);
	assert(fake.events[4].type == FAKE_DELAY && fake.events[4].value == 500U);
	assert(fake.events[5].type == FAKE_WRITE && fake.events[5].reg == 0U &&
	       fake.events[5].value == 0x2100U);
}

static void
test_power_off_sets_down_bit_without_delay(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, 0) == 0);
	assert(fake.event_count == 4U && fake.mdio_calls == 4U);
	assert(fake.events[3].type == FAKE_WRITE && fake.events[3].reg == 0U &&
	       fake.events[3].value == 0x3900U);
}

static void
test_read_and_each_write_failure_abort_without_later_io(void)
{
	struct fake_mdio fake;
	int failure;

	for (failure = 0; failure < 4; ++failure) {
		fake_init(&fake);
		fake.fail_at = failure;
		fake.fail_status = -EIO;
		assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, 1) == -EIO);
		assert(fake.mdio_calls == (unsigned int)(failure + 1));
		assert(fake.event_count == (unsigned int)(failure + 1));
		assert(fake.events[fake.event_count - 1].type != FAKE_DELAY);
	}

	/* A failed forced-mode power-down write must not be followed by a delay. */
	fake_init(&fake);
	fake.fail_at = 3;
	fake.fail_status = -EIO;
	assert(program_fake(&fake, 0x04a1U, 0x2100U, 0, 1) == -EIO);
	assert(fake.mdio_calls == 4U && fake.event_count == 4U);
	assert(fake.events[3].reg == MTK_ESW_LINK_MODE_REG_CONTROL);
}

static void
test_mdio_errors_are_preserved_and_positive_failures_become_eio(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -ETIMEDOUT;
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, 1) == -ETIMEDOUT);
	assert(fake.event_count == 1U);

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = 1;
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, 1) == -EIO);
	assert(fake.event_count == 2U);
}

static void
test_forced_final_write_failure_is_not_hidden_after_delay(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.fail_at = 4;
	fake.fail_status = -EIO;
	assert(program_fake(&fake, 0x04a1U, 0x2100U, 0, 1) == -EIO);
	assert(fake.mdio_calls == 5U);
	assert(fake.events[4].type == FAKE_DELAY && fake.events[4].value == 500U);
	assert(fake.events[5].type == FAKE_WRITE && fake.events[5].reg == 0U);
	assert(fake.event_count == 6U);
}

static void
test_invalid_io_and_boolean_arguments_are_rejected(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	assert(mtk_esw_link_mode_program(3U, 0x05e1U, 0x3100U, 1, 1,
					NULL, &fake) == -EINVAL);
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 2, 1) == -EINVAL);
	assert(program_fake(&fake, 0x05e1U, 0x3100U, 1, -1) == -EINVAL);
	assert(fake.event_count == 0U && fake.mdio_calls == 0U);
}

static void
test_all_defined_speed_and_flow_modes(void)
{
	unsigned int speed, flow;

	for (speed = 0U; speed <= 9U; speed++)
		for (flow = 0U; flow <= 2U; flow++)
			assert(mtk_esw_link_mode_is_valid((flow << 8) | speed));
	for (flow = 0U; flow <= 2U; flow++)
		assert(mtk_esw_link_mode_is_valid((flow << 8) | 15U));
}

static void
test_reserved_speed_flow_and_bits_are_rejected(void)
{
	unsigned int speed;

	for (speed = 10U; speed <= 14U; speed++)
		assert(!mtk_esw_link_mode_is_valid(speed));
	assert(!mtk_esw_link_mode_is_valid(3U << 8));
	assert(!mtk_esw_link_mode_is_valid(0x00000010U));
	assert(!mtk_esw_link_mode_is_valid(0x00000400U));
	assert(!mtk_esw_link_mode_is_valid(0x00010000U));
	assert(!mtk_esw_link_mode_is_valid(0x80000000U));
}

int
main(void)
{
	test_all_defined_speed_and_flow_modes();
	test_reserved_speed_flow_and_bits_are_rejected();
	test_autoneg_register_sequence_and_cache_ready_result();
	test_forced_mode_power_cycles_before_final_control_write();
	test_power_off_sets_down_bit_without_delay();
	test_read_and_each_write_failure_abort_without_later_io();
	test_mdio_errors_are_preserved_and_positive_failures_become_eio();
	test_forced_final_write_failure_is_not_hidden_after_delay();
	test_invalid_io_and_boolean_arguments_are_rejected();
	puts("MTK ESW link-mode validation and MDIO programming policy: PASS");
	return 0;
}
