#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../ra_switch_mdio_policy.h"

struct fake_io {
	unsigned int calls, reg[3], value[3], dev[3];
	int fail_at, fail_status;
	unsigned int read_value;
	char op[3];
};

static int fake_read(void *ctx, unsigned int port, unsigned int reg, unsigned int *value)
{
	struct fake_io *f = ctx;
	unsigned int n = f->calls++;
	(void)port;
	assert(n < 3U);
	f->op[n] = 'r'; f->reg[n] = reg;
	*value = 0xdeadbeefU;
	if ((int)n == f->fail_at) return f->fail_status;
	*value = f->read_value ? f->read_value : 0x12345678U;
	return 0;
}

static int fake_read_cl45(void *ctx, unsigned int port, unsigned int dev,
			  unsigned int reg, unsigned int *value)
{
	struct fake_io *f = ctx;
	unsigned int n = f->calls++;
	(void)port;
	assert(n < 3U);
	f->op[n] = 'c';
	f->reg[n] = reg;
	f->value[n] = dev;
	*value = 0xdeadbeefU;
	if ((int)n == f->fail_at)
		return f->fail_status;
	*value = f->read_value ? f->read_value : 0x12345678U;
	return 0;
}

static int fake_write(void *ctx, unsigned int port, unsigned int reg, unsigned int value)
{
	struct fake_io *f = ctx;
	unsigned int n = f->calls++;
	(void)port;
	assert(n < 3U);
	f->op[n] = 'w'; f->reg[n] = reg; f->value[n] = value;
	if ((int)n == f->fail_at) return f->fail_status;
	return 0;
}

static int fake_write_cl45(void *ctx, unsigned int port,
			    unsigned int dev, unsigned int reg,
			    unsigned int value)
{
	struct fake_io *f = ctx;
	unsigned int n = f->calls++;

	(void)port;
	assert(n < 3U);
	f->op[n] = 'C';
	f->dev[n] = dev;
	f->reg[n] = reg;
	f->value[n] = value;
	if ((int)n == f->fail_at)
		return f->fail_status;
	return 0;
}

static void test_checked_phy_writes_propagate_and_normalize_failures(void)
{
	struct fake_io f;

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_write_checked(3U, 0x1fU, 0x1234U,
					    fake_write, &f) == 0);
	assert(f.calls == 1U && f.op[0] == 'w' &&
	       f.reg[0] == 0x1fU && f.value[0] == 0x1234U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -ETIMEDOUT;
	assert(ra_switch_mdio_write_checked(3U, 0x1fU, 0x1234U,
					    fake_write, &f) == -ETIMEDOUT);
	assert(f.calls == 1U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	assert(ra_switch_mdio_write_checked(3U, 0x1fU, 0x1234U,
					    fake_write, &f) == -EIO);
	assert(f.calls == 1U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_write_checked(3U, 0x1fU, 0x1234U,
					    NULL, &f) == -EINVAL);
	assert(f.calls == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_write_cl45_checked(4U, 0x1eU, 0x145U,
						  0x1010U, fake_write_cl45,
						  &f) == 0);
	assert(f.calls == 1U && f.op[0] == 'C' &&
	       f.dev[0] == 0x1eU && f.reg[0] == 0x145U &&
	       f.value[0] == 0x1010U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -EHOSTDOWN;
	assert(ra_switch_mdio_write_cl45_checked(4U, 0x1eU, 0x145U,
						  0x1010U, fake_write_cl45,
						  &f) == -EHOSTDOWN);
	assert(f.calls == 1U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	assert(ra_switch_mdio_write_cl45_checked(4U, 0x1eU, 0x145U,
						  0x1010U, fake_write_cl45,
						  &f) == -EIO);
	assert(f.calls == 1U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_write_cl45_checked(4U, 0x1eU, 0x145U,
						  0x1010U, NULL, &f) == -EINVAL);
	assert(f.calls == 0U);
}

struct fake_cal_wait_io {
	unsigned int calls;
	unsigned int read_value;
	unsigned int reg[8];
	unsigned int value[8];
	char op[8];
	int fail_at;
	int fail_status;
	unsigned int delays;
};

static int fake_cal_wait_read(void *context, unsigned int port,
			      unsigned int reg, unsigned int *value)
{
	struct fake_cal_wait_io *fake = context;
	unsigned int n = fake->calls++;

	(void)port;
	assert(n < 8U);
	fake->op[n] = 'r';
	fake->reg[n] = reg;
	*value = 0xdeadbeefU;
	if ((int)n == fake->fail_at)
		return fake->fail_status;
	*value = fake->read_value;
	return 0;
}

static int fake_cal_wait_write(void *context, unsigned int port,
			       unsigned int reg, unsigned int value)
{
	struct fake_cal_wait_io *fake = context;
	unsigned int n = fake->calls++;

	(void)port;
	assert(n < 8U);
	fake->op[n] = 'w';
	fake->reg[n] = reg;
	fake->value[n] = value;
	if ((int)n == fake->fail_at)
		return fake->fail_status;
	return 0;
}

static void fake_cal_wait_delay(void *context)
{
	struct fake_cal_wait_io *fake = context;

	++fake->delays;
}

static void test_calibration_wait_checks_all_io_and_restores_trigger(void)
{
	struct fake_cal_wait_io fake;
	unsigned int ready;
	unsigned int i;
	int status;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.read_value = 0x2U;
	status = ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 3U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake, &ready);
	assert(status == 0 && ready == 1U);
	assert(fake.calls == 7U && fake.delays == 1U);
	assert(fake.op[0] == 'r' && fake.op[1] == 'w' &&
	       fake.op[2] == 'r' && fake.op[3] == 'w' &&
	       fake.op[4] == 'r' && fake.op[5] == 'r' &&
	       fake.op[6] == 'w');
	assert(fake.reg[0] == 24U && fake.value[1] == 0x2U);
	assert(fake.value[3] == 0x12U && fake.value[6] == 0x2U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.read_value = 0U;
	ready = 1U;
	status = ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 2U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake, &ready);
	assert(status == 0 && ready == 0U);
	assert(fake.calls == 8U && fake.delays == 2U);
	assert(fake.op[6] == 'r' && fake.op[7] == 'w');

	for (i = 0U; i < 5U; ++i) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = (int)i;
		fake.fail_status = -ETIMEDOUT;
		fake.read_value = 0x2U;
		ready = 1U;
		status = ra_switch_mdio_calibration_wait(
			0U, 24U, 0x10U, 0x2U, 2U, fake_cal_wait_read,
			fake_cal_wait_write, &fake, fake_cal_wait_delay,
			&fake, &ready);
		assert(status == -ETIMEDOUT && ready == 0U);
		assert(fake.calls == i + 3U);
	}
	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 4;
	fake.fail_status = 1;
	fake.read_value = 0x2U;
	ready = 1U;
	status = ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 2U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake, &ready);
	assert(status == -EIO && ready == 0U && fake.calls == 7U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 5;
	fake.fail_status = -EHOSTDOWN;
	fake.read_value = 0x2U;
	ready = 1U;
	status = ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 2U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake, &ready);
	assert(status == -EHOSTDOWN && ready == 0U && fake.calls == 6U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 6;
	fake.fail_status = -ETIMEDOUT;
	fake.read_value = 0x2U;
	ready = 1U;
	status = ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 2U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake, &ready);
	assert(status == -ETIMEDOUT && ready == 0U && fake.calls == 7U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 2U, NULL, fake_cal_wait_write,
		&fake, fake_cal_wait_delay, &fake, &ready) == -EINVAL);
	assert(ra_switch_mdio_calibration_wait(
		0U, 24U, 0x10U, 0x2U, 0U, fake_cal_wait_read,
		fake_cal_wait_write, &fake, fake_cal_wait_delay, &fake,
		&ready) == -EINVAL);
	assert(fake.calls == 0U);
}

static void test_restore_cl45_register_always_attempts_disable(void)
{
	struct fake_io fake;
	int status;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	status = ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 1,
		0x1eU, 0x00dbU, fake_write_cl45, &fake);
	assert(status == 0 && fake.calls == 2U);
	assert(fake.op[0] == 'C' && fake.dev[0] == 0x1eU &&
	       fake.reg[0] == 0x00e0U && fake.value[0] == 0x1234U);
	assert(fake.op[1] == 'C' && fake.dev[1] == 0x1eU &&
	       fake.reg[1] == 0x00dbU && fake.value[1] == 0U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.fail_status = -EHOSTDOWN;
	status = ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 1,
		0x1eU, 0x00dbU, fake_write_cl45, &fake);
	assert(status == -EHOSTDOWN && fake.calls == 2U);
	assert(fake.reg[1] == 0x00dbU && fake.value[1] == 0U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	status = ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 0,
		0x1eU, 0x00dbU, fake_write_cl45, &fake);
	assert(status == 0 && fake.calls == 1U);
	assert(fake.reg[0] == 0x00dbU && fake.value[0] == 0U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.fail_status = 1;
	status = ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 0,
		0x1eU, 0x00dbU, fake_write_cl45, &fake);
	assert(status == -EIO && fake.calls == 1U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 1;
	fake.fail_status = -ETIMEDOUT;
	status = ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 1,
		0x1eU, 0x00dbU, fake_write_cl45, &fake);
	assert(status == -ETIMEDOUT && fake.calls == 2U);
	assert(fake.reg[1] == 0x00dbU && fake.value[1] == 0U);

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(ra_switch_mdio_restore_cl45_and_disable(
		3U, 0x1eU, 0x00e0U, 0x1234U, 1,
		0x1eU, 0x00dbU, NULL, &fake) == -EINVAL);
	assert(fake.calls == 0U);
}

static void test_checked_update_bits_stops_on_read_error_and_preserves_bits(void)
{
	struct fake_io f;
	unsigned int updated = 0xfeedfaceU;
	int status;

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	f.read_value = 0xdeadU;
	status = ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x6000U, 0U, fake_read, fake_write, &f, &updated);
	assert(status == 0 && f.calls == 2U);
	assert(f.op[0] == 'r' && f.reg[0] == 24U);
	assert(f.op[1] == 'w' && f.reg[1] == 24U);
	assert(f.value[1] == (0xdeadU & ~0x6000U));
	assert(updated == (0xdeadU & ~0x6000U));

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -ETIMEDOUT;
	updated = 0xfeedfaceU;
	status = ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, fake_read, fake_write, &f, &updated);
	assert(status == -ETIMEDOUT && f.calls == 1U);
	assert(updated == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	updated = 0xfeedfaceU;
	status = ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, fake_read, fake_write, &f, &updated);
	assert(status == -EIO && f.calls == 1U && updated == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 1;
	f.fail_status = -EHOSTDOWN;
	updated = 0xfeedfaceU;
	status = ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, fake_read, fake_write, &f, &updated);
	assert(status == -EHOSTDOWN && f.calls == 2U && updated == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 1;
	f.fail_status = 1;
	updated = 0xfeedfaceU;
	status = ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0x1000U, fake_read, fake_write, &f, &updated);
	assert(status == -EIO && f.calls == 2U && updated == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, NULL, fake_write, &f,
		&updated) == -EINVAL);
	assert(ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, fake_read, NULL, &f,
		&updated) == -EINVAL);
	assert(ra_switch_mdio_update_bits_checked(
		2U, 24U, 0x2000U, 0U, fake_read, fake_write, &f,
		NULL) == -EINVAL);
	assert(f.calls == 0U);
}

static void test_checked_cleanup_attempts_both_writes_and_keeps_first_error(void)
{
	struct fake_io f;
	int status;

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	status = ra_switch_mdio_cleanup_two_writes(
		1U, 23U, 0U, 2U, 23U, 0U, fake_write, &f);
	assert(status == 0 && f.calls == 2U);
	assert(f.op[0] == 'w' && f.op[1] == 'w');
	assert(f.reg[0] == 23U && f.reg[1] == 23U);
	assert(f.value[0] == 0U && f.value[1] == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -ETIMEDOUT;
	status = ra_switch_mdio_cleanup_two_writes(
		1U, 23U, 0U, 2U, 23U, 0U, fake_write, &f);
	assert(status == -ETIMEDOUT && f.calls == 2U);
	assert(f.reg[1] == 23U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	status = ra_switch_mdio_cleanup_two_writes(
		1U, 23U, 0U, 2U, 23U, 0U, fake_write, &f);
	assert(status == -EIO && f.calls == 2U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 1;
	f.fail_status = -EHOSTDOWN;
	status = ra_switch_mdio_cleanup_two_writes(
		1U, 23U, 0U, 2U, 23U, 0U, fake_write, &f);
	assert(status == -EHOSTDOWN && f.calls == 2U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_cleanup_two_writes(
		1U, 23U, 0U, 2U, 23U, 0U, NULL, &f) == -EINVAL);
	assert(f.calls == 0U);
}

static void test_page_read_selects_global_and_local_bank(void)
{
	struct fake_io f;
	unsigned int value = 0;
	memset(&f, 0, sizeof(f)); f.fail_at = -1;
	assert(ra_switch_mdio_page_read(0, 3, 5, 7, fake_read, fake_write, &f, &value) == 0);
	assert(f.calls == 2 && f.op[0] == 'w' && f.reg[0] == 31 && f.value[0] == 0x5000);
	assert(f.op[1] == 'r' && f.reg[1] == 7 && value == 0x12345678U);
	memset(&f, 0, sizeof(f)); f.fail_at = -1;
	assert(ra_switch_mdio_page_read(1, 4, 6, 9, fake_read, fake_write, &f, &value) == 0);
	assert(f.calls == 2 && f.value[0] == 0xe000 && value == 0x12345678U);
}

static void test_page_select_failure_stops_second_access(void)
{
	struct fake_io f;
	unsigned int value = 0xfeedfaceU;
	memset(&f, 0, sizeof(f)); f.fail_at = 0; f.fail_status = -ETIMEDOUT;
	assert(ra_switch_mdio_page_read(0, 1, 2, 3, fake_read, fake_write, &f, &value) == -ETIMEDOUT);
	assert(f.calls == 1 && value == 0);
	memset(&f, 0, sizeof(f)); f.fail_at = 0; f.fail_status = -EIO;
	assert(ra_switch_mdio_page_write(1, 1, 2, 3, 4, fake_write, &f) == -EIO);
	assert(f.calls == 1 && f.reg[0] == 31);
}

static void test_register_failures_and_positive_status_are_safe(void)
{
	struct fake_io f;
	unsigned int value = 0xfeedfaceU;
	memset(&f, 0, sizeof(f)); f.fail_at = 1; f.fail_status = -EHOSTDOWN;
	assert(ra_switch_mdio_page_read(1, 2, 3, 4, fake_read, fake_write, &f, &value) == -EHOSTDOWN);
	assert(f.calls == 2 && value == 0);
	memset(&f, 0, sizeof(f)); f.fail_at = 1; f.fail_status = -ETIMEDOUT;
	assert(ra_switch_mdio_page_write(0, 2, 3, 4, 5, fake_write, &f) == -ETIMEDOUT);
	assert(f.calls == 2 && f.reg[1] == 4 && f.value[1] == 5);
	memset(&f, 0, sizeof(f)); f.fail_at = 0; f.fail_status = 1;
	assert(ra_switch_mdio_page_read(0, 0, 0, 1, fake_read, fake_write, &f, &value) == -EIO);
	assert(f.calls == 1 && value == 0);
	memset(&f, 0, sizeof(f)); f.fail_at = 1; f.fail_status = 1;
	assert(ra_switch_mdio_page_write(0, 0, 0, 1, 2, fake_write, &f) == -EIO);
	assert(f.calls == 2);
}

static void test_checked_cl45_reads_reject_partial_samples(void)
{
	struct fake_io f;
	unsigned int value = 0xfeedfaceU;

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -ETIMEDOUT;
	assert(ra_switch_mdio_read_cl45_checked(4U, 0x1eU, 0x9bU,
			fake_read_cl45, &f, &value) == -ETIMEDOUT);
	assert(f.calls == 1U && f.op[0] == 'c' &&
	       f.value[0] == 0x1eU && f.reg[0] == 0x9bU && value == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	value = 0xfeedfaceU;
	assert(ra_switch_mdio_read_cl45_checked(4U, 0x1eU, 0x9cU,
			fake_read_cl45, &f, &value) == -EIO);
	assert(f.calls == 1U && value == 0U);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	f.read_value = 0xa55aU;
	value = 0U;
	assert(ra_switch_mdio_read_cl45_checked(4U, 0x1eU, 0x9bU,
			fake_read_cl45, &f, &value) == 0);
	assert(f.calls == 1U && value == 0xa55aU);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	value = 0xfeedfaceU;
	assert(ra_switch_mdio_read_cl45_checked(4U, 0x1eU, 0x9bU,
			NULL, &f, &value) == -EINVAL);
	assert(f.calls == 0U && value == 0xfeedfaceU);
	assert(ra_switch_mdio_read_cl45_checked(4U, 0x1eU, 0x9bU,
			fake_read_cl45, &f, NULL) == -EINVAL);
	assert(f.calls == 0U);
}

static void test_checked_link_reads_preserve_state_on_failure(void)
{
	struct fake_io f;
	int link_up = 7;
	unsigned int value = 0xfeedfaceU;

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -ETIMEDOUT;
	assert(ra_switch_mdio_read_checked(1, 0x3008, fake_read, &f, &value) == -ETIMEDOUT);
	assert(f.calls == 1 && value == 0);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = -EIO;
	assert(ra_switch_mdio_read_link_status(1, 0x3008, 1U, fake_read, &f, &link_up) == -EIO);
	assert(f.calls == 1 && link_up == 7);

	memset(&f, 0, sizeof(f));
	f.fail_at = 0;
	f.fail_status = 1;
	assert(ra_switch_mdio_read_link_status(1, 1, 4U, fake_read, &f, &link_up) == -EIO);
	assert(f.calls == 1 && link_up == 7);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	f.read_value = 4U;
	assert(ra_switch_mdio_read_link_status(1, 1, 4U, fake_read, &f, &link_up) == 0);
	assert(f.calls == 1 && link_up == 1);

	memset(&f, 0, sizeof(f));
	f.fail_at = -1;
	assert(ra_switch_mdio_read_link_status(1, 1, 0U, fake_read, &f, &link_up) == -EINVAL);
	assert(f.calls == 0 && link_up == 1);
}

static void test_invalid_callbacks_are_rejected_without_io(void)
{
	struct fake_io f;
	unsigned int value = 0x55;
	int link_up = 3;
	memset(&f, 0, sizeof(f)); f.fail_at = -1;
	assert(ra_switch_mdio_page_read(0, 0, 0, 1, NULL, fake_write, &f, &value) == -EINVAL);
	assert(ra_switch_mdio_page_read(0, 0, 0, 1, fake_read, NULL, &f, &value) == -EINVAL);
	assert(ra_switch_mdio_page_read(0, 0, 0, 1, fake_read, fake_write, &f, NULL) == -EINVAL);
	assert(ra_switch_mdio_read_checked(0, 0, NULL, &f, &value) == -EINVAL);
	assert(ra_switch_mdio_read_checked(0, 0, fake_read, &f, NULL) == -EINVAL);
	assert(ra_switch_mdio_read_link_status(0, 0, 1U, NULL, &f, &link_up) == -EINVAL);
	assert(ra_switch_mdio_read_link_status(0, 0, 1U, fake_read, &f, NULL) == -EINVAL);
	assert(ra_switch_mdio_page_write(0, 0, 0, 1, 2, NULL, &f) == -EINVAL);
	assert(f.calls == 0 && value == 0x55 && link_up == 3);
}

struct fake_reset_poll {
	unsigned int calls;
	unsigned int delays;
	unsigned int ready_after;
	int fail_status;
};

static int fake_poll_read(void *context, unsigned int port,
			  unsigned int reg, unsigned int *value)
{
	struct fake_reset_poll *fake = context;
	(void)port;
	(void)reg;
	++fake->calls;
	if (fake->fail_status)
		return fake->fail_status;
	*value = fake->calls >= fake->ready_after ? 1U : 0U;
	return 0;
}

static void fake_poll_delay(void *context)
{
	struct fake_reset_poll *fake = context;
	++fake->delays;
}

static void test_reset_poll_waits_until_ready_and_is_bounded(void)
{
	struct fake_reset_poll fake;
	unsigned int value;

	memset(&fake, 0, sizeof(fake));
	fake.ready_after = 3U;
	assert(ra_switch_mdio_wait_nonzero(fake_poll_read, fake_poll_delay,
					   &fake, 31U, 0x7800U, 5U,
					   &value) == 0);
	assert(fake.calls == 3U && fake.delays == 3U && value == 1U);

	memset(&fake, 0, sizeof(fake));
	fake.ready_after = 99U;
	value = 0xfeedfaceU;
	assert(ra_switch_mdio_wait_nonzero(fake_poll_read, fake_poll_delay,
					   &fake, 31U, 0x7800U, 4U,
					   &value) == -ETIMEDOUT);
	assert(fake.calls == 4U && fake.delays == 4U && value == 0U);

	memset(&fake, 0, sizeof(fake));
	fake.ready_after = 99U;
	fake.fail_status = -EIO;
	value = 0xfeedfaceU;
	assert(ra_switch_mdio_wait_nonzero(fake_poll_read, fake_poll_delay,
					   &fake, 31U, 0x7800U, 4U,
					   &value) == -EIO);
	assert(fake.calls == 1U && fake.delays == 1U && value == 0U);
}

struct fake_phy_poll_io {
	unsigned int calls;
	unsigned int reg[6];
	unsigned int dev[6];
	unsigned int value[6];
	char op[6];
	int fail_at;
	int fail_status;
};

static int fake_phy_poll_next(struct fake_phy_poll_io *fake, char op,
			      unsigned int dev, unsigned int reg,
			      unsigned int *value)
{
	unsigned int n = fake->calls++;

	assert(n < 6U);
	fake->op[n] = op;
	fake->dev[n] = dev;
	fake->reg[n] = reg;
	if ((int)n == fake->fail_at)
		return fake->fail_status;
	*value = fake->value[n];
	return 0;
}

static int fake_phy_poll_read_c22(void *context, unsigned int port,
				  unsigned int reg, unsigned int *value)
{
	(void)port;
	return fake_phy_poll_next(context, '2', 0U, reg, value);
}

static int fake_phy_poll_read_cl45(void *context, unsigned int port,
				    unsigned int dev, unsigned int reg,
				    unsigned int *value)
{
	(void)port;
	return fake_phy_poll_next(context, '5', dev, reg, value);
}

static void init_fake_phy_poll(struct fake_phy_poll_io *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake->fail_at = -1;
	fake->value[0] = 0x4U;
	fake->value[1] = 0xc00U;
	fake->value[2] = 0xc00U;
	fake->value[3] = 0x28U;
	fake->value[4] = 0x6U;
	fake->value[5] = 0x6U;
}

static void assert_empty_phy_poll_snapshot(
		const struct ra_switch_mdio_phy_link_snapshot *snapshot)
{
	assert(snapshot->link_up == 0U);
	assert(snapshot->fc_phy == 0U);
	assert(snapshot->fc_lp == 0U);
	assert(snapshot->link_speed == 0U);
	assert(snapshot->local_eee == 0U);
	assert(snapshot->lp_eee == 0U);
}

static void test_phy_poll_snapshot_stops_and_discards_every_failed_read(void)
{
	struct fake_phy_poll_io fake;
	struct ra_switch_mdio_phy_link_snapshot snapshot;
	unsigned int i;
	int status;

	for (i = 0U; i < 6U; ++i) {
		init_fake_phy_poll(&fake);
		fake.fail_at = (int)i;
		fake.fail_status = -ETIMEDOUT;
		memset(&snapshot, 0xa5, sizeof(snapshot));
		status = ra_switch_mdio_read_phy_link_snapshot(
			0U, fake_phy_poll_read_c22,
			fake_phy_poll_read_cl45, &fake, &snapshot);
		assert(status == -ETIMEDOUT);
		assert(fake.calls == i + 1U);
		assert_empty_phy_poll_snapshot(&snapshot);
	}

	init_fake_phy_poll(&fake);
	fake.fail_at = 4;
	fake.fail_status = 1;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	status = ra_switch_mdio_read_phy_link_snapshot(
		0U, fake_phy_poll_read_c22, fake_phy_poll_read_cl45,
		&fake, &snapshot);
	assert(status == -EIO && fake.calls == 5U);
	assert_empty_phy_poll_snapshot(&snapshot);
}

static void test_phy_poll_snapshot_publishes_only_complete_sample(void)
{
	struct fake_phy_poll_io fake;
	struct ra_switch_mdio_phy_link_snapshot snapshot;

	init_fake_phy_poll(&fake);
	memset(&snapshot, 0, sizeof(snapshot));
	assert(ra_switch_mdio_read_phy_link_snapshot(
		0U, fake_phy_poll_read_c22, fake_phy_poll_read_cl45,
		&fake, &snapshot) == 0);
	assert(fake.calls == 6U && snapshot.link_up == 1U);
	assert(snapshot.fc_phy == 0xc00U && snapshot.fc_lp == 0xc00U);
	assert(snapshot.link_speed == 0x28U);
	assert(snapshot.local_eee == 0x6U && snapshot.lp_eee == 0x6U);
	assert(fake.op[0] == '2' && fake.reg[0] == 0x1U);
	assert(fake.op[1] == '2' && fake.reg[1] == 0x4U);
	assert(fake.op[2] == '2' && fake.reg[2] == 0x5U);
	assert(fake.op[3] == '5' && fake.dev[3] == 0x1eU && fake.reg[3] == 0xa2U);
	assert(fake.op[4] == '5' && fake.dev[4] == 0x7U && fake.reg[4] == 0x3cU);
	assert(fake.op[5] == '5' && fake.dev[5] == 0x7U && fake.reg[5] == 0x3dU);

	init_fake_phy_poll(&fake);
	fake.value[0] = 0U;
	assert(ra_switch_mdio_read_phy_link_snapshot(
		0U, fake_phy_poll_read_c22, fake_phy_poll_read_cl45,
		&fake, &snapshot) == 0);
	assert(fake.calls == 1U && snapshot.link_up == 0U);
	assert_empty_phy_poll_snapshot(&snapshot);
}

static void test_ge2_mac_value_rebuild_clears_stale_eee_bits(void)
{
	unsigned int value = 0xffU;

	value = ra_switch_mdio_update_ge2_mac_value(
		value, 0xc00U, 0xc00U, 0x28U, 0x6U, 0x6U);
	assert((value & 0xf2U) == 0xf2U);
	assert((value & 0xc0U) == 0xc0U);

	value = ra_switch_mdio_update_ge2_mac_value(
		value, 0xc00U, 0x400U, 0x04U, 0U, 0U);
	assert((value & 0x30U) == 0U);
	assert((value & 0x02U) == 0U);
	assert((value & 0x0cU) == 0x04U);
	assert((value & 0xc0U) == 0U);
	assert((value & 0x01U) == 0x01U);

	value = ra_switch_mdio_update_ge2_mac_value(
		value, 0U, 0U, 0x08U, 0x2U, 0x2U);
	assert((value & 0x80U) == 0U && (value & 0x40U) != 0U);
}

static void test_phy_poll_snapshot_rejects_invalid_callbacks(void)
{
	struct fake_phy_poll_io fake;
	struct ra_switch_mdio_phy_link_snapshot snapshot;

	init_fake_phy_poll(&fake);
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(ra_switch_mdio_read_phy_link_snapshot(
		0U, NULL, fake_phy_poll_read_cl45, &fake, &snapshot) == -EINVAL);
	assert(fake.calls == 0U);
	assert(ra_switch_mdio_read_phy_link_snapshot(
		0U, fake_phy_poll_read_c22, NULL, &fake, &snapshot) == -EINVAL);
	assert(fake.calls == 0U);
	assert(ra_switch_mdio_read_phy_link_snapshot(
		0U, fake_phy_poll_read_c22, fake_phy_poll_read_cl45,
		&fake, NULL) == -EINVAL);
	assert(fake.calls == 0U);
}

int main(void)
{
	test_page_read_selects_global_and_local_bank();
	test_checked_cleanup_attempts_both_writes_and_keeps_first_error();
	test_restore_cl45_register_always_attempts_disable();
	test_checked_update_bits_stops_on_read_error_and_preserves_bits();
	test_calibration_wait_checks_all_io_and_restores_trigger();
	test_checked_phy_writes_propagate_and_normalize_failures();
	test_page_select_failure_stops_second_access();
	test_register_failures_and_positive_status_are_safe();
	test_checked_link_reads_preserve_state_on_failure();
	test_checked_cl45_reads_reject_partial_samples();
	test_invalid_callbacks_are_rejected_without_io();
	test_reset_poll_waits_until_ready_and_is_bounded();
	test_phy_poll_snapshot_stops_and_discards_every_failed_read();
	test_phy_poll_snapshot_publishes_only_complete_sample();
	test_ge2_mac_value_rebuild_clears_stale_eee_bits();
	test_phy_poll_snapshot_rejects_invalid_callbacks();
	puts("RAETH paged PHY/link and GE2 polling MDIO failure propagation: PASS");
	return 0;
}
