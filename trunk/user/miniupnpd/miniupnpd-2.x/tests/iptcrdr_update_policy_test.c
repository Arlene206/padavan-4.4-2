#include <assert.h>
#include <stdio.h>

#include "../netfilter/iptcrdr_update_policy.h"

static void
test_rollback_decision(void)
{
	assert(miniupnpd_portmapping_rollback_required(-1, -1) == 0);
	assert(miniupnpd_portmapping_rollback_required(0, 0) == 0);
	assert(miniupnpd_portmapping_rollback_required(0, -1) == 1);
	assert(miniupnpd_portmapping_rollback_required(0, 1) == 1);
}

static void
test_nat_delete_rollback_decision(void)
{
	assert(miniupnpd_nat_delete_rollback_required(-1, -1) == 0);
	assert(miniupnpd_nat_delete_rollback_required(0, 0) == 0);
	assert(miniupnpd_nat_delete_rollback_required(0, -1) == 1);
	assert(miniupnpd_nat_delete_rollback_required(-1, 0) == 0);
}

static void
test_filter_delete_failure_wins_over_failed_nat_restore(void)
{
	assert(miniupnpd_nat_delete_failure_result(-1, 0) == -1);
	assert(miniupnpd_nat_delete_failure_result(-1, -1) == -1);
	assert(miniupnpd_nat_delete_failure_result(0, -1) == -1);
	assert(miniupnpd_nat_delete_failure_result(0, 0) == 0);
}

static void
test_peer_delete_rollback_decision(void)
{
	assert(miniupnpd_peer_delete_rollback_required(-1, -1) == 0);
	assert(miniupnpd_peer_delete_rollback_required(0, 0) == 0);
	assert(miniupnpd_peer_delete_rollback_required(0, -1) == 1);
	assert(miniupnpd_peer_delete_rollback_required(-1, 0) == 0);
}

static void
test_dscp_failure_wins_over_failed_peer_restore(void)
{
	assert(miniupnpd_peer_delete_failure_result(-1, 0) == -1);
	assert(miniupnpd_peer_delete_failure_result(-1, -1) == -1);
	assert(miniupnpd_peer_delete_failure_result(0, -1) == -1);
	assert(miniupnpd_peer_delete_failure_result(0, 0) == 0);
}

static void
test_primary_failure_wins(void)
{
	assert(miniupnpd_portmapping_failure_result(-1, 0) == -1);
	assert(miniupnpd_portmapping_failure_result(-1, -1) == -1);
	assert(miniupnpd_portmapping_failure_result(-2, -1) == -2);
	assert(miniupnpd_portmapping_failure_result(0, -1) == -1);
	assert(miniupnpd_portmapping_failure_result(0, 0) == 0);
}

int
main(void)
{
	test_rollback_decision();
	test_nat_delete_rollback_decision();
	test_filter_delete_failure_wins_over_failed_nat_restore();
	test_peer_delete_rollback_decision();
	test_dscp_failure_wins_over_failed_peer_restore();
	test_primary_failure_wins();
	puts("MiniUPnP NAT/filter update rollback policy: PASS");
	return 0;
}
