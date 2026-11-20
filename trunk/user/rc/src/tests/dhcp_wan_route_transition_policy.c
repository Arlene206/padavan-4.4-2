#include <assert.h>
#include <errno.h>

#include "dhcp_wan_route_transition_policy.h"

static void
test_lease_error_forces_renew_rebind(void)
{
	assert(!dhcp_wan_bound_retry_required(0, 0));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, 0));
	assert(dhcp_wan_bound_retry_required(0, EIO));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, EIO, 0));
	assert(dhcp_wan_bound_retry_required(0, 0, EIO));
	assert(dhcp_wan_bound_retry_required(0, EIO, EBUSY));
}

int
main(void)
{
	test_lease_error_forces_renew_rebind();
	return 0;
}
