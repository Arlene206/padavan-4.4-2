#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "detached_daemon_start_policy.h"

struct fixture {
	enum process_presence states[8];
	int errors[8];
	unsigned int state_count;
	unsigned int state_index;
	unsigned int waits[16];
	unsigned int wait_count;
	int wait_error;
	int stop_status;
	unsigned int stop_count;
};

static enum process_presence
scan_process(void *context, int *scan_error)
{
	struct fixture *fixture = (struct fixture *)context;
	unsigned int index = fixture->state_index;

	assert(index < fixture->state_count);
	fixture->state_index++;
	if (scan_error)
		*scan_error = fixture->errors[index];
	return fixture->states[index];
}

static int
wait_seconds(void *context, unsigned int seconds)
{
	struct fixture *fixture = (struct fixture *)context;

	assert(fixture->wait_count < sizeof(fixture->waits) /
	       sizeof(fixture->waits[0]));
	fixture->waits[fixture->wait_count++] = seconds;
	if (fixture->wait_error != 0) {
		errno = fixture->wait_error;
		return -1;
	}
	return 0;
}

static int
stop_process(void *context)
{
	struct fixture *fixture = (struct fixture *)context;

	fixture->stop_count++;
	return fixture->stop_status;
}

static void
test_failed_start_rolls_back_and_preserves_primary_error(void)
{
	struct fixture fixture;
	int stop_status = -1;
	int scan_error = -1;
	enum process_presence final_state = PROCESS_PRESENCE_UNKNOWN;

	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_ABSENT;
	fixture.state_count = 1;
	errno = 0;
	assert(detached_daemon_start_rollback_with_ops(
		ETIMEDOUT, ETIMEDOUT, stop_process, scan_process, &fixture,
		&stop_status, &final_state, &scan_error) == ETIMEDOUT);
	assert(errno == ETIMEDOUT);
	assert(fixture.stop_count == 1 && fixture.state_index == 1);
	assert(stop_status == 0);
	assert(final_state == PROCESS_PRESENCE_ABSENT);
	assert(scan_error == 0);
}

static void
test_running_child_after_failed_cleanup_keeps_primary_status(void)
{
	struct fixture fixture;
	int stop_status = 0;
	int scan_error = 0;
	enum process_presence final_state = PROCESS_PRESENCE_ABSENT;

	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_RUNNING;
	fixture.state_count = 1;
	fixture.stop_status = EBUSY;
	errno = 0;
	assert(detached_daemon_start_rollback_with_ops(
		ETIMEDOUT, ETIMEDOUT, stop_process, scan_process, &fixture,
		&stop_status, &final_state, &scan_error) == ETIMEDOUT);
	assert(errno == ETIMEDOUT);
	assert(fixture.stop_count == 1 && fixture.state_index == 1);
	assert(stop_status == EBUSY);
	assert(final_state == PROCESS_PRESENCE_RUNNING);
}

static void
test_unknown_rollback_inventory_preserves_primary_status(void)
{
	struct fixture fixture;
	int stop_status = 0;
	int scan_error = 0;
	enum process_presence final_state = PROCESS_PRESENCE_ABSENT;

	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_UNKNOWN;
	fixture.errors[0] = EACCES;
	fixture.state_count = 1;
	fixture.stop_status = EIO;
	errno = 0;
	assert(detached_daemon_start_rollback_with_ops(
		ETIMEDOUT, ETIMEDOUT, stop_process, scan_process, &fixture,
		&stop_status, &final_state, &scan_error) == ETIMEDOUT);
	assert(errno == ETIMEDOUT);
	assert(fixture.stop_count == 1 && fixture.state_index == 1);
	assert(stop_status == EIO);
	assert(final_state == PROCESS_PRESENCE_UNKNOWN);
	assert(scan_error == EACCES);
}

static void
test_successful_start_skips_rollback(void)
{
	struct fixture fixture;
	int stop_status = -1;
	int scan_error = -1;
	enum process_presence final_state = PROCESS_PRESENCE_RUNNING;

	memset(&fixture, 0, sizeof(fixture));
	assert(detached_daemon_start_rollback_with_ops(
		0, 0, stop_process, scan_process, &fixture,
		&stop_status, &final_state, &scan_error) == 0);
	assert(fixture.stop_count == 0 && fixture.state_index == 0);
	assert(stop_status == 0);
	assert(final_state == PROCESS_PRESENCE_UNKNOWN);
	assert(scan_error == 0);
}

static void
test_invalid_rollback_callbacks_preserve_primary_status(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	errno = 0;
	assert(detached_daemon_start_rollback_with_ops(
		ETIMEDOUT, ETIMEDOUT, NULL, scan_process, &fixture,
		NULL, NULL, NULL) == ETIMEDOUT);
	assert(errno == EINVAL);
	assert(fixture.state_index == 0 && fixture.stop_count == 0);
}

static void
test_launcher_failure_is_preserved_without_probing(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	assert(detached_daemon_start_postcondition_with_ops(
		17, scan_process, wait_seconds, &fixture, 1, 1, 4) == 17);
	assert(fixture.state_index == 0);
	assert(fixture.wait_count == 0);
}

static void
test_running_process_is_required_after_launcher(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_RUNNING;
	fixture.state_count = 1;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 1, 1, 4) == 0);
	assert(fixture.state_index == 1);
	assert(fixture.wait_count == 1);
	assert(fixture.waits[0] == 1);
}

static void
test_delayed_daemon_appearance_is_accepted(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_ABSENT;
	fixture.states[1] = PROCESS_PRESENCE_ABSENT;
	fixture.states[2] = PROCESS_PRESENCE_RUNNING;
	fixture.state_count = 3;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 1, 1, 4) == 0);
	assert(fixture.state_index == 3);
	assert(fixture.wait_count == 3);
}

static void
test_absent_child_times_out(void)
{
	struct fixture fixture;
	unsigned int i;
	memset(&fixture, 0, sizeof(fixture));
	for (i = 0; i < 4; i++)
		fixture.states[i] = PROCESS_PRESENCE_ABSENT;
	fixture.state_count = 4;
	errno = 0;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 1, 1, 4) == ETIMEDOUT);
	assert(errno == ETIMEDOUT);
	assert(fixture.state_index == 4);
	assert(fixture.wait_count == 4);
}

static void
test_unknown_inventory_fails_closed(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_UNKNOWN;
	fixture.errors[0] = EACCES;
	fixture.state_count = 1;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 1, 1, 4) == EACCES);
	assert(errno == EACCES);
	assert(fixture.state_index == 1);
	assert(fixture.wait_count == 1);
}

static void
test_unknown_without_error_and_invalid_state_fail_closed(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = PROCESS_PRESENCE_UNKNOWN;
	fixture.state_count = 1;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 0, 0, 1) == EIO);
	assert(errno == EIO);

	memset(&fixture, 0, sizeof(fixture));
	fixture.states[0] = (enum process_presence)99;
	fixture.state_count = 1;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 0, 0, 1) == EIO);
	assert(errno == EIO);
}

static void
test_wait_interruption_and_invalid_arguments_fail(void)
{
	struct fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.wait_error = EINTR;
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 1, 1, 4) == EINTR);
	assert(fixture.state_index == 0);
	assert(fixture.wait_count == 1);
	assert(detached_daemon_start_postcondition_with_ops(
		0, NULL, wait_seconds, &fixture, 0, 0, 4) == EINVAL);
	assert(detached_daemon_start_postcondition_with_ops(
		0, scan_process, wait_seconds, &fixture, 0, 0, 0) == EINVAL);
}

int
main(void)
{
	test_launcher_failure_is_preserved_without_probing();
	test_failed_start_rolls_back_and_preserves_primary_error();
	test_running_child_after_failed_cleanup_keeps_primary_status();
	test_unknown_rollback_inventory_preserves_primary_status();
	test_successful_start_skips_rollback();
	test_invalid_rollback_callbacks_preserve_primary_status();
	test_running_process_is_required_after_launcher();
	test_delayed_daemon_appearance_is_accepted();
	test_absent_child_times_out();
	test_unknown_inventory_fails_closed();
	test_unknown_without_error_and_invalid_state_fail_closed();
	test_wait_interruption_and_invalid_arguments_fail();
	puts("Detached-daemon startup postcondition regression: PASS");
	return 0;
}
