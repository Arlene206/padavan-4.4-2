#ifndef WAN6_TEARDOWN_POLICY_H
#define WAN6_TEARDOWN_POLICY_H

/*
 * Select the interface that actually owns the IPv6 state being torn down.
 * The saved runtime interface is authoritative while it still exists because
 * NVRAM may already describe the next configuration. A disabled configuration
 * with no surviving saved interface has no interface state to flush; a live
 * SIT device is still cleaned as a defensive fallback.
 */
static inline char *
wan6_teardown_select_ifname(char *previous_ifname,
			    int previous_if_exists,
			    int ipv6_disabled,
			    int ipv6_sit,
			    int sit_if_exists,
			    char *wan_ifname,
			    char *sit_ifname)
{
	if (previous_ifname && previous_ifname[0] && previous_if_exists)
		return previous_ifname;

	if (ipv6_disabled)
		return sit_if_exists ? sit_ifname : (char *)0;

	if (ipv6_sit)
		return sit_ifname;

	return wan_ifname;
}

#endif /* WAN6_TEARDOWN_POLICY_H */
