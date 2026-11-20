#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "vpn_client_nvram_policy.h"

struct fake_nvram {
    unsigned int calls;
    int fail_at;
    int failure_status;
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
    test_write_status_preserves_errno();
    test_read_status_distinguishes_absence_from_failure();
    test_runtime_clear_publishes_dns_domain_and_state_in_order();
    test_runtime_clear_stops_at_first_failed_write();
    test_invalid_sequence_does_not_write();
    puts("VPN client runtime NVRAM fail-closed policy regression: PASS");
    return 0;
}
