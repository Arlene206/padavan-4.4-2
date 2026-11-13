#ifndef WIRELESS_IOCTL_POLICY_H
#define WIRELESS_IOCTL_POLICY_H

#include <errno.h>

/*
 * Convert libc socket/ioctl/close failures into the negative-errno convention
 * used by RC callers. A missing errno is reported as EIO rather than success.
 */
static inline int
wireless_ioctl_error_status(int error_number)
{
    return (error_number > 0) ? -error_number : -EIO;
}

/*
 * Prefer the operation error over a later close error. A close failure is
 * still surfaced when the ioctl itself succeeded.
 */
static inline int
wireless_ioctl_status(int ioctl_result, int ioctl_errno,
                      int close_result, int close_errno)
{
    if (ioctl_result < 0)
        return wireless_ioctl_error_status(ioctl_errno);

    if (close_result < 0)
        return wireless_ioctl_error_status(close_errno);

    return ioctl_result;
}

#endif /* WIRELESS_IOCTL_POLICY_H */
