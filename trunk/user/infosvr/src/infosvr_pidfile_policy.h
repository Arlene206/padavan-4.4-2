#ifndef INFOSVR_PIDFILE_POLICY_H
#define INFOSVR_PIDFILE_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

/* Keep the lock on the opened pidfile inode for the daemon's whole lifetime. */
static inline int
infosvr_pidfile_try_exclusive_lock(int fd)
{
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}

	return flock(fd, LOCK_EX | LOCK_NB);
}

/* Replace a stale PID only after the caller has acquired the exclusive lock. */
static inline int
infosvr_pidfile_write(int fd, pid_t pid)
{
	char text[32];
	int length;
	int written_total = 0;

	if (fd < 0 || pid <= (pid_t)0) {
		errno = EINVAL;
		return -1;
	}

	length = snprintf(text, sizeof(text), "%ld", (long)pid);
	if (length <= 0 || (size_t)length >= sizeof(text)) {
		errno = EOVERFLOW;
		return -1;
	}
	if (ftruncate(fd, 0) != 0)
		return -1;
	if (lseek(fd, 0, SEEK_SET) < 0)
		return -1;

	while (written_total < length) {
		ssize_t written = write(fd, text + written_total,
					(size_t)(length - written_total));

		if (written < 0 && errno == EINTR)
			continue;
		if (written <= 0) {
			if (written == 0)
				errno = EIO;
			return -1;
		}
		written_total += (int)written;
	}

	return 0;
}

/* Return 1 only for an exact, complete owner record; 0 means not owned. */
static inline int
infosvr_pidfile_matches(int fd, pid_t pid)
{
	char expected[32];
	char actual[32];
	char extra;
	int expected_length;
	int actual_length = 0;
	ssize_t amount;

	if (fd < 0 || pid <= (pid_t)0) {
		errno = EINVAL;
		return -1;
	}

	expected_length = snprintf(expected, sizeof(expected), "%ld", (long)pid);
	if (expected_length <= 0 || (size_t)expected_length >= sizeof(expected)) {
		errno = EOVERFLOW;
		return -1;
	}
	if (lseek(fd, 0, SEEK_SET) < 0)
		return -1;

	while (actual_length < expected_length) {
		amount = read(fd, actual + actual_length,
			      (size_t)(expected_length - actual_length));
		if (amount < 0 && errno == EINTR)
			continue;
		if (amount < 0)
			return -1;
		if (amount == 0)
			return 0;
		actual_length += (int)amount;
	}

	do {
		amount = read(fd, &extra, 1);
	} while (amount < 0 && errno == EINTR);
	if (amount < 0)
		return -1;
	if (amount != 0)
		return 0;

	return memcmp(actual, expected, (size_t)expected_length) == 0;
}

#endif /* INFOSVR_PIDFILE_POLICY_H */
