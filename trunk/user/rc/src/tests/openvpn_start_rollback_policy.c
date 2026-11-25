#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "openvpn_start_rollback_policy.h"

struct fake_inventory {
	enum process_presence states[2];
	int errors[2];
	int scans;
	int waits;
	int wait_status;
	int wait_errno;
};

static enum process_presence
fake_scan(void *context, int *scan_error)
{
	struct fake_inventory *fake = (struct fake_inventory *)context;
	int index = fake->scans++;

	if (index > 1) {
		if (scan_error)
			*scan_error = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (scan_error)
		*scan_error = fake->errors[index];
	return fake->states[index];
}

static int
fake_wait(void *context, unsigned int seconds)
{
	struct fake_inventory *fake = (struct fake_inventory *)context;

	fake->waits++;
	if (seconds != OPENVPN_START_ROLLBACK_CONFIRM_WAIT_SECONDS) {
		errno = EINVAL;
		return -1;
	}
	if (fake->wait_status != 0) {
		errno = fake->wait_errno;
		return fake->wait_status;
	}
	return 0;
}

static void
test_two_absent_scans_are_required(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_ABSENT},
		{0, 0}, 0, 0, 0, 0
	};
	int scan_error = -1;

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, &scan_error) ==
		PROCESS_PRESENCE_ABSENT);
	assert(fake.scans == 2);
	assert(fake.waits == 1);
	assert(scan_error == 0);
}

static void
test_running_process_preserves_resources(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_ABSENT},
		{0, 0}, 0, 0, 0, 0
	};

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, NULL) ==
		PROCESS_PRESENCE_RUNNING);
	assert(fake.scans == 1);
	assert(fake.waits == 0);
}

static void
test_second_running_scan_revokes_absence_confirmation(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_RUNNING},
		{0, 0}, 0, 0, 0, 0
	};

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, NULL) ==
		PROCESS_PRESENCE_RUNNING);
	assert(fake.scans == 2);
	assert(fake.waits == 1);
}

static void
test_unknown_inventory_propagates_error(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_UNKNOWN, PROCESS_PRESENCE_ABSENT},
		{EACCES, 0}, 0, 0, 0, 0
	};
	int scan_error = 0;

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, &scan_error) ==
		PROCESS_PRESENCE_UNKNOWN);
	assert(fake.scans == 1);
	assert(fake.waits == 0);
	assert(scan_error == EACCES);
	assert(errno == EACCES);
}

static void
test_invalid_inventory_state_fails_closed(void)
{
	struct fake_inventory fake = {
		{(enum process_presence)99, PROCESS_PRESENCE_ABSENT},
		{0, 0}, 0, 0, 0, 0
	};
	int scan_error = 0;

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, &scan_error) ==
		PROCESS_PRESENCE_UNKNOWN);
	assert(scan_error == EIO);
	assert(errno == EIO);
}

static void
test_interrupted_wait_fails_closed(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_ABSENT},
		{0, 0}, 0, 0, -1, EINTR
	};
	int scan_error = 0;

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		fake_scan, fake_wait, &fake, &scan_error) ==
		PROCESS_PRESENCE_UNKNOWN);
	assert(fake.scans == 1);
	assert(fake.waits == 1);
	assert(scan_error == EINTR);
	assert(errno == EINTR);
}

static void
test_missing_operations_fail_closed(void)
{
	struct fake_inventory fake = {
		{PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_ABSENT},
		{0, 0}, 0, 0, 0, 0
	};
	int scan_error = 0;

	assert(openvpn_start_rollback_confirm_absent_with_ops(
		NULL, fake_wait, &fake, &scan_error) ==
		PROCESS_PRESENCE_UNKNOWN);
	assert(scan_error == EINVAL);
	assert(errno == EINVAL);
}

static void
test_cleanup_requires_absence_and_ownership(void)
{
	assert(openvpn_start_rollback_cleanup_owned(
		PROCESS_PRESENCE_ABSENT, 1));
	assert(!openvpn_start_rollback_cleanup_owned(
		PROCESS_PRESENCE_ABSENT, 0));
	assert(!openvpn_start_rollback_cleanup_owned(
		PROCESS_PRESENCE_RUNNING, 1));
	assert(!openvpn_start_rollback_cleanup_owned(
		PROCESS_PRESENCE_UNKNOWN, 1));
	assert(!openvpn_start_rollback_cleanup_owned(
		(enum process_presence)99, 1));
}

int
main(void)
{
	test_two_absent_scans_are_required();
	test_running_process_preserves_resources();
	test_second_running_scan_revokes_absence_confirmation();
	test_unknown_inventory_propagates_error();
	test_invalid_inventory_state_fails_closed();
	test_interrupted_wait_fails_closed();
	test_missing_operations_fail_closed();
	test_cleanup_requires_absence_and_ownership();
	puts("OpenVPN startup rollback inventory and ownership regression: PASS");
	return 0;
}
