#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "process_pidfile.h"

static void expect_valid(const char *record, size_t length,
			 pid_t expected_pid, unsigned long long expected_start)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_record(record, length, &pid, &start_time) == 0);
	assert(pid == expected_pid);
	assert(start_time == expected_start);
}

static void expect_invalid(const char *record, size_t length)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_record(record, length, &pid, &start_time) == -1);
}

int main(void)
{
	expect_valid("2 1", 3, (pid_t)2, 1ULL);
	expect_valid("00042 98765\n", 12, (pid_t)42, 98765ULL);
	expect_invalid("", 0);
	expect_invalid("2", 1);
	expect_invalid("1 2", 3);
	expect_invalid("0 2", 3);
	expect_invalid("2 0", 3);
	expect_invalid(" 2 3", 4);
	expect_invalid("2  3", 4);
	expect_invalid("2 3 ", 4);
	expect_invalid("2 3\nX", 5);
	expect_invalid("+2 3", 4);
	expect_invalid("2 +3", 4);
	expect_invalid("2147483648 3", 12);
	expect_invalid("2 18446744073709551616", 22);
	expect_invalid("1234567890123456789012345678901234567890123456789012345678901234", 64);
	puts("process PID-file record regression: PASS");
	return 0;
}
