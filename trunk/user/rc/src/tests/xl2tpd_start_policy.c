#include <stdio.h>
#include <errno.h>
#include "xl2tpd_start_policy.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); return 1; } } while (0)
int
main(void)
{
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
