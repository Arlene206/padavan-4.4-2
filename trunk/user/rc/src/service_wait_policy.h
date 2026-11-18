#ifndef PADAVAN_SERVICE_WAIT_POLICY_H
#define PADAVAN_SERVICE_WAIT_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <unistd.h>

/*
 * sleep() returns the unslept remainder after EINTR. Retry that remainder so
 * signal delivery cannot turn a configured wait into a burst of early probes.
 */
typedef unsigned int (*service_wait_sleep_fn)(unsigned int seconds,
					      void *context);

static inline unsigned int
service_wait_sleep_real(unsigned int seconds, void *context)
{
	(void)context;
	return sleep(seconds);
}

static inline int
service_wait_seconds_with(unsigned int seconds,
			  service_wait_sleep_fn sleep_fn, void *context)
{
	if (!sleep_fn) {
		errno = EINVAL;
		return -1;
	}
	while (seconds > 0) {
		unsigned int remaining = sleep_fn(seconds, context);

		if (remaining > seconds) {
			errno = EIO;
			return -1;
		}
		seconds = remaining;
	}
	return 0;
}

static inline int
service_wait_seconds(unsigned int seconds)
{
	return service_wait_seconds_with(seconds, service_wait_sleep_real, NULL);
}

#endif /* PADAVAN_SERVICE_WAIT_POLICY_H */
