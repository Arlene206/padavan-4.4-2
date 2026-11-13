#ifndef PADAVAN_FTP_DAEMON_POLICY_H
#define PADAVAN_FTP_DAEMON_POLICY_H

#include <errno.h>

/* Testable bounded wait for an FTP daemon to remain present after start. */
struct ftp_daemon_wait_ops {
	int (*is_running)(void *context, const char *daemon);
	void (*sleep_us)(void *context, unsigned int microseconds);
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
		if (ops->is_running(context, daemon) > 0) {
			ops->sleep_us(context, 500000U);
			if (ops->is_running(context, daemon) > 0)
				return 0;
		}
		ops->sleep_us(context, 100000U);
	}

	errno = ETIMEDOUT;
	return -1;
}

#endif /* PADAVAN_FTP_DAEMON_POLICY_H */
