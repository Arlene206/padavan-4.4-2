#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "lan_dhcp_callback_policy.h"

static void
test_deconfig_metadata_restores_static_values_and_clears_dhcp_options(void)
{
	struct dhcp_lease_nvram_value values[LAN_DHCP_DECONFIG_METADATA_COUNT];

	lan_dhcp_deconfig_metadata_prepare(
		values, "192.0.2.10", "255.255.255.0", "192.0.2.1",
		"router.example", "198.51.100.10", "255.255.255.128",
		"198.51.100.1", "lease.example", "203.0.113.53",
		"203.0.113.54", "3600");

	assert(LAN_DHCP_DECONFIG_METADATA_COUNT == 7);
	assert(strcmp(values[0].name, "lan_ipaddr_t") == 0);
	assert(strcmp(values[0].value, "192.0.2.10") == 0);
	assert(strcmp(values[0].previous_value, "198.51.100.10") == 0);
	assert(strcmp(values[1].name, "lan_netmask_t") == 0);
	assert(strcmp(values[1].value, "255.255.255.0") == 0);
	assert(strcmp(values[1].previous_value, "255.255.255.128") == 0);
	assert(strcmp(values[2].name, "lan_gateway_t") == 0);
	assert(strcmp(values[2].value, "192.0.2.1") == 0);
	assert(strcmp(values[2].previous_value, "198.51.100.1") == 0);
	assert(strcmp(values[3].name, "lan_domain_t") == 0);
	assert(strcmp(values[3].value, "router.example") == 0);
	assert(strcmp(values[3].previous_value, "lease.example") == 0);
	assert(strcmp(values[4].name, "lan_dns_t") == 0);
	assert(strcmp(values[4].value, "") == 0);
	assert(strcmp(values[4].previous_value, "203.0.113.53") == 0);
	assert(strcmp(values[5].name, "lan_wins_t") == 0);
	assert(strcmp(values[5].value, "") == 0);
	assert(strcmp(values[5].previous_value, "203.0.113.54") == 0);
	assert(strcmp(values[6].name, "lan_lease_t") == 0);
	assert(strcmp(values[6].value, "") == 0);
	assert(strcmp(values[6].previous_value, "3600") == 0);
}

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
test_host_refresh_treats_unknown_mode_as_ap(void)
{
	int refresh_as_ap = 0;

	assert(lan_dhcp_hosts_refresh_ap_mode_resolve(0, 0, &refresh_as_ap) == 0);
	assert(refresh_as_ap == 0);
	assert(lan_dhcp_hosts_refresh_ap_mode_resolve(0, 1, &refresh_as_ap) == 0);
	assert(refresh_as_ap == 1);

	/* A read failure must preserve the AP retry opportunity. */
	refresh_as_ap = 0;
	assert(lan_dhcp_hosts_refresh_ap_mode_resolve(EIO, 0, &refresh_as_ap) == EIO);
	assert(refresh_as_ap == 1);
	refresh_as_ap = 0;
	assert(lan_dhcp_hosts_refresh_ap_mode_resolve(ENODEV, 0, &refresh_as_ap) == ENODEV);
	assert(refresh_as_ap == 1);
	assert(lan_dhcp_hosts_refresh_ap_mode_resolve(0, 1, NULL) == EINVAL);
}

static void
test_networkmap_refreshes_changed_address_renewals(void)
{
	assert(lan_dhcp_networkmap_refresh_required(0, 0) == 1);
	assert(lan_dhcp_networkmap_refresh_required(0, 1) == 0);
	assert(lan_dhcp_networkmap_refresh_required(1, 1) == 1);
}

static void
test_networkmap_pending_refresh_retries_unchanged_renew(void)
{
	assert(lan_dhcp_networkmap_request_required(0, 1, 0) == 0);
	assert(lan_dhcp_networkmap_request_required(0, 1, 1) == 1);
	assert(lan_dhcp_networkmap_request_required(1, 1, 0) == 1);
	assert(lan_dhcp_networkmap_request_required(0, 0, 0) == 1);
}

static void
test_transaction_failure_remains_fatal(void)
{
	assert(lan_dhcp_bound_callback_status(EIO, 0) == EIO);
	assert(lan_dhcp_bound_callback_status(ENOSPC, EIO) == ENOSPC);
}

static void
test_runtime_metadata_refresh_prepares_atomic_four_key_tuple(void)
{
	struct dhcp_lease_nvram_value values[LAN_DHCP_STATUS_METADATA_COUNT];

	lan_dhcp_status_metadata_prepare(
		values, "192.0.2.10", "255.255.255.0", "router.example",
		"192.0.2.1", "198.51.100.10", "255.255.255.128",
		"old.example", "198.51.100.1");

	assert(LAN_DHCP_STATUS_METADATA_COUNT == 4);
	assert(strcmp(values[0].name, "lan_ipaddr_t") == 0);
	assert(strcmp(values[0].value, "192.0.2.10") == 0);
	assert(strcmp(values[0].previous_value, "198.51.100.10") == 0);
	assert(strcmp(values[1].name, "lan_netmask_t") == 0);
	assert(strcmp(values[1].value, "255.255.255.0") == 0);
	assert(strcmp(values[1].previous_value, "255.255.255.128") == 0);
	assert(strcmp(values[2].name, "lan_domain_t") == 0);
	assert(strcmp(values[2].value, "router.example") == 0);
	assert(strcmp(values[2].previous_value, "old.example") == 0);
	assert(strcmp(values[3].name, "lan_gateway_t") == 0);
	assert(strcmp(values[3].value, "192.0.2.1") == 0);
	assert(strcmp(values[3].previous_value, "198.51.100.1") == 0);
}

static void
test_ap_lan_protocol_mode_read_failure_fails_closed(void)
{
	int use_dhcp = 0;

	assert(lan_dhcp_ap_mode_resolve(0, 1, &use_dhcp) == 0);
	assert(use_dhcp == 1);
	assert(lan_dhcp_ap_mode_resolve(0, 0, &use_dhcp) == 0);
	assert(use_dhcp == 0);

	/* A backend error must not be converted into manual/static mode. */
	use_dhcp = 1;
	assert(lan_dhcp_ap_mode_resolve(EIO, 1, &use_dhcp) == EIO);
	assert(use_dhcp == 0);
	use_dhcp = 1;
	assert(lan_dhcp_ap_mode_resolve(ENODEV, 1, &use_dhcp) == ENODEV);
	assert(use_dhcp == 0);
	assert(lan_dhcp_ap_mode_resolve(0, 1, NULL) == EINVAL);
}

int
main(void)
{
	test_deconfig_metadata_restores_static_values_and_clears_dhcp_options();
	test_ap_lan_protocol_mode_read_failure_fails_closed();
	test_host_refresh_treats_unknown_mode_as_ap();
	test_runtime_metadata_refresh_prepares_atomic_four_key_tuple();
	test_successful_lease_commit_ignores_ancillary_refresh_failure();
	test_watchdog_and_monitor_notification_failures_are_ancillary();
	test_ap_same_value_renew_always_refreshes_hosts();
	test_networkmap_refreshes_changed_address_renewals();
	test_networkmap_pending_refresh_retries_unchanged_renew();
	test_transaction_failure_remains_fatal();
	puts("LAN DHCP runtime/deconfig metadata, networkmap and ancillary callback policy: PASS");
	return 0;
}
