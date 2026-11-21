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
test_invmatch_preserves_nvram_invmatch_semantics(void)
{
	int matches = -1;

	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", "1", 0,
					     &matches) == 0);
	assert(matches == 0);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "0", "1", 0,
					     &matches) == 0);
	assert(matches == 1);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "static", "1", 0,
					     &matches) == 0);
	assert(matches == 1);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "", "1", 0,
					     &matches) == 0);
	assert(matches == 1);

	/* nvram_invmatch() is false for a missing key (NULL nvram_get value). */
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 0,
					     &matches) == 0);
	assert(matches == 0);
	/* The policy helper keeps the missing-key default explicit for other callers. */
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 1,
					     &matches) == 0);
	assert(matches == 1);
}

static void
test_invmatch_backend_errors_preserve_output(void)
{
	int matches = 17;

	assert(dnsmasq_nvram_invmatch_status(-1, EACCES, NULL, "1", 0,
					     &matches) == EACCES);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(-1, 0, NULL, "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(2, 0, "1", "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(0, 0, "unexpected", "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(1, 0, NULL, "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
}

static void
test_invmatch_invalid_arguments_are_rejected(void)
{
	int matches = 17;

	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", NULL, 0,
					     &matches) == EINVAL);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", "1", 0,
					     NULL) == EINVAL);
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 2,
					     &matches) == EINVAL);
	assert(matches == 17);
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
test_checked_string_option_and_output_preservation(void)
{
	char value[16] = "unchanged";

	assert(dnsmasq_nvram_string_status(1, 0, "192.0.2.10", "",
					   value, sizeof(value)) == 0);
	assert(strcmp(value, "192.0.2.10") == 0);

	assert(dnsmasq_nvram_string_status(0, 0, NULL, "",
					   value, sizeof(value)) == 0);
	assert(value[0] == '\0');

	strcpy(value, "unchanged");
	assert(dnsmasq_nvram_string_status(-1, EACCES, NULL, "",
					   value, sizeof(value)) == EACCES);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(-1, 0, NULL, "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(2, 0, "x", "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(0, 0, "unexpected", "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, NULL, "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, "1234567890123456", "",
					   value, sizeof(value)) == ENAMETOOLONG);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, "1", "", NULL, 1) == EINVAL);
	assert(dnsmasq_nvram_string_status(1, 0, "1", "", value, 0) == EINVAL);
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
	test_invmatch_preserves_nvram_invmatch_semantics();
	test_invmatch_backend_errors_preserve_output();
	test_invmatch_invalid_arguments_are_rejected();
	test_integer_boolean_preserves_nonzero_nvram_get_int_semantics();
	test_checked_string_option_and_output_preservation();
	test_missing_keys_use_explicit_legacy_defaults();
	test_backend_errors_preserve_errno_and_output();
	test_inconsistent_results_fail_closed();
	test_invalid_arguments_are_rejected();
	return 0;
}
