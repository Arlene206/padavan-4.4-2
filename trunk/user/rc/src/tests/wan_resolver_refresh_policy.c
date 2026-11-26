#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "wan_resolver_refresh_policy.h"

int
main(void)
{
	assert(wan_resolver_refresh_policy_action(0) ==
	       WAN_RESOLVER_REFRESH_COMPLETE);
	assert(wan_resolver_refresh_policy_action(EIO) ==
	       WAN_RESOLVER_REFRESH_KEEP_WAN);
	assert(wan_resolver_refresh_policy_action(ENOSPC) ==
	       WAN_RESOLVER_REFRESH_KEEP_WAN);
	assert(wan_resolver_refresh_policy_action(-1) ==
	       WAN_RESOLVER_REFRESH_KEEP_WAN);
	/* DNSMasq returns a wait status when the cache reload command fails. */
	assert(wan_resolver_refresh_policy_action(256) ==
	       WAN_RESOLVER_REFRESH_KEEP_WAN);

	/* Replace-current mode excludes both WAN DNS families only when the VPN
	 * provided a usable upstream; otherwise keep the legacy WAN fallback. */
	assert(!wan_resolver_refresh_policy_replaces_wan_dns(0, 1));
	assert(!wan_resolver_refresh_policy_replaces_wan_dns(1, 1));
	assert(!wan_resolver_refresh_policy_replaces_wan_dns(2, 0));
	assert(!wan_resolver_refresh_policy_replaces_wan_dns(2, -1));
	assert(wan_resolver_refresh_policy_replaces_wan_dns(2, 1));
	assert(wan_resolver_refresh_policy_replaces_wan_dns(2, 2));
	assert(!wan_resolver_refresh_policy_replaces_wan_dns(3, 1));

	/* Unsafe/missing upstream input must never report a successful refresh. */
	assert(wan_resolver_refresh_policy_blocked_status(1, 0) == 0);
	assert(wan_resolver_refresh_policy_blocked_status(1, EPERM) == 0);
	assert(wan_resolver_refresh_policy_blocked_status(0, EPERM) == EPERM);
	assert(wan_resolver_refresh_policy_blocked_status(0, EACCES) == EACCES);
	assert(wan_resolver_refresh_policy_blocked_status(0, 0) == EIO);

	puts("WAN resolver publication/reload failure preserves active WAN/PPP state: PASS");
	return 0;
}
