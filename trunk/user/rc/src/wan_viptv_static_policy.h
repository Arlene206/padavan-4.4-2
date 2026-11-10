#ifndef WAN_VIPTV_STATIC_POLICY_H
#define WAN_VIPTV_STATIC_POLICY_H

#include <errno.h>

struct wan_viptv_static_ops {
	int (*configure_address)(void *context);
	void (*publish_address)(void *context);
	void (*publish_gateway)(void *context);
	int (*add_default_route)(void *context);
	void (*start_multicast)(void *context);
};

/*
 * Do not publish IPTV interface state until the address ioctl succeeds.
 * Publish the gateway before attempting its route so a later teardown still
 * has the route's source data. A route failure is reported, but multicast
 * startup remains best-effort and the published state is retained.
 */
static inline int
wan_viptv_static_start(int has_gateway,
		       const struct wan_viptv_static_ops *ops, void *context)
{
	int status;

	if ((has_gateway != 0 && has_gateway != 1) ||
	    !ops || !ops->configure_address ||
	    !ops->publish_address || !ops->publish_gateway ||
	    !ops->start_multicast ||
	    (has_gateway && !ops->add_default_route))
		return EINVAL;

	status = ops->configure_address(context);
	if (status != 0)
		return status;

	ops->publish_address(context);
	ops->publish_gateway(context);

	status = 0;
	if (has_gateway)
		status = ops->add_default_route(context);

	ops->start_multicast(context);
	return status;
}

#endif /* WAN_VIPTV_STATIC_POLICY_H */
