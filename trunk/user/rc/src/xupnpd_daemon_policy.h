#ifndef PADAVAN_XUPNPD_DAEMON_POLICY_H
#define PADAVAN_XUPNPD_DAEMON_POLICY_H

#include <errno.h>
#include "process_presence_start_policy.h"

typedef enum process_presence (*xupnpd_presence_scan_fn)(
	void *context, int *scan_error);
typedef int (*xupnpd_presence_wait_fn)(void *context,
				       unsigned int seconds);

/*
 * A successful shell/background launch only proves that its launcher exited.
 * Require an independently observed RUNNING child before reporting success.
 * Both the scan and wait operations are injected so the timeout/error paths
 * can be covered deterministically without a live daemon or real sleeps.
 */
static inline int
xupnpd_daemon_start_postcondition_with_ops(
	int launcher_status, xupnpd_presence_scan_fn scan,
	xupnpd_presence_wait_fn wait, void *context,
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

#endif /* PADAVAN_XUPNPD_DAEMON_POLICY_H */
