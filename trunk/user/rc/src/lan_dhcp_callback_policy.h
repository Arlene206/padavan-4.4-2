#ifndef PADAVAN_LAN_DHCP_CALLBACK_POLICY_H
#define PADAVAN_LAN_DHCP_CALLBACK_POLICY_H

/*
 * BusyBox commits its internal DHCP lease only when the bound/renew script
 * exits successfully. The address, route, resolver and NVRAM transaction
 * has already committed before ancillary host, watchdog, Internet-monitor,
 * IAPP and networkmap refreshes run. Failures of those optional follow-up
 * operations are logged by the caller, but must not make BusyBox retain an old
 * lease while the router uses the new one.
 */
/* Refresh AP host files on every RENEW. This periodic fallback guarantees a
 * later retry even when the NVRAM marker and PID 1 queue are unavailable. */
static inline int
lan_dhcp_hosts_refresh_required(int lease_changed, int is_renew,
				int is_ap_mode)
{
	return lease_changed || !is_renew || is_ap_mode;
}

/* Re-scan on initial BOUND and whenever RENEW changes the LAN subnet. */
static inline int
lan_dhcp_networkmap_refresh_required(int address_changed, int is_renew)
{
	return address_changed || !is_renew;
}

static inline int
lan_dhcp_bound_callback_status(int transaction_status, int ancillary_status)
{
	(void)ancillary_status;
	return transaction_status;
}

#endif /* PADAVAN_LAN_DHCP_CALLBACK_POLICY_H */
