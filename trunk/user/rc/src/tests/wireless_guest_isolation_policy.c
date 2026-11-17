#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wireless_guest_isolation_policy.h"

static void
test_empty_and_success_status(void)
{
	wireless_guest_isolation_status_t status;

	wireless_guest_isolation_status_reset(&status);
	assert(wireless_guest_isolation_status_result(&status) == 0);
	wireless_guest_isolation_status_record(&status, "bridge port", 0);
	assert(status.failure_count == 0);
	assert(status.first_operation == NULL);
}

static void
test_first_failure_and_independent_later_operations(void)
{
	wireless_guest_isolation_status_t status;

	wireless_guest_isolation_status_reset(&status);
	wireless_guest_isolation_status_record(&status, "5GHz bridge isolation", -1);
	wireless_guest_isolation_status_record(&status, "2.4GHz bridge isolation", 0);
	wireless_guest_isolation_status_record(&status, "ebtables flush", 256);
	assert(wireless_guest_isolation_status_result(&status) == -EIO);
	assert(status.first_status == -1);
	assert(status.failure_count == 2);
	assert(strcmp(status.first_operation, "5GHz bridge isolation") == 0);
}

static void
test_positive_system_wait_status_is_a_failure(void)
{
	wireless_guest_isolation_status_t status;

	wireless_guest_isolation_status_reset(&status);
	wireless_guest_isolation_status_record(&status, "ebtables rule", 256);
	assert(wireless_guest_isolation_status_result(&status) == -EIO);
	assert(status.first_status == 256);
	assert(status.failure_count == 1);
	assert(strcmp(status.first_operation, "ebtables rule") == 0);
}

static void
test_later_failures_do_not_replace_first_error(void)
{
	wireless_guest_isolation_status_t status;

	wireless_guest_isolation_status_reset(&status);
	wireless_guest_isolation_status_record(&status, "module load", ENODEV);
	wireless_guest_isolation_status_record(&status, "flush", EIO);
	wireless_guest_isolation_status_record(&status, "rule install", -1);
	assert(status.first_status == ENODEV);
	assert(status.failure_count == 3);
	assert(strcmp(status.first_operation, "module load") == 0);
}

static void
test_control_result_preserves_changed_boolean_only_on_success(void)
{
	wireless_guest_isolation_status_t status;

	wireless_guest_isolation_status_reset(&status);
	assert(wireless_guest_isolation_control_result(0, &status) == 0);
	assert(wireless_guest_isolation_control_result(1, &status) == 1);

	wireless_guest_isolation_status_record(&status, "ebtables rules", 256);
	assert(wireless_guest_isolation_control_result(0, &status) == -EIO);
	assert(wireless_guest_isolation_control_result(1, &status) == -EIO);
}

static void
test_null_status_is_fail_safe(void)
{
	assert(wireless_guest_isolation_status_result(NULL) == -EIO);
	assert(wireless_guest_isolation_control_result(1, NULL) == -EIO);
}

int
main(void)
{
	test_empty_and_success_status();
	test_first_failure_and_independent_later_operations();
	test_positive_system_wait_status_is_a_failure();
	test_later_failures_do_not_replace_first_error();
	test_control_result_preserves_changed_boolean_only_on_success();
	test_null_status_is_fail_safe();
	puts("Wireless guest LAN isolation status aggregation: PASS");
	return 0;
}
