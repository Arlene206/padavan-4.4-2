#ifndef PADAVAN_NETWORKMAP_DAEMON_POLICY_H
#define PADAVAN_NETWORKMAP_DAEMON_POLICY_H

#include <errno.h>
#include <sys/types.h>
#include "process_inventory.h"

enum networkmap_daemon_action {
	NETWORKMAP_DAEMON_ABORT = -1,
	NETWORKMAP_DAEMON_START = 0,
	NETWORKMAP_DAEMON_REFRESH = 1
};

/* Replacement start requires a complete inventory proving absence. */
static int
networkmap_start_allowed(enum process_presence state)
{
	return state == PROCESS_PRESENCE_ABSENT;
}

static enum networkmap_daemon_action
networkmap_action_for_presence(enum process_presence state)
{
	if (state == PROCESS_PRESENCE_ABSENT)
		return NETWORKMAP_DAEMON_START;
	if (state == PROCESS_PRESENCE_RUNNING)
		return NETWORKMAP_DAEMON_REFRESH;
	return NETWORKMAP_DAEMON_ABORT;
}

typedef enum process_presence (*networkmap_identity_validate_fn)(
	void *context, const struct process_identity *identity);
typedef int (*networkmap_signal_pid_fn)(void *context, pid_t pid, int sig);

/*
 * Preflight the complete snapshot before signalling; revalidate each PID/start
 * identity immediately before its targeted signal. Never signal on UNKNOWN.
 */
static int
networkmap_signal_snapshot(const struct process_snapshot *snapshot, int sig,
			   networkmap_identity_validate_fn validate,
			   networkmap_signal_pid_fn send_signal, void *context)
{
	size_t i;
	int saved_errno;
	enum process_presence state;

	if (!snapshot || !validate || !send_signal || sig <= 0) {
		errno = EINVAL;
		return -1;
	}
	if (snapshot->state != PROCESS_PRESENCE_RUNNING ||
	    !snapshot->items || snapshot->count == 0) {
		errno = snapshot->state == PROCESS_PRESENCE_ABSENT ? ESRCH : EIO;
		return -1;
	}

	for (i = 0; i < snapshot->count; i++) {
		errno = 0;
		state = validate(context, &snapshot->items[i]);
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			errno = errno ? errno : EIO;
			return -1;
		}
		if (state != PROCESS_PRESENCE_RUNNING &&
		    state != PROCESS_PRESENCE_ABSENT) {
			errno = EIO;
			return -1;
		}
	}

	for (i = 0; i < snapshot->count; i++) {
		errno = 0;
		state = validate(context, &snapshot->items[i]);
		if (state == PROCESS_PRESENCE_ABSENT)
			continue;
		if (state != PROCESS_PRESENCE_RUNNING) {
			errno = errno ? errno : EIO;
			return -1;
		}
		if (send_signal(context, snapshot->items[i].pid, sig) != 0) {
			saved_errno = errno ? errno : EIO;
			if (saved_errno == ESRCH)
				continue;
			errno = saved_errno;
			return -1;
		}
	}

	return 0;
}


#define NETWORKMAP_START_INITIAL_WAIT_SECONDS 1
#define NETWORKMAP_START_RETRY_WAIT_SECONDS 1
#define NETWORKMAP_START_PROBE_ATTEMPTS 4

/* eval() observes the daemonizing parent; verify that its detached child lives. */
static int
networkmap_start_postcondition(int launch_status,
			       enum process_presence process_state)
{
	if (launch_status != 0)
		return launch_status;
	return process_state == PROCESS_PRESENCE_RUNNING ? 0 : -1;
}

#endif
