/* GNU89 regression tests for DHCP relay client response destinations. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "udhcp_relay_client_dest_policy.h"

int
main(void)
{
    const uint32_t source = 0xc000020aU;
    const uint32_t ciaddr = 0xc000020bU;
    const uint32_t yiaddr = 0xc000020cU;
    const uint32_t bcast = UDHCP_RELAY_CLIENT_BROADCAST_ADDR;

    assert(udhcp_relay_client_destination(source, ciaddr, yiaddr, 1, 0) == bcast);
    assert(udhcp_relay_client_destination(source, ciaddr, yiaddr, 0, 1) == bcast);
    assert(udhcp_relay_client_destination(source, ciaddr, yiaddr, 0, 0) == ciaddr);
    assert(udhcp_relay_client_destination(source, 0U, yiaddr, 0, 0) == yiaddr);
    assert(udhcp_relay_client_destination(source, 0U, 0U, 0, 0) == source);
    assert(udhcp_relay_client_destination(0U, 0U, 0U, 0, 0) == bcast);

    puts("DHCP relay client destination policy regression: PASS");
    return 0;
}
