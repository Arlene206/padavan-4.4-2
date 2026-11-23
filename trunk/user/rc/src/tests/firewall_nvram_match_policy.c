#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "firewall_nvram_match_policy.h"

static void
test_present_values_match_exactly(void)
{
	int matches = -1;
	assert(firewall_nvram_match_status(1, 0, "1", "1", 0, &matches) == 0);
	assert(matches == 1);
	assert(firewall_nvram_match_status(1, 0, "0", "1", 0, &matches) == 0);
	assert(matches == 0);
}

static void
test_missing_key_uses_explicit_legacy_default(void)
{
	int matches = -1;
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 1, &matches) == 0);
	assert(matches == 1);
}

static void
test_backend_errors_preserve_status_and_output(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0, &matches) == EACCES);
	assert(matches == 23);
	assert(firewall_nvram_match_status(-1, 0, NULL, "1", 0, &matches) == EIO);
	assert(matches == 23);
}

static void
test_invalid_api_results_fail_closed(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(2, 0, "1", "1", 0, &matches) == EIO);
	assert(matches == 23);
	assert(firewall_nvram_match_status(1, 0, NULL, "1", 0, &matches) == EIO);
	assert(matches == 23);
	assert(firewall_nvram_match_status(0, 0, "unexpected", "1", 0, &matches) == EIO);
	assert(matches == 23);
}

static void
test_invalid_arguments_are_rejected(void)
{
	int matches = 23;
	assert(firewall_nvram_match_status(1, 0, "1", NULL, 0, &matches) == EINVAL);
	assert(firewall_nvram_match_status(1, 0, "1", "1", 0, NULL) == EINVAL);
	assert(matches == 23);
}

static void
test_not_zero_reader_preserves_invmatch_defaults(void)
{
    int enabled = -1;

    assert(firewall_nvram_not_zero_status(1, 0, "0", 1, &enabled) == 0);
    assert(enabled == 0);
    assert(firewall_nvram_not_zero_status(1, 0, "1", 1, &enabled) == 0);
    assert(enabled == 1);
    assert(firewall_nvram_not_zero_status(1, 0, "unexpected", 1, &enabled) == 0);
    assert(enabled == 1);
    assert(firewall_nvram_not_zero_status(0, 0, NULL, 1, &enabled) == 0);
    assert(enabled == 1);
    assert(firewall_nvram_not_zero_status(0, 0, NULL, 0, &enabled) == 0);
    assert(enabled == 0);
}

static void
test_not_zero_backend_failure_preserves_output(void)
{
    int enabled = 17;

    assert(firewall_nvram_not_zero_status(-1, EACCES, NULL, 1, &enabled) == EACCES);
    assert(enabled == 17);
    assert(firewall_nvram_not_zero_status(-1, 0, NULL, 1, &enabled) == EIO);
    assert(enabled == 17);
    assert(firewall_nvram_not_zero_status(2, 0, "0", 1, &enabled) == EIO);
    assert(enabled == 17);
    assert(firewall_nvram_not_zero_status(1, 0, NULL, 1, &enabled) == EIO);
    assert(enabled == 17);
}

static void
test_integer_modes_preserve_decimal_semantics(void)
{
	int number = -9;
	assert(firewall_nvram_int_status(1, 0, "0", 0, &number) == 0);
	assert(number == 0);
	assert(firewall_nvram_int_status(1, 0, "1", 0, &number) == 0);
	assert(number == 1);
	assert(firewall_nvram_int_status(1, 0, "2", 0, &number) == 0);
	assert(number == 2);
	assert(firewall_nvram_int_status(1, 0, "-1", 0, &number) == 0);
	assert(number == -1);
	assert(firewall_nvram_int_status(1, 0, "2147483647", 0, &number) == 0);
	assert(number == INT_MAX);
	assert(firewall_nvram_int_status(0, 0, NULL, 0, &number) == 0);
	assert(number == 0);
}

static void
test_integer_errors_are_fail_closed_and_preserve_output(void)
{
	int number = 41;
	assert(firewall_nvram_int_status(-1, EACCES, NULL, 0, &number) == EACCES);
	assert(number == 41);
	assert(firewall_nvram_int_status(-1, 0, NULL, 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(2, 0, "1", 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, NULL, 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "", 0, &number) == EIO);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "2junk", 0, &number) == EINVAL);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "2147483648", 0, &number) == ERANGE);
	assert(number == 41);
	assert(firewall_nvram_int_status(1, 0, "1", 0, NULL) == EINVAL);
	assert(number == 41);
}

static void
test_sshd_bruteforce_mode_missing_default_and_backend_error(void)
{
	int mode = 73;

	/* Keep the legacy mode-0 default for an absent NVRAM key. */
	assert(firewall_nvram_int_status(0, 0, NULL, 0, &mode) == 0);
	assert(mode == 0);

	mode = 73;
	assert(firewall_nvram_int_status(1, 0, "2", 0, &mode) == 0);
	assert(mode == 2);

	mode = 73;
	assert(firewall_nvram_int_status(-1, EACCES, NULL, 0, &mode) == EACCES);
	assert(mode == 73);
}

static void
test_wan_ttl_policy_reads_keep_legacy_defaults_and_fail_closed(void)
{
	int ttl_fix = 19;
	int ttl_value = 99;
	int multicast_enabled = 7;

	/* Missing settings keep their historical zero/disabled defaults. */
	assert(firewall_nvram_int_status(0, 0, NULL, 0, &ttl_fix) == 0);
	assert(ttl_fix == 0);
	assert(firewall_nvram_int_status(0, 0, NULL, 0, &ttl_value) == 0);
	assert(ttl_value == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0,
					   &multicast_enabled) == 0);
	assert(multicast_enabled == 0);

	ttl_fix = 19;
	ttl_value = 99;
	multicast_enabled = 7;
	assert(firewall_nvram_int_status(-1, EIO, NULL, 0, &ttl_fix) == EIO);
	assert(ttl_fix == 19);
	assert(firewall_nvram_int_status(-1, EACCES, NULL, 0, &ttl_value) == EACCES);
	assert(ttl_value == 99);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0,
					   &multicast_enabled) == EIO);
	assert(multicast_enabled == 7);
}

static void
test_dns_redirect_toggle_missing_default_and_backend_failure(void)
{
	int enabled = 42;

	/* An absent redirect toggle remains disabled, as before. */
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &enabled) == 0);
	assert(enabled == 0);

	enabled = 42;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0,
					   &enabled) == EIO);
	assert(enabled == 42);
}

static void
test_rp_filter_feature_switches_keep_legacy_defaults_and_fail_closed(void)
{
	int multicast_enabled = 23;
	int udpxy_disabled = 23;
	int xupnpd_disabled = 23;

	/* Missing multicast is false; inverse-match services remain not-zero. */
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &multicast_enabled) == 0);
	assert(multicast_enabled == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "0", 0, &udpxy_disabled) == 0);
	assert(udpxy_disabled == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "0", 0, &xupnpd_disabled) == 0);
	assert(xupnpd_disabled == 0);

	multicast_enabled = 23;
	udpxy_disabled = 23;
	xupnpd_disabled = 23;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &multicast_enabled) == EIO);
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "0", 0, &udpxy_disabled) == EACCES);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "0", 0, &xupnpd_disabled) == EIO);
	assert(multicast_enabled == 23);
	assert(udpxy_disabled == 23);
	assert(xupnpd_disabled == 23);
}

static void
test_nat_remote_listener_toggles_missing_default_and_backend_failure(void)
{
	int misc_http = 19;
	int https = 19;
	int ssh = 19;
	int ftp = 19;
	int udpxy = 19;

	/* All five exact-match listener gates historically default to disabled. */
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &misc_http) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &https) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &ssh) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &ftp) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &udpxy) == 0);
	assert(misc_http == 0 && https == 0 && ssh == 0 && ftp == 0 && udpxy == 0);

	misc_http = https = ssh = ftp = udpxy = 19;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &misc_http) == EIO);
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0, &https) == EACCES);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &ssh) == EIO);
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0, &ftp) == EACCES);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &udpxy) == EIO);
	assert(misc_http == 19 && https == 19 && ssh == 19 && ftp == 19 && udpxy == 19);
}

static void
test_filter_multicast_toggle_missing_default_and_backend_failure(void)
{
	int enabled = 31;

	/* Exact-match multicast policy remains disabled when its key is absent. */
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &enabled) == 0);
	assert(enabled == 0);

	enabled = 31;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &enabled) == EIO);
	assert(enabled == 31);
}

static void
test_firewall_log_mode_values_missing_default_and_backend_error(void)
{
	int accept = 21, drop = 21, both = 21;

	assert(firewall_nvram_match_status(1, 0, "accept", "accept", 0, &accept) == 0);
	assert(firewall_nvram_match_status(1, 0, "accept", "drop", 0, &drop) == 0);
	assert(firewall_nvram_match_status(1, 0, "accept", "both", 0, &both) == 0);
	assert(accept == 1 && drop == 0 && both == 0);
	accept = drop = both = 21;
	assert(firewall_nvram_match_status(1, 0, "both", "accept", 0, &accept) == 0);
	assert(firewall_nvram_match_status(1, 0, "both", "drop", 0, &drop) == 0);
	assert(firewall_nvram_match_status(1, 0, "both", "both", 0, &both) == 0);
	assert(accept == 0 && drop == 0 && both == 1);
	accept = drop = both = 21;
	assert(firewall_nvram_match_status(0, 0, NULL, "accept", 0, &accept) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "drop", 0, &drop) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "both", 0, &both) == 0);
	assert(accept == 0 && drop == 0 && both == 0);
	accept = drop = both = 21;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "accept", 0, &accept) == EIO);
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "drop", 0, &drop) == EACCES);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "both", 0, &both) == EIO);
	assert(accept == 21 && drop == 21 && both == 21);
}

static void
test_nat_auxiliary_policy_toggles_missing_default_and_backend_failure(void)
{
	int battlenet = 17;
	int fullcone = 17;
	int nat_loop = 17;

	/* BattleNET mapping, fullcone NAT and hairpin SNAT are disabled by default. */
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &battlenet) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &fullcone) == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &nat_loop) == 0);
	assert(battlenet == 0 && fullcone == 0 && nat_loop == 0);

	battlenet = fullcone = nat_loop = 17;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &battlenet) == EIO);
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0, &fullcone) == EACCES);
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &nat_loop) == EIO);
	assert(battlenet == 17 && fullcone == 17 && nat_loop == 17);
}

static void
test_pppoe_mwan_hook_toggle_missing_default_and_backend_failure(void)
{
	int enabled = 27;
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0, &enabled) == 0);
	assert(enabled == 0);
	enabled = 27;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "1", 0, &enabled) == EIO);
	assert(enabled == 27);
}

static void
test_nvram_read_status_distinguishes_missing_and_backend_error(void)
{
    assert(firewall_nvram_read_status(1, 0, "UDP") == 0);
    assert(firewall_nvram_read_status(0, 0, NULL) == 0);
    assert(firewall_nvram_read_status(-1, EACCES, NULL) == EACCES);
    assert(firewall_nvram_read_status(-1, 0, NULL) == EIO);
    assert(firewall_nvram_read_status(2, 0, "TCP") == EIO);
    assert(firewall_nvram_read_status(1, 0, NULL) == EIO);
    assert(firewall_nvram_read_status(0, 0, "unexpected") == EIO);
}


static void
test_checked_string_resolver_preserves_missing_default(void)
{
    const char *resolved = "stale";

    assert(firewall_nvram_string_value_status(0, 0, NULL,
                                              &resolved) == 0);
    assert(resolved != NULL);
    assert(strcmp(resolved, "") == 0);

    assert(firewall_nvram_string_value_status(1, 0, "192.0.2.17",
                                              &resolved) == 0);
    assert(strcmp(resolved, "192.0.2.17") == 0);
}

static void
test_checked_string_resolver_rejects_backend_and_malformed_results(void)
{
    const char *resolved = "stale";

    assert(firewall_nvram_string_value_status(-1, EACCES, NULL,
                                              &resolved) == EACCES);
    assert(resolved == NULL);
    assert(firewall_nvram_string_value_status(-1, 0, NULL,
                                              &resolved) == EIO);
    assert(resolved == NULL);
    assert(firewall_nvram_string_value_status(2, 0, "192.0.2.17",
                                              &resolved) == EIO);
    assert(resolved == NULL);
    assert(firewall_nvram_string_value_status(0, 0, "unexpected",
                                              &resolved) == EIO);
    assert(resolved == NULL);
}

static void
test_checked_string_copy_preserves_present_and_missing_values(void)
{
    char value[16] = "stale";

    assert(firewall_nvram_copy_string_status(1, 0, "192.0.2.1",
                                             value, sizeof(value)) == 0);
    assert(strcmp(value, "192.0.2.1") == 0);
    assert(firewall_nvram_copy_string_status(0, 0, NULL,
                                             value, sizeof(value)) == 0);
    assert(value[0] == '\0');
}

static void
test_checked_string_copy_fails_closed_on_backend_and_invalid_results(void)
{
    char value[16] = "stale";

    assert(firewall_nvram_copy_string_status(-1, EACCES, NULL,
                                             value, sizeof(value)) == EACCES);
    assert(value[0] == '\0');
    assert(firewall_nvram_copy_string_status(-1, 0, NULL,
                                             value, sizeof(value)) == EIO);
    assert(value[0] == '\0');
    assert(firewall_nvram_copy_string_status(2, 0, "192.0.2.1",
                                             value, sizeof(value)) == EIO);
    assert(value[0] == '\0');
    assert(firewall_nvram_copy_string_status(1, 0, NULL,
                                             value, sizeof(value)) == EIO);
    assert(value[0] == '\0');
    assert(firewall_nvram_copy_string_status(0, 0, "unexpected",
                                             value, sizeof(value)) == EIO);
    assert(value[0] == '\0');
}


static void
test_viptv_interface_snapshot_fails_closed_without_changing_legacy_absence(void)
{
	char interface_name[16] = "stale";

	/* A missing optional key still selects the legacy MAN-interface fallback. */
	assert(firewall_nvram_copy_string_status(0, 0, NULL,
						 interface_name, sizeof(interface_name)) == 0);
	assert(interface_name[0] == '\0');

	assert(firewall_nvram_copy_string_status(1, 0, "eth2",
						 interface_name, sizeof(interface_name)) == 0);
	assert(strcmp(interface_name, "eth2") == 0);

	/* Backend/close failures and values too long for an ifname must not be
	 * mistaken for an absent override. */
	assert(firewall_nvram_copy_string_status(-1, EIO, NULL,
						 interface_name, sizeof(interface_name)) == EIO);
	assert(interface_name[0] == '\0');
	assert(firewall_nvram_copy_string_status(1, 0, "1234567890123456",
						 interface_name, sizeof(interface_name)) == ENAMETOOLONG);
	assert(interface_name[0] == '\0');
}


static void
test_vpns_vnet_snapshot_preserves_missing_and_rejects_read_errors(void)
{
	char subnet[16] = "stale";

	/* An absent optional pool setting keeps the legacy empty-string default. */
	assert(firewall_nvram_copy_string_status(0, 0, NULL,
						 subnet, sizeof(subnet)) == 0);
	assert(subnet[0] == '\0');

	assert(firewall_nvram_copy_string_status(1, 0, "10.8.0.0",
						 subnet, sizeof(subnet)) == 0);
	assert(strcmp(subnet, "10.8.0.0") == 0);

	/* Backend/close failures and values too long for an IPv4 literal fail closed. */
	assert(firewall_nvram_copy_string_status(-1, EIO, NULL,
						 subnet, sizeof(subnet)) == EIO);
	assert(subnet[0] == '\0');
	assert(firewall_nvram_copy_string_status(1, 0, "255.255.255.255x",
						 subnet, sizeof(subnet)) == ENAMETOOLONG);
	assert(subnet[0] == '\0');
}

static void
test_vpns_vnet_requires_strict_ipv4_address(void)
{
	assert(firewall_nvram_ipv4_address_is_valid("10.8.0.0"));
	assert(firewall_nvram_ipv4_address_is_valid("192.168.255.254"));
	assert(!firewall_nvram_ipv4_address_is_valid(NULL));
	assert(!firewall_nvram_ipv4_address_is_valid(""));
	assert(!firewall_nvram_ipv4_address_is_valid("10.8.0"));
	assert(!firewall_nvram_ipv4_address_is_valid("10.8.0.0/24"));
	assert(!firewall_nvram_ipv4_address_is_valid("10.8.0.256"));
	assert(!firewall_nvram_ipv4_address_is_valid("256.8.0.1"));
	assert(!firewall_nvram_ipv4_address_is_valid("010.8.0.1"));
	assert(!firewall_nvram_ipv4_address_is_valid("10.8.0.0x"));
}


static void
test_checked_string_copy_rejects_truncation_and_invalid_arguments(void)
{
    char value[8] = "stale";

    assert(firewall_nvram_copy_string_status(1, 0, "192.0.2.100",
                                             value, sizeof(value)) == ENAMETOOLONG);
    assert(value[0] == '\0');
    assert(firewall_nvram_copy_string_status(1, 0, "x", NULL, sizeof(value)) == EINVAL);
    assert(firewall_nvram_copy_string_status(1, 0, "x", value, 0) == EINVAL);
    assert(value[0] == '\0');
}

static void
test_vts_protocol_name_preserves_legacy_modes(void)
{
    assert(strcmp(firewall_vts_protocol_name("TCP"), "tcp") == 0);
    assert(strcmp(firewall_vts_protocol_name("udp"), "udp") == 0);
    assert(strcmp(firewall_vts_protocol_name("BoTh"), "both") == 0);
    assert(strcmp(firewall_vts_protocol_name("OtHeR"), "other") == 0);
    assert(strcmp(firewall_vts_protocol_name(""), "tcp") == 0);
    assert(strcmp(firewall_vts_protocol_name("invalid"), "tcp") == 0);
    assert(firewall_vts_protocol_name(NULL) == NULL);
}
static void
test_lightweight_filter_drop_mode_match_uses_legacy_defaults(void)
{
	int matches = 23;

	assert(firewall_nvram_match_status(1, 0, "DROP", "DROP", 0, &matches) == 0);
	assert(matches == 1);
	assert(firewall_nvram_match_status(1, 0, "ACCEPT", "DROP", 0, &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "DROP", 0, &matches) == 0);
	assert(matches == 0);
	matches = 23;
	assert(firewall_nvram_match_status(-1, EIO, NULL, "DROP", 0, &matches) == EIO);
	assert(matches == 23);
}

static void
test_optional_ipv4_copy_preserves_present_and_missing_values(void)
{
	char address[16] = "stale";

	assert(firewall_nvram_optional_ipv4_copy_status(
		0, 0, NULL, address, sizeof(address)) == 0);
	assert(address[0] == '\0');

	assert(firewall_nvram_optional_ipv4_copy_status(
		1, 0, "192.0.2.17", address, sizeof(address)) == 0);
	assert(strcmp(address, "192.0.2.17") == 0);
}

static void
test_optional_ipv4_copy_fails_closed_on_backend_and_invalid_results(void)
{
	char address[16] = "stale";

	assert(firewall_nvram_optional_ipv4_copy_status(
		-1, EACCES, NULL, address, sizeof(address)) == EACCES);
	assert(address[0] == '\0');
	assert(firewall_nvram_optional_ipv4_copy_status(
		-1, 0, NULL, address, sizeof(address)) == EIO);
	assert(address[0] == '\0');
	assert(firewall_nvram_optional_ipv4_copy_status(
		2, 0, "192.0.2.17", address, sizeof(address)) == EIO);
	assert(address[0] == '\0');
}

static void
test_optional_ipv4_copy_disables_oversized_values(void)
{
	char address[16] = "stale";

	assert(firewall_nvram_optional_ipv4_copy_status(
		1, 0, "255.255.255.255-extra", address, sizeof(address)) == 0);
	assert(address[0] == '\0');
	assert(firewall_nvram_optional_ipv4_copy_status(
		1, 0, "192.0.2.1", address, sizeof(address)) == 0);
	assert(strcmp(address, "192.0.2.1") == 0);
}

int
main(void)
{
	test_present_values_match_exactly();
	test_missing_key_uses_explicit_legacy_default();
	test_backend_errors_preserve_status_and_output();
	test_invalid_api_results_fail_closed();
	test_invalid_arguments_are_rejected();
	test_not_zero_reader_preserves_invmatch_defaults();
	test_not_zero_backend_failure_preserves_output();
	test_integer_modes_preserve_decimal_semantics();
	test_integer_errors_are_fail_closed_and_preserve_output();
	test_sshd_bruteforce_mode_missing_default_and_backend_error();
	test_wan_ttl_policy_reads_keep_legacy_defaults_and_fail_closed();
	test_dns_redirect_toggle_missing_default_and_backend_failure();
	test_rp_filter_feature_switches_keep_legacy_defaults_and_fail_closed();
	test_nat_remote_listener_toggles_missing_default_and_backend_failure();
	test_filter_multicast_toggle_missing_default_and_backend_failure();
	test_firewall_log_mode_values_missing_default_and_backend_error();
	test_nat_auxiliary_policy_toggles_missing_default_and_backend_failure();
	test_pppoe_mwan_hook_toggle_missing_default_and_backend_failure();
	test_nvram_read_status_distinguishes_missing_and_backend_error();
	test_checked_string_resolver_preserves_missing_default();
	test_checked_string_resolver_rejects_backend_and_malformed_results();
	test_optional_ipv4_copy_preserves_present_and_missing_values();
	test_optional_ipv4_copy_fails_closed_on_backend_and_invalid_results();
	test_optional_ipv4_copy_disables_oversized_values();
	test_checked_string_copy_preserves_present_and_missing_values();
	test_checked_string_copy_fails_closed_on_backend_and_invalid_results();
	test_viptv_interface_snapshot_fails_closed_without_changing_legacy_absence();
	test_vpns_vnet_snapshot_preserves_missing_and_rejects_read_errors();
	test_vpns_vnet_requires_strict_ipv4_address();
	test_checked_string_copy_rejects_truncation_and_invalid_arguments();
	test_vts_protocol_name_preserves_legacy_modes();
	test_lightweight_filter_drop_mode_match_uses_legacy_defaults();
	return 0;
}
