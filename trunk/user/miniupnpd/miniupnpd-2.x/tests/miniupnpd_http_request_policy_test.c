#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "miniupnpd_http_request_policy.h"

static void
expect_length_error(const char *value, size_t length, int expected_errno)
{
    size_t parsed = 123U;

    errno = 0;
    assert(miniupnpd_http_parse_content_length(value, length, &parsed) == -1);
    assert(errno == expected_errno);
    assert(parsed == 123U);
}

static void
test_content_length_boundaries(void)
{
    size_t parsed = 999U;
    const char padded[] = " \t65536\t ";

    assert(miniupnpd_http_parse_content_length("0", 1U, &parsed) == 0);
    assert(parsed == 0U);
    assert(miniupnpd_http_parse_content_length("1", 1U, &parsed) == 0);
    assert(parsed == 1U);
    assert(miniupnpd_http_parse_content_length("65536", 5U, &parsed) == 0);
    assert(parsed == MINIUPNPD_HTTP_MAX_BODY_BYTES);
    assert(miniupnpd_http_parse_content_length(padded, sizeof(padded) - 1U,
                                               &parsed) == 0);
    assert(parsed == MINIUPNPD_HTTP_MAX_BODY_BYTES);
    expect_length_error("65537", 5U, ERANGE);
    expect_length_error("999999999999999999999999999999999999999", 39U, ERANGE);
}

static void
test_content_length_rejects_ambiguous_or_malformed_values(void)
{
    expect_length_error(NULL, 0U, EINVAL);
    expect_length_error("", 0U, EINVAL);
    expect_length_error(" \t", 2U, EINVAL);
    expect_length_error("-1", 2U, EINVAL);
    expect_length_error("+1", 2U, EINVAL);
    expect_length_error("1x", 2U, EINVAL);
    expect_length_error("1,1", 3U, EINVAL);
    expect_length_error("1 2", 3U, EINVAL);
    expect_length_error("1.0", 3U, EINVAL);
    expect_length_error("1\t2", 3U, EINVAL);
    expect_length_error(" 1\r", 3U, EINVAL);
}

static void
test_soap_action_quote_boundaries(void)
{
    size_t offset = 99U;
    size_t length = 99U;
    const char quoted[] = "  \"urn:schemas-upnp-org:service:WANIPConnection:1#GetStatus\" \t";
    const char unquoted[] = "urn:schemas-upnp-org:service:WANIPConnection:1#GetStatus";

    assert(miniupnpd_http_parse_soap_action(
        quoted, sizeof(quoted) - 1U, &offset, &length) == 0);
    assert(offset == 3U);
    assert(length == sizeof(unquoted) - 1U);
    assert(memcmp(quoted + offset, unquoted, length) == 0);

    assert(miniupnpd_http_parse_soap_action(
        unquoted, sizeof(unquoted) - 1U, &offset, &length) == 0);
    assert(offset == 0U && length == sizeof(unquoted) - 1U);

    assert(miniupnpd_http_parse_soap_action("\"\"", 2U, &offset, &length) == 0);
    assert(offset == 1U && length == 0U);
    assert(miniupnpd_http_parse_soap_action("", 0U, &offset, &length) == 0);
    assert(offset == 0U && length == 0U);
}

static void
expect_soap_action_error(const char *value, size_t size)
{
    size_t offset = 17U;
    size_t length = 23U;

    errno = 0;
    assert(miniupnpd_http_parse_soap_action(value, size, &offset, &length) == -1);
    assert(errno == EINVAL);
    assert(offset == 17U && length == 23U);
}

static void
test_soap_action_rejects_unbalanced_quotes_and_controls(void)
{
    expect_soap_action_error("\"", 1U);
    expect_soap_action_error("\"x", 2U);
    expect_soap_action_error("x\"", 2U);
    expect_soap_action_error("\"x'", 3U);
    expect_soap_action_error("x'y", 3U);
    expect_soap_action_error("x\"y", 3U);
    expect_soap_action_error("x\ty", 3U);
    expect_soap_action_error("x\177y", 3U);
    expect_soap_action_error(NULL, 0U);
}

static void
test_duplicate_content_length_rejected(void)
{
    int seen = 0;
    size_t parsed = 5U;

    assert(miniupnpd_http_record_content_length("8", 1U, &seen, &parsed) == 0);
    assert(seen == 1 && parsed == 8U);

    errno = 0;
    assert(miniupnpd_http_record_content_length("8", 1U, &seen, &parsed) == -1);
    assert(errno == EINVAL);
    assert(seen == 1 && parsed == 8U);
}

static void
test_invalid_destination_is_rejected(void)
{
    errno = 0;
    assert(miniupnpd_http_parse_content_length("1", 1U, NULL) == -1);
    assert(errno == EINVAL);
}

static void
test_request_buffer_growth_boundaries(void)
{
    size_t largest = (size_t)-1;

    assert(miniupnpd_http_request_append_fits(0U, 0U,
                                             MINIUPNPD_HTTP_MAX_REQUEST_BYTES));
    assert(miniupnpd_http_request_append_fits(
        MINIUPNPD_HTTP_MAX_HEADER_BYTES - 1U, 1U,
        MINIUPNPD_HTTP_MAX_HEADER_BYTES));
    assert(!miniupnpd_http_request_append_fits(
        MINIUPNPD_HTTP_MAX_HEADER_BYTES, 1U,
        MINIUPNPD_HTTP_MAX_HEADER_BYTES));
    assert(miniupnpd_http_request_append_fits(
        MINIUPNPD_HTTP_MAX_REQUEST_BYTES - 2048U, 2048U,
        MINIUPNPD_HTTP_MAX_REQUEST_BYTES));
    assert(!miniupnpd_http_request_append_fits(
        MINIUPNPD_HTTP_MAX_REQUEST_BYTES - 2047U, 2048U,
        MINIUPNPD_HTTP_MAX_REQUEST_BYTES));
    assert(!miniupnpd_http_request_append_fits(largest, 1U, largest));
    assert(!miniupnpd_http_request_append_fits(1U, largest, largest));
}

int
main(void)
{
    assert(MINIUPNPD_HTTP_MAX_HEADER_BYTES == 8192U);
    assert(MINIUPNPD_HTTP_MAX_BODY_BYTES == 65536U);
    test_content_length_boundaries();
    test_content_length_rejects_ambiguous_or_malformed_values();
    test_soap_action_quote_boundaries();
    test_soap_action_rejects_unbalanced_quotes_and_controls();
    test_duplicate_content_length_rejected();
    test_invalid_destination_is_rejected();
    test_request_buffer_growth_boundaries();
    puts("MiniUPnP bounded HTTP request framing policy: PASS");
    return 0;
}
