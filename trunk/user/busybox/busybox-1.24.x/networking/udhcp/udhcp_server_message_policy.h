/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_SERVER_MESSAGE_POLICY_H
#define UDHCP_SERVER_MESSAGE_POLICY_H

#include <stdint.h>

/* RFC 2131: DHCPDECLINE requires ciaddr to be zero. */
static inline int
udhcp_server_decline_ciaddr_valid(uint32_t ciaddr)
{
    return ciaddr == 0U;
}

/* RFC 2131: DHCPRELEASE must not carry the requested-IP option. */
static inline int
udhcp_server_release_has_no_requested_ip(int requested_ip_present)
{
    return requested_ip_present == 0;
}

#endif /* UDHCP_SERVER_MESSAGE_POLICY_H */
