#ifndef PADAVAN_SERVICE_WAIT_POLICY_H
#define PADAVAN_SERVICE_WAIT_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#define SERVICE_WAIT_MAX_INTERRUPTS 8U

/*
 * sleep() returns the unslept remainder after EINTR. Retry that remainder so
 * signal delivery cannot turn a configured wait into a burst of early probes.
 * A persistent signal storm must still not pin PID 1 in an unbounded loop.
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
	unsigned int interruptions = 0;

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
		if (remaining > 0) {
			if (interruptions >= SERVICE_WAIT_MAX_INTERRUPTS) {
				errno = EINTR;
				return -1;
			}
			++interruptions;
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

/*
 * Probe waits are service-start prerequisites: preserve errno for diagnostics
 * while returning the negative failure convention consumed by PID 1.
 */
static inline int
service_wait_failure_status(int wait_errno)
{
	errno = wait_errno > 0 ? wait_errno : EIO;
	return -1;
}


/*
 * nanosleep() reports an unslept remainder on EINTR. Retry that exact
 * remainder so short daemon-settling intervals are not shortened by signals.
 */
typedef int (*service_wait_nanosleep_fn)(const struct timespec *requested,
					 struct timespec *remaining,
					 void *context);

static inline int
service_wait_nanosleep_real(const struct timespec *requested,
			    struct timespec *remaining, void *context)
{
	(void)context;
	return nanosleep(requested, remaining);
}

static inline int
service_wait_timespec_with(struct timespec requested,
			   service_wait_nanosleep_fn sleep_fn, void *context)
{
	unsigned int interruptions = 0;

	if (!sleep_fn || requested.tv_sec < 0 || requested.tv_nsec < 0 ||
	    requested.tv_nsec >= 1000000000L) {
		errno = EINVAL;
		return -1;
	}
	while (requested.tv_sec > 0 || requested.tv_nsec > 0) {
		struct timespec remaining;
		int status;

		remaining.tv_sec = 0;
		remaining.tv_nsec = 0;
		errno = 0;
		status = sleep_fn(&requested, &remaining, context);
		if (status == 0)
			return 0;
		if (errno != EINTR)
			return -1;
		if (remaining.tv_sec < 0 || remaining.tv_nsec < 0 ||
		    remaining.tv_nsec >= 1000000000L ||
		    remaining.tv_sec > requested.tv_sec ||
		    (remaining.tv_sec == requested.tv_sec &&
		     remaining.tv_nsec > requested.tv_nsec)) {
			errno = EIO;
			return -1;
		}
		if (remaining.tv_sec == 0 && remaining.tv_nsec == 0) {
			requested = remaining;
			continue;
		}
		if (interruptions >= SERVICE_WAIT_MAX_INTERRUPTS) {
			errno = EINTR;
			return -1;
		}
		++interruptions;
		requested = remaining;
	}
	return 0;
}

static inline int
service_wait_microseconds_with(unsigned int microseconds,
			       service_wait_nanosleep_fn sleep_fn,
			       void *context)
{
	struct timespec requested;

	requested.tv_sec = microseconds / 1000000U;
	requested.tv_nsec = (long)(microseconds % 1000000U) * 1000L;
	return service_wait_timespec_with(requested, sleep_fn, context);
}

static inline int
service_wait_microseconds(unsigned int microseconds)
{
	return service_wait_microseconds_with(
		microseconds, service_wait_nanosleep_real, NULL);
}

#endif /* PADAVAN_SERVICE_WAIT_POLICY_H */
