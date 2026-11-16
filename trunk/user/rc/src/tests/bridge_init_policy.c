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
	test_bridge_add_idempotency_requires_confirmed_bridge();
	puts("RC bridge initialization status and idempotency policy: PASS");
	return 0;
}
