#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "httpd_daemon_policy.h"

static void test_launcher_failure_is_preserved(void)
{
	assert(httpd_start_postcondition(-1, 1) == -1);
	assert(httpd_start_postcondition(ENOENT, 1) == ENOENT);
	assert(httpd_start_postcondition(127, 0) == 127);
}

static void test_duplicate_launch_gate_requires_authoritative_absence(void)
{
	assert(httpd_start_can_launch(PROCESS_PRESENCE_ABSENT));
	assert(!httpd_start_can_launch(PROCESS_PRESENCE_RUNNING));
	assert(!httpd_start_can_launch(PROCESS_PRESENCE_UNKNOWN));
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_ABSENT) == 0);
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_RUNNING) == EBUSY);
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(httpd_start_precondition_status((enum process_presence)99) == EIO);
}

static void test_detached_child_must_be_confirmed_running(void)
{
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == ESRCH);
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(httpd_start_postcondition(0, (enum process_presence)99) == EIO);
	assert(httpd_start_postcondition(EPERM, PROCESS_PRESENCE_RUNNING) == EPERM);
	assert(httpd_start_postcondition(127, PROCESS_PRESENCE_UNKNOWN) == 127);
}

static void test_stop_state_requires_confirmed_exit(void)
{
	assert(httpd_stop_policy_publish_stopped(0));
	assert(!httpd_stop_policy_publish_stopped(-1));
	assert(!httpd_stop_policy_publish_stopped(EBUSY));
}

static void test_firewall_failure_requires_listener_shutdown(void)
{
	assert(!httpd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_RUNNING));
	assert(!httpd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_UNKNOWN));
	assert(!httpd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_ABSENT));
	assert(httpd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_RUNNING));
	assert(httpd_firewall_failure_requires_stop(-1, PROCESS_PRESENCE_UNKNOWN));
	assert(httpd_firewall_failure_requires_stop(1, (enum process_presence)99));
}

static void test_nvram_write_status_preserves_backend_failure(void)
{
	assert(httpd_nvram_write_status(0, 0) == 0);
	assert(httpd_nvram_write_status(0, ENOSPC) == 0);
	assert(httpd_nvram_write_status(-1, ENOSPC) == ENOSPC);
	assert(httpd_nvram_write_status(-1, 0) == EIO);
	assert(httpd_nvram_write_status(1, EACCES) == EACCES);
}

static void test_restart_requires_confirmed_exit(void)
{
	assert(httpd_restart_can_start(0));
	assert(!httpd_restart_can_start(-1));
	assert(!httpd_restart_can_start(EBUSY));
	assert(!httpd_restart_can_start(EPERM));
}

int main(void)
{
	test_launcher_failure_is_preserved();
	test_duplicate_launch_gate_requires_authoritative_absence();
	test_detached_child_must_be_confirmed_running();
	test_stop_state_requires_confirmed_exit();
	test_restart_requires_confirmed_exit();
	test_nvram_write_status_preserves_backend_failure();
	test_firewall_failure_requires_listener_shutdown();
	puts("HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS");
	return 0;
}
