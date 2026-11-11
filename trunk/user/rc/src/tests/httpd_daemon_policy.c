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

static void test_existing_daemon_blocks_duplicate_launch(void)
{
	assert(httpd_start_can_launch(0));
	assert(!httpd_start_can_launch(1));
}

static void test_detached_child_must_be_present(void)
{
	assert(httpd_start_postcondition(0, 0) == ESRCH);
	assert(httpd_start_postcondition(0, 1) == 0);
}

static void test_stop_state_requires_confirmed_exit(void)
{
	assert(httpd_stop_policy_publish_stopped(0));
	assert(!httpd_stop_policy_publish_stopped(-1));
	assert(!httpd_stop_policy_publish_stopped(EBUSY));
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
	test_existing_daemon_blocks_duplicate_launch();
	test_detached_child_must_be_present();
	test_stop_state_requires_confirmed_exit();
	test_restart_requires_confirmed_exit();
	puts("HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS");
	return 0;
}
