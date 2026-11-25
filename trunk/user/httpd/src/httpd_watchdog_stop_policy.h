#ifndef HTTPD_WATCHDOG_STOP_POLICY_H
#define HTTPD_WATCHDOG_STOP_POLICY_H

#include <errno.h>
#include <signal.h>

#include "process_inventory.h"

#define HTTPD_WATCHDOG_STOP_WAIT_ATTEMPTS 5

struct httpd_watchdog_stop_ops {
	enum process_presence (*presence)(void *context);
	int (*signal_process)(void *context, int signal_number);
	void (*sleep_one_second)(void *context);
};

/*
 * NVRAM profile replacement must not start while the watchdog can still read
 * or mutate the old configuration. Only a confirmed ABSENT snapshot is a
 * successful stop; UNKNOWN and a bounded stop timeout fail closed.
 */
static inline int
httpd_watchdog_stop_for_restore(const struct httpd_watchdog_stop_ops *ops,
				void *context)
{
	enum process_presence presence;
	int attempt;

	if (!ops || !ops->presence || !ops->signal_process ||
	    !ops->sleep_one_second) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	presence = ops->presence(context);
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (presence == PROCESS_PRESENCE_UNKNOWN) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if (presence != PROCESS_PRESENCE_RUNNING) {
		errno = EPROTO;
		return -1;
	}

	/* process_pidfile_signal() verifies PID + start_time + process identity. */
	errno = 0;
	if (ops->signal_process(context, SIGTERM) != 0 && errno != ESRCH)
		return -1;

	for (attempt = 0; attempt < HTTPD_WATCHDOG_STOP_WAIT_ATTEMPTS; attempt++) {
		errno = 0;
		presence = ops->presence(context);
		if (presence == PROCESS_PRESENCE_ABSENT)
			return 0;
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			if (errno == 0)
				errno = EIO;
			return -1;
		}
		if (presence != PROCESS_PRESENCE_RUNNING) {
			errno = EPROTO;
			return -1;
		}
		if (attempt + 1 < HTTPD_WATCHDOG_STOP_WAIT_ATTEMPTS)
			ops->sleep_one_second(context);
	}

	errno = ETIMEDOUT;
	return -1;
}

#endif
