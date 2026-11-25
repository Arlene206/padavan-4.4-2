#ifndef PADAVAN_FIREWALL_STARTUP_POLICY_H
#define PADAVAN_FIREWALL_STARTUP_POLICY_H

/*
 * Return the first status that must block WAN startup, or zero when the
 * configured firewall/NAT/IPv6 prerequisites are all satisfied.
 *
 * Keep this result separate from the boolean decision so PID 1 can expose a
 * blocked WAN transition as DEGRADED instead of silently reporting READY.
 */
static int
firewall_startup_policy_wan_failure_status(int firewall_enable_read_status,
                                           int nat_enable_read_status,
                                           int firewall_enabled,
                                           int ipv4_filter_status,
                                           int ipv6_filter_required,
                                           int ipv6_filter_status,
                                           int nat_required,
                                           int nat_restore_status,
                                           int switch_vlan_status,
                                           int ipv6_disable_required,
                                           int ipv6_disable_status)
{
	if (firewall_enable_read_status != 0)
		return firewall_enable_read_status;
	if (nat_enable_read_status != 0)
		return nat_enable_read_status;
	if (switch_vlan_status != 0)
		return switch_vlan_status;
	if (nat_required && nat_restore_status != 0)
		return nat_restore_status;
	if (ipv6_disable_required && ipv6_disable_status != 0)
		return ipv6_disable_status;
	if (!firewall_enabled)
		return 0;
	if (ipv4_filter_status != 0)
		return ipv4_filter_status;
	if (ipv6_filter_required && ipv6_filter_status != 0)
		return ipv6_filter_status;
	return 0;
}

/* Boolean compatibility wrapper; use the status API in startup callers. */
static int
firewall_startup_policy_allows_wan(int firewall_enabled,
                                   int ipv4_filter_status,
                                   int ipv6_filter_required,
                                   int ipv6_filter_status,
                                   int nat_required,
                                   int nat_restore_status,
                                   int switch_vlan_status,
                                   int ipv6_disable_required,
                                   int ipv6_disable_status)
{
	return firewall_startup_policy_wan_failure_status(
		0, 0, firewall_enabled, ipv4_filter_status,
		ipv6_filter_required, ipv6_filter_status, nat_required,
		nat_restore_status, switch_vlan_status,
		ipv6_disable_required, ipv6_disable_status) == 0;
}

/* An unreadable enable switch is not equivalent to a configured-disabled
 * feature: otherwise PID 1 could skip firewall prerequisites and bring WAN up. */
static int
firewall_startup_policy_allows_wan_checked(int firewall_enable_read_status,
                                           int nat_enable_read_status,
                                           int firewall_enabled,
                                           int ipv4_filter_status,
                                           int ipv6_filter_required,
                                           int ipv6_filter_status,
                                           int nat_required,
                                           int nat_restore_status,
                                           int switch_vlan_status,
                                           int ipv6_disable_required,
                                           int ipv6_disable_status)
{
	return firewall_startup_policy_wan_failure_status(
		firewall_enable_read_status, nat_enable_read_status,
		firewall_enabled, ipv4_filter_status, ipv6_filter_required,
		ipv6_filter_status, nat_required, nat_restore_status,
		switch_vlan_status, ipv6_disable_required,
		ipv6_disable_status) == 0;
}

#endif
