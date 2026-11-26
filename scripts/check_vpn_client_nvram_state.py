#!/usr/bin/env python3
"""Source contract for VPN client runtime NVRAM/DNS error handling."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def body(source, signature):
    start = source.find(signature)
    assert start >= 0, "missing function: " + signature
    opening = source.find("{", start + len(signature))
    assert opening >= 0, "missing body: " + signature
    depth = 0
    state = "code"
    position = opening
    while position < len(source):
        ch = source[position]
        nxt = source[position + 1] if position + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "*":
                state = "comment"
                position += 2
                continue
            if ch == "/" and nxt == "/":
                state = "line"
                position += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:position]
        elif state == "comment":
            if ch == "*" and nxt == "/":
                state = "code"
                position += 2
                continue
        elif state == "line":
            if ch == "\n":
                state = "code"
        elif state in ("string", "char"):
            if ch == "\\":
                position += 2
                continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        position += 1
    raise AssertionError("unterminated function: " + signature)


def ordered(source, tokens, label):
    position = -1
    for token in tokens:
        found = source.find(token, position + 1)
        assert found >= 0, label + ": missing/out of order " + repr(token)
        position = found


vpn = (ROOT / "trunk/user/rc/src/vpn_client.c").read_text()
header = (ROOT / "trunk/user/rc/src/vpn_client_nvram_policy.h").read_text()
assert "api_result == 0 && !value_present" in header
assert "api_result == 1 && value_present" in header
ppp_policy = (ROOT / "trunk/user/rc/src/ppp_option_value_policy.h").read_text()
ppp_test = (ROOT / "trunk/user/rc/src/tests/ppp_option_value_policy.c").read_text()
test = (ROOT / "trunk/user/rc/src/tests/vpn_client_nvram_policy.c").read_text()
rc_header = (ROOT / "trunk/user/rc/src/rc.h").read_text()
openvpn = (ROOT / "trunk/user/rc/src/vpn_openvpn.c").read_text()

assert '#include "vpn_client_nvram_policy.h"' in vpn
assert "int restore_dns_from_vpnc(void);" in rc_header
assert "void restore_dns_from_vpnc(void);" not in rc_header

start = body(vpn, "static int\nstart_vpn_client_locked(void)")
assert "vpn_client_read_start_config_snapshot(&config)" in start
assert "vpn_client_clear_runtime_state_status()" in start
assert "config.peer_line" in start and "config.user_line" in start
assert "config.pass_line" in start and "config.pppd_options" in start
assert "nvram_safe_get(" not in start
assert "nvram_get_int(" not in start
assert "nvram_match(" not in start
assert "nvram_invmatch(" not in start
assert "safe_pppd_line(" not in start
file_open = start.index("fp = ppp_options_file_open(vpnc_opt);")
assert start.index("vpn_client_read_start_config_snapshot(&config)") < file_open
for token in (
    "config.peer_line",
    "config.user_line",
    "config.pass_line",
    "config.pppd_options",
):
    assert start.find(token, file_open) >= file_open, token
assert "free(config.pppd_options);" in start
assert "pptp_server" in start
ordered(start, [
    "vpn_client_read_start_config_snapshot(&config)",
    "if (!config.enabled)",
    "i_type = config.type",
    "if (i_type == 2)",
    "vpn_client_clear_runtime_state_status()",
], "VPN startup snapshot before runtime mutation")

snapshot = body(vpn, "static int\nvpn_client_read_start_config_snapshot(")
ordered(snapshot, [
    "nvram_transaction_begin(&transaction)",
    '"vpnc_enable"',
    '"vpnc_type"',
    '"vpnc_auth"',
    '"vpnc_mppe"',
    '"vpnc_mtu"',
    '"vpnc_mru"',
    '"vpnc_dgw"',
    '"vpnc_peer"',
    '"vpnc_user"',
    '"vpnc_pass"',
    '"vpnc_pppd"',
    "nvram_transaction_end(&transaction)",
    "*config = pending;",
], "VPN PPP startup transaction")
assert "vpn_client_nvram_copy_status(" in vpn
assert "vpn_client_nvram_copy_status(" in header
assert "vpn_client_nvram_read_clamped_int_status(" in header
assert "vpn_client_nvram_copy_status" in test
assert "vpn_client_nvram_read_clamped_int_status" in test
clear_runtime = body(vpn, "int\nvpn_client_clear_runtime_state_status(")
assert "vpn_client_nvram_clear_runtime_state(" in clear_runtime

setter = body(vpn, "int\nvpn_client_nvram_set_temp_status(")
ordered(setter, [
    "api_result = nvram_set_temp(name, value);",
    "saved_errno = errno;",
    "status = vpn_client_nvram_write_status(api_result, saved_errno);"
], "checked VPN NVRAM setter")
restore = body(vpn, "int\nrestore_dns_from_vpnc(void)")
ordered(restore, [
    "api_result = nvram_get_checked(\"vpnc_dns_t\", &vpnc_dns);",
    "status = vpn_client_nvram_read_status(",
    "if (status != 0)",
    "else if (api_result == 1 && vpnc_dns[0] != '\\0')",
    "status = vpn_client_nvram_set_temp_status(NULL, \"vpnc_dns_t\", \"\");",
    "if (status != 0)",
    "refresh_resolvconf_nonfatal(\"VPN client DNS removal\", 0);"
], "VPN DNS cleanup")
assert ("else" + chr(10) + "\t\t\trefresh_resolvconf_nonfatal(\"VPN client DNS removal\", 0);") in restore
assert "nvram_safe_get(\"vpnc_dns_t\")" not in restore

l2tp_start = start.index('vpn_client_nvram_set_temp_status(\n\t\t\tNULL, "l2tp_cli_t", "1")')
l2tp_gate = start.index("if (saved_errno != 0)", l2tp_start)
l2tp_daemon = start.index("xl2tpd_status = safe_start_xl2tpd_locked()", l2tp_gate)
assert l2tp_start < l2tp_gate < l2tp_daemon
stop = body(vpn, "static int\nstop_vpn_client_locked(void)")
ordered(stop, [
    "stop_status = stop_vpn_client_ppp(0)",
    "route_status = restore_dns_from_vpnc();",
    "cleanup_status = route_status_accumulate(cleanup_status, route_status);",
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "l2tp_cli_t", "0")',
    "cleanup_status = route_status_accumulate(cleanup_status, route_status);",
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")',
    "unlink(VPNC_PPP_UP_SCRIPT)",
    "return cleanup_status;"
], "VPN stop NVRAM/DNS cleanup after PPP teardown")
assert "nvram_set_int_temp(" not in vpn

restart = body(vpn, "static int\nrestart_vpn_client_locked(void)")
assert "restore_dns_from_vpnc()" not in restart
ordered(restart, [
    "stop_status = stop_vpn_client_locked();",
    "sleep(1);",
    "start_status = start_vpn_client_locked();",
    "firewall_status = restart_firewall();"
], "VPN restart reuses stop-owned resolver cleanup")

ipup = body(vpn, "ipup_vpnc_main(int argc, char **argv)")
ordered(ipup, [
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "1")',
    "errno = 0;",
    'api_result = nvram_get_checked("vpnc_pdns", &pdns_value);',
    "read_errno = errno;",
    "pdns_status = vpn_client_nvram_read_dns_policy_status(",
    "if (pdns_status != 0)",
    "status = route_status_accumulate(status, pdns_status);",
    "if (pdns_status == 0) {",
    'vpn_client_nvram_set_temp_status(\n\t\t\tNULL, "vpnc_dns_t", buf)',
    'refresh_resolvconf_nonfatal("VPN client DNS update", 0);',
    'if (check_if_file_exist(script_name))'
], "VPN up callback checked DNS policy and mandatory state/hook")
assert 'nvram_get_int("vpnc_pdns")' not in ipup
assert "preserving existing resolver state" in ipup
assert 'if (strlen(buf) > 0)' not in ipup

restore = body(vpn, "int\nrestore_dns_from_vpnc(void)")
ordered(restore, [
    "vpn_client_nvram_read_status(",
    "if (status != 0)",
    "vpn_client_nvram_clear_domain_snapshot(",
    "status = route_status_accumulate(status, domain_status);",
    "return status;"
], "VPN DNS/domain snapshot cleanup")
assert "vpn_client_nvram_clear_domain_snapshot" in header

ipdown = body(vpn, "ipdown_vpnc_main(int argc, char **argv)")
ordered(ipdown, [
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")',
    "status = route_status_accumulate(status, route_status);",
    "route_status = restore_dns_from_vpnc();",
    "status = route_status_accumulate(status, route_status);",
    'if (check_if_file_exist(script_name))'
], "VPN down callback NVRAM status")

for token in (
    "test_cache_backed_text_is_copied_without_truncation",
    "test_write_status_preserves_errno",
    "test_read_status_distinguishes_absence_from_failure",
    "test_boolean_read_status_preserves_legacy_defaults_and_rejects_invalid_values",
    "test_runtime_clear_publishes_dns_domain_and_state_in_order",
    "test_runtime_clear_stops_at_first_failed_write",
    "test_domain_snapshot_clear_publishes_empty_value_and_reports_failure",
    "test_invalid_sequence_does_not_write",
):
    assert token in test and token + "();" in test
assert "test_multiline_allocator_preserves_format_and_blocks_ipparam" in ppp_test
assert "ppp_option_text_escape_alloc" in ppp_policy and "ppp_option_text_escape_alloc" in ppp_test
for token in (
    "vpn_client_nvram_write_status",
    "vpn_client_nvram_read_status",
    "vpn_client_nvram_read_bool_status",
    "vpn_client_nvram_clear_runtime_state",
    "vpn_client_nvram_copy_text",
):
    assert token in header

openvpn_up = body(openvpn, "static int\non_client_ifup(void)")
openvpn_down = body(openvpn, "static int\non_client_ifdown(void)")
client_snapshot = body(openvpn, "static int\nopenvpn_read_client_config_snapshot(")
ordered(client_snapshot, [
    '"vpnc_enable"',
    '"vpnc_type"',
    '"vpnc_ov_mode"',
    'openvpn_server_acl_copy_text(&transaction, "vpnc_peer"',
    'openvpn_server_acl_copy_text(&transaction, "vpnc_user"',
    'openvpn_server_acl_copy_text(&transaction, "vpnc_pass"',
    "nvram_transaction_end(&transaction)",
    "*config = pending;",
], "OpenVPN client snapshot and credentials")
secret = body(openvpn, "static int\nopenvpn_create_client_secret(")
assert "openvpn_client_secret_publish(secret_file, username, password)" in secret
assert 'nvram_safe_get("vpnc_user")' not in secret
assert 'nvram_safe_get("vpnc_pass")' not in secret
client_conf = body(openvpn, "static int\nopenvpn_create_client_conf(")
assert 'openvpn_create_client_secret("secret", config->username, config->password)' in client_conf
openvpn_start = body(openvpn, "int\nstart_openvpn_client(void)")
ordered(openvpn_start, [
    "openvpn_read_client_config_snapshot(&config)",
    "vpn_client_clear_runtime_state_status()",
    "openvpn_create_client_conf(vpnc_cfg, &config)",
    "_eval(openvpn_argv, NULL, 0, NULL)",
], "OpenVPN snapshot before runtime-state mutation")

openvpn_entry = body(openvpn, "int\novpn_client_script_main(int argc, char **argv)")
openvpn_up_compact = " ".join(openvpn_up.split())
assert 'api_result = nvram_get_checked("vpnc_pdns", &pdns_value);' in openvpn_up
assert "pdns_status = vpn_client_nvram_read_dns_policy_status(" in openvpn_up
assert 'nvram_get_int("vpnc_pdns")' not in openvpn_up
ordered(openvpn_up, [
    'api_result = nvram_get_checked("vpnc_pdns", &pdns_value);',
    "read_errno = errno;",
    "pdns_status = vpn_client_nvram_read_dns_policy_status(",
    "if (pdns_status != 0)",
    "status = route_status_accumulate(status, pdns_status);",
    "for (i = 0; i < ENV_SCAN_MAX; i++)",
    "if (pdns_status == 0) {",
    'values[value_count].name = "vpnc_dom_t";',
    'values[value_count].name = "vpnc_state_t";',
    "vpn_client_nvram_set_all_values(",
    'vpn_client_nvram_set_temp_status(\n\t\t\tNULL, "vpnc_state_t", "1")',
    'vpn_client_nvram_set_temp_status(NULL, "vpnc_dns_t", buf)',
    'call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "up");',
    "return status;"
], "OpenVPN client up checked DNS policy and mandatory state/hook")
assert "preserving existing resolver state" in openvpn_up
assert "vpn_server_config_int_status" not in openvpn_up
assert "pdns_value, 0, 0, 1," not in openvpn_up
assert openvpn_up.count("vpn_client_nvram_read_dns_policy_status(") == 1
assert "vpn_client_nvram_read_bool_status(" not in openvpn_up
assert "vpn_client_nvram_read_dns_policy_status(" in ipup
assert "vpn_client_nvram_read_bool_status(" not in ipup
for token in (
    "VPN_CLIENT_DNS_POLICY_DISABLED = 0",
    "VPN_CLIENT_DNS_POLICY_ADD_TO_CURRENT = 1",
    "VPN_CLIENT_DNS_POLICY_REPLACE_CURRENT = 2",
    "vpn_client_nvram_read_dns_policy_status(",
):
    assert token in header
assert "test_dns_policy_read_status_accepts_all_ui_modes" in test
assert "if (pdns_status == 0 && vpnc_pdns > 0)" in openvpn_up
ordered(openvpn_down, [
    'vpn_client_nvram_set_temp_status(NULL, "vpnc_state_t", "0")',
    "dns_status = restore_dns_from_vpnc();",
    "status = route_status_accumulate(status, dns_status);",
    'call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "down");',
    "return status;"
], "OpenVPN client down NVRAM status")
assert "return on_client_ifup();" in openvpn_entry
assert "return on_client_ifdown();" in openvpn_entry
assert "nvram_set_int_temp(" not in openvpn_up + openvpn_down
assert "nvram_set_temp(" not in openvpn_up + openvpn_down
assert "int vpn_client_nvram_set_temp_status(void *context" in rc_header
assert "vpn_client_nvram_set_all_values" in header
print("VPN client runtime NVRAM/DNS fail-closed source contract: PASS")
