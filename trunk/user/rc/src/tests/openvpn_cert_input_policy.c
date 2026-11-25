#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "openvpn_cert_input_policy.h"

static void
assert_invalid(const char *value, int expected_status)
{
    int days = 1234;

    assert(openvpn_cert_days_valid_parse(value, 365, &days) == expected_status);
    assert(days == 1234);
}

static void
test_defaults_and_compatible_whitespace(void)
{
    int days = 0;

    assert(openvpn_cert_days_valid_parse(NULL, 365, &days) == 0);
    assert(days == 365);
    assert(openvpn_cert_days_valid_parse("", 365, &days) == 0);
    assert(days == 365);
    assert(openvpn_cert_days_valid_parse("  \t ", 90, &days) == 0);
    assert(days == 90);
}

static void
test_valid_lifetimes(void)
{
    int days = 0;

    assert(openvpn_cert_days_valid_parse("1", 365, &days) == 0);
    assert(days == OPENVPN_CERT_DAYS_MIN);
    assert(openvpn_cert_days_valid_parse("365", 365, &days) == 0);
    assert(days == 365);
    assert(openvpn_cert_days_valid_parse("  +30  ", 365, &days) == 0);
    assert(days == 30);
    assert(openvpn_cert_days_valid_parse("36500", 365, &days) == 0);
    assert(days == OPENVPN_CERT_DAYS_MAX);
}

static void
test_malformed_and_out_of_range_lifetimes(void)
{
    assert_invalid("0", ERANGE);
    assert_invalid("-1", EINVAL);
    assert_invalid("36501", ERANGE);
    assert_invalid("30junk", EINVAL);
    assert_invalid("30.0", EINVAL);
    assert_invalid("1e2", EINVAL);
    assert_invalid("+", EINVAL);
    assert_invalid("  - 1", EINVAL);
    assert_invalid("999999999999999999999999999999", ERANGE);
}

static void
test_invalid_api_arguments(void)
{
    int days = 77;

    assert(openvpn_cert_days_valid_parse("30", 365, NULL) == EINVAL);
    assert(openvpn_cert_days_valid_parse("30", 0, &days) == EINVAL);
    assert(openvpn_cert_days_valid_parse("30", 36501, &days) == EINVAL);
    assert(days == 77);
}

int
main(void)
{
    test_defaults_and_compatible_whitespace();
    test_valid_lifetimes();
    test_malformed_and_out_of_range_lifetimes();
    test_invalid_api_arguments();

    puts("OpenVPN certificate lifetime argument policy: PASS");
    return 0;
}
