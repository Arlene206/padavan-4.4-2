#ifndef PADAVAN_FIREWALL_RP_FILTER_POLICY_H
#define PADAVAN_FIREWALL_RP_FILTER_POLICY_H

#include <errno.h>

/*
 * An absent interface is tolerable while retaining the restrictive default
 * mode, but a feature requiring rp_filter=0 must not proceed without it.
 * A successful write wins over a stale interface-presence snapshot.
 */
static int
firewall_rp_filter_write_status(int interface_present, int configured_value,
				int write_status, int write_errno)
{
	if (write_status == 0)
		return 0;
	if (!interface_present)
		return configured_value == 1 ? 0 : ENODEV;
	if (write_status > 0)
		return write_status;
	return write_errno > 0 ? write_errno : EIO;
}

#endif /* PADAVAN_FIREWALL_RP_FILTER_POLICY_H */
