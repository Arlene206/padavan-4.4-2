#include <assert.h>
#include "../firewall_startup_policy.h"

int
main(void)
{
	/* Explicitly disabled firewall must not block WAN startup. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1));

	/* Enabled firewall requires the IPv4 default filter to be installed. */
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1));
	assert(!firewall_startup_policy_allows_wan(1, -1, 0, 0));

	/* A compiled but disabled IPv6 stack is not a startup dependency. */
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1));

	/* An active IPv6 stack requires its own successful default filter. */
	assert(!firewall_startup_policy_allows_wan(1, 0, 1, -1));
	assert(firewall_startup_policy_allows_wan(1, 0, 1, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 1, 0));
	return 0;
}
