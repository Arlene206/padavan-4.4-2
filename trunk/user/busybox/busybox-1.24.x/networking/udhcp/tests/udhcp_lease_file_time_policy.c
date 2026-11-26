/* GNU89 regression tests for DHCP lease-file clock correction. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "udhcp_lease_file_time_policy.h"

static void
test_elapsed_time_within_existing_age_limit(void)
{
    int64_t elapsed = -1;

    assert(udhcp_lease_file_time_delta_valid(1001, 1000, &elapsed));
    assert(elapsed == 1);
    assert(udhcp_lease_file_time_delta_valid(44200, 1000, &elapsed));
    assert(elapsed == UDHCP_LEASE_FILE_MAX_CLOCK_SKEW);
    assert(!udhcp_lease_file_time_delta_valid(44201, 1000, &elapsed));
    assert(udhcp_lease_file_time_delta_valid(1000, 1000, &elapsed));
    assert(elapsed == 0);
}

static void
test_bounded_backward_clock_correction(void)
{
    int64_t elapsed = -1;

    assert(udhcp_lease_file_time_delta_valid(999, 1000, &elapsed));
    assert(elapsed == 0);
    assert(udhcp_lease_file_time_delta_valid(1000, 1000 + UDHCP_LEASE_FILE_MAX_CLOCK_SKEW,
                                             &elapsed));
    assert(elapsed == 0);
    assert(!udhcp_lease_file_time_delta_valid(1000,
                                               1001 + UDHCP_LEASE_FILE_MAX_CLOCK_SKEW,
                                               &elapsed));
}

static void
test_invalid_timestamps_are_rejected(void)
{
    int64_t elapsed = -1;

    assert(!udhcp_lease_file_time_delta_valid(-1, 1000, &elapsed));
    assert(!udhcp_lease_file_time_delta_valid(1000, -1, &elapsed));
}

int
main(void)
{
    test_elapsed_time_within_existing_age_limit();
    test_bounded_backward_clock_correction();
    test_invalid_timestamps_are_rejected();
    puts("DHCP lease file clock policy regression: PASS");
    return 0;
}
