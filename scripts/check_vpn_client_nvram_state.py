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
test = (ROOT / "trunk/user/rc/src/tests/vpn_client_nvram_policy.c").read_text()
rc_header = (ROOT / "trunk/user/rc/src/rc.h").read_text()

assert '#include "vpn_client_nvram_policy.h"' in vpn
assert "int restore_dns_from_vpnc(void);" in rc_header
assert "void restore_dns_from_vpnc(void);" not in rc_header

start = body(vpn, "static int\nstart_vpn_client_locked(void)")
ordered(start, [
    "saved_errno = vpn_client_nvram_clear_runtime_state(",
    "if (saved_errno != 0)",
    "return -1;",
    "i_type = nvram_get_int(\"vpnc_type\")"
], "VPN startup stale runtime cleanup")
assert "nvram_set_temp(\"vpnc_dns_t\", \"\")" not in start
assert "nvram_set_temp(\"vpnc_dom_t\", \"\")" not in start
assert "nvram_set_int_temp(\"vpnc_state_t\", 0)" not in start

setter = body(vpn, "static int\nvpn_client_nvram_set_temp_status(")
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
    "if (api_result == 0 || vpnc_dns[0] == '\\0')",
    "status = vpn_client_nvram_set_temp_status(NULL, \"vpnc_dns_t\", \"\");",
    "if (status != 0)",
    "refresh_resolvconf_nonfatal(\"VPN client DNS removal\", 0);"
], "VPN DNS cleanup")
assert "nvram_safe_get(\"vpnc_dns_t\")" not in restore

restart = body(vpn, "static int\nrestart_vpn_client_locked(void)")
ordered(restart, [
    "restore_status = restore_dns_from_vpnc();",
    "if (restore_status != 0)",
    "start_status = restore_status;",
    "} else {",
    "start_status = start_vpn_client_locked();",
    "firewall_status = restart_firewall();"
], "VPN restart resolver cleanup gate")

ipup = body(vpn, "ipup_vpnc_main(int argc, char **argv)")
ordered(ipup, [
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "1")',
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_dns_t", buf)',
    'if (route_status == 0)',
    'refresh_resolvconf_nonfatal("VPN client DNS update", 0);',
    'if (check_if_file_exist(script_name))'
], "VPN up callback NVRAM status")
assert 'if (strlen(buf) > 0)' not in ipup

ipdown = body(vpn, "ipdown_vpnc_main(int argc, char **argv)")
ordered(ipdown, [
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")',
    "status = route_status_accumulate(status, route_status);",
    "route_status = restore_dns_from_vpnc();",
    "status = route_status_accumulate(status, route_status);",
    'if (check_if_file_exist(script_name))'
], "VPN down callback NVRAM status")

for token in (
    "test_write_status_preserves_errno",
    "test_read_status_distinguishes_absence_from_failure",
    "test_runtime_clear_publishes_dns_domain_and_state_in_order",
    "test_runtime_clear_stops_at_first_failed_write",
    "test_invalid_sequence_does_not_write",
):
    assert token in test and token + "();" in test
for token in (
    "vpn_client_nvram_write_status",
    "vpn_client_nvram_read_status",
    "vpn_client_nvram_clear_runtime_state",
):
    assert token in header

print("VPN client runtime NVRAM/DNS fail-closed source contract: PASS")
