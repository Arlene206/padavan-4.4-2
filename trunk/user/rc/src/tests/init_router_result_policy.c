#include <assert.h>
#include <errno.h>
#include "init_router_result_policy.h"

int
main(void)
{
	assert(init_router_result_from_boot_status(0) == INIT_ROUTER_RESULT_READY);
	assert(init_router_result_from_boot_status(EIO) == INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_from_boot_status(-EINTR) == INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_from_boot_status(-1) == INIT_ROUTER_RESULT_DEGRADED);

	/* Degraded startup is ready for PID 1 event and deferred-hotplug handling. */
	assert(init_router_status_is_ready(INIT_ROUTER_RESULT_READY));
	assert(init_router_status_is_ready(INIT_ROUTER_RESULT_DEGRADED));
	assert(!init_router_status_is_ready(INIT_ROUTER_RESULT_FATAL));
	assert(!init_router_status_is_ready(2));

	assert(!init_router_status_is_degraded(INIT_ROUTER_RESULT_READY));
	assert(init_router_status_is_degraded(INIT_ROUTER_RESULT_DEGRADED));
	assert(!init_router_status_is_degraded(INIT_ROUTER_RESULT_FATAL));
	assert(!init_router_status_is_degraded(2));

	/* Early sysctl failures degrade READY but never mask a fatal prerequisite. */
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_READY, 0) ==
	       INIT_ROUTER_RESULT_READY);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_READY, EIO) ==
	       INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_READY, -EIO) ==
	       INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_DEGRADED, 0) ==
	       INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_DEGRADED, EIO) ==
	       INIT_ROUTER_RESULT_DEGRADED);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_FATAL, 0) ==
	       INIT_ROUTER_RESULT_FATAL);
	assert(init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_FATAL, EIO) ==
	       INIT_ROUTER_RESULT_FATAL);
	assert(init_router_result_merge_sysctl_status(2, EIO) == 2);

	/* Stale NTP state cannot survive if either per-boot reset failed. */
	assert(init_router_ntp_state_reset_succeeded(0, 0));
	assert(!init_router_ntp_state_reset_succeeded(EIO, 0));
	assert(!init_router_ntp_state_reset_succeeded(0, EIO));
	assert(!init_router_ntp_state_reset_succeeded(-1, -1));
	return 0;
}
