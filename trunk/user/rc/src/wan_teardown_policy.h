#ifndef WAN_TEARDOWN_POLICY_H
#define WAN_TEARDOWN_POLICY_H

#include <string.h>

struct wan_teardown_interface_ops {
	int (*ifdown)(void *context, const char *ifname);
};

/*
 * Attempt every distinct interface even after an earlier failure.  Callers
 * must not publish WAN-down state unless the complete teardown succeeds.
 */
static inline int
wan_teardown_ifdown_interfaces(const char *management_ifname,
			       const char *wan_ifname,
			       const struct wan_teardown_interface_ops *ops,
			       void *context)
{
	int result = 0;

	if (!management_ifname || !*management_ifname ||
	    !wan_ifname || !*wan_ifname || !ops || !ops->ifdown)
		return -1;

	if (ops->ifdown(context, management_ifname) != 0)
		result = -1;
	if (strcmp(management_ifname, wan_ifname) != 0 &&
	    ops->ifdown(context, wan_ifname) != 0)
		result = -1;

	return result;
}

#endif
