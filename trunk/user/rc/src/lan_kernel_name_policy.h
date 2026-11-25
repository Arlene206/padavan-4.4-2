/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_LAN_KERNEL_NAME_POLICY_H
#define PADAVAN_LAN_KERNEL_NAME_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Setter callbacks return zero on success or a positive errno value. */
typedef int (*lan_kernel_name_setter_t)(const char *name, size_t length);

typedef struct {
	lan_kernel_name_setter_t set_hostname;
	lan_kernel_name_setter_t set_domainname;
} lan_kernel_name_ops_t;

/*
 * The on-disk hostname/hosts pair and the running kernel name are separate
 * state. Attempt both kernel updates, but report the first failure so callers
 * can retry the complete publication without hiding a stale kernel name.
 */
static inline int
lan_kernel_names_apply(const char *hostname, const char *domainname,
		       const lan_kernel_name_ops_t *ops)
{
	int first_status = 0;
	int status;

	if (!hostname || !domainname || !ops ||
	    !ops->set_hostname || !ops->set_domainname)
		return EINVAL;

	status = ops->set_hostname(hostname, strlen(hostname));
	if (status != 0)
		first_status = status;

	status = ops->set_domainname(domainname, strlen(domainname));
	if (status != 0 && first_status == 0)
		first_status = status;

	return first_status;
}

#endif /* PADAVAN_LAN_KERNEL_NAME_POLICY_H */
