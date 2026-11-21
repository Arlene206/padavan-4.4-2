#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "firewall_nvram_match_policy.h"

static void
test_present_values_match_exactly(void)
{
	int matches = -1;
	assert(firewall_nvram_match_status(1, 0, "1", "1", 0, &matches) == 0);
	assert(matches == 1);
	assert(firewall_nvram_match_status(1, 0, "0", "1", 0, &matches) == 0);
	assert(matches == 0);
}

static void
test_missing_key_uses_explicit_legacy_default(void)
{
	int matches = -1;
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 1, &matches) == 0);
	assert(matches == 1);
}

static void
test_backend_errors_preserve_status_and_output(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0, &matches) == EACCES);
	assert(matches == 23);
	assert(firewall_nvram_match_status(-1, 0, NULL, "1", 0, &matches) == EIO);
	assert(matches == 23);
}

static void
test_invalid_api_results_fail_closed(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(2, 0, "1", "1", 0, &matches) == EIO);
	assert(matches == 23);
	assert(firewall_nvram_match_status(1, 0, NULL, "1", 0, &matches) == EIO);
	assert(matches == 23);
	assert(firewall_nvram_match_status(0, 0, "unexpected", "1", 0, &matches) == EIO);
	assert(matches == 23);
}

static void
test_invalid_arguments_are_rejected(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(1, 0, "1", NULL, 0, &matches) == EINVAL);
	assert(firewall_nvram_match_status(1, 0, "1", "1", 0, NULL) == EINVAL);
	assert(matches == 23);
}

static void
test_integer_modes_preserve_decimal_semantics(void)
{
	int number = -9;
	assert(firewall_nvram_int_status(1, 0, "0", 0, &number) == 0);
	assert(number == 0);
	assert(firewall_nvram_int_status(1, 0, "1", 0, &number) == 0);
	assert(number == 1);
	assert(firewall_nvram_int_status(1, 0, "2", 0, &number) == 0);
	assert(number == 2);
	assert(firewall_nvram_int_status(1, 0, "-1", 0, &number) == 0);
	assert(number == -1);
	assert(firewall_nvram_int_status(1, 0, "2147483647", 0, &number) == 0);
	assert(number == INT_MAX);
	assert(firewall_nvram_int_status(0, 0, NULL, 0, &number) == 0);
	assert(number == 0);
}

static void
test_integer_errors_are_fail_closed_and_preserve_output(void)
{
	int number = 41;
	assert(firewall_nvram_int_status(-1, EACCES, NULL, 0, &number) == EACCES);
	assert(number == 41);
	assert(firewall_nvram_int_status(-1, 0, NULL, 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(2, 0, "1", 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, NULL, 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "", 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "2junk", 0, &number) == EINVAL);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "2147483648", 0, &number) == ERANGE);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "1", 0, NULL) == EINVAL);
	assert(number == 41);
}

int
main(void)
{
	test_present_values_match_exactly();
	test_missing_key_uses_explicit_legacy_default();
	test_backend_errors_preserve_status_and_output();
	test_invalid_api_results_fail_closed();
	test_invalid_arguments_are_rejected();
	test_integer_modes_preserve_decimal_semantics();
	test_integer_errors_are_fail_closed_and_preserve_output();
	return 0;
}
