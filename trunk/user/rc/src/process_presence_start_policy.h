#ifndef PADAVAN_PROCESS_PRESENCE_START_POLICY_H
#define PADAVAN_PROCESS_PRESENCE_START_POLICY_H

#include <errno.h>
#include <process_inventory.h>

/* Decide whether an observed process can be started, notified, or must be
 * left untouched. UNKNOWN is never treated as ABSENT or RUNNING. */
enum process_presence_start_action {
	PROCESS_PRESENCE_START_ABORT = -1,
	PROCESS_PRESENCE_START_NEW = 0,
	PROCESS_PRESENCE_START_NOTIFY = 1
};

static inline enum process_presence_start_action
process_presence_start_action_for_state(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_ABSENT)
		return PROCESS_PRESENCE_START_NEW;
	if (presence == PROCESS_PRESENCE_RUNNING)
		return PROCESS_PRESENCE_START_NOTIFY;
	return PROCESS_PRESENCE_START_ABORT;
}

/* Report requested monitor startup failures without treating RUNNING as an
 * error. An UNKNOWN inventory must fail closed and remain visible to callers. */
static inline int
process_presence_start_status_for_action(
	enum process_presence_start_action action,
	int inventory_error, int launch_status)
{
	switch (action) {
	case PROCESS_PRESENCE_START_NEW:
		return launch_status;
	case PROCESS_PRESENCE_START_NOTIFY:
		return 0;
	case PROCESS_PRESENCE_START_ABORT:
		return inventory_error > 0 ? inventory_error : EIO;
	default:
		return EIO;
	}
}

/* A detached daemon is started only after its child is confirmed RUNNING. */
#define PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS 1
#define PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS 1
#define PROCESS_PRESENCE_START_PROBE_ATTEMPTS 4

static inline int
process_presence_start_postcondition(int launcher_status,
				     enum process_presence process_state)
{
	if (launcher_status != 0)
		return launcher_status;
	return process_state == PROCESS_PRESENCE_RUNNING ? 0 : -1;
}

#endif /* PADAVAN_PROCESS_PRESENCE_START_POLICY_H */
