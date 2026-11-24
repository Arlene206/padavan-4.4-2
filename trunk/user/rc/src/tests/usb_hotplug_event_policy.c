#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "usb_hotplug_event_policy.h"

static void
test_no_events_are_reported_as_pending(void)
{
	int unplug_pending = -1, plug_pending = -1;

	assert(usb_hotplug_event_pair_status(0, 0, 0, 0,
		&unplug_pending, &plug_pending) == 0);
	assert(unplug_pending == 0);
	assert(plug_pending == 0);
}

static void
test_coalesced_events_preserve_both_markers(void)
{
	int unplug_pending = 0, plug_pending = 0;

	assert(usb_hotplug_event_pair_status(0, 1, 0, 1,
		&unplug_pending, &plug_pending) == 0);
	assert(unplug_pending == 1);
	assert(plug_pending == 1);
}

static void
test_only_exact_one_is_an_event(void)
{
	int unplug_pending = 1, plug_pending = 1;

	assert(usb_hotplug_event_pair_status(0, 2, 0, -1,
		&unplug_pending, &plug_pending) == 0);
	assert(unplug_pending == 0);
	assert(plug_pending == 0);
}

static void
test_each_failed_marker_read_preserves_the_pair(void)
{
	int unplug_pending = 1, plug_pending = 1;

	assert(usb_hotplug_event_pair_status(EIO, 1, 0, 1,
		&unplug_pending, &plug_pending) == EIO);
	assert(unplug_pending == 0);
	assert(plug_pending == 0);
	assert(usb_hotplug_event_pair_status(0, 1, EACCES, 1,
		&unplug_pending, &plug_pending) == EACCES);
	assert(unplug_pending == 0);
	assert(plug_pending == 0);
	assert(usb_hotplug_event_pair_status(-1, 1, 0, 1,
		&unplug_pending, &plug_pending) == EIO);
	assert(unplug_pending == 0);
	assert(plug_pending == 0);
}

static void
test_ack_failure_does_not_replay_successful_action(void)
{
	int action_done = 0;

	assert(usb_hotplug_event_action_should_run(1, action_done));
	assert(usb_hotplug_event_ack_status(1, 0, EACCES,
		&action_done) == EACCES);
	assert(action_done == 1);
	assert(!usb_hotplug_event_action_should_run(1, action_done));
	/* Retry only the marker clear, not the already-completed side effect. */
	assert(usb_hotplug_event_ack_status(1, 0, 0, &action_done) == 0);
	assert(action_done == 0);
	assert(usb_hotplug_event_action_should_run(1, action_done));
}

static void
test_positive_stop_error_status_remains_retryable(void)
{
	int action_done = 0;

	/* stop_usb_printer_spoolers() returns a positive errno-style status. */
	assert(usb_hotplug_event_ack_status(1, EBUSY, 0, &action_done) == EBUSY);
	assert(action_done == 0);
	assert(usb_hotplug_event_action_should_run(1, action_done));
}
static void
test_action_failure_remains_eligible_for_retry(void)
{
	int action_done = 0;

	assert(usb_hotplug_event_action_should_run(1, action_done));
	assert(usb_hotplug_event_ack_status(1, -1, 0, &action_done) == EIO);
	assert(action_done == 0);
	assert(usb_hotplug_event_action_should_run(1, action_done));
}

static void
test_absent_event_releases_action_latch(void)
{
	int action_done = 1;

	assert(usb_hotplug_event_ack_status(0, 0, EIO, &action_done) == 0);
	assert(action_done == 0);
	assert(usb_hotplug_event_ack_status(1, 0, -1, &action_done) == EIO);
	assert(action_done == 1);
	assert(usb_hotplug_event_ack_status(1, 0, 0, &action_done) == 0);
	assert(action_done == 0);
	assert(usb_hotplug_event_ack_status(1, 0, 0, NULL) == EINVAL);
}

static void
test_null_outputs_are_rejected(void)
{
	int pending = 0;

	assert(usb_hotplug_event_pair_status(0, 0, 0, 0,
		NULL, &pending) == EINVAL);
	assert(usb_hotplug_event_pair_status(0, 0, 0, 0,
		&pending, NULL) == EINVAL);
	assert(!usb_hotplug_event_ack_allowed(0, 0));
	assert(!usb_hotplug_event_ack_allowed(1, EIO));
	assert(!usb_hotplug_event_ack_allowed(1, -1));
	assert(usb_hotplug_event_ack_allowed(1, 0));
}

int
main(void)
{
	test_no_events_are_reported_as_pending();
	test_coalesced_events_preserve_both_markers();
	test_only_exact_one_is_an_event();
	test_each_failed_marker_read_preserves_the_pair();
	test_ack_failure_does_not_replay_successful_action();
	test_positive_stop_error_status_remains_retryable();
	test_action_failure_remains_eligible_for_retry();
	test_absent_event_releases_action_latch();
	test_null_outputs_are_rejected();
	puts("USB deferred unplug/plug marker policy: PASS");
	return 0;
}
