#ifndef DETECT_INTERNET_NOTIFICATION_POLICY_H
#define DETECT_INTERNET_NOTIFICATION_POLICY_H

#include <errno.h>

/*
 * notify_rc_status() returns zero on accepted enqueue and nonzero on failure.
 * Preserve a useful errno where available, then a positive status; otherwise
 * return EIO so failed event submission cannot look like success.
 */
static inline int
detect_internet_notification_error(int queue_status, int queue_errno)
{
	if (queue_status == 0)
		return 0;
	if (queue_errno > 0)
		return queue_errno;
	if (queue_status > 0)
		return queue_status;
	return EIO;
}

#endif /* DETECT_INTERNET_NOTIFICATION_POLICY_H */
