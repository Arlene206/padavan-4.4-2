#ifndef PADAVAN_WAN_APCLI_WAIT_POLICY_H
#define PADAVAN_WAN_APCLI_WAIT_POLICY_H

#include <errno.h>

/*
 * wait_apcli_connected() returns 1 on association, 0 on timeout, and -1
 * when its interrupt-safe wait fails. Preserve that failure for WAN startup.
 */
static inline int
rc_wan_apcli_wait_status(int wait_result)
{
	if (wait_result > 0)
		return 0;
	if (wait_result == 0)
		return ETIMEDOUT;
	return errno > 0 ? errno : EIO;
}

#endif /* PADAVAN_WAN_APCLI_WAIT_POLICY_H */
