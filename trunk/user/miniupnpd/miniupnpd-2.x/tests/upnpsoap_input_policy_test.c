#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "upnpsoap_input_policy.h"

static void
expect_port_error(const char *text, int expected_errno)
{
	unsigned short port = 1234;

	errno = 0;
	assert(miniupnpd_soap_parse_port(text, &port) == -1);
	assert(errno == expected_errno);
	assert(port == 1234);
}

static void
expect_int_error(const char *text, int expected_errno)
{
	int value = 1234;

	errno = 0;
	assert(miniupnpd_soap_parse_int(text, &value) == -1);
	assert(errno == expected_errno);
	assert(value == 1234);
}

static void
test_port_boundaries(void)
{
	unsigned short port = 7;

	assert(miniupnpd_soap_parse_port("0", &port) == 0 && port == 0);
	assert(miniupnpd_soap_parse_port("1", &port) == 0 && port == 1);
	assert(miniupnpd_soap_parse_port("65535", &port) == 0 && port == 65535);
	expect_port_error("65536", ERANGE);
	expect_port_error("999999999999999999999999", ERANGE);
	expect_port_error(NULL, EINVAL);
	expect_port_error("", EINVAL);
	expect_port_error("-1", EINVAL);
	expect_port_error("+1", EINVAL);
	expect_port_error("12x", EINVAL);
	expect_port_error(" 12", EINVAL);
	expect_port_error("12 ", EINVAL);
}

static void
test_protocol_tokens(void)
{
	assert(miniupnpd_soap_protocol_valid("TCP"));
	assert(miniupnpd_soap_protocol_valid("tcp"));
	assert(miniupnpd_soap_protocol_valid("uDp"));
	assert(miniupnpd_soap_protocol_valid("UDP"));
#ifdef IPPROTO_UDPLITE
	assert(miniupnpd_soap_protocol_valid("UDPLITE"));
	assert(miniupnpd_soap_protocol_valid("udplite"));
#else
	assert(!miniupnpd_soap_protocol_valid("UDPLITE"));
#endif
	assert(!miniupnpd_soap_protocol_valid(NULL));
	assert(!miniupnpd_soap_protocol_valid(""));
	assert(!miniupnpd_soap_protocol_valid("TCPx"));
	assert(!miniupnpd_soap_protocol_valid("TCP UDP"));
	assert(!miniupnpd_soap_protocol_valid("<p>TCP</p>"));
}

static void
test_nonnegative_int_boundaries(void)
{
	char maximum[64];
	char overflow[64];
	int value = -1;

	assert(snprintf(maximum, sizeof(maximum), "%d", INT_MAX) > 0);
	assert(snprintf(overflow, sizeof(overflow), "%lu",
	                (unsigned long)INT_MAX + 1UL) > 0);
	assert(miniupnpd_soap_parse_int("0", &value) == 0 && value == 0);
	assert(miniupnpd_soap_parse_int("1", &value) == 0 && value == 1);
	assert(miniupnpd_soap_parse_int(maximum, &value) == 0 && value == INT_MAX);
	expect_int_error(overflow, ERANGE);
	expect_int_error("999999999999999999999999999999999999", ERANGE);
	expect_int_error(NULL, EINVAL);
	expect_int_error("", EINVAL);
	expect_int_error("-1", EINVAL);
	expect_int_error("+1", EINVAL);
	expect_int_error("1abc", EINVAL);
}

static void
test_invalid_output_arguments(void)
{
	errno = 0;
	assert(miniupnpd_soap_parse_port("1", NULL) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_parse_int("1", NULL) == -1);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_port_boundaries();
	test_protocol_tokens();
	test_nonnegative_int_boundaries();
	test_invalid_output_arguments();
	puts("MiniUPnP SOAP unsigned input policy: PASS");
	return 0;
}
