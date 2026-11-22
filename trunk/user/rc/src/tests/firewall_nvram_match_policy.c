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
	test_nvram_read_status_distinguishes_missing_and_backend_error();
	test_vts_protocol_name_preserves_legacy_modes();
	test_lightweight_filter_drop_mode_match_uses_legacy_defaults();
	return 0;
}
