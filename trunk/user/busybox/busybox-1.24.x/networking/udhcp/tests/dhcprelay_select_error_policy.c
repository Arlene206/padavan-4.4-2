/* GNU89 regression tests for DHCP relay select() error handling. */
#include <errno.h>
#include <stdio.h>

#include "udhcp_relay_select_error_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int
main(void)
{
    CHECK(udhcp_relay_select_was_interrupted(EINTR));
    CHECK(!udhcp_relay_select_was_interrupted(EBADF));
    CHECK(!udhcp_relay_select_was_interrupted(EINVAL));
    CHECK(!udhcp_relay_select_was_interrupted(ENOMEM));
    CHECK(!udhcp_relay_select_was_interrupted(0));

    puts("DHCP relay select error policy regression: PASS");
    return 0;
}
