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

int
main(void)
{
    test_decline_requires_zero_ciaddr();
    test_release_rejects_requested_ip_option();
    puts("DHCP server DECLINE/RELEASE field policy regression: PASS");
    return 0;
}
