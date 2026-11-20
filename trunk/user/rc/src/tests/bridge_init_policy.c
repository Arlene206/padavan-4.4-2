#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "bridge_init_policy.h"

static void
test_empty_and_success_status(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	assert(rc_bridge_init_result(&status) == 0);
	rc_bridge_init_record(&status, "base", 0);
	assert(status.failure_count == 0);
	assert(status.first_operation == NULL);
}

static void
test_first_failure_and_later_operations(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "VLAN rule", EIO);
	rc_bridge_init_record(&status, "VLAN rule", 0);
	rc_bridge_init_record(&status, "switch bridge mode", ENODEV);
	rc_bridge_init_record(&status, "bridge IFUP", -1);
	assert(rc_bridge_init_result(&status) == EIO);
	assert(status.failure_count == 3);
	assert(strcmp(status.first_operation, "VLAN rule") == 0);
}

static void
test_negative_first_status_is_preserved(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "MAC address", -1);
	rc_bridge_init_record(&status, "bridge", EIO);
	assert(rc_bridge_init_result(&status) == -1);
	assert(status.failure_count == 2);
}

static void
test_radio_setup_failures_are_recorded_without_stopping_later_operations(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "bring up 2.4GHz main interface", ENODEV);
	rc_bridge_init_record(&status, "bring up 5GHz main interface", EIO);
	rc_bridge_init_record(&status, "restore 5GHz main interface down", 0);
	assert(rc_bridge_init_result(&status) == ENODEV);
	assert(status.failure_count == 2);
	assert(strcmp(status.first_operation, "bring up 2.4GHz main interface") == 0);
}

static void
test_wireless_config_generation_failure_reaches_bridge_status(void)
{
	rc_bridge_init_status_t status;
	int operation_status;

	rc_bridge_init_status_reset(&status);
	operation_status = -1; /* gen_ralink_config_* failed to publish its file. */
	rc_bridge_init_record(&status,
		"prepare 2.4GHz wireless configuration", operation_status);

	/* init_bridge continues independent bridge setup after this failure. */
	operation_status = 0;
	rc_bridge_init_record(&status, "bring up LAN bridge", operation_status);
	assert(rc_bridge_init_result(&status) == -1);
	assert(status.failure_count == 1);
	assert(strcmp(status.first_operation,
		      "prepare 2.4GHz wireless configuration") == 0);
}

static void
test_bridge_add_idempotency_requires_confirmed_bridge(void)
{
	assert(rc_bridge_init_add_bridge_result(0, 0, -1) == 0);
	/* Existing bridge with no members or at least one member is valid. */
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, 1) == 0);
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, 0) == 0);
	/* A failed/unknown bridge inventory must not hide a failed add. */
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, -1) == -1);
	assert(rc_bridge_init_add_bridge_result(-1, EACCES, 1) == -1);
	assert(rc_bridge_init_add_bridge_result(EINVAL, EEXIST, 0) == EINVAL);
}

int
main(void)
{
	test_empty_and_success_status();
	test_first_failure_and_later_operations();
	test_negative_first_status_is_preserved();
	test_radio_setup_failures_are_recorded_without_stopping_later_operations();
	test_wireless_config_generation_failure_reaches_bridge_status();
	test_bridge_add_idempotency_requires_confirmed_bridge();
	puts("RC bridge initialization status and idempotency policy: PASS");
	return 0;
}
