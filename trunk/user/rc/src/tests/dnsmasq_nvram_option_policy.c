#include <assert.h>
#include <errno.h>

#include "dnsmasq_nvram_option_policy.h"

static void
test_present_values_keep_nvram_match_semantics(void)
{
	int enabled = -1;

	assert(dnsmasq_nvram_option_status(1, 0, "1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_option_status(1, 0, "0", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(1, 0, "true", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(1, 0, "", 0, &enabled) == 0);
	assert(enabled == 0);
}

static void
test_integer_boolean_preserves_nonzero_nvram_get_int_semantics(void)
{
	int enabled = 9;

	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "2", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "-1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "0", 0, &enabled) == 0);
	assert(enabled == 0);

	enabled = 9;
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "1junk", 0, &enabled) == EINVAL);
	assert(enabled == 9);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "999999999999999999999999",
						    0, &enabled) == ERANGE);
	assert(enabled == 9);
	assert(dnsmasq_nvram_integer_boolean_status(0, 0, NULL, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_integer_boolean_status(-1, EACCES, NULL, 0, &enabled) == EACCES);
	assert(enabled == 0);
}

static void
test_missing_keys_use_explicit_legacy_defaults(void)
{
	int enabled = -1;

	assert(dnsmasq_nvram_option_status(0, 0, NULL, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(0, 0, NULL, 1, &enabled) == 0);
	assert(enabled == 1);
}

static void
test_backend_errors_preserve_errno_and_output(void)
{
	int enabled = 19;

	assert(dnsmasq_nvram_option_status(-1, EACCES, NULL, 0, &enabled) == EACCES);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(-1, 0, NULL, 0, &enabled) == EIO);
	assert(enabled == 19);
}

static void
test_inconsistent_results_fail_closed(void)
{
	int enabled = 19;

	assert(dnsmasq_nvram_option_status(2, 0, "1", 0, &enabled) == EIO);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(1, 0, NULL, 0, &enabled) == EIO);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(0, 0, "1", 0, &enabled) == EIO);
	assert(enabled == 19);
}

static void
test_invalid_arguments_are_rejected(void)
{
	assert(dnsmasq_nvram_option_status(1, 0, "1", 0, NULL) == EINVAL);
}

int
main(void)
{
	test_present_values_keep_nvram_match_semantics();
	test_integer_boolean_preserves_nonzero_nvram_get_int_semantics();
	test_missing_keys_use_explicit_legacy_defaults();
	test_backend_errors_preserve_errno_and_output();
	test_inconsistent_results_fail_closed();
	test_invalid_arguments_are_rejected();
	return 0;
}
