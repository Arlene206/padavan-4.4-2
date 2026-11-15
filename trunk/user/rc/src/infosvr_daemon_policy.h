#ifndef INFOSVR_DAEMON_POLICY_H
#define INFOSVR_DAEMON_POLICY_H

#include "process_inventory.h"

/* Do not launch a replacement until the process inventory confirms absence. */
static inline int
infosvr_restart_can_start(int stop_status, enum process_presence process_state)
{
	return stop_status == 0 && process_state == PROCESS_PRESENCE_ABSENT;
}

#endif
