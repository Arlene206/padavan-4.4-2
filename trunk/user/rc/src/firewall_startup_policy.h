#ifndef PADAVAN_FIREWALL_STARTUP_POLICY_H
#define PADAVAN_FIREWALL_STARTUP_POLICY_H

/*
 * Do not start WAN unless required switch VLAN isolation is configured.
 * If IPv6 is explicitly disabled, its global disable sysctls must also have
 * completed successfully. NAT and enabled-firewall filters are further gates;
 * disabling firewall/NAT never bypasses switch or disabled-IPv6 prerequisites.
 */
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
	if (switch_vlan_status != 0)
		return 0;

	if (nat_required && nat_restore_status != 0)
		return 0;

	if (ipv6_disable_required && ipv6_disable_status != 0)
		return 0;

	if (!firewall_enabled)
		return 1;

	if (ipv4_filter_status != 0)
		return 0;

	if (ipv6_filter_required && ipv6_filter_status != 0)
		return 0;

	return 1;
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
	if (firewall_enable_read_status != 0 || nat_enable_read_status != 0)
		return 0;

	return firewall_startup_policy_allows_wan(firewall_enabled,
						  ipv4_filter_status,
						  ipv6_filter_required,
						  ipv6_filter_status,
						  nat_required,
						  nat_restore_status,
						  switch_vlan_status,
						  ipv6_disable_required,
						  ipv6_disable_status);
}

#endif
