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
static int mock_route_restore_sequence[2];
static unsigned int mock_route_restore_sequence_count;
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
	int call = mock_route_restore_calls++;

	mock_route_restore_ifname = lan_ifname;
	if ((unsigned int)call < mock_route_restore_sequence_count)
		return mock_route_restore_sequence[call];
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
	mock_route_restore_sequence[0] = 0;
	mock_route_restore_sequence[1] = 0;
	mock_route_restore_sequence_count = 0;
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
test_empty_optional_interface_selects_only_required_bridge(void)
{
	char *ifnames[2] = { NULL, NULL };
	unsigned int count = 99;

	assert(rc_wan_route_flush_select_interfaces(
		"", "br0", ifnames, 2, &count) == 0);
	assert(count == 1);
	assert(strcmp(ifnames[0], "br0") == 0);

	/* An empty optional interface must not become an EINVAL flush target. */
	reset_case(0, 0);
	assert(rc_wan_route_flush_and_cache_apply(
		ifnames, count, mock_flush_route, mock_flush_route_cache) == 0);
	assert(mock_calls == 1);
	assert(strcmp(mock_ifnames[0], "br0") == 0);
	assert(mock_cache_calls == 1);
}

static void
test_manual_interface_and_bridge_are_both_selected_in_order(void)
{
	char *ifnames[2] = { NULL, NULL };
	unsigned int count = 0;

	assert(rc_wan_route_flush_select_interfaces(
		"eth3", "br0", ifnames, 2, &count) == 0);
	assert(count == 2);
	assert(strcmp(ifnames[0], "eth3") == 0);
	assert(strcmp(ifnames[1], "br0") == 0);
}

static void
test_route_interface_selection_rejects_missing_bridge_and_small_buffer(void)
{
	char *ifnames[2] = { NULL, NULL };
	unsigned int count = 99;

	assert(rc_wan_route_flush_select_interfaces(
		"eth3", "", ifnames, 2, &count) == EINVAL);
	assert(count == 0);
	assert(rc_wan_route_flush_select_interfaces(
		"eth3", "br0", ifnames, 1, &count) == ENOSPC);
	assert(count == 0);
	assert(rc_wan_route_flush_select_interfaces(
		NULL, "br0", ifnames, 0, &count) == EINVAL);
	assert(count == 0);
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
test_transient_route_restore_failure_is_retried(void)
{
	int restore_status;
	int status;

	reset_case(0, 0);
	mock_route_restore_sequence[0] = EAGAIN;
	mock_route_restore_sequence[1] = 0;
	mock_route_restore_sequence_count = 2;
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 2);
}

static void
test_persistent_route_restore_failure_preserves_primary_error(void)
{
	int restore_status;
	int status;

	reset_case(0, 0);
	mock_route_restore_sequence[0] = EACCES;
	mock_route_restore_sequence[1] = EPERM;
	mock_route_restore_sequence_count = 2;
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == EPERM);
	assert(mock_route_restore_calls == 2);
}

static void
test_failed_reset_restores_lan_routes_and_preserves_first_error(void)
{
	int restore_status;
	int status;

	/* A transient failure is retried; a successful repair keeps reset first. */
	reset_case(0, 0);
	mock_route_restore_sequence[0] = EAGAIN;
	mock_route_restore_sequence[1] = 0;
	mock_route_restore_sequence_count = 2;
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 2);
	assert(strcmp(mock_route_restore_ifname, "br0") == 0);

	/* Persistent repair failure is observable without replacing reset error. */
	reset_case(0, 0);
	mock_route_restore_sequence[0] = EACCES;
	mock_route_restore_sequence[1] = EPERM;
	mock_route_restore_sequence_count = 2;
	status = rc_wan_restore_lan_routes_after_failed_reset(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == EPERM);
	assert(mock_route_restore_calls == 2);

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
test_failed_step_restores_lan_routes_and_preserves_first_error(void)
{
	int restore_status;
	int status;

	/* A successful route repair must not hide the original MAC-table error. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_step(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 1);
	assert(strcmp(mock_route_restore_ifname, "br0") == 0);

	/* A route repair failure is reported separately; the primary error wins. */
	reset_case(0, 0);
	mock_route_restore_status = EPERM;
	status = rc_wan_restore_lan_routes_after_failed_step(
		EIO, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == EIO);
	assert(restore_status == EPERM);
	assert(mock_route_restore_calls == 2);

	/* A zero primary status never invokes the recovery callback. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_step(
		0, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == 0);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 0);

	/* Invalid recovery metadata is reported without masking the primary. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_step(
		EIO, "br0", NULL, &restore_status);
	assert(status == EIO);
	assert(restore_status == EINVAL);
	assert(mock_route_restore_calls == 0);
}

static void
test_failed_route_flush_restores_lan_routes_and_preserves_first_error(void)
{
	int restore_status;
	int status;

	/* Interface/cache flush may have removed bridge routes before reporting an error. */
	reset_case(0, 0);
	status = rc_wan_restore_lan_routes_after_failed_step(
		ENOSPC, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == ENOSPC);
	assert(restore_status == 0);
	assert(mock_route_restore_calls == 1);
	assert(strcmp(mock_route_restore_ifname, "br0") == 0);

	/* Recovery failure is observable, while the route-flush error remains primary. */
	reset_case(0, 0);
	mock_route_restore_status = EPERM;
	status = rc_wan_restore_lan_routes_after_failed_step(
		ENOSPC, "br0", mock_restore_static_lan_routes, &restore_status);
	assert(status == ENOSPC);
	assert(restore_status == EPERM);
	assert(mock_route_restore_calls == 2);
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
	test_empty_optional_interface_selects_only_required_bridge();
	test_manual_interface_and_bridge_are_both_selected_in_order();
	test_route_interface_selection_rejects_missing_bridge_and_small_buffer();
	test_successful_flush_returns_zero();
	test_invalid_interface_does_not_skip_other_cleanup();
	test_global_cache_flush_is_attempted_and_errors_are_preserved();
	test_transient_route_restore_failure_is_retried();
	test_persistent_route_restore_failure_preserves_primary_error();
	test_failed_reset_restores_lan_routes_and_preserves_first_error();
	test_failed_step_restores_lan_routes_and_preserves_first_error();
	test_failed_route_flush_restores_lan_routes_and_preserves_first_error();
	test_invalid_policy_arguments_fail_closed();
	puts("WAN route flush, bounded LAN recovery retry and first-error policy: PASS");
	return 0;
}
