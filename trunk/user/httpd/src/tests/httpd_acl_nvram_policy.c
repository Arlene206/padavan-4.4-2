#include <assert.h>
#include <errno.h>

#include "httpd_acl_nvram_policy.h"

static void
test_missing_key_preserves_disabled_acl_default(void)
{
	int mode = -1, valid = -1;

	assert(httpd_acl_nvram_mode_status(0, EACCES, NULL, &mode, &valid) == 0);
	assert(mode == 0 && valid == 1);
}

static void
test_present_acl_modes_keep_decimal_and_whitespace_compatibility(void)
{
	int mode = -1, valid = -1;

	assert(httpd_acl_nvram_mode_status(1, 0, "0", &mode, &valid) == 0);
	assert(mode == 0 && valid == 1);
	assert(httpd_acl_nvram_mode_status(1, 0, "1", &mode, &valid) == 0);
	assert(mode == 1 && valid == 1);
	assert(httpd_acl_nvram_mode_status(1, 0, "2", &mode, &valid) == 0);
	assert(mode == 2 && valid == 1);
	assert(httpd_acl_nvram_mode_status(1, 0, " +2 \\t", &mode, &valid) == 0);
	assert(mode == 2 && valid == 1);
}

static void
test_malformed_or_negative_acl_values_fail_closed(void)
{
	int mode = -1, valid = 1;

	assert(httpd_acl_nvram_mode_status(1, 0, "invalid", &mode, &valid) != 0);
	assert(mode == 0 && valid == 0);
	assert(httpd_acl_client_allowed(valid, 0) == 0);
	assert(httpd_acl_client_allowed(valid, 1) == 1);

	valid = 1;
	assert(httpd_acl_nvram_mode_status(1, 0, "1junk", &mode, &valid) != 0);
	assert(mode == 0 && valid == 0);
	valid = 1;
	assert(httpd_acl_nvram_mode_status(1, 0, "-1", &mode, &valid) != 0);
	assert(mode == 0 && valid == 0);
	valid = 1;
	assert(httpd_acl_nvram_mode_status(1, 0, "", &mode, &valid) != 0);
	assert(mode == 0 && valid == 0);
	valid = 1;
	assert(httpd_acl_nvram_mode_status(1, 0, "999999999999999999999999999999999", &mode, &valid) != 0);
	assert(mode == 0 && valid == 0);
}

static void
test_backend_failure_does_not_disable_acl(void)
{
	int mode = 0, valid = 1;

	assert(httpd_acl_nvram_mode_status(-1, EACCES, NULL, &mode, &valid) == EACCES);
	assert(mode == 0 && valid == 0);
	assert(httpd_acl_client_allowed(valid, 0) == 0);
	assert(httpd_acl_client_allowed(valid, 1) == 1);
}

static void
test_failure_without_errno_and_unexpected_result_fail_closed(void)
{
	int mode = 0, valid = 1;

	assert(httpd_acl_nvram_mode_status(-1, 0, NULL, &mode, &valid) == EIO);
	assert(mode == 0 && valid == 0);
	valid = 1;
	assert(httpd_acl_nvram_mode_status(2, 0, "2", &mode, &valid) == EIO);
	assert(mode == 0 && valid == 0);
	valid = 1;
	assert(httpd_acl_nvram_mode_status(1, 0, NULL, &mode, &valid) == EIO);
	assert(mode == 0 && valid == 0);
}

static void
test_invalid_output_pointers_rejected(void)
{
	int mode = 0, valid = 0;

	assert(httpd_acl_nvram_mode_status(0, 0, NULL, NULL, &valid) == EINVAL);
	assert(httpd_acl_nvram_mode_status(0, 0, NULL, &mode, NULL) == EINVAL);
}

int
main(void)
{
	test_missing_key_preserves_disabled_acl_default();
	test_present_acl_modes_keep_decimal_and_whitespace_compatibility();
	test_malformed_or_negative_acl_values_fail_closed();
	test_backend_failure_does_not_disable_acl();
	test_failure_without_errno_and_unexpected_result_fail_closed();
	test_invalid_output_pointers_rejected();
	return 0;
}
