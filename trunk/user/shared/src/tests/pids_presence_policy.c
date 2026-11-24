#include <assert.h>
#include <stdio.h>

#include "pids_presence_policy.h"

static void
test_running_remains_true(void)
{
	assert(pids_presence_legacy_boolean(PROCESS_PRESENCE_RUNNING) == 1);
}

static void
test_confirmed_absence_remains_false(void)
{
	assert(pids_presence_legacy_boolean(PROCESS_PRESENCE_ABSENT) == 0);
}

static void
test_unknown_is_never_downgraded_to_absent(void)
{
	assert(pids_presence_legacy_boolean(PROCESS_PRESENCE_UNKNOWN) == 1);
}

static void
test_unrecognized_state_fails_closed(void)
{
	assert(pids_presence_legacy_boolean((enum process_presence)99) == 1);
}

int
main(void)
{
	test_running_remains_true();
	test_confirmed_absence_remains_false();
	test_unknown_is_never_downgraded_to_absent();
	test_unrecognized_state_fails_closed();
	puts("legacy pids() fail-closed presence mapping: PASS");
	return 0;
}
