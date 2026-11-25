#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "wan_ipv6_service_policy.h"

enum {
	TEST_IPV6_DISABLED = 11,
	TEST_IPV6_NATIVE_STATIC = 20,
	TEST_IPV6_NATIVE_DHCP6 = 31,
	TEST_IPV6_6IN4 = 42,
	TEST_IPV6_6TO4 = 53,
	TEST_IPV6_6RD = 64
};

static int
is_sit(int type, int *result)
{
	return wan_ipv6_service_is_sit_status(
		0, type, TEST_IPV6_DISABLED, TEST_IPV6_6IN4,
		TEST_IPV6_6TO4, TEST_IPV6_6RD, result);
}

static int
is_addr_static(int type, int dhcp_read_status, int dhcp_manual, int *result)
{
	return wan_ipv6_service_addr_static_status(
		0, type, TEST_IPV6_DISABLED, TEST_IPV6_NATIVE_STATIC,
		TEST_IPV6_NATIVE_DHCP6, TEST_IPV6_6IN4, TEST_IPV6_6TO4,
		TEST_IPV6_6RD, dhcp_read_status, dhcp_manual, result);
}

static int
is_dns_static(int type, int auto_read_status, int auto_manual, int *result)
{
	return wan_ipv6_service_dns_static_status(
		0, type, TEST_IPV6_DISABLED, TEST_IPV6_NATIVE_STATIC,
		TEST_IPV6_NATIVE_DHCP6, TEST_IPV6_6IN4, TEST_IPV6_6TO4,
		TEST_IPV6_6RD, auto_read_status, auto_manual, result);
}

static void
test_service_read_error_is_not_mapped_to_disabled(void)
{
	int result = 17;

	assert(wan_ipv6_service_is_sit_status(
		EIO, TEST_IPV6_DISABLED, TEST_IPV6_DISABLED, TEST_IPV6_6IN4,
		TEST_IPV6_6TO4, TEST_IPV6_6RD, &result) == EIO);
	assert(result == -1);
	assert(wan_ipv6_service_addr_static_status(
		EACCES, TEST_IPV6_DISABLED, TEST_IPV6_DISABLED,
		TEST_IPV6_NATIVE_STATIC, TEST_IPV6_NATIVE_DHCP6,
		TEST_IPV6_6IN4, TEST_IPV6_6TO4, TEST_IPV6_6RD,
		0, 0, &result) == EACCES);
	assert(result == -1);
	assert(wan_ipv6_service_dns_static_status(
		EIO, TEST_IPV6_DISABLED, TEST_IPV6_DISABLED,
		TEST_IPV6_NATIVE_STATIC, TEST_IPV6_NATIVE_DHCP6,
		TEST_IPV6_6IN4, TEST_IPV6_6TO4, TEST_IPV6_6RD,
		0, 0, &result) == EIO);
	assert(result == -1);
}

static void
test_sit_classification_preserves_all_transport_modes(void)
{
	int result = 99;

	assert(is_sit(TEST_IPV6_DISABLED, &result) == 0 && result == -1);
	assert(is_sit(TEST_IPV6_NATIVE_STATIC, &result) == 0 && result == 0);
	assert(is_sit(TEST_IPV6_NATIVE_DHCP6, &result) == 0 && result == 0);
	assert(is_sit(TEST_IPV6_6IN4, &result) == 0 && result == 1);
	assert(is_sit(TEST_IPV6_6TO4, &result) == 0 && result == 1);
	assert(is_sit(TEST_IPV6_6RD, &result) == 0 && result == 1);
}

static void
test_address_static_classification_preserves_legacy_rules(void)
{
	int result = 99;

	assert(is_addr_static(TEST_IPV6_DISABLED, 0, 0, &result) == 0 && result == -1);
	assert(is_addr_static(TEST_IPV6_NATIVE_STATIC, 0, 0, &result) == 0 && result == 1);
	assert(is_addr_static(TEST_IPV6_NATIVE_DHCP6, 0, 0, &result) == 0 && result == 0);
	assert(is_addr_static(TEST_IPV6_6IN4, 0, 0, &result) == 0 && result == 1);
	assert(is_addr_static(TEST_IPV6_6TO4, 0, 0, &result) == 0 && result == 0);
	assert(is_addr_static(TEST_IPV6_6RD, 0, 1, &result) == 0 && result == 1);
	assert(is_addr_static(TEST_IPV6_6RD, 0, 0, &result) == 0 && result == 0);
	assert(is_addr_static(TEST_IPV6_6RD, EACCES, 0, &result) == EACCES);
	assert(result == -1);
	assert(is_addr_static(TEST_IPV6_NATIVE_STATIC, EACCES, 0, &result) == 0);
	assert(result == 1);
}

static void
test_dns_static_classification_checks_only_manual_native_dhcp_mode(void)
{
	int result = 99;

	assert(is_dns_static(TEST_IPV6_DISABLED, 0, 0, &result) == 0 && result == -1);
	assert(is_dns_static(TEST_IPV6_NATIVE_STATIC, EIO, 0, &result) == 0 && result == 1);
	assert(is_dns_static(TEST_IPV6_6IN4, EIO, 0, &result) == 0 && result == 1);
	assert(is_dns_static(TEST_IPV6_6TO4, EIO, 0, &result) == 0 && result == 1);
	assert(is_dns_static(TEST_IPV6_6RD, EIO, 0, &result) == 0 && result == 1);
	assert(is_dns_static(TEST_IPV6_NATIVE_DHCP6, 0, 1, &result) == 0 && result == 1);
	assert(is_dns_static(TEST_IPV6_NATIVE_DHCP6, 0, 0, &result) == 0 && result == 0);
	assert(is_dns_static(TEST_IPV6_NATIVE_DHCP6, EIO, 0, &result) == EIO);
	assert(result == -1);
}

static void
test_null_outputs_fail_closed(void)
{
	assert(is_sit(TEST_IPV6_DISABLED, NULL) == EINVAL);
	assert(is_addr_static(TEST_IPV6_DISABLED, 0, 0, NULL) == EINVAL);
	assert(is_dns_static(TEST_IPV6_DISABLED, 0, 0, NULL) == EINVAL);
}

int
main(void)
{
	test_service_read_error_is_not_mapped_to_disabled();
	test_sit_classification_preserves_all_transport_modes();
	test_address_static_classification_preserves_legacy_rules();
	test_dns_static_classification_checks_only_manual_native_dhcp_mode();
	test_null_outputs_fail_closed();
	puts("WAN IPv6 service checked-read policy: PASS");
	return 0;
}
