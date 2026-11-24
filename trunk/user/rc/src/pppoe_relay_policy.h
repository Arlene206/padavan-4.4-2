#ifndef PPPOE_RELAY_POLICY_H
#define PPPOE_RELAY_POLICY_H

#include <errno.h>
#include "process_inventory.h"

/* Starting is safe only when the complete process inventory proves ABSENT. */
static inline int
pppoe_relay_should_start(enum process_presence presence)
{
	return presence == PROCESS_PRESENCE_ABSENT;
}

/* A successful launcher exit is not proof that its detached child survived. */
static inline int
pppoe_relay_start_status(int launch_status, enum process_presence presence)
{
	if (launch_status != 0)
		return launch_status;
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	return EIO;
}

#endif
