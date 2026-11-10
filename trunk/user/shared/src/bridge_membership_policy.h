#ifndef _BRIDGE_MEMBERSHIP_POLICY_H
#define _BRIDGE_MEMBERSHIP_POLICY_H

/*
 * Linux 4.4 net/bridge/br_private.h defines BR_MAX_PORTS as 1 << 10.
 * Query the entire bridge port list to avoid missing a later entry.
 */
#define BRIDGE_PORT_LIST_CAPACITY 1024

static inline int
bridge_port_list_contains(const int *ifindices, unsigned int count, int ifindex)
{
	unsigned int i;

	if (!ifindices || ifindex <= 0)
		return 0;

	for (i = 0; i < count; ++i) {
		if (ifindices[i] == ifindex)
			return 1;
	}
	return 0;
}

static inline int
bridge_cleanup_should_set_interface_down(int do_if_down, int interface_is_up)
{
	return do_if_down && interface_is_up;
}

static inline int
bridge_cleanup_status_update(int current_status, int operation_status)
{
	return (operation_status < 0) ? -1 : current_status;
}

#endif
