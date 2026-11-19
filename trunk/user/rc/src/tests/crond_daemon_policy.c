#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "crond_daemon_policy.h"

static void test_launcher_failure_is_not_hidden(void)
{
	assert(crond_start_postcondition(-1, PROCESS_PRESENCE_RUNNING) == -1);
	assert(crond_start_postcondition(127, PROCESS_PRESENCE_UNKNOWN) == -1);
}

static void test_detached_child_must_be_present(void)
{
	assert(crond_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == -1);
	assert(crond_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(crond_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(crond_start_postcondition(0, (enum process_presence)99) == -1);
}

static void test_probe_wait_errors_use_rc_failure_convention(void)
{
	assert(crond_start_probe_wait_status(0) == 0);
	assert(crond_start_probe_wait_status(EINTR) == -1);
	assert(crond_start_probe_wait_status(EIO) == -1);
	assert(crond_start_probe_wait_status(127) == -1);
	assert(crond_start_probe_wait_status(-1) == -1);
}

static void test_restart_waits_for_complete_stop(void)
{
	assert(crond_restart_can_start(0));
	assert(!crond_restart_can_start(-1));
	assert(!crond_restart_can_start(EBUSY));
	assert(!crond_restart_can_start(EPERM));
}

int main(void)
{
	test_launcher_failure_is_not_hidden();
	test_detached_child_must_be_present();
	test_probe_wait_errors_use_rc_failure_convention();
	test_restart_waits_for_complete_stop();
	puts("crond daemon startup and restart policy: PASS");
	return 0;
}
