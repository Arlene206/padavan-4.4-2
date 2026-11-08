#ifndef PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H
#define PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H

/* A resolver publication failure must not tear down an otherwise healthy
 * WAN, PPP or VPN link. update_resolvconf() already preserves the old file. */
#define WAN_RESOLVER_REFRESH_COMPLETE 0
#define WAN_RESOLVER_REFRESH_KEEP_WAN 1

static inline int
wan_resolver_refresh_policy_action(int refresh_status)
{
	return (refresh_status == 0) ? WAN_RESOLVER_REFRESH_COMPLETE :
		WAN_RESOLVER_REFRESH_KEEP_WAN;
}

#endif /* PADAVAN_WAN_RESOLVER_REFRESH_POLICY_H */
