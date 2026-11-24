#ifndef PADAVAN_EAPOL_CONFIG_POLICY_H
#define PADAVAN_EAPOL_CONFIG_POLICY_H

#include <errno.h>
#include <stdio.h>

/*
 * Emit a WPA supplicant quoted string without allowing control characters
 * to escape the value. Quotes and backslashes use the parser's C-style
 * string escaping rules.
 */
static inline int
wpa_eapol_config_write_quoted(FILE *stream, const char *value)
{
    const unsigned char *cursor;

    if (!stream || !value) {
        errno = EINVAL;
        return -1;
    }
    if (fputc('"', stream) == EOF)
        return -1;

    for (cursor = (const unsigned char *)value; *cursor; ++cursor) {
        if (*cursor < 0x20 || *cursor == 0x7f) {
            errno = EINVAL;
            return -1;
        }
        if (*cursor == '"' || *cursor == '\\') {
            if (fputc('\\', stream) == EOF)
                return -1;
        }
        if (fputc(*cursor, stream) == EOF)
            return -1;
    }

    if (fputc('"', stream) == EOF)
        return -1;
    return 0;
}

#endif /* PADAVAN_EAPOL_CONFIG_POLICY_H */
