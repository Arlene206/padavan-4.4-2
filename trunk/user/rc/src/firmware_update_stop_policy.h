#ifndef PADAVAN_FIRMWARE_UPDATE_STOP_POLICY_H
#define PADAVAN_FIRMWARE_UPDATE_STOP_POLICY_H

#include <errno.h>

/*
 * Record the errno from the first failed firmware-preparation stop operation.
 * Callers clear errno before each operation so an unset errno cannot leak a
 * stale value from an earlier successful stop.
 */
static inline int
firmware_update_stop_first_errno(int operation_status, int operation_errno,
				 int first_errno)
{
	if (first_errno > 0)
		return first_errno;
	if (operation_status == 0)
		return 0;
	return operation_errno > 0 ? operation_errno : EIO;
}

#endif /* PADAVAN_FIRMWARE_UPDATE_STOP_POLICY_H */
