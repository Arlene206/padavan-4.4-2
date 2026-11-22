#!/usr/bin/env python3
"""Pin checked DHCP integer snapshots before DNSMasq mutates generated files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SERVICES = (ROOT / "trunk/user/rc/src/services_ex.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/dnsmasq_nvram_option_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/dnsmasq-lifecycle.yml").read_text()
STATIC_HOST_NVRAM_POLICY = (ROOT / "trunk/user/rc/src/dnsmasq_static_host_nvram_policy.h").read_text()
STATIC_HOST_NVRAM_TEST = (ROOT / "trunk/user/rc/src/tests/dnsmasq_static_host_nvram_policy.c").read_text()
STATIC_HOST_POLICY = (ROOT / "trunk/user/rc/src/dnsmasq_static_host_policy.h").read_text()
STATIC_HOST_TEST = (ROOT / "trunk/user/rc/src/tests/dnsmasq_static_host_policy.c").read_text()



def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


reader = function_body(
    SERVICES, "dnsmasq_read_integer_range_option(const char *param_name")
assert "nvram_get_checked(param_name, &text)" in reader
assert "dnsmasq_nvram_integer_range_status(" in reader
assert "errno = status;" in reader

prepare = function_body(
    SERVICES, "prepare_dns_dhcpd(int is_ap_mode, int *service_enabled)")
compact = "".join(prepare.split())
assert 'dnsmasq_read_integer_range_option("dhcp_verbose",0,0,3,&i_verbose)' in compact
assert 'dnsmasq_read_integer_range_option("dhcp_lease",0,120,604800,&dhcp_lease)' in compact
assert 'nvram_get_int("dhcp_verbose")' not in prepare
assert 'nvram_get_int("dhcp_lease")' not in prepare
assert prepare.index('"dhcp_verbose"') < prepare.index("fill_static_ethers(")
assert prepare.index('"dhcp_lease"') < prepare.index("fill_static_ethers(")
assert "dhcp_lease);" in prepare


snapshot = function_body(POLICY, "dnsmasq_dhcp_config_nvram_snapshot_read(")
for token in (
    'fields[count].param_name = "lan_ipaddr"',
    'fields[count].param_name = "lan_netmask"',
    'fields[count].param_name = "lan_domain"',
    'fields[count].param_name = "dhcp_start"',
    'fields[count].param_name = "dhcp_end"',
    'fields[count].param_name = "dhcp_gateway_x"',
    'fields[count].param_name = "dhcp_dns1_x"',
    'fields[count].param_name = "dhcp_dns2_x"',
    'fields[count].param_name = "dhcp_dns3_x"',
    'fields[count].param_name = "dhcp_wins_x"',
    "rc_wan_nvram_copy_snapshot(",
    "if (status != 0)",
    "memset(snapshot, 0, sizeof(*snapshot))",
):
    assert token in snapshot, token

for marker in (
    "test_dhcp_snapshot_copies_rotating_cache_as_one_tuple",
    "test_dhcp_snapshot_skips_optional_keys_when_dhcp_disabled",
    "test_dhcp_snapshot_read_and_close_errors_invalidate_all_values",
    "test_dhcp_snapshot_begin_and_truncation_errors_fail_closed",
):
    assert marker in TEST and marker + "();" in TEST, marker

snapshot_call = prepare.index("dnsmasq_dhcp_config_nvram_snapshot_read(")
fill_static = prepare.index("fill_static_ethers(")
router_hosts = prepare.index("update_hosts_router(")
config_stage = prepare.index("dnsmasq_file_open(config_path)")
assert snapshot_call < fill_static < router_hosts < config_stage

for key in (
    "lan_ipaddr", "lan_netmask", "lan_domain", "dhcp_start", "dhcp_end",
    "dhcp_gateway_x", "dhcp_dns1_x", "dhcp_dns2_x", "dhcp_dns3_x", "dhcp_wins_x",
):
    assert 'nvram_safe_get("' + key + '")' not in prepare, key
for token in (
    "dhcp_nvram_snapshot.dhcp_gateway",
    "dhcp_nvram_snapshot.dhcp_dns1",
    "dhcp_nvram_snapshot.dhcp_dns2",
    "dhcp_nvram_snapshot.dhcp_dns3",
    "dhcp_nvram_snapshot.dhcp_wins",
):
    assert token in prepare, token
assert 'nvram_get_int("vlmcsd_enable")' not in prepare
assert prepare.index('"vlmcsd_enable"') < fill_static

for token in (
    "api_result == 0",
    "rc_wan_nvram_read_int_status(",
    "parsed < minimum || parsed > maximum",
    "return ERANGE;",
):
    assert token in POLICY, token

for token in (
    "test_checked_integer_range_options",
    '"120", 0, 120, 604800',
    '"604800", 0, 120, 604800',
    '"604801", 0, 120, 604800',
    'test_checked_string_option_and_output_preservation',
):
    assert token in TEST, token

assert "scripts/check_dnsmasq_integer_options.py" in WORKFLOW
assert "Verify DHCP integer snapshots precede generated-file changes" in WORKFLOW

mac_validator = STATIC_HOST_POLICY.split("dnsmasq_static_mac_is_valid(", 1)[1].split(
    "/* system() returns", 1
)[0]
assert "if (!mac || strlen(mac) != 12)" in mac_validator
assert mac_validator.index("strlen(mac) != 12") < mac_validator.index("for (i = 0; i < 12; i++)")
assert "char empty[1]" in STATIC_HOST_TEST
assert "char one_digit[2]" in STATIC_HOST_TEST
assert "char five_digits[6]" in STATIC_HOST_TEST
assert "char eleven_digits[12]" in STATIC_HOST_TEST
static_host_snapshot = function_body(
    STATIC_HOST_NVRAM_POLICY, "dnsmasq_static_host_nvram_snapshot_read(")
static_host_builder = function_body(
    SERVICES, "fill_static_ethers(const char *lan_ip, const char *lan_mask)")
assert "rc_wan_nvram_copy_value(" in static_host_snapshot
assert "rc_wan_nvram_parse_int_value(" in static_host_snapshot
assert "end_transaction(context)" in static_host_snapshot
assert "memset(snapshot, 0, sizeof(*snapshot))" in static_host_snapshot
assert "errno = status;" in static_host_snapshot
assert "item_count = DNSMASQ_STATIC_HOST_MAX" in static_host_snapshot
assert "dnsmasq_static_host_nvram_snapshot_read(" in static_host_builder
assert static_host_builder.index("dnsmasq_static_host_nvram_snapshot_read(") < static_host_builder.index("dnsmasq_file_open(")
assert "nvram_get_int(" not in static_host_builder
assert "nvram_safe_get(" not in static_host_builder
assert "nvram_snapshot.entries[j].mac" in static_host_builder
assert "nvram_snapshot.entries[j].ip" in static_host_builder
assert "nvram_snapshot.entries[j].hostname" in static_host_builder
for marker in (
    "test_success_copies_rotating_cache_values_atomically",
    "test_missing_keys_keep_disabled_and_empty_defaults",
    "test_read_error_invalidates_snapshot_and_closes_transaction",
    "test_end_and_begin_failures_are_fail_closed",
    "test_malformed_and_oversized_values_fail_closed",
    "test_item_count_is_bounded",
):
    assert marker in STATIC_HOST_NVRAM_TEST, marker
assert "trunk/user/rc/src/dnsmasq_static_host_nvram_policy.h" in WORKFLOW
assert "trunk/user/rc/src/tests/dnsmasq_static_host_nvram_policy.c" in WORKFLOW
assert "Compile and run checked static-host NVRAM snapshot regression" in WORKFLOW
assert "dnsmasq_dhcp6_nvram_snapshot_read" in POLICY
for token in (
    "test_dhcp6_snapshot_copies_stateful_values_and_parses_bounds",
    "test_dhcp6_snapshot_preserves_mode_defaults_and_skips_unused_keys",
    "test_dhcp6_snapshot_defaults_and_rejects_invalid_ranges",
    "test_dhcp6_snapshot_read_and_close_failures_clear_all_fields",
    "test_dhcp6_snapshot_rejects_invalid_mode_and_arguments",
):
    assert token in read("trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c")
assert "net_lan6.c" in WORKFLOW
print("DNSMasq checked DHCP integer options, bounded MAC validation and fail-closed static-host NVRAM snapshots: PASS")
