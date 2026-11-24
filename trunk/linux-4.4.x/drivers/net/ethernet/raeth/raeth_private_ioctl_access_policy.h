#ifndef RAETH_PRIVATE_IOCTL_ACCESS_POLICY_H
#define RAETH_PRIVATE_IOCTL_ACCESS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/*
 * Linux routes SIOCDEVPRIVATE requests to ndo_do_ioctl without a generic
 * CAP_NET_ADMIN check. Drivers must authorize hardware-control ioctls.
 */
static inline int
raeth_private_ioctl_access_status(int has_cap_net_admin)
{
	return has_cap_net_admin == 1 ? 0 : -EPERM;
}

#endif /* RAETH_PRIVATE_IOCTL_ACCESS_POLICY_H */
