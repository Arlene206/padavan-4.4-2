#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "miniupnpd_daemon_policy.h"

int
main(void)
{
	assert(miniupnpd_start_policy_status(EIO, 0) == EIO);
	assert(miniupnpd_start_policy_status(EIO, 1) == EIO);
	assert(miniupnpd_start_policy_status(0, 0) == ESRCH);
	assert(miniupnpd_start_policy_status(0, 1) == 0);
	assert(miniupnpd_start_policy_status(0, 7) == 0);

	assert(miniupnpd_stop_policy_status(EIO, 0) == EIO);
	assert(miniupnpd_stop_policy_status(EIO, 1) == EIO);
	assert(miniupnpd_stop_policy_status(0, 1) == EBUSY);
	assert(miniupnpd_stop_policy_status(0, 0) == 0);
	assert(miniupnpd_stop_policy_status(-1, 0) == -1);

	puts("miniupnpd daemon lifecycle policy: PASS");
	return 0;
}
