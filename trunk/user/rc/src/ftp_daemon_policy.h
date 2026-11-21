#ifndef PADAVAN_FTP_DAEMON_POLICY_H
#define PADAVAN_FTP_DAEMON_POLICY_H

#include <errno.h>
#include "process_inventory.h"

/* A failed detached launch is considered active unless absence is proven. */
static int
ftp_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
}

/* Testable bounded wait for an FTP daemon to remain present after start. */
struct ftp_daemon_wait_ops {
	int (*is_running)(void *context, const char *daemon);
	int (*sleep_us)(void *context, unsigned int microseconds);
};

static int
ftp_daemon_wait_for_stable_presence(const char *daemon, unsigned int attempts,
				    const struct ftp_daemon_wait_ops *ops,
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

#endif /* PADAVAN_FTP_DAEMON_POLICY_H */
