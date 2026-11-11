#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "dnsmasq_dhcp_range_policy.h"

static uint32_t
ip4(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	return ((uint32_t)a << 24) | ((uint32_t)b << 16) |
	       ((uint32_t)c << 8) | (uint32_t)d;
}

static void
test_configured_pool_conflicts(void)
{
	uint32_t lan = ip4(192, 168, 1, 1);
	uint32_t mask = ip4(255, 255, 255, 0);

	assert(dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 100), ip4(192, 168, 1, 200)));
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 1), ip4(192, 168, 1, 200)));
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 0), ip4(192, 168, 1, 100)));
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 100), ip4(192, 168, 1, 255)));
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 200), ip4(192, 168, 1, 100)));
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 2, 100), ip4(192, 168, 2, 200)));
	lan = ip4(192, 168, 1, 127);
	assert(!dnsmasq_dhcp_range_pool_is_valid(
		lan, mask, ip4(192, 168, 1, 100), ip4(192, 168, 1, 150)));
}

static void
test_default_pool_excludes_router(void)
{
	uint32_t start, end;
	uint32_t mask = ip4(255, 255, 255, 0);

	assert(dnsmasq_dhcp_range_default_pool(ip4(192,168,1,1), mask, &start, &end));
	assert(start == ip4(192,168,1,2) && end == ip4(192,168,1,254));
	assert(dnsmasq_dhcp_range_default_pool(ip4(192,168,1,254), mask, &start, &end));
	assert(start == ip4(192,168,1,1) && end == ip4(192,168,1,253));
	assert(dnsmasq_dhcp_range_default_pool(ip4(192,168,1,127), mask, &start, &end));
	assert(start == ip4(192,168,1,128) && end == ip4(192,168,1,254));
	assert(dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,1), ip4(255,255,255,252), &start, &end));
	assert(start == ip4(10,0,0,2) && end == ip4(10,0,0,2));
	assert(dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,2), ip4(255,255,255,252), &start, &end));
	assert(start == ip4(10,0,0,1) && end == ip4(10,0,0,1));
}

static void
test_invalid_subnets_fail_closed(void)
{
	uint32_t start, end;

	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,1), ip4(255,255,255,254), &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,1), ip4(255,255,255,255), &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,1), ip4(255,0,255,0), &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(ip4(10,0,0,1), 0U, &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,0), ip4(255,255,255,0), &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,255), ip4(255,255,255,0), &start, &end));
	assert(!dnsmasq_dhcp_range_default_pool(
		ip4(10,0,0,1), ip4(255,255,255,0), NULL, &end));
}

int
main(void)
{
	test_configured_pool_conflicts();
	test_default_pool_excludes_router();
	test_invalid_subnets_fail_closed();
	puts("DNSMasq DHCP pool conflict policy: PASS");
	return 0;
}
