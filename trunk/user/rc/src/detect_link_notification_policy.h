#ifndef PADAVAN_DETECT_LINK_NOTIFICATION_POLICY_H
#define PADAVAN_DETECT_LINK_NOTIFICATION_POLICY_H

#include <errno.h>

/* Normalize launcher statuses and -1/errno PID-file signal failures. */
static inline int
detect_link_notification_status(int operation_status, int saved_errno)
{
    if (operation_status == 0)
        return 0;
    if (operation_status > 0)
        return operation_status;
    return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* PADAVAN_DETECT_LINK_NOTIFICATION_POLICY_H */
