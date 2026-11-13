#ifndef WAN_TEARDOWN_POLICY_H
#define WAN_TEARDOWN_POLICY_H

#include <string.h>

struct wan_teardown_interface_ops {
	int (*ifdown)(void *context, const char *ifname);
};

/*
 * Network helpers use both ioctl-style (-1) and errno-valued failures.
 * Any nonzero operation status means the teardown is incomplete.
 */
static inline int
wan_teardown_status_update(int current_status, int operation_status)
{
	return (operation_status != 0) ? -1 : current_status;
}

/*
 * Attempt every distinct interface even after an earlier failure.  Callers
 * must not publish WAN-down state unless the complete teardown succeeds.
 * The USB network name is optional because most WAN modes have no NDIS link.
 */
static inline int
wan_teardown_ifdown_interfaces(const char *management_ifname,
			       const char *wan_ifname,
			       const char *usbnet_ifname,
			       const struct wan_teardown_interface_ops *ops,
			       void *context)
{
	const char *ifnames[3];
	int i, j, result = 0;

	if (!management_ifname || !*management_ifname ||
	    !wan_ifname || !*wan_ifname || !ops || !ops->ifdown)
		return -1;

	ifnames[0] = management_ifname;
	ifnames[1] = wan_ifname;
	ifnames[2] = usbnet_ifname;

	for (i = 0; i < 3; i++) {
		if (!ifnames[i] || !*ifnames[i])
			continue;
		for (j = 0; j < i; j++) {
			if (ifnames[j] && strcmp(ifnames[j], ifnames[i]) == 0)
				break;
		}
		if (j != i)
			continue;
		if (ops->ifdown(context, ifnames[i]) != 0)
			result = -1;
	}

	return result;
}

#endif
