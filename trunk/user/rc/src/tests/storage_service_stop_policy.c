#include <stdio.h>

#include "storage_service_stop_policy.h"

static int
check_case(const char *name, int command_status, int daemon_running, int expected)
{
	int actual = storage_service_stop_complete(command_status, daemon_running);

	if (actual != expected) {
		fprintf(stderr, "%s: got %d, expected %d\n", name, actual, expected);
		return 1;
	}
	return 0;
}

static int
check_merged_status(const char *name, int current_status, int next_status, int expected)
{
	int actual = storage_service_stop_status_merge(current_status, next_status);

	if (actual != expected) {
		fprintf(stderr, "%s: got %d, expected %d\n", name, actual, expected);
		return 1;
	}
	return 0;
}

int
main(void)
{
	if (check_case("successful stop", 0, 0, 0) ||
	    check_case("command failure", 1, 0, -1) ||
	    check_case("daemon remains", 0, 1, -1) ||
	    check_case("command and daemon failure", -1, 1, -1) ||
	    check_case("negative process query", 0, -1, -1) ||
	    check_merged_status("group stop success", 0, 0, 0) ||
	    check_merged_status("second daemon failure", 0, -1, -1) ||
	    check_merged_status("preserve first daemon failure", -2, -1, -2))
		return 1;

	puts("storage daemon stop postcondition and group result policy: PASS");
	return 0;
}
