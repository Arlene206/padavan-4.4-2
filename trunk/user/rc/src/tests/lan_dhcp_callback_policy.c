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
test_watchdog_and_monitor_notification_failures_are_ancillary(void)
{
	/* The lease tuple and network transaction have committed before these
	 * notifications. Their failures must be logged, not returned to BusyBox. */
	assert(lan_dhcp_bound_callback_status(0, EIO) == 0);
	assert(lan_dhcp_bound_callback_status(0, ETIMEDOUT) == 0);
	assert(lan_dhcp_bound_callback_status(0, ENOBUFS) == 0);
	assert(lan_dhcp_bound_callback_status(EIO, ETIMEDOUT) == EIO);
}

static void
test_ap_same_value_renew_always_refreshes_hosts(void)
{
	assert(lan_dhcp_hosts_refresh_required(1, 1, 0) == 1);
	assert(lan_dhcp_hosts_refresh_required(0, 0, 0) == 1);
	assert(lan_dhcp_hosts_refresh_required(0, 1, 0) == 0);
	/* AP renew is the durable fallback when the immediate queue cannot retry. */
	assert(lan_dhcp_hosts_refresh_required(0, 1, 1) == 1);
	assert(lan_dhcp_hosts_refresh_required(1, 1, 1) == 1);
}

static void
test_networkmap_refreshes_changed_address_renewals(void)
{
	assert(lan_dhcp_networkmap_refresh_required(0, 0) == 1);
	assert(lan_dhcp_networkmap_refresh_required(0, 1) == 0);
	assert(lan_dhcp_networkmap_refresh_required(1, 1) == 1);
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
	test_watchdog_and_monitor_notification_failures_are_ancillary();
	test_ap_same_value_renew_always_refreshes_hosts();
	test_networkmap_refreshes_changed_address_renewals();
	test_transaction_failure_remains_fatal();
	puts("LAN DHCP networkmap refresh and ancillary callback policy: PASS");
	return 0;
}
