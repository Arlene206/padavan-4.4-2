#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "vpn_server_restart_policy.h"
#include "vpn_client_restart_policy.h"
#include "rc_notify_batch_policy.h"

static void
test_success_stays_success(void)
{
    assert(vpn_server_restart_first_error(0, 0) == 0);
    assert(rc_notify_merge_action_result(0, vpn_server_restart_first_error(0, 0)) == 0);
}

static void
test_start_failure_is_returned(void)
{
    assert(vpn_server_restart_first_error(0, EIO) == EIO);
    assert(rc_notify_merge_action_result(0, vpn_server_restart_first_error(0, EIO)) == -1);
}

static void
test_restore_failure_is_returned_when_start_succeeded(void)
{
    assert(vpn_server_restart_first_error(0, EBUSY) == EBUSY);
    assert(rc_notify_merge_action_result(0, vpn_server_restart_first_error(0, EBUSY)) == -1);
}

static void
test_first_failure_survives_later_restore_and_unlock_errors(void)
{
    int status = vpn_server_restart_first_error(0, EIO);
    status = vpn_server_restart_first_error(status, EBUSY);
    status = vpn_server_restart_first_error(status, -1);
    assert(status == EIO);
    assert(rc_notify_merge_action_result(0, status) == -1);
}

static void
test_unlock_failure_is_returned_after_successful_operations(void)
{
    assert(vpn_server_restart_first_error(0, -1) == -1);
    assert(rc_notify_merge_action_result(0, vpn_server_restart_first_error(0, -1)) == -1);
}

static void
test_positive_wait_status_is_not_lost(void)
{
    assert(vpn_server_restart_first_error(0, 256) == 256);
}


static int openvpn_step_order;
static int openvpn_stop_status;
static int openvpn_clear_status;
static int openvpn_start_status;

static int
mock_openvpn_stop(void)
{
    openvpn_step_order = openvpn_step_order * 10 + 1;
    return openvpn_stop_status;
}

static int
mock_openvpn_clear_lease(void)
{
    openvpn_step_order = openvpn_step_order * 10 + 2;
    return openvpn_clear_status;
}

static int
mock_openvpn_start(void)
{
    openvpn_step_order = openvpn_step_order * 10 + 3;
    return openvpn_start_status;
}

static void
reset_openvpn_restart(int stop_status, int clear_status, int start_status)
{
    openvpn_step_order = 0;
    openvpn_stop_status = stop_status;
    openvpn_clear_status = clear_status;
    openvpn_start_status = start_status;
}

static void
test_openvpn_restart_runs_stop_clear_start_in_order(void)
{
    reset_openvpn_restart(0, 0, 0);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        mock_openvpn_clear_lease, mock_openvpn_start) == 0);
    assert(openvpn_step_order == 123);
}

static void
test_openvpn_restart_stop_failure_prevents_lease_clear_and_start(void)
{
    reset_openvpn_restart(EBUSY, 0, 0);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        mock_openvpn_clear_lease, mock_openvpn_start) == EBUSY);
    assert(openvpn_step_order == 1);
    assert(rc_notify_merge_action_result(0, EBUSY) == -1);
}

static void
test_openvpn_restart_lease_clear_failure_prevents_start(void)
{
    reset_openvpn_restart(0, EIO, 0);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        mock_openvpn_clear_lease, mock_openvpn_start) == EIO);
    assert(openvpn_step_order == 12);
}

static void
test_openvpn_restart_start_failure_is_returned(void)
{
    reset_openvpn_restart(0, 0, ESRCH);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        mock_openvpn_clear_lease, mock_openvpn_start) == ESRCH);
    assert(openvpn_step_order == 123);
}

static void
test_openvpn_restart_rejects_invalid_steps(void)
{
    reset_openvpn_restart(0, 0, 0);
    assert(vpn_openvpn_server_restart_run(NULL,
        mock_openvpn_clear_lease, mock_openvpn_start) == EINVAL);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        NULL, mock_openvpn_start) == EINVAL);
    assert(vpn_openvpn_server_restart_run(mock_openvpn_stop,
        mock_openvpn_clear_lease, NULL) == EINVAL);
    assert(openvpn_step_order == 0);
}

static void
test_shared_xl2tpd_restoration_decision(void)
{
    assert(vpn_server_restart_should_restore_shared_xl2tpd(1, 1, 0));
    assert(vpn_server_restart_should_restore_shared_xl2tpd(1, 0, 1));
    assert(vpn_server_restart_should_restore_shared_xl2tpd(1, 1, 1));
    assert(!vpn_server_restart_should_restore_shared_xl2tpd(0, 1, 0));
    assert(!vpn_server_restart_should_restore_shared_xl2tpd(0, 0, 1));
    assert(!vpn_server_restart_should_restore_shared_xl2tpd(1, 0, 0));
}

static void
test_client_restart_restores_shared_xl2tpd_after_incomplete_stop(void)
{
    assert(vpn_client_restart_should_restore_shared_xl2tpd(1, 1, 0));
    assert(vpn_client_restart_should_restore_shared_xl2tpd(1, 0, 1));
    assert(vpn_client_restart_should_restore_shared_xl2tpd(1, 1, 1));
    assert(!vpn_client_restart_should_restore_shared_xl2tpd(0, 1, 0));
    assert(!vpn_client_restart_should_restore_shared_xl2tpd(0, 0, 1));
    assert(!vpn_client_restart_should_restore_shared_xl2tpd(1, 0, 0));
}

static void
test_shared_xl2tpd_restore_is_attempted_when_dependency_read_fails(void)
{
	assert(vpn_server_restart_should_attempt_restore_shared_xl2tpd(1, EIO, 0, 0));
	assert(vpn_server_restart_should_attempt_restore_shared_xl2tpd(1, -1, 0, 0));
	assert(!vpn_server_restart_should_attempt_restore_shared_xl2tpd(0, EIO, 0, 0));
}

int
main(void)
{
    test_success_stays_success();
    test_shared_xl2tpd_restoration_decision();
    test_shared_xl2tpd_restore_is_attempted_when_dependency_read_fails();
    test_client_restart_restores_shared_xl2tpd_after_incomplete_stop();
    test_start_failure_is_returned();
    test_restore_failure_is_returned_when_start_succeeded();
    test_first_failure_survives_later_restore_and_unlock_errors();
    test_unlock_failure_is_returned_after_successful_operations();
    test_positive_wait_status_is_not_lost();
    test_openvpn_restart_runs_stop_clear_start_in_order();
    test_openvpn_restart_stop_failure_prevents_lease_clear_and_start();
    test_openvpn_restart_lease_clear_failure_prevents_start();
    test_openvpn_restart_start_failure_is_returned();
    test_openvpn_restart_rejects_invalid_steps();
    puts("VPN server and OpenVPN restart status propagation policy: PASS");
    return 0;
}
