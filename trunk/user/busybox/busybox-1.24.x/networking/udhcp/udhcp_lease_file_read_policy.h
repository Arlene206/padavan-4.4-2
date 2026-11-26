/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_LEASE_FILE_READ_POLICY_H
#define UDHCP_LEASE_FILE_READ_POLICY_H

#include <stddef.h>
#include <sys/types.h>

enum udhcp_lease_file_read_status {
    UDHCP_LEASE_FILE_READ_ERROR = -1,
    UDHCP_LEASE_FILE_READ_EOF = 0,
    UDHCP_LEASE_FILE_READ_COMPLETE = 1
};

/*
 * full_read() returns zero only at EOF, a negative value on read failure,
 * and a positive byte count otherwise. A short positive record is corrupt,
 * not EOF; callers may retain earlier complete records and report the tail.
 */
static inline int
udhcp_lease_file_read_status(ssize_t bytes_read, size_t expected_size)
{
    if (bytes_read < 0)
        return UDHCP_LEASE_FILE_READ_ERROR;
    if (bytes_read == 0)
        return UDHCP_LEASE_FILE_READ_EOF;
    if ((size_t)bytes_read != expected_size)
        return UDHCP_LEASE_FILE_READ_ERROR;
    return UDHCP_LEASE_FILE_READ_COMPLETE;
}

#endif /* UDHCP_LEASE_FILE_READ_POLICY_H */
