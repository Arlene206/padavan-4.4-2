#include <assert.h>
#include <stdio.h>

#include "optional_daemon_policy.h"

int main(void)
{
	assert(optional_daemon_start_allowed(PROCESS_PRESENCE_ABSENT));
	assert(!optional_daemon_start_allowed(PROCESS_PRESENCE_RUNNING));
	assert(!optional_daemon_start_allowed(PROCESS_PRESENCE_UNKNOWN));
	assert(!optional_daemon_start_allowed((enum process_presence)9));

	assert(optional_daemon_stop_complete(0, PROCESS_PRESENCE_ABSENT) == 0);
	assert(optional_daemon_stop_complete(-1, PROCESS_PRESENCE_ABSENT) == -1);
	assert(optional_daemon_stop_complete(-7, PROCESS_PRESENCE_RUNNING) == -7);
	assert(optional_daemon_stop_complete(0, PROCESS_PRESENCE_RUNNING) == -1);
	assert(optional_daemon_stop_complete(0, PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(optional_daemon_stop_complete(0, (enum process_presence)9) == -1);

	assert(!optional_daemon_failed_start_may_survive(PROCESS_PRESENCE_ABSENT));
	assert(optional_daemon_failed_start_may_survive(PROCESS_PRESENCE_RUNNING));
	assert(optional_daemon_failed_start_may_survive(PROCESS_PRESENCE_UNKNOWN));
	assert(optional_daemon_failed_start_may_survive((enum process_presence)9));
	puts("optional daemon tri-state start/stop postcondition policy: PASS");
	return 0;
}
