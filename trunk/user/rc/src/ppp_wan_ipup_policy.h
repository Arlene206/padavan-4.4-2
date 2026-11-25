#ifndef PPP_WAN_IPUP_POLICY_H
#define PPP_WAN_IPUP_POLICY_H

#include <errno.h>

#include "dhcp_lease_policy.h"

struct ppp_wan_ipup_ops {
	int (*set_interface_address)(void *context, const char *ipaddr,
				     const char *netmask);
	int (*publish_address)(void *context, const char *ipaddr,
			       const char *netmask);
	int (*clear_interface_address)(void *context);
};

/*
 * Apply PPP interface configuration before publishing its runtime NVRAM
 * tuple. Failed publication clears the new interface address and propagates
 * the original NVRAM error so WAN routes are not created from mixed state.
 */
static inline int
ppp_wan_ipup_apply_address(const char *ipaddr, const char *remote_ip,
			   const char *netmask, const struct ppp_wan_ipup_ops *ops,
			   void *context)
{
	int status;

	if (!ipaddr || !*ipaddr || !dhcp_lease_ipv4_is_valid(ipaddr) ||
	    !remote_ip || !*remote_ip || !dhcp_lease_ipv4_is_valid(remote_ip) ||
	    !netmask || !*netmask || !dhcp_lease_netmask_is_valid(netmask) ||
	    !ops || !ops->set_interface_address || !ops->publish_address ||
	    !ops->clear_interface_address) {
		errno = EINVAL;
		return EINVAL;
	}

	status = ops->set_interface_address(context, ipaddr, netmask);
	if (status != 0)
		return status;

	status = ops->publish_address(context, ipaddr, netmask);
	if (status != 0) {
		/*
		 * The interface is live but its complete runtime NVRAM tuple could
		 * not be committed. Remove the address before returning to pppd.
		 */
		(void)ops->clear_interface_address(context);
		return status;
	}
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
