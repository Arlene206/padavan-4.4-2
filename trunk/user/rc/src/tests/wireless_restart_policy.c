#include "wireless_restart_policy.h"

#include <stdio.h>

static int generator_calls;
static int generator_argument;
static int generator_status;

static int
fake_generate_config(int disable_autoscan)
{
    generator_calls++;
    generator_argument = disable_autoscan;
    return generator_status;
}

static int
test_no_reload_skips_generator(void)
{
    generator_calls = 0;
    if (wireless_restart_preflight(0, fake_generate_config) != 0)
        return 1;
    if (generator_calls != 0)
        return 1;
    if (wireless_restart_preflight(0, NULL) != 0)
        return 1;
    return 0;
}

static int
test_reload_generates_once_before_caller_continues(void)
{
    generator_calls = 0;
    generator_argument = -1;
    generator_status = 0;

    if (wireless_restart_preflight(1, fake_generate_config) != 0)
        return 1;
    if (generator_calls != 1 || generator_argument != 0)
        return 1;
    return 0;
}

static int
test_generation_failure_is_propagated(void)
{
    generator_calls = 0;
    generator_status = -5;

    if (wireless_restart_preflight(1, fake_generate_config) != -5)
        return 1;
    if (generator_calls != 1)
        return 1;
    return 0;
}

static int
test_missing_generator_fails_closed(void)
{
    if (wireless_restart_preflight(1, NULL) != -EINVAL)
        return 1;
    return 0;
}

static int
test_nonnegative_generator_status_is_preserved(void)
{
    generator_calls = 0;
    generator_status = 1;

    if (wireless_restart_preflight(1, fake_generate_config) != 1)
        return 1;
    if (generator_calls != 1)
        return 1;
    return 0;
}

int
main(void)
{
    if (test_no_reload_skips_generator() ||
        test_reload_generates_once_before_caller_continues() ||
        test_generation_failure_is_propagated() ||
        test_missing_generator_fails_closed() ||
        test_nonnegative_generator_status_is_preserved()) {
        fprintf(stderr, "Wireless restart preflight regression: FAIL\n");
        return 1;
    }

    printf("Wireless restart config preflight preserves active radios on generation failure: PASS\n");
    return 0;
}
