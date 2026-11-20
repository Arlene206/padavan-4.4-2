#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "miniupnpd_pinhole_policy.h"

struct uid_fixture {
	unsigned short blocked[8];
	size_t blocked_count;
	int block_all;
};

static int
uid_is_used(unsigned short uid, void *context)
{
	struct uid_fixture *fixture = (struct uid_fixture *)context;
	size_t i;

	if (fixture->block_all)
		return 1;
	for (i = 0; i < fixture->blocked_count; ++i) {
		if (fixture->blocked[i] == uid)
			return 1;
	}
	return 0;
}

static void
test_uid_selection(void)
{
	struct uid_fixture fixture;
	unsigned short selected = 0;

	memset(&fixture, 0, sizeof(fixture));
	fixture.blocked[0] = 7;
	fixture.blocked[1] = 8;
	fixture.blocked[2] = 9;
	fixture.blocked_count = 3;
	assert(miniupnpd_pinhole_find_free_uid(7, uid_is_used, &fixture,
	                                      &selected) == 0);
	assert(selected == 10);
}

static void
test_uid_wraparound(void)
{
	struct uid_fixture fixture;
	unsigned short selected = 0;

	memset(&fixture, 0, sizeof(fixture));
	fixture.blocked[0] = 65535;
	fixture.blocked[1] = 1;
	fixture.blocked_count = 2;
	assert(miniupnpd_pinhole_find_free_uid(65535, uid_is_used, &fixture,
	                                      &selected) == 0);
	assert(selected == 2);
}

static void
test_uid_space_exhausted(void)
{
	struct uid_fixture fixture;
	unsigned short selected = 4321;

	memset(&fixture, 0, sizeof(fixture));
	fixture.block_all = 1;
	errno = 0;
	assert(miniupnpd_pinhole_find_free_uid(1, uid_is_used, &fixture,
	                                      &selected) == -1);
	assert(errno == ENOSPC);
	assert(selected == 4321);
	errno = 0;
	assert(miniupnpd_pinhole_find_free_uid(1, NULL, &fixture,
	                                      &selected) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_pinhole_find_free_uid(1, uid_is_used, &fixture,
	                                      NULL) == -1);
	assert(errno == EINVAL);
}

static void
assert_zero_address(const struct in6_addr *address)
{
	static const struct in6_addr zero = IN6ADDR_ANY_INIT;
	assert(memcmp(address, &zero, sizeof(zero)) == 0);
}

static void
test_wildcard_remote_hosts(void)
{
	struct in6_addr address;
	int wildcard = 0;

	memset(&address, 0xff, sizeof(address));
	assert(miniupnpd_pinhole_parse_remote_ipv6(NULL, &address,
	                                          &wildcard) == 0);
	assert(wildcard == 1);
	assert_zero_address(&address);
	memset(&address, 0xff, sizeof(address));
	wildcard = 0;
	assert(miniupnpd_pinhole_parse_remote_ipv6("", &address,
	                                          &wildcard) == 0);
	assert(wildcard == 1);
	assert_zero_address(&address);
	memset(&address, 0xff, sizeof(address));
	wildcard = 0;
	assert(miniupnpd_pinhole_parse_remote_ipv6("*", &address,
	                                          &wildcard) == 0);
	assert(wildcard == 1);
	assert_zero_address(&address);
}

static void
test_specific_remote_host(void)
{
	struct in6_addr address;
	struct in6_addr expected;
	int wildcard = 1;

	assert(inet_pton(AF_INET6, "2001:db8::42", &expected) == 1);
	assert(miniupnpd_pinhole_parse_remote_ipv6("2001:db8::42", &address,
	                                          &wildcard) == 0);
	assert(wildcard == 0);
	assert(memcmp(&address, &expected, sizeof(address)) == 0);
}

static void
test_invalid_remote_host(void)
{
	struct in6_addr address;
	struct in6_addr original;
	int wildcard = 7;

	memset(&address, 0x5a, sizeof(address));
	original = address;
	errno = 0;
	assert(miniupnpd_pinhole_parse_remote_ipv6("not-an-ip", &address,
	                                          &wildcard) == -1);
	assert(errno == EINVAL);
	assert(memcmp(&address, &original, sizeof(address)) == 0);
	assert(wildcard == 7);
	errno = 0;
	assert(miniupnpd_pinhole_parse_remote_ipv6("192.0.2.1", &address,
	                                          &wildcard) == -1);
	assert(errno == EINVAL);
}

static void
test_internal_client_validation(void)
{
	struct in6_addr address;
	struct in6_addr original;

	assert(miniupnpd_pinhole_parse_internal_ipv6("2001:db8::1",
	                                             &address) == 0);
	memset(&address, 0xa5, sizeof(address));
	original = address;
	errno = 0;
	assert(miniupnpd_pinhole_parse_internal_ipv6("bad-address",
	                                             &address) == -1);
	assert(errno == EINVAL);
	assert(memcmp(&address, &original, sizeof(address)) == 0);
	assert(miniupnpd_pinhole_parse_internal_ipv6(NULL, &address) == -1);
	assert(miniupnpd_pinhole_parse_internal_ipv6("2001:db8::1", NULL) == -1);
}

int
main(void)
{
	test_uid_selection();
	test_uid_wraparound();
	test_uid_space_exhausted();
	test_wildcard_remote_hosts();
	test_specific_remote_host();
	test_invalid_remote_host();
	test_internal_client_validation();
	puts("MiniUPnP IPv6 pinhole policy: PASS");
	return 0;
}
