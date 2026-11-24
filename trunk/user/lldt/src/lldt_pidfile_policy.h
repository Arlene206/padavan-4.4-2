#ifndef PADAVAN_LLDT_PIDFILE_POLICY_H
#define PADAVAN_LLDT_PIDFILE_POLICY_H

#include <errno.h>
#include <sys/file.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#define LLDT_PIDFILE_MAX_DIGITS 10U
#define LLDT_PIDFILE_MAX_TEXT 11U

/* Parse the exact byte count returned by read(); text need not be NUL-terminated. */
static inline int
lldt_pidfile_parse_pid(const char *text, size_t length, pid_t *pid_out)
{
	unsigned long value = 0;
	size_t index;

	if (!pid_out) {
		errno = EINVAL;
		return -1;
	}
	*pid_out = (pid_t)0;
	if (!text) {
		errno = EINVAL;
		return -1;
	}
	if (length > LLDT_PIDFILE_MAX_TEXT) {
		errno = EINVAL;
		return -1;
	}
	if (length > 0 && text[length - 1] == '\n')
		length--;
	if (length == 0 || length > LLDT_PIDFILE_MAX_DIGITS) {
		errno = EINVAL;
		return -1;
	}

	for (index = 0; index < length; index++) {
		unsigned int digit;

		if (text[index] < '0' || text[index] > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(text[index] - '0');
		if (value > ((unsigned long)INT_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}

	if (value == 0) {
		errno = EINVAL;
		return -1;
	}
	*pid_out = (pid_t)value;
	return 0;
}

/*
 * The lock is tied to this open file description. A second process opening
 * the sidecar independently cannot pass the nonblocking exclusive lock.
 */
static inline int
lldt_pidfile_try_exclusive_lock(int fd)
{
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	return flock(fd, LOCK_EX | LOCK_NB);
}

/* Validate an opened PID/lock descriptor before reading or truncating it. */
static inline int
lldt_pidfile_validate_regular_file(const struct stat *st, uid_t expected_uid)
{
	if (!st || !S_ISREG(st->st_mode) || st->st_uid != expected_uid ||
	    st->st_nlink != 1 || (st->st_mode & (S_IWGRP | S_IWOTH))) {
		errno = EPERM;
		return -1;
	}
	return 0;
}

#define LLDT_INTERFACE_NAME_MAX 15U

/* Validate the interface name before placing it in ifreq or a PID path. */
static inline int
lldt_validate_interface_name(const char *interface)
{
	size_t length;

	if (!interface || !interface[0] || strchr(interface, '/')) {
		errno = EINVAL;
		return -1;
	}
	length = strlen(interface);
	if (length > LLDT_INTERFACE_NAME_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return 0;
}

/* Preserve the full interface identity in its per-interface PID path. */
static inline int
lldt_pidfile_format_path(char *path, size_t path_size,
			 const char *program, const char *interface)
{
	int written;

	if (!path || path_size == 0 || !program || !program[0] ||
	    strchr(program, '/')) {
		errno = EINVAL;
		return -1;
	}
	if (lldt_validate_interface_name(interface) != 0)
		return -1;
	written = snprintf(path, path_size, "/var/run/%s-%s.pid",
			   program, interface);
	if (written < 0)
		return -1;
	if ((size_t)written >= path_size) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_LLDT_PIDFILE_POLICY_H */
