#ifndef PADAVAN_WAN_APCLI_WAIT_POLICY_H
#define PADAVAN_WAN_APCLI_WAIT_POLICY_H

#include <errno.h>

/* wait_apcli_connected() returns 1 on association and 0 on timeout. */
static inline int
rc_wan_apcli_wait_status(int connected)
{
	return connected ? 0 : ETIMEDOUT;
}

#endif /* PADAVAN_WAN_APCLI_WAIT_POLICY_H */
