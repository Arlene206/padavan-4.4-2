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
	test_invalid_output_is_rejected();
	puts("deferred storage start event read policy: PASS");
	return 0;
}
