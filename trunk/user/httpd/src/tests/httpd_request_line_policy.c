#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "httpd_request_line_policy.h"

static FILE *
stream_with_bytes(const void *bytes, size_t length)
{
    FILE *stream = tmpfile();

    assert(stream != NULL);
    if (length != 0U)
        assert(fwrite(bytes, 1, length, stream) == length);
    assert(fflush(stream) == 0);
    rewind(stream);
    return stream;
}

static void
test_complete_lf_and_crlf_lines_are_preserved(void)
{
    static const char input[] =
        "GET /index.asp HTTP/1.0\r\n"
        "Host: router\r\n"
        "\r\n";
    char line[64];
    FILE *stream = stream_with_bytes(input, sizeof(input) - 1U);

    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_OK);
    assert(strcmp(line, "GET /index.asp HTTP/1.0\r\n") == 0);
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_OK);
    assert(strcmp(line, "Host: router\r\n") == 0);
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_OK);
    assert(strcmp(line, "\r\n") == 0);
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_EOF);
    assert(fclose(stream) == 0);
}

static void
test_embedded_nul_is_rejected_in_request_and_header_lines(void)
{
    static const unsigned char request_line[] = {
        'G', 'E', 'T', ' ', '/', 'i', 'n', 'd', 'e', 'x', '.', 'a', 's', 'p',
        '\0', ' ', 'H', 'T', 'T', 'P', '/', '1', '.', '0', '\r', '\n'
    };
    static const unsigned char header_line[] = {
        'C', 'o', 'n', 't', 'e', 'n', 't', '-', 'L', 'e', 'n', 'g', 't', 'h',
        ':', ' ', '0', '\0', '9', '\r', '\n'
    };
    char line[64];
    FILE *stream;

    stream = stream_with_bytes(request_line, sizeof(request_line));
    errno = 0;
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EPROTO);
    assert(fclose(stream) == 0);

    stream = stream_with_bytes(header_line, sizeof(header_line));
    errno = 0;
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EPROTO);
    assert(fclose(stream) == 0);
}

static void
test_overlong_and_incomplete_lines_fail_closed(void)
{
    static const char overlong[] = "abcd\n";
    static const char incomplete[] = "partial";
    char small[5];
    char line[16];
    FILE *stream;

    stream = stream_with_bytes(overlong, sizeof(overlong) - 1U);
    errno = 0;
    assert(httpd_request_line_read(stream, small, sizeof(small)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EMSGSIZE);
    assert(fclose(stream) == 0);

    stream = stream_with_bytes(incomplete, sizeof(incomplete) - 1U);
    errno = 0;
    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EPROTO);
    assert(fclose(stream) == 0);
}

static void
test_clean_eof_and_invalid_arguments_are_distinct(void)
{
    char line[4];
    FILE *stream = stream_with_bytes(NULL, 0U);

    assert(httpd_request_line_read(stream, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_EOF);
    assert(fclose(stream) == 0);

    errno = 0;
    assert(httpd_request_line_read(NULL, line, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EINVAL);
    errno = 0;
    assert(httpd_request_line_read(stdout, NULL, sizeof(line)) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EINVAL);
    errno = 0;
    assert(httpd_request_line_read(stdout, line, 1U) ==
           HTTPD_REQUEST_LINE_READ_ERROR);
    assert(errno == EINVAL);
}

int
main(void)
{
    test_complete_lf_and_crlf_lines_are_preserved();
    test_embedded_nul_is_rejected_in_request_and_header_lines();
    test_overlong_and_incomplete_lines_fail_closed();
    test_clean_eof_and_invalid_arguments_are_distinct();
    puts("HTTPD bounded byte-safe request-line parser regression: PASS");
    return 0;
}
