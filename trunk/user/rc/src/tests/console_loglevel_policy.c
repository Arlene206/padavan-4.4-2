#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "console_loglevel_policy.h"

static void
test_present_console_loglevel_keeps_legacy_integer_conversion(void)
{
	int level = -1;

	assert(rc_console_loglevel_parse_status(1, 0, "5", &level) == 0);
	assert(level == 5);
	assert(rc_console_loglevel_parse_status(1, 0, "not-a-number", &level) == 0);
	assert(level == 0);
}

static void
test_missing_console_loglevel_keeps_legacy_zero_default(void)
{
	int level = -1;

	assert(rc_console_loglevel_parse_status(0, EACCES, NULL, &level) == 0);
	assert(level == 0);
}

static void
test_backend_errors_do_not_publish_a_new_level(void)
{
	int level = 91;

	assert(rc_console_loglevel_parse_status(-1, EACCES, NULL, &level) == EACCES);
	assert(level == 91);
	assert(rc_console_loglevel_parse_status(-1, 0, NULL, &level) == EIO);
	assert(level == 91);
}

static void
test_invalid_results_and_arguments_fail_closed(void)
{
	int level = 91;

	assert(rc_console_loglevel_parse_status(2, 0, "7", &level) == EIO);
	assert(level == 91);
	assert(rc_console_loglevel_parse_status(1, 0, NULL, &level) == EIO);
	assert(level == 91);
	assert(rc_console_loglevel_parse_status(0, 0, NULL, NULL) == EINVAL);
}

int
main(void)
{
	test_present_console_loglevel_keeps_legacy_integer_conversion();
	test_missing_console_loglevel_keeps_legacy_zero_default();
	test_backend_errors_do_not_publish_a_new_level();
	test_invalid_results_and_arguments_fail_closed();
	puts("RC console loglevel checked-read policy: PASS");
	return 0;
}
