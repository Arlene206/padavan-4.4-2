#ifndef PADAVAN_RSTATS_PERSISTENCE_POLICY_H
#define PADAVAN_RSTATS_PERSISTENCE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Failed periodic flash saves stay due and are retried on the next rstats
 * tick. Saturation avoids unbounded counter growth while storage is down. */
/* Checked NVRAM reads use 0=missing, 1=present and negative=backend error.
 * Writes use zero=success; preserve errno when the backend reports failure. */
/*
 * Raw history records persist ring cursors outside process memory. Validate
 * both cursors before loading them; bump_history() uses them as array indices.
 */
static inline int
rstats_history_ring_indices_valid(int daily_tail, int daily_capacity,
                                  int monthly_tail, int monthly_capacity)
{
    return daily_capacity > 0 && monthly_capacity > 0 &&
           daily_tail >= 0 && daily_tail < daily_capacity &&
           monthly_tail >= 0 && monthly_tail < monthly_capacity;
}

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
 * nvram_get_int() historically maps an absent key or a failed read to zero.
 * Keep the absent-key default and atoi parsing, but never interpret a backend
 * failure as an explicit disabled setting.
 */
static inline int
rstats_nvram_storage_mode(int read_result, int saved_errno,
                          const char *value, int *storage_mode)
{
    int status;

    if (!storage_mode)
        return EINVAL;
    status = rstats_nvram_read_status(read_result, saved_errno);
    if (status != 0)
        return status;
    if (read_result == 0) {
        *storage_mode = 0;
        return 0;
    }
    if (!value)
        return EIO;
    *storage_mode = atoi(value);
    return 0;
}

/*
 * Copy a selector returned by nvram_get_checked(). The getter exposes a
 * shared cache pointer, so copy it before any later NVRAM operation.
 * Missing keys preserve the legacy empty/default selection.
 */
/*
 * Preserve nvram_match("rstats_stored", "0") semantics while distinguishing
 * backend failures: a missing key or any value other than the exact string
 * "0" permits the legacy history load, but an unreadable setting is UNKNOWN.
 */
static inline int
rstats_nvram_history_load_status(int read_result, int saved_errno,
                                 const char *value, int *load_history)
{
    int status;

    if (!load_history)
        return EINVAL;
    *load_history = 0;
    status = rstats_nvram_read_status(read_result, saved_errno);
    if (status != 0)
        return status;
    if (read_result == 0) {
        *load_history = 1;
        return 0;
    }
    if (!value)
        return EIO;
    *load_history = strcmp(value, "0") != 0;
    return 0;
}

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
rstats_history_file_write_status(int write_result, int saved_errno)
{
    if (write_result == 0)
        return 0;
    if (write_result > 0)
        return EIO;
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

/*
 * Parse one complete Linux /proc/net/dev row. The counters are RX bytes,
 * seven other RX fields, TX bytes, and seven other TX fields. Only the byte
 * counters are retained, but the whole row must be present before it can be
 * committed to the live speed history.
 */
static inline int
rstats_netdev_parse_record(const char *line, char *ifname,
                           size_t ifname_capacity,
                           uint64_t *rx_bytes, uint64_t *tx_bytes)
{
    const char *colon, *name_begin, *name_end, *p;
    uint64_t rx = 0, tx = 0;
    size_t name_length;
    unsigned int field;

    if (!line || !ifname || ifname_capacity == 0 ||
        !rx_bytes || !tx_bytes)
        return EINVAL;

    colon = strchr(line, ':');
    if (!colon)
        return EINVAL;

    name_begin = line;
    while (*name_begin == ' ' || *name_begin == '\t')
        ++name_begin;
    name_end = colon;
    while (name_end > name_begin &&
           (name_end[-1] == ' ' || name_end[-1] == '\t'))
        --name_end;
    name_length = (size_t)(name_end - name_begin);
    if (name_length == 0)
        return EINVAL;
    if (name_length >= ifname_capacity)
        return ENAMETOOLONG;

    p = colon + 1;
    for (field = 0; field < 16; ++field) {
        uint64_t value = 0;
        int have_digit = 0;

        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            ++p;
        while (*p >= '0' && *p <= '9') {
            unsigned int digit = (unsigned int)(*p - '0');

            have_digit = 1;
            if (value > (~(uint64_t)0 - digit) / 10U)
                return ERANGE;
            value = value * 10U + digit;
            ++p;
        }
        if (!have_digit)
            return EINVAL;
        if (*p != '\0' && *p != ' ' && *p != '\t' &&
            *p != '\r' && *p != '\n')
            return EINVAL;
        if (field == 0)
            rx = value;
        else if (field == 8)
            tx = value;
    }

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        ++p;
    if (*p != '\0')
        return EINVAL;

    memcpy(ifname, name_begin, name_length);
    ifname[name_length] = '\0';
    *rx_bytes = rx;
    *tx_bytes = tx;
    return 0;
}

/* A failed read or close invalidates the entire interface-counter snapshot. */
static inline int
rstats_netdev_snapshot_status(int read_failed, int read_errno,
                              int close_status, int close_errno)
{
    if (read_failed)
        return read_errno > 0 ? read_errno : EIO;
    if (close_status != 0)
        return close_errno > 0 ? close_errno : EIO;
    return 0;
}

#endif /* PADAVAN_RSTATS_PERSISTENCE_POLICY_H */
