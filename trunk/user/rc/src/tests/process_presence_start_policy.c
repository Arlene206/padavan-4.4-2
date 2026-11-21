#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "process_presence_start_policy.h"

static void
test_start_status_preserves_launch_and_inventory_failures(void)
{
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_NEW, EIO, 0) == 0);
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_NEW, EIO, -1) == -1);
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_NEW, 0, EACCES) == EACCES);

	/* Existing processes are a successful no-op; never signal duplicate start. */
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_NOTIFY, EIO, EACCES) == 0);

	/* A failed or incomplete procfs scan cannot silently skip a requested monitor. */
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_ABORT, ENOMEM, 0) == ENOMEM);
	assert(process_presence_start_status_for_action(
		PROCESS_PRESENCE_START_ABORT, 0, 0) == EIO);
	assert(process_presence_start_status_for_action(
		(enum process_presence_start_action)7, 0, 0) == EIO);
}

int
main(void)
{
	assert(process_presence_start_action_for_state(PROCESS_PRESENCE_ABSENT) ==
	       PROCESS_PRESENCE_START_NEW);
	assert(process_presence_start_action_for_state(PROCESS_PRESENCE_RUNNING) ==
	       PROCESS_PRESENCE_START_NOTIFY);
	assert(process_presence_start_action_for_state(PROCESS_PRESENCE_UNKNOWN) ==
	       PROCESS_PRESENCE_START_ABORT);
	assert(process_presence_start_action_for_state((enum process_presence)7) ==
	       PROCESS_PRESENCE_START_ABORT);
	assert(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS == 1);
	assert(PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS == 1);
	assert(PROCESS_PRESENCE_START_PROBE_ATTEMPTS == 4);
	assert(process_presence_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(process_presence_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == -1);
	assert(process_presence_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(process_presence_start_postcondition(0, (enum process_presence)9) == -1);
	assert(process_presence_start_postcondition(17, PROCESS_PRESENCE_RUNNING) == 17);
	assert(process_presence_start_postcondition(17, PROCESS_PRESENCE_ABSENT) == 17);

	test_start_status_preserves_launch_and_inventory_failures();

	puts("Process presence start/notify and detached-daemon postcondition policy: PASS");
	return 0;
}
