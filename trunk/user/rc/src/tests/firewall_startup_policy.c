#include <assert.h>
#include "../firewall_startup_policy.h"

int
main(void)
{
	/* An explicitly disabled firewall does not require default filter rules. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 0, -1));

	/* Enabled NAT is always a WAN-start prerequisite, even without filtering. */
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, -1));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, 1));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 1, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 0, -1, 1, -1));

	/* A disabled NAT feature may tolerate failure to install its empty baseline. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1));
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, -1));

	/* Enabled firewall still requires IPv4 and each active IPv6 filter. */
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 1, -1, 0, 0));
	assert(firewall_startup_policy_allows_wan(1, 0, 1, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 1, 0, 0, 0));
	return 0;
}
