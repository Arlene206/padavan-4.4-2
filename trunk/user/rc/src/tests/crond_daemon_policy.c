#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "crond_daemon_policy.h"

static void test_launcher_failure_is_not_hidden(void)
{
	assert(crond_start_postcondition(-1, 1) == -1);
	assert(crond_start_postcondition(127, 1) == -1);
}

static void test_detached_child_must_be_present(void)
{
	assert(crond_start_postcondition(0, 0) == -1);
	assert(crond_start_postcondition(0, 1) == 0);
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
	test_restart_waits_for_complete_stop();
	puts("crond daemon startup and restart policy: PASS");
	return 0;
}
