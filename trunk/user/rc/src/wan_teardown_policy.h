#ifndef WAN_TEARDOWN_POLICY_H
#define WAN_TEARDOWN_POLICY_H

#include <errno.h>
#include <string.h>

/*
 * Static/DHCP IPoE always clears its indexed DNS snapshot. Other WAN modes
 * clear it only when a checked global MAN DNS snapshot proves one is present.
 * A failed read must preserve the demand-PPP snapshot rather than look empty.
 */
static inline int
wan_teardown_should_clear_dns_snapshot(int force_clear, int read_status,
				       const char *global_dns,
				       int *clear_dns)
{
	if (!clear_dns)
		return EINVAL;
	*clear_dns = 0;
	if (force_clear) {
		*clear_dns = 1;
		return 0;
	}
	if (read_status != 0)
		return read_status;
	if (!global_dns)
		return EINVAL;
	*clear_dns = strlen(global_dns) > 6;
	return 0;
}

/*
 * A WAN gateway /32 route is removed only when the checked gateway, address
 * and netmask snapshot is complete. On a read error, keep the host route
 * rather than guessing whether it is an off-subnet route installed by WAN-up.
 */
static inline int
wan_teardown_gateway_host_route_required(int gateway_read_status,
					 int address_read_status,
					 int netmask_read_status,
					 int gateway_valid,
					 int ppp_index,
					 int gateway_off_subnet)
{
	return gateway_read_status == 0 && address_read_status == 0 &&
		netmask_read_status == 0 && gateway_valid &&
		ppp_index < 0 && gateway_off_subnet;
}

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

/* Keep the initiating teardown error; otherwise report a failed monitor resume. */
static inline int
wan_teardown_abort_status(int operation_status, int resume_status)
{
	if (operation_status != 0)
		return operation_status;
	if (resume_status != 0)
		return resume_status;
	return -1;
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
