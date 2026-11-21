#include <assert.h>
#include <errno.h>

#include "dhcp_wan_route_transition_policy.h"

static void
test_lease_error_forces_renew_rebind(void)
{
	assert(!dhcp_wan_bound_retry_required(0, 0, 0));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, 0, 0));
	assert(dhcp_wan_bound_retry_required(0, EIO, 0));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, EIO, 0));
	assert(dhcp_wan_bound_retry_required(0, 0, EIO));
	assert(dhcp_wan_bound_retry_required(0, EIO, EBUSY));
}

static void
test_retry_decision_fails_closed_on_each_status_read_error(void)
{
	int retry_required = 1;

	assert(dhcp_wan_bound_retry_decide(EACCES, 0, 0, 0, 0, 0,
					   &retry_required) == EACCES);
	assert(retry_required == 0);
	retry_required = 1;
	assert(dhcp_wan_bound_retry_decide(0, EIO, 0, 0, 0, 0,
					   &retry_required) == EIO);
	assert(retry_required == 0);
	retry_required = 1;
	assert(dhcp_wan_bound_retry_decide(0, 0, ENOENT, 0, 0, 0,
					   &retry_required) == ENOENT);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(EACCES, EIO, ENOENT, 0, 0, 0,
					   &retry_required) == EACCES);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, 0, NULL) == EINVAL);
}

static void
test_retry_decision_requires_a_complete_checked_tuple(void)
{
	int retry_required = -1;

	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, 0,
					   &retry_required) == 0);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, EADDRINUSE, 0, 0,
					   &retry_required) == 0);
	assert(retry_required == 1);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, EIO, 0,
					   &retry_required) == 0);
	assert(retry_required == 1);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, EBUSY,
					   &retry_required) == 0);
	assert(retry_required == 1);
}

int
main(void)
{
	test_lease_error_forces_renew_rebind();
	test_retry_decision_fails_closed_on_each_status_read_error();
	test_retry_decision_requires_a_complete_checked_tuple();
	return 0;
}
