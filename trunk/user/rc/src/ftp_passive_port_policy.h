#ifndef PADAVAN_FTP_PASSIVE_PORT_POLICY_H
#define PADAVAN_FTP_PASSIVE_PORT_POLICY_H

#include <errno.h>

/* Preserve the legacy port clamp, but never publish a failed or inverted pair. */
static inline int
ftp_passive_port_range_status(int minimum_status, int minimum_value,
			      int maximum_status, int maximum_value,
			      int *minimum_port, int *maximum_port)
{
	if (!minimum_port || !maximum_port) {
		errno = EINVAL;
		return EINVAL;
	}
	*minimum_port = 0;
	*maximum_port = 0;
	if (minimum_status != 0)
		return minimum_status > 0 ? minimum_status : EIO;
	if (maximum_status != 0)
		return maximum_status > 0 ? maximum_status : EIO;

	if (minimum_value < 1)
		minimum_value = 1;
	else if (minimum_value > 65535)
		minimum_value = 65535;
	if (maximum_value < 1)
		maximum_value = 1;
	else if (maximum_value > 65535)
		maximum_value = 65535;
	if (minimum_value > maximum_value) {
		errno = EINVAL;
		return EINVAL;
	}
	*minimum_port = minimum_value;
	*maximum_port = maximum_value;
	return 0;
}

#endif /* PADAVAN_FTP_PASSIVE_PORT_POLICY_H */
