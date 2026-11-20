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
	return 0;
}
