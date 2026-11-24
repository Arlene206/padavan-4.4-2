#ifndef MTK_ESW_IOCTL_ACCESS_POLICY_H
#define MTK_ESW_IOCTL_ACCESS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/*
 * The MTK ESW ioctl changes shared switch state and is an administrative
 * interface. Require the caller's effective credentials to hold CAP_NET_ADMIN
 * at each ioctl, not only when the device node is opened.
 */
static inline int
mtk_esw_ioctl_access_status(int has_cap_net_admin)
{
	return has_cap_net_admin == 1 ? 0 : -EPERM;
}

#endif /* MTK_ESW_IOCTL_ACCESS_POLICY_H */
