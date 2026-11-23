#ifndef DETECT_WAN_ARP_RESULT_POLICY_H
#define DETECT_WAN_ARP_RESULT_POLICY_H
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#define DETECT_WAN_ARP_PROBE_ERROR (-1)
#define DETECT_WAN_ARP_PROBE_NO_REPLY 0
#define DETECT_WAN_ARP_PROBE_REPLY 1

static inline int
detect_wan_arp_hex_nibble(char value)
{
    if (value >= '0' && value <= '9')
        return value - '0';
    if (value >= 'a' && value <= 'f')
        return value - 'a' + 10;
    if (value >= 'A' && value <= 'F')
        return value - 'A' + 10;
    return -1;
}

/*
 * Parse exactly six colon-separated hexadecimal octets. The WAN override
 * formatter intentionally preserves legacy non-hex text compatibility, so
 * validate the normalized value here before constructing an ARP frame.
 * Keep output untouched on failure.
 */
static inline int
detect_wan_arp_parse_hardware_address(const char *text,
                                      uint8_t hardware_address[6])
{
    uint8_t parsed[6];
    size_t length, octet;

    if (!text || !hardware_address)
        return EINVAL;

    length = strlen(text);
    if (length != 17)
        return EINVAL;

    for (octet = 0; octet < 6; ++octet) {
        size_t offset = octet * 3;
        int high, low;

        high = detect_wan_arp_hex_nibble(text[offset]);
        low = detect_wan_arp_hex_nibble(text[offset + 1]);
        if (high < 0 || low < 0)
            return EINVAL;
        if (octet < 5 && text[offset + 2] != ':')
            return EINVAL;
        parsed[octet] = (uint8_t)((high << 4) | low);
    }

    memcpy(hardware_address, parsed, sizeof(parsed));
    return 0;
}

/*
 * Accept only a complete Ethernet/IPv4 ARP reply for this exact probe.
 * Packet sockets can receive unrelated and malformed frames; accepting one
 * after checking only a subset of its identity can hide a dead gateway and
 * suppress DHCP recovery. Packed IPv4 fields may be unaligned on MIPS.
 */
struct detect_wan_arp_reply {
    uint16_t ethernet_protocol;
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_length;
    uint8_t protocol_length;
    uint16_t operation;
    const void *ethernet_source;
    const void *ethernet_destination;
    const void *sender_hardware;
    const void *sender_protocol;
    const void *target_hardware;
    const void *target_protocol;
};

static inline int
detect_wan_arp_reply_matches(const struct detect_wan_arp_reply *reply,
                             const void *local_hardware,
                             uint32_t gateway_address,
                             uint32_t local_address)
{
    uint32_t sender_address, target_address;

    if (!reply || !local_hardware || !reply->ethernet_source ||
        !reply->ethernet_destination || !reply->sender_hardware ||
        !reply->sender_protocol || !reply->target_hardware ||
        !reply->target_protocol)
        return 0;

    if (reply->ethernet_protocol != htons(ETH_P_ARP) ||
        reply->hardware_type != htons(ARPHRD_ETHER) ||
        reply->protocol_type != htons(ETH_P_IP) ||
        reply->hardware_length != 6 || reply->protocol_length != 4 ||
        reply->operation != htons(ARPOP_REPLY))
        return 0;

    memcpy(&sender_address, reply->sender_protocol, sizeof(sender_address));
    memcpy(&target_address, reply->target_protocol, sizeof(target_address));

    return sender_address == gateway_address &&
           target_address == local_address &&
           memcmp(reply->ethernet_source, reply->sender_hardware, 6) == 0 &&
           memcmp(reply->ethernet_destination, local_hardware, 6) == 0 &&
           memcmp(reply->target_hardware, local_hardware, 6) == 0;
}

/* Local socket/I/O failure is not evidence that the gateway did not answer. */
static inline int
detect_wan_arp_probe_result(int reply_received, int operation_error)
{
    if (operation_error != 0) {
        errno = operation_error > 0 ? operation_error : EIO;
        return DETECT_WAN_ARP_PROBE_ERROR;
    }
    return reply_received ? DETECT_WAN_ARP_PROBE_REPLY :
                            DETECT_WAN_ARP_PROBE_NO_REPLY;
}
/* Renewal needs three real no-reply probes, no local errors, and link-up. */
static inline int
detect_wan_arp_should_renew(int retries_exhausted, int probe_count,
                            int required_probes, int ap_mode, int link_up,
                            int probe_error)
{
    return retries_exhausted && required_probes > 0 &&
           probe_count == required_probes && !probe_error &&
           (ap_mode || link_up);
}
#endif /* DETECT_WAN_ARP_RESULT_POLICY_H */
