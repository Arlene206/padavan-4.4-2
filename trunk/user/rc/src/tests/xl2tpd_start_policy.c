#include <stdio.h>
#include <errno.h>
#include "xl2tpd_start_policy.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); return 1; } } while (0)

static int
test_runtime_marker_write_status(void)
{
	CHECK(xl2tpd_start_policy_write_status(0, 0) == 0);
	CHECK(xl2tpd_start_policy_write_status(-1, EACCES) == EACCES);
	CHECK(xl2tpd_start_policy_write_status(-1, 0) == EIO);
	CHECK(xl2tpd_start_policy_write_status(1, 0) == EIO);
	return 0;
}

static int
test_config_flags(void)
{
	struct xl2tpd_config_flags flags;

	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 0, "1", "1", "1", "1", "1", "",
		&flags) == 0);
	CHECK(flags.use_vpn_server == 1);
	CHECK(flags.use_wan_lac == 1);
	CHECK(flags.use_vpnc == 1);
	CHECK(flags.vpnc_default_route == 2);

	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 1, "1", "0", "0", "1", "", "1",
		&flags) == 0);
	CHECK(flags.use_vpn_server == 0);
	CHECK(flags.use_wan_lac == 0);
	CHECK(flags.use_vpnc == 0);
	CHECK(flags.vpnc_default_route == 0);

	/* Missing/empty selector keys preserve nvram_match defaults. */
	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 0, "", "", "", "", "", "",
		&flags) == 0);
	CHECK(flags.use_vpn_server == 0 && flags.use_vpnc == 0);
	CHECK(flags.use_wan_lac == 1); /* non-RPL2TP builds always use WAN LAC */
	CHECK(flags.vpnc_default_route == 0);

	/* The RPL2TP transport selector preserves nvram_invmatch semantics. */
	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 1, "", "", "", "", "", "0",
		&flags) == 0);
	CHECK(flags.use_wan_lac == 1);
	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 1, "", "", "", "", "", "1",
		&flags) == 0);
	CHECK(flags.use_wan_lac == 0);

	CHECK(xl2tpd_start_policy_config_flags(
		0, 2, 3, 0, "", "", "", "", "", "",
		&flags) == 0);
	CHECK(flags.use_wan_lac == 0);

	flags.use_vpn_server = 1;
	flags.use_wan_lac = 1;
	flags.use_vpnc = 1;
	flags.vpnc_default_route = 2;
	CHECK(xl2tpd_start_policy_config_flags(
		EIO, 3, 3, 0, "1", "1", "1", "1", "1", "",
		&flags) == EIO);
	CHECK(flags.use_vpn_server == 0 && flags.use_wan_lac == 0 &&
	      flags.use_vpnc == 0 && flags.vpnc_default_route == 0);
	CHECK(xl2tpd_start_policy_config_flags(
		0, 3, 3, 0, "1", "1", "1", "1", "1", "", NULL) == EINVAL);
	return 0;
}

static int
test_vpnc_marker_rolls_back_only_after_failed_start(void)
{
	CHECK(!xl2tpd_start_policy_vpnc_marker_rollback_required(0, -1));
	CHECK(!xl2tpd_start_policy_vpnc_marker_rollback_required(1, 0));
	CHECK(!xl2tpd_start_policy_vpnc_marker_rollback_required(1, 1));
	CHECK(xl2tpd_start_policy_vpnc_marker_rollback_required(1, -1));
	return 0;
}

int
main(void)
{
	CHECK(test_runtime_marker_write_status() == 0);
	CHECK(test_config_flags() == 0);
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_RUNNING) == 1);
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_ABSENT) == 0);
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_UNKNOWN) == -1);
	CHECK(xl2tpd_start_policy_result((enum process_presence)99) == -1);
	CHECK(xl2tpd_start_policy_completion_result(1, 0) == 1);
	CHECK(xl2tpd_start_policy_completion_result(0, 0) == 0);
	CHECK(xl2tpd_start_policy_completion_result(0, 1) == -1);
	CHECK(xl2tpd_start_policy_completion_result(0, 127) == -1);
	CHECK(xl2tpd_start_policy_completion_result(1, 1) == -1);
	CHECK(xl2tpd_start_policy_completion_result(-1, 0) == -1);
	CHECK(xl2tpd_start_policy_completion_result(99, 0) == -1);
	CHECK(xl2tpd_start_policy_wan_launch_result(0) == 0);
	CHECK(xl2tpd_start_policy_wan_launch_result(1) == 0);
	CHECK(xl2tpd_start_policy_wan_launch_result(-1) == -1);
	CHECK(xl2tpd_start_policy_wan_launch_result(2) == -1);

	CHECK(!xl2tpd_start_policy_wan_marker_rollback_required(0, -1));
	CHECK(!xl2tpd_start_policy_wan_marker_rollback_required(0, 0));
	CHECK(!xl2tpd_start_policy_wan_marker_rollback_required(1, 0));
	CHECK(xl2tpd_start_policy_wan_marker_rollback_required(1, -1));
	CHECK(xl2tpd_start_policy_wan_marker_rollback_required(1, 1));
	CHECK(test_vpnc_marker_rolls_back_only_after_failed_start() == 0);
	CHECK(!xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(0));
	CHECK(xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(-1));
	CHECK(xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(1));
	CHECK(xl2tpd_start_policy_rpl2tp_failure_status(0, 0) == 0);
	CHECK(xl2tpd_start_policy_rpl2tp_failure_status(7, 0) == 7);
	CHECK(xl2tpd_start_policy_rpl2tp_failure_status(7, EBUSY) == 7);
	CHECK(xl2tpd_start_policy_rpl2tp_failure_status(0, EBUSY) == EBUSY);
	puts("xl2tpd process-presence and launch-result policy: PASS");
	return 0;
}
