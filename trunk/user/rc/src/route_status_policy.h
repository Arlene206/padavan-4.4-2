/*
 * Aggregate route operation errors without short-circuiting later cleanup.
 */
#ifndef ROUTE_STATUS_POLICY_H
#define ROUTE_STATUS_POLICY_H

#include <errno.h>
static inline int
route_status_accumulate(int current_status, int operation_status)
{
	return current_status ? current_status : operation_status;
}

/* A callback must not report success when its session lock release failed. */
static inline int
route_status_finish(int operation_status, int unlock_status)
{
	return unlock_status ? unlock_status : operation_status;
}

/*
 * SIOCADDRT reports EEXIST when this exact route is already installed.
 * Managed route setup can run at PPP preseed, disconnect and reconnect
 * boundaries, so an identical existing route already satisfies the request.
 * Call this only for add operations; missing-route delete errors stay visible.
 */
static inline int
route_status_idempotent_add(int operation_status)
{
	return operation_status == EEXIST ? 0 : operation_status;
}

/* Demand PPP is ready for traffic only after its complete preset route set. */
/* SIOCDELRT reports ESRCH when the requested route is already absent. */
static inline int
route_status_idempotent_delete(int operation_status)
{
	return operation_status == ESRCH ? 0 : operation_status;
}

/* Never infer subnet ownership from malformed or missing IPv4 state. */
static inline int
route_status_gateway_host_route_required(int gateway_valid,
					 int address_valid,
					 int netmask_valid,
					 int gateway_off_subnet)
{
	return gateway_valid && address_valid && netmask_valid &&
		gateway_off_subnet;
}

/* WAN host routes are only needed on non-PPP interfaces. */
static inline int
route_status_wan_gateway_host_route_required(int gateway_valid,
					     int address_valid,
					     int netmask_valid,
					     int ppp_index,
					     int gateway_off_subnet)
{
	return ppp_index < 0 &&
		route_status_gateway_host_route_required(
			gateway_valid, address_valid, netmask_valid,
			gateway_off_subnet);
}

/* An unreadable L2TP server selector must not be mistaken for disabled. */
static inline int
route_status_l2tp_server_start_requested(int selector_status, int enabled)
{
	return selector_status == 0 && enabled;
}

/* Keep WAN/config failures first; xl2tpd uses 1 as an already-running success. */
static inline int
route_status_l2tp_server_start_result(int wan_status, int selector_status,
				      int daemon_status)
{
	int status = route_status_accumulate(wan_status, selector_status);

	if (status == 0 && daemon_status < 0)
		status = daemon_status;
	return status;
}

static inline int
route_status_demand_ready(int route_status)
{
	return route_status == 0;
}
#endif
