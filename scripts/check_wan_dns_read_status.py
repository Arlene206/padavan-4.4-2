#!/usr/bin/env python3
"""Verify WAN DNS-mode reads fail closed before resolver publication."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / "trunk/user/rc/src/net_wan.c").read_text()
policy = (ROOT / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
test = (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()


def code_characters(text, start):
    """Yield C source characters while ignoring comments and literals."""
    index = start
    length = len(text)
    while index < length:
        if text.startswith("//", index):
            newline = text.find(chr(10), index + 2)
            index = length if newline < 0 else newline + 1
            continue
        if text.startswith("/*", index):
            close = text.find("*/", index + 2)
            index = length if close < 0 else close + 2
            continue
        character = text[index]
        if character in ('"', "'"):
            quote = character
            index += 1
            while index < length:
                if ord(text[index]) == 92:
                    index += 2
                elif text[index] == quote:
                    index += 1
                    break
                else:
                    index += 1
            continue
        yield index, character
        index += 1


def body(text, signature):
    # Find a real definition, not a forward declaration. Ignore braces and
    # semicolons occurring in comments, string literals or character literals.
    start = text.find(signature)
    opening = -1
    while start >= 0:
        after = start + len(signature)
        for position, character in code_characters(text, after):
            if character == ";":
                break
            if character == "{":
                opening = position
                break
        if opening >= 0:
            break
        start = text.find(signature, start + 1)
    if start < 0 or opening < 0:
        raise AssertionError("missing function body: " + signature)

    depth = 0
    for index, character in code_characters(text, opening):
        if character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    raise AssertionError("unterminated function: " + signature)


# Regression-test the brace scanner with braces inside comments/literals and
# with a forward declaration before the implementation.
_parser_probe_source = r'''
int probe(void);
int probe(void) {
    const char *close_brace = "}";
    char open_brace = '{';
    /* ignored: } { */
    if (open_brace == '{') { return close_brace[0] == '}' ? 1 : 0; }
    return 0;
}
'''
_parser_probe = body(_parser_probe_source, "probe(void)")
_parser_escape_probe_source = r'''
int escaped_probe(void) {
    const char *escaped = "quote: \" }";
    // ignored } { on this line
    return escaped[0] == 'q' ? 1 : 0;
}
'''
if "return escaped[0]" not in body(_parser_escape_probe_source, "escaped_probe(void)"):
    raise AssertionError("C string escape/comment parser regression")

if "if (open_brace == '{')" not in _parser_probe or "return 0;" not in _parser_probe:
    raise AssertionError("C-aware function body parser regression")


def require(condition, message):
    if not condition:
        raise AssertionError(message)


require(source.count("get_wan_unit_value_int(") == 1,
        "internal WAN decisions must not use the error-collapsing integer getter")

checked_int = body(
    source,
    "get_wan_unit_value_int_status(int unit, const char *param_name, int *value)",
)
require("nvram_get_checked(wanN_param, &stored_value)" in checked_int,
        "WAN integer getter must use the checked NVRAM API")
require("rc_wan_nvram_read_int_status(api_result, saved_errno" in checked_int,
        "WAN integer getter must preserve missing/error states")

parse_int = body(policy, "rc_wan_nvram_parse_int_value(const char *text, int *value)")
for token in ("strtol(text, &end, 10)", "errno == ERANGE", "parsed < INT_MIN",
              "parsed > INT_MAX", "if (*end)"):
    require(token in parse_int, "strict integer parser missing " + token)

read_int = body(policy, "rc_wan_nvram_read_int_status(int api_result, int saved_errno")
for token in ("rc_wan_nvram_read_status(api_result, saved_errno)",
              "if (api_result == 0)", "if (!text)",
              "rc_wan_nvram_parse_int_value(text, value)"):
    require(token in read_int, "checked read policy missing " + token)

dns_status = body(source, "get_wan_dns_static_status(int *is_static)")
require('nvram_get_checked("modem_dnsa", &stored_value)' in dns_status,
        "USB modem DNS mode must use checked reads")
require('get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled)' in dns_status,
        "WAN DNS mode must use checked integer reads")
legacy_dns = body(source, "get_wan_dns_static(void)")
require("get_wan_dns_static_status(&is_static)" in legacy_dns and
        "if (status != 0)" in legacy_dns,
        "legacy DNS selector must retain a compatibility fallback on read errors")

resolver = body(source, "update_resolvconf(int is_first_run, int do_not_notify)")
for token in ("get_wan_dns_static_status(&dns_static)", "file_unlock(lock)",
              "errno = dns_static_status", "return dns_static_status"):
    require(token in resolver, "resolver error path missing " + token)
require(resolver.index("return dns_static_status") <
        resolver.index("dnsmasq_file_open(resolv_temp)"),
        "resolver staging must not start before DNS mode is confirmed")

for case in ("test_checked_integer_read_preserves_missing_default",
             "test_checked_integer_read_accepts_complete_decimal_values",
             "test_checked_integer_read_rejects_malformed_and_overflow_values",
             "test_checked_integer_read_propagates_backend_errors"):
    require(case in test, "WAN integer regression missing " + case)

print("WAN DNS-mode checked-read and resolver fail-closed contract: PASS")


# PPPoE manual mode and demand are control-plane decisions: backend errors
# must abort and restore WAN state, while missing keys retain their zero default.
wan_start = " ".join(body(source, "start_wan(void)").split())
manual_pattern = r'nvram_status\s*=\s*get_wan_unit_value_int_status\s*\(\s*unit,\s*"pppoe_man",\s*&i_pppoe_man\s*\)\s*;'
manual_matches = list(re.finditer(manual_pattern, wan_start))
manual_positions = [match.start() for match in manual_matches]
require(len(manual_positions) == 2,
        "both RAS and regular PPP paths must use checked PPPoE manual-mode reads")
for position, match in zip(manual_positions, manual_matches):
    failure = wan_start[position + len(match.group(0)):position + len(match.group(0)) + 600]
    require("if (nvram_status != 0)" in failure and
            "restore_wan_after_firewall_failure" in failure and
            "continue;" in failure,
            "PPPoE manual-mode read failure must restore WAN and skip PPP activation")
assert 'get_wan_unit_value_int(unit, "pppoe_man")' not in wan_start
idle_match = re.search(r'nvram_status\s*=\s*get_wan_unit_value_int_status\s*\(\s*unit,\s*"pppoe_idletime",\s*&i_demand\s*\)\s*;', wan_start)
require(idle_match is not None,
        "PPPoE idle-time must use checked NVRAM reads")
idle_read = idle_match.group(0)
idle_position = idle_match.start()
demand_match = re.search(
    r'set_wan_unit_value_int_status\s*\(\s*unit,\s*"pppoe_demand",\s*i_demand\s*\)\s*;',
    wan_start[idle_position:],
)
require(demand_match is not None,
        "PPPoE demand publication must follow the checked idle-time read")
idle_end = idle_position + demand_match.start()
idle_failure = wan_start[idle_position:idle_end]
require("if (nvram_status != 0)" in idle_failure and
        "restore_wan_after_firewall_failure(unit, wan_ifname)" in idle_failure and
        "continue;" in idle_failure,
        "PPPoE idle-time read failure must restore WAN before demand publication")
assert 'get_wan_unit_value_int(unit, "pppoe_idletime")' not in wan_start
print("PPPoE manual-mode and demand NVRAM errors abort WAN activation: PASS")


# MAN and physical WAN MTUs must be checked before any interface/client setup.
wanx_start = " ".join(body(source, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)").split())
require('get_wan_unit_value_int_status(unit, "man_mtu", &man_mtu)' in wanx_start,
        "MAN MTU must use checked NVRAM reads")
require(wanx_start.index("if (nvram_status != 0)") < wanx_start.index("wan_static_ipv4_snapshot("),
        "MAN MTU read errors must stop before address snapshot/client setup")
require("return nvram_status" in wanx_start,
        "MAN MTU read errors must propagate to the launch caller")
wan_start = " ".join(body(source, "start_wan(void)").split())
require('get_wan_unit_value_int_status(unit, "mtu", &wan_mtu)' in wan_start,
        "physical WAN MTU must use checked NVRAM reads")
mtu_read = wan_start.index('get_wan_unit_value_int_status(unit, "mtu", &wan_mtu)')
mtu_error = wan_start.index("if (nvram_status != 0)", mtu_read)
mtu_ifup = wan_start.index('ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL)', mtu_read)
require(mtu_read < mtu_error < mtu_ifup and
        "continue;" in wan_start[mtu_error:mtu_ifup],
        "physical MTU read errors must abort the unit before interface activation")
assert 'get_wan_unit_value_int(unit, "man_mtu")' not in source
assert 'get_wan_unit_value_int(unit, "mtu")' not in wan_start
print("WAN MTU reads fail closed before MAN/physical interface activation: PASS")


# EAPOL and IPoE/Kabinet authentication-mode decisions must use checked reads.
wan_start = " ".join(body(source, "start_wan(void)").split())
require('get_wan_unit_value_int_status(unit, "auth_mode", &wan_auth_mode)' in wan_start,
        "EAPOL mode must use checked NVRAM reads")
require("auth_status = nvram_status" in wan_start and "if (auth_status != 0)" in wan_start,
        "EAPOL read failures must use the existing startup cleanup path")
wan_up = " ".join(body(source, "wan_up_internal(char *wan_ifname, int unit, int is_static)").split())
require('get_wan_unit_value_int_status(unit, "auth_mode", &auth_mode)' in wan_up,
        "Kabinet startup must use checked auth-mode reads")
require("if (auth_status != 0)" in wan_up and "start_auth_kabinet()" in wan_up,
        "Kabinet startup must not run when mode read fails")
wan_down = " ".join(body(source, "wan_down_internal(char *wan_ifname, int unit, int is_static)").split())
require('get_wan_unit_value_int_status(unit, "auth_mode", &auth_mode)' in wan_down,
        "Kabinet teardown must use checked auth-mode reads")
require("if (auth_status != 0 || auth_mode == 1)" in wan_down and "stop_auth_kabinet()" in wan_down,
        "Kabinet teardown must attempt defensive stop when auth mode is unknown")
assert 'get_wan_unit_value_int(unit, "auth_mode")' not in source
print("EAPOL/Kabinet auth-mode reads fail closed and teardown remains defensive: PASS")


# MAN DNS-route creation is fail-closed; teardown still attempts idempotent removal.
add_man_routes = " ".join(body(source, "add_man_gateway_routes(char *man_ifname, int unit, int metric, int *gateway_route_status)").split())
require('get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled)' in add_man_routes,
        "MAN route creation must use checked DNS-mode reads")
require("is_man_dns_auto = 0" in add_man_routes and
        "skipping optional DNS routes" in add_man_routes and
        "if (is_man_dns_auto && !is_modem_active" in add_man_routes,
        "MAN DNS route creation must skip optional routes when mode is unknown")
teardown_man_routes = " ".join(body(source, "man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)").split())
require('get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled)' in teardown_man_routes,
        "MAN route teardown must use checked DNS-mode reads")
require("is_man_dns_auto = 1" in teardown_man_routes and
        "attempting DNS route removal" in teardown_man_routes and
        "if (is_man_dns_auto && !is_modem_active" in teardown_man_routes,
        "MAN route teardown must attempt best-effort DNS route removal when mode is unknown")
assert 'get_wan_unit_value_int(unit, "dnsenable_x")' not in add_man_routes
assert 'get_wan_unit_value_int(unit, "dnsenable_x")' not in teardown_man_routes
print("MAN DNS-route setup/teardown preserves NVRAM read errors and best-effort cleanup: PASS")


# The global SYN-cookie security gate must never mistake a backend error for
# the configured default. Missing-key zero remains compatible with nvram_get_int.
checked_global = body(source, "get_nvram_int_checked_status(const char *param_name, int *value)")
for token in ("nvram_get_checked(param_name, &stored_value)",
              "rc_wan_nvram_read_int_status(api_result, saved_errno"):
    require(token in checked_global, "global checked NVRAM getter missing " + token)
wan_start = " ".join(body(source, "start_wan(void)").split())
require('get_nvram_int_checked_status("fw_syn_cook", &tcp_syncookies_enabled)' in wan_start,
        "TCP SYN-cookie startup gate must use checked NVRAM reads")
require("if (nvram_status != 0)" in wan_start and
        "cannot read TCP SYN-cookie setting" in wan_start and
        "return nvram_status" in wan_start,
        "WAN startup must abort before sysctl and interface setup on a security-setting read error")
assert 'nvram_get_int("fw_syn_cook")' not in wan_start
assert wan_start.index('get_nvram_int_checked_status("fw_syn_cook", &tcp_syncookies_enabled)') < wan_start.index("set_tcp_syncookies(")
print("WAN SYN-cookie security policy read fails closed before activation: PASS")


# USB modem startup must not silently interpret NVRAM read failure as RAS mode.
wan_start = " ".join(body(source, "start_wan(void)").split())
modem_read = 'get_nvram_int_checked_status("modem_type", &modem_type)'
require(modem_read in wan_start,
        "USB modem startup must use checked transport-type reads")
modem_pos = wan_start.index(modem_read)
modem_error = wan_start.index("if (nvram_status != 0)", modem_pos)
modem_branch = wan_start.index("if (modem_type == 3)", modem_pos)
modem_cleanup = wan_start.index("restore_wan_after_firewall_failure(unit, wan_ifname)", modem_error)
modem_continue = wan_start.index("continue;", modem_cleanup)
require(modem_pos < modem_error < modem_cleanup < modem_continue < modem_branch,
        "modem type read failure must restore WAN and skip transport activation")
assert 'nvram_get_int("modem_type")' not in wan_start
print("USB modem startup fails closed when transport type is unreadable: PASS")

# The EAPOL read/start failure guard must close before the WAN protocol's
# success path; otherwise start_wan() becomes syntactically unbalanced.
wan_start = body(source, "start_wan(void)")
require("if (auth_status != 0) {" in wan_start and
        "continue;\n\t\t\t\t}\n\n\t\t\t/* We are done configuration */" in wan_start,
        "EAPOL failure branch must close before WAN protocol activation")
