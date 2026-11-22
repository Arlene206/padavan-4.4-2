#ifndef RC_WAN_ROUTE_FLUSH_POLICY_H
#define RC_WAN_ROUTE_FLUSH_POLICY_H

#include <errno.h>

#include "service_start_status.h"

typedef int (*rc_wan_route_flush_fn)(char *ifname);

/*
 * The manual WAN interface is optional while WAN configuration is staged.
 * The LAN bridge is always a required route domain for a full restart.
 */
static inline int
rc_wan_route_flush_select_interfaces(const char *optional_ifname,
				     const char *required_ifname,
				     char **ifnames,
				     unsigned int capacity,
				     unsigned int *count_out)
{
	unsigned int count = 0;

	if (count_out)
		*count_out = 0;
	if (!required_ifname || !required_ifname[0] || !ifnames ||
	    !count_out || capacity == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	if (optional_ifname && optional_ifname[0]) {
		if (capacity < 2) {
			errno = ENOSPC;
			return ENOSPC;
		}
		ifnames[count++] = (char *)optional_ifname;
	}
	ifnames[count++] = (char *)required_ifname;
	*count_out = count;
	return 0;
}


/*
 * Flush every requested interface even if an earlier flush fails. Returning
 * the first error prevents a WAN restart from rebuilding routes on top of
 * stale interface routes, while independent cleanup attempts still run.
 */
static inline int
rc_wan_route_flush_apply(char *const *ifnames, unsigned int count,
			 rc_wan_route_flush_fn flush)
{
	int first_status = 0;
	int status;
	unsigned int i;

	if ((!ifnames && count != 0) || !flush)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!ifnames[i] || !ifnames[i][0])
			status = EINVAL;
		else
			status = flush(ifnames[i]);
		first_status = rc_first_service_start_status(first_status, status);
	}

	return first_status;
}

typedef int (*rc_wan_route_cache_flush_fn)(void);

/* Attempt the independent global cache flush even after interface failures. */
static inline int
rc_wan_route_flush_and_cache_apply(char *const *ifnames, unsigned int count,
				   rc_wan_route_flush_fn flush,
				   rc_wan_route_cache_flush_fn flush_cache)
{
	int first_status = rc_wan_route_flush_apply(ifnames, count, flush);
	int cache_status = flush_cache ? flush_cache() : EINVAL;

	return rc_first_service_start_status(first_status, cache_status);
}

/*
 * A failed step happens after the restart path has flushed routes from the
 * WAN interface and LAN bridge. Restore LAN static routes before aborting,
 * while preserving the original step failure as the primary error.
 */
typedef int (*rc_wan_static_route_restore_fn)(char *lan_ifname);

static inline int
rc_wan_restore_lan_routes_after_failed_step(
	int primary_status, char *lan_ifname,
	rc_wan_static_route_restore_fn restore, int *restore_status)
{
	int route_status;

	if (restore_status)
		*restore_status = 0;
	if (primary_status == 0)
		return 0;

	if (!lan_ifname || !lan_ifname[0] || !restore) {
		route_status = EINVAL;
	} else {
		/*
		 * Route flush/reset failures can race transient route-table
		 * updates. Re-adds are idempotent (EEXIST is normalized), so
		 * make one bounded recovery retry before abandoning LAN reachability.
		 */
		route_status = restore(lan_ifname);
		if (route_status != 0)
			route_status = restore(lan_ifname);
	}

	if (restore_status)
		*restore_status = route_status;

	/* Recovery must never replace the original failed restart step. */
	return rc_first_service_start_status(primary_status, route_status);
}

/* Keep the reset-specific name used by existing callers and source contracts. */
static inline int
rc_wan_restore_lan_routes_after_failed_reset(
	int reset_status, char *lan_ifname,
	rc_wan_static_route_restore_fn restore, int *restore_status)
{
	return rc_wan_restore_lan_routes_after_failed_step(
		reset_status, lan_ifname, restore, restore_status);
}

#endif /* RC_WAN_ROUTE_FLUSH_POLICY_H */
