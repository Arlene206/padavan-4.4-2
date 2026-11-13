#ifndef _BRIDGE_MEMBERSHIP_POLICY_H
#define _BRIDGE_MEMBERSHIP_POLICY_H

#include <errno.h>
#include <string.h>

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
bridge_membership_request_satisfied(int is_add, int membership)
{
	if (membership < 0)
		return 0;

	return is_add ? (membership > 0) : (membership == 0);
}

/*
 * Linux 4.4 reports EBUSY for a duplicate add and EINVAL when a delete
 * races with another detach or the port is not owned by this bridge.
 */
static inline int
bridge_membership_operation_changed(int is_add, int membership_before,
				    int operation_status)
{
	if (operation_status != 0 ||
	    (membership_before != 0 && membership_before != 1))
		return 0;

	return is_add ? (membership_before == 0) : (membership_before == 1);
}

static inline int
bridge_membership_race_is_idempotent(int is_add, int error_number)
{
	return is_add ? (error_number == EBUSY) : (error_number == EINVAL);
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

static inline int
bridge_cleanup_should_remove_vlan(int do_if_rem, const char *ifname,
				   const char *lan_ifname)
{
	if (!do_if_rem || !ifname || !ifname[0] || !lan_ifname)
		return 0;

	return strcmp(ifname, lan_ifname) != 0 && strchr(ifname, '.') != NULL;
}

#endif
