#ifndef P910ND_IO_POLICY_H
#define P910ND_IO_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>

/* Retry interrupted stream operations; leave EAGAIN/EWOULDBLOCK to select(). */
static inline ssize_t
p910nd_read_retry_eintr(int fd, void *buffer, size_t length,
                        ssize_t (*read_fn)(int, void *, size_t))
{
	ssize_t result;

	do {
		result = read_fn(fd, buffer, length);
	} while (result < 0 && errno == EINTR);

	return result;
}

static inline ssize_t
p910nd_write_retry_eintr(int fd, const void *buffer, size_t length,
                         ssize_t (*write_fn)(int, const void *, size_t))
{
	ssize_t result;

	do {
		result = write_fn(fd, buffer, length);
	} while (result < 0 && errno == EINTR);

	return result;
}

static inline int
p910nd_io_would_block(int error)
{
	return error == EAGAIN || error == EWOULDBLOCK;
}

/* A positive request followed by a zero-byte write cannot make progress. */
static inline int
p910nd_write_no_progress(ssize_t result, size_t requested)
{
	return requested > 0 && result == 0;
}

/* FD_SET is only defined for descriptor values in [0, FD_SETSIZE). */
static inline int
p910nd_fd_is_selectable(int fd, int fd_setsize)
{
	return fd >= 0 && fd_setsize > 0 && fd < fd_setsize;
}

/* Validate monotonic timestamps before comparing or advancing deadlines. */
static inline int
p910nd_timespec_is_valid(const struct timespec *value)
{
	return value != NULL && value->tv_sec >= 0 &&
	       value->tv_nsec >= 0 && value->tv_nsec < 1000000000L;
}

/* Return 1 when now has reached/passed deadline, 0 before it, -1 if invalid. */
static inline int
p910nd_timespec_deadline_reached(const struct timespec *now,
                                 const struct timespec *deadline)
{
	if (!p910nd_timespec_is_valid(now) ||
	    !p910nd_timespec_is_valid(deadline)) {
		errno = EINVAL;
		return -1;
	}

	return now->tv_sec > deadline->tv_sec ||
	       (now->tv_sec == deadline->tv_sec &&
	        now->tv_nsec >= deadline->tv_nsec);
}

/* Compare elapsed monotonic time at nanosecond precision. */
static inline int
p910nd_timespec_elapsed(const struct timespec *now,
                        const struct timespec *started,
                        unsigned long timeout_seconds)
{
	unsigned long elapsed_seconds;

	if (!p910nd_timespec_is_valid(now) ||
	    !p910nd_timespec_is_valid(started)) {
		errno = EINVAL;
		return -1;
	}
	if (now->tv_sec < started->tv_sec ||
	    (now->tv_sec == started->tv_sec &&
	     now->tv_nsec < started->tv_nsec))
		return 0;

	elapsed_seconds = (unsigned long)(now->tv_sec - started->tv_sec);
	if (elapsed_seconds > timeout_seconds)
		return 1;
	if (elapsed_seconds < timeout_seconds)
		return 0;
	return now->tv_nsec >= started->tv_nsec;
}

/* Add a relative millisecond duration, preserving subsecond carry. */
static inline int
p910nd_timespec_add_milliseconds(const struct timespec *started,
                                 unsigned long milliseconds,
                                 struct timespec *deadline)
{
	unsigned long seconds, remainder, max_seconds;
	long nanoseconds;

	if (!p910nd_timespec_is_valid(started) || !deadline) {
		errno = EINVAL;
		return -1;
	}

	seconds = milliseconds / 1000UL;
	remainder = milliseconds % 1000UL;
	nanoseconds = started->tv_nsec + (long)(remainder * 1000000UL);
	if (nanoseconds >= 1000000000L) {
		nanoseconds -= 1000000000L;
		seconds++;
	}

	max_seconds = (unsigned long)LONG_MAX;
	if (seconds > max_seconds ||
	    (unsigned long)started->tv_sec > max_seconds - seconds) {
		errno = EOVERFLOW;
		return -1;
	}

	deadline->tv_sec = started->tv_sec + (time_t)seconds;
	deadline->tv_nsec = nanoseconds;
	return 0;
}

#endif
