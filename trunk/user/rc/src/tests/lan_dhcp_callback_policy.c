#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "lan_dhcp_callback_policy.h"

static void
test_successful_lease_commit_ignores_ancillary_refresh_failure(void)
{
	const int ancillary_failures[] = { EIO, ENOSPC, ETIMEDOUT, ECHILD };
	size_t i;

	assert(lan_dhcp_bound_callback_status(0, 0) == 0);
	for (i = 0; i < sizeof(ancillary_failures) / sizeof(ancillary_failures[0]); i++)
		assert(lan_dhcp_bound_callback_status(0, ancillary_failures[i]) == 0);
}

static void
test_hosts_refresh_retries_only_a_pending_same_value_renew(void)
{
	assert(lan_dhcp_hosts_refresh_required(1, 1, 0) == 1);
	assert(lan_dhcp_hosts_refresh_required(0, 0, 0) == 1);
	assert(lan_dhcp_hosts_refresh_required(0, 1, 0) == 0);
	assert(lan_dhcp_hosts_refresh_required(0, 1, 1) == 1);
	assert(lan_dhcp_hosts_refresh_required(1, 1, 1) == 1);
}

static void
test_transaction_failure_remains_fatal(void)
{
	assert(lan_dhcp_bound_callback_status(EIO, 0) == EIO);
	assert(lan_dhcp_bound_callback_status(ENOSPC, EIO) == ENOSPC);
}

int
main(void)
{
	test_successful_lease_commit_ignores_ancillary_refresh_failure();
	test_hosts_refresh_retries_only_a_pending_same_value_renew();
	test_transaction_failure_remains_fatal();
	puts("LAN DHCP post-commit ancillary status and retry regression: PASS");
	return 0;
}
