#ifndef WAN_VIPTV_STATIC_POLICY_H
#define WAN_VIPTV_STATIC_POLICY_H

#include <errno.h>
#include "route_status_policy.h"

struct wan_viptv_static_ops {
	int (*configure_address)(void *context);
	int (*publish_snapshot)(void *context);
	int (*clear_address)(void *context);
	int (*add_default_route)(void *context);
	int (*start_multicast)(void *context);
};

/*
 * Do not publish IPTV interface state until the address ioctl succeeds.
 * The address, mask and gateway are one compensatable NVRAM tuple. If that
 * publication fails, clear the configured address and do not install routes
 * or start multicast against state that recovery cannot identify. A later
 * route failure is reported, but multicast startup remains best-effort.
 */
static inline int
wan_viptv_static_start(int has_gateway,
		       const struct wan_viptv_static_ops *ops, void *context)
{
	int status, rollback_status, multicast_status;

	if ((has_gateway != 0 && has_gateway != 1) ||
	    !ops || !ops->configure_address ||
	    !ops->publish_snapshot || !ops->clear_address ||
	    !ops->start_multicast ||
	    (has_gateway && !ops->add_default_route))
		return EINVAL;

	status = ops->configure_address(context);
	if (status != 0)
		return status;

	status = ops->publish_snapshot(context);
	if (status != 0) {
		rollback_status = ops->clear_address(context);
		return route_status_accumulate(status, rollback_status);
	}

	status = 0;
	if (has_gateway)
		status = ops->add_default_route(context);

	/* A route failure must not skip multicast setup; keep the first error. */
	multicast_status = ops->start_multicast(context);
	return route_status_accumulate(status, multicast_status);
}

/*
 * The VLAN interface name is the recovery handle used by later WAN teardown.
 * Never leave a newly created VLAN active when its recovery identity cannot
 * be published; run the existing ownership-aware rollback and retain the
 * original publication error.
 */
struct wan_viptv_ifname_ops {
	int (*publish)(void *context);
	int (*rollback_vlan)(void *context);
};

static inline int
wan_viptv_ifname_publish_or_rollback(
	const struct wan_viptv_ifname_ops *ops, void *context)
{
	int status, rollback_status;

	if (!ops || !ops->publish || !ops->rollback_vlan)
		return EINVAL;

	status = ops->publish(context);
	if (status == 0)
		return 0;

	rollback_status = ops->rollback_vlan(context);
	return route_status_accumulate(status, rollback_status);
}


#endif /* WAN_VIPTV_STATIC_POLICY_H */
