#ifndef DETECT_WAN_ARP_RESULT_POLICY_H
#define DETECT_WAN_ARP_RESULT_POLICY_H
#include <errno.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#define DETECT_WAN_ARP_PROBE_ERROR (-1)
#define DETECT_WAN_ARP_PROBE_NO_REPLY 0
#define DETECT_WAN_ARP_PROBE_REPLY 1

/*
 * Parse the configured gateway without collapsing malformed non-empty input
 * into INADDR_ANY. Empty and explicit 0.0.0.0 retain the legacy no-gateway
 * behavior; keep the caller's output unchanged when parsing fails.
 */
static inline int
detect_wan_arp_parse_gateway_address(const char *text,
                                     uint32_t *gateway_address)
{
    struct in_addr parsed;

    if (!text || !gateway_address)
        return EINVAL;
    if (!*text) {
        *gateway_address = INADDR_ANY;
        return 0;
    }
    if (!inet_aton(text, &parsed))
        return EINVAL;

    *gateway_address = parsed.s_addr;
    return 0;
}

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
    int all_zero = 1, all_broadcast = 1;

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
        if (parsed[octet] != 0)
            all_zero = 0;
        if (parsed[octet] != 0xff)
            all_broadcast = 0;
    }

    /* An Ethernet source must be a unicast, non-reserved station address. */
    if ((parsed[0] & 0x01) != 0 || all_zero || all_broadcast)
        return EINVAL;

    memcpy(hardware_address, parsed, sizeof(parsed));
    return 0;
}

/*
 * Reject non-unicast sender identities in replies. Keep locally administered
 * unicast addresses valid; only the multicast bit and reserved all-zero /
 * broadcast values are disallowed.
 */
static inline int
detect_wan_arp_reply_source_hardware_valid(const void *source_hardware)
{
    const uint8_t *address = (const uint8_t *)source_hardware;
    int all_zero = 1, all_broadcast = 1;
    size_t index;

    if (!address || (address[0] & 0x01) != 0)
        return 0;

    for (index = 0; index < 6; ++index) {
        if (address[index] != 0)
            all_zero = 0;
        if (address[index] != 0xff)
            all_broadcast = 0;
    }
    return !all_zero && !all_broadcast;
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

    if (!detect_wan_arp_reply_source_hardware_valid(reply->sender_hardware))
        return 0;

    memcpy(&sender_address, reply->sender_protocol, sizeof(sender_address));
    memcpy(&target_address, reply->target_protocol, sizeof(target_address));

    return sender_address == gateway_address &&
           target_address == local_address &&
           memcmp(reply->ethernet_source, reply->sender_hardware, 6) == 0 &&
           memcmp(reply->ethernet_destination, local_hardware, 6) == 0 &&
           memcmp(reply->target_hardware, local_hardware, 6) == 0;
}

/*
 * get_interface_addr4() and get_wan_ip4() use INADDR_ANY for both a missing
 * address and lookup failures. Callers clear/capture errno around the lookup:
 * a clean zero or EADDRNOTAVAIL means no IPv4 address; other failures must
 * remain errors so a failed local lookup cannot masquerade as a healthy peer.
 */
static inline int
detect_wan_local_ip4_status(in_addr_t address, int lookup_errno)
{
    if (address != INADDR_ANY)
        return 0;
    if (lookup_errno == 0 || lookup_errno == EADDRNOTAVAIL)
        return 0;
    return lookup_errno > 0 ? lookup_errno : EIO;
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
