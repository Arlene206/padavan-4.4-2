#ifndef PADAVAN_RSTATS_PERSISTENCE_POLICY_H
#define PADAVAN_RSTATS_PERSISTENCE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Failed periodic flash saves stay due and are retried on the next rstats
 * tick. Saturation avoids unbounded counter growth while storage is down. */
/* Checked NVRAM reads use 0=missing, 1=present and negative=backend error.
 * Writes use zero=success; preserve errno when the backend reports failure. */
static inline int
rstats_nvram_read_status(int read_result, int saved_errno)
{
    if (read_result == 0 || read_result == 1)
        return 0;
    if (read_result > 1)
        return EIO;
    return saved_errno > 0 ? saved_errno : EIO;
}

/*
 * Copy a selector returned by nvram_get_checked(). The getter exposes a
 * shared cache pointer, so copy it before any later NVRAM operation.
 * Missing keys preserve the legacy empty/default selection.
 */
static inline int
rstats_nvram_copy_selector(int read_result, int saved_errno,
                           const char *value, char *destination,
                           size_t capacity)
{
    int status;
    size_t length;

    if (!destination || capacity == 0)
        return EINVAL;
    destination[0] = '\0';

    status = rstats_nvram_read_status(read_result, saved_errno);
    if (status != 0)
        return status;
    if (read_result == 0)
        return 0;
    if (!value)
        return EIO;

    length = strlen(value);
    if (length >= capacity)
        return ENAMETOOLONG;
    memcpy(destination, value, length + 1U);
    return 0;
}

static inline int
rstats_nvram_write_status(int write_result, int saved_errno)
{
    if (write_result == 0)
        return 0;
    if (write_result > 0)
        return write_result;
    return saved_errno > 0 ? saved_errno : EIO;
}

static inline int
rstats_storage_save_result(long *elapsed, long threshold,
                           int operation_status, int saved_errno)
{
    if (!elapsed || threshold < 1)
        return EINVAL;
    if (operation_status == 0) {
        *elapsed = 0;
        return 0;
    }
    *elapsed = threshold;
    if (operation_status > 0)
        return operation_status;
    return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* PADAVAN_RSTATS_PERSISTENCE_POLICY_H */
