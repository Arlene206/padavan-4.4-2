#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "detect_wan_udhcpc_policy.h"

int
main(void)
{
	assert(detect_wan_udhcpc_should_renew(PROCESS_PRESENCE_RUNNING) == 1);
	assert(detect_wan_udhcpc_should_renew(PROCESS_PRESENCE_ABSENT) == 0);
	errno = 0;
	assert(detect_wan_udhcpc_should_renew(PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(errno == EIO);
	errno = 0;
	assert(detect_wan_udhcpc_should_renew((enum process_presence)17) == -1);
	assert(errno == EIO);
	puts("WAN DHCP renewal tri-state policy: PASS");
	return 0;
}
