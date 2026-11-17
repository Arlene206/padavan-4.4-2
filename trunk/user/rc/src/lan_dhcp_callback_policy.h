#ifndef PADAVAN_LAN_DHCP_CALLBACK_POLICY_H
#define PADAVAN_LAN_DHCP_CALLBACK_POLICY_H

/*
 * BusyBox commits its internal DHCP lease only when the bound/renew script
 * exits successfully. The address, route, resolver and NVRAM transaction
 * has already committed before hostname-file publication runs. An ancillary
 * hosts-file failure is logged, but must not make BusyBox retain an old lease
 * while the router is already using the new one.
 */
/* Retry ancillary host publication after a same-value RENEW only when an
 * earlier publication failed; do not repeat it for every healthy renewal. */
static inline int
lan_dhcp_hosts_refresh_required(int lease_changed, int is_renew,
				int retry_pending)
{
	return lease_changed || !is_renew || retry_pending;
}


static inline int
lan_dhcp_bound_callback_status(int transaction_status, int ancillary_status)
{
	(void)ancillary_status;
	return transaction_status;
}

#endif /* PADAVAN_LAN_DHCP_CALLBACK_POLICY_H */
