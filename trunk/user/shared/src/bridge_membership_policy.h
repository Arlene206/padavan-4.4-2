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
bridge_port_list_is_empty(const int *ifindices, unsigned int count)
{
	unsigned int i;

	if (!ifindices)
		return -1;

	for (i = 0; i < count; ++i) {
		if (ifindices[i])
			return 0;
	}
	return 1;
}

static inline int
bridge_batch_membership_detached_safe(int attach_attempted, int membership_added,
				      int detach_succeeded)
{
	if (!attach_attempted)
		return 1;

	return membership_added && detach_succeeded;
}

static inline int
bridge_batch_can_remove_created_bridge(int created_by_call, int empty_snapshot)
{
	return created_by_call && empty_snapshot == 1;
}

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

static inline int
bridge_membership_operation_changed(int is_add, int membership_before,
				    int operation_status)
{
	if (operation_status != 0 ||
	    (membership_before != 0 && membership_before != 1))
		return 0;

	return is_add ? (membership_before == 0) : (membership_before == 1);
}

/*
 * Linux 4.4 reports EBUSY for a duplicate add and EINVAL when a delete
 * races with another detach or the port is not owned by this bridge.
 */
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

static inline int
bridge_rollback_can_remove_created_vlan(int created_by_call,
					int membership_known_detached)
{
	return created_by_call && membership_known_detached;
}

static inline int
bridge_rollback_can_restore_interface(int existed_before, int state_is_known)
{
	return existed_before && state_is_known;
}


typedef struct {
	int (*get_param)(const char *, const char *, int *);
	int (*set_param)(const char *, const char *, int);
} bridge_port_m2u_ops_t;

/*
 * multicast_fast_leave and multicast_to_unicast form one logical setting.
 * Snapshot both values before writing either; on a failed write, restore all
 * attempted writes in reverse order while preserving the primary errno.
 */
static inline int
bridge_port_m2u_apply_with_ops(const char *ifname, int enabled,
			       const bridge_port_m2u_ops_t *ops,
			       int *rollback_status, int *rollback_errno)
{
	int old_fast_leave, old_to_unicast;
	int attempted = 0, ret, primary_errno;
	int first_rollback_status = 0, first_rollback_errno = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (rollback_errno)
		*rollback_errno = 0;
	if (!ifname || !ifname[0] || (enabled != 0 && enabled != 1) ||
	    !ops || !ops->get_param || !ops->set_param) {
		errno = EINVAL;
		return -1;
	}

	ret = ops->get_param(ifname, "multicast_fast_leave", &old_fast_leave);
	if (ret != 0)
		return ret;
	ret = ops->get_param(ifname, "multicast_to_unicast", &old_to_unicast);
	if (ret != 0)
		return ret;

	if ((old_fast_leave != 0 && old_fast_leave != 1) ||
	    (old_to_unicast != 0 && old_to_unicast != 1)) {
		errno = EPROTO;
		return -1;
	}

	if (old_fast_leave != enabled) {
		attempted |= 1;
		ret = ops->set_param(ifname, "multicast_fast_leave", enabled);
		if (ret != 0)
			goto rollback;
	}
	if (old_to_unicast != enabled) {
		attempted |= 2;
		ret = ops->set_param(ifname, "multicast_to_unicast", enabled);
		if (ret != 0)
			goto rollback;
	}
	return 0;

rollback:
	primary_errno = errno ? errno : EIO;
	if (attempted & 2) {
		int restore_status = ops->set_param(
			ifname, "multicast_to_unicast", old_to_unicast);
		if (restore_status != 0 && first_rollback_status == 0) {
			first_rollback_status = restore_status;
			first_rollback_errno = errno ? errno : EIO;
		}
	}
	if (attempted & 1) {
		int restore_status = ops->set_param(
			ifname, "multicast_fast_leave", old_fast_leave);
		if (restore_status != 0 && first_rollback_status == 0) {
			first_rollback_status = restore_status;
			first_rollback_errno = errno ? errno : EIO;
		}
	}
	if (rollback_status)
		*rollback_status = first_rollback_status;
	if (rollback_errno)
		*rollback_errno = first_rollback_errno;
	errno = primary_errno;
	return ret;
}

#endif
