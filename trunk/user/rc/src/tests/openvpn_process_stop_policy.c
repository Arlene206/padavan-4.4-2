#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include "openvpn_process_stop_policy.h"

static void
expect_pid(const char *text, size_t length, pid_t expected)
{
	pid_t pid = 0;
	assert(openvpn_process_stop_parse_pid(text, length, &pid) == 0);
	assert(pid == expected);
}

static void
expect_invalid(const char *text, size_t length, int expected_errno)
{
	pid_t pid = 0;
	errno = 0;
	assert(openvpn_process_stop_parse_pid(text, length, &pid) == -1);
	assert(errno == expected_errno);
}

int
main(void)
{
	expect_pid("4242", 4, (pid_t)4242);
	expect_pid("4242\n", 5, (pid_t)4242);
	expect_pid("00042\n", 6, (pid_t)42);
	expect_pid("2147483647", 10, (pid_t)INT_MAX);
	expect_invalid(NULL, 0, EINVAL);
	expect_invalid("", 0, EINVAL);
	expect_invalid("0", 1, EINVAL);
	expect_invalid("1\n", 2, EINVAL);
	expect_invalid("+42", 3, EINVAL);
	expect_invalid("-42", 3, EINVAL);
	expect_invalid("42 ", 3, EINVAL);
	expect_invalid("42\n\n", 4, EINVAL);
	expect_invalid("2147483648", 10, ERANGE);
	assert(openvpn_process_stop_result(PROCESS_PRESENCE_ABSENT, 0) == 0);
	assert(openvpn_process_stop_result(PROCESS_PRESENCE_RUNNING, 0) == -1);
	assert(openvpn_process_stop_result(PROCESS_PRESENCE_UNKNOWN, 0) == -1);
	assert(openvpn_process_stop_result(PROCESS_PRESENCE_ABSENT, EIO) == EIO);
	assert(openvpn_process_stop_result(PROCESS_PRESENCE_RUNNING, EPERM) == EPERM);
	puts("OpenVPN numeric PID and fail-closed stop policy: PASS");
	return 0;
}
