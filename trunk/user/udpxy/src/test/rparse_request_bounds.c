/* GNU89 regression tests for UDPXY's length-delimited request parser. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rparse.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return __LINE__; \
    } \
} while (0)

static int
test_unterminated_request_buffer(void)
{
    char no_header[32];
    char no_space[32];
    char valid[] = {
        'G', 'E', 'T', ' ', '/', 'u', 'd', 'p', '/',
        '2', '3', '9', '.', '1', '.', '2', '.', '3', ':',
        '5', '0', '0', '0', ' ', 'H', 'T', 'T', 'P'
    };
    char request[64];
    size_t request_len;

    memset(no_header, 'X', sizeof(no_header));
    request_len = sizeof(request);
    CHECK(get_request(no_header, sizeof(no_header), request, &request_len) == 1);

    memset(no_space, 'A', sizeof(no_space));
    memcpy(no_space, "GET /", 5U);
    request_len = sizeof(request);
    CHECK(get_request(no_space, sizeof(no_space), request, &request_len) == 3);

    request_len = sizeof(request);
    CHECK(get_request(valid, sizeof(valid), request, &request_len) == 0);
    CHECK(strcmp(request, "udp/239.1.2.3:5000") == 0);

    request_len = strlen("udp/239.1.2.3:5000");
    CHECK(get_request(valid, sizeof(valid), request, &request_len) == 3);

    request_len = sizeof(request);
    CHECK(get_request("GET /", 5U, request, &request_len) == 2);

    {
        const char embedded_nul[] = {
            'G', 'E', 'T', ' ', '/', 'u', 'd', 'p', '\0',
            '/', '2', '3', '9', ' ', 'H'
        };
        request_len = sizeof(request);
        CHECK(get_request(embedded_nul, sizeof(embedded_nul), request, &request_len) == 3);
    }

    return 0;
}

static int
test_length_delimited_parameters(void)
{
    const char plain[] = {
        '/', 'u', 'd', 'p', '/', '2', '3', '9', ':', '5', '0', '0', '0'
    };
    const char with_query[] = {
        '/', 'u', 'd', 'p', '/', '2', '3', '9', ':', '5', '0', '0', '0',
        '?', 'x', '=', '1'
    };
    const char embedded_nul[] = {
        '/', 'u', 'd', 'p', '\0', '/', '2', '3', '9'
    };
    char cmd[8], opt[32], tail[8];

    CHECK(parse_param(plain, sizeof(plain), cmd, sizeof(cmd),
                      opt, sizeof(opt), tail, sizeof(tail)) == 0);
    CHECK(strcmp(cmd, "udp") == 0);
    CHECK(strcmp(opt, "239:5000") == 0);
    CHECK(tail[0] == '\0');

    CHECK(parse_param(with_query, sizeof(with_query), cmd, sizeof(cmd),
                      opt, sizeof(opt), tail, sizeof(tail)) == 0);
    CHECK(strcmp(cmd, "udp") == 0);
    CHECK(strcmp(opt, "239:5000") == 0);
    CHECK(strcmp(tail, "?x=1") == 0);

    CHECK(parse_param(embedded_nul, sizeof(embedded_nul), cmd, sizeof(cmd),
                      opt, sizeof(opt), tail, sizeof(tail)) == 0);
    CHECK(strcmp(cmd, "udp") == 0);
    CHECK(opt[0] == '\0');

    CHECK(parse_param(plain, sizeof(plain), cmd, 3U,
                      opt, sizeof(opt), tail, sizeof(tail)) == EOVERFLOW);
    CHECK(parse_param(plain, sizeof(plain), cmd, sizeof(cmd),
                      opt, 8U, tail, sizeof(tail)) == EOVERFLOW);
    CHECK(parse_param(plain, sizeof(plain), cmd, sizeof(cmd),
                      opt, sizeof(opt), tail, 0U) == 0);

    return 0;
}

static int
test_source_address_bounds_and_port(void)
{
    char source[16], group[16];
    char long_ssm[128];
    const char ordinary[] = "239.1.2.3:5000";
    const char ssm[] = "192.0.2.1@239.1.2.3:5000";
    const char invalid_port[] = "239.1.2.3:5000junk";
    char unterminated[] = {
        '2', '3', '9', '.', '1', '.', '2', '.', '3', ':', '5', '0', '0', '0'
    };
    uint16_t port = 0U;
    size_t source_len = 100U;

    memset(source, 'S', sizeof(source));
    memset(group, 'G', sizeof(group));
    CHECK(parse_udprelay(ordinary, sizeof(ordinary), source, sizeof(source),
                         group, sizeof(group), &port) == 0);
    CHECK(source[0] == '\0');
    CHECK(strcmp(group, "239.1.2.3") == 0);
    CHECK(port == 5000U);

    CHECK(parse_udprelay(ssm, sizeof(ssm), source, sizeof(source),
                         group, sizeof(group), &port) == 0);
    CHECK(strcmp(source, "192.0.2.1") == 0);
    CHECK(strcmp(group, "239.1.2.3") == 0);
    CHECK(port == 5000U);

    memset(long_ssm, '1', sizeof(long_ssm));
    long_ssm[source_len] = '@';
    memcpy(long_ssm + source_len + 1U, "239.1.2.3:5000",
           sizeof("239.1.2.3:5000"));
    CHECK(parse_udprelay(long_ssm, source_len + 1U +
                         sizeof("239.1.2.3:5000"), source, sizeof(source),
                         group, sizeof(group), &port) == 3);
    CHECK(source[0] == '\0' && group[0] == '\0');

    port = 4321U;
    CHECK(parse_udprelay(invalid_port, sizeof(invalid_port),
                         source, sizeof(source), group, sizeof(group),
                         &port) == 3);
    CHECK(port == 4321U);
    CHECK(parse_udprelay(unterminated, sizeof(unterminated),
                         source, sizeof(source), group, sizeof(group),
                         &port) == 1);

    CHECK(parse_udprelay(ordinary, sizeof(ordinary), source, sizeof(source),
                         group, 4U, &port) == 3);
    CHECK(source[0] == '\0' && group[0] == '\0');

    return 0;
}

int
main(void)
{
    int rc;

    rc = test_unterminated_request_buffer();
    if (rc != 0)
        return rc;
    rc = test_length_delimited_parameters();
    if (rc != 0)
        return rc;
    rc = test_source_address_bounds_and_port();
    if (rc != 0)
        return rc;

    puts("UDPXY request parser bounds regression: PASS");
    return 0;
}
