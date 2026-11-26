/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_BOOTREQUEST_POLICY_H
#define UDHCP_RELAY_BOOTREQUEST_POLICY_H

#include <stdint.h>

/* RFC 1542 section 4.1.1: discard incoming hops > 16. */
#define UDHCP_RELAY_MAX_HOPS 16U

static inline uint32_t
udhcp_relay_select_giaddr(uint32_t existing_giaddr, uint32_t interface_addr,
                          uint32_t fallback_addr)
{
    if (existing_giaddr != 0U)
        return existing_giaddr;
    if (interface_addr != 0U)
        return interface_addr;
    return fallback_addr;
}

static inline int
udhcp_relay_increment_hops(uint8_t *hops)
{
    if (hops == 0 || *hops > UDHCP_RELAY_MAX_HOPS)
        return 0;
    ++(*hops);
    return 1;
}

#endif /* UDHCP_RELAY_BOOTREQUEST_POLICY_H */
