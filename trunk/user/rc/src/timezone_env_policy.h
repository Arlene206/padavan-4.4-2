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

/*
 * Update the cached kernel timezone only after settimeofday succeeds. A
 * failed syscall must leave the previous cache value intact so a later
 * notification can retry the same requested offset.
 */
static inline int
rc_timezone_kernel_cache_status(int syscall_status, int saved_errno,
                                int requested_minuteswest,
                                int *cached_minuteswest)
{
	if (!cached_minuteswest)
		return EINVAL;
	if (syscall_status != 0)
		return saved_errno > 0 ? saved_errno : EIO;

	*cached_minuteswest = requested_minuteswest;
	return 0;
}

/* Normalize failed libc time conversions without trusting stale errno. */
static inline int
rc_timezone_conversion_status(int conversion_succeeded, int saved_errno)
{
	if (conversion_succeeded)
		return 0;
	return saved_errno > 0 ? saved_errno : EOVERFLOW;
}

#endif /* TIMEZONE_ENV_POLICY_H */
