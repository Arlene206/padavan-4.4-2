/* GNU89 regression tests for DHCP relay giaddr/hop handling. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "udhcp_relay_bootrequest_policy.h"

static void
test_giaddr_selection(void)
{
    assert(udhcp_relay_select_giaddr(0x01020304U, 0x0a000001U, 0x0a000002U)
           == 0x01020304U);
    assert(udhcp_relay_select_giaddr(0U, 0x0a000001U, 0x0a000002U)
           == 0x0a000001U);
    assert(udhcp_relay_select_giaddr(0U, 0U, 0x0a000002U)
           == 0x0a000002U);
    assert(udhcp_relay_select_giaddr(0U, 0U, 0U) == 0U);
}

static void
test_bootrequest_opcode(void)
{
    assert(udhcp_relay_is_bootrequest(1U));
    assert(!udhcp_relay_is_bootrequest(0U));
    assert(!udhcp_relay_is_bootrequest(2U));
    assert(!udhcp_relay_is_bootrequest(255U));
}

static void
test_hop_limit(void)
{
    uint8_t hops = 0U;

    assert(udhcp_relay_increment_hops(&hops));
    assert(hops == 1U);

    hops = (uint8_t)(UDHCP_RELAY_MAX_HOPS - 1U);
    assert(udhcp_relay_increment_hops(&hops));
    assert(hops == UDHCP_RELAY_MAX_HOPS);

    /* RFC says discard values exceeding 16; 16 itself is incremented. */
    hops = (uint8_t)UDHCP_RELAY_MAX_HOPS;
    assert(udhcp_relay_increment_hops(&hops));
    assert(hops == UDHCP_RELAY_MAX_HOPS + 1U);

    hops = (uint8_t)(UDHCP_RELAY_MAX_HOPS + 1U);
    assert(!udhcp_relay_increment_hops(&hops));
    assert(hops == UDHCP_RELAY_MAX_HOPS + 1U);

    hops = 255U;
    assert(!udhcp_relay_increment_hops(&hops));
    assert(hops == 255U);
    assert(!udhcp_relay_increment_hops(NULL));
}

int
main(void)
{
    test_giaddr_selection();
    test_bootrequest_opcode();
    test_hop_limit();
    puts("DHCP relay giaddr/hop policy regression: PASS");
    return 0;
}
