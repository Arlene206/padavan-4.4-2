#ifndef PADAVAN_INADYN_DAEMON_POLICY_H
#define PADAVAN_INADYN_DAEMON_POLICY_H

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <process_inventory.h>
#include <string.h>

enum inadyn_daemon_action {
	INADYN_DAEMON_ABORT = -1,
	INADYN_DAEMON_START = 0,
	INADYN_DAEMON_RELOAD = 1
};

enum inadyn_restore_action {
	INADYN_RESTORE_ABORT = -1,
	INADYN_RESTORE_NOOP = 0,
	INADYN_RESTORE_LAUNCH = 1
};

/* Restore the last-known-good daemon only when this operation stopped one. */
static enum inadyn_restore_action
inadyn_restore_action_for_state(int previous_running,
				enum process_presence current_state)
{
	if (!previous_running || current_state == PROCESS_PRESENCE_RUNNING)
		return INADYN_RESTORE_NOOP;
	if (current_state == PROCESS_PRESENCE_ABSENT)
		return INADYN_RESTORE_LAUNCH;
	return INADYN_RESTORE_ABORT;
}

extract checked DDNS parser/* WAN notifications are normal when optional DDNS is disabled or in AP mode. */
static int
inadyn_update_should_run(int ddns_enabled, int is_ap_mode)
{
	return ddns_enabled && !is_ap_mode;
}

#define INADYN_START_WAIT_ATTEMPTS 4

static int inadyn_presence_error(enum process_presence state, int scan_error);

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

/* Failed detached launches are active unless absence is proven. */
static int
inadyn_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
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

/* system() returns a wait status, not a shell exit code.  Do not return the
 * encoded status to rc's main(), where (for example) 256 would wrap to zero.
 */
static int
inadyn_cache_command_status(int command_status, int command_errno)
{
	if (command_status == 0)
		return 0;
	if (command_status == -1 && command_errno > 0)
		return command_errno;
	return EIO;
}

typedef int (*inadyn_cache_clear_fn)(void *context, const char *path);
typedef int (*inadyn_cache_mkdir_fn)(void *context, const char *path,
				     mode_t mode);
typedef int (*inadyn_cache_lstat_fn)(void *context, const char *path,
				     struct stat *st);
typedef int (*inadyn_cache_chmod_fn)(void *context, const char *path,
				     mode_t mode);

/*
 * A requested cache reset is mandatory. EEXIST from mkdir is accepted only
 * when lstat confirms the existing path is a real directory, not a file or
 * symlink. Errors prevent configuration publication and daemon launch.
 */
static int
inadyn_prepare_cache_directory(int clear_cache, const char *path,
			       inadyn_cache_clear_fn clear_fn,
			       inadyn_cache_mkdir_fn mkdir_fn,
			       inadyn_cache_lstat_fn lstat_fn,
			       inadyn_cache_chmod_fn chmod_fn,
			       void *context)
{
	struct stat st;
	int result;
	int saved_errno;

	if (!path || !*path || !mkdir_fn || !lstat_fn || !chmod_fn ||
	    (clear_cache && !clear_fn)) {
		errno = EINVAL;
		return EINVAL;
	}

	if (clear_cache) {
		errno = 0;
		result = clear_fn(context, path);
		if (result != 0) {
			if (result < 0) {
				saved_errno = errno;
				errno = saved_errno > 0 ? saved_errno : EIO;
				return errno;
			}
			errno = result;
			return result;
		}
	}

	errno = 0;
	result = mkdir_fn(context, path, 0700);
	saved_errno = errno;
	if (result != 0 &&
	    !(result == -1 && saved_errno == EEXIST)) {
		if (result < 0) {
			errno = saved_errno > 0 ? saved_errno : EIO;
			return errno;
		}
		errno = result;
		return result;
	}

	errno = 0;
	result = lstat_fn(context, path, &st);
	saved_errno = errno;
	if (result != 0) {
		if (result < 0) {
			errno = saved_errno > 0 ? saved_errno : EIO;
			return errno;
		}
		errno = result;
		return result;
	}
	if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		return ENOTDIR;
	}
	if (st.st_uid != geteuid()) {
		errno = EPERM;
		return EPERM;
	}

	/* The cache is used by a root daemon. Never accept a shared writable dir. */
	if ((st.st_mode & 0777) != 0700) {
		errno = 0;
		result = chmod_fn(context, path, 0700);
		saved_errno = errno;
		if (result != 0) {
			if (result < 0) {
				errno = saved_errno > 0 ? saved_errno : EIO;
				return errno;
			}
			errno = result;
			return result;
		}

		errno = 0;
		result = lstat_fn(context, path, &st);
		saved_errno = errno;
		if (result != 0) {
			if (result < 0) {
				errno = saved_errno > 0 ? saved_errno : EIO;
				return errno;
			}
			errno = result;
			return result;
		}
		if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid() ||
		    (st.st_mode & 0777) != 0700) {
			errno = EPERM;
			return EPERM;
		}
	}

	return 0;
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
