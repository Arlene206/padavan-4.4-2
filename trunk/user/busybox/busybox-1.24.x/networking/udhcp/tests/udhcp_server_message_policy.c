/* GNU89 regression tests for DHCP server DECLINE/RELEASE fields. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "udhcp_server_message_policy.h"

static void
test_decline_requires_zero_ciaddr(void)
{
    assert(udhcp_server_decline_ciaddr_valid(0U));
    assert(!udhcp_server_decline_ciaddr_valid(1U));
    assert(!udhcp_server_decline_ciaddr_valid(0xffffffffU));
}

static void
test_release_rejects_requested_ip_option(void)
{
    assert(udhcp_server_release_has_no_requested_ip(0));
    assert(!udhcp_server_release_has_no_requested_ip(1));
}

static void
test_inform_requires_nonzero_ciaddr(void)
{
    assert(!udhcp_server_inform_ciaddr_valid(0U));
    assert(udhcp_server_inform_ciaddr_valid(1U));
    assert(udhcp_server_inform_ciaddr_valid(0xffffffffU));
}

static void
test_request_requires_valid_state_field_combinations(void)
{
    /* SELECTING: server-id and requested IP, with ciaddr zero. */
    assert(udhcp_server_request_fields_valid(1, 1, 0U));
    /* INIT-REBOOT: requested IP only, with ciaddr zero. */
    assert(udhcp_server_request_fields_valid(0, 1, 0U));
    /* RENEWING/REBINDING: ciaddr only. */
    assert(udhcp_server_request_fields_valid(0, 0, 1U));
    assert(udhcp_server_request_fields_valid(0, 0, 0xffffffffU));

    /* Reject malformed combinations before the server can renew a lease. */
    assert(!udhcp_server_request_fields_valid(1, 0, 0U));
    assert(!udhcp_server_request_fields_valid(1, 1, 1U));
    assert(!udhcp_server_request_fields_valid(1, 0, 1U));
    assert(!udhcp_server_request_fields_valid(0, 1, 1U));
    assert(!udhcp_server_request_fields_valid(0, 0, 0U));
}

int
main(void)
{
    test_decline_requires_zero_ciaddr();
    test_release_rejects_requested_ip_option();
    test_inform_requires_nonzero_ciaddr();
    test_request_requires_valid_state_field_combinations();
    puts("DHCP server message field policy regression: PASS");
    return 0;
}
