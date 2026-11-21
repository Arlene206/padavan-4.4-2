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
	if (!storage_service_failed_start_may_leave_daemon(0) ||
	    !storage_service_failed_start_may_leave_daemon(1) ||
	    !storage_service_failed_start_may_leave_daemon(-1) ||
	    !storage_service_failed_start_may_leave_daemon(9))
		return 1;

	if (check_case("successful stop", 0, 0, 0) ||
	    check_case("command failure", 1, 0, -1) ||
	    check_case("daemon remains", 0, 1, -1) ||
	    check_case("command and daemon failure", -1, 1, -1) ||
	    check_case("negative process query", 0, -1, -1) ||
	    check_merged_status("group stop success", 0, 0, 0) ||
	    check_merged_status("second daemon failure", 0, -1, -1) ||
	    check_merged_status("preserve first daemon failure", -2, -1, -2) ||
	    check_merged_status("rollback preserves stop failure", -1, -2, -1) ||
	    storage_service_process_presence_changed(0, 0) ||
	    storage_service_process_presence_changed(1, 1) ||
	    !storage_service_process_presence_changed(0, 1) ||
	    !storage_service_process_presence_changed(1, 0) ||
	    !storage_service_process_presence_changed(-1, 0) ||
	    !storage_service_process_presence_changed(1, -1))
		return 1;

	if (storage_safe_remove_complete(0, 0, 0) != 0 ||
	    storage_safe_remove_complete(-1, 0, 0) == 0 ||
	    storage_safe_remove_complete(0, -1, 0) == 0 ||
	    storage_safe_remove_complete(0, 0, 1) == 0 ||
	    storage_safe_remove_complete(0, 0, -1) == 0)
		return 1;
	puts("storage daemon stop and tri-state presence policies: PASS");
	return 0;
}
