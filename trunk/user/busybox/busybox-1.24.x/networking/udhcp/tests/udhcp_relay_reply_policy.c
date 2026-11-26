/* GNU89 regression tests for DHCP relay BOOTREPLY opcode validation. */
#include <stdio.h>

#include "udhcp_relay_reply_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int
main(void)
{
    CHECK(udhcp_relay_is_bootreply(2U));
    CHECK(!udhcp_relay_is_bootreply(0U));
    CHECK(!udhcp_relay_is_bootreply(1U));
    CHECK(!udhcp_relay_is_bootreply(3U));
    CHECK(!udhcp_relay_is_bootreply(255U));

    puts("DHCP relay BOOTREPLY opcode regression: PASS");
    return 0;
}
