/* GNU89 regression tests for DHCP lease-file framing outcomes. */
#include <assert.h>
#include <stdio.h>
#include "udhcp_lease_file_read_policy.h"

static void
test_complete_header_and_record(void)
{
    assert(udhcp_lease_file_read_status(sizeof(long long), sizeof(long long)) ==
           UDHCP_LEASE_FILE_READ_COMPLETE);
    assert(udhcp_lease_file_read_status(32, 32) ==
           UDHCP_LEASE_FILE_READ_COMPLETE);
}

static void
test_eof_is_distinct_from_truncation(void)
{
    assert(udhcp_lease_file_read_status(0, 8) ==
           UDHCP_LEASE_FILE_READ_EOF);
    assert(udhcp_lease_file_read_status(1, 8) ==
           UDHCP_LEASE_FILE_READ_ERROR);
    assert(udhcp_lease_file_read_status(31, 32) ==
           UDHCP_LEASE_FILE_READ_ERROR);
    assert(udhcp_lease_file_read_status(33, 32) ==
           UDHCP_LEASE_FILE_READ_ERROR);
}

static void
test_read_errors_are_rejected(void)
{
    assert(udhcp_lease_file_read_status(-1, 8) ==
           UDHCP_LEASE_FILE_READ_ERROR);
}

int
main(void)
{
    test_complete_header_and_record();
    test_eof_is_distinct_from_truncation();
    test_read_errors_are_rejected();
    puts("DHCP lease file framing policy regression: PASS");
    return 0;
}
