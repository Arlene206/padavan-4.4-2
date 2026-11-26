#ifndef PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H
#define PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H

#include <errno.h>

/* A resolver publication or DNS reload failure must not tear down an
 * otherwise healthy WAN, PPP or VPN link. Failed publication preserves the
 * old file; a failed reload is reported after the new file is published. */
#define WAN_RESOLVER_REFRESH_COMPLETE 0
#define WAN_RESOLVER_REFRESH_KEEP_WAN 1

static inline int
wan_resolver_refresh_policy_action(int refresh_status)
{
	return (refresh_status == 0) ? WAN_RESOLVER_REFRESH_COMPLETE :
		WAN_RESOLVER_REFRESH_KEEP_WAN;
}

/* A published resolv.conf is not a complete refresh when the DNSMasq
 * upstream snapshot is unsafe and the cache signal must be suppressed. */
/* VPN replace mode must suppress every WAN-family upstream, but only
 * when the VPN supplied at least one usable DNS server. Empty/invalid VPN
 * lists retain the legacy WAN fallback instead of publishing no upstream. */
static inline int
wan_resolver_refresh_policy_replaces_wan_dns(int vpn_dns_mode,
					     int valid_vpn_dns_count)
{
	return vpn_dns_mode == 2 && valid_vpn_dns_count > 0;
}


static inline int
wan_resolver_refresh_policy_blocked_status(int dns_servers_ready,
					   int validation_status)
{
	if (dns_servers_ready)
		return 0;
	return validation_status != 0 ? validation_status : EIO;
}

#endif /* PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H */
