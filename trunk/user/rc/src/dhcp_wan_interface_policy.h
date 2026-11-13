#ifndef DHCP_WAN_INTERFACE_POLICY_H
#define DHCP_WAN_INTERFACE_POLICY_H

#include "dhcp_lease_policy.h"

struct dhcp_wan_interface_ops {
	int (*clear_address)(void *context);
	int (*set_address)(void *context, const char *ipaddr, const char *netmask);
};

/*
 * Configure a DHCP WAN address before the caller publishes lease state or
 * removes old routes. On any failed ioctl, best-effort restore the previous
 * address (or clear the interface when no previous lease was valid).
 */
static inline int
dhcp_wan_interface_apply(const char *old_ipaddr, const char *old_netmask,
			 const char *new_ipaddr, const char *new_netmask,
			 int clear_before_set,
			 const struct dhcp_wan_interface_ops *ops, void *context,
			 int *rollback_status)
{
	int status, rollback = 0;
	int old_address_valid;

	if (rollback_status)
		*rollback_status = 0;
	if (!new_ipaddr || !new_netmask ||
	    !dhcp_lease_ipv4_is_valid(new_ipaddr) ||
	    !dhcp_lease_netmask_is_valid(new_netmask) ||
	    !ops || !ops->clear_address || !ops->set_address ||
	    (clear_before_set != 0 && clear_before_set != 1))
		return EINVAL;

	old_address_valid = dhcp_lease_ipv4_is_valid(old_ipaddr) &&
		dhcp_lease_netmask_is_valid(old_netmask);

	if (clear_before_set) {
		status = ops->clear_address(context);
		if (status != 0)
			goto rollback;
	}

	status = ops->set_address(context, new_ipaddr, new_netmask);
	if (status == 0)
		return 0;

rollback:
	if (old_address_valid)
		rollback = ops->set_address(context, old_ipaddr, old_netmask);
	else
		rollback = ops->clear_address(context);

	if (rollback_status)
		*rollback_status = rollback;
	return status;
}

#endif /* DHCP_WAN_INTERFACE_POLICY_H */
