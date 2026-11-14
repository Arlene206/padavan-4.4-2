#ifndef LPD_PIDFILE_POLICY_H
#define LPD_PIDFILE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* Parse one complete decimal PID record; no whitespace or partial values. */
static int lpd_pidfile_parse(const char *text, size_t length, pid_t *pid)
{
	char buffer[32];
	char *end;
	unsigned long value;
	size_t i;

	if (!text || !pid || length == 0 || length >= sizeof(buffer)) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < length; i++) {
		if (text[i] < '0' || text[i] > '9') {
			errno = EINVAL;
			return -1;
		}
	}
	memcpy(buffer, text, length);
	buffer[length] = '\0';
	errno = 0;
	value = strtoul(buffer, &end, 10);
	if (errno == ERANGE || end == buffer || *end ||
	    value <= 1 || value > (unsigned long)INT_MAX) {
		errno = EINVAL;
		return -1;
	}
	*pid = (pid_t)value;
	return 0;
}

#endif
