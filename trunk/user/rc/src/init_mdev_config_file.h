#ifndef INIT_MDEV_CONFIG_FILE_H
#define INIT_MDEV_CONFIG_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/* Open a private same-directory staging file for the root-owned mdev policy. */
static inline int
init_mdev_config_open(const char *path, char *staged_path,
                      size_t staged_path_size, FILE **stream)
{
    int fd;
    int saved_errno;
    int path_len;
    FILE *fp;

    if (!path || !*path || !staged_path || staged_path_size == 0 || !stream)
        return -EINVAL;

    *stream = NULL;
    staged_path[0] = '\0';
    path_len = snprintf(staged_path, staged_path_size, "%s.tmp.XXXXXX", path);
    if (path_len < 0 || (size_t)path_len >= staged_path_size) {
        staged_path[0] = '\0';
        return -ENAMETOOLONG;
    }

    fd = mkstemp(staged_path);
    if (fd < 0) {
        saved_errno = errno ? errno : EIO;
        staged_path[0] = '\0';
        return -saved_errno;
    }

    /* PID 1 uses umask(0000); explicitly keep the policy non-writable. */
    if (fchmod(fd, 0644) < 0) {
        saved_errno = errno ? errno : EIO;
        close(fd);
        unlink(staged_path);
        staged_path[0] = '\0';
        return -saved_errno;
    }

    fp = fdopen(fd, "w");
    if (!fp) {
        saved_errno = errno ? errno : EIO;
        close(fd);
        unlink(staged_path);
        staged_path[0] = '\0';
        return -saved_errno;
    }

    *stream = fp;
    return 0;
}

/* Publish only a complete, flushed, durable staging file; retain old on error. */
static inline int
init_mdev_config_publish(FILE **stream, const char *staged_path,
                         const char *path)
{
    FILE *fp;
    int saved_errno = 0;

    if (!stream || !*stream || !staged_path || !*staged_path ||
        !path || !*path)
        return -EINVAL;

    fp = *stream;
    *stream = NULL;

    if (ferror(fp))
        saved_errno = errno ? errno : EIO;
    if (!saved_errno && fflush(fp) != 0)
        saved_errno = errno ? errno : EIO;
    if (!saved_errno && fsync(fileno(fp)) < 0)
        saved_errno = errno ? errno : EIO;
    if (fclose(fp) != 0 && !saved_errno)
        saved_errno = errno ? errno : EIO;
    if (!saved_errno && rename(staged_path, path) < 0)
        saved_errno = errno ? errno : EIO;

    if (saved_errno) {
        unlink(staged_path);
        return -saved_errno;
    }

    return 0;
}

#endif /* INIT_MDEV_CONFIG_FILE_H */
