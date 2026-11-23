#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../link_mode_policy.h"
#include "../mib_snapshot_policy.h"

enum fake_event_type { FAKE_READ = 1, FAKE_WRITE, FAKE_DELAY, FAKE_DELAY_US };

struct fake_event {
	enum fake_event_type type;
	unsigned int phy, reg, value;
};

struct fake_mdio {
	struct fake_event events[512];
	unsigned int event_count, mdio_calls;
	int fail_at, fail_status;
	unsigned int read_value;
	unsigned int busy_reads;
	int busy_forever;
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
	if (reg == MTK_ESW_MAC_TABLE_REG || reg == MTK_ESW_VLAN_TABLE_REG) {
		if (fake->busy_forever || fake->busy_reads) {
			*value = (reg == MTK_ESW_VLAN_TABLE_REG) ?
				MTK_ESW_VLAN_TABLE_BUSY_MASK :
				MTK_ESW_MAC_TABLE_BUSY_MASK;
			if (fake->busy_reads)
				fake->busy_reads--;
		} else {
			*value = 0U;
		}
	} else {
		*value = fake->read_value;
	}
	fake->events[fake->event_count - 1].value = *value;
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

static void
fake_delay_us(void *context, unsigned int microseconds)
{
	struct fake_mdio *fake = (struct fake_mdio *)context;

	fake_record(fake, FAKE_DELAY_US, 0U, 0U, microseconds);
}

static const struct mtk_esw_link_mode_io fake_ops = {
	fake_read, fake_write, fake_delay, fake_delay_us
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
	fake->busy_reads = 0U;
	fake->busy_forever = 0;
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

static int
power_fake(struct fake_mdio *fake, int power_on, int force,
	   int enable_autoneg, int *power_changed)
{
	return mtk_esw_phy_power_set(3U, power_on, force, enable_autoneg,
				     &fake_ops, fake, power_changed);
}

static void
test_first_error_accumulator_preserves_first_failure(void)
{
	assert(mtk_esw_first_error(0, 0) == 0);
	assert(mtk_esw_first_error(0, -ETIMEDOUT) == -ETIMEDOUT);
	assert(mtk_esw_first_error(-EIO, -ETIMEDOUT) == -EIO);
	assert(mtk_esw_first_error(0, 1) == -EIO);
	assert(mtk_esw_first_error(-ETIMEDOUT, 1) == -ETIMEDOUT);
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
test_phy_power_set_reads_and_writes_before_reporting_change(void)
{
	struct fake_mdio fake;
	int changed = 0;

	fake_init(&fake);
	fake.read_value = 0x3100U; /* currently powered on */
	assert(power_fake(&fake, 0, 0, 1, &changed) == 0);
	assert(changed == 1 && fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[0].type == FAKE_READ &&
	       fake.events[0].reg == MTK_ESW_LINK_MODE_REG_CONTROL);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].value == 0x3900U);

	fake_init(&fake);
	fake.read_value = 0x3900U; /* currently powered down */
	changed = 0;
	assert(power_fake(&fake, 1, 0, 1, &changed) == 0);
	assert(changed == 1 && fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].value == 0x3300U);
}

static void
test_phy_power_set_skips_unchanged_state_but_honors_force(void)
{
	struct fake_mdio fake;
	int changed = 1;

	fake_init(&fake);
	fake.read_value = 0x3100U;
	assert(power_fake(&fake, 1, 0, 1, &changed) == 0);
	assert(changed == 0 && fake.mdio_calls == 1U && fake.event_count == 1U);

	fake_init(&fake);
	fake.read_value = 0x3100U;
	assert(power_fake(&fake, 1, 1, 1, &changed) == 0);
	assert(changed == 0 && fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].value == 0x3300U);
}

static void
test_phy_power_set_read_and_write_errors_are_propagated(void)
{
	struct fake_mdio fake;
	int changed = 1;

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -ETIMEDOUT;
	assert(power_fake(&fake, 1, 0, 1, &changed) == -ETIMEDOUT);
	assert(changed == 0 && fake.mdio_calls == 1U && fake.event_count == 1U);

	fake_init(&fake);
	fake.read_value = 0x3100U;
	fake.fail_at = 1;
	fake.fail_status = -EIO;
	assert(power_fake(&fake, 0, 0, 1, &changed) == -EIO);
	assert(changed == 0 && fake.mdio_calls == 2U && fake.event_count == 2U);
}

static void
test_phy_power_set_normalizes_positive_callback_failure(void)
{
	struct fake_mdio fake;
	int changed = 0;

	fake_init(&fake);
	fake.read_value = 0x3100U;
	fake.fail_at = 1;
	fake.fail_status = 1;
	assert(power_fake(&fake, 0, 0, 1, &changed) == -EIO);
	assert(changed == 0 && fake.mdio_calls == 2U);
}

static void
test_phy_power_set_rejects_invalid_inputs(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_write = fake_ops;
	int changed = 1;

	fake_init(&fake);
	assert(mtk_esw_phy_power_set(3U, 1, 0, 1, NULL, &fake, &changed) == -EINVAL);
	assert(power_fake(&fake, 2, 0, 1, &changed) == -EINVAL);
	assert(power_fake(&fake, 1, -1, 1, &changed) == -EINVAL);
	assert(power_fake(&fake, 1, 0, 2, &changed) == -EINVAL);
	missing_write.write = NULL;
	assert(mtk_esw_phy_power_set(3U, 1, 0, 1, &missing_write,
				     &fake, &changed) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_mac_table_clear_waits_for_confirmed_idle_state(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.busy_reads = 2U;
	assert(mtk_esw_mac_table_clear(0, &fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 4U && fake.event_count == 7U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].phy == MTK_ESW_MAC_TABLE_PHY &&
	       fake.events[0].reg == MTK_ESW_MAC_TABLE_REG &&
	       fake.events[0].value == MTK_ESW_MAC_TABLE_CLEAR_ALL);
	assert(fake.events[1].type == FAKE_DELAY_US &&
	       fake.events[1].value == MTK_ESW_MAC_TABLE_POLL_DELAY_US);
	assert(fake.events[2].type == FAKE_READ &&
	       fake.events[2].value == MTK_ESW_MAC_TABLE_BUSY_MASK);
	assert(fake.events[3].type == FAKE_DELAY_US);
	assert(fake.events[4].type == FAKE_READ &&
	       fake.events[4].value == MTK_ESW_MAC_TABLE_BUSY_MASK);
	assert(fake.events[5].type == FAKE_DELAY_US);
	assert(fake.events[6].type == FAKE_READ && fake.events[6].value == 0U);
}

static void
test_mac_table_clear_preserves_static_only_command(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	assert(mtk_esw_mac_table_clear(1, &fake_ops, &fake) == 0);
	assert(fake.event_count == 3U && fake.mdio_calls == 2U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].value == MTK_ESW_MAC_TABLE_CLEAR_STATIC);
	assert(fake.events[1].type == FAKE_DELAY_US);
	assert(fake.events[2].type == FAKE_READ);
}

static void
test_mac_table_clear_aborts_when_command_write_fails(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -ETIMEDOUT;
	assert(mtk_esw_mac_table_clear(0, &fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_WRITE);

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = 1;
	assert(mtk_esw_mac_table_clear(0, &fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
}

static void
test_mac_table_clear_treats_read_failure_as_unknown(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = -EIO;
	assert(mtk_esw_mac_table_clear(0, &fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 2U && fake.event_count == 3U);
	assert(fake.events[0].type == FAKE_WRITE);
	assert(fake.events[1].type == FAKE_DELAY_US);
	assert(fake.events[2].type == FAKE_READ);
}

static void
test_mac_table_clear_times_out_when_busy_never_clears(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.busy_forever = 1;
	assert(mtk_esw_mac_table_clear(0, &fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == 1U + MTK_ESW_MAC_TABLE_POLL_LIMIT);
	assert(fake.event_count == 1U + 2U * MTK_ESW_MAC_TABLE_POLL_LIMIT);
	assert(fake.events[fake.event_count - 1].type == FAKE_READ &&
	       fake.events[fake.event_count - 1].value == MTK_ESW_MAC_TABLE_BUSY_MASK);
}

static void
test_mac_table_clear_rejects_invalid_mode_and_missing_delay(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_delay = fake_ops;

	fake_init(&fake);
	assert(mtk_esw_mac_table_clear(2, &fake_ops, &fake) == -EINVAL);
	missing_delay.delay_us = NULL;
	assert(mtk_esw_mac_table_clear(0, &missing_delay, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_register_update_bits_preserves_unmasked_bits_and_propagates_errors(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_read = fake_ops;
	struct mtk_esw_link_mode_io missing_write = fake_ops;
	const unsigned int phy = 0x1fU, reg = 0x7d00U;
	const unsigned int mask = 0x00077777U;
	unsigned int expected;

	/* LED-on clears the target field and then sets it, preserving all other bits. */
	fake_init(&fake);
	fake.read_value = 0xa5a5c0d0U;
	expected = (fake.read_value & ~mask) | mask;
	assert(mtk_esw_register_update_bits(phy, reg, mask, mask,
					    &fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[0].type == FAKE_READ &&
	       fake.events[0].phy == phy && fake.events[0].reg == reg);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].phy == phy && fake.events[1].reg == reg &&
	       fake.events[1].value == expected);

	/* LED-off clears the field without disturbing unrelated register bits. */
	fake_init(&fake);
	fake.read_value = 0xa5a5c0d0U;
	expected = fake.read_value & ~mask;
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &fake_ops, &fake) == 0);
	assert(fake.events[1].value == expected);

	/* Read failure must never turn into a write based on an all-zero value. */
	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -EACCES;
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &fake_ops, &fake) == -EACCES);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_READ);

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = -ETIMEDOUT;
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == 2U && fake.events[1].type == FAKE_WRITE);

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = 1;
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &fake_ops, &fake) == -EIO);

	fake_init(&fake);
	missing_read.read = NULL;
	missing_write.write = NULL;
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U, NULL, &fake) == -EINVAL);
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &missing_read, &fake) == -EINVAL);
	assert(mtk_esw_register_update_bits(phy, reg, mask, 0U,
					    &missing_write, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_register_write_propagates_mdio_failures(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_write = fake_ops;

	fake_init(&fake);
	assert(mtk_esw_register_write(0x1fU, 0x7d00U, 0x12345678U,
				      &fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].phy == 0x1fU &&
	       fake.events[0].reg == 0x7d00U &&
	       fake.events[0].value == 0x12345678U);

	fake_init(&fake);
	fake.fail_at = 0U;
	fake.fail_status = -EACCES;
	assert(mtk_esw_register_write(0x1fU, 0x7d00U, 1U,
				      &fake_ops, &fake) == -EACCES);
	assert(fake.mdio_calls == 1U && fake.events[0].type == FAKE_WRITE);

	fake_init(&fake);
	fake.fail_at = 0U;
	fake.fail_status = 1;
	assert(mtk_esw_register_write(0x1fU, 0x7d00U, 1U,
				      &fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 1U);

	fake_init(&fake);
	missing_write.write = NULL;
	assert(mtk_esw_register_write(0x1fU, 0x7d00U, 1U,
				      NULL, &fake) == -EINVAL);
	assert(mtk_esw_register_write(0x1fU, 0x7d00U, 1U,
				      &missing_write, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_register_read_propagates_mdio_failures(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_read = fake_ops;
	unsigned int value = 0xdeadbeefU;

	fake_init(&fake);
	fake.read_value = 0x12345678U;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, &value,
				     &fake_ops, &fake) == 0);
	assert(value == 0x12345678U);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_READ &&
	       fake.events[0].phy == 0x1fU &&
	       fake.events[0].reg == 0x40a8U);

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -EACCES;
	value = 0xdeadbeefU;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, &value,
				     &fake_ops, &fake) == -EACCES);
	assert(value == 0U && fake.mdio_calls == 1U);

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = 1;
	value = 0xdeadbeefU;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, &value,
				     &fake_ops, &fake) == -EIO);
	assert(value == 0U && fake.mdio_calls == 1U);

	fake_init(&fake);
	missing_read.read = NULL;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, NULL,
				     &fake_ops, &fake) == -EINVAL);
	value = 0xdeadbeefU;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, &value,
				     NULL, &fake) == -EINVAL);
	assert(value == 0U);
	value = 0xdeadbeefU;
	assert(mtk_esw_register_read(0x1fU, 0x40a8U, &value,
				     &missing_read, &fake) == -EINVAL);
	assert(value == 0U);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}


struct fake_mib_register_io {
	unsigned int calls;
	int fail_at;
	int fail_status;
	unsigned int registers[32];
};

static int
fake_mib_register_read(void *context, unsigned int reg, unsigned int *value)
{
	struct fake_mib_register_io *fake =
		(struct fake_mib_register_io *)context;
	unsigned int call = fake->calls++;

	assert(call < sizeof(fake->registers) / sizeof(fake->registers[0]));
	fake->registers[call] = reg;
	if (fake->fail_at >= 0 && (int)call == fake->fail_at)
		return fake->fail_status;
	*value = reg;
	return 0;
}

static void
test_mib_snapshot_publishes_complete_counter_set(void)
{
	static const unsigned int expected_offsets[] = {
		0x40a8U, 0x40acU, 0x4048U, 0x404cU,
		0x4000U, 0x4004U, 0x4008U, 0x400cU, 0x4010U,
		0x4014U, 0x402cU,
		0x4060U, 0x4064U, 0x4068U, 0x406cU, 0x4070U,
		0x4074U, 0x4078U, 0x408cU
	};
	struct fake_mib_register_io fake;
	struct mtk_esw_mib_snapshot snapshot;
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	memset(&snapshot, 0, sizeof(snapshot));
	assert(mtk_esw_mib_snapshot_read(0x200U, fake_mib_register_read,
					 &fake, &snapshot) == 0);
	assert(fake.calls == sizeof(expected_offsets) / sizeof(expected_offsets[0]));
	for (i = 0U; i < fake.calls; ++i)
		assert(fake.registers[i] == 0x200U + expected_offsets[i]);

	assert(snapshot.rx_good_octets ==
	       (((unsigned long long)(0x40acU + 0x200U) << 32) |
		(unsigned long long)(0x40a8U + 0x200U)));
	assert(snapshot.tx_good_octets ==
	       (((unsigned long long)(0x404cU + 0x200U) << 32) |
		(unsigned long long)(0x4048U + 0x200U)));
	assert(snapshot.tx_drop_frames == 0x4000U + 0x200U);
	assert(snapshot.tx_crc_error == 0x4004U + 0x200U);
	assert(snapshot.tx_unicast_frames == 0x4008U + 0x200U);
	assert(snapshot.tx_multicast_frames == 0x400cU + 0x200U);
	assert(snapshot.tx_broadcast_frames == 0x4010U + 0x200U);
	assert(snapshot.tx_collision == 0x4014U + 0x200U);
	assert(snapshot.tx_pause_frames == 0x402cU + 0x200U);
	assert(snapshot.rx_drop_frames == 0x4060U + 0x200U);
	assert(snapshot.rx_filter_frames == 0x4064U + 0x200U);
	assert(snapshot.rx_unicast_frames == 0x4068U + 0x200U);
	assert(snapshot.rx_multicast_frames == 0x406cU + 0x200U);
	assert(snapshot.rx_broadcast_frames == 0x4070U + 0x200U);
	assert(snapshot.rx_alignment_error == 0x4074U + 0x200U);
	assert(snapshot.rx_crc_error == 0x4078U + 0x200U);
	assert(snapshot.rx_pause_frames == 0x408cU + 0x200U);
}

static void
test_mib_snapshot_propagates_each_register_failure_without_publishing(void)
{
	struct fake_mib_register_io fake;
	struct mtk_esw_mib_snapshot snapshot, before;
	unsigned int fail_at;

	for (fail_at = 0U; fail_at < 19U; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = (int)fail_at;
		fake.fail_status = -ETIMEDOUT;
		memset(&snapshot, 0xa5, sizeof(snapshot));
		memcpy(&before, &snapshot, sizeof(before));

		assert(mtk_esw_mib_snapshot_read(0x100U,
						 fake_mib_register_read,
						 &fake, &snapshot) == -ETIMEDOUT);
		assert(fake.calls == fail_at + 1U);
		assert(memcmp(&snapshot, &before, sizeof(snapshot)) == 0);
	}
}

static void
test_mib_snapshot_rejects_invalid_read_context_and_range(void)
{
	struct fake_mib_register_io fake;
	struct mtk_esw_mib_snapshot snapshot, before;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	memset(&snapshot, 0x5a, sizeof(snapshot));
	memcpy(&before, &snapshot, sizeof(before));
	assert(mtk_esw_mib_snapshot_read(0U, NULL, &fake, &snapshot) == -EINVAL);
	assert(mtk_esw_mib_snapshot_read(0xffffbf54U,
					 fake_mib_register_read,
					 &fake, &snapshot) == -EINVAL);
	assert(fake.calls == 0U);
	assert(memcmp(&snapshot, &before, sizeof(snapshot)) == 0);
}

static void
test_port_register_writes_continue_after_first_failure(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = -ETIMEDOUT;
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0x100U,
					    4U, 0x0bU, 0x55aaU,
					    &fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == 3U && fake.event_count == 3U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].phy == 0x1fU &&
	       fake.events[0].reg == 0x1000U &&
	       fake.events[0].value == 0x55aaU);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].reg == 0x1100U);
	assert(fake.events[2].type == FAKE_WRITE &&
	       fake.events[2].reg == 0x1300U);

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = 1;
	assert(mtk_esw_register_write_ports(0x1fU, 0x2000U, 0x10U,
					    3U, 0x07U, 0U,
					    &fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 3U && fake.event_count == 3U);
	assert(fake.events[2].reg == 0x2020U);
}

static void
test_port_register_write_rejects_invalid_io_and_ranges(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_write = fake_ops;

	fake_init(&fake);
	missing_write.write = NULL;
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0x100U,
					    4U, 0x0fU, 1U,
					    NULL, &fake) == -EINVAL);
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0x100U,
					    4U, 0x0fU, 1U,
					    &missing_write, &fake) == -EINVAL);
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0U,
					    4U, 0x0fU, 1U,
					    &fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0x100U,
					    0U, 0x0fU, 1U,
					    &fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_register_write_ports(0x1fU, 0x1000U, 0x100U,
					    33U, 0x0fU, 1U,
					    &fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_register_write_ports(0x1fU, ~0U - 0x10U, 0x20U,
					    2U, 0x03U, 1U,
					    &fake_ops, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_jumbo_frame_register_update_is_fail_closed_and_preserves_other_bits(void)
{
	struct fake_mdio fake;
	struct mtk_esw_link_mode_io missing_read = fake_ops;
	struct mtk_esw_link_mode_io missing_write = fake_ops;
	const unsigned int reg = 0x1234U;
	unsigned int expected;

	fake_init(&fake);
	fake.read_value = 0xa5a5c0d0U;
	expected = (fake.read_value & ~MTK_ESW_JUMBO_MAX_RX_MASK) |
		   MTK_ESW_JUMBO_MAX_RX_9K | MTK_ESW_JUMBO_RX_ENABLE;
	assert(mtk_esw_jumbo_frames_apply(reg, 1, &fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[0].type == FAKE_READ &&
	       fake.events[0].phy == MTK_ESW_MAC_TABLE_PHY &&
	       fake.events[0].reg == reg);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].phy == MTK_ESW_MAC_TABLE_PHY &&
	       fake.events[1].reg == reg && fake.events[1].value == expected);

	fake_init(&fake);
	fake.read_value = 0x12345678U;
	expected = (fake.read_value & ~MTK_ESW_JUMBO_MAX_RX_MASK) |
		   MTK_ESW_JUMBO_MAX_RX_9K | MTK_ESW_JUMBO_RX_DISABLE;
	assert(mtk_esw_jumbo_frames_apply(reg, 0, &fake_ops, &fake) == 0);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].value == expected);

	/* A read error must not be converted into a zero-based register write. */
	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -EACCES;
	assert(mtk_esw_jumbo_frames_apply(reg, 1, &fake_ops, &fake) == -EACCES);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_READ);

	/* A write error remains visible and prevents cached-state publication. */
	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = -ETIMEDOUT;
	assert(mtk_esw_jumbo_frames_apply(reg, 0, &fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == 2U && fake.event_count == 2U);
	assert(fake.events[1].type == FAKE_WRITE);

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = 1;
	assert(mtk_esw_jumbo_frames_apply(reg, 1, &fake_ops, &fake) == -EIO);

	fake_init(&fake);
	missing_read.read = NULL;
	missing_write.write = NULL;
	assert(mtk_esw_jumbo_frames_apply(reg, 1, NULL, &fake) == -EINVAL);
	assert(mtk_esw_jumbo_frames_apply(reg, 1, &missing_read, &fake) == -EINVAL);
	assert(mtk_esw_jumbo_frames_apply(reg, 1, &missing_write, &fake) == -EINVAL);
	assert(mtk_esw_jumbo_frames_apply(reg, 2, &fake_ops, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_vlan_entry_apply_stops_on_each_failed_stage(void)
{
	struct fake_mdio fake;
	unsigned int index;

	fake_init(&fake);
	assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
					MTK_ESW_VLAN_TABLE_REG, 27U,
					0x12345678U, 0x87654321U,
					&fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 4U && fake.event_count == 5U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].reg == 0x94U &&
	       fake.events[0].value == 0x12345678U);
	assert(fake.events[1].type == FAKE_WRITE &&
	       fake.events[1].reg == 0x98U &&
	       fake.events[1].value == 0x87654321U);
	assert(fake.events[2].type == FAKE_WRITE &&
	       fake.events[2].reg == MTK_ESW_VLAN_TABLE_REG);
	assert(fake.events[3].type == FAKE_DELAY_US);
	assert(fake.events[4].type == FAKE_READ);

	for (index = 0; index < 4U; ++index) {
		fake_init(&fake);
		fake.fail_at = index;
		fake.fail_status = -EIO;
		assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
						MTK_ESW_VLAN_TABLE_REG, 27U,
						0x12345678U, 0x87654321U,
						&fake_ops, &fake) == -EIO);
		assert(fake.mdio_calls == index + 1U);
		if (index < 3U)
			assert(fake.event_count == index + 1U);
		else
			assert(fake.event_count == 5U);
		assert(fake.events[0].type == FAKE_WRITE);
		if (index == 0U)
			assert(fake.events[0].reg == 0x94U);
		else if (index == 1U)
			assert(fake.events[1].reg == 0x98U);
		else if (index == 2U)
			assert(fake.events[2].reg == MTK_ESW_VLAN_TABLE_REG);
		else
			assert(fake.events[4].type == FAKE_READ);
	}

	fake_init(&fake);
	fake.fail_at = 0U;
	fake.fail_status = 1;
	assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
					MTK_ESW_VLAN_TABLE_REG, 27U,
					0x12345678U, 0x87654321U,
					&fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].reg == 0x94U);

	fake_init(&fake);
	assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
					MTK_ESW_VLAN_TABLE_REG, 4096U,
					1U, 2U, &fake_ops, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);

	assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
					MTK_ESW_VLAN_TABLE_REG, 1U,
					1U, 2U, NULL, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
	/* VLAN entry write errors with positive status are normalized. */
	fake_init(&fake);
	fake.fail_at = 0U;
	fake.fail_status = 1;
	assert(mtk_esw_vlan_entry_apply(0x94U, 0x98U,
					MTK_ESW_VLAN_TABLE_REG, 27U,
					0x12345678U, 0x87654321U,
					&fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_WRITE && fake.events[0].reg == 0x94U);

}

static void
test_vlan_table_write_waits_for_idle_and_propagates_io_failures(void)
{
	struct fake_mdio fake;

	fake_init(&fake);
	fake.busy_reads = 2U;
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 1U, 4094U,
					&fake_ops, &fake) == 0);
	assert(fake.mdio_calls == 4U && fake.event_count == 7U);
	assert(fake.events[0].type == FAKE_WRITE &&
	       fake.events[0].phy == MTK_ESW_MAC_TABLE_PHY &&
	       fake.events[0].reg == MTK_ESW_VLAN_TABLE_REG &&
	       fake.events[0].value ==
		((1U << 12) | 4094U | MTK_ESW_VLAN_TABLE_BUSY_MASK));
	assert(fake.events[1].type == FAKE_DELAY_US &&
	       fake.events[1].value == MTK_ESW_VLAN_TABLE_POLL_DELAY_US);
	assert(fake.events[2].type == FAKE_READ &&
	       (fake.events[2].value & MTK_ESW_VLAN_TABLE_BUSY_MASK));
	assert(fake.events[6].type == FAKE_READ &&
	       !(fake.events[6].value & MTK_ESW_VLAN_TABLE_BUSY_MASK));

	fake_init(&fake);
	fake.fail_at = 0;
	fake.fail_status = -EIO;
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 1U, 2U,
					&fake_ops, &fake) == -EIO);
	assert(fake.mdio_calls == 1U && fake.event_count == 1U);
	assert(fake.events[0].type == FAKE_WRITE);

	fake_init(&fake);
	fake.fail_at = 1;
	fake.fail_status = -EACCES;
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 2U, 4095U,
					&fake_ops, &fake) == -EACCES);
	assert(fake.mdio_calls == 2U && fake.event_count == 3U);
	assert(fake.events[2].type == FAKE_READ);

	fake_init(&fake);
	fake.busy_forever = 1;
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 2U, 1U,
					&fake_ops, &fake) == -ETIMEDOUT);
	assert(fake.mdio_calls == MTK_ESW_VLAN_TABLE_POLL_LIMIT + 1U);
	assert(fake.event_count == 1U + (MTK_ESW_VLAN_TABLE_POLL_LIMIT * 2U));

	fake_init(&fake);
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 0U, 1U,
					&fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 3U, 1U,
					&fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 1U, 4096U,
					&fake_ops, &fake) == -EINVAL);
	assert(mtk_esw_vlan_table_write(MTK_ESW_VLAN_TABLE_REG, 1U, 1U,
					NULL, &fake) == -EINVAL);
	assert(fake.mdio_calls == 0U && fake.event_count == 0U);
}

static void
test_applied_value_cache_is_committed_only_after_success(void)
{
	unsigned int applied = 2U;

	assert(mtk_esw_applied_value_commit(-ETIMEDOUT, 7U, &applied) ==
	       -ETIMEDOUT);
	assert(applied == 2U);

	assert(mtk_esw_applied_value_commit(1, 7U, &applied) == -EIO);
	assert(applied == 2U);

	assert(mtk_esw_applied_value_commit(0, 7U, &applied) == 0);
	assert(applied == 7U);

	assert(mtk_esw_applied_value_commit(0, 8U, NULL) == -EINVAL);
	assert(applied == 7U);
}

static void
test_vlan_rule_snapshot_is_committed_only_after_hardware_success(void)
{
	const unsigned int requested[4] = {
		0x00000002U, 0x00010003U, 0x01020004U, 0x00000005U
	};
	unsigned int applied[4] = {
		0x00000001U, 0x00000006U, 0x00000007U, 0x00000008U
	};
	const unsigned int previous[4] = {
		0x00000001U, 0x00000006U, 0x00000007U, 0x00000008U
	};

	assert(mtk_esw_vlan_rules_commit(-ETIMEDOUT, 4U,
					 requested, applied) == -ETIMEDOUT);
	assert(memcmp(applied, previous, sizeof(applied)) == 0);

	/* Positive callback errors are normalized, without publishing a partial state. */
	assert(mtk_esw_vlan_rules_commit(1, 4U, requested, applied) == -EIO);
	assert(memcmp(applied, previous, sizeof(applied)) == 0);

	assert(mtk_esw_vlan_rules_commit(0, 4U, requested, applied) == 0);
	assert(memcmp(applied, requested, sizeof(applied)) == 0);
}

static void
test_vlan_rule_snapshot_commit_rejects_invalid_buffers(void)
{
	const unsigned int requested[1] = { 1U };
	unsigned int applied[1] = { 2U };

	assert(mtk_esw_vlan_rules_commit(0, 0U, requested, applied) == -EINVAL);
	assert(mtk_esw_vlan_rules_commit(0, 1U, NULL, applied) == -EINVAL);
	assert(mtk_esw_vlan_rules_commit(0, 1U, requested, NULL) == -EINVAL);
	assert(applied[0] == 2U);
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
	test_register_update_bits_preserves_unmasked_bits_and_propagates_errors();
	test_register_write_propagates_mdio_failures();
	test_register_read_propagates_mdio_failures();
	test_mib_snapshot_publishes_complete_counter_set();
	test_mib_snapshot_propagates_each_register_failure_without_publishing();
	test_mib_snapshot_rejects_invalid_read_context_and_range();
	test_port_register_writes_continue_after_first_failure();
	test_port_register_write_rejects_invalid_io_and_ranges();
	test_jumbo_frame_register_update_is_fail_closed_and_preserves_other_bits();
	test_vlan_entry_apply_stops_on_each_failed_stage();
	test_vlan_table_write_waits_for_idle_and_propagates_io_failures();
	test_applied_value_cache_is_committed_only_after_success();
	test_vlan_rule_snapshot_is_committed_only_after_hardware_success();
	test_vlan_rule_snapshot_commit_rejects_invalid_buffers();
	test_all_defined_speed_and_flow_modes();
	test_first_error_accumulator_preserves_first_failure();
	test_reserved_speed_flow_and_bits_are_rejected();
	test_phy_power_set_reads_and_writes_before_reporting_change();
	test_phy_power_set_skips_unchanged_state_but_honors_force();
	test_phy_power_set_read_and_write_errors_are_propagated();
	test_phy_power_set_normalizes_positive_callback_failure();
	test_phy_power_set_rejects_invalid_inputs();
	test_mac_table_clear_waits_for_confirmed_idle_state();
	test_mac_table_clear_preserves_static_only_command();
	test_mac_table_clear_aborts_when_command_write_fails();
	test_mac_table_clear_treats_read_failure_as_unknown();
	test_mac_table_clear_times_out_when_busy_never_clears();
	test_mac_table_clear_rejects_invalid_mode_and_missing_delay();
	test_autoneg_register_sequence_and_cache_ready_result();
	test_forced_mode_power_cycles_before_final_control_write();
	test_power_off_sets_down_bit_without_delay();
	test_read_and_each_write_failure_abort_without_later_io();
	test_mdio_errors_are_preserved_and_positive_failures_become_eio();
	test_forced_final_write_failure_is_not_hidden_after_delay();
	test_invalid_io_and_boolean_arguments_are_rejected();
	puts("MTK ESW link-mode and MAC-table MDIO policies: PASS");
	return 0;
}
