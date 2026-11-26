/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_REPLY_POLICY_H
#define UDHCP_RELAY_REPLY_POLICY_H

#define UDHCP_RELAY_BOOTREPLY_OPCODE 2U

static inline int
udhcp_relay_is_bootreply(unsigned char opcode)
{
    return opcode == UDHCP_RELAY_BOOTREPLY_OPCODE;
}

#endif /* UDHCP_RELAY_REPLY_POLICY_H */
