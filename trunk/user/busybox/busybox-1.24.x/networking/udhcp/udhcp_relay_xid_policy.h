/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_XID_POLICY_H
#define UDHCP_RELAY_XID_POLICY_H

#define UDHCP_RELAY_MAX_PENDING_XIDS 1024U

struct udhcp_relay_xid_budget {
    unsigned int pending;
};

static inline int
udhcp_relay_xid_reserve(struct udhcp_relay_xid_budget *budget)
{
    if (NULL == budget || budget->pending >= UDHCP_RELAY_MAX_PENDING_XIDS)
        return 0;
    ++budget->pending;
    return 1;
}

static inline void
udhcp_relay_xid_release(struct udhcp_relay_xid_budget *budget)
{
    if (NULL != budget && budget->pending > 0U)
        --budget->pending;
}

#endif /* UDHCP_RELAY_XID_POLICY_H */
