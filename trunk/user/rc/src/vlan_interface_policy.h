#ifndef RC_VLAN_INTERFACE_POLICY_H
#define RC_VLAN_INTERFACE_POLICY_H

#include <errno.h>

/* Only ENODEV/ENXIO prove that an interface is absent. */
static inline int
vlan_interface_probe_result(int ifflags, int probe_errno, int *exists)
{
	if (!exists)
		return EINVAL;

	if (ifflags >= 0) {
		*exists = 1;
		return 0;
	}

	*exists = 0;
	if (probe_errno == ENODEV || probe_errno == ENXIO)
		return 0;

	return probe_errno > 0 ? probe_errno : EIO;
}

/* Preserve the first failure while attempting both cleanup operations. */
static inline int
vlan_interface_cleanup_status(int down_status, int delete_status)
{
	return down_status != 0 ? down_status : delete_status;
}

/* An incompletely rolled-back interface remains owned by this transaction. */
static inline int
vlan_interface_retain_ownership(int created, int cleanup_status)
{
	return created && cleanup_status != 0;
}

#endif
