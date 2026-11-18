#ifndef PADAVAN_FIREWALL_STARTUP_POLICY_H
#define PADAVAN_FIREWALL_STARTUP_POLICY_H

/*
 * Do not start WAN unless the required switch VLAN isolation was configured.
 * The IPv4 NAT baseline and restrictive firewall filters are additional
 * prerequisites. Explicitly disabled firewall/NAT features are not startup
 * dependencies, but they never bypass the switch-isolation requirement.
 */
static int
firewall_startup_policy_allows_wan(int firewall_enabled,
				   int ipv4_filter_status,
				   int ipv6_filter_required,
				   int ipv6_filter_status,
				   int nat_required,
				   int nat_restore_status,
				   int switch_vlan_status)
{
	if (switch_vlan_status != 0)
		return 0;

	if (nat_required && nat_restore_status != 0)
		return 0;

	if (!firewall_enabled)
		return 1;

	if (ipv4_filter_status != 0)
		return 0;

	if (ipv6_filter_required && ipv6_filter_status != 0)
		return 0;

	return 1;
}

#endif
