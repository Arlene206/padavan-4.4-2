#ifndef NVRAM_PROFILE_FILE_H
#define NVRAM_PROFILE_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct nvram_profile_file_ops {
    int (*mkstemp_fn)(char *template_path);
    int (*fchmod_fn)(int fd, mode_t mode);
    ssize_t (*write_fn)(int fd, const void *buffer, size_t length);
    int (*fsync_fn)(int fd);
    int (*close_fn)(int fd);
    int (*rename_fn)(const char *old_path, const char *new_path);
    int (*unlink_fn)(const char *path);
};

static const struct nvram_profile_file_ops nvram_profile_file_system_ops = {
    mkstemp, fchmod, write, fsync, close, rename, unlink
};

static inline int
nvram_profile_file_write_segment(int fd, const void *buffer, size_t length,
                                 const struct nvram_profile_file_ops *ops)
{
    const unsigned char *cursor = (const unsigned char *)buffer;
    size_t written = 0;

    if (!buffer && length != 0) {
        errno = EINVAL;
        return -1;
    }

    while (written < length) {
        ssize_t result = ops->write_fn(fd, cursor + written, length - written);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (result == 0) {
            errno = EIO;
            return -1;
        }
        written += (size_t)result;
    }

    return 0;
}

/*
 * Publish a complete NVRAM profile via a private same-directory staging file.
 * Existing non-regular objects are rejected; profile files contain credentials
 * and are therefore always published with owner-only permissions.
 */
static inline int
nvram_profile_write_atomic_with_ops(const char *path,
                                    const void *header, size_t header_length,
                                    const void *payload, size_t payload_length,
                                    const void *padding, size_t padding_length,
                                    const struct nvram_profile_file_ops *ops)
{
    static const char suffix[] = ".tmp.XXXXXX";
    struct stat existing;
    char *temporary;
    size_t path_length;
    int fd = -1;
    int created = 0;
    int saved_errno;
    int count;
    int result;

    if (!path || !*path || (!header && header_length != 0) ||
        (!payload && payload_length != 0) || (!padding && padding_length != 0)) {
        errno = EINVAL;
        return -1;
    }

    if (!ops)
        ops = &nvram_profile_file_system_ops;
    if (!ops->mkstemp_fn || !ops->fchmod_fn || !ops->write_fn ||
        !ops->fsync_fn || !ops->close_fn || !ops->rename_fn ||
        !ops->unlink_fn) {
        errno = EINVAL;
        return -1;
    }

    if (lstat(path, &existing) == 0) {
        if (!S_ISREG(existing.st_mode)) {
            errno = EINVAL;
            return -1;
        }
    } else if (errno != ENOENT) {
        return -1;
    }

    path_length = strlen(path);
    if (path_length > (size_t)-1 - sizeof(suffix)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    temporary = (char *)malloc(path_length + sizeof(suffix));
    if (!temporary)
        return -1;

    count = snprintf(temporary, path_length + sizeof(suffix), "%s%s",
                     path, suffix);
    if (count < 0 || (size_t)count >= path_length + sizeof(suffix)) {
        free(temporary);
        errno = ENAMETOOLONG;
        return -1;
    }

    fd = ops->mkstemp_fn(temporary);
    if (fd < 0) {
        saved_errno = errno ? errno : EIO;
        free(temporary);
        errno = saved_errno;
        return -1;
    }
    created = 1;

    if (ops->fchmod_fn(fd, 0600) != 0)
        goto fail;
    if (nvram_profile_file_write_segment(fd, header, header_length, ops) != 0 ||
        nvram_profile_file_write_segment(fd, payload, payload_length, ops) != 0 ||
        nvram_profile_file_write_segment(fd, padding, padding_length, ops) != 0)
        goto fail;

    do {
        result = ops->fsync_fn(fd);
    } while (result < 0 && errno == EINTR);
    if (result != 0)
        goto fail;

    if (ops->close_fn(fd) != 0) {
        fd = -1; /* Linux releases the descriptor even when close reports EINTR. */
        goto fail;
    }
    fd = -1;

    if (ops->rename_fn(temporary, path) != 0)
        goto fail;

    free(temporary);
    return 0;

fail:
    saved_errno = errno ? errno : EIO;
    if (fd >= 0)
        (void)ops->close_fn(fd);
    if (created)
        (void)ops->unlink_fn(temporary);
    free(temporary);
    errno = saved_errno;
    return -1;
}

static inline int
nvram_profile_write_atomic(const char *path,
                           const void *header, size_t header_length,
                           const void *payload, size_t payload_length,
                           const void *padding, size_t padding_length)
{
    return nvram_profile_write_atomic_with_ops(
        path, header, header_length, payload, payload_length,
        padding, padding_length, &nvram_profile_file_system_ops);
}

#endif /* NVRAM_PROFILE_FILE_H */
