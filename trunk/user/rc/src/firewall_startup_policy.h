#ifndef PADAVAN_FIREWALL_STARTUP_POLICY_H
#define PADAVAN_FIREWALL_STARTUP_POLICY_H

/*
 * A router with filtering enabled must not bring up WAN until every active
 * IP family has its restrictive default INPUT/FORWARD policy installed.
 * When filtering is explicitly disabled, preserve the configured behavior.
 */
static int
firewall_startup_policy_allows_wan(int firewall_enabled,
				   int ipv4_filter_status,
				   int ipv6_filter_required,
				   int ipv6_filter_status)
{
	if (!firewall_enabled)
		return 1;

	if (ipv4_filter_status != 0)
		return 0;

	if (ipv6_filter_required && ipv6_filter_status != 0)
		return 0;

	return 1;
}

#endif
