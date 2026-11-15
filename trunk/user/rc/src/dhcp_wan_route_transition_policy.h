#ifndef DHCP_WAN_ROUTE_TRANSITION_POLICY_H
#define DHCP_WAN_ROUTE_TRANSITION_POLICY_H

#include <errno.h>
#include "dhcp_lease_policy.h"

typedef int (*dhcp_wan_route_transition_operation)(void *context);

struct dhcp_wan_route_transition_ops {
	dhcp_wan_route_transition_operation delete_host_route;
	dhcp_wan_route_transition_operation delete_default_route;
	dhcp_wan_route_transition_operation delete_dhcp_routes;
	dhcp_wan_route_transition_operation restore_old_address;
	dhcp_wan_route_transition_operation restore_old_routes;
};

/*
 * Remove the previous lease's route inventory before publishing a replacement.
 * Every requested delete is attempted. A failed delete causes the old address
 * and route inventory to be restored; rollback attempts are never short-cut.
 * ESRCH is harmless for deletes because the requested route is already absent.
 */
static inline int
dhcp_wan_route_transition_apply(int remove_host_route,
				int remove_default_route,
				int remove_dhcp_routes,
				const struct dhcp_wan_route_transition_ops *ops,
				void *context, int *rollback_status)
{
	int status = 0, rollback = 0, operation_status;

	if (rollback_status)
		*rollback_status = 0;
	if ((remove_host_route != 0 && remove_host_route != 1) ||
	    (remove_default_route != 0 && remove_default_route != 1) ||
	    (remove_dhcp_routes != 0 && remove_dhcp_routes != 1) ||
	    !ops || !ops->restore_old_address || !ops->restore_old_routes ||
	    (remove_host_route && !ops->delete_host_route) ||
	    (remove_default_route && !ops->delete_default_route) ||
	    (remove_dhcp_routes && !ops->delete_dhcp_routes))
		return EINVAL;

	if (remove_host_route) {
		operation_status = ops->delete_host_route(context);
		if (!dhcp_lease_route_delete_is_ok(operation_status))
			status = operation_status;
	}
	if (remove_default_route) {
		operation_status = ops->delete_default_route(context);
		if (!dhcp_lease_route_delete_is_ok(operation_status) && !status)
			status = operation_status;
	}
	if (remove_dhcp_routes) {
		operation_status = ops->delete_dhcp_routes(context);
		if (!dhcp_lease_route_delete_is_ok(operation_status) && !status)
			status = operation_status;
	}
	if (status == 0)
		return 0;

	/* Restore both parts of the old lease even if the first rollback fails. */
	operation_status = ops->restore_old_address(context);
	if (operation_status != 0)
		rollback = operation_status;
	operation_status = ops->restore_old_routes(context);
	if (operation_status != 0 && !rollback)
		rollback = operation_status;
	if (rollback_status)
		*rollback_status = rollback;

	return status;
}

#endif /* DHCP_WAN_ROUTE_TRANSITION_POLICY_H */
