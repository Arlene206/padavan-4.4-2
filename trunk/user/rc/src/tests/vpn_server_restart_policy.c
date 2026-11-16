#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "vpn_server_restart_policy.h"
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

int
main(void)
{
    test_success_stays_success();
    test_start_failure_is_returned();
    test_restore_failure_is_returned_when_start_succeeded();
    test_first_failure_survives_later_restore_and_unlock_errors();
    test_unlock_failure_is_returned_after_successful_operations();
    test_positive_wait_status_is_not_lost();
    puts("VPN server restart status propagation policy: PASS");
    return 0;
}
