#ifndef RALINK_CONFIG_FILE_H
#define RALINK_CONFIG_FILE_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * Stage a privileged SKU-link update together with its previous inode.
 * Commit the snapshot only after the related wireless .dat configuration
 * has been published; otherwise rollback restores the previous path state.
 */
typedef struct {
    char destination[PATH_MAX];
    char directory[PATH_MAX];
    char backup[PATH_MAX];
    char staged_link[PATH_MAX];
    int had_original;
    int published;
    int active;
} ralink_config_symlink_transaction_t;

static inline int
ralink_config_symlink_transaction_begin(
    const char *link_path, ralink_config_symlink_transaction_t *transaction)
{
    struct stat info;
    char parent[PATH_MAX];
    const char *slash;
    size_t parent_length;
    int length;

    if (!link_path || !*link_path || !transaction) {
        errno = EINVAL;
        return -1;
    }

    memset(transaction, 0, sizeof(*transaction));
    length = snprintf(transaction->destination,
                      sizeof(transaction->destination), "%s", link_path);
    if (length < 0 || (size_t)length >= sizeof(transaction->destination)) {
        errno = ENAMETOOLONG;
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

    if (lstat(link_path, &info) == 0) {
        if (!S_ISREG(info.st_mode) && !S_ISLNK(info.st_mode)) {
            errno = EINVAL;
            return -1;
        }
        transaction->had_original = 1;
    } else if (errno != ENOENT) {
        return -1;
    }

    length = snprintf(transaction->directory,
                      sizeof(transaction->directory),
                      "%s/.ralink-sku-link.XXXXXX", parent);
    if (length < 0 || (size_t)length >= sizeof(transaction->directory)) {
        errno = ENAMETOOLONG;
        transaction->directory[0] = '\0';
        return -1;
    }
    if (!mkdtemp(transaction->directory))
        return -1;

    length = snprintf(transaction->backup, sizeof(transaction->backup),
                      "%s/original", transaction->directory);
    if (length < 0 || (size_t)length >= sizeof(transaction->backup)) {
        errno = ENAMETOOLONG;
        rmdir(transaction->directory);
        transaction->directory[0] = '\0';
        transaction->had_original = 0;
        return -1;
    }
    length = snprintf(transaction->staged_link,
                      sizeof(transaction->staged_link),
                      "%s/link", transaction->directory);
    if (length < 0 || (size_t)length >= sizeof(transaction->staged_link)) {
        errno = ENAMETOOLONG;
        rmdir(transaction->directory);
        transaction->directory[0] = '\0';
        transaction->had_original = 0;
        return -1;
    }

    if (transaction->had_original &&
        link(link_path, transaction->backup) != 0) {
        int saved_errno = errno ? errno : EIO;
        rmdir(transaction->directory);
        transaction->directory[0] = '\0';
        transaction->had_original = 0;
        errno = saved_errno;
        return -1;
    }

    transaction->active = 1;
    return 0;
}

static inline int
ralink_config_symlink_transaction_publish(
    ralink_config_symlink_transaction_t *transaction, const char *target)
{
    int saved_errno;

    if (!transaction || !transaction->active || transaction->published) {
        errno = EINVAL;
        return -1;
    }

    if (target && *target) {
        if (symlink(target, transaction->staged_link) != 0)
            return -1;
        if (rename(transaction->staged_link, transaction->destination) != 0) {
            saved_errno = errno ? errno : EIO;
            unlink(transaction->staged_link);
            errno = saved_errno;
            return -1;
        }
    } else if (unlink(transaction->destination) != 0 && errno != ENOENT) {
        return -1;
    }

    transaction->published = 1;
    return 0;
}

static inline int
ralink_config_symlink_transaction_rollback(
    ralink_config_symlink_transaction_t *transaction)
{
    if (!transaction || !transaction->active) {
        errno = EINVAL;
        return -1;
    }

    if (transaction->published) {
        if (transaction->had_original) {
            if (rename(transaction->backup, transaction->destination) != 0)
                return -1;
            transaction->had_original = 0;
        } else if (unlink(transaction->destination) != 0 && errno != ENOENT) {
            return -1;
        }
        transaction->published = 0;
    }

    if (unlink(transaction->staged_link) != 0 && errno != ENOENT)
        return -1;
    if (transaction->had_original) {
        if (unlink(transaction->backup) != 0 && errno != ENOENT)
            return -1;
        transaction->had_original = 0;
    }
    if (rmdir(transaction->directory) != 0)
        return -1;

    transaction->active = 0;
    return 0;
}

static inline int
ralink_config_symlink_transaction_commit(
    ralink_config_symlink_transaction_t *transaction)
{
    if (!transaction || !transaction->active || !transaction->published) {
        errno = EINVAL;
        return -1;
    }

    if (transaction->had_original) {
        if (unlink(transaction->backup) != 0 && errno != ENOENT)
            return -1;
        transaction->had_original = 0;
    }
    if (unlink(transaction->staged_link) != 0 && errno != ENOENT)
        return -1;
    if (rmdir(transaction->directory) != 0)
        return -1;

    transaction->active = 0;
    transaction->published = 0;
    return 0;
}

/* Convenience wrapper for callers without a companion configuration file. */
static inline int
ralink_config_publish_symlink_atomic(const char *target, const char *link_path)
{
    ralink_config_symlink_transaction_t transaction;
    int saved_errno;

    if (!target || !*target) {
        errno = EINVAL;
        return -1;
    }
    if (ralink_config_symlink_transaction_begin(link_path, &transaction) != 0)
        return -1;
    if (ralink_config_symlink_transaction_publish(&transaction, target) != 0) {
        saved_errno = errno ? errno : EIO;
        ralink_config_symlink_transaction_rollback(&transaction);
        errno = saved_errno;
        return -1;
    }
    return ralink_config_symlink_transaction_commit(&transaction);
}

/*
 * nvram_get_checked() returns 1 for a value, 0 for an absent optional key,
 * and a negative result on backend failure. Preserve that distinction so a
 * transient read error cannot silently select a different regulatory SKU.
 */
static inline int
ralink_config_nvram_read_status(int read_result, int read_errno,
                                const char *value)
{
    if (read_result < 0)
        return read_errno > 0 ? read_errno : EIO;
    if (read_result > 1 || (read_result == 1 && !value))
        return EIO;
    return 0;
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
