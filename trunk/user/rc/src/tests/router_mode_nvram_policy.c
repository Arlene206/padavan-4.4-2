#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "router_mode_nvram_policy.h"

struct fake_nvram {
    int calls;
    int fail_at;
    int failure_status;
    char names[3][32];
    char values[3][32];
};

static int
fake_set(void *opaque, const char *name, const char *value)
{
    struct fake_nvram *fake = (struct fake_nvram *)opaque;
    int call = fake->calls++;

    assert(call < 3);
    snprintf(fake->names[call], sizeof(fake->names[call]), "%s", name);
    snprintf(fake->values[call], sizeof(fake->values[call]), "%s", value);
    if (call == fake->fail_at)
        return fake->failure_status;
    return 0;
}

static void
reset_fake(struct fake_nvram *fake)
{
    memset(fake, 0, sizeof(*fake));
    fake->fail_at = -1;
    fake->failure_status = ENOSPC;
}

static void
test_gateway_mode_publishes_nat_and_route(void)
{
    struct fake_nvram fake;
    int effective_mode = -1;

    reset_fake(&fake);
    assert(rc_router_mode_nvram_configure(1, fake_set, &fake,
                                          &effective_mode) == 0);
    assert(effective_mode == 1);
    assert(fake.calls == 2);
    assert(strcmp(fake.names[0], "wan_nat_x") == 0);
    assert(strcmp(fake.values[0], "1") == 0);
    assert(strcmp(fake.names[1], "wan_route_x") == 0);
    assert(strcmp(fake.values[1], "IP_Routed") == 0);
}

static void
test_router_mode_disables_nat_and_keeps_routed_path(void)
{
    struct fake_nvram fake;
    int effective_mode = -1;

    reset_fake(&fake);
    assert(rc_router_mode_nvram_configure(4, fake_set, &fake,
                                          &effective_mode) == 0);
    assert(effective_mode == 4);
    assert(fake.calls == 2);
    assert(strcmp(fake.values[0], "0") == 0);
    assert(strcmp(fake.names[1], "wan_route_x") == 0);
    assert(strcmp(fake.values[1], "IP_Routed") == 0);
}

static void
test_access_point_mode_only_publishes_bridged_route(void)
{
    struct fake_nvram fake;
    int effective_mode = -1;

    reset_fake(&fake);
    assert(rc_router_mode_nvram_configure(3, fake_set, &fake,
                                          &effective_mode) == 0);
    assert(effective_mode == 3);
    assert(fake.calls == 1);
    assert(strcmp(fake.names[0], "wan_route_x") == 0);
    assert(strcmp(fake.values[0], "IP_Bridged") == 0);
}

static void
test_unknown_mode_is_published_as_gateway_before_dependent_values(void)
{
    struct fake_nvram fake;
    int effective_mode = -1;

    reset_fake(&fake);
    assert(rc_router_mode_nvram_configure(99, fake_set, &fake,
                                          &effective_mode) == 0);
    assert(effective_mode == 1);
    assert(fake.calls == 3);
    assert(strcmp(fake.names[0], "sw_mode") == 0);
    assert(strcmp(fake.values[0], "1") == 0);
    assert(strcmp(fake.names[1], "wan_nat_x") == 0);
    assert(strcmp(fake.values[1], "1") == 0);
    assert(strcmp(fake.names[2], "wan_route_x") == 0);
    assert(strcmp(fake.values[2], "IP_Routed") == 0);
}

static void
test_each_required_write_failure_aborts_the_sequence(void)
{
    struct fake_nvram fake;
    int effective_mode;
    int fail_at;

    for (fail_at = 0; fail_at < 2; ++fail_at) {
        reset_fake(&fake);
        fake.fail_at = fail_at;
        assert(rc_router_mode_nvram_configure(1, fake_set, &fake,
                                              &effective_mode) == ENOSPC);
        assert(fake.calls == fail_at + 1);
    }

    for (fail_at = 0; fail_at < 3; ++fail_at) {
        reset_fake(&fake);
        fake.fail_at = fail_at;
        assert(rc_router_mode_nvram_configure(99, fake_set, &fake,
                                              &effective_mode) == ENOSPC);
        assert(fake.calls == fail_at + 1);
    }
}

static void
test_router_mode_parser_rejects_noncanonical_values(void)
{
    assert(rc_router_mode_parse("1") == 1);
    assert(rc_router_mode_parse("3") == 3);
    assert(rc_router_mode_parse("4") == 4);
    assert(rc_router_mode_parse(NULL) == 0);
    assert(rc_router_mode_parse("") == 0);
    assert(rc_router_mode_parse("03") == 0);
    assert(rc_router_mode_parse("3junk") == 0);
    assert(rc_router_mode_parse(" 3") == 0);
    assert(rc_router_mode_parse("+3") == 0);
}

static void
test_checked_read_status_distinguishes_absence_from_backend_failure(void)
{
    assert(rc_router_mode_nvram_read_status(0, 0, 0) == 0);
    assert(rc_router_mode_nvram_read_status(1, 0, 1) == 0);
    assert(rc_router_mode_nvram_read_status(-1, EACCES, 0) == EACCES);
    assert(rc_router_mode_nvram_read_status(-1, 0, 0) == EIO);
    assert(rc_router_mode_nvram_read_status(1, 0, 0) == EIO);
    assert(rc_router_mode_nvram_read_status(0, 0, 1) == EIO);
    assert(rc_router_mode_nvram_read_status(2, 0, 1) == EIO);
}

static void
test_invalid_arguments_do_not_write_nvram(void)
{
    struct fake_nvram fake;
    int effective_mode;

    reset_fake(&fake);
    assert(rc_router_mode_nvram_configure(1, NULL, &fake,
                                          &effective_mode) == EINVAL);
    assert(fake.calls == 0);
    assert(rc_router_mode_nvram_configure(1, fake_set, &fake, NULL) == EINVAL);
    assert(fake.calls == 0);
}


static void
test_router_mode_update_detection_preserves_ap_mode(void)
{
    int update_needed = 77;

    assert(rc_router_mode_update_needed(1, 0, "3", &update_needed) == 0);
    assert(update_needed == 0);
    assert(rc_router_mode_update_needed(0, 0, NULL, &update_needed) == 0);
    assert(update_needed == 1);
    assert(rc_router_mode_update_needed(1, 0, "1", &update_needed) == 0);
    assert(update_needed == 1);

    update_needed = 77;
    assert(rc_router_mode_update_needed(-1, EACCES, NULL,
                                        &update_needed) == EACCES);
    assert(update_needed == 77);
    assert(rc_router_mode_update_needed(-1, 0, NULL,
                                        &update_needed) == EIO);
    assert(update_needed == 77);
    assert(rc_router_mode_update_needed(0, 0, "3",
                                        &update_needed) == EIO);
    assert(update_needed == 77);
    assert(rc_router_mode_update_needed(2, 0, "1",
                                        &update_needed) == EIO);
    assert(update_needed == 77);
    assert(rc_router_mode_update_needed(1, 0, "3", NULL) == EINVAL);
    assert(rc_router_mode_update_needed(1, 0, "3invalid",
                                        &update_needed) == EINVAL);
    assert(update_needed == 77);
}

static void
test_router_mode_update_target_preserves_nat_mapping(void)
{
    int target_mode = 77;

    assert(rc_router_mode_update_target(1, 0, "0", &target_mode) == 0);
    assert(target_mode == 4);
    assert(rc_router_mode_update_target(1, 0, "1", &target_mode) == 0);
    assert(target_mode == 1);
    assert(rc_router_mode_update_target(1, 0, "2", &target_mode) == 0);
    assert(target_mode == 1);
    assert(rc_router_mode_update_target(0, 0, NULL, &target_mode) == 0);
    assert(target_mode == 4);

    target_mode = 77;
    assert(rc_router_mode_update_target(-1, EBUSY, NULL,
                                        &target_mode) == EBUSY);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(-1, 0, NULL,
                                        &target_mode) == EIO);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(0, 0, "0",
                                        &target_mode) == EIO);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(1, 0, NULL,
                                        &target_mode) == EIO);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(2, 0, "1",
                                        &target_mode) == EIO);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(1, 0, "1junk",
                                        &target_mode) == EINVAL);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(1, 0, "99999999999999999999",
                                        &target_mode) == ERANGE);
    assert(target_mode == 77);
    assert(rc_router_mode_update_target(1, 0, "1", NULL) == EINVAL);
}

static void
test_router_mode_update_write_status(void)
{
    assert(rc_router_mode_nvram_write_status(0, EACCES) == 0);
    assert(rc_router_mode_nvram_write_status(-1, EACCES) == EACCES);
    assert(rc_router_mode_nvram_write_status(-1, 0) == EIO);
    assert(rc_router_mode_nvram_write_status(1, 0) == EIO);
}

int
main(void)
{
    test_gateway_mode_publishes_nat_and_route();
    test_router_mode_disables_nat_and_keeps_routed_path();
    test_access_point_mode_only_publishes_bridged_route();
    test_unknown_mode_is_published_as_gateway_before_dependent_values();
    test_each_required_write_failure_aborts_the_sequence();
    test_router_mode_parser_rejects_noncanonical_values();
    test_checked_read_status_distinguishes_absence_from_backend_failure();
    test_router_mode_update_detection_preserves_ap_mode();
    test_router_mode_update_target_preserves_nat_mapping();
    test_router_mode_update_write_status();
    test_invalid_arguments_do_not_write_nvram();
    puts("Router-mode NVRAM publication regression: PASS");
    return 0;
}
