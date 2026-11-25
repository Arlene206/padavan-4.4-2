#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "storage_deferred_start_policy.h"

static void
test_no_pending_event_does_not_start(void)
{
	int should_start = -1;
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 0,
		0, 0, 0, &should_start) == 0);
	assert(should_start == 0);
}

static void
test_each_pending_trigger_starts_services(void)
{
	int should_start = 0;
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 1,
		0, 0, 0, &should_start) == 0);
	assert(should_start == 1);
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 0,
		0, 1, 0, &should_start) == 0);
	assert(should_start == 1);
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 0,
		0, 0, 1, &should_start) == 0);
	assert(should_start == 1);
}

static void
test_unplug_in_progress_suppresses_startup(void)
{
	int should_start = -1;
	assert(storage_deferred_start_dispatch_status(0, 1, 0, 1,
		0, 1, 1, &should_start) == 0);
	assert(should_start == 0);
}

static void
test_each_marker_read_error_fails_closed(void)
{
	int should_start = -1;
	assert(storage_deferred_start_dispatch_status(EIO, 0, 0, 1,
		0, 1, 1, &should_start) == EIO);
	assert(should_start == 0);
	assert(storage_deferred_start_dispatch_status(0, 0, EACCES, 1,
		0, 1, 1, &should_start) == EACCES);
	assert(should_start == 0);
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 1,
		EIO, 1, 1, &should_start) == EIO);
	assert(should_start == 0);
	assert(storage_deferred_start_dispatch_status(-1, 0, 0, 1,
		0, 1, 1, &should_start) == EIO);
	assert(should_start == 0);
}

static void
test_deferred_ack_failure_does_not_repeat_completed_cleanup(void)
{
	int action_done = 0;

	assert(storage_deferred_event_action_should_run(1, action_done));
	assert(storage_deferred_event_ack_status(1, 0, EACCES,
		&action_done) == EACCES);
	assert(action_done == 1);
	assert(!storage_deferred_event_action_should_run(1, action_done));
	assert(storage_deferred_event_ack_status(1, 0, 0,
		&action_done) == 0);
	assert(action_done == 0);
	assert(storage_deferred_event_action_should_run(1, action_done));
}

static void
test_deferred_cleanup_failure_remains_retryable(void)
{
	int action_done = 0;

	assert(storage_deferred_event_action_should_run(1, action_done));
	assert(storage_deferred_event_ack_status(1, EIO, 0,
		&action_done) == EIO);
	assert(action_done == 0);
	assert(storage_deferred_event_action_should_run(1, action_done));
	assert(storage_deferred_event_ack_status(1, -1, 0,
		&action_done) == EIO);
	assert(action_done == 0);
}

static void
test_deferred_ack_absent_event_releases_latch(void)
{
	int action_done = 1;

	assert(storage_deferred_event_ack_status(0, 0, EIO,
		&action_done) == 0);
	assert(action_done == 0);
	assert(storage_deferred_event_ack_status(1, 0, -1,
		&action_done) == EIO);
	assert(action_done == 1);
	assert(storage_deferred_event_ack_status(1, 0, 0,
		&action_done) == 0);
	assert(action_done == 0);
	assert(storage_deferred_event_ack_status(1, 0, 0, NULL) == EINVAL);
}

static void
test_invalid_output_is_rejected(void)
{
	assert(storage_deferred_start_dispatch_status(0, 0, 0, 0,
		0, 0, 0, NULL) == EINVAL);
}

int
main(void)
{
	test_no_pending_event_does_not_start();
	test_each_pending_trigger_starts_services();
	test_unplug_in_progress_suppresses_startup();
	test_each_marker_read_error_fails_closed();
	test_deferred_ack_failure_does_not_repeat_completed_cleanup();
	test_deferred_cleanup_failure_remains_retryable();
	test_deferred_ack_absent_event_releases_latch();
	test_invalid_output_is_rejected();
	puts("deferred storage start and cleanup acknowledgement policy: PASS");
	return 0;
}
