#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../ra_switch_mdio_policy.h"

struct fake_io {
	unsigned int calls, reg[3], value[3];
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

int main(void)
{
	test_page_read_selects_global_and_local_bank();
	test_page_select_failure_stops_second_access();
	test_register_failures_and_positive_status_are_safe();
	test_checked_link_reads_preserve_state_on_failure();
	test_invalid_callbacks_are_rejected_without_io();
	test_reset_poll_waits_until_ready_and_is_bounded();
	puts("RAETH paged PHY/link MDIO failure propagation: PASS");
	return 0;
}
