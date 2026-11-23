#ifndef RALINK_CONFIG_FILE_H
#define RALINK_CONFIG_FILE_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Build a replacement in the destination directory so rename() is atomic
 * on the same filesystem. Keep new files owner-only and preserve old modes.
 */
static inline int
ralink_config_open_atomic(const char *path, char *temp_path,
                          size_t temp_path_size, FILE **stream)
{
    struct stat info;
    mode_t mode = 0600;
    int fd;
    int length;

    if (!path || !*path || !temp_path || !stream || temp_path_size == 0)
        return -1;

    *stream = NULL;
    temp_path[0] = '\0';

    if (lstat(path, &info) == 0) {
        if (!S_ISREG(info.st_mode))
            return -1;
        mode = info.st_mode & 0777;
    } else if (errno != ENOENT) {
        return -1;
    }

    length = snprintf(temp_path, temp_path_size, "%s.tmpXXXXXX", path);
    if (length < 0 || (size_t)length >= temp_path_size) {
        temp_path[0] = '\0';
        return -1;
    }

    fd = mkstemp(temp_path);
    if (fd < 0) {
        temp_path[0] = '\0';
        return -1;
    }

    if (fchmod(fd, mode) < 0) {
        close(fd);
        unlink(temp_path);
        temp_path[0] = '\0';
        return -1;
    }

    *stream = fdopen(fd, "w");
    if (!*stream) {
        close(fd);
        unlink(temp_path);
        temp_path[0] = '\0';
        return -1;
    }

    return 0;
}

/* Flush and synchronize data before closing; delayed write errors fail closed. */
static inline int
ralink_config_finish_stream(FILE *stream)
{
    int failed = 0;
    int fd;

    if (!stream)
        return -1;

    fd = fileno(stream);
    if (fflush(stream) != 0)
        failed = 1;
    if (ferror(stream))
        failed = 1;
    if (!failed && fsync(fd) != 0)
        failed = 1;
    if (fclose(stream) != 0)
        failed = 1;

    return failed ? -1 : 0;
}

static inline int
ralink_config_publish_atomic(FILE **stream, const char *temp_path,
                             const char *path)
{
    FILE *fp;
    int failed;

    if (!stream || !*stream || !temp_path || !*temp_path || !path || !*path)
        return -1;

    fp = *stream;
    *stream = NULL;
    failed = (ralink_config_finish_stream(fp) < 0);

    if (!failed && rename(temp_path, path) < 0)
        failed = 1;

    if (failed)
        unlink(temp_path);

    return failed ? -1 : 0;
}

static inline void
ralink_config_discard_atomic(FILE **stream, const char *temp_path)
{
    if (stream && *stream) {
        fclose(*stream);
        *stream = NULL;
    }
    if (temp_path && *temp_path)
        unlink(temp_path);
}


/*
 * Replace a privileged SKU symlink through a private directory on the same
 * filesystem. The old destination remains intact until rename() publishes
 * the new link atomically.
 */
static inline int
ralink_config_publish_symlink_atomic(const char *target, const char *link_path)
{
    char parent[PATH_MAX];
    char temporary_directory[PATH_MAX];
    char temporary_link[PATH_MAX];
    const char *slash;
    size_t parent_length;
    int length;
    int link_created = 0;
    int saved_errno;

    if (!target || !*target || !link_path || !*link_path) {
        errno = EINVAL;
        return -1;
    }

    slash = strrchr(link_path, '/');
    if (!slash) {
        memcpy(parent, ".", 2U);
    } else if (slash == link_path) {
        memcpy(parent, "/", 2U);
    } else {
        parent_length = (size_t)(slash - link_path);
        if (parent_length >= sizeof(parent)) {
            errno = ENAMETOOLONG;
            return -1;
        }
        memcpy(parent, link_path, parent_length);
        parent[parent_length] = '\0';
    }

    length = snprintf(temporary_directory, sizeof(temporary_directory),
                      "%s/.ralink-sku-link.XXXXXX", parent);
    if (length < 0 || (size_t)length >= sizeof(temporary_directory)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    if (!mkdtemp(temporary_directory))
        return -1;

    temporary_link[0] = '\0';
    length = snprintf(temporary_link, sizeof(temporary_link),
                      "%s/link", temporary_directory);
    if (length < 0 || (size_t)length >= sizeof(temporary_link)) {
        saved_errno = ENAMETOOLONG;
        goto failed;
    }

    if (symlink(target, temporary_link) != 0) {
        saved_errno = errno ? errno : EIO;
        goto failed;
    }
    link_created = 1;

    if (rename(temporary_link, link_path) != 0) {
        saved_errno = errno ? errno : EIO;
        goto failed;
    }
    link_created = 0;

    if (rmdir(temporary_directory) != 0)
        return -1;

    return 0;

failed:
    if (link_created)
        unlink(temporary_link);
    rmdir(temporary_directory);
    errno = saved_errno;
    return -1;
}

/* Missing optional links are already clean; other unlink errors are real. */
static inline int
ralink_config_unlink_optional(const char *path)
{
    if (!path || !*path) {
        errno = EINVAL;
        return -1;
    }
    if (unlink(path) == 0 || errno == ENOENT)
        return 0;
    return -1;
}

#endif /* RALINK_CONFIG_FILE_H */
