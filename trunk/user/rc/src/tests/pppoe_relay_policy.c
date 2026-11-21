#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "pppoe_relay_policy.h"

int main(void)
{
	assert(pppoe_relay_should_start(PROCESS_PRESENCE_ABSENT));
	assert(!pppoe_relay_should_start(PROCESS_PRESENCE_RUNNING));
	assert(!pppoe_relay_should_start(PROCESS_PRESENCE_UNKNOWN));
	assert(!pppoe_relay_should_start((enum process_presence)9));

	assert(pppoe_relay_start_status(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(pppoe_relay_start_status(0, PROCESS_PRESENCE_ABSENT) == ESRCH);
	assert(pppoe_relay_start_status(0, PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(pppoe_relay_start_status(0, (enum process_presence)9) == EIO);
	assert(pppoe_relay_start_status(ENOENT, PROCESS_PRESENCE_RUNNING) == ENOENT);
	assert(pppoe_relay_start_status(ETIMEDOUT, PROCESS_PRESENCE_ABSENT) == ETIMEDOUT);

	puts("PPPoE relay tri-state startup/readiness policy: PASS");
	return 0;
}
