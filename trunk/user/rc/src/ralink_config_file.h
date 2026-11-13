#ifndef RALINK_CONFIG_FILE_H
#define RALINK_CONFIG_FILE_H

#include <errno.h>
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

#endif /* RALINK_CONFIG_FILE_H */
