#ifndef PADAVAN_SAMBA_DAEMON_POLICY_H
#define PADAVAN_SAMBA_DAEMON_POLICY_H

#include <errno.h>

/*
 * Wait for a daemon to remain present across a settling interval. The small
 * callback surface keeps the timeout behavior testable without router APIs.
 */
struct samba_daemon_wait_ops {
	int (*is_running)(void *context, const char *daemon);
	void (*sleep_us)(void *context, unsigned int microseconds);
};

static int
samba_daemon_wait_for_stable_presence(const char *daemon,
				      unsigned int attempts,
				      const struct samba_daemon_wait_ops *ops,
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

#endif /* PADAVAN_SAMBA_DAEMON_POLICY_H */
