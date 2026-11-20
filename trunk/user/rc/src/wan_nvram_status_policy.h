#ifndef PADAVAN_WAN_NVRAM_STATUS_POLICY_H
#define PADAVAN_WAN_NVRAM_STATUS_POLICY_H

#include <errno.h>

/* NVRAM APIs use 0/-1 and report the concrete failure through errno. */
static int
rc_wan_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* PADAVAN_WAN_NVRAM_STATUS_POLICY_H */
