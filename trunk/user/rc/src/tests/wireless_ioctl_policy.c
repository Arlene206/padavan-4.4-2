#include "wireless_ioctl_policy.h"

#include <stdio.h>

static int
test_socket_errno_is_normalized(void)
{
    if (wireless_ioctl_error_status(EACCES) != -EACCES)
        return 1;
    if (wireless_ioctl_error_status(0) != -EIO)
        return 1;
    return 0;
}

static int
test_ioctl_failure_wins_over_close_failure(void)
{
    return wireless_ioctl_status(-1, EBUSY, -1, EBADF) != -EBUSY;
}

static int
test_ioctl_failure_without_errno_fails_closed(void)
{
    return wireless_ioctl_status(-1, 0, 0, 0) != -EIO;
}

static int
test_close_failure_is_reported_after_successful_ioctl(void)
{
    if (wireless_ioctl_status(0, 0, -1, EBADF) != -EBADF)
        return 1;
    if (wireless_ioctl_status(0, 0, -1, 0) != -EIO)
        return 1;
    return 0;
}

static int
test_successful_ioctl_and_close_preserve_result(void)
{
    if (wireless_ioctl_status(0, 0, 0, 0) != 0)
        return 1;
    if (wireless_ioctl_status(1, 0, 0, 0) != 1)
        return 1;
    return 0;
}

int
main(void)
{
    if (test_socket_errno_is_normalized() ||
        test_ioctl_failure_wins_over_close_failure() ||
        test_ioctl_failure_without_errno_fails_closed() ||
        test_close_failure_is_reported_after_successful_ioctl() ||
        test_successful_ioctl_and_close_preserve_result()) {
        fprintf(stderr, "Ralink wireless ioctl error policy: FAIL\n");
        return 1;
    }

    printf("Ralink wireless ioctl errors preserve operation failure and report close failure: PASS\n");
    return 0;
}
