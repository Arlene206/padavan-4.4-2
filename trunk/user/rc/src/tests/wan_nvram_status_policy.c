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
	const char *names[4];
	const char *values[4];
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

	assert(index < 4);
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

int
main(void)
{
	test_successful_required_param_sequence();
	test_required_param_sequence_stops_at_first_error();
	test_invalid_required_param_arguments_fail_closed();
	test_checked_read_accepts_present_and_absent_values();
	test_checked_read_preserves_backend_error();
	test_checked_read_without_errno_falls_back_to_eio();
	test_checked_read_rejects_unexpected_result();
	test_modem_transport_selection_fails_closed_on_read_error();
	test_checked_integer_read_preserves_missing_default();
	test_checked_integer_read_accepts_complete_decimal_values();
	test_checked_integer_read_rejects_malformed_and_overflow_values();
	test_checked_integer_read_propagates_backend_errors();
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
	test_bounded_text_copy_survives_source_invalidation_and_rejects_truncation();
	test_checked_nvram_copy_treats_missing_key_as_empty();
	test_checked_nvram_copy_preserves_backend_failure();
	test_checked_nvram_copy_rejects_truncation();
	test_checked_nvram_copy_rejects_invalid_arguments();
	test_transactional_snapshot_copies_coherent_tuple();
	test_transactional_snapshot_copies_modem_ras_credentials_before_cache_overwrite();
	test_transactional_snapshot_treats_missing_key_as_empty();
	test_transactional_snapshot_releases_lock_and_discards_partial_tuple();
	test_transactional_snapshot_rejects_oversized_field_and_releases_lock();
	test_transactional_snapshot_begin_failure_does_not_read_or_end();
	test_transactional_snapshot_end_failure_invalidates_values();
	test_transactional_snapshot_validates_before_opening_lock();
	return 0;
}
