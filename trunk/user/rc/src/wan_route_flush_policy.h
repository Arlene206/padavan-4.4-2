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

#endif /* RC_WAN_ROUTE_FLUSH_POLICY_H */
