#ifndef PADAVAN_PROCESS_PRESENCE_START_POLICY_H
#define PADAVAN_PROCESS_PRESENCE_START_POLICY_H

#include <process_inventory.h>

/* Decide whether an observed process can be started, notified, or must be
 * left untouched. UNKNOWN is never treated as ABSENT or RUNNING. */
enum process_presence_start_action {
	PROCESS_PRESENCE_START_ABORT = -1,
	PROCESS_PRESENCE_START_NEW = 0,
	PROCESS_PRESENCE_START_NOTIFY = 1
};

static enum process_presence_start_action
process_presence_start_action_for_state(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_ABSENT)
		return PROCESS_PRESENCE_START_NEW;
	if (presence == PROCESS_PRESENCE_RUNNING)
		return PROCESS_PRESENCE_START_NOTIFY;
	return PROCESS_PRESENCE_START_ABORT;
}

#endif /* PADAVAN_PROCESS_PRESENCE_START_POLICY_H */
