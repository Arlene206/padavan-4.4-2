#ifndef PADAVAN_MINIUPNPD_DAEMON_POLICY_H
#define PADAVAN_MINIUPNPD_DAEMON_POLICY_H

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <process_inventory.h>

#define MINIUPNPD_START_WAIT_ATTEMPTS 4

/* eval() observes the daemonizing parent, not necessarily its child. */
static int
miniupnpd_start_policy_status(int launch_status, int daemon_running)
{
	if (launch_status != 0)
		return launch_status;
	return daemon_running ? 0 : ESRCH;
}

/* Do not proceed with WAN/firewall transitions while the old daemon remains. */
static int
miniupnpd_stop_policy_status(int stop_status, int daemon_running)
{
	if (stop_status != 0)
		return stop_status;
	return daemon_running ? EBUSY : 0;
}


enum miniupnpd_daemon_action {
	MINIUPNPD_DAEMON_ABORT = -1,
	MINIUPNPD_DAEMON_START = 0,
	MINIUPNPD_DAEMON_NOTIFY = 1
};

static int
miniupnpd_start_allowed(enum process_presence state)
{
	return state == PROCESS_PRESENCE_ABSENT;
}

/* Keep an already-running daemon aligned with the current service prerequisites. */
static int
miniupnpd_update_should_stop(int upnp_enabled, int nat_enabled, int ap_mode)
{
	return !upnp_enabled || !nat_enabled || ap_mode;
}

static enum miniupnpd_daemon_action
miniupnpd_action_for_presence(enum process_presence state)
{
	if (state == PROCESS_PRESENCE_ABSENT)
		return MINIUPNPD_DAEMON_START;
	if (state == PROCESS_PRESENCE_RUNNING)
		return MINIUPNPD_DAEMON_NOTIFY;
	return MINIUPNPD_DAEMON_ABORT;
}

static int
miniupnpd_presence_error(enum process_presence state, int scan_error)
{
	if (state == PROCESS_PRESENCE_RUNNING || state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (state == PROCESS_PRESENCE_UNKNOWN)
		return scan_error > 0 ? scan_error : EIO;
	return EINVAL;
}

typedef enum process_presence (*miniupnpd_identity_validate_fn)(
	void *context, const struct process_identity *identity);
typedef int (*miniupnpd_signal_pid_fn)(void *context, pid_t pid, int sig);

/* Signal only a complete snapshot after PID/start-time preflight/revalidation. */
static int
miniupnpd_signal_snapshot(const struct process_snapshot *snapshot, int sig,
			  miniupnpd_identity_validate_fn validate,
			  miniupnpd_signal_pid_fn send_signal, void *context)
{
	size_t i;
	int saved_errno;
	enum process_presence state;

	if (!snapshot || !validate || !send_signal || sig != SIGUSR1) {
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

#endif /* PADAVAN_MINIUPNPD_DAEMON_POLICY_H */
