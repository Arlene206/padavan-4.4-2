#include <stdio.h>
#include "lanauth_signal_level_policy.h"

static int
test_auth1_uses_ascii_level_snapshot(void)
{
    sig_atomic_t level = 1;

    if (lanauth_auth1_level_character(level) != '1' || level != 1)
        return 1;
    level = 2;
    if (lanauth_auth1_level_character(level) != '2' || level != 2)
        return 1;
    return 0;
}

static int
test_auth2_uses_binary_level_snapshot(void)
{
    sig_atomic_t level = 1;

    if (lanauth_auth2_level_byte(level) != 0 || level != 1)
        return 1;
    level = 2;
    if (lanauth_auth2_level_byte(level) != 1 || level != 2)
        return 1;
    return 0;
}

int
main(void)
{
    if (test_auth1_uses_ascii_level_snapshot() != 0 ||
        test_auth2_uses_binary_level_snapshot() != 0) {
        fprintf(stderr, "LAN authentication signal-level regression failed\n");
        return 1;
    }
    puts("LAN authentication signal-level snapshot policy: PASS");
    return 0;
}
