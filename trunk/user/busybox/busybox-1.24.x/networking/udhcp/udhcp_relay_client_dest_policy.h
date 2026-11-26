/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_CLIENT_DEST_POLICY_H
#define UDHCP_RELAY_CLIENT_DEST_POLICY_H

#include <stdint.h>

#define UDHCP_RELAY_CLIENT_BROADCAST_ADDR 0xffffffffU

/*
 * A relay must honor the BOOTP broadcast flag and broadcast DHCPNAK.
 * Otherwise prefer the protocol client address (ciaddr/yiaddr) over the
 * source address captured from the initial UDP datagram. Initial DHCP
 * clients commonly send from 0.0.0.0, which is not a reply destination.
 */
static inline uint32_t
udhcp_relay_client_destination(uint32_t source_addr, uint32_t ciaddr,
                               uint32_t yiaddr, int broadcast, int nak)
{
    if (broadcast || nak)
        return UDHCP_RELAY_CLIENT_BROADCAST_ADDR;
    if (ciaddr != 0U)
        return ciaddr;
    if (yiaddr != 0U)
        return yiaddr;
    if (source_addr != 0U)
        return source_addr;
    return UDHCP_RELAY_CLIENT_BROADCAST_ADDR;
}

#endif /* UDHCP_RELAY_CLIENT_DEST_POLICY_H */
