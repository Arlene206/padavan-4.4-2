#include <arpa/inet.h>
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
test_nonzero_port_validation(void)
{
	unsigned short port = 321;
	int status;

	assert(miniupnpd_soap_parse_nonzero_port("1", &port) == 0 && port == 1);
	assert(miniupnpd_soap_parse_nonzero_port("65535", &port) == 0 && port == 65535);

	errno = 0;
	status = miniupnpd_soap_parse_nonzero_port("0", &port);
	assert(status == -1 && errno == EINVAL && port == 65535);
	errno = 0;
	status = miniupnpd_soap_parse_nonzero_port(NULL, &port);
	assert(status == -1 && errno == EINVAL && port == 65535);
	errno = 0;
	status = miniupnpd_soap_parse_nonzero_port("65536", &port);
	assert(status == -1 && errno == ERANGE && port == 65535);
}

static void
test_lease_duration_normalization(void)
{
	assert(miniupnpd_soap_normalize_lease_duration(0U) ==
	       MINIUPNPD_SOAP_MAX_LEASE_DURATION);
	assert(miniupnpd_soap_normalize_lease_duration(1U) == 1U);
	assert(miniupnpd_soap_normalize_lease_duration(
		MINIUPNPD_SOAP_MAX_LEASE_DURATION) ==
	       MINIUPNPD_SOAP_MAX_LEASE_DURATION);
	assert(miniupnpd_soap_normalize_lease_duration(
		MINIUPNPD_SOAP_MAX_LEASE_DURATION + 1U) ==
	       MINIUPNPD_SOAP_MAX_LEASE_DURATION);
	assert(miniupnpd_soap_normalize_lease_duration(UINT_MAX) ==
	       MINIUPNPD_SOAP_MAX_LEASE_DURATION);
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

static void
test_protocol_number_boundaries(void)
{
	int protocol = -1;

	assert(miniupnpd_soap_parse_protocol_number("6", &protocol) == 0 && protocol == 6);
	assert(miniupnpd_soap_parse_protocol_number("17", &protocol) == 0 && protocol == 17);
	assert(miniupnpd_soap_parse_protocol_number("65535", &protocol) == 0 && protocol == 65535);
	errno = 0;
	assert(miniupnpd_soap_parse_protocol_number("65536", &protocol) == -1 && errno == ERANGE);
	errno = 0;
	assert(miniupnpd_soap_parse_protocol_number("6junk", &protocol) == -1 && errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_parse_protocol_number("-1", &protocol) == -1 && errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_parse_protocol_number(NULL, &protocol) == -1 && errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_parse_protocol_number("6", NULL) == -1 && errno == EINVAL);
}

static void
test_ipv6_peer_identity_comparison(void)
{
	struct in6_addr sender, equivalent, different;

	assert(inet_pton(AF_INET6, "2001:db8::1", &sender) == 1);
	assert(inet_pton(AF_INET6, "2001:0db8:0:0:0:0:0:1", &equivalent) == 1);
	assert(inet_pton(AF_INET6, "2001:db8::2", &different) == 1);
	assert(miniupnpd_soap_ipv6_peer_matches(&sender, &equivalent));
	assert(!miniupnpd_soap_ipv6_peer_matches(&sender, &different));
	assert(!miniupnpd_soap_ipv6_peer_matches(NULL, &sender));
	assert(!miniupnpd_soap_ipv6_peer_matches(&sender, NULL));
}

static void
test_required_text_presence(void)
{
	assert(!miniupnpd_soap_required_text_present(NULL));
	assert(miniupnpd_soap_required_text_present(""));
	assert(miniupnpd_soap_required_text_present("port mapping"));
}


static void
test_delete_range_result_status(void)
{
	struct miniupnpd_soap_delete_range_status status;

	miniupnpd_soap_delete_range_status_init(&status);
	assert(status.attempted == 0 && status.failed == 0);
	assert(miniupnpd_soap_delete_range_error(&status) == 730);

	miniupnpd_soap_delete_range_record(&status, 0);
	miniupnpd_soap_delete_range_record(&status, 0);
	assert(status.attempted == 2 && status.failed == 0);
	assert(miniupnpd_soap_delete_range_error(&status) == 0);

	miniupnpd_soap_delete_range_record(&status, -1);
	assert(status.attempted == 3 && status.failed == 1);
	assert(miniupnpd_soap_delete_range_error(&status) == 501);

	miniupnpd_soap_delete_range_status_init(&status);
	miniupnpd_soap_delete_range_record(&status, -1);
	miniupnpd_soap_delete_range_record(&status, -EIO);
	assert(status.attempted == 2 && status.failed == 2);
	assert(miniupnpd_soap_delete_range_error(&status) == 501);
	assert(miniupnpd_soap_delete_range_error(NULL) == 501);
}

int
main(void)
{
	test_port_boundaries();
	test_nonzero_port_validation();
	test_lease_duration_normalization();
	test_protocol_tokens();
	test_nonnegative_int_boundaries();
	test_protocol_number_boundaries();
	test_ipv6_peer_identity_comparison();
	test_required_text_presence();
	test_delete_range_result_status();
	test_invalid_output_arguments();
	puts("MiniUPnP SOAP unsigned input and range-delete result policy: PASS");
	return 0;
}
