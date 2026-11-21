#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "lpd_pidfile_policy.h"

static void expect_valid(const char *text, size_t length, pid_t expected)
{
	pid_t pid = 0;
	assert(lpd_pidfile_parse(text, length, &pid) == 0);
	assert(pid == expected);
}

static void expect_invalid(const char *text, size_t length)
{
	pid_t pid = 0;
	assert(lpd_pidfile_parse(text, length, &pid) == -1);
	assert(errno == EINVAL);
}

int main(void)
{
	char maximum[32];

	expect_valid("2", 1, (pid_t)2);
	expect_valid("00042", 5, (pid_t)42);
	snprintf(maximum, sizeof(maximum), "%d", INT_MAX);
	expect_valid(maximum, strlen(maximum), (pid_t)INT_MAX);

	expect_invalid("", 0);
	expect_invalid("0", 1);
	expect_invalid("1", 1);
	expect_invalid("+2", 2);
	expect_invalid("-2", 2);
	expect_invalid(" 2", 2);
	expect_invalid("2 ", 2);
	expect_invalid("2\n", 2);
	expect_invalid("2x", 2);
	expect_invalid("2147483648", 10);
	expect_invalid("999999999999999999999999999999", 30);
	expect_invalid("12345678901234567890123456789012", 32);

	puts("LPD PID-file parser regression: PASS");
	return 0;
}
