#ifndef CROND_DAEMON_POLICY_H
#define CROND_DAEMON_POLICY_H

#include <process_inventory.h>

#define CROND_START_INITIAL_WAIT_SECONDS 1
#define CROND_START_RETRY_WAIT_SECONDS 1
#define CROND_START_PROBE_ATTEMPTS 4

/*
 * service_start_probe_wait() reports positive errno values. Normalize them
 * to the negative failure convention used by RC restart callers.
 */
static inline int
crond_start_probe_wait_status(int wait_status)
{
	return wait_status == 0 ? 0 : -1;
}

/* Normalize launcher and detached-child postcondition failures for RC. */
static inline int
crond_start_postcondition(int launcher_status,
			  enum process_presence process_presence)
{
	if (launcher_status != 0)
		return -1;

	if (process_presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	return -1;
}

/* Never launch a replacement while the previous scheduler may still run. */
static inline int
crond_restart_can_start(int stop_status)
{
	return stop_status == 0;
}

#endif
