#ifndef PADAVAN_FIREWALL_MANGLE_TTL_POLICY_H
#define PADAVAN_FIREWALL_MANGLE_TTL_POLICY_H

#include <errno.h>

/* A configured TTL write is required; preserve positive errno-style results. */
static int
firewall_mangle_ttl_write_status(int configured_value, int write_status,
				 int write_errno)
{
	if (configured_value <= 1 || write_status == 0)
		return 0;
	if (write_status > 0)
		return write_status;
	return write_errno > 0 ? write_errno : EIO;
}

#endif /* PADAVAN_FIREWALL_MANGLE_TTL_POLICY_H */
