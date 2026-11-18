#ifndef DETECT_INTERNET_NOTIFICATION_POLICY_H
#define DETECT_INTERNET_NOTIFICATION_POLICY_H

#include <errno.h>

/*
 * Normalize status/errno pairs from the launcher, PID-file signal and event
 * queue APIs. A zero status always wins; otherwise prefer errno, then a
 * positive status, and finally EIO for an opaque negative failure.
 */
static inline int
detect_internet_operation_error(int operation_status, int operation_errno)
{
	if (operation_status == 0)
		return 0;
	if (operation_errno > 0)
		return operation_errno;
	if (operation_status > 0)
		return operation_status;
	return EIO;
}

/* Keep the queue-specific name for existing event submission callsites. */
static inline int
detect_internet_notification_error(int queue_status, int queue_errno)
{
	return detect_internet_operation_error(queue_status, queue_errno);
}

#endif /* DETECT_INTERNET_NOTIFICATION_POLICY_H */
