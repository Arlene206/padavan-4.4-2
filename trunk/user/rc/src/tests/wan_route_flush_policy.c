#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_route_flush_policy.h"

static int mock_status[2];
static int mock_calls;
static char *mock_ifnames[2];
static int mock_cache_status;
static int mock_cache_calls;
static int mock_route_restore_status;
static int mock_route_restore_calls;
static char *mock_route_restore_ifname;

static int
mock_flush_route(char *ifname)
{
	int index = mock_calls++;
	assert(index < 2);
	mock_ifnames[index] = ifname;
	return mock_status[index];
}

static int
mock_flush_route_cache(void)
{
	++mock_cache_calls;
	return mock_cache_status;
}

static int
mock_restore_static_lan_routes(char *lan_ifname)
{
	++mock_route_restore_calls;
	mock_route_restore_ifname = lan_ifname;
	return mock_route_restore_status;
}

static void
reset_case(int first_status, int second_status)
{
	mock_status[0] = first_status;
	mock_status[1] = second_status;
	mock_calls = 0;
	mock_cache_status = 0;
	mock_cache_calls = 0;
	mock_route_restore_status = 0;
	mock_route_restore_calls = 0;
	mock_route_restore_ifname = NULL;
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
test_global_cache_flush_is_attempted_and_errors_are_preserved(void)
{
	char *ifnames[] = { "eth3", "br0" };

	/* Global cache cleanup still runs after a per-interface route failure. */
	reset_case(EIO, EPERM);
	mock_cache_status = ENOSPC;
	assert(rc_wan_route_flush_and_cache_apply(
		ifnames, 2, mock_flush_route, mock_flush_route_cache) == EIO);
	assert(mock_calls == 2);
	assert(mock_cache_calls == 1);

	/* A cache-only failure must prevent route-dependent restart work. */
	reset_case(0, 0);
	mock_cache_status = EACCES;
	assert(rc_wan_route_flush_and_cache_apply(
		ifnames, 2, mock_flush_route, mock_flush_route_cache) == EACCES);
	assert(mock_calls == 2);
	assert(mock_cache_calls == 1);

	reset_case(0, 0);
	assert(rc_wan_route_flush_and_cache_apply(
		ifnames, 2, mock_flush_route, mock_flush_route_cache) == 0);
	assert(mock_calls == 2);
	assert(mock_cache_calls == 1);

	/* A missing cache callback is an error, but interface cleanup was tried. */
	reset_case(0, 0);
	assert(rc_wan_route_flush_and_cache_apply(
		ifnames, 2, mock_flush_route, NULL) == EINVAL);
	assert(mock_calls == 2);
	assert(mock_cache_calls == 0);
}

static void
test_failed_reset_restores_lan_routes_and_preserves_first_error(void)
{
	int restore_status;
	int status;

	/* The original reset error must survive a successful LAN route repair. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 1);
	assert(strcmp(mock_route_restore_ifname, "br0") == 0);

	/* A route repair error is reported separately without replacing reset error. */
	reset_case(0, 0);
	mock_route_restore_status = EPERM;
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == EPERM);
	assert(mock_route_restore_calls == 1);

	/* Successful protocol staging must not invoke the recovery callback. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_reset(
		0, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == 0);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 0);

	/* Missing recovery callback fails closed while preserving the reset error. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", NULL, &restore_status);
	assert(status == EIO);
	assert(restore_status == EINVAL);
	assert(mock_route_restore_calls == 0);
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
	test_global_cache_flush_is_attempted_and_errors_are_preserved();
	test_failed_reset_restores_lan_routes_and_preserves_first_error();
	test_invalid_policy_arguments_fail_closed();
	puts("WAN route flush attempt and first-error policy: PASS");
	return 0;
}
