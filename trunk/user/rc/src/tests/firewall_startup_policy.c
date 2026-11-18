#include <assert.h>
#include <errno.h>
#include "../firewall_startup_policy.h"

static void
test_switch_vlan_isolation_is_always_required(void)
{
	/* Disabled firewall/NAT settings cannot bypass switch isolation. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, EIO));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, -1));

	/* A VLAN failure also blocks WAN when NAT and firewall rules succeeded. */
	assert(!firewall_startup_policy_allows_wan(1, 0, 0, 0, 1, 0, EIO));
}

int
main(void)
{
	/* An explicitly disabled firewall does not require default filter rules. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 0, -1, 0));

	/* Enabled NAT is always a WAN-start prerequisite, even without filtering. */
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, -1, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, 1, 0));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 1, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 0, -1, 1, -1, 0));

	/* A disabled NAT feature may tolerate failure to install its empty baseline. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0));
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, -1, 0));

	/* Enabled firewall still requires IPv4 and each active IPv6 filter. */
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 1, -1, 0, 0, 0));
	assert(firewall_startup_policy_allows_wan(1, 0, 1, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 1, 0, 0, 0, 0));

	test_switch_vlan_isolation_is_always_required();
	return 0;
}
