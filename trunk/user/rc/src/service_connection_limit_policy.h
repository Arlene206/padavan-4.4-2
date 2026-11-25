#ifndef PADAVAN_SERVICE_CONNECTION_LIMIT_POLICY_H
#define PADAVAN_SERVICE_CONNECTION_LIMIT_POLICY_H

#include <errno.h>

/*
 * Preserve the historic 1..maximum clamp without interpreting a failed
 * NVRAM read as the lower bound. Callers must abort staged config writes
 * whenever the read status is non-zero.
 */
static inline int
service_connection_limit_value_status(int read_status, int value,
				       int maximum, int *connections)
{
	if (!connections || maximum < 1)
		return EINVAL;
	*connections = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value < 1)
		value = 1;
	if (value > maximum)
		value = maximum;
	*connections = value;
	return 0;
}

#endif /* PADAVAN_SERVICE_CONNECTION_LIMIT_POLICY_H */
