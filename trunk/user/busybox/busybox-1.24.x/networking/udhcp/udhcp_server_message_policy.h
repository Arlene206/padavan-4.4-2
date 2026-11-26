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

/* RFC 2131: DHCPINFORM requires the client's already configured address. */
static inline int
udhcp_server_inform_ciaddr_valid(uint32_t ciaddr)
{
    return ciaddr != 0U;
}

/*
 * DHCPREQUEST field combinations from RFC 2131:
 * SELECTING: server-id + requested IP, ciaddr zero.
 * INIT-REBOOT: requested IP only, ciaddr zero.
 * RENEWING/REBINDING: ciaddr nonzero, neither option present.
 */
static inline int
udhcp_server_request_fields_valid(int server_id_present,
                                  int requested_ip_present,
                                  uint32_t ciaddr)
{
    if (ciaddr == 0U)
        return requested_ip_present != 0;
    return server_id_present == 0 && requested_ip_present == 0;
}

/*
 * SELECTING may be NAKed if this server cannot honor the request.
 * INIT-REBOOT must remain silent when this server has no client record;
 * RENEWING/REBINDING are not NAKed by this request path.
 * The field-combination validator has already rejected invalid tuples.
 */
static inline int
udhcp_server_request_should_nak(int server_id_present,
                                int requested_ip_present,
                                int lease_known)
{
    return server_id_present != 0 ||
           (requested_ip_present != 0 && lease_known != 0);
}

#endif /* UDHCP_SERVER_MESSAGE_POLICY_H */
