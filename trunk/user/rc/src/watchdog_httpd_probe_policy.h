#ifndef WATCHDOG_HTTPD_PROBE_POLICY_H
#define WATCHDOG_HTTPD_PROBE_POLICY_H

#include <errno.h>
#include <stdlib.h>

/* Login uptime must be a complete, unsigned decimal value. */
static inline int
watchdog_httpd_login_timestamp_parse(const char *text,
                                     unsigned long *timestamp)
{
    const char *cursor;
    char *end;
    unsigned long parsed;

    if (!text || !timestamp || !text[0])
        return EINVAL;
    for (cursor = text; *cursor; ++cursor) {
        if (*cursor < '0' || *cursor > '9')
            return EINVAL;
    }

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno == ERANGE)
        return ERANGE;
    if (end == text || *end != '\0')
        return EINVAL;

    *timestamp = parsed;
    return 0;
}

/*
 * A watchdog HTTP probe can trigger a restart. Missing login state, HTTPS,
 * malformed ports and any checked-read failure must therefore skip it.
 */
static inline int
watchdog_httpd_probe_config_usable(int read_status, int login_timestamp_present,
                                   int http_port, int http_proto)
{
    return read_status == 0 && login_timestamp_present &&
           http_port >= 1 && http_port <= 65535 && http_proto == 0;
}

/* Do not call a service dead when its port changed or could not be read. */
static inline int
watchdog_httpd_probe_port_unchanged(int read_status, int current_port,
                                    int expected_port)
{
    return read_status == 0 && expected_port >= 1 && expected_port <= 65535 &&
           current_port == expected_port;
}

/* A missing prior result is fine; other unlink failures leave stale data. */
static inline int
watchdog_httpd_probe_stale_file_remove_status(int remove_result,
                                               int saved_errno)
{
    if (remove_result == 0 || saved_errno == ENOENT)
        return 0;
    return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* WATCHDOG_HTTPD_PROBE_POLICY_H */
