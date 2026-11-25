#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "detect_wan_arp_result_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_empty_and_unspecified_gateway_keep_legacy_no_gateway_state(void)
{
    uint32_t address = htonl(0xc0000201U);

    CHECK(detect_wan_arp_parse_gateway_address("", &address) == 0);
    CHECK(address == INADDR_ANY);
    address = htonl(0xc0000201U);
    CHECK(detect_wan_arp_parse_gateway_address("0.0.0.0", &address) == 0);
    CHECK(address == INADDR_ANY);
    return 0;
}

static int
test_valid_gateway_is_parsed_in_network_byte_order(void)
{
    uint32_t address = INADDR_ANY;

    CHECK(detect_wan_arp_parse_gateway_address("192.0.2.1", &address) == 0);
    CHECK(address == htonl(0xc0000201U));
    return 0;
}

static int
test_invalid_gateway_is_not_aliased_to_no_gateway(void)
{
    static const char * const invalid[] = {
        "not-an-ip", "192.0.2.256", "192.0.2.1junk",
        "192.000.002.001", "0300.0.2.1", "192.0.2",
        "0xc0000201", "3221225985", "192.0.2.1 "
    };
    uint32_t address, before;
    size_t index;

    for (index = 0; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        address = htonl(0xdeadbeefU);
        before = address;
        CHECK(detect_wan_arp_parse_gateway_address(invalid[index],
                                                   &address) == EINVAL);
        CHECK(address == before);
    }
    return 0;
}

static int
test_invalid_arguments_fail_without_publishing_output(void)
{
    uint32_t address = htonl(0xdeadbeefU);
    uint32_t before = address;

    CHECK(detect_wan_arp_parse_gateway_address(NULL, &address) == EINVAL);
    CHECK(address == before);
    CHECK(detect_wan_arp_parse_gateway_address("192.0.2.1", NULL) == EINVAL);
    return 0;
}

int
main(void)
{
    CHECK(test_empty_and_unspecified_gateway_keep_legacy_no_gateway_state() == 0);
    CHECK(test_valid_gateway_is_parsed_in_network_byte_order() == 0);
    CHECK(test_invalid_gateway_is_not_aliased_to_no_gateway() == 0);
    CHECK(test_invalid_arguments_fail_without_publishing_output() == 0);
    puts("WAN gateway address fail-closed parsing policy: PASS");
    return 0;
}
