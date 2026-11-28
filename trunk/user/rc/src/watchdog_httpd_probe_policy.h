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

/*
 * A missing result file is expected while detached wget starts. Other open,
 * read, and close errors make the probe inconclusive, not proof HTTPD is dead.
 */
static inline int
watchdog_httpd_probe_file_io_status(int open_status, int read_error,
                                    int read_errno, int close_result,
                                    int close_errno)
{
    if (open_status != 0)
        return open_status == ENOENT ? 0 : (open_status > 0 ? open_status : EIO);
    if (read_error)
        return read_errno > 0 ? read_errno : EIO;
    if (close_result != 0)
        return close_errno > 0 ? close_errno : EIO;
    return 0;
}

#endif /* WATCHDOG_HTTPD_PROBE_POLICY_H */
