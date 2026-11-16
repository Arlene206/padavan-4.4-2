#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ipv6_restart_policy.h"

static void
check_runtime_type(int restart_status, int previous_type,
		   int configured_type, int expected_type)
{
	int runtime_type = previous_type;

	if (ipv6_restart_should_publish_type(restart_status))
		runtime_type = configured_type;

	assert(runtime_type == expected_type);
}

int
main(void)
{
	check_runtime_type(0, 0, 1, 1);
	check_runtime_type(0, 1, 0, 0);
	check_runtime_type(EBUSY, 0, 1, 0);
	check_runtime_type(EIO, 1, 0, 1);
	check_runtime_type(-1, 0, 1, 0);
	check_runtime_type(-1, 1, 0, 1);
	puts("IPv6 restart runtime-mode commit policy: PASS");
	return 0;
}
