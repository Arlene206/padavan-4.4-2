/* GNU89 regression tests for DHCP relay datagram length validation. */
#include <stdio.h>

#include "udhcp_relay_packet_bounds.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int
main(void)
{
    const size_t capacity = 548U;
    const size_t minimum = 240U;

    CHECK(!udhcp_relay_packet_length_valid(-1, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid(0, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)minimum - 1, capacity, minimum));
    CHECK(udhcp_relay_packet_length_valid((int)minimum, capacity, minimum));
    CHECK(udhcp_relay_packet_length_valid((int)capacity, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)capacity + 1, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)minimum, minimum - 1U, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)capacity, capacity, capacity + 1U));

    puts("DHCP relay datagram length bounds regression: PASS");
    return 0;
}
