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


typedef struct {
	int (*set_admin_up)(const char *, int);
	int (*set_mtu)(const char *, int);
	int (*set_hwaddr)(const char *, const char *);
} vlan_interface_restore_ops_t;

static inline void
vlan_interface_restore_record(int status, int *first_status, int *failure_count)
{
	if (status == 0)
		return;
	if (*first_status == 0)
		*first_status = status;
	(*failure_count)++;
}

/* Roll back a reused VLAN from a complete snapshot taken before mutation. */
static inline int
vlan_interface_restore_existing(const vlan_interface_restore_ops_t *ops,
				 const char *ifname, int was_up,
				 int restore_mtu, int original_mtu,
				 int restore_hwaddr, const char *original_hwaddr,
				 int *failure_count)
{
	int first_status = 0;
	int failures = 0;
	int status;

	if (failure_count)
		*failure_count = 0;
	if (!ops || !ops->set_admin_up || !ops->set_mtu ||
	    !ops->set_hwaddr ||
	    !ifname || !ifname[0] || (restore_hwaddr &&
	    (!original_hwaddr || !original_hwaddr[0])))
		return EINVAL;

	status = ops->set_admin_up(ifname, 0);
	vlan_interface_restore_record(status, &first_status, &failures);
	if (restore_mtu) {
		status = ops->set_mtu(ifname, original_mtu);
		vlan_interface_restore_record(status, &first_status, &failures);
	}
	if (restore_hwaddr) {
		status = ops->set_hwaddr(ifname, original_hwaddr);
		vlan_interface_restore_record(status, &first_status, &failures);
	}
	status = ops->set_admin_up(ifname, was_up ? 1 : 0);
	vlan_interface_restore_record(status, &first_status, &failures);

	if (failure_count)
		*failure_count = failures;
	return first_status;
}

#endif
