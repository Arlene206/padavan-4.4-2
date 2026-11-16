#ifndef PADAVAN_INADYN_DAEMON_POLICY_H
#define PADAVAN_INADYN_DAEMON_POLICY_H

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <process_inventory.h>

enum inadyn_daemon_action {
	INADYN_DAEMON_ABORT = -1,
	INADYN_DAEMON_START = 0,
	INADYN_DAEMON_RELOAD = 1
};

#define INADYN_START_WAIT_ATTEMPTS 4

/* A successful fork/daemonize return is not a running-service postcondition. */
static int
inadyn_start_status(int launch_status, enum process_presence state,
		    int scan_error)
{
	int status;

	if (launch_status != 0)
		return launch_status;
	if (state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (state == PROCESS_PRESENCE_UNKNOWN)
		status = inadyn_presence_error(state, scan_error);
	else if (state == PROCESS_PRESENCE_ABSENT)
		status = ETIMEDOUT;
	else
		status = EINVAL;

	errno = status;
	return status;
}

static enum inadyn_daemon_action
inadyn_action_for_presence(enum process_presence state)
{
	if (state == PROCESS_PRESENCE_ABSENT)
		return INADYN_DAEMON_START;
	if (state == PROCESS_PRESENCE_RUNNING)
		return INADYN_DAEMON_RELOAD;
	return INADYN_DAEMON_ABORT;
}

static int
inadyn_presence_error(enum process_presence state, int scan_error)
{
	if (state == PROCESS_PRESENCE_RUNNING || state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (state == PROCESS_PRESENCE_UNKNOWN)
		return scan_error > 0 ? scan_error : EIO;
	return EINVAL;
}

typedef enum process_presence (*inadyn_identity_validate_fn)(
	void *context, const struct process_identity *identity);
typedef int (*inadyn_signal_pid_fn)(void *context, pid_t pid, int sig);

/* Require every captured identity to be live before publishing a new config. */
static int
inadyn_validate_snapshot(const struct process_snapshot *snapshot,
			 inadyn_identity_validate_fn validate, void *context)
{
	size_t i;
	enum process_presence state;

	if (!snapshot || !validate || snapshot->state != PROCESS_PRESENCE_RUNNING ||
	    !snapshot->items || snapshot->count == 0) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < snapshot->count; i++) {
		errno = 0;
		state = validate(context, &snapshot->items[i]);
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			errno = errno ? errno : EIO;
			return -1;
		}
		if (state != PROCESS_PRESENCE_RUNNING) {
			errno = state == PROCESS_PRESENCE_ABSENT ? ESRCH : EIO;
			return -1;
		}
	}
	return 0;
}

/* Signal only captured identities; never fan out by process name. */
static int
inadyn_signal_snapshot(const struct process_snapshot *snapshot, int sig,
		       inadyn_identity_validate_fn validate,
		       inadyn_signal_pid_fn send_signal, void *context)
{
	size_t i;
	int saved_errno;
	enum process_presence state;

	if (!snapshot || !validate || !send_signal ||
	    (sig != SIGHUP && sig != SIGUSR1)) {
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

#endif /* PADAVAN_INADYN_DAEMON_POLICY_H */
