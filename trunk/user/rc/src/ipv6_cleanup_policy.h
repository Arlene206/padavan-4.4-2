#ifndef RC_IPV6_CLEANUP_POLICY_H
#define RC_IPV6_CLEANUP_POLICY_H

#include <errno.h>

#include "service_start_status.h"

typedef int (*rc_ipv6_global_cleanup_fn)(void);
typedef int (*rc_ipv6_interface_cleanup_fn)(char *ifname);

/*
 * Disable IPv6 only after attempting all independent stale-state cleanup
 * operations. Preserve the first failure for the enclosing restart result.
 */
static inline int
rc_ipv6_disable_cleanup_apply(rc_ipv6_global_cleanup_fn clear_routes,
			     rc_ipv6_global_cleanup_fn clear_addrs,
			     rc_ipv6_interface_cleanup_fn clear_neigh,
			     char *bridge_ifname)
{
	int first_status = 0;
	int status;

	if (!clear_routes || !clear_addrs || !clear_neigh)
		return EINVAL;

	status = clear_routes();
	first_status = rc_first_service_start_status(first_status, status);

	status = clear_addrs();
	first_status = rc_first_service_start_status(first_status, status);

	status = clear_neigh(bridge_ifname);
	first_status = rc_first_service_start_status(first_status, status);

	return first_status;
}

#endif /* RC_IPV6_CLEANUP_POLICY_H */
