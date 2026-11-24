#ifndef SHUTILS_PIDFILE_POLICY_H
#define SHUTILS_PIDFILE_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
 * Parse a legacy PID-only record without accepting a numeric prefix from a
 * malformed record. Base-zero conversion preserves the historical helper's
 * decimal, octal and hexadecimal integer syntax; surrounding whitespace
 * remains accepted for old PID files.
 */
static inline int
shutils_pidfile_parse_record(const char *text, size_t length, pid_t *pid)
{
	char buffer[64];
	char *end;
	unsigned long value;
	size_t i = 0;

	if (!text || !pid || length == 0 || length >= sizeof(buffer)) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < length; i++) {
		if (text[i] == '\0') {
			errno = EINVAL;
			return -1;
		}
	}
	memcpy(buffer, text, length);
	buffer[length] = '\0';

	i = 0;
	while (buffer[i] && isspace((unsigned char)buffer[i]))
		i++;
	if (buffer[i] == '\0' || buffer[i] == '-') {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	value = strtoul(buffer, &end, 0);
	if (errno == ERANGE)
		return -1;
	if (end == buffer + i) {
		errno = EINVAL;
		return -1;
	}
	while (*end && isspace((unsigned char)*end))
		end++;
	if (*end != '\0' || value <= 1) {
		errno = EINVAL;
		return -1;
	}
	if (value > (unsigned long)INT_MAX) {
		errno = ERANGE;
		return -1;
	}

	*pid = (pid_t)value;
	return 0;
}

/* PID files direct privileged signal delivery. Trust only an owned, single-link
 * regular file that cannot be modified by another uid or group member. */
static inline int
shutils_pidfile_metadata_is_safe(const struct stat *status, uid_t owner)
{
	return status && ((status->st_mode & S_IFMT) == S_IFREG) && status->st_uid == owner &&
	       status->st_nlink == 1 && (status->st_mode & 022) == 0;
}

/*
 * PID files are control inputs to privileged signal delivery. Never follow a
 * symlink supplied at the configured pathname.
 */
static inline int
shutils_pidfile_open(const char *pidfile)
{
	if (pidfile == NULL || pidfile[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
	return open(pidfile, O_RDONLY | O_NONBLOCK | O_NOFOLLOW);
}

#endif
