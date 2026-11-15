#ifndef PADAVAN_SSHD_DAEMON_POLICY_H
#define PADAVAN_SSHD_DAEMON_POLICY_H

#include "process_inventory.h"

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

#endif
