#!/usr/bin/env python3
"""Verify DNSMasq failed-start cleanup remains serialized and fail-closed."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
SERVICES = (RC / "services_ex.c").read_text()
POLICY = (RC / "dnsmasq_restart_policy.h").read_text()
TEST = (RC / "tests/dnsmasq_restart_policy.c").read_text()
RC_H = (RC / "rc.h").read_text()
LAN6 = (RC / "net_lan6.c").read_text()
DNSMASQ_OPTION_POLICY = (RC / "dnsmasq_nvram_option_policy.h").read_text()
DNSMASQ_OPTION_TEST = (RC / "tests/dnsmasq_nvram_option_policy.c").read_text()
DNSMASQ_WORKFLOW = (ROOT / ".github/workflows/dnsmasq-lifecycle.yml").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    position = opening
    while position < len(source):
        ch = source[position]
        nxt = source[position + 1] if position + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block"
                position += 1
            elif ch == "/" and nxt == "/":
                state = "line"
                position += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:position]
        elif state in ("string", "char"):
            if ch == "\\":
                position += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block" and ch == "*" and nxt == "/":
            state = "code"
            position += 1
        elif state == "line" and ch == "\n":
            state = "code"
        position += 1
    raise AssertionError("unterminated function: " + signature)


def compact(source: str) -> str:
    return " ".join(source.split())


rollback = compact(body(
    SERVICES,
    "static int\ndnsmasq_rollback_failed_start(int start_status, const char *operation)",
))
assert "start_errno = start_status > 0 ? start_status :" in rollback
assert "rollback_status = stop_dns_dhcpd_locked()" in rollback
assert "presence = dnsmasq_process_presence(&scan_error)" in rollback
assert "dnsmasq_failed_start_may_leave_daemon(presence)" in rollback
assert "errno = start_errno;" in rollback
assert rollback.index("stop_dns_dhcpd_locked()") < rollback.index(
    "dnsmasq_process_presence(&scan_error)"
) < rollback.index("dnsmasq_failed_start_may_leave_daemon")
assert rollback.index("dnsmasq_failed_start_may_leave_daemon") < rollback.index(
    "errno = start_errno;"
)

start = compact(body(SERVICES, "int\nstart_dns_dhcpd_locked(int is_ap_mode)"))
assert start.index("errno = 0;") < start.index('eval("/usr/sbin/dnsmasq")')
assert "status = dnsmasq_confirm_started(launch_status)" in start
assert "status = dnsmasq_rollback_failed_start(status, \"start\")" in start
assert start.index("dnsmasq_confirm_started(launch_status)") < start.index(
    "dnsmasq_rollback_failed_start(status, \"start\")"
)

restart = compact(body(SERVICES, "static int\nrestart_dhcpd_locked(void)"))
assert restart.index("stop_dns_dhcpd_locked()") < restart.index(
    'eval("/usr/sbin/dnsmasq")'
)
assert restart.index("errno = 0;") < restart.index('eval("/usr/sbin/dnsmasq")')
assert "start_status = dnsmasq_confirm_started(launch_status)" in restart
assert "start_status = dnsmasq_rollback_failed_start(start_status, \"restart\")" in restart
assert restart.index("dnsmasq_confirm_started(launch_status)") < restart.index(
    "dnsmasq_rollback_failed_start(start_status, \"restart\")"
)

for public, internal, operation in (
    ("start_dns_dhcpd(int is_ap_mode)", "start_dns_dhcpd_locked(is_ap_mode)", "start"),
    ("restart_dhcpd(void)", "restart_dhcpd_locked()", "restart"),
):
    entry = body(SERVICES, "int\n" + public)
    assert entry.index("dnsmasq_lifecycle_lock_acquire()") < entry.index(internal)
    assert entry.index(internal) < entry.index("dnsmasq_lifecycle_lock_finish(")
    assert f'"{operation}"' in entry

assert "dnsmasq_failed_start_may_leave_daemon(enum process_presence state)" in POLICY
assert "return state != PROCESS_PRESENCE_ABSENT;" in POLICY
assert "dnsmasq_failed_start_may_leave_daemon(PROCESS_PRESENCE_UNKNOWN)" in TEST
assert "dnsmasq_failed_start_may_leave_daemon((enum process_presence)7)" in TEST
assert "int start_dns_dhcpd_locked(int is_ap_mode);" in RC_H
assert "int stop_dns_dhcpd_locked(void);" in RC_H

# Security-sensitive resolver options must be checked before any replacement
# config or generated host input is staged.
prepare = compact(body(
    SERVICES,
    "static int\nprepare_dns_dhcpd(int is_ap_mode, int *service_enabled)",
))
option_reader = compact(body(
    SERVICES,
    "dnsmasq_read_boolean_option(const char *param_name, int *enabled)",
))
assert "nvram_get_checked(param_name, &value)" in option_reader
assert "dnsmasq_nvram_option_status(api_result, saved_errno, value, 0, enabled)" in option_reader
assert "preserving last-known-good configuration" in option_reader
option_names = (
    ("dhcp_filter_aaaa", "dhcp_filter_aaaa_enabled"),
    ("dhcp_all_servers", "dhcp_all_servers_enabled"),
    ("dhcp_strict_order", "dhcp_strict_order_enabled"),
    ("dhcp_stop_dns_rebind", "dhcp_stop_dns_rebind_enabled"),
    ("dhcp_proxy_dnssec", "dhcp_proxy_dnssec_enabled"),
)
gate_positions = []
for option_name, local_name in option_names:
    gate = 'dnsmasq_read_boolean_option( "' + option_name + '", &' + local_name + ')'
    assert gate in prepare, "missing checked option read: " + option_name
    gate_positions.append(prepare.index(gate))
assert gate_positions == sorted(gate_positions)
first_option = min(gate_positions)
for side_effect in (
    "dnsmasq_dhcp_range_store_nvram_pair(",
    "fill_static_ethers(ipaddr, netmask)",
    "update_hosts_ap()",
    "update_hosts_router(ipaddr)",
    "create_file(DNS_RESOLV_CONF)",
    "dnsmasq_file_open(config_path)",
):
    assert first_option < prepare.index(side_effect), side_effect
for option_name, local_name in option_names:
    assert 'nvram_match("' + option_name + '", "1")' not in prepare
    assert "if (!is_ap_mode && " + local_name + ")" in prepare
assert "api_result < 0" in DNSMASQ_OPTION_POLICY
assert "api_result != 1 || !value" in DNSMASQ_OPTION_POLICY
assert "test_backend_errors_preserve_errno_and_output" in DNSMASQ_OPTION_TEST
assert "test_missing_keys_use_explicit_legacy_defaults" in DNSMASQ_OPTION_TEST
for event in ("push", "pull_request"):
    event_section = DNSMASQ_WORKFLOW.split("  " + event + ":", 1)[1]
    end_marker = "  pull_request:" if event == "push" else "  workflow_dispatch:"
    event_paths = event_section.split(end_marker, 1)[0]
    assert event_paths.count('"trunk/user/rc/src/dnsmasq_nvram_option_policy.h"') == 1
    assert event_paths.count('"trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c"') == 1
assert "Compile and run DNSMasq checked option NVRAM regression" in DNSMASQ_WORKFLOW
print("DNSMasq resolver option NVRAM fail-closed contract: PASS")

# DHCP enablement is a checked decision, not a raw nvram_match/invmatch boolean.
invmatch_policy = compact(body(
    DNSMASQ_OPTION_POLICY,
    "dnsmasq_nvram_invmatch_status(",
))
assert "if (!matches || !match_value || (missing_matches != 0 && missing_matches != 1)) return EINVAL;" in invmatch_policy
assert "if (api_result < 0) return saved_errno > 0 ? saved_errno : EIO;" in invmatch_policy
assert "if (api_result == 0)" in invmatch_policy
assert "*matches = strcmp(value, match_value) != 0;" in invmatch_policy
invmatch_reader = compact(body(
    SERVICES,
    "dnsmasq_read_invmatch_option(const char *param_name, const char *match_value,",
))
assert "nvram_get_checked(param_name, &value)" in invmatch_reader
assert "dnsmasq_nvram_invmatch_status(api_result, saved_errno, value, match_value, missing_matches, matches)" in invmatch_reader
assert "preserving last-known-good configuration" in invmatch_reader

dhcp_enable = compact(body(
    SERVICES,
    "int\nis_dhcpd_enabled_status(int is_ap_mode, int *enabled)",
))
assert 'dnsmasq_read_boolean_option("dhcp_enable_x", enabled)' in dhcp_enable
assert 'dnsmasq_read_boolean_option("lan_dhcpd_x", &lan_dhcp_enabled)' in dhcp_enable
assert 'dnsmasq_read_invmatch_option("lan_proto_x", "1", 0, &lan_proto_not_dhcp)' in dhcp_enable
assert "if (!lan_dhcp_enabled) return 0;" in dhcp_enable
assert "*enabled = lan_proto_not_dhcp;" in dhcp_enable
legacy_dhcp_enable = compact(body(SERVICES, "int\nis_dhcpd_enabled(int is_ap_mode)"))
assert "is_dhcpd_enabled_status(is_ap_mode, &enabled)" in legacy_dhcp_enable
assert "nvram_match(" not in legacy_dhcp_enable and "nvram_invmatch(" not in legacy_dhcp_enable

assert "int is_dhcpd_enabled_status(int is_ap_mode, int *enabled);" in RC_H
assert "option_status = is_dhcpd_enabled_status(is_ap_mode, &i_dhcp_enable);" in prepare
assert "if (option_status != 0) return option_status;" in prepare
assert "is_dhcpd_enabled(is_ap_mode)" not in prepare
dhcp_enable_check = prepare.index("is_dhcpd_enabled_status(")
dhcp_string_snapshot = prepare.index("dnsmasq_dhcp_config_nvram_snapshot_read(")
fill_static = prepare.index("fill_static_ethers(ipaddr, netmask)")
assert dhcp_enable_check < dhcp_string_snapshot < fill_static
assert 'dnsmasq_read_string_option("dhcp_wins_x"' not in prepare
assert "dhcp_nvram_snapshot.dhcp_wins" in prepare
assert prepare.index("is_dhcpd_enabled_status(") < prepare.index("fill_static_ethers(ipaddr, netmask)")

# DHCPv6 enablement, mode, IRT and values must be checked before config staging.
radv_read = prepare.index("get_lan_radv_enabled_status(&radv_enabled)")
radv_error = prepare.index("if (dhcp6_status != 0)", radv_read)
mode_read = prepare.index("get_lan_dhcp6s_mode_status(&dhcp6_mode)", radv_error)
irt_read = prepare.index("get_lan_dhcp6s_irt_status(&dhcp6_irt)", mode_read)
v6_snapshot = prepare.index("dnsmasq_dhcp6_nvram_snapshot_read(", irt_read)
v6_error = prepare.index("if (dhcp6_status != 0)", v6_snapshot)
v6_open = prepare.index("dnsmasq_file_open(config_path)")
assert radv_read < radv_error < mode_read < irt_read < v6_snapshot < v6_error < v6_open
assert 'nvram_safe_get("dhcp_dnsv6_x")' not in prepare
for key in ("ip6_lan_sflt", "ip6_lan_sfps", "ip6_lan_sfpe"):
    assert 'nvram_safe_get_int("' + key + '"' not in prepare
assert "get_lan_radv_enabled_status(int *enabled)" in LAN6
assert "get_lan_dhcp6s_mode_status(int *mode)" in LAN6
assert "get_lan_dhcp6s_irt_status(int *irt)" in LAN6
assert "parsed_mode < 0 || parsed_mode > 3" in LAN6

smb_header = compact(body(SERVICES, "int\nwrite_smb_conf_header(FILE *fp)"))
smb_mode_read = smb_header.index("get_ap_mode_status(&is_ap_mode)")
smb_mode_error = smb_header.index("if (ap_mode_status != 0)", smb_mode_read)
smb_dhcp_check = smb_header.index("is_dhcpd_enabled_status(is_ap_mode, &dhcp_enabled)")
assert smb_mode_read < smb_mode_error < smb_dhcp_check
assert "cannot determine AP/router mode; refusing smb.conf header publication" in smb_header
assert "errno = ap_mode_status;" in smb_header
assert "if (dhcp_status != 0)" in smb_header and "return -1;" in smb_header
assert smb_dhcp_check < smb_header.index('fprintf(fp, "[global]\\n")')
assert "else if (dhcp_enabled)" in smb_header
assert "is_dhcpd_enabled(get_ap_mode())" not in smb_header

assert "test_invmatch_preserves_nvram_invmatch_semantics" in DNSMASQ_OPTION_TEST
assert "test_invmatch_backend_errors_preserve_output" in DNSMASQ_OPTION_TEST
assert "test_invmatch_invalid_arguments_are_rejected" in DNSMASQ_OPTION_TEST
for event in ("push", "pull_request"):
    event_section = DNSMASQ_WORKFLOW.split("  " + event + ":", 1)[1]
    end_marker = "  pull_request:" if event == "push" else "  workflow_dispatch:"
    event_paths = event_section.split(end_marker, 1)[0]
    assert event_paths.count('"trunk/user/rc/src/services_ex.c"') == 1
    assert event_paths.count('"trunk/user/rc/src/rc.h"') == 1
    assert event_paths.count('"scripts/check_dnsmasq_start_rollback.py"') == 1

print("DNSMasq failed-start rollback and lifecycle-lock contract: PASS")
