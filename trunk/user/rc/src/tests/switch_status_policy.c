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

int
main(void)
{
    CHECK(test_success_publishes_both_counters() == 0);
    CHECK(test_positive_errno_is_negative_and_does_not_publish() == 0);
    CHECK(test_negative_error_is_preserved() == 0);
    CHECK(test_null_output_is_rejected() == 0);
    puts("Switch port-byte error propagation policy: PASS");
    return 0;
}
