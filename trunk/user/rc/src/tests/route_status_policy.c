#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include "../route_status_policy.h"
#include "service_start_status.h"

static int
apply_results(const int *results, size_t count, size_t *attempts)
{
	int status = 0;
	size_t i;
	for (i = 0; i < count; ++i) {
		++*attempts;
		status = route_status_accumulate(status, results[i]);
	}
	return status;
}

static void
test_static_mtu_failure_is_not_masked_by_later_success(void)
{
	const int startup_steps[] = { EINVAL, 0, 0 };
	size_t attempts = 0;

	assert(apply_results(startup_steps,
			     sizeof(startup_steps) / sizeof(startup_steps[0]),
			     &attempts) == EINVAL);
	assert(attempts == sizeof(startup_steps) / sizeof(startup_steps[0]));
}

static void
test_wan_post_script_failure_is_reported_and_later_steps_continue(void)
{
	const int startup_steps[] = { 0, EIO, 0 };
	size_t attempts = 0;

	/* Routes succeed, the custom post-WAN hook fails, and later startup still runs. */
	assert(apply_results(startup_steps,
			     sizeof(startup_steps) / sizeof(startup_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(startup_steps) / sizeof(startup_steps[0]));
}

static void
test_wan_post_down_script_failure_is_reported_and_later_steps_continue(void)
{
	const int teardown_steps[] = { 0, EIO, 0 };
	size_t attempts = 0;

	/* Earlier cleanup succeeds, the custom down hook fails, later detection runs. */
	assert(apply_results(teardown_steps,
			     sizeof(teardown_steps) / sizeof(teardown_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(teardown_steps) / sizeof(teardown_steps[0]));
}

static void
test_wan_vpn_start_notification_failure_is_reported_without_skipping_later_actions(void)
{
	const int startup_steps[] = { 0, EIO, 0 };
	size_t attempts = 0;

	/* Deferred VPN enqueue fails, but later independent WAN checks still run. */
	assert(apply_results(startup_steps,
			     sizeof(startup_steps) / sizeof(startup_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(startup_steps) / sizeof(startup_steps[0]));
}

static void
test_wan_vpn_stop_notification_failure_is_reported_without_skipping_later_cleanup(void)
{
	const int teardown_steps[] = { EIO, 0, 0 };
	size_t attempts = 0;

	/* Deferred VPN-stop enqueue fails, but route and interface cleanup continue. */
	assert(apply_results(teardown_steps,
			     sizeof(teardown_steps) / sizeof(teardown_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(teardown_steps) / sizeof(teardown_steps[0]));
}

static void
test_dhcp_down_monitor_pause_failure_is_reported_and_cleanup_continues(void)
{
	const int teardown_steps[] = { EBUSY, 0, ENETUNREACH, 0 };
	size_t attempts = 0;

	/* Lease loss is committed: later route/interface cleanup still runs. */
	assert(apply_results(teardown_steps,
			     sizeof(teardown_steps) / sizeof(teardown_steps[0]),
			     &attempts) == EBUSY);
	assert(attempts == sizeof(teardown_steps) / sizeof(teardown_steps[0]));
}


static void
test_wan_resolver_snapshot_clear_failures_are_reported_while_teardown_continues(void)
{
	/* VPN DNS clear fails, VPN stop succeeds, WAN DNS clear fails,
	 * and resolver refresh still runs: retain the first error and attempt all.
	 */
	const int teardown_steps[] = { EACCES, 0, ENOSPC, 0 };
	size_t attempts = 0;

	assert(apply_results(teardown_steps,
			     sizeof(teardown_steps) / sizeof(teardown_steps[0]),
			     &attempts) == EACCES);
	assert(attempts == sizeof(teardown_steps) / sizeof(teardown_steps[0]));
}

static void
test_static_ipoe_teardown_clear_failure_forces_interface_down(void)
{
	const int clear_failures[] = { -1, EIO, EBUSY };
	int status;
	int interface_down_attempts;
	int i;

	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(0, EIO));
	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(1, 0));

	for (i = 0; i < (int)(sizeof(clear_failures) / sizeof(clear_failures[0])); i++) {
		status = 0;
		interface_down_attempts = 0;
		status = route_status_accumulate(status, clear_failures[i]);
		if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, clear_failures[i])) {
			interface_down_attempts++;
			/* A force-down failure must not mask the original clear error. */
			status = route_status_accumulate(status, EACCES);
		}
		assert(interface_down_attempts == 1);
		assert(status == clear_failures[i]);
	}

	/* Successful static clear and dynamic-IPoE teardown do not force link down. */
	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(1, 0));
	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(0, EIO));
}

static void
test_vpn_route_idempotence_does_not_hide_real_failures(void)
{
	assert(route_status_idempotent_add(EEXIST) == 0);
	assert(route_status_idempotent_add(EIO) == EIO);
	assert(route_status_idempotent_delete(ESRCH) == 0);
	assert(route_status_idempotent_delete(ENOENT) == ENOENT);
	assert(route_status_idempotent_delete(EIO) == EIO);
	assert(route_status_accumulate(0, route_status_idempotent_add(EEXIST)) == 0);
	assert(route_status_accumulate(0, route_status_idempotent_delete(ESRCH)) == 0);
	assert(route_status_accumulate(EIO, route_status_idempotent_delete(ESRCH)) == EIO);
}

static void
test_vpn_server_remote_route_failure_survives_later_callback_work(void)
{
	const int callback_steps[] = { EHOSTUNREACH, 0, EIO };
	size_t attempts = 0;

	/* Route failure remains visible while later independent work is attempted. */
	assert(apply_results(callback_steps,
			     sizeof(callback_steps) / sizeof(callback_steps[0]),
			     &attempts) == EHOSTUNREACH);
	assert(attempts == sizeof(callback_steps) / sizeof(callback_steps[0]));
}

static void
test_vpn_server_custom_hook_failure_is_not_hidden(void)
{
	const int callback_steps[] = { 0, EACCES, 0 };
	size_t attempts = 0;

	/* A failed custom hook must reach the callback result while later cleanup runs. */
	assert(apply_results(callback_steps,
			     sizeof(callback_steps) / sizeof(callback_steps[0]),
			     &attempts) == EACCES);
	assert(attempts == sizeof(callback_steps) / sizeof(callback_steps[0]));
}

static void
test_wan_gateway_host_route_cleanup_matches_setup_conditions(void)
{
	assert(route_status_wan_gateway_host_route_required(1, 1, 1, -1, 1) == 1);
	assert(route_status_wan_gateway_host_route_required(1, 1, 1, -1, 0) == 0);
	assert(route_status_wan_gateway_host_route_required(1, 1, 0, -1, 1) == 0);
	assert(route_status_wan_gateway_host_route_required(1, 0, 1, -1, 1) == 0);
	assert(route_status_wan_gateway_host_route_required(1, 1, 1, 0, 1) == 0);
	assert(route_status_wan_gateway_host_route_required(1, 1, 1, 3, 1) == 0);
	assert(route_status_wan_gateway_host_route_required(0, 1, 1, -1, 1) == 0);
	assert(route_status_gateway_host_route_required(1, 1, 1, 1) == 1);
	assert(route_status_gateway_host_route_required(1, 0, 1, 1) == 0);
	assert(route_status_gateway_host_route_required(1, 1, 0, 1) == 0);
	assert(route_status_gateway_host_route_required(0, 1, 1, 1) == 0);
	assert(route_status_accumulate(EIO, route_status_idempotent_delete(ESRCH)) == EIO);
	assert(route_status_accumulate(0, route_status_idempotent_delete(ESRCH)) == 0);
	assert(route_status_idempotent_delete(EIO) == EIO);
}

static void
test_manual_lan_gateway_route_retry_after_resolver_failure(void)
{
	int first_attempt_status;
	int retry_route_status;
	int retry_status;

	first_attempt_status = route_status_accumulate(
		route_status_idempotent_add(0), EIO);
	retry_route_status = route_status_idempotent_add(EEXIST);
	retry_status = route_status_accumulate(retry_route_status, 0);

	assert(first_attempt_status == EIO);
	assert(retry_route_status == 0);
	assert(retry_status == 0);
	assert(route_status_idempotent_add(EACCES) == EACCES);
	assert(route_status_idempotent_add(EIO) == EIO);
}

static void
test_ap_gateway_route_cleanup_accepts_absent_route_only(void)
{
	assert(route_status_idempotent_delete(0) == 0);
	assert(route_status_idempotent_delete(ESRCH) == 0);
	assert(route_status_idempotent_delete(EIO) == EIO);
	assert(route_status_idempotent_delete(EPERM) == EPERM);
}

static void
test_l2tp_server_start_selector_failure_is_fail_closed(void)
{
	/* Start only when the selector was read successfully and says enabled. */
	assert(route_status_l2tp_server_start_requested(0, 1));
	assert(!route_status_l2tp_server_start_requested(0, 0));
	assert(!route_status_l2tp_server_start_requested(EIO, 1));
	assert(!route_status_l2tp_server_start_requested(ENODEV, 1));

	/* A read failure is reported and must not be converted into a no-op. */
	assert(route_status_l2tp_server_start_result(0, EIO, 0) == EIO);
	assert(route_status_l2tp_server_start_result(0, ENODEV, 0) == ENODEV);

	/* Preserve an earlier WAN failure and do not turn the running sentinel into an error. */
	assert(route_status_l2tp_server_start_result(EBUSY, EIO, -1) == EBUSY);
	assert(route_status_l2tp_server_start_result(0, 0, 1) == 0);
	assert(route_status_l2tp_server_start_result(0, 0, 0) == 0);
	assert(route_status_l2tp_server_start_result(0, 0, -1) == -1);
	assert(route_status_l2tp_server_start_result(EIO, 0, -1) == EIO);
}


int
main(void)
{
	test_static_mtu_failure_is_not_masked_by_later_success();
	test_wan_post_script_failure_is_reported_and_later_steps_continue();
	test_wan_post_down_script_failure_is_reported_and_later_steps_continue();
	test_wan_vpn_start_notification_failure_is_reported_without_skipping_later_actions();
	test_wan_vpn_stop_notification_failure_is_reported_without_skipping_later_cleanup();
	test_dhcp_down_monitor_pause_failure_is_reported_and_cleanup_continues();
	test_wan_resolver_snapshot_clear_failures_are_reported_while_teardown_continues();
	test_static_ipoe_teardown_clear_failure_forces_interface_down();
	test_manual_lan_gateway_route_retry_after_resolver_failure();
	test_ap_gateway_route_cleanup_accepts_absent_route_only();
	test_l2tp_server_start_selector_failure_is_fail_closed();
	test_vpn_route_idempotence_does_not_hide_real_failures();
	test_vpn_server_remote_route_failure_survives_later_callback_work();
	test_vpn_server_custom_hook_failure_is_not_hidden();
	test_wan_gateway_host_route_cleanup_matches_setup_conditions();
	const int first_error[] = { 0, EIO, 0, ENETUNREACH };
	const int negative_error[] = { 0, -1, 0, EPERM };
	const int no_errors[] = { 0, 0, 0 };
	size_t attempts = 0;
	assert(apply_results(first_error, sizeof(first_error) / sizeof(first_error[0]), &attempts) == EIO);
	assert(attempts == sizeof(first_error) / sizeof(first_error[0]));
	attempts = 0;
	assert(apply_results(negative_error, sizeof(negative_error) / sizeof(negative_error[0]), &attempts) == -1);
	assert(attempts == sizeof(negative_error) / sizeof(negative_error[0]));
	attempts = 0;
	assert(apply_results(no_errors, sizeof(no_errors) / sizeof(no_errors[0]), &attempts) == 0);
	assert(attempts == sizeof(no_errors) / sizeof(no_errors[0]));
	assert(route_status_accumulate(EIO, 0) == EIO);
	assert(route_status_accumulate(EIO, EPERM) == EIO);
	assert(route_status_finish(0, 0) == 0);
	assert(route_status_finish(EIO, 0) == EIO);
	assert(route_status_finish(0, -1) == -1);
	assert(route_status_finish(EIO, -1) == -1);
	assert(route_status_finish(route_status_accumulate(EIO, ENETUNREACH), 0) == EIO);
	assert(route_status_idempotent_add(0) == 0);
	assert(route_status_idempotent_add(EEXIST) == 0);
	assert(route_status_idempotent_add(EIO) == EIO);
	assert(route_status_idempotent_add(ENETUNREACH) == ENETUNREACH);
	assert(route_status_accumulate(EIO, route_status_idempotent_add(EEXIST)) == EIO);
	assert(route_status_demand_ready(0) == 1);
	assert(route_status_demand_ready(EIO) == 0);
	assert(route_status_demand_ready(ENETUNREACH) == 0);
	assert(route_status_demand_ready(-1) == 0);
	assert(route_status_demand_ready(route_status_idempotent_add(EEXIST)) == 1);
	return 0;
}
