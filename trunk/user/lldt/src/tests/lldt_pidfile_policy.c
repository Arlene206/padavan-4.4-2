#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>

#include "lldt_pidfile_policy.h"

static void
expect_valid(const char *text, size_t length, pid_t expected)
{
	pid_t actual = (pid_t)-1;

	errno = 0;
	assert(lldt_pidfile_parse_pid(text, length, &actual) == 0);
	assert(actual == expected);
}

static void
expect_invalid(const char *text, size_t length, int expected_errno)
{
	pid_t actual = (pid_t)123;

	errno = 0;
	assert(lldt_pidfile_parse_pid(text, length, &actual) == -1);
	assert(errno == expected_errno);
	assert(actual == (pid_t)0);
}


static int
test_pidfile_lock_serializes_competing_open(void)
{
	char lock_path[] = "/tmp/lldt-pidfile-lock.XXXXXX";
	int first_fd = -1, second_fd = -1;
	int result = 1;

	first_fd = mkstemp(lock_path);
	if (first_fd < 0)
		return 1;
	if (lldt_pidfile_try_exclusive_lock(first_fd) != 0)
		goto out;
	second_fd = open(lock_path, O_RDWR);
	if (second_fd < 0)
		goto out;
	errno = 0;
	if (lldt_pidfile_try_exclusive_lock(second_fd) == 0)
		goto out;
	if (errno == EWOULDBLOCK || errno == EAGAIN)
		result = 0;
out:
	if (second_fd >= 0)
		close(second_fd);
	if (first_fd >= 0)
		close(first_fd);
	unlink(lock_path);
	return result;
}


int
main(void)
{
	static const char no_nul[] = { '4', '2' };
	static const char embedded_nul[] = { '4', '2', '\0', '3' };
	static const char overflow[] = "2147483648";
	static const char maximum[] = "2147483647\n";
	static const char too_many_digits[] = "00000000001";
	static const char too_long[] = "123456789012";
	pid_t actual = 0;

	expect_valid("1", 1, (pid_t)1);
	expect_valid("42\n", 3, (pid_t)42);
	expect_valid("00042", 5, (pid_t)42);
	expect_valid(no_nul, sizeof(no_nul), (pid_t)42);
	expect_valid(maximum, sizeof(maximum) - 1, (pid_t)INT_MAX);

	expect_invalid(NULL, 1, EINVAL);
	expect_invalid("", 0, EINVAL);
	expect_invalid("\n", 1, EINVAL);
	expect_invalid("0", 1, EINVAL);
	expect_invalid("-1", 2, EINVAL);
	expect_invalid("+1", 2, EINVAL);
	expect_invalid(" 1", 2, EINVAL);
	expect_invalid("1x", 2, EINVAL);
	expect_invalid("1\n\n", 3, EINVAL);
	expect_invalid("1\r\n", 3, EINVAL);
	expect_invalid(embedded_nul, sizeof(embedded_nul), EINVAL);
	expect_invalid(overflow, sizeof(overflow) - 1, ERANGE);
	expect_invalid(too_many_digits, sizeof(too_many_digits) - 1, EINVAL);
	expect_invalid(too_long, sizeof(too_long) - 1, EINVAL);
	expect_invalid("2147483647\n\n", 12, EINVAL);

	errno = 0;
	assert(lldt_pidfile_parse_pid("1", 1, NULL) == -1);
	assert(errno == EINVAL);
	assert(lldt_pidfile_parse_pid(NULL, 0, &actual) == -1);
	assert(errno == EINVAL);

	puts("LLTD bounded PID-file parser regression: PASS");
	if (test_pidfile_lock_serializes_competing_open() != 0)
		return 1;
	return 0;
}
