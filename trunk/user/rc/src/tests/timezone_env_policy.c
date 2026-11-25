#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "timezone_env_policy.h"

static void
test_present_value_is_formatted_exactly(void)
{
	char destination[64] = "unchanged";

	assert(rc_timezone_env_format_status(
		1, 0, "America/Los_Angeles", destination,
		sizeof(destination)) == 0);
	assert(strcmp(destination, "TZ=America/Los_Angeles") == 0);
}

static void
test_missing_key_preserves_legacy_empty_timezone(void)
{
	char destination[16] = "unchanged";

	assert(rc_timezone_env_format_status(
		0, EACCES, NULL, destination, sizeof(destination)) == 0);
	assert(strcmp(destination, "TZ=") == 0);
}

static void
test_backend_error_leaves_destination_untouched(void)
{
	char destination[32] = "keep-existing-value";

	assert(rc_timezone_env_format_status(
		-1, EACCES, NULL, destination, sizeof(destination)) == EACCES);
	assert(strcmp(destination, "keep-existing-value") == 0);
	assert(rc_timezone_env_format_status(
		-1, 0, NULL, destination, sizeof(destination)) == EIO);
	assert(strcmp(destination, "keep-existing-value") == 0);
}

static void
test_invalid_read_results_and_present_null_are_rejected(void)
{
	char destination[32] = "keep-existing-value";

	assert(rc_timezone_env_format_status(
		2, 0, "UTC", destination, sizeof(destination)) == EIO);
	assert(strcmp(destination, "keep-existing-value") == 0);
	assert(rc_timezone_env_format_status(
		1, 0, NULL, destination, sizeof(destination)) == EIO);
	assert(strcmp(destination, "keep-existing-value") == 0);
	assert(rc_timezone_env_format_status(
		0, 0, NULL, NULL, sizeof(destination)) == EINVAL);
}

static void
test_timezone_length_boundary_is_checked_without_truncation(void)
{
	char destination[8] = "retain";
	char exact_fit[8] = "before";

	assert(rc_timezone_env_format_status(
		1, 0, "1234", exact_fit, sizeof(exact_fit)) == 0);
	assert(strcmp(exact_fit, "TZ=1234") == 0);

	assert(rc_timezone_env_format_status(
		1, 0, "12345", destination, sizeof(destination)) == ENAMETOOLONG);
	assert(strcmp(destination, "retain") == 0);
	assert(rc_timezone_env_format_status(
		1, 0, "UTC", destination, 3U) == EINVAL);
	assert(strcmp(destination, "retain") == 0);
}

static void
test_time_conversion_failures_have_deterministic_status(void)
{
	assert(rc_timezone_conversion_status(1, ERANGE) == 0);
	assert(rc_timezone_conversion_status(0, ERANGE) == ERANGE);
	assert(rc_timezone_conversion_status(0, 0) == EOVERFLOW);
}

static void
test_cache_validity_does_not_collide_with_negative_one_offset(void)
{
	assert(rc_timezone_kernel_cache_needs_update(0, 0, -1));
	assert(!rc_timezone_kernel_cache_needs_update(1, -1, -1));
	assert(rc_timezone_kernel_cache_needs_update(1, -1, 0));
}

static void
test_kernel_timezone_cache_changes_only_after_success(void)
{
	int cached_minuteswest = 60;

	assert(rc_timezone_kernel_cache_status(
		-1, EPERM, -300, &cached_minuteswest) == EPERM);
	assert(cached_minuteswest == 60);
	assert(rc_timezone_kernel_cache_status(
		-1, 0, -300, &cached_minuteswest) == EIO);
	assert(cached_minuteswest == 60);
	assert(rc_timezone_kernel_cache_status(
		0, EACCES, -300, &cached_minuteswest) == 0);
	assert(cached_minuteswest == -300);
	assert(rc_timezone_kernel_cache_status(
		0, 0, 120, NULL) == EINVAL);
}

int
main(void)
{
	test_present_value_is_formatted_exactly();
	test_missing_key_preserves_legacy_empty_timezone();
	test_backend_error_leaves_destination_untouched();
	test_invalid_read_results_and_present_null_are_rejected();
	test_timezone_length_boundary_is_checked_without_truncation();
	test_time_conversion_failures_have_deterministic_status();
	test_cache_validity_does_not_collide_with_negative_one_offset();
	test_kernel_timezone_cache_changes_only_after_success();
	puts("RC timezone environment checked-read policy: PASS");
	return 0;
}
