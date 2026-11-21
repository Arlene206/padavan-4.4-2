#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "vpn_server_config_policy.h"

static void
test_enable_missing_key_is_disabled(void)
{
	int enabled = 1;

	assert(vpn_server_config_enable_status(0, 0, NULL, &enabled) == 0);
	assert(enabled == 0);
}

static void
test_enable_requires_exact_one(void)
{
	int enabled = 0;

	assert(vpn_server_config_enable_status(1, 0, "1", &enabled) == 0);
	assert(enabled == 1);
	assert(vpn_server_config_enable_status(1, 0, "0", &enabled) == 0);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(1, 0, "01", &enabled) == 0);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(1, 0, " 1", &enabled) == 0);
	assert(enabled == 0);
}

static void
test_enable_backend_error_fails_closed(void)
{
	int enabled = 1;

	assert(vpn_server_config_enable_status(-1, EACCES, NULL, &enabled) == EACCES);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(-1, 0, NULL, &enabled) == EIO);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(2, 0, "1", &enabled) == EIO);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(1, 0, NULL, &enabled) == EIO);
	assert(enabled == 0);
	assert(vpn_server_config_enable_status(0, 0, NULL, NULL) == EINVAL);
}

static void
test_type_missing_key_preserves_pptp_default(void)
{
	int type = 2;

	assert(vpn_server_config_type_status(0, 0, NULL, &type) == 0);
	assert(type == 0);
}

static void
test_type_accepts_only_supported_modes(void)
{
	int type = -1;

	assert(vpn_server_config_type_status(1, 0, "0", &type) == 0);
	assert(type == 0);
	assert(vpn_server_config_type_status(1, 0, "1", &type) == 0);
	assert(type == 1);
	assert(vpn_server_config_type_status(1, 0, "2", &type) == 0);
	assert(type == 2);
	assert(vpn_server_config_type_status(1, 0, " 1 \t", &type) == 0);
	assert(type == 1);
}

static void
test_type_rejects_malformed_and_unknown_values(void)
{
	int type = 77;

	assert(vpn_server_config_type_status(1, 0, "", &type) == EINVAL);
	assert(type == 77);
	assert(vpn_server_config_type_status(1, 0, "1x", &type) == EINVAL);
	assert(type == 77);
	assert(vpn_server_config_type_status(1, 0, "-1", &type) == EINVAL);
	assert(type == 77);
	assert(vpn_server_config_type_status(1, 0, "+1", &type) == EINVAL);
	assert(type == 77);
	assert(vpn_server_config_type_status(1, 0, "3", &type) == EINVAL);
	assert(type == 77);
	assert(vpn_server_config_type_status(1, 0, "99999999999999999999999", &type) == ERANGE);
	assert(type == 77);
}

static void
test_type_backend_error_fails_closed(void)
{
	int type = 42;

	assert(vpn_server_config_type_status(-1, EIO, NULL, &type) == EIO);
	assert(type == 42);
	assert(vpn_server_config_type_status(-1, 0, NULL, &type) == EIO);
	assert(type == 42);
	assert(vpn_server_config_type_status(2, 0, "1", &type) == EIO);
	assert(type == 42);
	assert(vpn_server_config_type_status(1, 0, NULL, &type) == EIO);
	assert(type == 42);
	assert(vpn_server_config_type_status(0, 0, NULL, NULL) == EINVAL);
}

static void
test_int_missing_and_empty_values_use_default(void)
{
	int value = 99;

	assert(vpn_server_config_int_status(0, 0, NULL, 0, 0, 50, &value) == 0);
	assert(value == 0);
	assert(vpn_server_config_int_status(1, 0, "", 0, 0, 50, &value) == 0);
	assert(value == 0);
	assert(vpn_server_config_int_status(1, 0, "  \t", 7, 0, 50, &value) == 0);
	assert(value == 7);
}

static void
test_int_accepts_only_complete_bounded_decimal(void)
{
	int value = -1;

	assert(vpn_server_config_int_status(1, 0, "0", 0, 0, 50, &value) == 0);
	assert(value == 0);
	assert(vpn_server_config_int_status(1, 0, " 25 \t", 0, 0, 50, &value) == 0);
	assert(value == 25);
	assert(vpn_server_config_int_status(1, 0, "50", 0, 0, 50, &value) == 0);
	assert(value == 50);
}

static void
test_int_rejects_malformed_and_out_of_range_values(void)
{
	int value = 19;

	assert(vpn_server_config_int_status(1, 0, "10x", 0, 0, 50, &value) == EINVAL);
	assert(value == 19);
	assert(vpn_server_config_int_status(1, 0, "-1", 0, 0, 50, &value) == EINVAL);
	assert(value == 19);
	assert(vpn_server_config_int_status(1, 0, "+1", 0, 0, 50, &value) == EINVAL);
	assert(value == 19);
	assert(vpn_server_config_int_status(1, 0, "51", 0, 0, 50, &value) == EINVAL);
	assert(value == 19);
	assert(vpn_server_config_int_status(1, 0, "999999999999999999999999", 0, 0, 50, &value) == ERANGE);
	assert(value == 19);
}

static void
test_int_backend_error_fails_closed(void)
{
	int value = 19;

	assert(vpn_server_config_int_status(-1, EACCES, NULL, 0, 0, 50, &value) == EACCES);
	assert(value == 19);
	assert(vpn_server_config_int_status(-1, 0, NULL, 0, 0, 50, &value) == EIO);
	assert(value == 19);
	assert(vpn_server_config_int_status(2, 0, "1", 0, 0, 50, &value) == EIO);
	assert(value == 19);
	assert(vpn_server_config_int_status(1, 0, NULL, 0, 0, 50, &value) == EIO);
	assert(value == 19);
	assert(vpn_server_config_int_status(0, 0, NULL, 99, 0, 50, &value) == EINVAL);
	assert(value == 19);
	assert(vpn_server_config_int_status(0, 0, NULL, 0, 51, 50, &value) == EINVAL);
	assert(value == 19);
}

static void
test_pool_ipv4_parser_is_strict(void)
{
	uint32_t address = 0;
	assert(vpn_server_pool_parse_ipv4("192.168.1.1", &address) == 0);
	assert(address == 0xc0a80101U);
	assert(vpn_server_pool_parse_ipv4("192.168.1", &address) == EINVAL);
	assert(vpn_server_pool_parse_ipv4("192.168.1.999", &address) == EINVAL);
	assert(vpn_server_pool_parse_ipv4("not-an-ip", &address) == EINVAL);
	assert(vpn_server_pool_parse_ipv4(NULL, &address) == EINVAL);
}

static void
test_pool_lan_defaults_and_client_range(void)
{
	uint32_t local = 0, mask = 0, begin = 0, end = 0;
	assert(vpn_server_pool_resolve_lan(0xc0a80101U, 0xffffff00U,
					   245, 254, 50,
					   &local, &mask, &begin, &end) == 0);
	assert(local == 0xc0a801f5U && mask == 0xffffff00U);
	assert(begin == 246U && end == 254U);
}

static void
test_pool_lan_small_subnet_is_clamped_safely(void)
{
	uint32_t local = 0, mask = 0, begin = 0, end = 0;
	assert(vpn_server_pool_resolve_lan(0xc0a80101U, 0xfffffff8U,
					   245, 254, 50,
					   &local, &mask, &begin, &end) == 0);
	assert(local == 0xc0a80105U && mask == 0xfffffff8U);
	assert(begin == 6U && end == 6U);
}

static void
test_pool_lan_rejects_invalid_masks_hosts_and_gateway_overlap(void)
{
	uint32_t local = 1, mask = 2, begin = 3, end = 4;
	assert(vpn_server_pool_resolve_lan(0xc0a80101U, 0xff00ff00U,
					   245, 254, 50,
					   &local, &mask, &begin, &end) == EINVAL);
	assert(local == 1 && mask == 2 && begin == 3 && end == 4);
	assert(vpn_server_pool_resolve_lan(0xc0a80100U, 0xffffff00U,
					   20, 30, 50,
					   &local, &mask, &begin, &end) == EINVAL);
	assert(vpn_server_pool_resolve_lan(0xc0a801faU, 0xffffff00U,
					   245, 254, 50,
					   &local, &mask, &begin, &end) == EADDRINUSE);
	assert(vpn_server_pool_resolve_lan(0xc0a80101U, 0xffffff00U,
					   20, 30, 5,
					   &local, &mask, &begin, &end) == EINVAL);
}

static void
test_pool_virtual_private_network_and_lan_separation(void)
{
	uint32_t local = 0, mask = 0, begin = 0, end = 0;
	assert(vpn_server_pool_resolve_virtual(0x0a080001U,
					       0xc0a80101U, 0xffffff00U,
					       50, &local, &mask, &begin, &end) == 0);
	assert(local == 0x0a080001U && mask == 0xffffff00U);
	assert(begin == 2U && end == 51U);
	assert(vpn_server_pool_resolve_virtual(0xc0a80100U,
					       0xc0a80101U, 0xffffff00U,
					       50, &local, &mask, &begin, &end) == EADDRINUSE);
	assert(vpn_server_pool_resolve_virtual(0xc0a80000U,
					       0xc0a80101U, 0xffff0000U,
					       50, &local, &mask, &begin, &end) == EADDRINUSE);
	assert(vpn_server_pool_resolve_virtual(0x08080800U,
					       0xc0a80101U, 0xffffff00U,
					       50, &local, &mask, &begin, &end) == EINVAL);
}

static void
test_vpn_callback_mode_bounds(void)
{
	int virtual_mode = -1, cast_mode = -1;

	assert(vpn_server_config_int_status(1, 0, "2", 0, 0, 2,
					    &virtual_mode) == 0);
	assert(virtual_mode == 2);
	assert(vpn_server_config_int_status(1, 0, "3", 0, 0, 2,
					    &virtual_mode) == EINVAL);
	assert(vpn_server_config_int_status(1, 0, "3", 0, 0, 3,
					    &cast_mode) == 0);
	assert(cast_mode == 3);
	assert(vpn_server_config_int_status(1, 0, "4", 0, 0, 3,
					    &cast_mode) == EINVAL);
}

static void
test_openvpn_server_option_bounds(void)
{
	int value = -1;

	assert(vpn_server_config_int_status(1, 0, "5", 0, 0, 5, &value) == 0);
	assert(value == 5);
	assert(vpn_server_config_int_status(1, 0, "6", 0, 0, 5, &value) == EINVAL);
	assert(vpn_server_config_int_status(1, 0, "15", 0, 0, 15, &value) == 0);
	assert(value == 15);
	assert(vpn_server_config_int_status(1, 0, "16", 0, 0, 15, &value) == EINVAL);
	assert(vpn_server_config_int_status(1, 0, "4", 0, 0, 4, &value) == 0);
	assert(value == 4);
	assert(vpn_server_config_int_status(1, 0, "0", 1194, 1, 65535, &value) == EINVAL);
	assert(vpn_server_config_int_status(1, 0, "65535", 1194, 1, 65535, &value) == 0);
	assert(value == 65535);
	assert(vpn_server_config_int_status(0, 0, NULL, 1194, 1, 65535, &value) == 0);
	assert(value == 1194);
}

static void
test_openvpn_server_mode_bounds(void)
{
	int mode = -1;

	assert(vpn_server_config_int_status(1, 0, "0", 0, 0, 1, &mode) == 0);
	assert(mode == 0);
	assert(vpn_server_config_int_status(1, 0, "1", 0, 0, 1, &mode) == 0);
	assert(mode == 1);
	assert(vpn_server_config_int_status(1, 0, "2", 0, 0, 1, &mode) == EINVAL);
	assert(vpn_server_config_int_status(1, 0, "01", 0, 0, 1, &mode) == 0);
	assert(mode == 1);
	assert(vpn_server_config_int_status(-1, EIO, NULL, 0, 0, 1, &mode) == EIO);
}

static void
test_xl2tpd_stop_set_ownership_policy(void)
{
	assert(!vpn_server_xl2tpd_should_be_in_stop_set(0, 1, 0));
	assert(!vpn_server_xl2tpd_should_be_in_stop_set(1, 0, 0));
	assert(vpn_server_xl2tpd_should_be_in_stop_set(1, 1, 0));
	assert(vpn_server_xl2tpd_should_be_in_stop_set(0, 0, 1));
	assert(vpn_server_xl2tpd_should_be_in_stop_set(1, 1, 1));
}

static void
test_route_netmask_validation(void)
{
	assert(vpn_server_route_netmask_status(0xffffff00U) == 0);
	assert(vpn_server_route_netmask_status(0xffff0000U) == 0);
	assert(vpn_server_route_netmask_status(0x80000000U) == 0);
	assert(vpn_server_route_netmask_status(0xffffffffU) == 0);
	assert(vpn_server_route_netmask_status(0) == EINVAL);
	assert(vpn_server_route_netmask_status(0xff00ff00U) == EINVAL);
	assert(vpn_server_route_netmask_status(0xffffff01U) == EINVAL);
}

int
main(void)
{
	test_enable_missing_key_is_disabled();
	test_enable_requires_exact_one();
	test_enable_backend_error_fails_closed();
	test_type_missing_key_preserves_pptp_default();
	test_type_accepts_only_supported_modes();
	test_type_rejects_malformed_and_unknown_values();
	test_type_backend_error_fails_closed();
	test_int_missing_and_empty_values_use_default();
	test_int_accepts_only_complete_bounded_decimal();
	test_int_rejects_malformed_and_out_of_range_values();
	test_int_backend_error_fails_closed();
	test_pool_ipv4_parser_is_strict();
	test_pool_lan_defaults_and_client_range();
	test_pool_lan_small_subnet_is_clamped_safely();
	test_pool_lan_rejects_invalid_masks_hosts_and_gateway_overlap();
	test_pool_virtual_private_network_and_lan_separation();
	test_vpn_callback_mode_bounds();
	test_openvpn_server_option_bounds();
	test_openvpn_server_mode_bounds();
	test_xl2tpd_stop_set_ownership_policy();
	test_route_netmask_validation();

	puts("VPN server configuration read gate policy: PASS");
	return 0;
}
