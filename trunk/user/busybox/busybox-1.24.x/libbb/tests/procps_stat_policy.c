#include <stdio.h>
#include <string.h>

#include "procps_stat_policy.h"

static unsigned int failures;

#define CHECK(condition, label) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); \
		++failures; \
	} \
} while (0)

static void
expect_valid(char *record, const char *expected, const char *label)
{
	char *start = NULL;
	char *end = NULL;
	size_t expected_length = strlen(expected);
	int result = procps_stat_comm_bounds(record, strlen(record),
	                                     &start, &end);

	CHECK(result == 1, label);
	if (!result)
		return;
	CHECK(end >= start, "valid command bounds are ordered");
	CHECK((size_t)(end - start) == expected_length,
	      "valid command length is exact");
	if ((size_t)(end - start) == expected_length)
		CHECK(memcmp(start, expected, expected_length) == 0,
		      "valid command bytes are preserved");
}

static void
expect_invalid(char *record, size_t length, const char *label)
{
	char *start = record;
	char *end = record;
	int result = procps_stat_comm_bounds(record, length, &start, &end);

	CHECK(result == 0, label);
	CHECK(start == NULL && end == NULL,
	      "failed parse clears both output pointers");
}

int
main(void)
{
	char normal[] =
		"123 (daemon worker) S 1 2 3 0 -1 0 0 0 0 0 0 0 20 0 1 0 12345 4096 1";
	char nested[] =
		"124 (worker (nested) name with ) marker) R 1 2 3 0 -1 0 0 0 0 0 0 0 20 0 1 0 12345 4096 1";
	char empty_comm[] = "125 () S 1 2";
	char empty[] = "";
	char no_open[] = "126 daemon) S 1";
	char no_close[] = "127 (daemon S 1";
	char bad_pid[] = "x (daemon) S 1";
	char extra_pid_space[] = "128  (daemon) S 1";
	char no_state_separator[] = "129 (daemon)S 1";
	char no_state_field[] = "130 (daemon) ";
	char embedded_nul[] = {
		'1', '3', '1', ' ', '(', 'd', 'a', 'e', 'm', 'o', 'n',
		'\0', ')', ' ', 'S', ' ', '1', '\0'
	};
	char *start = (char *)normal;
	char *end = (char *)normal;
	int result;

	expect_valid(normal, "daemon worker", "normal /proc stat record");
	expect_valid(nested, "worker (nested) name with ) marker",
	             "comm containing nested and unmatched right parentheses");
	expect_valid(empty_comm, "", "empty comm field remains bounded");

	expect_invalid(empty, 0, "empty read is rejected");
	expect_invalid(no_open, strlen(no_open), "missing opening parenthesis");
	expect_invalid(no_close, strlen(no_close), "missing closing parenthesis");
	expect_invalid(bad_pid, strlen(bad_pid), "non-decimal PID prefix");
	expect_invalid(extra_pid_space, strlen(extra_pid_space),
	               "multiple spaces before comm");
	expect_invalid(no_state_separator, strlen(no_state_separator),
	               "missing separator after comm");
	expect_invalid(no_state_field, strlen(no_state_field),
	               "missing state field");
	expect_invalid(embedded_nul, sizeof(embedded_nul) - 1,
	               "embedded NUL in proc stat read");

	result = procps_stat_comm_bounds(NULL, 0, &start, &end);
	CHECK(result == 0 && start == NULL && end == NULL,
	      "NULL input clears outputs and is rejected");
	start = (char *)normal;
	end = (char *)normal;
	result = procps_stat_comm_bounds(normal, strlen(normal), NULL, &end);
	CHECK(result == 0, "NULL start output is rejected");
	result = procps_stat_comm_bounds(normal, strlen(normal), &start, NULL);
	CHECK(result == 0, "NULL end output is rejected");

	if (failures) {
		fprintf(stderr, "procps stat parser regression: %u failure(s)\n",
		        failures);
		return 1;
	}
	puts("BusyBox procps stat parser regression: PASS");
	return 0;
}
