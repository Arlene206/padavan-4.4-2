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

/* The WAN /32 gateway host route is installed only for a valid off-subnet
 * gateway on a non-PPP interface. Use the same predicate at up and down. */
static inline int
route_status_wan_gateway_host_route_required(int gateway_valid,
					     int ppp_index,
					     int gateway_off_subnet)
{
	return gateway_valid && ppp_index < 0 && gateway_off_subnet;
}

static inline int
route_status_demand_ready(int route_status)
{
	return route_status == 0;
}
#endif
