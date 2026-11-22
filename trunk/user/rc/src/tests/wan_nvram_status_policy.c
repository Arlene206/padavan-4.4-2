#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
test_checked_read_accepts_present_and_absent_values(void)
{
	assert(rc_wan_nvram_read_status(1, EACCES) == 0);
	assert(rc_wan_nvram_read_status(0, EIO) == 0);
}

static void
test_checked_read_preserves_backend_error(void)
{
	assert(rc_wan_nvram_read_status(-1, EACCES) == EACCES);
}

static void
test_checked_read_without_errno_falls_back_to_eio(void)
{
	assert(rc_wan_nvram_read_status(-1, 0) == EIO);
}

static void
test_checked_read_rejects_unexpected_result(void)
{
	assert(rc_wan_nvram_read_status(2, 0) == EIO);
}

static void
test_checked_string_match_requires_an_exact_value(void)
{
	int matches = -1;

	assert(rc_wan_nvram_match_status(0, "1", "1", &matches) == 0);
	assert(matches == 1);
	assert(rc_wan_nvram_match_status(0, "0", "1", &matches) == 0);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(0, "01", "1", &matches) == 0);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(0, "1junk", "1", &matches) == 0);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(0, "", "1", &matches) == 0);
	assert(matches == 0);
}

static void
test_checked_string_match_preserves_backend_errors(void)
{
	int matches = 1;

	assert(rc_wan_nvram_match_status(EACCES, "1", "1", &matches) == EACCES);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(EIO, NULL, "1", &matches) == EIO);
	assert(matches == 0);
}

static void
test_dhcp6_6rd_selector_preserves_missing_value_and_backend_errors(void)
{
	int disabled = -1;

	assert(rc_wan_nvram_match_status(0, "0", "0", &disabled) == 0);
	assert(disabled == 1);
	assert(rc_wan_nvram_match_status(0, "", "0", &disabled) == 0);
	assert(disabled == 0); /* Missing key keeps legacy automatic behavior. */
	assert(rc_wan_nvram_match_status(0, "malformed", "0", &disabled) == 0);
	assert(disabled == 0); /* Only the exact string "0" disables DHCP 6rd. */
	assert(rc_wan_nvram_match_status(EACCES, "0", "0", &disabled) == EACCES);
	assert(disabled == 0);
}

static void
test_checked_string_match_rejects_invalid_arguments(void)
{
	int matches = 1;

	assert(rc_wan_nvram_match_status(0, NULL, "1", &matches) == EIO);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(0, "1", NULL, &matches) == EINVAL);
	assert(matches == 0);
	assert(rc_wan_nvram_match_status(0, "1", "1", NULL) == EINVAL);
}


static void
test_wisp_active_status_requires_checked_identity(void)
{
	int active = 91;

	assert(rc_wan_nvram_wisp_active_status(0, 0, 1, &active) == 0);
	assert(active == 1);
	assert(rc_wan_nvram_wisp_active_status(0, 0, 0, &active) == 0);
	assert(active == 0);

	active = 91;
	assert(rc_wan_nvram_wisp_active_status(EACCES, 0, 1, &active) == EACCES);
	assert(active == 0);
	active = 91;
	assert(rc_wan_nvram_wisp_active_status(0, EINVAL, 1, &active) == EINVAL);
	assert(active == 0);
	assert(rc_wan_nvram_wisp_active_status(EACCES, EINVAL, 1, &active) == EACCES);
	assert(active == 0);
	assert(rc_wan_nvram_wisp_active_status(0, 0, 1, NULL) == EINVAL);
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



static void
test_checked_integer_range_preserves_valid_values_and_legacy_fallback(void)
{
	int normalized = 91;

	assert(rc_wan_nvram_int_range_status(0, 0, 0, 7, 0, &normalized) == 0);
	assert(normalized == 0);
	assert(rc_wan_nvram_int_range_status(0, 7, 0, 7, 0, &normalized) == 0);
	assert(normalized == 7);
	assert(rc_wan_nvram_int_range_status(0, 8, 0, 7, 0, &normalized) == 0);
	assert(normalized == 0);
	assert(rc_wan_nvram_int_range_status(0, -1, 0, 2, 0, &normalized) == 0);
	assert(normalized == 0);
}

static void
test_checked_integer_range_preserves_read_errors_and_output(void)
{
	int normalized = 73;

	assert(rc_wan_nvram_int_range_status(EACCES, 2, 0, 7, 0, &normalized) == EACCES);
	assert(normalized == 73);
	assert(rc_wan_nvram_int_range_status(EIO, 2, 0, 7, 0, &normalized) == EIO);
	assert(normalized == 73);
	assert(rc_wan_nvram_int_range_status(0, 2, 0, 7, 8, &normalized) == EINVAL);
	assert(normalized == 73);
	assert(rc_wan_nvram_int_range_status(0, 2, 7, 0, 0, &normalized) == EINVAL);
	assert(normalized == 73);
	assert(rc_wan_nvram_int_range_status(0, 2, 0, 7, 0, NULL) == EINVAL);
}

static void
test_modem_transport_selection_fails_closed_on_read_error(void)
{
	int is_ndis = -1;

	assert(rc_wan_nvram_modem_is_ndis_status(EACCES, 3, &is_ndis) == EACCES);
	assert(is_ndis == -1);
	assert(rc_wan_nvram_modem_is_ndis_status(0, 3, &is_ndis) == 0);
	assert(is_ndis == 1);
	assert(rc_wan_nvram_modem_is_ndis_status(0, 0, &is_ndis) == 0);
	assert(is_ndis == 0);
	assert(rc_wan_nvram_modem_is_ndis_status(0, 3, NULL) == EINVAL);
}


struct fake_missing_default {
	const char *target_name;
	const char *target_value;
	const char *source_name;
	const char *source_value;
	int fail_read_at;
	int read_failure;
	int write_failure;
	unsigned int read_calls;
	unsigned int write_calls;
	char written_name[32];
	char written_value[64];
};

static int
fake_get_owned(void *opaque, const char *param_name,
	       char **value, int *present)
{
	struct fake_missing_default *fake =
		(struct fake_missing_default *)opaque;
	const char *stored = NULL;
	unsigned int index = fake->read_calls++;

	assert(index < 4);
	*value = NULL;
	*present = 0;
	if ((int)index == fake->fail_read_at)
		return fake->read_failure ? fake->read_failure : EIO;

	if (strcmp(param_name, fake->target_name) == 0)
		stored = fake->target_value;
	else if (strcmp(param_name, fake->source_name) == 0)
		stored = fake->source_value;
	else
		return ENOENT;

	if (!stored)
		return 0;
	*value = (char *)malloc(strlen(stored) + 1);
	if (!*value)
		return ENOMEM;
	memcpy(*value, stored, strlen(stored) + 1);
	*present = 1;
	return 0;
}

static int
fake_set_default(void *opaque, const char *param_name, const char *value)
{
	struct fake_missing_default *fake =
		(struct fake_missing_default *)opaque;

	fake->write_calls++;
	snprintf(fake->written_name, sizeof(fake->written_name), "%s", param_name);
	snprintf(fake->written_value, sizeof(fake->written_value), "%s", value);
	return fake->write_failure;
}

static void
init_fake_missing_default(struct fake_missing_default *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake->target_name = "wan0_proto";
	fake->source_name = "wan_proto";
	fake->fail_read_at = -1;
}

static void
test_wan0_default_copy_preserves_empty_existing_override(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	fake.target_value = "";
	fake.source_value = "dhcp";
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == 0);
	assert(fake.read_calls == 1);
	assert(fake.write_calls == 0);
}

static void
test_wan0_default_copy_uses_checked_source_value(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	fake.source_value = "pppoe";
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == 0);
	assert(fake.read_calls == 2);
	assert(fake.write_calls == 1);
	assert(strcmp(fake.written_name, "wan0_proto") == 0);
	assert(strcmp(fake.written_value, "pppoe") == 0);
}

static void
test_wan0_default_copy_uses_factory_fallback_when_source_is_absent(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == 0);
	assert(fake.read_calls == 2);
	assert(fake.write_calls == 1);
	assert(strcmp(fake.written_value, "static") == 0);
}

static void
test_wan0_default_copy_stops_on_target_read_error(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	fake.fail_read_at = 0;
	fake.read_failure = EBUSY;
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == EBUSY);
	assert(fake.read_calls == 1);
	assert(fake.write_calls == 0);
}

static void
test_wan0_default_copy_stops_on_source_read_error(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	fake.fail_read_at = 1;
	fake.read_failure = EIO;
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == EIO);
	assert(fake.read_calls == 2);
	assert(fake.write_calls == 0);
}

static void
test_wan0_default_copy_propagates_write_error(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	fake.write_failure = ENOSPC;
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == ENOSPC);
	assert(fake.read_calls == 2);
	assert(fake.write_calls == 1);
}

static void
test_wan0_default_copy_rejects_invalid_arguments(void)
{
	struct fake_missing_default fake;

	init_fake_missing_default(&fake);
	assert(rc_wan_nvram_copy_missing_default(
		NULL, fake_set_default, &fake,
		fake.target_name, fake.source_name, "static") == EINVAL);
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, NULL, &fake,
		fake.target_name, fake.source_name, "static") == EINVAL);
	assert(rc_wan_nvram_copy_missing_default(
		fake_get_owned, fake_set_default, &fake,
		"", fake.source_name, "static") == EINVAL);
	assert(fake.read_calls == 0);
	assert(fake.write_calls == 0);
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
test_wan0_boot_values_stop_at_each_nvram_failure(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "unit", "0" },
		{ "primary", "1" },
		{ "ifname", "eth0" },
		{ "uptime", "0000000000" },
		{ "dltime", "0000000000" },
		{ "bytes_rx", "00000000000000000000" },
		{ "bytes_tx", "00000000000000000000" },
		{ "ifname_t", "" }
	};
	struct fake_value_copy fake;
	unsigned int fail_at, i;

	for (fail_at = 0; fail_at < sizeof(values) / sizeof(values[0]); ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = (int)fail_at;
		fake.failure_status = EACCES;
		assert(rc_wan_nvram_set_required_values(
			values, (unsigned int)(sizeof(values) / sizeof(values[0])),
			fake_set_value, &fake) == EACCES);
		assert(fake.calls == fail_at + 1);
		for (i = 0; i <= fail_at; ++i) {
			assert(strcmp(fake.copied_names[i], values[i].param_name) == 0);
			assert(strcmp(fake.copied_values[i], values[i].value) == 0);
		}
		if (fail_at + 1 < sizeof(values) / sizeof(values[0]))
			assert(fake.copied_names[fail_at + 1][0] == '\0');
	}
}

static void
test_runtime_identity_publication_checks_both_writes(void)
{
	struct fake_value_copy fake;
	unsigned int fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_publish_runtime_identity(
		fake_set_value, &fake, "PPPoE", "ppp0") == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.copied_names[0], "proto_t") == 0);
	assert(strcmp(fake.copied_values[0], "PPPoE") == 0);
	assert(strcmp(fake.copied_names[1], "ifname_t") == 0);
	assert(strcmp(fake.copied_values[1], "ppp0") == 0);

	for (fail_at = 0; fail_at < 2; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = (int)fail_at;
		fake.failure_status = ENOSPC;
		assert(rc_wan_nvram_publish_runtime_identity(
			fake_set_value, &fake, "IPoE", "eth2") == ENOSPC);
		assert(fake.calls == fail_at + 1);
		assert(strcmp(fake.copied_names[0], "proto_t") == 0);
		if (fail_at == 0)
			assert(fake.copied_names[1][0] == '\0');
		else {
			assert(strcmp(fake.copied_names[1], "ifname_t") == 0);
			assert(strcmp(fake.copied_values[1], "eth2") == 0);
		}
	}

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_publish_runtime_identity(
		fake_set_value, &fake, "", "ppp0") == EINVAL);
	assert(fake.calls == 0);
	assert(rc_wan_nvram_publish_runtime_identity(
		fake_set_value, &fake, "PPPoE", "") == EINVAL);
	assert(fake.calls == 0);
	assert(rc_wan_nvram_publish_runtime_identity(
		NULL, &fake, "PPPoE", "ppp0") == EINVAL);
	assert(fake.calls == 0);
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
test_wan_deconfig_statistics_clear_attempts_every_counter(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "uptime", "0" }, { "dltime", "0" },
		{ "bytes_rx", "0" }, { "bytes_tx", "0" }
	};
	struct fake_value_copy fake;
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.failure_status = ENOSPC;
	assert(rc_wan_nvram_set_all_values(
		values, 4, fake_set_value, &fake) == ENOSPC);
	assert(fake.calls == 4);
	assert(strcmp(fake.copied_names[0], "uptime") == 0);
	assert(strcmp(fake.copied_names[1], "dltime") == 0);
	assert(strcmp(fake.copied_names[2], "bytes_rx") == 0);
	assert(strcmp(fake.copied_names[3], "bytes_tx") == 0);
	for (i = 0; i < 4; ++i)
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


static void
test_wan6_reset_config_preserves_identity_when_required_write_fails(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "addr6", "2001:db8::10/64" },
		{ "gate6", "2001:db8::1" },
		{ "6rd_relay", "192.0.2.1" },
		{ "6rd_size", "32" },
		{ "dns6", "2001:db8::53" },
		{ "ifname6", "" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 4;
	fake.failure_status = ENOSPC;
	assert(rc_wan_nvram_set_required_values(
		values, 6, fake_set_value, &fake) == ENOSPC);
	assert(fake.calls == 5);
	assert(strcmp(fake.copied_names[0], "addr6") == 0);
	assert(strcmp(fake.copied_names[1], "gate6") == 0);
	assert(strcmp(fake.copied_names[2], "6rd_relay") == 0);
	assert(strcmp(fake.copied_names[3], "6rd_size") == 0);
	assert(strcmp(fake.copied_names[4], "dns6") == 0);
	assert(fake.copied_names[5][0] == '\0');
}

static void
test_wan6_reset_config_clears_identity_only_after_all_values_succeed(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "addr6", "2001:db8::10/64" },
		{ "gate6", "2001:db8::1" },
		{ "6rd_relay", "192.0.2.1" },
		{ "6rd_size", "32" },
		{ "dns6", "2001:db8::53" },
		{ "ifname6", "" }
	};
	struct fake_value_copy fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_set_required_values(
		values, 6, fake_set_value, &fake) == 0);
	assert(fake.calls == 6);
	assert(strcmp(fake.copied_names[5], "ifname6") == 0);
	assert(strcmp(fake.copied_values[5], "") == 0);
}


struct fake_nvram_reader {
	const char *values[4];
	const char *names[4];
	unsigned int calls;
	int fail_at;
	int missing_at;
	int failure_errno;
	char cache_slot[32];
};

static int
fake_nvram_get(void *opaque, const char *param_name, char **value)
{
	static const char *const names[] = {
		"wan0_ipaddr", "wan0_netmask", "lan_ipaddr", "lan_netmask"
	};
	struct fake_nvram_reader *fake = (struct fake_nvram_reader *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 4);
	assert(strcmp(param_name,
		      fake->names[index] ? fake->names[index] : names[index]) == 0);
	if ((int)index == fake->fail_at) {
		*value = NULL;
		errno = fake->failure_errno;
		return -1;
	}
	if ((int)index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}

static void
test_checked_nvram_copy_survives_shared_cache_overwrite(void)
{
	struct fake_nvram_reader fake;
	char wan_addr[16], wan_mask[16], lan_addr[16], lan_mask[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.missing_at = -1;
	fake.values[0] = "198.51.100.2";
	fake.values[1] = "255.255.255.0";
	fake.values[2] = "192.0.2.1";
	fake.values[3] = "255.255.255.0";

	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "wan0_ipaddr",
				       wan_addr, sizeof(wan_addr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "wan0_netmask",
				       wan_mask, sizeof(wan_mask)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "lan_ipaddr",
				       lan_addr, sizeof(lan_addr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "lan_netmask",
				       lan_mask, sizeof(lan_mask)) == 0);
	assert(fake.calls == 4);
	assert(strcmp(wan_addr, "198.51.100.2") == 0);
	assert(strcmp(wan_mask, "255.255.255.0") == 0);
	assert(strcmp(lan_addr, "192.0.2.1") == 0);
	assert(strcmp(lan_mask, "255.255.255.0") == 0);
}

struct fake_nvram_text_cache {
	const char *names[6];
	const char *values[6];
	unsigned int calls;
	char cache_slot[32];
};


static int
fake_nvram_text_cache_get(void *opaque, const char *param_name,
			  char **value)
{
	struct fake_nvram_text_cache *fake =
		(struct fake_nvram_text_cache *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 6);
	assert(strcmp(param_name, fake->names[index]) == 0);
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}


static void
test_viptv_settings_survive_shared_cache_overwrite(void)
{
	struct fake_nvram_text_cache fake;
	char vinet_iflast[24], viptv_iflast[24];
	char address[16], netmask[16], gateway[16], wan_hwaddr[24];

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_viflast";
	fake.names[1] = "viptv_ifname";
	fake.names[2] = "viptv_ipaddr";
	fake.names[3] = "viptv_netmask";
	fake.names[4] = "viptv_gateway";
	fake.names[5] = "wan_hwaddr";
	fake.values[0] = "eth2.10";
	fake.values[1] = "eth2.20";
	fake.values[2] = "198.51.100.2";
	fake.values[3] = "255.255.255.0";
	fake.values[4] = "198.51.100.1";
	fake.values[5] = "02:00:00:00:00:01";

	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[0], vinet_iflast,
				       sizeof(vinet_iflast)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[1], viptv_iflast,
				       sizeof(viptv_iflast)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[2], address, sizeof(address)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[3], netmask, sizeof(netmask)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[4], gateway, sizeof(gateway)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[5], wan_hwaddr, sizeof(wan_hwaddr)) == 0);
	assert(fake.calls == 6);
	assert(strcmp(vinet_iflast, "eth2.10") == 0);
	assert(strcmp(viptv_iflast, "eth2.20") == 0);
	assert(strcmp(address, "198.51.100.2") == 0);
	assert(strcmp(netmask, "255.255.255.0") == 0);
	assert(strcmp(gateway, "198.51.100.1") == 0);
	assert(strcmp(wan_hwaddr, "02:00:00:00:00:01") == 0);
}


static void
test_dhcp_hostname_and_vendor_class_survive_shared_cache_overwrite(void)
{
	struct fake_nvram_text_cache fake;
	char hostname[256], vendor_class[256];

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_hostname";
	fake.names[1] = "wan0_vci";
	fake.values[0] = "router-edge";
	fake.values[1] = "padavan-device";

	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[0], hostname,
				       sizeof(hostname)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[1], vendor_class,
				       sizeof(vendor_class)) == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.cache_slot, "padavan-device") == 0);
	assert(strcmp(hostname, "router-edge") == 0);
	assert(strcmp(vendor_class, "padavan-device") == 0);
}


static void
test_wan_interface_names_survive_shared_cache_overwrite(void)
{
	struct fake_nvram_reader fake;
	char man_ifname[16], wan_ifname[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.missing_at = -1;
	fake.names[0] = "wan0_man_ifname";
	fake.names[1] = "wan0_ifname_t";
	fake.values[0] = "br0";
	fake.values[1] = "usbnet0";

	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake,
				       "wan0_man_ifname", man_ifname,
				       sizeof(man_ifname)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake,
				       "wan0_ifname_t", wan_ifname,
				       sizeof(wan_ifname)) == 0);
	assert(fake.calls == 2);
	assert(strcmp(man_ifname, "br0") == 0);
	assert(strcmp(wan_ifname, "usbnet0") == 0);
}


static void
test_lan_subnet_identity_survives_shared_cache_overwrite(void)
{
	struct fake_nvram_text_cache fake;
	char lan_ipaddr[16], lan_netmask[16], overwritten[32];

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "lan_ipaddr";
	fake.names[1] = "lan_netmask";
	fake.names[2] = "wan0_ipaddr";
	fake.names[3] = "wan0_gateway";
	fake.values[0] = "192.168.10.1";
	fake.values[1] = "255.255.255.0";
	fake.values[2] = "198.51.100.2";
	fake.values[3] = "198.51.100.1";

	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[0], lan_ipaddr,
				       sizeof(lan_ipaddr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[1], lan_netmask,
				       sizeof(lan_netmask)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[2], overwritten,
				       sizeof(overwritten)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[3], overwritten,
				       sizeof(overwritten)) == 0);
	assert(fake.calls == 4);
	assert(strcmp(fake.cache_slot, "198.51.100.1") == 0);
	assert(strcmp(lan_ipaddr, "192.168.10.1") == 0);
	assert(strcmp(lan_netmask, "255.255.255.0") == 0);
}


static void
test_dhcp_fallback_netmask_survives_previous_tuple_reads(void)
{
	struct fake_nvram_text_cache fake;
	char lease_netmask[16], overwritten[32];

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_netmask";
	fake.names[1] = "wan0_ipaddr";
	fake.names[2] = "wan0_gateway";
	fake.values[0] = "255.255.255.0";
	fake.values[1] = "198.51.100.2";
	fake.values[2] = "198.51.100.1";

	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[0], lease_netmask,
				       sizeof(lease_netmask)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[1], overwritten,
				       sizeof(overwritten)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[2], overwritten,
				       sizeof(overwritten)) == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.cache_slot, "198.51.100.1") == 0);
	assert(strcmp(lease_netmask, "255.255.255.0") == 0);
}


static void
test_gateway_arp_identity_snapshots_survive_cache_reuse(void)
{
	struct fake_nvram_text_cache fake;
	char gateway[16], hardware_address[18], overwritten[32];

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "lan_gateway_t";
	fake.names[1] = "lan_hwaddr";
	fake.names[2] = "vlan_vid_cpu";
	fake.values[0] = "192.0.2.1";
	fake.values[1] = "02:11:22:33:44:55";
	fake.values[2] = "7";

	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[0], gateway,
				       sizeof(gateway)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[1], hardware_address,
				       sizeof(hardware_address)) == 0);
	assert(rc_wan_nvram_copy_value(fake_nvram_text_cache_get, &fake,
				       fake.names[2], overwritten,
				       sizeof(overwritten)) == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.cache_slot, "7") == 0);
	assert(strcmp(gateway, "192.0.2.1") == 0);
	assert(strcmp(hardware_address, "02:11:22:33:44:55") == 0);
}


static void
test_bounded_text_copy_survives_source_invalidation_and_rejects_truncation(void)
{
	char cache_slot[32] = "198.51.100.27";
	char destination[16] = "stale";

	assert(rc_wan_nvram_copy_text(cache_slot, destination,
				      sizeof(destination)) == 0);
	assert(strcmp(destination, "198.51.100.27") == 0);

	/* Model nvram_set_temp() invalidating the shared cache storage. */
	memset(cache_slot, 'x', sizeof(cache_slot));
	cache_slot[sizeof(cache_slot) - 1] = '\0';
	assert(strcmp(destination, "198.51.100.27") == 0);

	assert(rc_wan_nvram_copy_text("255.255.255.255", destination,
				      sizeof(destination)) == 0);
	assert(strcmp(destination, "255.255.255.255") == 0);
	assert(rc_wan_nvram_copy_text(NULL, destination,
				      sizeof(destination)) == 0);
	assert(destination[0] == '\0');

	strcpy(destination, "stale");
	assert(rc_wan_nvram_copy_text("255.255.255.255x", destination,
				      sizeof(destination)) == ENAMETOOLONG);
	assert(destination[0] == '\0');
	assert(rc_wan_nvram_copy_text("192.0.2.1", NULL, 16) == EINVAL);
	assert(rc_wan_nvram_copy_text("192.0.2.1", destination, 0) == EINVAL);

	/* The helper is also safe when source and destination are identical. */
	strcpy(destination, "192.0.2.1");
	assert(rc_wan_nvram_copy_text(destination, destination,
				      sizeof(destination)) == 0);
	assert(strcmp(destination, "192.0.2.1") == 0);
}


static void
test_legacy_mac_override_preserves_mac_conv_formatting(void)
{
	char formatted[18];

	assert(rc_wan_nvram_format_mac_override(
		"001122aAbBcC", formatted, sizeof(formatted)) == 0);
	assert(strcmp(formatted, "00:11:22:aA:bB:cC") == 0);

	/* The legacy converter accepts any twelve characters, not only hex. */
	assert(rc_wan_nvram_format_mac_override(
		"zz1122334455", formatted, sizeof(formatted)) == 0);
	assert(strcmp(formatted, "zz:11:22:33:44:55") == 0);
}

static void
test_legacy_mac_override_zero_and_broadcast_values_use_fallback(void)
{
	char formatted[18] = "stale";

	assert(rc_wan_nvram_format_mac_override(
		"000000000000", formatted, sizeof(formatted)) == 0);
	assert(formatted[0] == '\0');
	assert(rc_wan_nvram_format_mac_override(
		"ffffffffffff", formatted, sizeof(formatted)) == 0);
	assert(formatted[0] == '\0');
	assert(rc_wan_nvram_format_mac_override(
		"FFFFFFFFFFFF", formatted, sizeof(formatted)) == 0);
	assert(formatted[0] == '\0');
	assert(rc_wan_nvram_format_mac_override(
		"00112233445", formatted, sizeof(formatted)) == 0);
	assert(formatted[0] == '\0');
	assert(rc_wan_nvram_format_mac_override(
		NULL, formatted, sizeof(formatted)) == 0);
	assert(formatted[0] == '\0');
}

static void
test_legacy_mac_override_rejects_invalid_destination(void)
{
	char formatted[17] = "stale";

	assert(rc_wan_nvram_format_mac_override(
		"001122334455", formatted, sizeof(formatted)) == ENAMETOOLONG);
	assert(formatted[0] == '\0');
	assert(rc_wan_nvram_format_mac_override(
		"001122334455", NULL, 18) == EINVAL);
	assert(rc_wan_nvram_format_mac_override(
		"001122334455", formatted, 0) == EINVAL);
}


static void
test_checked_nvram_copy_treats_missing_key_as_empty(void)
{
	struct fake_nvram_reader fake;
	char destination[16] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.missing_at = 0;
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "wan0_ipaddr",
				       destination, sizeof(destination)) == 0);
	assert(fake.calls == 1);
	assert(destination[0] == '\0');
}

static void
test_checked_nvram_copy_preserves_backend_failure(void)
{
	struct fake_nvram_reader fake;
	char destination[16] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.missing_at = -1;
	fake.failure_errno = EACCES;
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "wan0_ipaddr",
				       destination, sizeof(destination)) == EACCES);
	assert(fake.calls == 1);
	assert(destination[0] == '\0');
}

static void
test_checked_nvram_copy_rejects_truncation(void)
{
	struct fake_nvram_reader fake;
	char destination[8] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.missing_at = -1;
	fake.values[0] = "192.0.2.123";
	assert(rc_wan_nvram_copy_value(fake_nvram_get, &fake, "wan0_ipaddr",
				       destination, sizeof(destination)) == ENAMETOOLONG);
	assert(destination[0] == '\0');
}

static void
test_checked_nvram_copy_rejects_invalid_arguments(void)
{
	char destination[8];

	assert(rc_wan_nvram_copy_value(NULL, NULL, "wan0_ipaddr",
				       destination, sizeof(destination)) == EINVAL);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, NULL, "wan0_ipaddr",
				       NULL, sizeof(destination)) == EINVAL);
	assert(rc_wan_nvram_copy_value(fake_nvram_get, NULL, "wan0_ipaddr",
				       destination, 0) == EINVAL);
}


struct fake_nvram_transaction {
	const char *names[16];
	const char *values[16];
	unsigned int read_calls;
	unsigned int begin_calls;
	unsigned int end_calls;
	int begin_error;
	int read_fail_at;
	int missing_at;
	int read_error;
	int end_error;
	char cache_slot[64];
};

static int
fake_snapshot_begin(void *opaque)
{
	struct fake_nvram_transaction *fake =
		(struct fake_nvram_transaction *)opaque;

	++fake->begin_calls;
	if (fake->begin_error != 0) {
		errno = fake->begin_error;
		return -1;
	}
	return 0;
}

static int
fake_snapshot_get(void *opaque, const char *param_name, char **value)
{
	struct fake_nvram_transaction *fake =
		(struct fake_nvram_transaction *)opaque;
	unsigned int index = fake->read_calls++;

	assert(index < 16);
	assert(strcmp(param_name, fake->names[index]) == 0);
	if ((int)index == fake->read_fail_at) {
		*value = NULL;
		errno = fake->read_error;
		return -1;
	}
	if ((int)index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}

static int
fake_snapshot_end(void *opaque)
{
	struct fake_nvram_transaction *fake =
		(struct fake_nvram_transaction *)opaque;

	++fake->end_calls;
	if (fake->end_error != 0) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}

static void
init_fake_snapshot(struct fake_nvram_transaction *fake,
		   struct rc_wan_nvram_snapshot_field *fields,
		   char destinations[4][64])
{
	memset(fake, 0, sizeof(*fake));
	fake->names[0] = "wanx_gateway";
	fake->names[1] = "wanx_ipaddr";
	fake->names[2] = "wanx_netmask";
	fake->names[3] = "wanx_dns";
	fake->values[0] = "198.51.100.1";
	fake->values[1] = "192.0.2.2";
	fake->values[2] = "255.255.255.0";
	fake->values[3] = "1.1.1.1 8.8.8.8";
	fake->read_fail_at = -1;
	fake->missing_at = -1;

	fields[0].param_name = fake->names[0];
	fields[0].destination = destinations[0];
	fields[0].destination_size = sizeof(destinations[0]);
	fields[1].param_name = fake->names[1];
	fields[1].destination = destinations[1];
	fields[1].destination_size = sizeof(destinations[1]);
	fields[2].param_name = fake->names[2];
	fields[2].destination = destinations[2];
	fields[2].destination_size = sizeof(destinations[2]);
	fields[3].param_name = fake->names[3];
	fields[3].destination = destinations[3];
	fields[3].destination_size = sizeof(destinations[3]);
}

static void
test_transactional_snapshot_copies_modem_ras_credentials_before_cache_overwrite(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];

	init_fake_snapshot(&fake, fields, destinations);
	fake.names[0] = "modem_type";
	fake.names[1] = "modem_user";
	fake.names[2] = "modem_pass";
	fake.names[3] = "modem_isp";
	fake.values[0] = "3";
	fake.values[1] = "alice";
	fake.values[2] = "s3cret";
	fake.values[3] = "Carrier-A";
	fields[0].param_name = fake.names[0];
	fields[1].param_name = fake.names[1];
	fields[2].param_name = fake.names[2];
	fields[3].param_name = fake.names[3];

	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 4);
	assert(fake.end_calls == 1);
	assert(strcmp(destinations[0], "3") == 0);
	assert(strcmp(destinations[1], "alice") == 0);
	assert(strcmp(destinations[2], "s3cret") == 0);
	assert(strcmp(destinations[3], "Carrier-A") == 0);
}


#define DHCP_RENEW_SNAPSHOT_TEST_COUNT 14

static void
test_transactional_dhcp_renew_snapshot_copies_complete_state(void)
{
	static const char *const names[DHCP_RENEW_SNAPSHOT_TEST_COUNT] = {
		"wan0_err", "wan0_route_err", "wan0_mtu_err",
		"wan0_netmask", "wan0_gateway", "wan0_ipaddr",
		"wan0_routes", "wan0_routes_ms", "wan0_routes_rfc",
		"wan0_mtu", "wan0_dns", "wan0_domain", "wan0_wins", "wan0_lease"
	};
	static const char *const values[DHCP_RENEW_SNAPSHOT_TEST_COUNT] = {
		"0", "0", "0", "255.255.255.0", "198.51.100.1", "192.0.2.2",
		"198.51.100.0/24", "", "203.0.113.0/24", "1492",
		"1.1.1.1 8.8.8.8", "example.test", "192.0.2.10", "3600"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[DHCP_RENEW_SNAPSHOT_TEST_COUNT];
	char destinations[DHCP_RENEW_SNAPSHOT_TEST_COUNT][64];
	unsigned int i, fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < DHCP_RENEW_SNAPSHOT_TEST_COUNT; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}

	assert(rc_wan_nvram_copy_snapshot(
		fields, DHCP_RENEW_SNAPSHOT_TEST_COUNT, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == DHCP_RENEW_SNAPSHOT_TEST_COUNT);
	assert(fake.end_calls == 1);
	for (i = 0; i < DHCP_RENEW_SNAPSHOT_TEST_COUNT; ++i)
		assert(strcmp(destinations[i], values[i]) == 0);

	/* A failure at any tuple position invalidates every copied field. */
	for (fail_at = 0; fail_at < DHCP_RENEW_SNAPSHOT_TEST_COUNT; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)fail_at;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < DHCP_RENEW_SNAPSHOT_TEST_COUNT; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			snprintf(destinations[i], sizeof(destinations[i]), "stale");
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
		}
		assert(rc_wan_nvram_copy_snapshot(
			fields, DHCP_RENEW_SNAPSHOT_TEST_COUNT, fake_snapshot_begin,
			fake_snapshot_get, fake_snapshot_end, &fake) == EACCES);
		assert(fake.begin_calls == 1);
		assert(fake.read_calls == fail_at + 1);
		assert(fake.end_calls == 1);
		for (i = 0; i < DHCP_RENEW_SNAPSHOT_TEST_COUNT; ++i)
			assert(destinations[i][0] == '\0');
	}
}


static void
test_transactional_iptv_overlap_lan_subnet_snapshot(void)
{
	static const char *const names[2] = {
		"lan_ipaddr", "lan_netmask"
	};
	static const char *const values[2] = {
		"192.168.33.1", "255.255.255.0"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[2];
	char destinations[2][64];
	unsigned int i, fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 2; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(
		fields, 2, fake_snapshot_begin, fake_snapshot_get,
		fake_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 2 &&
	       fake.end_calls == 1);
	assert(strcmp(destinations[0], "192.168.33.1") == 0);
	assert(strcmp(destinations[1], "255.255.255.0") == 0);
	/* The callback copies cache-backed text before the next transaction read. */
	assert(strcmp(fake.cache_slot, "255.255.255.0") == 0);

	/* A failure at either member discards the complete subnet tuple. */
	for (fail_at = 0; fail_at < 2; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)fail_at;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 2; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
			strcpy(destinations[i], "stale");
		}
		assert(rc_wan_nvram_copy_snapshot(
			fields, 2, fake_snapshot_begin, fake_snapshot_get,
			fake_snapshot_end, &fake) == EACCES);
		assert(fake.begin_calls == 1);
		assert(fake.read_calls == fail_at + 1);
		assert(fake.end_calls == 1);
		assert(destinations[0][0] == '\0');
		assert(destinations[1][0] == '\0');
	}

	/* A transaction-close error invalidates both successful reads too. */
	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	fake.end_error = EIO;
	for (i = 0; i < 2; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
		strcpy(destinations[i], "stale");
	}
	assert(rc_wan_nvram_copy_snapshot(
		fields, 2, fake_snapshot_begin, fake_snapshot_get,
		fake_snapshot_end, &fake) == EIO);
	assert(fake.begin_calls == 1 && fake.read_calls == 2 &&
	       fake.end_calls == 1);
	assert(destinations[0][0] == '\0');
	assert(destinations[1][0] == '\0');
}


static void
test_transactional_snapshot_copies_coherent_tuple(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];

	init_fake_snapshot(&fake, fields, destinations);
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 4);
	assert(fake.end_calls == 1);
	assert(strcmp(destinations[0], "198.51.100.1") == 0);
	assert(strcmp(destinations[1], "192.0.2.2") == 0);
	assert(strcmp(destinations[2], "255.255.255.0") == 0);
	assert(strcmp(destinations[3], "1.1.1.1 8.8.8.8") == 0);
}

static void
test_transactional_wan_route_snapshot_reads_one_generation(void)
{
	static const char *const names[] = {
		"wan0_gateway", "wan0_ipaddr", "wan0_netmask"
	};
	static const char *const values[] = {
		"198.51.100.1", "192.0.2.2", "255.255.255.0"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char destinations[3][64];
	unsigned int i, fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	for (i = 0; i < 3; ++i)
		assert(strcmp(destinations[i], values[i]) == 0);

	/* Any read failure invalidates the entire ownership tuple. */
	for (fail_at = 0; fail_at < 3; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)fail_at;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 3; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			strcpy(destinations[i], "stale");
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
		}
		assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
						  fake_snapshot_get, fake_snapshot_end,
						  &fake) == EACCES);
		assert(fake.begin_calls == 1);
		assert(fake.read_calls == fail_at + 1);
		assert(fake.end_calls == 1);
		for (i = 0; i < 3; ++i)
			assert(destinations[i][0] == '\0');
	}

	/* A failed transaction close also invalidates the complete route tuple. */
	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	fake.end_error = EIO;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		strcpy(destinations[i], "stale");
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EIO);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	for (i = 0; i < 3; ++i)
		assert(destinations[i][0] == '\0');
}


static void
test_transactional_dhcp_route_options_snapshot(void)
{
	static const char *const names[] = {
		"wan0_routes", "wan0_routes_rfc", "wan0_routes_ms"
	};
	static const char *const values[] = {
		"10.0.0.0/8 192.0.2.1",
		"172.16.0.0/16 192.0.2.1",
		"198.18.0.0/15 192.0.2.1"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char destinations[3][128];
	unsigned int i, fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	assert(strcmp(fake.cache_slot, values[2]) == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(destinations[i], values[i]) == 0);

	/* A missing optional route format stays empty without corrupting peers. */
	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = 1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
		strcpy(destinations[i], "stale");
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(strcmp(destinations[0], values[0]) == 0);
	assert(destinations[1][0] == '\0');
	assert(strcmp(destinations[2], values[2]) == 0);

	/* Any failed key or transaction close invalidates the entire tuple. */
	for (fail_at = 0; fail_at < 3; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)fail_at;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 3; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
			strcpy(destinations[i], "stale");
		}
		assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
						  fake_snapshot_get, fake_snapshot_end,
						  &fake) == EACCES);
		for (i = 0; i < 3; ++i)
			assert(destinations[i][0] == '\0');
	}

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	fake.end_error = EIO;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
		strcpy(destinations[i], "stale");
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EIO);
	for (i = 0; i < 3; ++i)
		assert(destinations[i][0] == '\0');
}


static void
test_transactional_snapshot_treats_missing_key_as_empty(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];

	init_fake_snapshot(&fake, fields, destinations);
	fake.missing_at = 0;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 4);
	assert(fake.end_calls == 1);
	assert(destinations[0][0] == '\0');
	assert(strcmp(destinations[1], "192.0.2.2") == 0);
}

static void
test_transactional_snapshot_releases_lock_and_discards_partial_tuple(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];
	unsigned int i;

	init_fake_snapshot(&fake, fields, destinations);
	fake.read_fail_at = 2;
	fake.read_error = EACCES;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EACCES);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	for (i = 0; i < 4; ++i)
		assert(destinations[i][0] == '\0');
}

static void
test_transactional_snapshot_rejects_oversized_field_and_releases_lock(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];
	unsigned int i;

	init_fake_snapshot(&fake, fields, destinations);
	fields[3].destination_size = 8;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == ENAMETOOLONG);
	assert(fake.read_calls == 4);
	assert(fake.begin_calls == 1);
	assert(fake.end_calls == 1);
	for (i = 0; i < 4; ++i)
		assert(destinations[i][0] == '\0');
}

static void
test_transactional_snapshot_begin_failure_does_not_read_or_end(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];
	unsigned int i;

	init_fake_snapshot(&fake, fields, destinations);
	fake.begin_error = EBUSY;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EBUSY);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 0);
	assert(fake.end_calls == 0);
	for (i = 0; i < 4; ++i)
		assert(destinations[i][0] == '\0');
}

static void
test_transactional_snapshot_end_failure_invalidates_values(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];
	unsigned int i;

	init_fake_snapshot(&fake, fields, destinations);
	fake.end_error = EIO;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EIO);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 4);
	assert(fake.end_calls == 1);
	for (i = 0; i < 4; ++i)
		assert(destinations[i][0] == '\0');
}

static void
test_transactional_snapshot_validates_before_opening_lock(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[4];
	char destinations[4][64];

	init_fake_snapshot(&fake, fields, destinations);
	fields[1].destination_size = 0;
	assert(rc_wan_nvram_copy_snapshot(fields, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EINVAL);
	assert(fake.begin_calls == 0);
	assert(fake.read_calls == 0);
	assert(fake.end_calls == 0);
	assert(rc_wan_nvram_copy_snapshot(NULL, 4, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == EINVAL);
}

static void
test_lan_boot_runtime_snapshot_stops_at_each_failed_write(void)
{
	static const struct rc_wan_nvram_value values[] = {
		{ "lan_ipaddr_t", "192.168.1.1" },
		{ "lan_netmask_t", "255.255.255.0" },
		{ "lan_gateway_t", "" },
		{ "lan_domain_t", "" },
		{ "lan_dns_t", "" }
	};
	struct fake_value_copy fake;
	unsigned int fail_at, i;
	const unsigned int count =
		(unsigned int)(sizeof(values) / sizeof(values[0]));

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_set_required_values(
		values, count, fake_set_value, &fake) == 0);
	assert(fake.calls == count);
	for (i = 0; i < count; ++i) {
		assert(strcmp(fake.copied_names[i], values[i].param_name) == 0);
		assert(strcmp(fake.copied_values[i], values[i].value) == 0);
	}

	for (fail_at = 0; fail_at < count; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = (int)fail_at;
		fake.failure_status = ENOSPC;
		assert(rc_wan_nvram_set_required_values(
			values, count, fake_set_value, &fake) == ENOSPC);
		assert(fake.calls == fail_at + 1);
		for (i = 0; i <= fail_at; ++i) {
			assert(strcmp(fake.copied_names[i], values[i].param_name) == 0);
			assert(strcmp(fake.copied_values[i], values[i].value) == 0);
		}
		if (fail_at + 1 < count)
			assert(fake.copied_names[fail_at + 1][0] == '\0');
	}
}


static void
test_ipv6_if_ppp_status_preserves_missing_default_and_errors(void)
{
	int is_ppp = 77;

	assert(rc_wan_nvram_ipv6_if_ppp_status(0, 0, NULL, &is_ppp) == 0);
	assert(is_ppp == 1); /* Missing key keeps the legacy zero/PPP default. */
	is_ppp = 77;
	assert(rc_wan_nvram_ipv6_if_ppp_status(1, 0, "0", &is_ppp) == 0);
	assert(is_ppp == 1);
	is_ppp = 77;
	assert(rc_wan_nvram_ipv6_if_ppp_status(1, 0, "1", &is_ppp) == 0);
	assert(is_ppp == 0);
	is_ppp = 77;
	assert(rc_wan_nvram_ipv6_if_ppp_status(-1, EACCES, NULL, &is_ppp) == EACCES);
	assert(is_ppp == -1);
	is_ppp = 77;
	assert(rc_wan_nvram_ipv6_if_ppp_status(-1, 0, NULL, &is_ppp) == EIO);
	assert(is_ppp == -1);
	is_ppp = 77;
	assert(rc_wan_nvram_ipv6_if_ppp_status(1, 0, "invalid", &is_ppp) == EINVAL);
	assert(is_ppp == -1);
	assert(rc_wan_nvram_ipv6_if_ppp_status(1, 0, "1", NULL) == EINVAL);
}

static void
test_checked_integer_read_preserves_missing_default(void)
{
	int value = 91;

	assert(rc_wan_nvram_read_int_status(0, 0, NULL, 0, &value) == 0);
	assert(value == 0);
	value = 91;
	assert(rc_wan_nvram_read_int_status(0, 0, NULL, 1, &value) == 0);
	assert(value == 1);
}

static void
test_checked_integer_read_accepts_complete_decimal_values(void)
{
	int value = 0;

	assert(rc_wan_nvram_read_int_status(1, 0, " \t+42\r\n", 0, &value) == 0);
	assert(value == 42);
	assert(rc_wan_nvram_read_int_status(1, 0, "-17", 0, &value) == 0);
	assert(value == -17);
	assert(rc_wan_nvram_read_int_status(1, 0, "", 0, &value) == 0);
	assert(value == 0);
	assert(rc_wan_nvram_read_int_status(1, 0, " \t\r\n", 0, &value) == 0);
	assert(value == 0);
	assert(rc_wan_nvram_read_int_status(1, 0, "2147483647", 0, &value) == 0);
	assert(value == INT_MAX);
	assert(rc_wan_nvram_read_int_status(1, 0, "-2147483648", 0, &value) == 0);
	assert(value == INT_MIN);
}

static void
test_checked_integer_read_rejects_malformed_and_overflow_values(void)
{
	int value = 73;

	assert(rc_wan_nvram_read_int_status(1, 0, "42junk", 0, &value) == EINVAL);
	assert(value == 73);
	assert(rc_wan_nvram_read_int_status(1, 0, "1 2", 0, &value) == EINVAL);
	assert(value == 73);
	assert(rc_wan_nvram_read_int_status(1, 0, "+", 0, &value) == EINVAL);
	assert(value == 73);
	assert(rc_wan_nvram_read_int_status(1, 0, "2147483648", 0, &value) == ERANGE);
	assert(value == 73);
	assert(rc_wan_nvram_read_int_status(1, 0, "-2147483649", 0, &value) == ERANGE);
	assert(value == 73);
	assert(rc_wan_nvram_read_int_status(1, 0, "99999999999999999999999", 0, &value) == ERANGE);
	assert(value == 73);
}

static void
test_checked_integer_read_propagates_backend_errors(void)
{
	int value = 19;

	assert(rc_wan_nvram_read_int_status(-1, EACCES, NULL, 0, &value) == EACCES);
	assert(value == 19);
	assert(rc_wan_nvram_read_int_status(-1, 0, NULL, 0, &value) == EIO);
	assert(value == 19);
	assert(rc_wan_nvram_read_int_status(1, 0, NULL, 0, &value) == EIO);
	assert(value == 19);
	assert(rc_wan_nvram_read_int_status(2, 0, "5", 0, &value) == EIO);
	assert(value == 19);
	assert(rc_wan_nvram_read_int_status(1, 0, "5", 0, NULL) == EINVAL);
}

static void
test_dhcp6_integer_snapshot_is_transactional_and_all_or_nothing(void)
{
	static const char *const names[3] = {
		"ip6_wan_dhcp", "ip6_dns_auto", "ip6_lan_auto"
	};
	static const char *const stored[3] = { "1", "0", "1" };
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char destinations[3][32];
	const char *texts[3];
	int values[3];
	unsigned int i, fail_at;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = stored[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
		texts[i] = destinations[i];
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 3 && fake.end_calls == 1);
	assert(rc_wan_nvram_parse_int_snapshot(texts, 3, values) == 0);
	assert(values[0] == 1 && values[1] == 0 && values[2] == 1);

	/* An absent legacy key keeps nvram_get_int()'s zero default. */
	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = 1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = stored[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
		strcpy(destinations[i], "stale");
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
					  fake_snapshot_get, fake_snapshot_end,
					  &fake) == 0);
	assert(destinations[1][0] == '\0');
	for (i = 0; i < 3; ++i)
		texts[i] = destinations[i];
	assert(rc_wan_nvram_parse_int_snapshot(texts, 3, values) == 0);
	assert(values[0] == 1 && values[1] == 0 && values[2] == 1);

	/* Any backend failure invalidates the complete snapshot. */
	for (fail_at = 0; fail_at < 3; ++fail_at) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)fail_at;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 3; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = stored[i];
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
			strcpy(destinations[i], "stale");
		}
		assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
						  fake_snapshot_get, fake_snapshot_end,
						  &fake) == EACCES);
		assert(fake.end_calls == 1);
		for (i = 0; i < 3; ++i)
			assert(destinations[i][0] == '\0');
	}

	/* Malformed later members cannot leave earlier decoded flags visible. */
	texts[0] = "1";
	texts[1] = "1junk";
	texts[2] = "0";
	values[0] = values[1] = values[2] = 91;
	assert(rc_wan_nvram_parse_int_snapshot(texts, 3, values) == EINVAL);
	assert(values[0] == 0 && values[1] == 0 && values[2] == 0);
	texts[1] = "2147483648";
	values[0] = values[1] = values[2] = 91;
	assert(rc_wan_nvram_parse_int_snapshot(texts, 3, values) == ERANGE);
	assert(values[0] == 0 && values[1] == 0 && values[2] == 0);
}



static void
test_wan6_sit_6rd_parameters_share_one_transaction(void)
{
	static const char *const names[2] = {
		"wan0_6rd_size", "wan0_6rd_relay"
	};
	static const char *const values[2] = { "32", "192.0.2.1" };
	struct fake_nvram_transaction fake;
	int size4 = 99;
	char relay4[64] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = values[0];
	fake.values[1] = values[1];
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &size4, relay4, sizeof(relay4),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 2 &&
	       fake.end_calls == 1);
	assert(size4 == 32 && strcmp(relay4, "192.0.2.1") == 0);
}

static void
test_wan6_sit_6rd_snapshot_discards_both_values_on_read_errors(void)
{
	static const char *const names[2] = {
		"wan0_6rd_size", "wan0_6rd_relay"
	};
	static const char *const values[2] = { "24", "198.51.100.1" };
	struct fake_nvram_transaction fake;
	int size4;
	char relay4[64];
	unsigned int failed;

	for (failed = 0; failed < 2; ++failed) {
		memset(&fake, 0, sizeof(fake));
		fake.names[0] = names[0];
		fake.names[1] = names[1];
		fake.values[0] = values[0];
		fake.values[1] = values[1];
		fake.read_fail_at = (int)failed;
		fake.missing_at = -1;
		fake.read_error = EACCES;
		size4 = 99;
		strcpy(relay4, "stale");
		assert(rc_wan_nvram_copy_int_text_snapshot(
			names[0], names[1], &size4, relay4, sizeof(relay4),
			fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
			&fake) == EACCES);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
		assert(size4 == 0 && relay4[0] == '\0');
	}

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = values[0];
	fake.values[1] = values[1];
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	fake.end_error = EIO;
	size4 = 99;
	strcpy(relay4, "stale");
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &size4, relay4, sizeof(relay4),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == EIO);
	assert(fake.begin_calls == 1 && fake.read_calls == 2 &&
	       fake.end_calls == 1);
	assert(size4 == 0 && relay4[0] == '\0');
}

static void
test_wan6_sit_6rd_snapshot_preserves_missing_and_rejects_bad_size(void)
{
	static const char *const names[2] = {
		"wan0_6rd_size", "wan0_6rd_relay"
	};
	struct fake_nvram_transaction fake;
	int size4 = 99;
	char relay4[64] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = "unused";
	fake.values[1] = "192.0.2.9";
	fake.read_fail_at = -1;
	fake.missing_at = 0;
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &size4, relay4, sizeof(relay4),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == 0);
	assert(size4 == 0 && strcmp(relay4, "192.0.2.9") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = "32trailing";
	fake.values[1] = "192.0.2.9";
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	size4 = 99;
	strcpy(relay4, "stale");
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &size4, relay4, sizeof(relay4),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == EINVAL);
	assert(size4 == 0 && relay4[0] == '\0');
}

static void
test_wan6_unit_address_snapshot_copies_one_coherent_tuple(void)
{
	static const char *const names[3] = {
		"wan2_ipaddr", "wan2_gateway", "wan2_addr6"
	};
	static const char *const values[3] = {
		"198.51.100.20", "198.51.100.1", "2001:db8:20::1/64"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char destinations[3][64];
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 3; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 3 && fake.end_calls == 1);
	for (i = 0; i < 3; ++i)
		assert(strcmp(destinations[i], values[i]) == 0);
}

static void
test_wan6_unit_address_snapshot_discards_partial_tuple_on_each_read_error(void)
{
	static const char *const names[3] = {
		"wan2_ipaddr", "wan2_gateway", "wan2_addr6"
	};
	static const char *const values[3] = {
		"198.51.100.20", "198.51.100.1", "2001:db8:20::1/64"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char destinations[3][64];
	unsigned int i, failed;

	for (failed = 0; failed < 3; ++failed) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)failed;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 3; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			strcpy(destinations[i], "stale");
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
		}
		assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_snapshot_begin,
			fake_snapshot_get, fake_snapshot_end, &fake) == EACCES);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
		for (i = 0; i < 3; ++i)
			assert(destinations[i][0] == '\0');
	}
}

static void
test_wan6_reset_snapshot_copies_static_identity_and_6rd_settings(void)
{
	static const char *const names[5] = {
		"ip6_wan_addr", "ip6_wan_size", "ip6_wan_gate",
		"ip6_6rd_relay", "ip6_6rd_size"
	};
	static const char *const values[5] = {
		"2001:db8::10", "64", "2001:db8::1", "192.0.2.1", "32"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[5];
	char destinations[5][64];
	const char *size_texts[1];
	int parsed_sizes[1];
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 5; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 5, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 5 && fake.end_calls == 1);
	for (i = 0; i < 5; ++i)
		assert(strcmp(destinations[i], values[i]) == 0);
	size_texts[0] = destinations[1];
	assert(rc_wan_nvram_parse_int_snapshot(size_texts, 1, parsed_sizes) == 0);
	assert(parsed_sizes[0] == 64);
}

static void
test_wan6_reset_snapshot_clears_every_field_on_each_read_failure(void)
{
	static const char *const names[5] = {
		"ip6_wan_addr", "ip6_wan_size", "ip6_wan_gate",
		"ip6_6rd_relay", "ip6_6rd_size"
	};
	static const char *const values[5] = {
		"2001:db8::10", "64", "2001:db8::1", "192.0.2.1", "32"
	};
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[5];
	char destinations[5][64];
	unsigned int i, failed;

	for (failed = 0; failed < 5; ++failed) {
		memset(&fake, 0, sizeof(fake));
		fake.read_fail_at = (int)failed;
		fake.read_error = EACCES;
		fake.missing_at = -1;
		for (i = 0; i < 5; ++i) {
			fake.names[i] = names[i];
			fake.values[i] = values[i];
			strcpy(destinations[i], "stale");
			fields[i].param_name = names[i];
			fields[i].destination = destinations[i];
			fields[i].destination_size = sizeof(destinations[i]);
		}
		assert(rc_wan_nvram_copy_snapshot(fields, 5, fake_snapshot_begin,
			fake_snapshot_get, fake_snapshot_end, &fake) == EACCES);
		assert(fake.end_calls == 1);
		for (i = 0; i < 5; ++i)
			assert(destinations[i][0] == '\0');
	}
}

static void
test_wan6_reset_snapshot_reads_only_6rd_fields_in_dynamic_mode(void)
{
	static const char *const names[2] = { "ip6_6rd_relay", "ip6_6rd_size" };
	static const char *const values[2] = { "192.0.2.1", "32" };
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field fields[2];
	char destinations[2][64];
	unsigned int i;

	memset(&fake, 0, sizeof(fake));
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	for (i = 0; i < 2; ++i) {
		fake.names[i] = names[i];
		fake.values[i] = values[i];
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = sizeof(destinations[i]);
	}
	assert(rc_wan_nvram_copy_snapshot(fields, 2, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == 0);
	assert(fake.read_calls == 2 && fake.begin_calls == 1 && fake.end_calls == 1);
	assert(strcmp(destinations[0], values[0]) == 0);
	assert(strcmp(destinations[1], values[1]) == 0);
}


static void
test_dhcp6c_hwaddr_snapshot_keeps_missing_mac_as_empty(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field field;
	char hardware_address[32] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan_hwaddr";
	fake.values[0] = "02:11:22:33:44:55";
	fake.read_fail_at = -1;
	fake.missing_at = 0;
	field.param_name = "wan_hwaddr";
	field.destination = hardware_address;
	field.destination_size = sizeof(hardware_address);
	assert(rc_wan_nvram_copy_snapshot(&field, 1, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 1 && fake.end_calls == 1);
	assert(hardware_address[0] == '\0');
}

static void
test_dhcp6c_hwaddr_snapshot_fails_closed_on_backend_error(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field field;
	char hardware_address[32] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan_hwaddr";
	fake.values[0] = "02:11:22:33:44:55";
	fake.read_fail_at = 0;
	fake.read_error = EACCES;
	fake.missing_at = -1;
	field.param_name = "wan_hwaddr";
	field.destination = hardware_address;
	field.destination_size = sizeof(hardware_address);
	assert(rc_wan_nvram_copy_snapshot(&field, 1, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == EACCES);
	assert(fake.end_calls == 1 && hardware_address[0] == '\0');
}

static void
test_dhcp6c_hwaddr_snapshot_fails_closed_on_transaction_close_error(void)
{
	struct fake_nvram_transaction fake;
	struct rc_wan_nvram_snapshot_field field;
	char hardware_address[32] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan_hwaddr";
	fake.values[0] = "02:11:22:33:44:55";
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	fake.end_error = EIO;
	field.param_name = "wan_hwaddr";
	field.destination = hardware_address;
	field.destination_size = sizeof(hardware_address);
	assert(rc_wan_nvram_copy_snapshot(&field, 1, fake_snapshot_begin,
		fake_snapshot_get, fake_snapshot_end, &fake) == EIO);
	assert(fake.end_calls == 1 && hardware_address[0] == '\0');
}

static void
test_internet_loss_action_propagates_nvram_read_errors(void)
{
	int action = 99;

	assert(rc_wan_internet_loss_action_status(EIO, 2, &action) == EIO);
	assert(action == 0);
	assert(rc_wan_internet_loss_action_status(EINVAL, 3, &action) == EINVAL);
	assert(action == 0);
	assert(rc_wan_internet_loss_action_status(0, 0, &action) == 0);
	assert(action == 0);
	assert(rc_wan_internet_loss_action_status(0, 2, &action) == 0);
	assert(action == 2);
	assert(rc_wan_internet_loss_action_status(0, 3, &action) == 0);
	assert(action == 3);
	/* Match nvram_safe_get_int(..., 0, 0, 3): out-of-range defaults to 0. */
	assert(rc_wan_internet_loss_action_status(0, -1, &action) == 0);
	assert(action == 0);
	assert(rc_wan_internet_loss_action_status(0, 4, &action) == 0);
	assert(action == 0);
	assert(rc_wan_internet_loss_action_status(0, 1, NULL) == EINVAL);
}
static void
test_vpnc_dns_policy_and_servers_share_one_transaction(void)
{
	static const char *const names[2] = { "vpnc_pdns", "vpnc_dns_t" };
	static const char *const values[2] = { "2", "10.8.0.1 10.8.0.2" };
	struct fake_nvram_transaction fake;
	int pdns = 0;
	char servers[512] = {0};

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = values[0];
	fake.values[1] = values[1];
	fake.read_fail_at = -1;
	fake.missing_at = -1;
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &pdns, servers, sizeof(servers),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 2 &&
	       fake.end_calls == 1);
	assert(pdns == 2 && strcmp(servers, values[1]) == 0);
}

static void
test_vpnc_dns_snapshot_backend_errors_clear_policy_and_servers(void)
{
	static const char *const names[2] = { "vpnc_pdns", "vpnc_dns_t" };
	static const char *const values[2] = { "2", "10.8.0.1" };
	struct fake_nvram_transaction fake;
	int failed;

	for (failed = 0; failed < 2; ++failed) {
		int pdns = 2;
		char servers[64] = "stale-vpn-dns";

		memset(&fake, 0, sizeof(fake));
		fake.names[0] = names[0];
		fake.names[1] = names[1];
		fake.values[0] = values[0];
		fake.values[1] = values[1];
		fake.read_fail_at = failed;
		fake.missing_at = -1;
		fake.read_error = EACCES;
		assert(rc_wan_nvram_copy_int_text_snapshot(
			names[0], names[1], &pdns, servers, sizeof(servers),
			fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
			&fake) == EACCES);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
		assert(pdns == 0 && servers[0] == '\0');
	}
}

static void
test_vpnc_dns_missing_server_keeps_legacy_empty_fallback_signal(void)
{
	static const char *const names[2] = { "vpnc_pdns", "vpnc_dns_t" };
	struct fake_nvram_transaction fake;
	int pdns = 0;
	char servers[64] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = names[0];
	fake.names[1] = names[1];
	fake.values[0] = "2";
	fake.values[1] = "ignored";
	fake.read_fail_at = -1;
	fake.missing_at = 1;
	assert(rc_wan_nvram_copy_int_text_snapshot(
		names[0], names[1], &pdns, servers, sizeof(servers),
		fake_snapshot_begin, fake_snapshot_get, fake_snapshot_end,
		&fake) == 0);
	assert(pdns == 2 && servers[0] == '\0');
}
int
main(void)
{
	test_vpnc_dns_policy_and_servers_share_one_transaction();
	test_vpnc_dns_snapshot_backend_errors_clear_policy_and_servers();
	test_vpnc_dns_missing_server_keeps_legacy_empty_fallback_signal();
	test_internet_loss_action_propagates_nvram_read_errors();
	test_successful_required_param_sequence();
	test_required_param_sequence_stops_at_first_error();
	test_invalid_required_param_arguments_fail_closed();
	test_checked_read_accepts_present_and_absent_values();
	test_checked_read_preserves_backend_error();
	test_checked_read_without_errno_falls_back_to_eio();
	test_checked_read_rejects_unexpected_result();
	test_modem_transport_selection_fails_closed_on_read_error();
	test_checked_integer_range_preserves_valid_values_and_legacy_fallback();
	test_checked_integer_range_preserves_read_errors_and_output();
	test_checked_integer_read_preserves_missing_default();
	test_ipv6_if_ppp_status_preserves_missing_default_and_errors();
	test_checked_integer_read_accepts_complete_decimal_values();
	test_checked_integer_read_rejects_malformed_and_overflow_values();
	test_checked_integer_read_propagates_backend_errors();
	test_dhcp6_integer_snapshot_is_transactional_and_all_or_nothing();
	test_checked_string_match_requires_an_exact_value();
	test_checked_string_match_preserves_backend_errors();
	test_dhcp6_6rd_selector_preserves_missing_value_and_backend_errors();
	test_checked_string_match_rejects_invalid_arguments();
	test_wisp_active_status_requires_checked_identity();
	test_success_ignores_stale_errno();
	test_failure_preserves_errno();
	test_failure_without_errno_falls_back_to_eio();
	test_unexpected_nonzero_result_is_not_success();
	test_required_value_sequence_success();
	test_required_value_sequence_stops_at_first_error();
	test_wan0_boot_values_stop_at_each_nvram_failure();
	test_lan_boot_runtime_snapshot_stops_at_each_failed_write();
	test_wan0_default_copy_preserves_empty_existing_override();
	test_wan0_default_copy_uses_checked_source_value();
	test_wan0_default_copy_uses_factory_fallback_when_source_is_absent();
	test_wan0_default_copy_stops_on_target_read_error();
	test_wan0_default_copy_stops_on_source_read_error();
	test_wan0_default_copy_propagates_write_error();
	test_wan0_default_copy_rejects_invalid_arguments();
	test_required_value_sequence_rejects_invalid_values();
	test_runtime_identity_publication_checks_both_writes();
	test_wan6_reset_config_preserves_identity_when_required_write_fails();
	test_wan6_reset_config_clears_identity_only_after_all_values_succeed();
	test_all_value_sequence_attempts_every_write_and_preserves_first_error();
	test_wan_statistics_clear_attempts_every_counter();
	test_wan_deconfig_statistics_clear_attempts_every_counter();
	test_vpn_runtime_state_clear_attempts_every_key_and_preserves_first_error();
	test_all_value_sequence_validates_before_writing();
	test_checked_nvram_copy_survives_shared_cache_overwrite();
	test_wan_interface_names_survive_shared_cache_overwrite();
	test_viptv_settings_survive_shared_cache_overwrite();
	test_dhcp_hostname_and_vendor_class_survive_shared_cache_overwrite();
	test_lan_subnet_identity_survives_shared_cache_overwrite();
	test_dhcp_fallback_netmask_survives_previous_tuple_reads();
	test_gateway_arp_identity_snapshots_survive_cache_reuse();
	test_bounded_text_copy_survives_source_invalidation_and_rejects_truncation();
	test_legacy_mac_override_preserves_mac_conv_formatting();
	test_legacy_mac_override_zero_and_broadcast_values_use_fallback();
	test_legacy_mac_override_rejects_invalid_destination();

	test_checked_nvram_copy_treats_missing_key_as_empty();
	test_checked_nvram_copy_preserves_backend_failure();
	test_checked_nvram_copy_rejects_truncation();
	test_checked_nvram_copy_rejects_invalid_arguments();
	test_transactional_dhcp_renew_snapshot_copies_complete_state();
	test_transactional_iptv_overlap_lan_subnet_snapshot();
	test_transactional_wan_route_snapshot_reads_one_generation();
	test_transactional_dhcp_route_options_snapshot();
	test_transactional_snapshot_copies_coherent_tuple();
	test_transactional_snapshot_copies_modem_ras_credentials_before_cache_overwrite();
	test_transactional_snapshot_treats_missing_key_as_empty();
	test_transactional_snapshot_releases_lock_and_discards_partial_tuple();
	test_transactional_snapshot_rejects_oversized_field_and_releases_lock();
	test_transactional_snapshot_begin_failure_does_not_read_or_end();
	test_transactional_snapshot_end_failure_invalidates_values();
	test_transactional_snapshot_validates_before_opening_lock();
	test_wan6_sit_6rd_parameters_share_one_transaction();
	test_wan6_sit_6rd_snapshot_discards_both_values_on_read_errors();
	test_wan6_sit_6rd_snapshot_preserves_missing_and_rejects_bad_size();
	test_wan6_unit_address_snapshot_copies_one_coherent_tuple();
	test_wan6_unit_address_snapshot_discards_partial_tuple_on_each_read_error();
	test_wan6_reset_snapshot_copies_static_identity_and_6rd_settings();
	test_wan6_reset_snapshot_clears_every_field_on_each_read_failure();
	test_wan6_reset_snapshot_reads_only_6rd_fields_in_dynamic_mode();
	test_dhcp6c_hwaddr_snapshot_keeps_missing_mac_as_empty();
	test_dhcp6c_hwaddr_snapshot_fails_closed_on_backend_error();
	test_dhcp6c_hwaddr_snapshot_fails_closed_on_transaction_close_error();
	return 0;
}
