#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "vpn_client_nvram_policy.h"

struct fake_nvram {
    unsigned int calls;
    int fail_at;
    int failure_status;
    int statuses[3];
    char names[3][32];
    char values[3][8];
};

static int
fake_set(void *opaque, const char *name, const char *value)
{
    struct fake_nvram *fake = (struct fake_nvram *)opaque;
    unsigned int call = fake->calls++;

    assert(call < 3);
    snprintf(fake->names[call], sizeof(fake->names[call]), "%s", name);
    snprintf(fake->values[call], sizeof(fake->values[call]), "%s", value);
    if (fake->statuses[call] != 0)
        return fake->statuses[call];
    return ((int)call == fake->fail_at) ? fake->failure_status : 0;
}

static void
reset_fake(struct fake_nvram *fake)
{
    memset(fake, 0, sizeof(*fake));
    fake->fail_at = -1;
    fake->failure_status = EACCES;
}

static void
test_boolean_read_status_preserves_legacy_defaults_and_rejects_invalid_values(void)
{
    int result;

    result = 77;
    assert(vpn_client_nvram_read_bool_status(0, 0, NULL, 0, &result) == 0);
    assert(result == 0);
    result = 77;
    assert(vpn_client_nvram_read_bool_status(0, 0, NULL, 1, &result) == 0);
    assert(result == 1);

    result = 77;
    assert(vpn_client_nvram_read_bool_status(1, 0, "0", 0, &result) == 0);
    assert(result == 0);
    result = 77;
    assert(vpn_client_nvram_read_bool_status(1, 0, " 01 \t", 0, &result) == 0);
    assert(result == 1);
    result = 77;
    assert(vpn_client_nvram_read_bool_status(1, 0, "", 0, &result) == 0);
    assert(result == 0);
    result = 77;
    assert(vpn_client_nvram_read_bool_status(1, 0, "   ", 0, &result) == 0);
    assert(result == 0);

    result = 77;
    assert(vpn_client_nvram_read_bool_status(-1, EACCES, "1", 0, &result) == EACCES);
    assert(result == 77);
    assert(vpn_client_nvram_read_bool_status(-1, 0, "1", 0, &result) == EIO);
    assert(vpn_client_nvram_read_bool_status(1, 0, NULL, 0, &result) == EIO);
    assert(vpn_client_nvram_read_bool_status(2, 0, "1", 0, &result) == EIO);
    assert(vpn_client_nvram_read_bool_status(1, 0, "-1", 0, &result) == EINVAL);
    assert(vpn_client_nvram_read_bool_status(1, 0, "2", 0, &result) == EINVAL);
    assert(vpn_client_nvram_read_bool_status(1, 0, "1x", 0, &result) == EINVAL);
    assert(vpn_client_nvram_read_bool_status(1, 0, "+1", 0, &result) == EINVAL);
    assert(vpn_client_nvram_read_bool_status(1, 0, "1", 2, &result) == EINVAL);
    assert(vpn_client_nvram_read_bool_status(1, 0, "1", 0, NULL) == EINVAL);
    assert(result == 77);
}

static void
test_cache_backed_text_is_copied_without_truncation(void)
{
    char destination[8];

    assert(vpn_client_nvram_copy_text("peer.example", destination,
                                      sizeof(destination)) == ENAMETOOLONG);
    assert(destination[0] == '\0');
    assert(vpn_client_nvram_copy_text("peer", destination,
                                      sizeof(destination)) == 0);
    assert(strcmp(destination, "peer") == 0);
    assert(vpn_client_nvram_copy_text(NULL, destination,
                                      sizeof(destination)) == 0);
    assert(destination[0] == '\0');
    assert(vpn_client_nvram_copy_text("peer", NULL, sizeof(destination)) ==
           EINVAL);
    assert(vpn_client_nvram_copy_text("peer", destination, 0) == EINVAL);
}

static void
test_independent_runtime_writes_attempt_all_and_keep_first_error(void)
{
    struct fake_nvram fake;
    const struct vpn_client_nvram_value values[] = {
        { "vpnc_state_t", "1" },
        { "vpnc_dom_t", "example.test" },
        { "vpnc_dns_t", "1.1.1.1" }
    };

    reset_fake(&fake);
    fake.statuses[0] = EACCES;
    fake.statuses[1] = ENOSPC;
    assert(vpn_client_nvram_set_all_values(values, 3, fake_set, &fake) ==
           EACCES);
    assert(fake.calls == 3);
    assert(strcmp(fake.names[0], "vpnc_state_t") == 0);
    assert(strcmp(fake.names[1], "vpnc_dom_t") == 0);
    assert(strcmp(fake.names[2], "vpnc_dns_t") == 0);
}

static void
test_write_status_preserves_errno(void)
{
    assert(vpn_client_nvram_write_status(0, 0) == 0);
    assert(vpn_client_nvram_write_status(-1, ENOSPC) == ENOSPC);
    assert(vpn_client_nvram_write_status(-1, 0) == EIO);
    assert(vpn_client_nvram_write_status(1, EBUSY) == EBUSY);
}

static void
test_read_status_distinguishes_absence_from_failure(void)
{
    assert(vpn_client_nvram_read_status(0, 0, 0) == 0);
    assert(vpn_client_nvram_read_status(1, 0, 1) == 0);
    assert(vpn_client_nvram_read_status(-1, EACCES, 0) == EACCES);
    assert(vpn_client_nvram_read_status(-1, 0, 0) == EIO);
    assert(vpn_client_nvram_read_status(1, 0, 0) == EIO);
    assert(vpn_client_nvram_read_status(0, 0, 1) == EIO);
    assert(vpn_client_nvram_read_status(2, 0, 1) == EIO);
}

static void
test_runtime_clear_publishes_dns_domain_and_state_in_order(void)
{
    struct fake_nvram fake;

    reset_fake(&fake);
    assert(vpn_client_nvram_clear_runtime_state(fake_set, &fake) == 0);
    assert(fake.calls == 3);
    assert(strcmp(fake.names[0], "vpnc_dns_t") == 0);
    assert(strcmp(fake.values[0], "") == 0);
    assert(strcmp(fake.names[1], "vpnc_dom_t") == 0);
    assert(strcmp(fake.values[1], "") == 0);
    assert(strcmp(fake.names[2], "vpnc_state_t") == 0);
    assert(strcmp(fake.values[2], "0") == 0);
}

static void
test_domain_snapshot_clear_publishes_empty_value_and_reports_failure(void)
{
    struct fake_nvram fake;

    reset_fake(&fake);
    assert(vpn_client_nvram_clear_domain_snapshot(fake_set, &fake) == 0);
    assert(fake.calls == 1);
    assert(strcmp(fake.names[0], "vpnc_dom_t") == 0);
    assert(strcmp(fake.values[0], "") == 0);

    reset_fake(&fake);
    fake.fail_at = 0;
    fake.failure_status = ENOSPC;
    assert(vpn_client_nvram_clear_domain_snapshot(fake_set, &fake) ==
           ENOSPC);
    assert(fake.calls == 1);
}

static void
test_runtime_clear_stops_at_first_failed_write(void)
{
    struct fake_nvram fake;
    int fail_at;

    for (fail_at = 0; fail_at < 3; ++fail_at) {
        reset_fake(&fake);
        fake.fail_at = fail_at;
        assert(vpn_client_nvram_clear_runtime_state(fake_set, &fake) ==
               EACCES);
        assert(fake.calls == (unsigned int)(fail_at + 1));
    }
}

static void
test_invalid_sequence_does_not_write(void)
{
    struct fake_nvram fake;

    reset_fake(&fake);
    assert(vpn_client_nvram_clear_runtime_state(NULL, &fake) == EINVAL);
    assert(fake.calls == 0);
    assert(vpn_client_nvram_set_required_values(NULL, 1, fake_set,
                                                &fake) == EINVAL);
    assert(fake.calls == 0);
}

int
main(void)
{
    test_cache_backed_text_is_copied_without_truncation();
    test_independent_runtime_writes_attempt_all_and_keep_first_error();
    test_write_status_preserves_errno();
    test_read_status_distinguishes_absence_from_failure();
    test_boolean_read_status_preserves_legacy_defaults_and_rejects_invalid_values();
    test_runtime_clear_publishes_dns_domain_and_state_in_order();
    test_domain_snapshot_clear_publishes_empty_value_and_reports_failure();
    test_runtime_clear_stops_at_first_failed_write();
    test_invalid_sequence_does_not_write();
    puts("VPN client runtime NVRAM fail-closed policy regression: PASS");
    return 0;
}
