#include <assert.h>
#include <errno.h>
#include "../firewall_startup_policy.h"
#include "../service_start_status.h"
#include "../init_router_result_policy.h"

static void
test_firewall_failure_degrades_router_readiness(void)
{
	int status;

	/* Required firewall/NAT operations report their original failure. */
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 1, EIO, 0, 0, 0, 0, 0, 0, 0);
	assert(status == EIO);
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 1, 0, 1, EIO, 0, 0, 0, 0, 0);
	assert(status == EIO);
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 0, -1, 0, -1, 1, EIO, 0, 0, 0);
	assert(status == EIO);
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 0, -1, 0, -1, 0, -1, EBUSY, 0, 0);
	assert(status == EBUSY);
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 0, -1, 0, -1, 0, -1, 0, 1, EIO);
	assert(status == EIO);

	/* Unreadable selectors take precedence and preserve the first status. */
	status = firewall_startup_policy_wan_failure_status(
		EBUSY, EIO, 0, -1, 0, -1, 0, -1, EIO, 0, 0);
	assert(status == EBUSY);

	/* Irrelevant failed baselines remain compatible with disabled features. */
	assert(firewall_startup_policy_wan_failure_status(
		0, 0, 0, -1, 1, EIO, 0, EIO, 0, 0, 0) == 0);

	/* A required failure maps to DEGRADED and never overwrites an earlier error. */
	status = firewall_startup_policy_wan_failure_status(
		0, 0, 1, EIO, 0, 0, 0, 0, 0, 0, 0);
	assert(status == EIO);
	assert(init_router_result_from_boot_status(
		rc_first_service_start_status(0, status)) == INIT_ROUTER_RESULT_DEGRADED);
	assert(rc_first_service_start_status(EBUSY, status) == EBUSY);
}

static void
test_switch_vlan_isolation_is_always_required(void)
{
	/* Disabled firewall/NAT settings cannot bypass switch isolation. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, EIO, 0, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, -1, 0, 0));

	/* A VLAN failure also blocks WAN when NAT and firewall rules succeeded. */
	assert(!firewall_startup_policy_allows_wan(1, 0, 0, 0, 1, 0, EIO, 0, 0));
}

static void
test_disabled_ipv6_requires_verified_global_disable(void)
{
	/* Configured-disabled IPv6 must be verified even with firewall/NAT off. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 1, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 1, EIO));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 1, -1));
	/* Enabled IPv6 remains governed by the existing firewall-filter gate. */
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 0, -1, 0, 0, EIO));
}

static void
test_unreadable_firewall_enable_state_blocks_wan(void)
{
	/* A failed checked NVRAM read must never collapse to "firewall disabled". */
	assert(!firewall_startup_policy_allows_wan_checked(
		EIO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan_checked(
		0, EIO, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan_checked(
		EIO, EIO, 0, 0, 0, 0, 0, 0, 0, 0, 0));

	/* Successful reads retain explicit disabled-feature compatibility. */
	assert(firewall_startup_policy_allows_wan_checked(
		0, 0, 0, -1, 0, -1, 0, -1, 0, 0, 0));
}

int
main(void)
{
	/* An explicitly disabled firewall does not require default filter rules. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 0, 0));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 0, -1, 0, 0, 0));

	/* Enabled NAT is always a WAN-start prerequisite, even without filtering. */
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, -1, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(0, -1, 0, -1, 1, 1, 0, 0, 0));
	assert(firewall_startup_policy_allows_wan(0, -1, 1, -1, 1, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 0, -1, 1, -1, 0, 0, 0));

	/* A disabled NAT feature may tolerate failure to install its empty baseline. */
	assert(firewall_startup_policy_allows_wan(0, -1, 0, -1, 0, -1, 0, 0, 0));
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, -1, 0, 0, 0));

	/* Enabled firewall still requires IPv4 and each active IPv6 filter. */
	assert(firewall_startup_policy_allows_wan(1, 0, 0, -1, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 0, 0, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, 0, 1, -1, 0, 0, 0, 0, 0));
	assert(firewall_startup_policy_allows_wan(1, 0, 1, 0, 0, 0, 0, 0, 0));
	assert(!firewall_startup_policy_allows_wan(1, -1, 1, 0, 0, 0, 0, 0, 0));

	test_unreadable_firewall_enable_state_blocks_wan();
	test_firewall_failure_degrades_router_readiness();
	test_switch_vlan_isolation_is_always_required();
	test_disabled_ipv6_requires_verified_global_disable();
	return 0;
}
