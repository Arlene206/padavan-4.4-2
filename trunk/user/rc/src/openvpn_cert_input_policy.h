#ifndef PADAVAN_OPENVPN_CERT_INPUT_POLICY_H
#define PADAVAN_OPENVPN_CERT_INPUT_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

/* Certificate lifetimes are expressed in days by openvpn-cert.sh. */
#define OPENVPN_CERT_DAYS_MIN 1
#define OPENVPN_CERT_DAYS_MAX 36500

/*
 * Parse the optional client certificate lifetime without atoi()'s prefix
 * acceptance or overflow ambiguity. Empty input retains the historical
 * 365-day default; explicit values are limited to a generous 100 years.
 */
static inline int
openvpn_cert_days_valid_parse(const char *value, int default_value,
                              int *result)
{
    const unsigned char *cursor;
    char *end;
    long parsed;

    if (!result || default_value < OPENVPN_CERT_DAYS_MIN ||
        default_value > OPENVPN_CERT_DAYS_MAX)
        return EINVAL;

    if (!value) {
        *result = default_value;
        return 0;
    }

    cursor = (const unsigned char *)value;
    while (*cursor && isspace(*cursor))
        ++cursor;
    if (!*cursor) {
        *result = default_value;
        return 0;
    }

    if (*cursor == '+')
        ++cursor;
    if (!isdigit(*cursor))
        return EINVAL;
    while (isdigit(*cursor))
        ++cursor;
    while (*cursor && isspace(*cursor))
        ++cursor;
    if (*cursor)
        return EINVAL;

    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno == ERANGE)
        return ERANGE;
    if (end == value)
        return EINVAL;
    if (parsed < OPENVPN_CERT_DAYS_MIN ||
        parsed > OPENVPN_CERT_DAYS_MAX)
        return ERANGE;

    *result = (int)parsed;
    return 0;
}

#endif /* PADAVAN_OPENVPN_CERT_INPUT_POLICY_H */
