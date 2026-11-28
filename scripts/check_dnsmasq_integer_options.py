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
assert "snapshot->arp_enabled != 0" in static_host_snapshot
assert "snapshot->arp_enabled != 1" in static_host_snapshot
assert "test_invalid_arp_enable_flag_fails_closed" in STATIC_HOST_NVRAM_TEST
assert "test_invalid_arp_enable_flag_fails_closed();" in STATIC_HOST_NVRAM_TEST
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
    assert token in TEST
assert "net_lan6.c" in WORKFLOW

split_dns_snapshot = function_body(POLICY, "dnsmasq_split_dns_nvram_snapshot_read(")
for token in (
    'fields[0].param_name = "vpnc_pdns"',
    'fields[1].param_name = "vpnc_dom_t"',
    'fields[2].param_name = "vpnc_dns_t"',
    'fields[3].param_name = "wanx_domain"',
    'fields[4].param_name = "wanx_dns"',
    "rc_wan_nvram_copy_snapshot(",
    "rc_wan_nvram_parse_int_snapshot(",
    "if (status != 0) {\n\t\tmemset(snapshot, 0, sizeof(*snapshot));\n\t\treturn status;\n\t}",
    "memset(snapshot, 0, sizeof(*snapshot))",
    "vpn_private_dns > 0",
):
    assert token in split_dns_snapshot, token

split_dns_publish = function_body(SERVICES, "fill_dnsmasq_servers(void)")
assert "dnsmasq_split_dns_nvram_snapshot_read(" in split_dns_publish
assert split_dns_publish.index('file_lock("dservers")') < split_dns_publish.index(
    "dnsmasq_split_dns_nvram_snapshot_read("
) < split_dns_publish.index("dnsmasq_file_open(servers_path)")
for key in ("vpnc_pdns", "vpnc_dom_t", "vpnc_dns_t", "wanx_domain", "wanx_dns"):
    assert f'nvram_safe_get("{key}")' not in split_dns_publish, key
assert 'nvram_get_int("vpnc_pdns")' not in split_dns_publish
for token in (
    "nvram_snapshot.vpn_private_dns_enabled",
    "nvram_snapshot.vpn_domain",
    "nvram_snapshot.vpn_dns_servers",
    "nvram_snapshot.man_domain",
    "nvram_snapshot.man_dns_servers",
    "file_unlock(lock);",
    "preserving existing server file",
):
    assert token in split_dns_publish, token
split_dns_zero_helper = function_body(TEST, "dnsmasq_split_dns_snapshot_is_zero(")
assert "dnsmasq_split_dns_field_is_zero(" in split_dns_zero_helper
assert "assert(dnsmasq_split_dns_snapshot_is_zero(&snapshot));" in TEST

for marker in (
    "test_split_dns_snapshot_copies_one_generation",
    "test_split_dns_snapshot_preserves_missing_and_nonpositive_defaults",
    "test_split_dns_snapshot_read_failure_at_every_key_invalidates_tuple",
    "test_split_dns_snapshot_begin_close_and_parse_errors_fail_closed",
    "test_split_dns_snapshot_rejects_oversized_domain_and_server_lists",
):
    assert marker in TEST and marker + "();" in TEST, marker

print("DNSMasq checked DHCP integer options, bounded MAC validation and fail-closed static-host NVRAM snapshots: PASS")

# Atomic DNSMasq publication must persist the staged inode and both rename
# directory entries; otherwise power loss can lose a just-published config.
dnsmasq_file_helper = (ROOT / "trunk/user/rc/src/dnsmasq_file.h").read_text()
dnsmasq_file_test = (ROOT / "trunk/user/rc/src/tests/dnsmasq_file.c").read_text()
sync_directory = function_body(
    dnsmasq_file_helper, "dnsmasq_file_sync_directory(const char *directory)")
sync_parent = function_body(
    dnsmasq_file_helper, "dnsmasq_file_sync_parent(const char *path)")
publish = function_body(
    dnsmasq_file_helper, "dnsmasq_file_publish_internal(FILE *fp, char *path")
assert "O_DIRECTORY | O_NOFOLLOW" in sync_directory
assert sync_directory.index("DNSMASQ_FILE_FSYNC(fd)") < sync_directory.index("close(fd)")
assert "strrchr(parent, '/')" in sync_parent
assert "dnsmasq_file_sync_directory(parent)" in sync_parent
assert publish.index("fflush(fp)") < publish.index("fchmod(fileno(fp), mode)")
assert publish.index("fchmod(fileno(fp), mode)") < publish.index("DNSMASQ_FILE_FSYNC(fileno(fp))")
assert publish.index("DNSMASQ_FILE_FSYNC(fileno(fp))") < publish.rindex("if (fclose(fp) != 0)")
assert publish.index("fclose(fp)") < publish.index("rename(path, destination)")
assert publish.index("rename(path, destination)") < publish.index(
    "dnsmasq_file_sync_parent(destination)")
assert publish.index("dnsmasq_file_sync_parent(destination)") < publish.index(
    "dnsmasq_file_sync_parent(path)")
assert "test_dnsmasq_file_directory_sync_helpers" in dnsmasq_file_test
assert dnsmasq_file_test.index("test_dnsmasq_file_directory_sync_helpers(directory)") > dnsmasq_file_test.index(
    "assert(mkdtemp(directory) != NULL)")
print("DNSMasq staged publication file and directory durability contract: PASS")

# Rollback hard links must be stable before their destinations are replaced,
# and restoring them must persist the rename/unlink before reporting success.
backup_create = function_body(
    dnsmasq_file_helper, "dnsmasq_file_backup_create(const char *destination")
backup_restore = function_body(
    dnsmasq_file_helper, "dnsmasq_file_backup_restore(const char *destination")
assert backup_create.index("link(destination, backup_path)") < backup_create.index(
    "dnsmasq_file_sync_parent(destination)")
assert backup_create.index("dnsmasq_file_sync_parent(destination)") < backup_create.index(
    "dnsmasq_file_sync_parent(backup_path)")
assert backup_create.index("dnsmasq_file_sync_parent(backup_path)") < backup_create.index(
    "return 1")
assert backup_restore.index("rename(backup_path, destination)") < backup_restore.index(
    "dnsmasq_file_sync_parent(destination)")
assert backup_restore.index("dnsmasq_file_sync_parent(destination)") < backup_restore.index(
    "dnsmasq_file_sync_parent(backup_path)")
assert backup_restore.index("dnsmasq_file_sync_parent(backup_path)") < backup_restore.index(
    "backup_path[0] = '\\0'")
assert backup_restore.index("unlink(destination)") < backup_restore.rindex(
    "dnsmasq_file_sync_parent(destination)")
print("DNSMasq rollback snapshot and restore durability contract: PASS")
