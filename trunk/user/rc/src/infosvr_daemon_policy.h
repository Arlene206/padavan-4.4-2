#ifndef INFOSVR_DAEMON_POLICY_H
#define INFOSVR_DAEMON_POLICY_H

#include "process_inventory.h"

/* Do not launch a replacement until the process inventory confirms absence. */
static inline int
infosvr_restart_can_start(int stop_status, enum process_presence process_state)
{
	return stop_status == 0 && process_state == PROCESS_PRESENCE_ABSENT;
}


#define INFOSVR_START_INITIAL_WAIT_SECONDS 1
#define INFOSVR_START_RETRY_WAIT_SECONDS 1
#define INFOSVR_START_PROBE_ATTEMPTS 4

/* eval() observes the daemonizing parent; verify that the detached child lives. */
static inline int
infosvr_start_postcondition(int launch_status,
			    enum process_presence process_state)
{
	if (launch_status != 0)
		return -1;
	return process_state == PROCESS_PRESENCE_RUNNING ? 0 : -1;
}

#endif
