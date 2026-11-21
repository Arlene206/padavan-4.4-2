#ifndef PADAVAN_FTP_DAEMON_POLICY_H
#define PADAVAN_FTP_DAEMON_POLICY_H

#include <errno.h>

#include "process_inventory.h"

/* A failed detached launch is considered active unless absence is proven. */
static int
ftp_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
}

/* Do not turn an NVRAM backend error into an implicit disabled value. */
static inline int
ftp_enable_value_status(int read_status, int value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	*enabled = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value != 0 && value != 1)
		return EINVAL;
	*enabled = value;
	return 0;
}

#endif /* PADAVAN_FTP_DAEMON_POLICY_H */
