#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "usb_printer_child_reap_policy.h"

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s (errno=%d)\n", __FILE__, __LINE__, #expr, errno); return 1; } } while (0)

static int fake_calls;
static pid_t fake_child;
static int fake_status;

static pid_t
fake_waitpid_retry(pid_t child, int *status, int options)
{
    fake_calls++;
    if (child != fake_child || options != 0) {
        errno = EINVAL;
        return -1;
    }
    if (fake_calls == 1) {
        errno = EINTR;
        return -1;
    }
    *status = fake_status;
    return child;
}

static pid_t
fake_waitpid_no_child(pid_t child, int *status, int options)
{
    (void)child;
    (void)status;
    (void)options;
    errno = ECHILD;
    return -1;
}

static int
test_waitpid_retries_eintr(void)
{
    int status = 0;

    fake_calls = 0;
    fake_child = 321;
    fake_status = 127 << 8;
    CHECK(usb_printer_reap_failed_exec_child(fake_child, &status, fake_waitpid_retry) == 0);
    CHECK(fake_calls == 2);
    CHECK(status == fake_status);
    return 0;
}

static int
test_invalid_arguments_fail(void)
{
    int status = 0;

    errno = 0;
    CHECK(usb_printer_reap_failed_exec_child(0, &status, fake_waitpid_retry) == EINVAL);
    CHECK(errno == EINVAL);
    errno = 0;
    CHECK(usb_printer_reap_failed_exec_child(1, NULL, fake_waitpid_retry) == EINVAL);
    CHECK(errno == EINVAL);
    errno = 0;
    CHECK(usb_printer_reap_failed_exec_child(1, &status, NULL) == EINVAL);
    CHECK(errno == EINVAL);
    return 0;
}

static int
test_waitpid_error_is_preserved(void)
{
    int status = 0;

    errno = 0;
    CHECK(usb_printer_reap_failed_exec_child(321, &status, fake_waitpid_no_child) == ECHILD);
    CHECK(errno == ECHILD);
    return 0;
}

static int
test_real_exec_failure_child_is_reaped(void)
{
    pid_t child;
    int status = 0;

    child = fork();
    CHECK(child >= 0);
    if (child == 0)
        _exit(127);

    CHECK(usb_printer_reap_failed_exec_child(child, &status, waitpid) == 0);
    CHECK(WIFEXITED(status));
    CHECK(WEXITSTATUS(status) == 127);

    errno = 0;
    CHECK(waitpid(child, &status, WNOHANG) == -1);
    CHECK(errno == ECHILD);
    return 0;
}

int
main(void)
{
    CHECK(test_waitpid_retries_eintr() == 0);
    CHECK(test_invalid_arguments_fail() == 0);
    CHECK(test_waitpid_error_is_preserved() == 0);
    CHECK(test_real_exec_failure_child_is_reaped() == 0);
    puts("USB printer failed-exec child reap policy: PASS");
    return 0;
}
