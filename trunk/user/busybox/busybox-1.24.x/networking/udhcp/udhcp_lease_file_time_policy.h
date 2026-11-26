/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_LEASE_FILE_TIME_POLICY_H
#define UDHCP_LEASE_FILE_TIME_POLICY_H

#include <stdint.h>

#define UDHCP_LEASE_FILE_MAX_CLOCK_SKEW (12 * 60 * 60)

/*
 * Lease files store relative lease durations alongside a wall-clock stamp.
 * Accept a bounded future stamp caused by NTP clock correction and treat it
 * as zero elapsed time. Reject larger clock deltas and invalid timestamps.
 */
static inline int
udhcp_lease_file_time_delta_valid(int64_t now, int64_t written_at,
                                  int64_t *time_passed)
{
    if (now < 0 || written_at < 0)
        return 0;

    if (written_at > now) {
        if (written_at - now > UDHCP_LEASE_FILE_MAX_CLOCK_SKEW)
            return 0;
        *time_passed = 0;
        return 1;
    }

    *time_passed = now - written_at;
    return *time_passed <= UDHCP_LEASE_FILE_MAX_CLOCK_SKEW;
}

#endif /* UDHCP_LEASE_FILE_TIME_POLICY_H */
