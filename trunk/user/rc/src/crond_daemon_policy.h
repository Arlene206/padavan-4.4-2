#ifndef CROND_DAEMON_POLICY_H
#define CROND_DAEMON_POLICY_H

#define CROND_START_INITIAL_WAIT_SECONDS 1
#define CROND_START_RETRY_WAIT_SECONDS 1
#define CROND_START_PROBE_ATTEMPTS 4

/* Normalize launcher and detached-child postcondition failures for RC. */
static inline int
crond_start_postcondition(int launcher_status, int process_present)
{
	if (launcher_status != 0)
		return -1;

	return process_present ? 0 : -1;
}

/* Never launch a replacement while the previous scheduler may still run. */
static inline int
crond_restart_can_start(int stop_status)
{
	return stop_status == 0;
}

#endif
