#ifndef PADAVAN_SAMBA_DAEMON_POLICY_H
#define PADAVAN_SAMBA_DAEMON_POLICY_H

#include <errno.h>
#include "process_inventory.h"

/*
 * Wait for confirmed daemon presence across a settling interval. UNKNOWN is
 * an inventory failure, not ABSENT: stop immediately and preserve errno.
 */
struct samba_daemon_wait_ops {
	enum process_presence (*get_presence)(void *context, const char *daemon);
	int (*sleep_us)(void *context, unsigned int microseconds);
};

static int
samba_daemon_wait_for_stable_presence(const char *daemon,
				      unsigned int attempts,
				      const struct samba_daemon_wait_ops *ops,
				      void *context)
{
	unsigned int attempt;
	enum process_presence state;

	if (!daemon || !*daemon || attempts == 0 || !ops ||
	    !ops->get_presence || !ops->sleep_us) {
		errno = EINVAL;
		return -1;
	}

	for (attempt = 0; attempt < attempts; ++attempt) {
		errno = 0;
		state = ops->get_presence(context, daemon);
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			if (!errno)
				errno = EIO;
			return -1;
		}
		if (state != PROCESS_PRESENCE_ABSENT &&
		    state != PROCESS_PRESENCE_RUNNING) {
			errno = EIO;
			return -1;
		}
		if (state == PROCESS_PRESENCE_RUNNING) {
			if (ops->sleep_us(context, 500000U) != 0) {
				if (!errno)
					errno = EIO;
				return -1;
			}
			errno = 0;
			state = ops->get_presence(context, daemon);
			if (state == PROCESS_PRESENCE_UNKNOWN) {
				if (!errno)
					errno = EIO;
				return -1;
			}
			if (state != PROCESS_PRESENCE_ABSENT &&
			    state != PROCESS_PRESENCE_RUNNING) {
				errno = EIO;
				return -1;
			}
			if (state == PROCESS_PRESENCE_RUNNING)
				return 0;
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

#endif /* PADAVAN_SAMBA_DAEMON_POLICY_H */
