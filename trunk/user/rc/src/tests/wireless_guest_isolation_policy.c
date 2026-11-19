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

typedef struct {
	int bring_down_status;
	int detach_status;
	int verify_status;
	char order[4];
	unsigned int calls;
} rollback_mock_t;

static void
rollback_mock_record(rollback_mock_t *mock, char step)
{
	assert(mock->calls < sizeof(mock->order));
	mock->order[mock->calls++] = step;
}

static int
rollback_mock_bring_down(void *context)
{
	rollback_mock_t *mock = (rollback_mock_t *)context;

	rollback_mock_record(mock, 'D');
	return mock->bring_down_status;
}

static int
rollback_mock_detach(void *context)
{
	rollback_mock_t *mock = (rollback_mock_t *)context;

	rollback_mock_record(mock, 'B');
	return mock->detach_status;
}

static int
rollback_mock_verify_down(void *context)
{
	rollback_mock_t *mock = (rollback_mock_t *)context;

	rollback_mock_record(mock, 'V');
	return mock->verify_status;
}

static void
test_fail_closed_rollback_attempts_every_step(void)
{
	static const wireless_guest_isolation_rollback_ops_t ops = {
		rollback_mock_bring_down,
		rollback_mock_detach,
		rollback_mock_verify_down
	};
	wireless_guest_isolation_status_t status;
	rollback_mock_t mock = { 0, -EBUSY, -EIO, { 0 }, 0 };

	wireless_guest_isolation_status_reset(&status);
	wireless_guest_isolation_status_record(&status,
		"guest bridge membership", -EACCES);

	assert(wireless_guest_isolation_status_rollback(&status, &ops, &mock) == -EIO);
	assert(mock.calls == 3);
	assert(memcmp(mock.order, "DBV", 3) == 0);
	assert(status.first_status == -EACCES);
	assert(strcmp(status.first_operation, "guest bridge membership") == 0);
	assert(status.failure_count == 3);
}

static void
test_fail_closed_rollback_success_and_invalid_ops(void)
{
	static const wireless_guest_isolation_rollback_ops_t ops = {
		rollback_mock_bring_down,
		rollback_mock_detach,
		rollback_mock_verify_down
	};
	wireless_guest_isolation_status_t status;
	rollback_mock_t mock = { 0, 0, 0, { 0 }, 0 };

	wireless_guest_isolation_status_reset(&status);
	assert(wireless_guest_isolation_status_rollback(&status, &ops, &mock) == 0);
	assert(mock.calls == 3);
	assert(memcmp(mock.order, "DBV", 3) == 0);
	assert(status.failure_count == 0);

	wireless_guest_isolation_status_reset(&status);
	assert(wireless_guest_isolation_status_rollback(&status, NULL, NULL) == -EIO);
	assert(status.failure_count == 1);
	assert(strcmp(status.first_operation, "validate guest AP rollback operations") == 0);
}

static void
test_startup_rollback_tracks_only_interfaces_started_here(void)
{
	assert(wireless_guest_startup_rollback_candidate(0, 1));
	assert(!wireless_guest_startup_rollback_candidate(1, 1));
	assert(!wireless_guest_startup_rollback_candidate(0, 0));
	assert(!wireless_guest_startup_rollback_candidate(1, 0));
}

static void
test_interface_flag_read_errors_are_not_reported_as_down(void)
{
	int is_up = 7;

	/* Generic socket/ioctl errors are unknown, never a false DOWN result. */
	assert(wireless_guest_isolation_link_state(-1, EACCES, ENODEV,
						   0x1, &is_up) == -EIO);
	assert(is_up == 7);

	/* ENODEV is the expected absent optional VIF during boot. */
	assert(wireless_guest_isolation_link_state(-1, ENODEV, ENODEV,
						   0x1, &is_up) == 0);
	assert(is_up == 0);
	assert(wireless_guest_isolation_link_state(0, 0, ENODEV,
						   0x1, &is_up) == 0);
	assert(is_up == 0);
	assert(wireless_guest_isolation_link_state(0x1, 0, ENODEV,
						   0x1, &is_up) == 0);
	assert(is_up == 1);
	assert(wireless_guest_isolation_link_state(0, 0, ENODEV,
						   0x1, NULL) == -EINVAL);
}

static void
test_null_status_is_fail_safe(void)
{
	assert(wireless_guest_isolation_status_result(NULL) == -EIO);
	assert(wireless_guest_isolation_control_result(1, NULL) == -EIO);
}

static void
test_ebtables_enable_requires_confirmed_module_presence(void)
{
	assert(wireless_guest_isolation_module_presence_error(1) == 0);
	assert(wireless_guest_isolation_module_presence_error(0) == ENODEV);
	assert(wireless_guest_isolation_module_presence_error(-EACCES) == EACCES);
	assert(wireless_guest_isolation_module_presence_error(-EIO) == EIO);
}

static void
test_ebtables_disable_preserves_unknown_module_state(void)
{
	assert(wireless_guest_isolation_ebtables_disable_action(1) ==
	       WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_AND_UNLOAD);
	assert(wireless_guest_isolation_ebtables_disable_action(0) ==
	       WIRELESS_GUEST_EBTABLES_DISABLE_NONE);
	assert(wireless_guest_isolation_ebtables_disable_action(-EACCES) ==
	       WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY);
	assert(wireless_guest_isolation_ebtables_disable_action(-EIO) ==
	       WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY);
}

int
main(void)
{
	test_empty_and_success_status();
	test_ebtables_enable_requires_confirmed_module_presence();
	test_ebtables_disable_preserves_unknown_module_state();
	test_first_failure_and_independent_later_operations();
	test_positive_system_wait_status_is_a_failure();
	test_later_failures_do_not_replace_first_error();
	test_control_result_preserves_changed_boolean_only_on_success();
	test_fail_closed_rollback_attempts_every_step();
	test_fail_closed_rollback_success_and_invalid_ops();
	test_interface_flag_read_errors_are_not_reported_as_down();
	test_startup_rollback_tracks_only_interfaces_started_here();
	test_null_status_is_fail_safe();
	puts("Wireless guest LAN isolation status aggregation: PASS");
	return 0;
}
