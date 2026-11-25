#ifndef MINIUPNPD_HTTP_REQUEST_POLICY_H
#define MINIUPNPD_HTTP_REQUEST_POLICY_H

#include <errno.h>
#include <stddef.h>

#define MINIUPNPD_HTTP_MAX_HEADER_BYTES 8192U
#define MINIUPNPD_HTTP_MAX_BODY_BYTES 65536U
#define MINIUPNPD_HTTP_MAX_REQUEST_BYTES \
    (MINIUPNPD_HTTP_MAX_HEADER_BYTES + MINIUPNPD_HTTP_MAX_BODY_BYTES)

/*
 * Parse an exact Content-Length field value (excluding field name and CRLF).
 * Only optional HTTP whitespace around decimal digits is accepted. The
 * destination is unchanged when the value is invalid or exceeds the limit.
 */
static inline int
miniupnpd_http_parse_content_length(const char *value, size_t length,
                                    size_t *length_out)
{
    size_t first = 0;
    size_t last = length;
    size_t parsed = 0;

    if (!value || !length_out) {
        errno = EINVAL;
        return -1;
    }

    while (first < last &&
           (value[first] == ' ' || value[first] == '\t'))
        ++first;
    while (last > first &&
           (value[last - 1] == ' ' || value[last - 1] == '\t'))
        --last;
    if (first == last) {
        errno = EINVAL;
        return -1;
    }

    while (first < last) {
        unsigned int digit;
        unsigned int character = (unsigned char)value[first++];

        if (character < '0' || character > '9') {
            errno = EINVAL;
            return -1;
        }
        digit = character - '0';
        if (parsed > ((size_t)MINIUPNPD_HTTP_MAX_BODY_BYTES -
                      (size_t)digit) / 10U) {
            errno = ERANGE;
            return -1;
        }
        parsed = parsed * 10U + (size_t)digit;
    }

    *length_out = parsed;
    return 0;
}

/*
 * Accept only one Content-Length field. A duplicate is rejected even when
 * both values match, so different HTTP components cannot choose different
 * message boundaries.
 */
static inline int
miniupnpd_http_record_content_length(const char *value, size_t length,
                                     int *seen, size_t *length_out)
{
    size_t parsed;

    if (!seen || !length_out) {
        errno = EINVAL;
        return -1;
    }
    if (*seen) {
        errno = EINVAL;
        return -1;
    }
    if (miniupnpd_http_parse_content_length(value, length, &parsed) != 0)
        return -1;

    *seen = 1;
    *length_out = parsed;
    return 0;
}

/* Overflow-safe bound check used before every request-buffer growth. */
static inline int
miniupnpd_http_request_append_fits(size_t current, size_t incoming,
                                   size_t maximum)
{
    return current <= maximum && incoming <= maximum - current;
}

#endif /* MINIUPNPD_HTTP_REQUEST_POLICY_H */
