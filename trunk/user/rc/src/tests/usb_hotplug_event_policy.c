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
test_null_outputs_are_rejected(void)
{
	int pending = 0;

	assert(usb_hotplug_event_pair_status(0, 0, 0, 0,
		NULL, &pending) == EINVAL);
	assert(usb_hotplug_event_pair_status(0, 0, 0, 0,
		&pending, NULL) == EINVAL);
}

int
main(void)
{
	test_no_events_are_reported_as_pending();
	test_coalesced_events_preserve_both_markers();
	test_only_exact_one_is_an_event();
	test_each_failed_marker_read_preserves_the_pair();
	test_null_outputs_are_rejected();
	puts("USB deferred unplug/plug marker policy: PASS");
	return 0;
}
