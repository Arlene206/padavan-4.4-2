#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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


static char mock_dhcp_start[32];
static char mock_dhcp_end[32];
static int mock_set_calls;
static int mock_fail_call_a;
static int mock_fail_call_b;

static int
mock_nvram_set(const char *name, const char *value)
{
	mock_set_calls++;
	if (mock_set_calls == mock_fail_call_a ||
	    mock_set_calls == mock_fail_call_b) {
		errno = EACCES;
		return -1;
	}
	if (strcmp(name, "dhcp_start") == 0)
		snprintf(mock_dhcp_start, sizeof(mock_dhcp_start), "%s", value);
	else if (strcmp(name, "dhcp_end") == 0)
		snprintf(mock_dhcp_end, sizeof(mock_dhcp_end), "%s", value);
	else {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

static void
reset_mock_nvram(void)
{
	snprintf(mock_dhcp_start, sizeof(mock_dhcp_start), "%s", "192.168.1.100");
	snprintf(mock_dhcp_end, sizeof(mock_dhcp_end), "%s", "192.168.1.200");
	mock_set_calls = 0;
	mock_fail_call_a = 0;
	mock_fail_call_b = 0;
}

static void
test_nvram_pair_update_success(void)
{
	int rollback_failed = -1;

	reset_mock_nvram();
	assert(dnsmasq_dhcp_range_store_nvram_pair(
		"192.168.1.100", "192.168.1.200",
		"192.168.1.101", "192.168.1.201",
		mock_nvram_set, &rollback_failed) == 0);
	assert(strcmp(mock_dhcp_start, "192.168.1.101") == 0);
	assert(strcmp(mock_dhcp_end, "192.168.1.201") == 0);
	assert(mock_set_calls == 2);
	assert(rollback_failed == 0);
}

static void
test_nvram_pair_second_write_rolls_back(void)
{
	int rollback_failed = -1;

	reset_mock_nvram();
	mock_fail_call_a = 2;
	assert(dnsmasq_dhcp_range_store_nvram_pair(
		"192.168.1.100", "192.168.1.200",
		"192.168.1.101", "192.168.1.201",
		mock_nvram_set, &rollback_failed) == -1);
	assert(strcmp(mock_dhcp_start, "192.168.1.100") == 0);
	assert(strcmp(mock_dhcp_end, "192.168.1.200") == 0);
	assert(mock_set_calls == 4);
	assert(rollback_failed == 0);
	assert(errno == EACCES);
}

static void
test_nvram_pair_first_write_rolls_back(void)
{
	int rollback_failed = -1;

	reset_mock_nvram();
	mock_fail_call_a = 1;
	assert(dnsmasq_dhcp_range_store_nvram_pair(
		"192.168.1.100", "192.168.1.200",
		"192.168.1.101", "192.168.1.201",
		mock_nvram_set, &rollback_failed) == -1);
	assert(strcmp(mock_dhcp_start, "192.168.1.100") == 0);
	assert(strcmp(mock_dhcp_end, "192.168.1.200") == 0);
	assert(mock_set_calls == 3);
	assert(rollback_failed == 0);
	assert(errno == EACCES);
}

static void
test_nvram_pair_reports_incomplete_rollback(void)
{
	int rollback_failed = 0;

	reset_mock_nvram();
	mock_fail_call_a = 2;
	mock_fail_call_b = 4;
	assert(dnsmasq_dhcp_range_store_nvram_pair(
		"192.168.1.100", "192.168.1.200",
		"192.168.1.101", "192.168.1.201",
		mock_nvram_set, &rollback_failed) == -1);
	assert(strcmp(mock_dhcp_start, "192.168.1.101") == 0);
	assert(strcmp(mock_dhcp_end, "192.168.1.200") == 0);
	assert(mock_set_calls == 4);
	assert(rollback_failed == 1);
	assert(errno == EIO);
}

int
main(void)
{
	test_configured_pool_conflicts();
	test_default_pool_excludes_router();
	test_invalid_subnets_fail_closed();
	test_nvram_pair_update_success();
	test_nvram_pair_second_write_rolls_back();
	test_nvram_pair_first_write_rolls_back();
	test_nvram_pair_reports_incomplete_rollback();
	puts("DNSMasq DHCP pool conflict and NVRAM rollback policy: PASS");
	return 0;
}
