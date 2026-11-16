#ifndef RC_WAN_ROUTE_FLUSH_POLICY_H
#define RC_WAN_ROUTE_FLUSH_POLICY_H

#include <errno.h>

#include "service_start_status.h"

typedef int (*rc_wan_route_flush_fn)(char *ifname);

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

#endif /* RC_WAN_ROUTE_FLUSH_POLICY_H */
