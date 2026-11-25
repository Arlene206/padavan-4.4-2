#include <stdio.h>
#include <errno.h>

#include "detect_link_notification_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_success_ignores_stale_errno(void)
{
    CHECK(detect_link_notification_status(0, EACCES) == 0);
    return 0;
}

static int
test_positive_launcher_status_is_preserved(void)
{
    CHECK(detect_link_notification_status(17, EACCES) == 17);
    return 0;
}

static int
test_signal_failure_preserves_errno(void)
{
    CHECK(detect_link_notification_status(-1, ESRCH) == ESRCH);
    return 0;
}

static int
test_errno_less_failure_is_not_reported_as_success(void)
{
    CHECK(detect_link_notification_status(-1, 0) == EIO);
    return 0;
}

int
main(void)
{
    CHECK(test_success_ignores_stale_errno() == 0);
    CHECK(test_positive_launcher_status_is_preserved() == 0);
    CHECK(test_signal_failure_preserves_errno() == 0);
    CHECK(test_errno_less_failure_is_not_reported_as_success() == 0);
    puts("detect_link monitor notification result policy: PASS");
    return 0;
}
