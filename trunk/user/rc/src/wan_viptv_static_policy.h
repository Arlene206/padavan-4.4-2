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

/*
 * Retiring an old IPTV VLAN must not destroy its only recovery handle before
 * the interface is actually removed. Clear the identity first; if VLAN
 * removal fails, restore the original identity so a later teardown can retry.
 */
struct wan_viptv_old_vlan_ops {
	int (*clear_identity)(void *context);
	int (*remove_vlan)(void *context);
	int (*restore_identity)(void *context);
};

static inline int
wan_viptv_ifname_remove_old_vlan(
	const struct wan_viptv_old_vlan_ops *ops, void *context)
{
	int status, restore_status;

	if (!ops || !ops->clear_identity || !ops->remove_vlan ||
	    !ops->restore_identity)
		return EINVAL;

	status = ops->clear_identity(context);
	if (status != 0)
		return status;

	status = ops->remove_vlan(context);
	if (status == 0)
		return 0;

	restore_status = ops->restore_identity(context);
	return route_status_accumulate(status, restore_status);
}



struct wan_viptv_dynamic_start_ops {
	int (*start)(void *context);
	int (*stop_client)(void *context);
	int (*rollback_vlan)(void *context);
};

struct wan_viptv_dynamic_start_result {
	int stop_status;
	int stop_errno;
	int rollback_status;
	int rollback_errno;
};

/* Stop a possibly detached DHCP/ZeroConf child before removing its VLAN.
 * If stop is uncertain, retain the published interface recovery identity. */
static inline int
wan_viptv_dynamic_start_with_rollback(
	const struct wan_viptv_dynamic_start_ops *ops, void *context,
	struct wan_viptv_dynamic_start_result *result)
{
	int start_status, start_errno;

	if (!ops || !ops->start || !ops->stop_client ||
	    !ops->rollback_vlan || !result)
		return EINVAL;
	result->stop_status = 0;
	result->stop_errno = 0;
	result->rollback_status = 0;
	result->rollback_errno = 0;

	errno = 0;
	start_status = ops->start(context);
	start_errno = errno;
	if (start_status == 0)
		return 0;
	if (start_status < 0 && start_errno == 0)
		start_errno = EIO;

	errno = 0;
	result->stop_status = ops->stop_client(context);
	result->stop_errno = errno;
	if (result->stop_status == 0) {
		errno = 0;
		result->rollback_status = ops->rollback_vlan(context);
		result->rollback_errno = errno;
	}
	errno = start_errno;
	return start_status;
}

#endif /* WAN_VIPTV_STATIC_POLICY_H */
