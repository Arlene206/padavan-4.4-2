#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "lan_dhcp_callback_policy.h"

static void
test_successful_lease_commit_ignores_ancillary_hosts_failure(void)
{
	assert(lan_dhcp_bound_callback_status(0, 0) == 0);
	assert(lan_dhcp_bound_callback_status(0, EIO) == 0);
	assert(lan_dhcp_bound_callback_status(0, ENOSPC) == 0);
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
	test_successful_lease_commit_ignores_ancillary_hosts_failure();
	test_transaction_failure_remains_fatal();
	puts("LAN DHCP post-commit ancillary status regression: PASS");
	return 0;
}
