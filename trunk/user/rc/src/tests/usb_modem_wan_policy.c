#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "usb_modem_wan_policy.h"

struct fake_nvram_writer {
	int result;
	int saved_errno;
	int calls;
	char name[32];
	int value;
};

static int
fake_nvram_set_int(void *opaque, const char *name, int value)
{
	struct fake_nvram_writer *fake = (struct fake_nvram_writer *)opaque;

	fake->calls++;
	snprintf(fake->name, sizeof(fake->name), "%s", name);
	fake->value = value;
	errno = fake->saved_errno;
	return fake->result;
}

static void
test_success_publishes_unit_key_and_value(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.saved_errno = EACCES;
	assert(usb_modem_wan_publish_nvram(0, 0, fake_nvram_set_int, &fake) == 0);
	assert(fake.calls == 1);
	assert(strcmp(fake.name, "wan0_modem_dev") == 0);
	assert(fake.value == 0);
}

static void
test_nonzero_unit_formats_expected_key(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(usb_modem_wan_publish_nvram(12, 7, fake_nvram_set_int, &fake) == 0);
	assert(strcmp(fake.name, "wan12_modem_dev") == 0);
	assert(fake.value == 7);
}

static void
test_write_failure_preserves_errno(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.result = -1;
	fake.saved_errno = ENOSPC;
	assert(usb_modem_wan_publish_nvram(0, 1, fake_nvram_set_int, &fake) == ENOSPC);
	assert(fake.calls == 1);
}

static void
test_write_failure_without_errno_fails_closed(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.result = -1;
	assert(usb_modem_wan_publish_nvram(0, 1, fake_nvram_set_int, &fake) == EIO);
	assert(fake.calls == 1);
}

static void
test_unexpected_nonzero_result_fails_closed(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.result = 1;
	assert(usb_modem_wan_publish_nvram(0, 1, fake_nvram_set_int, &fake) == EIO);
	assert(fake.calls == 1);
}

static void
test_invalid_arguments_do_not_write(void)
{
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(usb_modem_wan_publish_nvram(-1, 0, fake_nvram_set_int, &fake) == EINVAL);
	assert(usb_modem_wan_publish_nvram(0, -1, fake_nvram_set_int, &fake) == EINVAL);
	assert(usb_modem_wan_publish_nvram(0, 0, NULL, &fake) == EINVAL);
	assert(fake.calls == 0);
}

static void
test_key_format_rejects_truncation(void)
{
	char key[8] = "x";

	assert(usb_modem_wan_format_nvram_key(0, key, sizeof(key)) == ENAMETOOLONG);
	assert(key[0] == '\0');
}

static void
test_key_format_rejects_invalid_arguments(void)
{
	char key[32];

	assert(usb_modem_wan_format_nvram_key(-1, key, sizeof(key)) == EINVAL);
	assert(usb_modem_wan_format_nvram_key(0, NULL, sizeof(key)) == EINVAL);
	assert(usb_modem_wan_format_nvram_key(0, key, 0) == EINVAL);
}

int
main(void)
{
	test_success_publishes_unit_key_and_value();
	test_nonzero_unit_formats_expected_key();
	test_write_failure_preserves_errno();
	test_write_failure_without_errno_fails_closed();
	test_unexpected_nonzero_result_fails_closed();
	test_invalid_arguments_do_not_write();
	test_key_format_rejects_truncation();
	test_key_format_rejects_invalid_arguments();
	puts("USB modem WAN NVRAM publication policy regression: PASS");
	return 0;
}
