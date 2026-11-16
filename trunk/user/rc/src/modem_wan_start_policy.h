#ifndef RC_MODEM_WAN_START_POLICY_H
#define RC_MODEM_WAN_START_POLICY_H

/* Do not migrate the active WAN interface while miniupnpd is still bound. */
static inline int
modem_wan_start_migration_succeeded(int migration_status)
{
	return migration_status == 0;
}

/* Preserve the first failed transition; DHCP status matters only afterward. */
static inline int
modem_wan_start_status(int migration_status, int dhcp_status)
{
	return migration_status != 0 ? migration_status : dhcp_status;
}

#endif
