#ifndef XL2TPD_START_POLICY_H
#define XL2TPD_START_POLICY_H
#include <process_inventory.h>
/* 1 = running, 0 = absent, -1 = unknown (fail closed). */
static inline int
xl2tpd_start_policy_result(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 1;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return -1;
}

/*
 * Convert the launch result without confusing a positive child exit status
 * with the "already running" sentinel returned by xl2tpd_start_policy_result.
 */
static inline int
xl2tpd_start_policy_completion_result(int presence_status, int command_status)
{
	if (command_status != 0)
		return -1;
	if (presence_status == 1 || presence_status == 0)
		return presence_status;
	return -1;
}

/* A failed RPL2TP session must roll back the daemon launched for it. */
static inline int
xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(int session_status)
{
	return session_status != 0;
}

/* Preserve the primary session failure while recording daemon rollback failure. */
static inline int
xl2tpd_start_policy_rpl2tp_failure_status(int session_status, int stop_status)
{
	return session_status != 0 ? session_status : stop_status;
}

/* WAN PPP treats an already-running shared daemon as a successful start. */
static inline int
xl2tpd_start_policy_wan_launch_result(int start_status)
{
	if (start_status == 0 || start_status == 1)
		return 0;
	return -1;
}
#endif /* XL2TPD_START_POLICY_H */
