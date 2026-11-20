#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

struct fake_param_copy {
	int fail_at;
	int failure_status;
	unsigned int calls;
	char copied[4][16];
};

static int
fake_set_param(void *opaque, const char *param_name)
{
	struct fake_param_copy *fake = (struct fake_param_copy *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 4);
	snprintf(fake->copied[index], sizeof(fake->copied[index]), "%s", param_name);
	if ((int)index == fake->fail_at)
		return fake->failure_status;
	return 0;
}

static void
test_successful_required_param_sequence(void)
{
	static const char *const params[] = { "ipaddr", "netmask", "gateway", "mtu" };
	struct fake_param_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_set_required_params(params, 4, fake_set_param, &fake) == 0);
	assert(fake.calls == 4);
	assert(strcmp(fake.copied[0], "ipaddr") == 0);
	assert(strcmp(fake.copied[1], "netmask") == 0);
	assert(strcmp(fake.copied[2], "gateway") == 0);
	assert(strcmp(fake.copied[3], "mtu") == 0);
}

static void
test_required_param_sequence_stops_at_first_error(void)
{
	static const char *const params[] = { "ipaddr", "netmask", "gateway", "mtu" };
	struct fake_param_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 2;
	fake.failure_status = EACCES;
	assert(rc_wan_nvram_set_required_params(params, 4, fake_set_param, &fake) == EACCES);
	assert(fake.calls == 3);
	assert(strcmp(fake.copied[0], "ipaddr") == 0);
	assert(strcmp(fake.copied[1], "netmask") == 0);
	assert(strcmp(fake.copied[2], "gateway") == 0);
	assert(fake.copied[3][0] == '\0');
}

static void
test_invalid_required_param_arguments_fail_closed(void)
{
	static const char *const params[] = { "ipaddr" };
	static const char *const empty_params[] = { "" };
	struct fake_param_copy fake;

	memset(&fake, 0, sizeof(fake));
	assert(rc_wan_nvram_set_required_params(NULL, 1, fake_set_param, &fake) == EINVAL);
	assert(rc_wan_nvram_set_required_params(params, 1, NULL, &fake) == EINVAL);
	assert(rc_wan_nvram_set_required_params(empty_params, 1, fake_set_param, &fake) == EINVAL);
	assert(fake.calls == 0);
}

static void
test_success_ignores_stale_errno(void)
{
	assert(rc_wan_nvram_write_status(0, EACCES) == 0);
}

static void
test_failure_preserves_errno(void)
{
	assert(rc_wan_nvram_write_status(-1, EACCES) == EACCES);
}

static void
test_failure_without_errno_falls_back_to_eio(void)
{
	assert(rc_wan_nvram_write_status(-1, 0) == EIO);
}

static void
test_unexpected_nonzero_result_is_not_success(void)
{
	assert(rc_wan_nvram_write_status(1, 0) == EIO);
}


struct fake_value_copy {
	int fail_at;
	int failure_status;
	unsigned int calls;
	char copied_names[8][16];
	char copied_values[8][32];
};

static int
fake_set_value(void *opaque, const char *param_name, const char *value)
{
	struct fake_value_copy *fake = (struct fake_value_copy *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 8);
	snprintf(fake->copied_names[index], sizeof(fake->copied_names[index]),
		 "%s", param_name);
	snprintf(fake->copied_values[index], sizeof(fake->copied_values[index]),
		 "%s", value);
	if ((int)index == fake->fail_at)
		return fake->failure_status;
	return 0;
}

static void
test_required_value_sequence_success(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "dns", "" }, { "routes", "" }, { "err", "0" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_set_required_values(
		values, 3, fake_set_value, &fake) == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.copied_names[0], "dns") == 0);
	assert(strcmp(fake.copied_values[0], "") == 0);
	assert(strcmp(fake.copied_names[1], "routes") == 0);
	assert(strcmp(fake.copied_names[2], "err") == 0);
	assert(strcmp(fake.copied_values[2], "0") == 0);
}

static void
test_required_value_sequence_stops_at_first_error(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "dns", "" }, { "routes", "" }, { "err", "0" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 1;
	fake.failure_status = EACCES;
	assert(rc_wan_nvram_set_required_values(
		values, 3, fake_set_value, &fake) == EACCES);
	assert(fake.calls == 2);
	assert(strcmp(fake.copied_names[0], "dns") == 0);
	assert(strcmp(fake.copied_names[1], "routes") == 0);
	assert(fake.copied_names[2][0] == '\0');
}

static void
test_required_value_sequence_rejects_invalid_values(void)
{
	static const struct rc_wan_nvram_value empty_name[] = {
		{ "", "value" }
	};
	static const struct rc_wan_nvram_value null_value[] = {
		{ "dns", NULL }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	assert(rc_wan_nvram_set_required_values(
		NULL, 1, fake_set_value, &fake) == EINVAL);
	assert(rc_wan_nvram_set_required_values(
		empty_name, 1, fake_set_value, &fake) == EINVAL);
	assert(rc_wan_nvram_set_required_values(
		null_value, 1, fake_set_value, &fake) == EINVAL);
	assert(fake.calls == 0);
}


static void
test_all_value_sequence_attempts_every_write_and_preserves_first_error(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "wanx_gateway", "" }, { "wanx_dns", "" },
		{ "wanx_routes", "" }, { "wanx_routes_ms", "" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 1;
	fake.failure_status = EACCES;
	assert(rc_wan_nvram_set_all_values(
		values, 4, fake_set_value, &fake) == EACCES);
	assert(fake.calls == 4);
	assert(strcmp(fake.copied_names[0], "wanx_gateway") == 0);
	assert(strcmp(fake.copied_names[1], "wanx_dns") == 0);
	assert(strcmp(fake.copied_names[2], "wanx_routes") == 0);
	assert(strcmp(fake.copied_names[3], "wanx_routes_ms") == 0);
	assert(strcmp(fake.copied_values[0], "") == 0);
}


static void
test_wan_statistics_clear_attempts_every_counter(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "err", "0" }, { "uptime", "0" }, { "dltime", "0" },
		{ "bytes_rx", "0" }, { "bytes_tx", "0" }
	};
	struct fake_value_copy fake;
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 2;
	fake.failure_status = ENOSPC;
	assert(rc_wan_nvram_set_all_values(values, 5, fake_set_value, &fake) == ENOSPC);
	assert(fake.calls == 5);
	assert(strcmp(fake.copied_names[0], "err") == 0);
	assert(strcmp(fake.copied_names[1], "uptime") == 0);
	assert(strcmp(fake.copied_names[2], "dltime") == 0);
	assert(strcmp(fake.copied_names[3], "bytes_rx") == 0);
	assert(strcmp(fake.copied_names[4], "bytes_tx") == 0);
	for (i = 0; i < 5; ++i)
		assert(strcmp(fake.copied_values[i], "0") == 0);
}

static void
test_vpn_runtime_state_clear_attempts_every_key_and_preserves_first_error(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "l2tp_wan_t", "0" }, { "vpnc_dns_t", "" },
		{ "vpnc_dom_t", "" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.failure_status = EIO;
	assert(rc_wan_nvram_set_all_values(values, 3, fake_set_value, &fake) == EIO);
	assert(fake.calls == 3);
	assert(strcmp(fake.copied_names[0], "l2tp_wan_t") == 0);
	assert(strcmp(fake.copied_names[1], "vpnc_dns_t") == 0);
	assert(strcmp(fake.copied_names[2], "vpnc_dom_t") == 0);
	assert(strcmp(fake.copied_values[0], "0") == 0);
	assert(strcmp(fake.copied_values[1], "") == 0);
	assert(strcmp(fake.copied_values[2], "") == 0);
}

static void
test_all_value_sequence_validates_before_writing(void)
{
	static const struct rc_wan_nvram_value invalid[] = {
		{ "wanx_gateway", "" }, { "", "value" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	assert(rc_wan_nvram_set_all_values(
		invalid, 2, fake_set_value, &fake) == EINVAL);
	assert(fake.calls == 0);
}

int
main(void)
{
	test_successful_required_param_sequence();
	test_required_param_sequence_stops_at_first_error();
	test_invalid_required_param_arguments_fail_closed();
	test_success_ignores_stale_errno();
	test_failure_preserves_errno();
	test_failure_without_errno_falls_back_to_eio();
	test_unexpected_nonzero_result_is_not_success();
	test_required_value_sequence_success();
	test_required_value_sequence_stops_at_first_error();
	test_required_value_sequence_rejects_invalid_values();
	test_all_value_sequence_attempts_every_write_and_preserves_first_error();
	test_wan_statistics_clear_attempts_every_counter();
	test_vpn_runtime_state_clear_attempts_every_key_and_preserves_first_error();
	test_all_value_sequence_validates_before_writing();
	return 0;
}
