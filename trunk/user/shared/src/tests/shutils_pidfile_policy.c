#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "shutils_pidfile_policy.h"

static void expect_valid(const char *text, size_t length, pid_t expected)
{
	pid_t pid = 0;

	errno = 0;
	assert(shutils_pidfile_parse_record(text, length, &pid) == 0);
	assert(pid == expected);
}

static void expect_invalid(const char *text, size_t length, int expected_errno)
{
	pid_t pid = (pid_t)99;

	errno = 0;
	assert(shutils_pidfile_parse_record(text, length, &pid) == -1);
	assert(errno == expected_errno);
	assert(pid == (pid_t)99);
}

int main(void)
{
	char too_long[64];
	char overflow[32];

	expect_valid("2", 1, (pid_t)2);
	expect_valid(" 42\n", 4, (pid_t)42);
	expect_valid("+42\r\n", 5, (pid_t)42);
	expect_valid("052", 3, (pid_t)42);
	expect_valid("0x2a\n", 5, (pid_t)42);
	expect_valid("2147483647", 10, (pid_t)INT_MAX);

	expect_invalid(NULL, 1, EINVAL);
	expect_invalid("2", 0, EINVAL);
	expect_invalid("", 0, EINVAL);
	expect_invalid("   \n", 4, EINVAL);
	expect_invalid("0", 1, EINVAL);
	expect_invalid("1", 1, EINVAL);
	expect_invalid("-2", 2, EINVAL);
	expect_invalid("2junk", 5, EINVAL);
	expect_invalid("2\n3", 3, EINVAL);
	expect_invalid("2 3", 3, EINVAL);
	expect_invalid("0x", 2, EINVAL);
	expect_invalid("2\0junk", 6, EINVAL);
	expect_invalid("2147483648", 10, ERANGE);
	snprintf(overflow, sizeof(overflow), "%s", "99999999999999999999999999999");
	expect_invalid(overflow, strlen(overflow), ERANGE);
	memset(too_long, '9', sizeof(too_long));
	expect_invalid(too_long, sizeof(too_long), EINVAL);

	puts("legacy shared PID-file parser regression: PASS");
	return 0;
}
