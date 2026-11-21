#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ddns_restart_policy.h"
#include "rc_notify_batch_policy.h"

static int start_calls;
static int stop_calls;
static int start_status;
static int stop_status;
static int start_clear_cache;

static int
mock_start(int clear_cache)
{
    start_calls++;
    start_clear_cache = clear_cache;
    return start_status;
}

static int
mock_stop(void)
{
    stop_calls++;
    return stop_status;
}

static void
reset_case(int start_rc, int stop_rc)
{
    start_calls = 0;
    stop_calls = 0;
    start_status = start_rc;
    stop_status = stop_rc;
    start_clear_cache = -1;
}

static void
test_enabled_router_uses_one_start_transaction(void)
{
    reset_case(0, 0);
    assert(rc_ddns_restart_run(0, 1, mock_start, mock_stop) == 0);
    assert(start_calls == 1 && stop_calls == 0);
    assert(start_clear_cache == 1);
    assert(rc_notify_merge_action_result(0, 0) == 0);
}

static void
test_enabled_router_propagates_start_failure(void)
{
    reset_case(EIO, 0);
    assert(rc_ddns_restart_run(0, 1, mock_start, mock_stop) == EIO);
    assert(start_calls == 1 && stop_calls == 0);
    assert(start_clear_cache == 1);
    assert(rc_notify_merge_action_result(0, start_status) == -1);
}

static void
test_disabled_ddns_stops_without_starting(void)
{
    reset_case(0, 0);
    assert(rc_ddns_restart_run(0, 0, mock_start, mock_stop) == 0);
    assert(start_calls == 0 && stop_calls == 1);
    assert(start_clear_cache == -1);
}

static void
test_disabled_ddns_propagates_stop_failure(void)
{
    reset_case(0, EBUSY);
    assert(rc_ddns_restart_run(0, 0, mock_start, mock_stop) == EBUSY);
    assert(start_calls == 0 && stop_calls == 1);
    assert(rc_notify_merge_action_result(0, stop_status) == -1);
}

static void
test_ap_mode_stops_even_when_ddns_is_enabled(void)
{
    reset_case(0, 0);
    assert(rc_ddns_restart_run(1, 1, mock_start, mock_stop) == 0);
    assert(start_calls == 0 && stop_calls == 1);
}

static void
test_ap_mode_propagates_stop_failure(void)
{
    reset_case(0, EIO);
    assert(rc_ddns_restart_run(1, 1, mock_start, mock_stop) == EIO);
    assert(start_calls == 0 && stop_calls == 1);
    assert(rc_notify_merge_action_result(0, stop_status) == -1);
}

static void
test_invalid_callbacks_fail_without_side_effects(void)
{
    reset_case(0, 0);
    assert(rc_ddns_restart_run(0, 1, NULL, mock_stop) == EINVAL);
    assert(rc_ddns_restart_run(0, 0, mock_start, NULL) == EINVAL);
    assert(start_calls == 0 && stop_calls == 0);
}

int
main(void)
{
    test_enabled_router_uses_one_start_transaction();
    test_enabled_router_propagates_start_failure();
    test_disabled_ddns_stops_without_starting();
    test_disabled_ddns_propagates_stop_failure();
    test_ap_mode_stops_even_when_ddns_is_enabled();
    test_ap_mode_propagates_stop_failure();
    test_invalid_callbacks_fail_without_side_effects();
    puts("RC DDNS restart result policy: PASS");
    return 0;
}
