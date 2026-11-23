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

static void
test_missing_modem_dev_retains_zero_default(void)
{
	int devnum = 99;
	assert(usb_modem_wan_parse_devnum_status(0, 0, NULL, &devnum) == 0);
	assert(devnum == 0);
}

static void
test_present_modem_dev_parses_zero_and_nonzero(void)
{
	int devnum = -1;
	assert(usb_modem_wan_parse_devnum_status(1, 0, "0", &devnum) == 0);
	assert(devnum == 0);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "7", &devnum) == 0);
	assert(devnum == 7);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "-1", &devnum) == 0);
	assert(devnum == -1);
}

static void
test_parser_preserves_legacy_atoi_numeric_prefix_semantics(void)
{
	int devnum = 0;
	assert(usb_modem_wan_parse_devnum_status(1, 0, " 2\t", &devnum) == 0);
	assert(devnum == 2);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "2modem", &devnum) == 0);
	assert(devnum == 2);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "modem", &devnum) == 0);
	assert(devnum == 0);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "", &devnum) == 0);
	assert(devnum == 0);
	assert(usb_modem_wan_parse_devnum_status(1, 0, "  \t", &devnum) == 0);
	assert(devnum == 0);
}

static void
test_backend_read_errors_are_not_treated_as_absent_key(void)
{
	int devnum = 7;
	assert(usb_modem_wan_parse_devnum_status(-1, ENOSPC, NULL, &devnum) == ENOSPC);
	assert(devnum == 0);
	assert(usb_modem_wan_parse_devnum_status(-1, 0, NULL, &devnum) == EIO);
	assert(usb_modem_wan_parse_devnum_status(2, 0, "7", &devnum) == EIO);
	assert(usb_modem_wan_parse_devnum_status(1, 0, NULL, &devnum) == EIO);
}

static void
test_integer_overflow_is_reported(void)
{
	int devnum = 0;
	assert(usb_modem_wan_parse_devnum_status(
		1, 0, "999999999999999999999999", &devnum) == ERANGE);
	assert(devnum == 0);
}

static void
test_null_output_is_rejected(void)
{
	assert(usb_modem_wan_parse_devnum_status(0, 0, NULL, NULL) == EINVAL);
}

static void
test_devnum_match_requires_exact_selected_identity(void)
{
	int matches = 0;

	assert(usb_modem_wan_devnum_matches_status(1, 0, "17", 17, &matches) == 0);
	assert(matches == 1);
	assert(usb_modem_wan_devnum_matches_status(1, 0, "18", 17, &matches) == 0);
	assert(matches == 0);
}

static void
test_devnum_match_preserves_absent_key_default(void)
{
	int matches = 1;

	assert(usb_modem_wan_devnum_matches_status(0, 0, NULL, 17, &matches) == 0);
	assert(matches == 0);
	assert(usb_modem_wan_devnum_matches_status(0, 0, NULL, 0, &matches) == 0);
	assert(matches == 1);
}

static void
test_devnum_match_propagates_backend_errors(void)
{
	int matches = 1;

	assert(usb_modem_wan_devnum_matches_status(-1, ENOSPC, NULL, 17, &matches) == ENOSPC);
	assert(matches == 0);
	assert(usb_modem_wan_devnum_matches_status(-1, 0, NULL, 17, &matches) == EIO);
	assert(usb_modem_wan_devnum_matches_status(2, 0, "17", 17, &matches) == EIO);
}

static void
test_devnum_match_rejects_null_output(void)
{
	assert(usb_modem_wan_devnum_matches_status(1, 0, "17", 17, NULL) == EINVAL);
}

static void
test_legacy_boolean_fails_closed_on_identity_read_error(void)
{
	assert(usb_modem_wan_legacy_boolean(0, 0) == 0);
	assert(usb_modem_wan_legacy_boolean(0, 1) == 1);
	assert(usb_modem_wan_legacy_boolean(EIO, 0) == 1);
	assert(usb_modem_wan_legacy_boolean(ENOSPC, 1) == 1);
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
	test_missing_modem_dev_retains_zero_default();
	test_present_modem_dev_parses_zero_and_nonzero();
	test_parser_preserves_legacy_atoi_numeric_prefix_semantics();
	test_backend_read_errors_are_not_treated_as_absent_key();
	test_integer_overflow_is_reported();
	test_null_output_is_rejected();
	test_devnum_match_requires_exact_selected_identity();
	test_devnum_match_preserves_absent_key_default();
	test_devnum_match_propagates_backend_errors();
	test_devnum_match_rejects_null_output();
	test_legacy_boolean_fails_closed_on_identity_read_error();
	puts("USB modem WAN checked identity read policy regression: PASS");
	return 0;
}
