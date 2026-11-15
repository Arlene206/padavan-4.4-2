#include <assert.h>
#include <stdio.h>

#include "pppoe_relay_policy.h"

int main(void)
{
	assert(pppoe_relay_should_start(PROCESS_PRESENCE_ABSENT));
	assert(!pppoe_relay_should_start(PROCESS_PRESENCE_RUNNING));
	assert(!pppoe_relay_should_start(PROCESS_PRESENCE_UNKNOWN));
	assert(!pppoe_relay_should_start((enum process_presence)9));
	puts("PPPoE relay tri-state startup policy: PASS");
	return 0;
}
