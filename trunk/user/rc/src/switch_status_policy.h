#ifndef SWITCH_STATUS_POLICY_H
#define SWITCH_STATUS_POLICY_H

#include <errno.h>
#include <stdint.h>

/*
 * Switch ioctl wrappers use zero for success and positive errno values for
 * open/ioctl/close failures. The rstats caller uses the usual negative-error
 * convention, so normalize errors before returning and publish counters only
 * after a completely successful request.
 */
static inline int
switch_status_publish_port_bytes(int ioctl_result, uint64_t received,
                                 uint64_t transmitted, uint64_t *rx,
                                 uint64_t *tx)
{
    if (!rx || !tx)
        return -EINVAL;

    if (ioctl_result != 0)
        return (ioctl_result > 0) ? -ioctl_result : ioctl_result;

    *rx = received;
    *tx = transmitted;
    return 0;
}

#endif /* SWITCH_STATUS_POLICY_H */
