#include <assert.h>
#include <errno.h>

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

int
main(void)
{
	test_present_values_match_exactly();
	test_missing_key_uses_explicit_legacy_default();
	test_backend_errors_preserve_status_and_output();
	test_invalid_api_results_fail_closed();
	test_invalid_arguments_are_rejected();
	return 0;
}
