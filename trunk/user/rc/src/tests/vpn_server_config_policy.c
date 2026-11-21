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

	puts("VPN server configuration read gate policy: PASS");
	return 0;
}
