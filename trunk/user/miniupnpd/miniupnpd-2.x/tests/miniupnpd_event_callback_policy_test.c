#include <assert.h>
#include <string.h>
#include <sys/socket.h>

#include "miniupnpd_event_callback_policy.h"

static void
expect_invalid(const char *url)
{
	struct miniupnpd_event_callback parsed;

	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) != 0);
}

static void
test_ipv4_default_and_explicit_ports(void)
{
	struct miniupnpd_event_callback parsed;
	const char *url = "http://192.0.2.10/event";

	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) == 0);
	assert(parsed.family == AF_INET);
	assert(strcmp(parsed.host, "192.0.2.10") == 0);
	assert(parsed.port == 80);
	assert(strcmp(url + parsed.path_offset, "/event") == 0);

	url = "http://192.0.2.10:49152/notify?seq=1";
	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) == 0);
	assert(parsed.family == AF_INET);
	assert(parsed.port == 49152);
	assert(strcmp(url + parsed.path_offset, "/notify?seq=1") == 0);
}

static void
test_ipv6_bracket_and_port_boundaries(void)
{
	struct miniupnpd_event_callback parsed;
	const char *url = "http://[2001:db8::10]/event";

	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) == 0);
	assert(parsed.family == AF_INET6);
	assert(strcmp(parsed.host, "2001:db8::10") == 0);
	assert(parsed.port == 80);

	url = "http://[2001:db8::10]:65535/event";
	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) == 0);
	assert(parsed.port == 65535);
}

static void
test_missing_path_and_invalid_ports_are_rejected(void)
{
	expect_invalid("http://192.0.2.10");
	expect_invalid("http://192.0.2.10:80");
	expect_invalid("http://[2001:db8::10]:80");
	expect_invalid("http://192.0.2.10:");
	expect_invalid("http://192.0.2.10:abc/event");
	expect_invalid("http://192.0.2.10:0/event");
	expect_invalid("http://192.0.2.10:65536/event");
	expect_invalid("http://192.0.2.10:80x/event");
}

static void
test_bad_scheme_authority_and_path_bytes_are_rejected(void)
{
	expect_invalid("https://192.0.2.10/event");
	expect_invalid("http://example.test/event");
	expect_invalid("http://[2001:db8::10/event");
	expect_invalid("http://192.0.2.10] /event");
	expect_invalid("http://192.0.2.10/event path");
	expect_invalid("http://192.0.2.10/event#fragment");
	expect_invalid("http://192.0.2.10/event%2");
	expect_invalid("http://192.0.2.10/event\r\nInjected: yes");
}

static void
test_minimal_absolute_path_is_supported(void)
{
	struct miniupnpd_event_callback parsed;
	const char *url = "http://192.0.2.10/";

	assert(miniupnpd_event_parse_callback_url(url, strlen(url), &parsed) == 0);
	assert(url[parsed.path_offset] == '/');
	assert(url[parsed.path_offset + 1] == '\0');
}

static void
test_notification_length_boundaries(void)
{
	assert(miniupnpd_event_buffer_length_fits(0, 1));
	assert(miniupnpd_event_buffer_length_fits(1023, 1024));
	assert(!miniupnpd_event_buffer_length_fits(1024, 1024));
	assert(!miniupnpd_event_buffer_length_fits(-1, 1024));
	assert(!miniupnpd_event_buffer_length_fits(0, 0));
}

int
main(void)
{
	test_ipv4_default_and_explicit_ports();
	test_ipv6_bracket_and_port_boundaries();
	test_missing_path_and_invalid_ports_are_rejected();
	test_bad_scheme_authority_and_path_bytes_are_rejected();
	test_minimal_absolute_path_is_supported();
	test_notification_length_boundaries();
	return 0;
}
