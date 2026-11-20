#ifndef PADAVAN_LAN_DHCP_CALLBACK_POLICY_H
#define PADAVAN_LAN_DHCP_CALLBACK_POLICY_H

#include "dhcp_lease_policy.h"

/* The static LAN identity and DHCP-only options form one deconfig commit. */
#define LAN_DHCP_DECONFIG_METADATA_COUNT 7

static inline void
lan_dhcp_deconfig_metadata_prepare(
	struct dhcp_lease_nvram_value values[LAN_DHCP_DECONFIG_METADATA_COUNT],
	const char *address, const char *netmask, const char *gateway,
	const char *domain, const char *previous_address,
	const char *previous_netmask, const char *previous_gateway,
	const char *previous_domain, const char *previous_dns,
	const char *previous_wins, const char *previous_lease)
{
	values[0].name = "lan_ipaddr_t";
	values[0].value = address;
	values[0].previous_value = previous_address;
	values[1].name = "lan_netmask_t";
	values[1].value = netmask;
	values[1].previous_value = previous_netmask;
	values[2].name = "lan_gateway_t";
	values[2].value = gateway;
	values[2].previous_value = previous_gateway;
	values[3].name = "lan_domain_t";
	values[3].value = domain;
	values[3].previous_value = previous_domain;
	values[4].name = "lan_dns_t";
	values[4].value = "";
	values[4].previous_value = previous_dns;
	values[5].name = "lan_wins_t";
	values[5].value = "";
	values[5].previous_value = previous_wins;
	values[6].name = "lan_lease_t";
	values[6].value = "";
	values[6].previous_value = previous_lease;
}

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
