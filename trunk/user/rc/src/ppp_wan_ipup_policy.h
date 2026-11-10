#ifndef PPP_WAN_IPUP_POLICY_H
#define PPP_WAN_IPUP_POLICY_H

#include <errno.h>

struct ppp_wan_ipup_ops {
	int (*set_interface_address)(void *context, const char *ipaddr,
				     const char *netmask);
	void (*publish_address)(void *context, const char *ipaddr,
			       const char *netmask);
};

/*
 * Keep PPP lease publication behind the interface operation. A failed
 * ifconfig must not replace the last-known-good WAN address in NVRAM.
 */
static inline int
ppp_wan_ipup_apply_address(const char *ipaddr, const char *netmask,
			   const struct ppp_wan_ipup_ops *ops, void *context)
{
	int status;

	if (!ipaddr || !*ipaddr || !netmask || !*netmask ||
	    !ops || !ops->set_interface_address || !ops->publish_address) {
		errno = EINVAL;
		return EINVAL;
	}

	status = ops->set_interface_address(context, ipaddr, netmask);
	if (status != 0)
		return status;

	ops->publish_address(context, ipaddr, netmask);
	return 0;
}

/*
 * Keep the WAN route-setup error observable to the caller, but never report
 * success when releasing the callback-session lock itself failed.
 */
static inline int
ppp_wan_ipup_finish_status(int route_status, int unlock_status)
{
	if (unlock_status != 0)
		return unlock_status;
	return route_status;
}

#endif /* PPP_WAN_IPUP_POLICY_H */
