#ifndef IAPP_TCP_DEADLINE_POLICY_H
#define IAPP_TCP_DEADLINE_POLICY_H

#include <sys/time.h>
#include <time.h>

/*
 * Convert one absolute monotonic deadline into select()'s relative timeout.
 * Return 1 while time remains, 0 when expired, and -1 for invalid inputs.
 * Round a positive sub-microsecond remainder up so select() cannot expire
 * before the absolute deadline due to timeval precision loss.
 */
static inline int
iapp_tcp_deadline_remaining(const struct timespec *now,
			    const struct timespec *deadline,
			    struct timeval *remaining)
{
	time_t seconds;
	long nanoseconds;

	if (now == NULL || deadline == NULL || remaining == NULL ||
	    now->tv_nsec < 0 || now->tv_nsec >= 1000000000L ||
	    deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000L)
		return -1;

	if (now->tv_sec > deadline->tv_sec ||
	    (now->tv_sec == deadline->tv_sec &&
	     now->tv_nsec >= deadline->tv_nsec)) {
		remaining->tv_sec = 0;
		remaining->tv_usec = 0;
		return 0;
	}

	seconds = deadline->tv_sec - now->tv_sec;
	nanoseconds = deadline->tv_nsec - now->tv_nsec;
	if (nanoseconds < 0) {
		seconds--;
		nanoseconds += 1000000000L;
	}

	remaining->tv_sec = seconds;
	remaining->tv_usec = (nanoseconds + 999L) / 1000L;
	if (remaining->tv_usec >= 1000000L) {
		remaining->tv_sec++;
		remaining->tv_usec -= 1000000L;
	}
	return 1;
}

#endif /* IAPP_TCP_DEADLINE_POLICY_H */
