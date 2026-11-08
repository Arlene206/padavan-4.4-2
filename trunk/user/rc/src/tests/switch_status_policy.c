#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "../switch_status_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_success_publishes_both_counters(void)
{
    uint64_t rx = 11;
    uint64_t tx = 22;

    CHECK(switch_status_publish_port_bytes(0, 0x123456789ULL,
                                           0xABCDEF012ULL, &rx, &tx) == 0);
    CHECK(rx == 0x123456789ULL);
    CHECK(tx == 0xABCDEF012ULL);
    return 0;
}

static int
test_positive_errno_is_negative_and_does_not_publish(void)
{
    uint64_t rx = 111;
    uint64_t tx = 222;
    int errors[] = { EIO, ENODEV, EBADF };
    unsigned int i;

    for (i = 0; i < sizeof(errors) / sizeof(errors[0]); ++i) {
        CHECK(switch_status_publish_port_bytes(errors[i], 1, 2,
                                               &rx, &tx) == -errors[i]);
        CHECK(rx == 111);
        CHECK(tx == 222);
    }
    return 0;
}

static int
test_negative_error_is_preserved(void)
{
    uint64_t rx = 333;
    uint64_t tx = 444;

    CHECK(switch_status_publish_port_bytes(-EIO, 5, 6, &rx, &tx) == -EIO);
    CHECK(rx == 333);
    CHECK(tx == 444);
    return 0;
}

static int
test_null_output_is_rejected(void)
{
    uint64_t tx = 555;

    CHECK(switch_status_publish_port_bytes(0, 7, 8, NULL, &tx) == -EINVAL);
    CHECK(switch_status_publish_port_bytes(0, 7, 8, &tx, NULL) == -EINVAL);
    CHECK(tx == 555);
    return 0;
}

static int
test_vlan_pvid_returns_value_only_after_success(void)
{
    CHECK(switch_status_read_vlan_pvid(0, 2) == 2);
    CHECK(switch_status_read_vlan_pvid(0, 4094) == 4094);
    return 0;
}

static int
test_vlan_pvid_normalizes_ioctl_errors(void)
{
    CHECK(switch_status_read_vlan_pvid(EIO, 2) == -EIO);
    CHECK(switch_status_read_vlan_pvid(ENODEV, 4094) == -ENODEV);
    CHECK(switch_status_read_vlan_pvid(-EIO, 2) == -EIO);
    return 0;
}


static int
test_vlan_fallback_rejects_failed_or_invalid_pvid(void)
{
    int vlan_vid = 77;
    CHECK(switch_status_resolve_vlan_vid(100, 1, -EIO, 0, &vlan_vid) == 0);
    CHECK(vlan_vid == 100);
    CHECK(switch_status_resolve_vlan_vid(0, 0, 2, 1, &vlan_vid) == 0);
    CHECK(vlan_vid == 2);
    vlan_vid = 77;
    CHECK(switch_status_resolve_vlan_vid(0, 0, -EIO, 0, &vlan_vid) == -EIO);
    CHECK(vlan_vid == 77);
    CHECK(switch_status_resolve_vlan_vid(0, 0, 4095, 0, &vlan_vid) == -EINVAL);
    CHECK(vlan_vid == 77);
    CHECK(switch_status_resolve_vlan_vid(0, 0, 2, 1, NULL) == -EINVAL);
    return 0;
}

static int
test_failed_pvid_samples_do_not_signal_vlan_change(void)
{
    CHECK(switch_status_vlan_pvid_changed(2, 3) == 1);
    CHECK(switch_status_vlan_pvid_changed(2, 2) == 0);
    CHECK(switch_status_vlan_pvid_changed(-EIO, 3) == 0);
    CHECK(switch_status_vlan_pvid_changed(2, -EIO) == 0);
    CHECK(switch_status_vlan_pvid_changed(-ENODEV, -EIO) == 0);
    return 0;
}

static int
test_link_refresh_retries_failed_switch_reads(void)
{
    CHECK(switch_status_link_refresh_required(0, 0, 0, 0) == 1);
    CHECK(switch_status_link_refresh_required(1, 0, 0, 0) == 0);
    CHECK(switch_status_link_refresh_required(1, 1, 0, 0) == 1);
    CHECK(switch_status_link_refresh_required(1, 0, EIO, 0) == 1);
    CHECK(switch_status_link_refresh_required(1, 0, 0, 1) == 1);

    CHECK(switch_status_link_refresh_failed(0, 1, 0) == 0);
    CHECK(switch_status_link_refresh_failed(0, 0, 0) == 0);
    CHECK(switch_status_link_refresh_failed(EIO, 1, 0) == 1);
    CHECK(switch_status_link_refresh_failed(0, -1, 0) == 1);
    CHECK(switch_status_link_refresh_failed(0, 1, EIO) == 1);
    return 0;
}

int
main(void)
{
    CHECK(test_success_publishes_both_counters() == 0);
    CHECK(test_positive_errno_is_negative_and_does_not_publish() == 0);
    CHECK(test_negative_error_is_preserved() == 0);
    CHECK(test_null_output_is_rejected() == 0);
    CHECK(test_vlan_pvid_returns_value_only_after_success() == 0);
    CHECK(test_vlan_pvid_normalizes_ioctl_errors() == 0);
    CHECK(test_vlan_fallback_rejects_failed_or_invalid_pvid() == 0);
    CHECK(test_failed_pvid_samples_do_not_signal_vlan_change() == 0);
    CHECK(test_link_refresh_retries_failed_switch_reads() == 0);
    puts("Switch status error propagation and link-refresh retry policy: PASS");
    return 0;
}
