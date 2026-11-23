#include <errno.h>
#include <stdio.h>
#include "detect_wan_arp_result_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

struct arp_reply_fixture {
    struct detect_wan_arp_reply reply;
    unsigned char ethernet_source[6];
    unsigned char ethernet_destination[6];
    unsigned char sender_hardware[6];
    unsigned char sender_protocol[sizeof(uint32_t) + 1];
    unsigned char target_hardware[6];
    unsigned char target_protocol[sizeof(uint32_t) + 1];
    unsigned char local_hardware[6];
    uint32_t gateway_address;
    uint32_t local_address;
};

static void
initialize_valid_arp_reply(struct arp_reply_fixture *fixture)
{
    static const unsigned char local_mac[6] =
        { 0x02, 0x11, 0x22, 0x33, 0x44, 0x55 };
    static const unsigned char gateway_mac[6] =
        { 0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0xee };

    memset(fixture, 0, sizeof(*fixture));
    fixture->reply.ethernet_protocol = htons(ETH_P_ARP);
    fixture->reply.hardware_type = htons(ARPHRD_ETHER);
    fixture->reply.protocol_type = htons(ETH_P_IP);
    fixture->reply.hardware_length = 6;
    fixture->reply.protocol_length = 4;
    fixture->reply.operation = htons(ARPOP_REPLY);

    memcpy(fixture->ethernet_source, gateway_mac, sizeof(gateway_mac));
    memcpy(fixture->sender_hardware, gateway_mac, sizeof(gateway_mac));
    memcpy(fixture->ethernet_destination, local_mac, sizeof(local_mac));
    memcpy(fixture->target_hardware, local_mac, sizeof(local_mac));
    memcpy(fixture->local_hardware, local_mac, sizeof(local_mac));

    fixture->gateway_address = htonl(0xc0000201U);
    fixture->local_address = htonl(0xc0000202U);
    memcpy(fixture->sender_protocol + 1, &fixture->gateway_address,
           sizeof(fixture->gateway_address));
    memcpy(fixture->target_protocol + 1, &fixture->local_address,
           sizeof(fixture->local_address));

    fixture->reply.ethernet_source = fixture->ethernet_source;
    fixture->reply.ethernet_destination = fixture->ethernet_destination;
    fixture->reply.sender_hardware = fixture->sender_hardware;
    fixture->reply.sender_protocol = fixture->sender_protocol + 1;
    fixture->reply.target_hardware = fixture->target_hardware;
    fixture->reply.target_protocol = fixture->target_protocol + 1;
}

static int
test_arp_reply_requires_matching_l2_l3_tuple(void)
{
    struct arp_reply_fixture fixture;

    initialize_valid_arp_reply(&fixture);
    CHECK(detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                       fixture.gateway_address, fixture.local_address));
    return 0;
}

static int
test_malformed_and_unrelated_arp_replies_are_rejected(void)
{
    struct arp_reply_fixture fixture;

    initialize_valid_arp_reply(&fixture);
    fixture.reply.ethernet_protocol = 0;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.hardware_type = 0;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.protocol_type = 0;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.hardware_length = 5;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.protocol_length = 16;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.operation = htons(ARPOP_REQUEST);
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.ethernet_source[0] ^= 0x02;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.ethernet_destination[0] ^= 0x02;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.target_hardware[0] ^= 0x02;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.sender_protocol[1] ^= 0x01;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.target_protocol[1] ^= 0x01;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));

    initialize_valid_arp_reply(&fixture);
    fixture.reply.sender_protocol = NULL;
    CHECK(!detect_wan_arp_reply_matches(&fixture.reply, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));
    CHECK(!detect_wan_arp_reply_matches(NULL, fixture.local_hardware,
                                        fixture.gateway_address, fixture.local_address));
    return 0;
}

static int
test_arp_reply_rejects_invalid_sender_hardware_address(void)
{
    struct arp_reply_fixture fixture;
    static const unsigned char invalid_sources[][6] = {
        { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x01 },
        { 0x33, 0x33, 0x00, 0x00, 0x00, 0x01 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    };
    size_t index;

    for (index = 0;
         index < sizeof(invalid_sources) / sizeof(invalid_sources[0]);
         ++index) {
        initialize_valid_arp_reply(&fixture);
        memcpy(fixture.ethernet_source, invalid_sources[index], 6);
        memcpy(fixture.sender_hardware, invalid_sources[index], 6);
        CHECK(!detect_wan_arp_reply_matches(
            &fixture.reply, fixture.local_hardware,
            fixture.gateway_address, fixture.local_address));
    }

    initialize_valid_arp_reply(&fixture);
    fixture.reply.sender_hardware = NULL;
    CHECK(!detect_wan_arp_reply_matches(
        &fixture.reply, fixture.local_hardware,
        fixture.gateway_address, fixture.local_address));
    return 0;
}

static int
test_arp_source_hardware_address_requires_colon_hex(void)
{
    static const uint8_t expected[6] =
        { 0x02, 0x11, 0x22, 0xaa, 0xbb, 0xff };
    uint8_t actual[6] = { 0 };
    int status;

    status = detect_wan_arp_parse_hardware_address(
        "02:11:22:aA:BB:ff", actual);
    CHECK(status == 0);
    CHECK(memcmp(actual, expected, sizeof(expected)) == 0);

    memset(actual, 0, sizeof(actual));
    status = detect_wan_arp_parse_hardware_address(
        "00:11:22:33:44:55", actual);
    CHECK(status == 0);
    CHECK(actual[0] == 0x00 && actual[5] == 0x55);
    return 0;
}

static int
test_invalid_arp_source_hardware_address_is_rejected_transactionally(void)
{
    static const char * const invalid_values[] = {
        "", "02:11:22:33:44", "02:11:22:33:44:55:66",
        "02-11-22-33-44-55", "02:11:22:33:44:5g",
        "02:11:22:33:44:5", "02:11:22:33:44:5Z",
        "0:11:22:33:44:55", "0211:22:33:44:55",
        " 02:11:22:33:44:55", "00:00:00:00:00:00",
        "ff:ff:ff:ff:ff:ff", "01:00:5e:00:00:01",
        "33:33:00:00:00:01"
    };
    uint8_t actual[6], before[6];
    size_t index, byte_index;

    for (index = 0;
         index < sizeof(invalid_values) / sizeof(invalid_values[0]);
         ++index) {
        memset(actual, 0xa5, sizeof(actual));
        memcpy(before, actual, sizeof(before));
        CHECK(detect_wan_arp_parse_hardware_address(
                  invalid_values[index], actual) == EINVAL);
        CHECK(memcmp(actual, before, sizeof(actual)) == 0);
    }

    memset(actual, 0x5a, sizeof(actual));
    memcpy(before, actual, sizeof(before));
    CHECK(detect_wan_arp_parse_hardware_address(NULL, actual) == EINVAL);
    CHECK(memcmp(actual, before, sizeof(actual)) == 0);
    CHECK(detect_wan_arp_parse_hardware_address(
              "02:11:22:33:44:55", NULL) == EINVAL);

    for (byte_index = 0; byte_index < sizeof(actual); ++byte_index)
        CHECK(actual[byte_index] == before[byte_index]);
    return 0;
}

static int
test_local_ipv4_lookup_errors_are_not_absent_addresses(void)
{
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, 0) == 0);
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, EADDRNOTAVAIL) == 0);
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, EACCES) == EACCES);
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, EINVAL) == EINVAL);
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, EIO) == EIO);
    CHECK(detect_wan_local_ip4_status(INADDR_ANY, -1) == EIO);
    CHECK(detect_wan_local_ip4_status(htonl(0xc0000202U), EIO) == 0);
    return 0;
}

static int
test_probe_result_keeps_local_errors_distinct(void)
{
    errno = 0;
    CHECK(detect_wan_arp_probe_result(1, 0) == DETECT_WAN_ARP_PROBE_REPLY);
    CHECK(detect_wan_arp_probe_result(0, 0) == DETECT_WAN_ARP_PROBE_NO_REPLY);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(0, EACCES) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EACCES);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(1, EIO) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EIO);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(0, -1) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EIO);
    return 0;
}

static int
test_dhcp_renew_requires_three_real_no_reply_probes(void)
{
    CHECK(detect_wan_arp_should_renew(1, 3, 3, 0, 1, 0));
    CHECK(detect_wan_arp_should_renew(1, 3, 3, 1, 0, 0));
    CHECK(!detect_wan_arp_should_renew(0, 3, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 2, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 4, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 3, 3, 0, 0, 0));
    CHECK(!detect_wan_arp_should_renew(1, 3, 3, 0, 1, EIO));
    return 0;
}

int
main(void)
{
    CHECK(test_arp_reply_requires_matching_l2_l3_tuple() == 0);
    CHECK(test_malformed_and_unrelated_arp_replies_are_rejected() == 0);
    CHECK(test_arp_reply_rejects_invalid_sender_hardware_address() == 0);
    CHECK(test_arp_source_hardware_address_requires_colon_hex() == 0);
    CHECK(test_invalid_arp_source_hardware_address_is_rejected_transactionally() == 0);
    CHECK(test_local_ipv4_lookup_errors_are_not_absent_addresses() == 0);
    CHECK(test_probe_result_keeps_local_errors_distinct() == 0);
    CHECK(test_dhcp_renew_requires_three_real_no_reply_probes() == 0);
    puts("WAN ARP reply validation and DHCP-renewal policy: PASS");
    return 0;
}
