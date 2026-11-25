/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_RELAY_SELECT_FD_POLICY_H
#define UDHCP_RELAY_SELECT_FD_POLICY_H

static inline int
udhcp_relay_fd_fits_select(int fd, int fd_set_size)
{
    return fd >= 0 && fd_set_size > 0 && fd < fd_set_size;
}

#endif /* UDHCP_RELAY_SELECT_FD_POLICY_H */
