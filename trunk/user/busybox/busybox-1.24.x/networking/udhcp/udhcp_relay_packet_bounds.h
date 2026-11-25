/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_PACKET_BOUNDS_H
#define UDHCP_RELAY_PACKET_BOUNDS_H

#include <stddef.h>

/*
 * recvfrom(..., MSG_TRUNC, ...) reports the original datagram length on
 * Linux even when the receive buffer was too small. Reject both truncated
 * packets and datagrams too short to contain the BOOTP cookie plus a
 * minimally encoded DHCP option field before inspecting the packet.
 */
static inline int
udhcp_relay_packet_length_valid(int received_length, size_t packet_capacity,
                                size_t minimum_length)
{
    if (received_length < 0)
        return 0;
    if ((size_t)received_length > packet_capacity)
        return 0;
    if ((size_t)received_length < minimum_length)
        return 0;
    return 1;
}

#endif /* UDHCP_RELAY_PACKET_BOUNDS_H */
