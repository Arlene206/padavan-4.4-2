#ifndef PADAVAN_RSTATS_PERSISTENCE_POLICY_H
#define PADAVAN_RSTATS_PERSISTENCE_POLICY_H

#include <errno.h>

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
