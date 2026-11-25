#ifndef PADAVAN_LAN_UDHCPC_HOSTNAME_POLICY_H
#define PADAVAN_LAN_UDHCPC_HOSTNAME_POLICY_H
#include <errno.h>
static inline int
lan_udhcpc_hostname_status(const char *hostname, int lookup_status,
                           int hostname_is_valid)
{
	if (lookup_status != 0)
		return lookup_status;
	if (!hostname || !hostname[0] || !hostname_is_valid)
		return EINVAL;
	return 0;
}
#endif
