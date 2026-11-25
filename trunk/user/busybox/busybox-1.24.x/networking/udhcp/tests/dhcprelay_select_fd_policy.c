/* GNU89 regression tests for the select()/fd_set descriptor boundary. */
#include <stdio.h>
#include "udhcp_relay_select_fd_policy.h"
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)
int
main(void)
{
    const int fd_set_size = 1024;
    CHECK(!udhcp_relay_fd_fits_select(-1, fd_set_size));
    CHECK(udhcp_relay_fd_fits_select(0, fd_set_size));
    CHECK(udhcp_relay_fd_fits_select(fd_set_size - 1, fd_set_size));
    CHECK(!udhcp_relay_fd_fits_select(fd_set_size, fd_set_size));
    CHECK(!udhcp_relay_fd_fits_select(fd_set_size + 1, fd_set_size));
    CHECK(!udhcp_relay_fd_fits_select(0, 0));
    CHECK(!udhcp_relay_fd_fits_select(0, -1));
    puts("DHCP relay select descriptor bounds regression: PASS");
    return 0;
}
