/* GNU89 regression tests for DHCP relay datagram length validation. */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "udhcp_relay_packet_bounds.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_datagram_truncation_reporting(void)
{
    int sockets[2] = { -1, -1 };
    unsigned char packet[64];
    unsigned char buffer[16];
    ssize_t received;

    CHECK(socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) == 0);
    memset(packet, 0xa5, sizeof(packet));
    CHECK(send(sockets[0], packet, sizeof(packet), 0) == (ssize_t)sizeof(packet));
    received = recvfrom(sockets[1], buffer, sizeof(buffer), MSG_TRUNC, NULL, NULL);
    CHECK(received == (ssize_t)sizeof(packet));
    CHECK(!udhcp_relay_packet_length_valid((int)received, sizeof(buffer), 1U));
    CHECK(close(sockets[0]) == 0);
    CHECK(close(sockets[1]) == 0);
    return 0;
}


static int
test_message_type_option_requires_exactly_one_byte(void)
{
    CHECK(!udhcp_relay_message_type_length_valid(0, 0U));
    CHECK(udhcp_relay_message_type_length_valid(1, 1U));
    CHECK(!udhcp_relay_message_type_length_valid(1, 0U));
    CHECK(!udhcp_relay_message_type_length_valid(1, 2U));
    CHECK(!udhcp_relay_message_type_length_valid(1, 255U));
    CHECK(!udhcp_relay_message_type_length_valid(-1, 0U));
    CHECK(!udhcp_relay_message_type_length_valid(-1, 1U));
    CHECK(!udhcp_relay_message_type_length_valid(2, 1U));
    return 0;
}

int
main(void)
{
    const size_t capacity = 548U;
    const size_t minimum = 244U;

    CHECK(!udhcp_relay_packet_length_valid(-1, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid(0, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)minimum - 1, capacity, minimum));
    CHECK(udhcp_relay_packet_length_valid((int)minimum, capacity, minimum));
    CHECK(udhcp_relay_packet_length_valid((int)capacity, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)capacity + 1, capacity, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)minimum, minimum - 1U, minimum));
    CHECK(!udhcp_relay_packet_length_valid((int)capacity, capacity, capacity + 1U));
    CHECK(test_datagram_truncation_reporting() == 0);
    CHECK(test_message_type_option_requires_exactly_one_byte() == 0);

    puts("DHCP relay datagram length and message-type bounds regression: PASS");
    return 0;
}
