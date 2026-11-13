#ifndef NVRAM_RESTORE_TRANSACTION_H
#define NVRAM_RESTORE_TRANSACTION_H
#include <errno.h>
#include <string.h>
#include "nvram_profile_policy.h"

struct nvram_restore_transaction_ops {
    int (*clear)(void *context);
    int (*set)(void *context, const char *name, const char *value, int is_temporary);
    int (*should_apply)(void *context, const char *name);
    void *context;
};

static int
nvram_restore_transaction_apply_records(char *buffer, unsigned long length,
                                        int is_temporary, int filter_system,
                                        const struct nvram_restore_transaction_ops *ops,
                                        int continue_after_error, int *first_error)
{
    unsigned long position = 0;
    int failed = 0;

    while (position < length && buffer[position] != '\0') {
        unsigned long end = position, equal;
        int result = 0;
        while (end < length && buffer[end] != '\0')
            ++end;
        if (end == length) {
            errno = EINVAL;
            return -1;
        }
        equal = position;
        while (equal < end && buffer[equal] != '=')
            ++equal;
        if (equal == position || equal == end) {
            errno = EINVAL;
            return -1;
        }
        buffer[equal] = '\0';
        if (!filter_system || ops->should_apply(ops->context, buffer + position))
            result = ops->set(ops->context, buffer + position,
                              buffer + equal + 1, is_temporary);
        buffer[equal] = '=';
        if (result < 0) {
            int saved_errno = errno ? errno : EIO;
            failed = 1;
            if (first_error && *first_error == 0)
                *first_error = saved_errno;
            if (!continue_after_error) {
                errno = saved_errno;
                return -1;
            }
        }
        position = end + 1;
    }
    return failed ? -1 : 0;
}

/* Apply a profile; on failure rebuild the persistent and volatile pre-restore state. */
static int
nvram_restore_transaction_run(char *profile, unsigned long profile_length,
                              char *persistent_snapshot, unsigned long persistent_length,
                              char *temporary_snapshot, unsigned long temporary_length,
                              unsigned long max_name_length, unsigned long max_value_length,
                              const struct nvram_restore_transaction_ops *ops,
                              int *rollback_failed)
{
    int primary_error = 0, rollback_error = 0;

    if (rollback_failed)
        *rollback_failed = 0;
    if (!profile || !persistent_snapshot || !temporary_snapshot ||
        !ops || !ops->clear || !ops->set || !ops->should_apply ||
        !rollback_failed ||
        !nvram_profile_payload_valid((unsigned char *)profile, profile_length,
                                     max_name_length, max_value_length) ||
        !nvram_profile_payload_valid((unsigned char *)persistent_snapshot,
                                     persistent_length, max_name_length, max_value_length) ||
        !nvram_profile_payload_valid((unsigned char *)temporary_snapshot,
                                     temporary_length, max_name_length, max_value_length)) {
        errno = EINVAL;
        return -1;
    }

    if (ops->clear(ops->context) < 0) {
        primary_error = errno ? errno : EIO;
        goto rollback;
    }
    if (nvram_restore_transaction_apply_records(
            profile, profile_length, 0, 1, ops, 0, &primary_error) < 0) {
        if (primary_error == 0)
            primary_error = errno ? errno : EIO;
        goto rollback;
    }
    return 0;

rollback:
    if (ops->clear(ops->context) < 0)
        *rollback_failed = 1;
    if (nvram_restore_transaction_apply_records(
            persistent_snapshot, persistent_length, 0, 0, ops, 1,
            &rollback_error) < 0)
        *rollback_failed = 1;
    if (nvram_restore_transaction_apply_records(
            temporary_snapshot, temporary_length, 1, 0, ops, 1,
            &rollback_error) < 0)
        *rollback_failed = 1;
    errno = primary_error;
    return -1;
}
#endif
