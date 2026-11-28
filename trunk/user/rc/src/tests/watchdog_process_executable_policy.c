#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "watchdog_process_executable_policy.h"

static void
test_only_rc_watchdog_executable_aliases_are_accepted(void)
{
	assert(watchdog_process_executable_path_matches("/sbin/rc") == 1);
	assert(watchdog_process_executable_path_matches("/sbin/watchdog") == 1);
	assert(watchdog_process_executable_path_matches("/sbin/rc (deleted)") == 1);
	assert(watchdog_process_executable_path_matches("/sbin/watchdog (deleted)") == 1);
	assert(watchdog_process_executable_path_matches("/usr/bin/sleep") == 0);
	assert(watchdog_process_executable_path_matches("/bin/busybox") == 0);
	assert(watchdog_process_executable_path_matches("/sbin/rc-helper") == 0);
	assert(watchdog_process_executable_path_matches("/tmp/watchdog") == 0);
}

static void
test_watchdog_argv0_is_exact_basename(void)
{
	assert(watchdog_process_argv0_matches("watchdog") == 1);
	assert(watchdog_process_argv0_matches("/sbin/watchdog") == 1);
	assert(watchdog_process_argv0_matches("/tmp/watchdog") == 1);
	assert(watchdog_process_argv0_matches("watchdog-helper") == 0);
	assert(watchdog_process_argv0_matches("/sbin/rc") == 0);
	assert(watchdog_process_argv0_matches("") == 0);
}

static void
test_null_inputs_fail_closed(void)
{
	errno = 0;
	assert(watchdog_process_executable_path_matches(NULL) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(watchdog_process_argv0_matches(NULL) == -1);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_only_rc_watchdog_executable_aliases_are_accepted();
	test_watchdog_argv0_is_exact_basename();
	test_null_inputs_fail_closed();
	puts("RC watchdog executable identity policy: PASS");
	return 0;
}
