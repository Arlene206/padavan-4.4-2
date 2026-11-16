#ifndef RC_HWNAT_CONFIG_POLICY_H
#define RC_HWNAT_CONFIG_POLICY_H

#include <errno.h>

/*
 * An enabled HW NAT module is only configured when the WAN VLAN lookup is
 * valid and the driver accepts the requested VID. Optional offload failures
 * are returned to the caller for status reporting; callers may still proceed
 * with ordinary software forwarding.
 */
static inline int
rc_hwnat_configure_policy(int allowed, int module_loaded, int wan_vid,
			  int (*set_wan_vid)(int))
{
	int status;

	if (!allowed)
		return 0;
	if (wan_vid < 0)
		return EIO;
	if (!module_loaded)
		return 0;
	if (!set_wan_vid)
		return EINVAL;

	status = set_wan_vid(wan_vid);
	if (status == 0)
		return 0;

	return status > 0 ? status : EIO;
}

#endif /* RC_HWNAT_CONFIG_POLICY_H */
