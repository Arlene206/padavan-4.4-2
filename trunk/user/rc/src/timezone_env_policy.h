#ifndef TIMEZONE_ENV_POLICY_H
#define TIMEZONE_ENV_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/*
 * Format the TZ environment assignment from nvram_get_checked()'s
 * tri-state result. Missing keys retain the legacy empty-TZ behavior;
 * backend errors and overlong values leave the destination untouched.
 */
static inline int
rc_timezone_env_format_status(int read_result, int read_errno,
                              const char *time_zone, char *destination,
                              size_t destination_size)
{
	const char *value;
	size_t value_length;

	if (!destination || destination_size < 4U)
		return EINVAL;
	if (read_result < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_result > 1)
		return EIO;

	if (read_result == 0)
		value = "";
	else {
		if (!time_zone)
			return EIO;
		value = time_zone;
	}

	value_length = strlen(value);
	if (value_length > destination_size - 4U)
		return ENAMETOOLONG;

	memcpy(destination, "TZ=", 3U);
	memcpy(destination + 3U, value, value_length + 1U);
	return 0;
}

#endif /* TIMEZONE_ENV_POLICY_H */
