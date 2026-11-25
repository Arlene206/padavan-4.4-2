#ifndef PADAVAN_SERVICE_DAEMON_WAIT_POLICY_H
#define PADAVAN_SERVICE_DAEMON_WAIT_POLICY_H

#include <errno.h>
#include "process_inventory.h"

/* Generic bounded post-launch wait for daemons that detach from their
 * launcher's process. UNKNOWN is never treated as ABSENT. */
struct service_daemon_wait_ops {
	int (*is_running)(void *context, const char *daemon);
	int (*sleep_us)(void *context, unsigned int microseconds);
};

static int
service_daemon_wait_for_stable_presence(const char *daemon, unsigned int attempts,
					const struct service_daemon_wait_ops *ops,
					void *context)
{
	unsigned int attempt;

	if (!daemon || !*daemon || attempts == 0 || !ops ||
	    !ops->is_running || !ops->sleep_us) {
		errno = EINVAL;
		return -1;
	}

	for (attempt = 0; attempt < attempts; ++attempt) {
		int presence = ops->is_running(context, daemon);

		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			if (!errno)
				errno = EIO;
			return -1;
		}
		if (presence != PROCESS_PRESENCE_ABSENT &&
		    presence != PROCESS_PRESENCE_RUNNING) {
			errno = EIO;
			return -1;
		}
		if (presence == PROCESS_PRESENCE_RUNNING) {
			if (ops->sleep_us(context, 500000U) != 0) {
				if (!errno)
					errno = EIO;
				return -1;
			}
			presence = ops->is_running(context, daemon);
			if (presence == PROCESS_PRESENCE_UNKNOWN) {
				if (!errno)
					errno = EIO;
				return -1;
			}
			if (presence == PROCESS_PRESENCE_RUNNING)
				return 0;
			if (presence != PROCESS_PRESENCE_ABSENT) {
				errno = EIO;
				return -1;
			}
		}
		if (ops->sleep_us(context, 100000U) != 0) {
			if (!errno)
				errno = EIO;
			return -1;
		}
	}

	errno = ETIMEDOUT;
	return -1;
}

#endif /* PADAVAN_SERVICE_DAEMON_WAIT_POLICY_H */
