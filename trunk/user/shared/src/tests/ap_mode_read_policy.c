#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ap_mode_read_policy.h"

static void
test_missing_and_routed_settings_keep_router_mode(void)
{
    int mode = 7;

    assert(rc_ap_mode_read_status(0, 0, NULL, &mode) == 0 && mode == 0);
    assert(rc_ap_mode_read_status(1, 0, "IP_Routed", &mode) == 0 && mode == 0);
    assert(rc_ap_mode_read_status(1, 0, "", &mode) == 0 && mode == 0);
}

static void
test_bridged_setting_selects_access_point_mode_exactly(void)
{
    int mode = 0;

    assert(rc_ap_mode_read_status(1, 0, "IP_Bridged", &mode) == 0 && mode == 1);
    assert(rc_ap_mode_read_status(1, 0, "ip_bridged", &mode) == 0 && mode == 0);
    assert(rc_ap_mode_read_status(1, 0, "IP_Bridged-extra", &mode) == 0 && mode == 0);
}

static void
test_backend_failures_preserve_output_and_errno(void)
{
    int mode = 7;

    assert(rc_ap_mode_read_status(-1, EACCES, NULL, &mode) == EACCES);
    assert(mode == 7);
    assert(rc_ap_mode_read_status(-1, 0, NULL, &mode) == EIO);
    assert(mode == 7);
}

static void
test_inconsistent_results_are_rejected_without_changing_output(void)
{
    int mode = 7;

    assert(rc_ap_mode_read_status(0, 0, "IP_Bridged", &mode) == EIO);
    assert(mode == 7);
    assert(rc_ap_mode_read_status(1, 0, NULL, &mode) == EIO);
    assert(mode == 7);
    assert(rc_ap_mode_read_status(2, 0, "IP_Bridged", &mode) == EIO);
    assert(mode == 7);
    assert(rc_ap_mode_read_status(1, 0, "IP_Routed", NULL) == EINVAL);
}

int
main(void)
{
    test_missing_and_routed_settings_keep_router_mode();
    test_bridged_setting_selects_access_point_mode_exactly();
    test_backend_failures_preserve_output_and_errno();
    test_inconsistent_results_are_rejected_without_changing_output();
    puts("AP/router mode checked NVRAM read regression: PASS");
    return 0;
}
