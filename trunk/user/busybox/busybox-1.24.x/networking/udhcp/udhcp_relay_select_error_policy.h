/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_SELECT_ERROR_POLICY_H
#define UDHCP_RELAY_SELECT_ERROR_POLICY_H

#include <errno.h>

static inline int
udhcp_relay_select_was_interrupted(int error_number)
{
    return error_number == EINTR;
}

#endif /* UDHCP_RELAY_SELECT_ERROR_POLICY_H */
