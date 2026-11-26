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
 * Restore the old interface address and its route inventory. Both independent
 * recovery operations are attempted and the first error is retained.
 */
static inline int
dhcp_wan_route_transition_restore_old(
	const struct dhcp_wan_route_transition_ops *ops, void *context,
	int *rollback_status)
{
	int status = 0, operation_status;

	if (rollback_status)
		*rollback_status = 0;
	if (!ops || !ops->restore_old_address || !ops->restore_old_routes)
		return EINVAL;

	operation_status = ops->restore_old_address(context);
	if (operation_status != 0)
		status = operation_status;
	operation_status = ops->restore_old_routes(context);
	if (operation_status != 0 && status == 0)
		status = operation_status;
	if (rollback_status)
		*rollback_status = status;
	return status;
}

/*
 * Abort a lease transition after its NVRAM snapshot failed. Keep the snapshot
 * error primary while attempting both network rollback operations.
 */
static inline int
dhcp_wan_route_transition_abort_snapshot(
	int snapshot_status, const struct dhcp_wan_route_transition_ops *ops,
	void *context, int *rollback_status)
{
	if (rollback_status)
		*rollback_status = 0;
	if (snapshot_status == 0)
		return 0;
	(void)dhcp_wan_route_transition_restore_old(ops, context,
						    rollback_status);
	return snapshot_status;
}

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
	rollback = dhcp_wan_route_transition_restore_old(
		ops, context, rollback_status);
	if (rollback_status)
		*rollback_status = rollback;

	return status;
}

/* Ancillary service startup must not make a successfully configured DHCP
 * address/route tuple look like a rejected DHCP ACK to udhcpc. */
static inline int
dhcp_bound_callback_status(int route_setup_status, int mtu_status)
{
	if (route_setup_status != 0)
		return route_setup_status;
	return mtu_status;
}

static inline int
dhcp_wan_route_setup_retry_required(int route_setup_status)
{
	return route_setup_status != 0;
}

/* Retry the complete lease application after address/lease failure as well
 * as route failure, even if the DHCP server renews an unchanged tuple. */
static inline int
dhcp_wan_bound_retry_required(int lease_error, int route_setup_status,
			      int mtu_setup_status)
{
	return lease_error != 0 ||
		dhcp_wan_route_setup_retry_required(route_setup_status) ||
		mtu_setup_status != 0;
}

/* Never convert an unreadable previous status field to the healthy value 0.
 * A complete checked tuple is required before renewal can skip a full rebind. */
static inline int
dhcp_wan_bound_retry_decide(int lease_read_status, int route_read_status,
			    int mtu_read_status, int lease_error,
			    int route_setup_status, int mtu_setup_status,
			    int *retry_required)
{
	if (!retry_required)
		return EINVAL;
	*retry_required = 0;
	if (lease_read_status != 0)
		return lease_read_status;
	if (route_read_status != 0)
		return route_read_status;
	if (mtu_read_status != 0)
		return mtu_read_status;

	*retry_required = dhcp_wan_bound_retry_required(
		lease_error, route_setup_status, mtu_setup_status);
	return 0;
}


enum {
	DHCP_WAN_RETRY_MARKER_LEASE = 0,
	DHCP_WAN_RETRY_MARKER_ROUTE,
	DHCP_WAN_RETRY_MARKER_MTU,
	DHCP_WAN_RETRY_MARKER_COUNT
};

typedef int (*dhcp_wan_retry_marker_writer)(void *context,
					   unsigned int marker, int value);

/*
 * Publish all three retry indicators. If any write fails, best-effort latch
 * every indicator nonzero so a later unchanged DHCP ACK cannot mistake a
 * partially published retry tuple for a healthy lease. The caller still
 * returns the original publication error to udhcpc.
 */
static inline int
dhcp_wan_bound_retry_markers_publish(
	int lease_error, int route_error, int mtu_error,
	dhcp_wan_retry_marker_writer writer, void *context)
{
	const int values[DHCP_WAN_RETRY_MARKER_COUNT] = {
		lease_error, route_error, mtu_error
	};
	unsigned int marker;
	int status, first_status = 0;

	if (!writer)
		return EINVAL;

	for (marker = 0; marker < DHCP_WAN_RETRY_MARKER_COUNT; ++marker) {
		status = writer(context, marker, values[marker]);
		if (status != 0 && first_status == 0)
			first_status = status;
	}

	if (first_status != 0) {
		for (marker = 0; marker < DHCP_WAN_RETRY_MARKER_COUNT; ++marker)
			(void)writer(context, marker, 1);
	}

	return first_status;
}

#endif /* DHCP_WAN_ROUTE_TRANSITION_POLICY_H */
