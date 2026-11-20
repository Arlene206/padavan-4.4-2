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
