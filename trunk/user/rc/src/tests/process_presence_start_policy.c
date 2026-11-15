#include <assert.h>
#include <stdio.h>

#include "process_presence_start_policy.h"

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

	puts("Process presence start/notify tri-state policy: PASS");
	return 0;
}
