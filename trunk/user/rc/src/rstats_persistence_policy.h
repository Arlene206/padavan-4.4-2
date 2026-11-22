#ifndef PADAVAN_RSTATS_PERSISTENCE_POLICY_H
#define PADAVAN_RSTATS_PERSISTENCE_POLICY_H

#include <errno.h>

/* Failed periodic flash saves stay due and are retried on the next rstats
 * tick. Saturation avoids unbounded counter growth while storage is down. */
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
