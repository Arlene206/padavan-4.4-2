#ifndef CONSOLE_LOGLEVEL_POLICY_H
#define CONSOLE_LOGLEVEL_POLICY_H

#include <errno.h>
#include <stdlib.h>

/*
 * Preserve the legacy integer conversion and missing-key default, while
 * distinguishing a missing NVRAM key from a backend/read/close failure.
 */
static inline int
rc_console_loglevel_parse_status(int read_result, int read_errno,
                                 const char *value, int *log_level)
{
	int parsed_level;

	if (!log_level)
		return EINVAL;
	if (read_result < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_result > 1 || (read_result == 1 && !value))
		return EIO;

	parsed_level = (read_result == 1) ? atoi(value) : 0;
	*log_level = parsed_level;
	return 0;
}

#endif /* CONSOLE_LOGLEVEL_POLICY_H */
