#ifndef PADAVAN_DETACHED_DAEMON_START_POLICY_H
#define PADAVAN_DETACHED_DAEMON_START_POLICY_H

#include <errno.h>
#include "process_presence_start_policy.h"

typedef enum process_presence (*detached_daemon_presence_scan_fn)(
	void *context, int *scan_error);
typedef int (*detached_daemon_presence_wait_fn)(void *context,
						 unsigned int seconds);
typedef int (*detached_daemon_presence_stop_fn)(void *context);

/*
 * A successful shell/background launch only proves that its launcher exited.
 * Require an independently observed RUNNING child before reporting success.
 * Both scan and wait are injected so timeout and error paths are deterministic.
 */
static inline int
detached_daemon_start_postcondition_with_ops(
	int launcher_status, detached_daemon_presence_scan_fn scan,
	detached_daemon_presence_wait_fn wait, void *context,
	unsigned int initial_wait_seconds,
	unsigned int retry_wait_seconds, unsigned int attempts)
{
	unsigned int attempt;

	if (launcher_status != 0)
		return launcher_status;
	if (!scan || !wait || attempts == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	if (initial_wait_seconds != 0) {
		errno = 0;
		if (wait(context, initial_wait_seconds) != 0) {
			int saved_errno = errno ? errno : EINTR;
			errno = saved_errno;
			return saved_errno;
		}
	}

	for (attempt = 0; attempt < attempts; attempt++) {
		int scan_error = 0;
		enum process_presence state = scan(context, &scan_error);

		if (process_presence_start_postcondition(0, state) == 0)
			return 0;
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			int saved_errno = scan_error > 0 ? scan_error : EIO;
			errno = saved_errno;
			return saved_errno;
		}
		if (state != PROCESS_PRESENCE_ABSENT) {
			errno = EIO;
			return EIO;
		}
		if (attempt + 1 < attempts && retry_wait_seconds != 0) {
			errno = 0;
			if (wait(context, retry_wait_seconds) != 0) {
				int saved_errno = errno ? errno : EINTR;
				errno = saved_errno;
				return saved_errno;
			}
		}
	}

	errno = ETIMEDOUT;
	return ETIMEDOUT;
}

/*
 * A detached launcher may fail its readiness window and still leave a late
 * child behind. Attempt a targeted stop, then take a fresh process snapshot.
 * Preserve the primary startup status/errno; report cleanup and inventory
 * outcomes separately so callers can log an incomplete rollback.
 */
static inline int
detached_daemon_start_rollback_with_ops(
	int start_status, int start_errno,
	detached_daemon_presence_stop_fn stop,
	detached_daemon_presence_scan_fn scan, void *context,
	int *stop_status_out, enum process_presence *final_state_out,
	int *scan_error_out)
{
	int stop_status = 0;
	int scan_error = 0;
	enum process_presence final_state = PROCESS_PRESENCE_UNKNOWN;

	if (stop_status_out)
		*stop_status_out = 0;
	if (final_state_out)
		*final_state_out = PROCESS_PRESENCE_UNKNOWN;
	if (scan_error_out)
		*scan_error_out = 0;
	if (start_status == 0)
		return 0;
	if (!stop || !scan) {
		errno = EINVAL;
		return start_status;
	}

	stop_status = stop(context);
	final_state = scan(context, &scan_error);
	if (stop_status_out)
		*stop_status_out = stop_status;
	if (final_state_out)
		*final_state_out = final_state;
	if (scan_error_out)
		*scan_error_out = scan_error;

	errno = start_errno > 0 ? start_errno :
		(start_status > 0 ? start_status : EIO);
	return start_status;
}

#endif /* PADAVAN_DETACHED_DAEMON_START_POLICY_H */
