#ifndef RC_NETWORK_SYSCTL_POLICY_H
#define RC_NETWORK_SYSCTL_POLICY_H
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/* Attempt every independent setting and preserve the first failure. */
typedef int (*rc_network_sysctl_operation_fn)(void *context, unsigned int operation);
static inline int
rc_network_sysctl_apply(unsigned int operation_count,
			rc_network_sysctl_operation_fn operation, void *context)
{
	unsigned int i;
	int first_status = 0;
	if (operation_count == 0)
		return 0;
	if (!operation)
		return EINVAL;
	for (i = 0; i < operation_count; ++i) {
		int status = operation(context, i);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	return first_status;
}

/* An explicitly enabled SYN-cookie setting is a WAN admission prerequisite. */
static inline int
rc_network_sysctl_tcp_syncookies_allows_wan(int syncookies_required,
					     int syncookies_write_status)
{
	return !syncookies_required || syncookies_write_status == 0;
}


/* Preserve the legacy missing-key default, but never treat backend errors as disabled. */
static inline int
rc_network_sysctl_parse_syncookies_status(int read_result, int read_errno,
					  const char *value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	if (read_result < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_result > 1 || (read_result == 1 && !value))
		return EIO;

	if (read_result == 0) {
		*enabled = 0;
		return 0;
	}

	/* strtol() otherwise accepts leading whitespace and an optional sign. */
	if (value[0] < '0' || value[0] > '9')
		return EINVAL;

	{
		char *end;
		long parsed_value;

		errno = 0;
		parsed_value = strtol(value, &end, 10);
		if (errno == ERANGE)
			return ERANGE;
		if (end == value || *end != '\0' ||
		    (parsed_value != 0 && parsed_value != 1))
			return EINVAL;
		*enabled = (int)parsed_value;
	}
	return 0;
}

#endif /* RC_NETWORK_SYSCTL_POLICY_H */
