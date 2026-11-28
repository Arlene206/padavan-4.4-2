#ifndef IAPP_EVENT_LOG_POLL_POLICY_H
#define IAPP_EVENT_LOG_POLL_POLICY_H

#include <time.h>

/*
 * Event snapshots are polled on a monotonic clock. Return 1 when at least one
 * full second has elapsed, 0 when the interval is shorter or the supplied
 * clock sample moved backwards, and -1 for malformed input.
 */
static inline int
iapp_event_log_poll_due(const struct timespec *now,
			const struct timespec *last)
{
	time_t elapsed_seconds;

	/*
	 * CLOCK_MONOTONIC samples have nonnegative seconds. Reject malformed
	 * synthetic samples too, so the signed elapsed-time subtraction below
	 * cannot overflow when one endpoint is negative.
	 */
	if (!now || !last ||
	    now->tv_sec < 0 || last->tv_sec < 0 ||
	    now->tv_nsec < 0 || now->tv_nsec >= 1000000000L ||
	    last->tv_nsec < 0 || last->tv_nsec >= 1000000000L)
		return -1;

	if (now->tv_sec < last->tv_sec ||
	    (now->tv_sec == last->tv_sec && now->tv_nsec < last->tv_nsec))
		return 0;

	elapsed_seconds = now->tv_sec - last->tv_sec;
	if (elapsed_seconds > 1)
		return 1;
	if (elapsed_seconds < 1)
		return 0;
	return now->tv_nsec >= last->tv_nsec;
}

#endif /* IAPP_EVENT_LOG_POLL_POLICY_H */
