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
    CHECK(test_probe_result_keeps_local_errors_distinct() == 0);
    CHECK(test_dhcp_renew_requires_three_real_no_reply_probes() == 0);
    puts("WAN ARP reply validation and DHCP-renewal policy: PASS");
    return 0;
}
