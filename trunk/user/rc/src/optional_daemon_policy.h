#ifndef OPTIONAL_DAEMON_POLICY_H
#define OPTIONAL_DAEMON_POLICY_H

#include "process_inventory.h"

/* Never report a successful stop or start a replacement on UNKNOWN. */
/* Only a complete inventory confirming ABSENT permits a daemon start. */
static inline int
optional_daemon_start_allowed(enum process_presence state)
{
	return state == PROCESS_PRESENCE_ABSENT;
}

static inline int
optional_daemon_failed_start_may_survive(enum process_presence after_stop)
{
	return after_stop != PROCESS_PRESENCE_ABSENT;
}

static inline int
optional_daemon_stop_complete(int command_status, enum process_presence state)
{
	if (command_status != 0)
		return command_status;
	return state == PROCESS_PRESENCE_ABSENT ? 0 : -1;
}

#endif
