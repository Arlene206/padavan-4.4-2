#ifndef HTTPD_REQUEST_LINE_POLICY_H
#define HTTPD_REQUEST_LINE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define HTTPD_REQUEST_LINE_READ_ERROR (-1)
#define HTTPD_REQUEST_LINE_READ_EOF 0
#define HTTPD_REQUEST_LINE_READ_OK 1

/*
 * Read exactly one bounded HTTP request/header line.
 *
 * fgets() cannot report how many bytes it consumed when an input line contains
 * an embedded NUL. The caller then parses a truncated C string and may
 * disagree with the bytes actually received. Read bytewise so every input
 * byte is accounted for before the line is exposed to string parsers.
 *
 * The returned line includes LF (and an optional preceding CR). A line that
 * is too long, contains NUL, or ends prematurely is rejected. EOF before any
 * byte is distinct from an incomplete line.
 */
static inline int
httpd_request_line_read(FILE *stream, char *buffer, size_t capacity)
{
    size_t length = 0;
    int ch;

    if (!stream || !buffer || capacity < 2U) {
        errno = EINVAL;
        return HTTPD_REQUEST_LINE_READ_ERROR;
    }

    while (length < capacity - 1U) {
        ch = fgetc(stream);
        if (ch == EOF) {
            if (ferror(stream)) {
                if (errno == 0)
                    errno = EIO;
                return HTTPD_REQUEST_LINE_READ_ERROR;
            }
            if (length == 0U)
                return HTTPD_REQUEST_LINE_READ_EOF;
            errno = EPROTO;
            return HTTPD_REQUEST_LINE_READ_ERROR;
        }

        if (ch == '\0') {
            errno = EPROTO;
            return HTTPD_REQUEST_LINE_READ_ERROR;
        }

        buffer[length++] = (char)ch;
        if (ch == '\n') {
            buffer[length] = '\0';
            return HTTPD_REQUEST_LINE_READ_OK;
        }
    }

    errno = EMSGSIZE;
    return HTTPD_REQUEST_LINE_READ_ERROR;
}

#endif /* HTTPD_REQUEST_LINE_POLICY_H */
