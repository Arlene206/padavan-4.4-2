#ifndef PADAVAN_SSHD_DAEMON_POLICY_H
#define PADAVAN_SSHD_DAEMON_POLICY_H

#include "process_inventory.h"

/* Direct starts also require a complete inventory confirming absence. */
static inline int
sshd_start_allowed(enum process_presence process_state)
{
	return process_state == PROCESS_PRESENCE_ABSENT;
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

#endif
