#ifndef RC_NETWORK_SYSCTL_POLICY_H
#define RC_NETWORK_SYSCTL_POLICY_H
#include <errno.h>

/* Attempt every independent setting and preserve the first failure. */
typedef int (*rc_network_sysctl_operation_fn)(void *context, unsigned int operation);
static inline int
rc_network_sysctl_apply(unsigned int operation_count,
			rc_network_sysctl_operation_fn operation, void *context)
{
	unsigned int i;
	int first_status = 0;
	if (operation_count == 0)
		return 0;
	if (!operation)
		return EINVAL;
	for (i = 0; i < operation_count; ++i) {
		int status = operation(context, i);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	return first_status;
}

/* An explicitly enabled SYN-cookie setting is a WAN admission prerequisite. */
static inline int
rc_network_sysctl_tcp_syncookies_allows_wan(int syncookies_required,
					     int syncookies_write_status)
{
	return !syncookies_required || syncookies_write_status == 0;
}

#endif /* RC_NETWORK_SYSCTL_POLICY_H */
