#ifndef PADAVAN_FIREWALL_STARTUP_POLICY_H
#define PADAVAN_FIREWALL_STARTUP_POLICY_H

/*
 * Do not start WAN with a required but unapplied IPv4 NAT baseline. Filtering
 * is a separate prerequisite: when enabled, every active IP family must
 * have its restrictive default INPUT/FORWARD policy installed. When a
 * feature is explicitly disabled, its status is not a startup dependency.
 */
static int
firewall_startup_policy_allows_wan(int firewall_enabled,
				   int ipv4_filter_status,
				   int ipv6_filter_required,
				   int ipv6_filter_status,
				   int nat_required,
				   int nat_restore_status)
{
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
