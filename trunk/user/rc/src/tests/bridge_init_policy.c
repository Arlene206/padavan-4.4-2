#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "bridge_init_policy.h"
#include "wan_nvram_status_policy.h"

struct fake_bridge_nvram_cache {
	char cache[32];
	unsigned int calls;
	int fail_at;
	int failure_status;
};

static int
fake_bridge_nvram_get(void *opaque, const char *param_name, char **value)
{
	struct fake_bridge_nvram_cache *fake =
		(struct fake_bridge_nvram_cache *)opaque;
	unsigned int index = fake->calls++;

	if ((int)index == fake->fail_at) {
		errno = fake->failure_status;
		*value = NULL;
		return -1;
	}

	if (strcmp(param_name, "lan_hwaddr") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s",
			 "02:11:22:33:44:55");
	else if (strcmp(param_name, "lan_ipaddr") == 0 ||
		 strcmp(param_name, "lan_ipaddr_t") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "192.0.2.10");
	else if (strcmp(param_name, "lan_netmask") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "255.255.255.0");
	else if (strcmp(param_name, "lan_gateway") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "192.0.2.1");
	else if (strcmp(param_name, "lan_gateway_t") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "192.0.2.1");
	else if (strcmp(param_name, "lan_dns1") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "8.8.8.8");
	else if (strcmp(param_name, "lan_dns2") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "1.1.1.1");
	else if (strcmp(param_name, "lan_domain") == 0 ||
		 strcmp(param_name, "lan_domain_t") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "router.example");
	else if (strcmp(param_name, "computer_name") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "invalid host");
	else if (strcmp(param_name, "productid") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "Padavan-Router");
	else if (strcmp(param_name, "udpxy_enable_x") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "4022");
	else if (strcmp(param_name, "udpxy_clients") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "100");
	else if (strcmp(param_name, "udpxy_renew_period") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "30");
	else if (strcmp(param_name, "xupnpd_udpxy") == 0)
		snprintf(fake->cache, sizeof(fake->cache), "%s", "1");
	else
		snprintf(fake->cache, sizeof(fake->cache), "%s",
			 "next-nvram-value");
	*value = fake->cache;
	return 1;
}

static void
test_stop_lan_ap_gateway_snapshot_survives_cache_reuse(void)
{
	struct fake_bridge_nvram_cache fake;
	char ap_gateway[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_gateway_t", ap_gateway,
				       sizeof(ap_gateway)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_ipaddr_t", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.cache, "192.0.2.10") == 0);
	assert(strcmp(ap_gateway, "192.0.2.1") == 0);
}

static void
test_stop_lan_router_address_read_failure_is_preserved(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_ip[16] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.failure_status = EACCES;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_ipaddr_t", lan_ip,
				       sizeof(lan_ip)) == EACCES);
	assert(fake.calls == 1);
	assert(lan_ip[0] == '\0');
}


struct bridge_wireless_reader_fixture {
	const char *fail_key;
	int failure_status;
	int calls;
	int values[4];
	const char *seen_keys[4];
};

static int
fake_bridge_wireless_reader(void *opaque, const char *name, int *value)
{
	static const char * const keys[4] = {
		"rt_radio_x", "rt_mode_x", "wl_radio_x", "wl_mode_x"
	};
	struct bridge_wireless_reader_fixture *fixture =
		(struct bridge_wireless_reader_fixture *)opaque;
	int call_index, index;

	if (!fixture || !name || !value)
		return EINVAL;
	call_index = fixture->calls++;
	if (call_index >= 4)
		return EOVERFLOW;
	fixture->seen_keys[call_index] = name;
	if (fixture->fail_key && strcmp(fixture->fail_key, name) == 0)
		return fixture->failure_status;
	for (index = 0; index < 4; ++index) {
		if (strcmp(keys[index], name) == 0) {
			*value = fixture->values[index];
			return 0;
		}
	}
	return EINVAL;
}

static void
test_bridge_wireless_selectors_are_snapshotted_before_startup(void)
{
	static const char * const expected_keys[4] = {
		"rt_radio_x", "rt_mode_x", "wl_radio_x", "wl_mode_x"
	};
	struct bridge_wireless_reader_fixture fixture;
	rc_bridge_wireless_state_t state;
	int index;

	memset(&fixture, 0, sizeof(fixture));
	fixture.values[0] = 1;
	fixture.values[1] = 3;
	fixture.values[2] = 1;
	fixture.values[3] = 0;
	assert(rc_bridge_wireless_state_read(
		       fake_bridge_wireless_reader, &fixture, 1, 1, &state) == 0);
	assert(fixture.calls == 4);
	for (index = 0; index < 4; ++index)
		assert(strcmp(fixture.seen_keys[index], expected_keys[index]) == 0);
	assert(state.rt_radio_on == 1 && state.rt_mode_x == 3);
	assert(state.wl_radio_on == 1 && state.wl_mode_x == 0);
}

static void
test_bridge_wireless_selector_failure_aborts_and_clears_snapshot(void)
{
	static const char * const failed_keys[4] = {
		"rt_radio_x", "rt_mode_x", "wl_radio_x", "wl_mode_x"
	};
	static const int failures[4] = { EACCES, EIO, ENODEV, EINVAL };
	struct bridge_wireless_reader_fixture fixture;
	rc_bridge_wireless_state_t state;
	int index, status;

	for (index = 0; index < 4; ++index) {
		memset(&fixture, 0, sizeof(fixture));
		fixture.values[0] = 1;
		fixture.values[1] = 3;
		fixture.values[2] = 1;
		fixture.values[3] = 3;
		fixture.fail_key = failed_keys[index];
		fixture.failure_status = failures[index];
		memset(&state, 0x7f, sizeof(state));
		status = rc_bridge_wireless_state_read(
			fake_bridge_wireless_reader, &fixture, 1, 1, &state);
		assert(status == failures[index]);
		assert(fixture.calls == index + 1);
		assert(state.rt_radio_on == 0 && state.rt_mode_x == 0);
		assert(state.wl_radio_on == 0 && state.wl_mode_x == 0);
	}

	/* Missing/empty values map to zero through the checked reader. */
	memset(&fixture, 0, sizeof(fixture));
	assert(rc_bridge_wireless_state_read(
		       fake_bridge_wireless_reader, &fixture, 1, 1, &state) == 0);
	assert(fixture.calls == 4);
	assert(state.rt_radio_on == 0 && state.rt_mode_x == 0);
	assert(state.wl_radio_on == 0 && state.wl_mode_x == 0);

	/* MII-only/single-band builds must not read unused selectors. */
	memset(&fixture, 0, sizeof(fixture));
	fixture.values[0] = 1;
	assert(rc_bridge_wireless_state_read(
		       fake_bridge_wireless_reader, &fixture, 0, 0, &state) == 0);
	assert(fixture.calls == 1);
	assert(strcmp(fixture.seen_keys[0], "rt_radio_x") == 0);
	assert(state.rt_radio_on == 1 && state.rt_mode_x == 0);
	assert(state.wl_radio_on == 0 && state.wl_mode_x == 0);
	assert(rc_bridge_wireless_state_read(
		       fake_bridge_wireless_reader, &fixture, 2, 0, &state) == EINVAL);
}

static void
test_bridge_lan_hwaddr_cache_copy_is_stable(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_hwaddr[24];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_hwaddr", lan_hwaddr,
				       sizeof(lan_hwaddr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "vlan_vid_cpu", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.cache, "next-nvram-value") == 0);
	assert(strcmp(lan_hwaddr, "02:11:22:33:44:55") == 0);
}

static void
test_bridge_lan_hwaddr_read_error_aborts_preflight(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_hwaddr[24] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.failure_status = EACCES;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_hwaddr", lan_hwaddr,
				       sizeof(lan_hwaddr)) == EACCES);
	assert(fake.calls == 1);
	assert(lan_hwaddr[0] == '\0');
}

static void
test_start_lan_nvram_snapshot_owns_all_values(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_ipaddr[16], lan_netmsk[16], lan_dname[256];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_ipaddr", lan_ipaddr,
				       sizeof(lan_ipaddr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_netmask", lan_netmsk,
				       sizeof(lan_netmsk)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_domain", lan_dname,
				       sizeof(lan_dname)) == 0);
	assert(strcmp(lan_ipaddr, "192.0.2.10") == 0);
	assert(strcmp(lan_netmsk, "255.255.255.0") == 0);
	assert(strcmp(lan_dname, "router.example") == 0);
	assert(strcmp(fake.cache, "router.example") == 0);
	assert(fake.calls == 3);
}

static void
test_update_hosts_ap_nvram_snapshot_owns_all_values(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_ipaddr[16], lan_dname[256];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_ipaddr_t", lan_ipaddr,
				       sizeof(lan_ipaddr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_domain_t", lan_dname,
				       sizeof(lan_dname)) == 0);
	assert(strcmp(lan_ipaddr, "192.0.2.10") == 0);
	assert(strcmp(lan_dname, "router.example") == 0);
	assert(fake.calls == 2);
}

static void
test_hostname_fallback_is_owned_across_cache_reuse(void)
{
	struct fake_bridge_nvram_cache fake;
	char host_name[256];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "computer_name", host_name,
				       sizeof(host_name)) == 0);
	assert(strchr(host_name, ' ') != NULL);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "productid", host_name,
				       sizeof(host_name)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "udpxy_enable_x", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.cache, "4022") == 0);
	assert(strcmp(host_name, "Padavan-Router") == 0);
}

static void
test_xupnpd_lan_address_is_owned_across_port_reads(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_addr[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_ipaddr_t", lan_addr,
				       sizeof(lan_addr)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "udpxy_enable_x", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "xupnpd_udpxy", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.cache, "1") == 0);
	assert(strcmp(lan_addr, "192.0.2.10") == 0);
}

static void
test_udpxy_launch_options_are_owned_across_reads(void)
{
	struct fake_bridge_nvram_cache fake;
	char udpxy_clients[16], udpxy_renew_period[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "udpxy_clients", udpxy_clients,
				       sizeof(udpxy_clients)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "udpxy_renew_period", udpxy_renew_period,
				       sizeof(udpxy_renew_period)) == 0);
	assert(strcmp(fake.cache, "30") == 0);
	assert(strcmp(udpxy_clients, "100") == 0);
	assert(strcmp(udpxy_renew_period, "30") == 0);
	assert(fake.calls == 2);
}

static void
test_manual_lan_gateway_snapshot_survives_dns_reads(void)
{
	struct fake_bridge_nvram_cache fake;
	char gateway_ip[16];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_gateway", gateway_ip,
				       sizeof(gateway_ip)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_dns1", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_dns2", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(strcmp(fake.cache, "1.1.1.1") == 0);
	assert(strcmp(gateway_ip, "192.0.2.1") == 0);
	assert(fake.calls == 3);
}

static void
test_router_hostname_domain_snapshot_survives_ipv6_reads(void)
{
	struct fake_bridge_nvram_cache fake;
	char lan_domain[256];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "lan_domain", lan_domain,
				       sizeof(lan_domain)) == 0);
	assert(rc_wan_nvram_copy_value(fake_bridge_nvram_get, &fake,
				       "ipv6_service", fake.cache,
				       sizeof(fake.cache)) == 0);
	assert(strcmp(fake.cache, "next-nvram-value") == 0);
	assert(strcmp(lan_domain, "router.example") == 0);
	assert(fake.calls == 2);
}

static void
test_empty_and_success_status(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	assert(rc_bridge_init_result(&status) == 0);
	rc_bridge_init_record(&status, "base", 0);
	assert(status.failure_count == 0);
	assert(status.first_operation == NULL);
}

static void
test_first_failure_and_later_operations(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "VLAN rule", EIO);
	rc_bridge_init_record(&status, "VLAN rule", 0);
	rc_bridge_init_record(&status, "switch bridge mode", ENODEV);
	rc_bridge_init_record(&status, "bridge IFUP", -1);
	assert(rc_bridge_init_result(&status) == EIO);
	assert(status.failure_count == 3);
	assert(strcmp(status.first_operation, "VLAN rule") == 0);
}

static void
test_negative_first_status_is_preserved(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "MAC address", -1);
	rc_bridge_init_record(&status, "bridge", EIO);
	assert(rc_bridge_init_result(&status) == -1);
	assert(status.failure_count == 2);
}

static void
test_radio_setup_failures_are_recorded_without_stopping_later_operations(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	rc_bridge_init_record(&status, "bring up 2.4GHz main interface", ENODEV);
	rc_bridge_init_record(&status, "bring up 5GHz main interface", EIO);
	rc_bridge_init_record(&status, "restore 5GHz main interface down", 0);
	assert(rc_bridge_init_result(&status) == ENODEV);
	assert(status.failure_count == 2);
	assert(strcmp(status.first_operation, "bring up 2.4GHz main interface") == 0);
}

static void
test_wireless_config_generation_failure_reaches_bridge_status(void)
{
	rc_bridge_init_status_t status;
	int operation_status;

	rc_bridge_init_status_reset(&status);
	operation_status = -1; /* gen_ralink_config_* failed to publish its file. */
	rc_bridge_init_record(&status,
		"prepare 2.4GHz wireless configuration", operation_status);

	/* init_bridge continues independent bridge setup after this failure. */
	operation_status = 0;
	rc_bridge_init_record(&status, "bring up LAN bridge", operation_status);
	assert(rc_bridge_init_result(&status) == -1);
	assert(status.failure_count == 1);
	assert(strcmp(status.first_operation,
		      "prepare 2.4GHz wireless configuration") == 0);
}

static void
test_reload_marker_write_failures_are_reported_and_aggregated(void)
{
	rc_bridge_init_status_t status;

	rc_bridge_init_status_reset(&status);
	/* A failed NVRAM marker write must not be silently lost. */
	rc_bridge_init_record(&status, "clear wireless service reload marker", -1);
	/* Continue the independent marker write and retain the failure count. */
	rc_bridge_init_record(&status, "clear router service reload marker", EIO);

	assert(rc_bridge_init_result(&status) == -1);
	assert(status.failure_count == 2);
	assert(strcmp(status.first_operation,
		      "clear wireless service reload marker") == 0);
}

static void
test_bridge_add_idempotency_requires_confirmed_bridge(void)
{
	assert(rc_bridge_init_add_bridge_result(0, 0, -1) == 0);
	/* Existing bridge with no members or at least one member is valid. */
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, 1) == 0);
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, 0) == 0);
	/* A failed/unknown bridge inventory must not hide a failed add. */
	assert(rc_bridge_init_add_bridge_result(-1, EEXIST, -1) == -1);
	assert(rc_bridge_init_add_bridge_result(-1, EACCES, 1) == -1);
	assert(rc_bridge_init_add_bridge_result(EINVAL, EEXIST, 0) == EINVAL);
}

int
main(void)
{
	test_bridge_wireless_selectors_are_snapshotted_before_startup();
	test_bridge_wireless_selector_failure_aborts_and_clears_snapshot();
	test_manual_lan_gateway_snapshot_survives_dns_reads();
	test_router_hostname_domain_snapshot_survives_ipv6_reads();
	test_udpxy_launch_options_are_owned_across_reads();
	test_hostname_fallback_is_owned_across_cache_reuse();
	test_xupnpd_lan_address_is_owned_across_port_reads();
	test_stop_lan_ap_gateway_snapshot_survives_cache_reuse();
	test_stop_lan_router_address_read_failure_is_preserved();
	test_bridge_lan_hwaddr_cache_copy_is_stable();
	test_start_lan_nvram_snapshot_owns_all_values();
	test_update_hosts_ap_nvram_snapshot_owns_all_values();
	test_bridge_lan_hwaddr_read_error_aborts_preflight();
	test_empty_and_success_status();
	test_first_failure_and_later_operations();
	test_negative_first_status_is_preserved();
	test_radio_setup_failures_are_recorded_without_stopping_later_operations();
	test_wireless_config_generation_failure_reaches_bridge_status();
	test_reload_marker_write_failures_are_reported_and_aggregated();
	test_bridge_add_idempotency_requires_confirmed_bridge();
	puts("RC bridge/LAN/hostname/IPTV NVRAM cache ownership policy: PASS");
	return 0;
}
