#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_route_flush_policy.h"

static int mock_status[2];
static int mock_calls;
static char *mock_ifnames[2];

static int
mock_flush_route(char *ifname)
{
	int index = mock_calls++;
	assert(index < 2);
	mock_ifnames[index] = ifname;
	return mock_status[index];
}

static void
reset_case(int first_status, int second_status)
{
	mock_status[0] = first_status;
	mock_status[1] = second_status;
	mock_calls = 0;
	mock_ifnames[0] = NULL;
	mock_ifnames[1] = NULL;
}

static void
test_all_interfaces_are_attempted_and_first_error_is_preserved(void)
{
	char *ifnames[] = { "eth3", "br0" };

	reset_case(EIO, EPERM);
	assert(rc_wan_route_flush_apply(ifnames, 2, mock_flush_route) == EIO);
	assert(mock_calls == 2);
	assert(strcmp(mock_ifnames[0], "eth3") == 0);
	assert(strcmp(mock_ifnames[1], "br0") == 0);

	reset_case(0, EPERM);
	assert(rc_wan_route_flush_apply(ifnames, 2, mock_flush_route) == EPERM);
	assert(mock_calls == 2);
}

static void
test_successful_flush_returns_zero(void)
{
	char *ifnames[] = { "eth3", "br0" };

	reset_case(0, 0);
	assert(rc_wan_route_flush_apply(ifnames, 2, mock_flush_route) == 0);
	assert(mock_calls == 2);
}

static void
test_invalid_interface_does_not_skip_other_cleanup(void)
{
	char *ifnames[] = { NULL, "br0" };

	reset_case(0, 0);
	assert(rc_wan_route_flush_apply(ifnames, 2, mock_flush_route) == EINVAL);
	assert(mock_calls == 1);
	assert(strcmp(mock_ifnames[0], "br0") == 0);
}

static void
test_invalid_policy_arguments_fail_closed(void)
{
	char *ifnames[] = { "eth3" };

	reset_case(0, 0);
	assert(rc_wan_route_flush_apply(NULL, 1, mock_flush_route) == EINVAL);
	assert(rc_wan_route_flush_apply(ifnames, 1, NULL) == EINVAL);
	assert(mock_calls == 0);
}

int
main(void)
{
	test_all_interfaces_are_attempted_and_first_error_is_preserved();
	test_successful_flush_returns_zero();
	test_invalid_interface_does_not_skip_other_cleanup();
	test_invalid_policy_arguments_fail_closed();
	puts("WAN route flush attempt and first-error policy: PASS");
	return 0;
}
