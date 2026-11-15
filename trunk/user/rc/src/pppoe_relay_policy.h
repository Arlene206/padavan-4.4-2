#ifndef PPPOE_RELAY_POLICY_H
#define PPPOE_RELAY_POLICY_H

#include "process_inventory.h"

/* Starting is safe only when the complete process inventory proves ABSENT. */
static inline int
pppoe_relay_should_start(enum process_presence presence)
{
	return presence == PROCESS_PRESENCE_ABSENT;
}

#endif
