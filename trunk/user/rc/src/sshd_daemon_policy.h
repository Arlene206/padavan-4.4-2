#ifndef PADAVAN_SSHD_DAEMON_POLICY_H
#define PADAVAN_SSHD_DAEMON_POLICY_H

#include "process_presence_start_policy.h"

/* Direct starts also require a complete inventory confirming absence. */
static inline int
sshd_start_allowed(enum process_presence process_state)
{
	return process_state == PROCESS_PRESENCE_ABSENT;
}

/* UNKNOWN initial inventory must never trigger a name-based stop. */
static inline int
sshd_restart_before_state_allows_stop(enum process_presence process_state)
{
	return process_state == PROCESS_PRESENCE_ABSENT ||
	       process_state == PROCESS_PRESENCE_RUNNING;
}

/* Stop only when the initial inventory confirms a running daemon. */
static inline int
sshd_restart_requires_stop(enum process_presence process_state)
{
	return process_state == PROCESS_PRESENCE_RUNNING;
}

/* A detached launcher is successful only after its daemon is confirmed RUNNING. */
static inline int
sshd_start_postcondition(int launcher_status,
			 enum process_presence process_state)
{
	return process_presence_start_postcondition(launcher_status, process_state);
}

/* Restart only after both the stop command and process inventory confirm absence. */
static inline int
sshd_restart_can_start(int stop_status, enum process_presence process_state)
{
	return stop_status == 0 && process_state == PROCESS_PRESENCE_ABSENT;
}

/* An unknown before/after state requires conservative firewall reconciliation. */
static inline int
sshd_firewall_refresh_required(enum process_presence before,
			       enum process_presence after)
{
	return before == PROCESS_PRESENCE_UNKNOWN ||
	       after == PROCESS_PRESENCE_UNKNOWN || before != after;
}

/* Fail closed if public SSH remains or its postcondition is unknown after a failed firewall refresh. */
static inline int
sshd_firewall_failure_requires_stop(int firewall_status,
			    enum process_presence after_state)
{
	return firewall_status != 0 &&
	       after_state != PROCESS_PRESENCE_ABSENT;
}

/* Keep process inventory and stop targeting on the same selected daemon. */
static inline const char *
sshd_process_name_for_binaries(int dropbear_available, int openssh_available)
{
	if (dropbear_available)
		return "dropbear";
	if (openssh_available)
		return "sshd";
	return NULL;
}

#endif
