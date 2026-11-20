#ifndef PADAVAN_INIT_ROUTER_RESULT_POLICY_H
#define PADAVAN_INIT_ROUTER_RESULT_POLICY_H

/*
 * A router with failed optional boot services is still alive. Keep that state
 * distinct from fatal prerequisites so PID 1 can continue recovery events.
 */
enum init_router_result {
	INIT_ROUTER_RESULT_FATAL = -1,
	INIT_ROUTER_RESULT_READY = 0,
	INIT_ROUTER_RESULT_DEGRADED = 1
};

static inline int
init_router_result_from_boot_status(int boot_status)
{
	return boot_status == 0 ? INIT_ROUTER_RESULT_READY :
				  INIT_ROUTER_RESULT_DEGRADED;
}

static inline int
init_router_status_is_ready(int status)
{
	return status == INIT_ROUTER_RESULT_READY ||
	       status == INIT_ROUTER_RESULT_DEGRADED;
}

static inline int
init_router_status_is_degraded(int status)
{
	return status == INIT_ROUTER_RESULT_DEGRADED;
}

#endif

/*
 * Early sysctl errors degrade a running router but must not mask fatal gates.
 * Preserve unknown router states rather than promoting them to READY.
 */
static inline int
init_router_result_merge_sysctl_status(int router_status, int sysctl_status)
{
	if (router_status != INIT_ROUTER_RESULT_READY &&
	    router_status != INIT_ROUTER_RESULT_DEGRADED)
		return router_status;
	if (router_status == INIT_ROUTER_RESULT_DEGRADED || sysctl_status != 0)
		return INIT_ROUTER_RESULT_DEGRADED;
	return INIT_ROUTER_RESULT_READY;
}
