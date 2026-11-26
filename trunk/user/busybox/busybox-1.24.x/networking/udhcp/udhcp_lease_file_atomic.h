/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_LEASE_FILE_ATOMIC_H
#define UDHCP_LEASE_FILE_ATOMIC_H

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct udhcp_lease_file_io_ops {
    int (*mkstemp_fn)(char *path_template);
    ssize_t (*write_fn)(int fd, const void *buffer, size_t length);
    int (*fsync_fn)(int fd);
    int (*close_fn)(int fd);
    int (*rename_fn)(const char *old_path, const char *new_path);
    int (*unlink_fn)(const char *path);
    int (*open_dir_fn)(const char *path);
    int (*fsync_dir_fn)(int fd);
    int (*close_dir_fn)(int fd);
};

#define UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED 1

static inline int
udhcp_lease_file_write_all(int fd, const void *buffer, size_t length,
                           const struct udhcp_lease_file_io_ops *ops)
{
    const unsigned char *cursor = buffer;

    while (length != 0) {
        ssize_t written = ops->write_fn(fd, cursor, length);

        if (written < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (written == 0 || (size_t)written > length) {
            errno = EIO;
            return -1;
        }
        cursor += written;
        length -= (size_t)written;
    }

    return 0;
}

/*
 * Write beside the destination and rename only after the complete payload
 * has been written, synced and closed. Then sync the parent directory so
 * the rename itself is durable. Return 1 when rename succeeded but directory
 * fsync failed; the new file is visible, but crash durability is uncertain.
 */
static inline int
udhcp_lease_file_write_atomic(const char *path, const void *buffer,
                              size_t length,
                              const struct udhcp_lease_file_io_ops *ops)
{
    static const char suffix[] = ".tmp.XXXXXX";
    size_t path_length;
    char *temporary_path;
    char *parent_path;
    char *last_slash;
    int fd = -1;
    int directory_fd = -1;
    int temporary_created = 0;
    int saved_errno;

    if (!path || path[0] == '\0' || (!buffer && length != 0) || !ops ||
        !ops->mkstemp_fn || !ops->write_fn || !ops->fsync_fn ||
        !ops->close_fn || !ops->rename_fn || !ops->unlink_fn ||
        !ops->open_dir_fn || !ops->fsync_dir_fn || !ops->close_dir_fn) {
        errno = EINVAL;
        return -1;
    }

    path_length = strlen(path);
    if (path_length > (size_t)-1 - sizeof(suffix)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    temporary_path = malloc(path_length + sizeof(suffix));
    parent_path = malloc(path_length + 1);
    if (!temporary_path || !parent_path) {
        free(temporary_path);
        free(parent_path);
        errno = ENOMEM;
        return -1;
    }
    memcpy(temporary_path, path, path_length);
    memcpy(temporary_path + path_length, suffix, sizeof(suffix));

    memcpy(parent_path, path, path_length + 1);
    last_slash = strrchr(parent_path, '/');
    if (!last_slash) {
        parent_path[0] = '.';
        parent_path[1] = '\0';
    } else if (last_slash == parent_path) {
        parent_path[1] = '\0';
    } else {
        *last_slash = '\0';
    }

    directory_fd = ops->open_dir_fn(parent_path);
    if (directory_fd < 0)
        goto fail;

    fd = ops->mkstemp_fn(temporary_path);
    if (fd < 0)
        goto fail;
    temporary_created = 1;
    if (udhcp_lease_file_write_all(fd, buffer, length, ops) != 0)
        goto fail;
    if (ops->fsync_fn(fd) != 0)
        goto fail;
    if (ops->close_fn(fd) != 0) {
        fd = -1;
        goto fail;
    }
    fd = -1;
    if (ops->rename_fn(temporary_path, path) != 0)
        goto fail;
    temporary_created = 0;

    if (ops->fsync_dir_fn(directory_fd) != 0) {
        saved_errno = errno;
        ops->close_dir_fn(directory_fd);
        free(parent_path);
        free(temporary_path);
        errno = saved_errno;
        return UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED;
    }

    ops->close_dir_fn(directory_fd);
    free(parent_path);
    free(temporary_path);
    return 0;

fail:
    saved_errno = errno;
    if (fd >= 0)
        ops->close_fn(fd);
    if (temporary_created)
        ops->unlink_fn(temporary_path);
    if (directory_fd >= 0)
        ops->close_dir_fn(directory_fd);
    free(parent_path);
    free(temporary_path);
    errno = saved_errno;
    return -1;
}

#endif /* UDHCP_LEASE_FILE_ATOMIC_H */
