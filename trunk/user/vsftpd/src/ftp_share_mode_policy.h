#ifndef PADAVAN_VSFTPD_SHARE_MODE_POLICY_H
#define PADAVAN_VSFTPD_SHARE_MODE_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/*
 * Keep the per-session authorization mode consistent with the configuration
 * publisher: only modes 1..4 are supported, and absent/unsupported values
 * select mode 2 (authenticated local users only). Never let atoi() turn a
 * malformed numeric prefix such as "1junk" into anonymous mode 1.
 */
static inline int
ftp_share_mode_read_status(int api_result, int saved_errno,
                           const char *text, int *mode)
{
    const char *start;
    char *end;
    long parsed;

    if (!mode)
        return EINVAL;
    *mode = 2;

    if (api_result == 0)
        return 0;
    if (api_result < 0)
        return saved_errno > 0 ? saved_errno : EIO;
    if (api_result != 1 || !text)
        return EIO;

    start = text;
    while (*start && isspace((unsigned char)*start))
        ++start;
    if (!*start) {
        parsed = 0;
    } else {
        errno = 0;
        parsed = strtol(start, &end, 10);
        if (end == start)
            return EINVAL;
        if (errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
            return ERANGE;
        while (*end && isspace((unsigned char)*end))
            ++end;
        if (*end)
            return EINVAL;
    }

    if (parsed < 1 || parsed > 4)
        parsed = 2;
    *mode = (int)parsed;
    return 0;
}

#endif /* PADAVAN_VSFTPD_SHARE_MODE_POLICY_H */
