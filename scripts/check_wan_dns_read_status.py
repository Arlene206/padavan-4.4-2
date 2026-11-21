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


def function_body_at_column_zero_terminator(text, signature):
    """Extract a function despite mutually exclusive preprocessor brace branches."""
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

    for index, character in code_characters(text, opening):
        if character != "}":
            continue
        line_start = text.rfind(chr(10), 0, index) + 1
        if line_start == index:
            return text[opening + 1:index]
    raise AssertionError("unterminated function: " + signature)


_parser_conditional_source = r'''
int conditional_probe(int value) {
#if defined(LEFT_BRANCH)
    if (value) {
#else
    if (!value) {
#endif
        value++;
    }
    return value;
}
'''
_parser_conditional_body = function_body_at_column_zero_terminator(
    _parser_conditional_source, "conditional_probe(int value)"
)
_require_markers = ("if (value)", "if (!value)", "return value;")
if not all(marker in _parser_conditional_body for marker in _require_markers):
    raise AssertionError("C preprocessor alternate-brace function extraction regression")


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
wan_start = " ".join(function_body_at_column_zero_terminator(source, "start_wan(void)").split())
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
restore_start = idle_failure.find("restore_wan_after_firewall_failure(")
restore_end = idle_failure.find(")", restore_start) if restore_start >= 0 else -1
restore_call = (
    "".join(idle_failure[restore_start:restore_end + 1].split())
    if restore_end >= 0 else ""
)
continue_position = idle_failure.find("continue;")
require("if (nvram_status != 0)" in idle_failure and
        restore_call == "restore_wan_after_firewall_failure(unit,wan_ifname)" and
        continue_position > restore_start >= 0,
        "PPPoE idle-time read failure must restore WAN before demand publication")
assert 'get_wan_unit_value_int(unit, "pppoe_idletime")' not in wan_start
print("PPPoE manual-mode and demand NVRAM errors abort WAN activation: PASS")


# MAN and physical WAN MTUs must be checked before any interface/client setup.
wanx_start = " ".join(body(source, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)").split())
wanx_start_compact = "".join(wanx_start.split())
require('get_wan_unit_value_int_status(unit,"man_mtu",&man_mtu)' in wanx_start_compact,
        "MAN MTU must use checked NVRAM reads")
require(wanx_start.index("if (nvram_status != 0)") < wanx_start.index("wan_static_ipv4_snapshot("),
        "MAN MTU read errors must stop before address snapshot/client setup")
require("return nvram_status" in wanx_start,
        "MAN MTU read errors must propagate to the launch caller")
wan_start = " ".join(function_body_at_column_zero_terminator(source, "start_wan(void)").split())
wan_start_compact = "".join(wan_start.split())
mtu_read_call = 'get_wan_unit_value_int_status(unit,"mtu",&wan_mtu)'
mtu_ifup_call = 'ifconfig(wan_ifname,IFUP,"0.0.0.0",NULL)'
require(mtu_read_call in wan_start_compact,
        "physical WAN MTU must use checked NVRAM reads")
mtu_read = wan_start_compact.index(mtu_read_call)
mtu_error = wan_start_compact.index("if(nvram_status!=0)", mtu_read)
mtu_ifup = wan_start_compact.index(mtu_ifup_call, mtu_read)
require(mtu_read < mtu_error < mtu_ifup and
        "continue;" in wan_start_compact[mtu_error:mtu_ifup],
        "physical MTU read errors must abort the unit before interface activation")
assert 'get_wan_unit_value_int(unit, "man_mtu")' not in source
assert 'get_wan_unit_value_int(unit, "mtu")' not in wan_start
print("WAN MTU reads fail closed before MAN/physical interface activation: PASS")


# EAPOL and IPoE/Kabinet authentication-mode decisions must use checked reads.
wan_start = " ".join(function_body_at_column_zero_terminator(source, "start_wan(void)").split())
wan_start_compact = "".join(wan_start.split())
require('get_wan_unit_value_int_status(unit,"auth_mode",&wan_auth_mode)' in wan_start_compact,
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
add_man_routes_compact = "".join(add_man_routes.split())
require('get_wan_unit_value_int_status(unit,"dnsenable_x",&dns_enabled)' in add_man_routes_compact,
        "MAN route creation must use checked DNS-mode reads")
require("is_man_dns_auto=0" in add_man_routes_compact and
        "skippingoptionalDNSroutes" in add_man_routes_compact and
        "if(is_man_dns_auto&&!is_modem_active" in add_man_routes_compact,
        "MAN DNS route creation must skip optional routes when mode is unknown")
teardown_man_routes = " ".join(body(source, "man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)").split())
teardown_man_routes_compact = "".join(teardown_man_routes.split())
require('get_wan_unit_value_int_status(unit,"dnsenable_x",&dns_enabled)' in teardown_man_routes_compact,
        "MAN route teardown must use checked DNS-mode reads")
require("is_man_dns_auto=1" in teardown_man_routes_compact and
        "attemptingDNSrout eremoval".replace(" ", "") in teardown_man_routes_compact and
        "if(is_man_dns_auto&&!is_modem_active" in teardown_man_routes_compact,
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
wan_start = " ".join(function_body_at_column_zero_terminator(source, "start_wan(void)").split())
wan_start_compact = "".join(wan_start.split())
syncookie_read = 'get_nvram_int_checked_status("fw_syn_cook",&tcp_syncookies_enabled)'
require(syncookie_read in wan_start_compact,
        "TCP SYN-cookie startup gate must use checked NVRAM reads")
require("if(nvram_status!=0)" in wan_start_compact and
        "cannotreadTCPSYN-cookiesetting" in wan_start_compact and
        "returnnvram_status" in wan_start_compact,
        "WAN startup must abort before sysctl and interface setup on a security-setting read error")
assert 'nvram_get_int("fw_syn_cook")' not in wan_start_compact
assert wan_start_compact.index(syncookie_read) < wan_start_compact.index("set_tcp_syncookies(")
print("WAN SYN-cookie security policy read fails closed before activation: PASS")


# USB modem startup must not silently interpret NVRAM read failure as RAS mode.
wan_start = " ".join(function_body_at_column_zero_terminator(source, "start_wan(void)").split())
wan_start_compact = "".join(wan_start.split())
modem_read = 'get_nvram_int_checked_status("modem_type",&modem_type)'
require(modem_read in wan_start_compact,
        "USB modem startup must use checked transport-type reads")
modem_pos = wan_start_compact.index(modem_read)
modem_error = wan_start_compact.index("if(nvram_status!=0)", modem_pos)
modem_branch = wan_start_compact.index("if(modem_type==3)", modem_pos)
modem_cleanup = wan_start_compact.index("restore_wan_after_firewall_failure(unit,wan_ifname)", modem_error)
modem_continue = wan_start_compact.index("continue;", modem_cleanup)
require(modem_pos < modem_error < modem_cleanup < modem_continue < modem_branch,
        "modem type read failure must restore WAN and skip transport activation")
assert 'nvram_get_int("modem_type")' not in wan_start_compact
print("USB modem startup fails closed when transport type is unreadable: PASS")

# USB modem stop-path and interface selection must not collapse an NVRAM
# transport-type read error into the legacy RAS default.
modem_policy = body(policy, "rc_wan_nvram_modem_is_ndis_status(int read_status")
for token in ("if (!is_ndis)", "if (read_status != 0)", "(modem_type == 3)"):
    require(token in modem_policy, "checked modem transport policy missing " + token)
require("test_modem_transport_selection_fails_closed_on_read_error" in test,
        "modem transport read-error regression missing")

modem_reader = body(source, "get_wan_modem_is_ndis_status(int *is_ndis)")
require('get_nvram_int_checked_status("modem_type", &modem_type)' in modem_reader and
        "rc_wan_nvram_modem_is_ndis_status(status, modem_type, is_ndis)" in modem_reader,
        "modem transport helper must preserve checked NVRAM errors")

stop_locked = function_body_at_column_zero_terminator(source, "stop_wan_locked(void)")
stop_locked_compact = "".join(stop_locked.split())
require("is_usb_modem=get_usb_modem_wan(unit)" in stop_locked_compact and
        "modem_status=get_wan_modem_is_ndis_status(&is_ndis_modem)" in stop_locked_compact,
        "full WAN teardown must check modem transport before side effects")
require(stop_locked_compact.index("modem_status=get_wan_modem_is_ndis_status(&is_ndis_modem)") <
        stop_locked_compact.index("notify_pause_detect_internet()"),
        "full WAN teardown must read modem transport before stopping services")
assert 'nvram_get_int("modem_type")' not in stop_locked_compact

manual_disconnect = "".join(body(source, "manual_wan_disconnect(void)").split())
require("status=get_wan_modem_is_ndis_status(&is_ndis_modem)" in manual_disconnect and
        "if(status!=0)" in manual_disconnect and
        "errno=status" in manual_disconnect and
        "if(is_ndis_modem)status=stop_wan()" in manual_disconnect and
        "elsestatus=stop_wan_ppp()" in manual_disconnect,
        "manual disconnect must not guess the modem stop path after a read error")
assert 'nvram_get_int("modem_type")' not in manual_disconnect

wan_ifname = "".join(body(source, "get_wan_ifname(char wan_ifname[16])").split())
require("status=get_wan_modem_is_ndis_status(&is_ndis_modem)" in wan_ifname and
        "if(status!=0)" in wan_ifname and "returnstatus" in wan_ifname,
        "WAN interface selection must propagate modem-type read errors")
assert 'nvram_get_int("modem_type")' not in wan_ifname
require("man_ifname_value=get_man_ifname(unit)" in wan_ifname and
        "rc_wan_nvram_copy_text(man_ifname_value,man_ifname,sizeof(man_ifname))" in wan_ifname,
        "management interface name must be copied before another NVRAM read")
require("get_wan_unit_value_copy_status(unit,\"ifname_t\",ifname_temp,sizeof(ifname_temp))" in wan_ifname and
        "is_usb_modem=get_usb_modem_wan(unit)" in wan_ifname,
        "WAN temporary interface name must be checked and copied before modem selection")
assert "char*ifname_temp=get_wan_unit_value(unit,\"ifname_t\")" not in wan_ifname

wan_ip4 = "".join(body(source, "get_wan_ip4(int only_broadband_wan)").split())
require("rc_wan_nvram_copy_text(man_ifname_value,man_ifname,sizeof(man_ifname))" in wan_ip4 and
        "get_wan_unit_value_copy_status(unit,\"ifname_t\",ifname_temp,sizeof(ifname_temp))" in wan_ip4,
        "WAN IP lookup must copy cache-backed interface names before later NVRAM reads")
require("if(status!=0)returnINADDR_ANY" in wan_ip4 and
        "if(!only_broadband_wan&&get_usb_modem_wan(unit))" in wan_ip4,
        "WAN IP lookup must fail closed if interface identity cannot be copied")
assert "char*ifname_temp=get_wan_unit_value(unit,\"ifname_t\")" not in wan_ip4
require("test_wan_interface_names_survive_shared_cache_overwrite" in test,
        "WAN interface-name cache overwrite regression missing")

# IPTV VLAN config used to retain pointers into nvram_linux.c's reusable
# cache; copy every value before the next NVRAM lookup.
viptv = "".join(function_body_at_column_zero_terminator(
    source, "launch_viptv_wan(void)"
).split())
require('rc_wan_nvram_copy_value(wan_nvram_checked_get_callback,NULL,"vlan_filter",vlan_filter,sizeof(vlan_filter))' in viptv,
        "IPTV VLAN filter must use the checked value-copy helper")
for key in ("vlan_vid_cpu", "vlan_vid_iptv", "vlan_pri_iptv", "viptv_mode"):
    require(f'get_nvram_int_checked_status("{key}",' in viptv,
            "IPTV VLAN configuration must use checked reads: " + key)
require('get_wan_unit_value_copy_status(0,"viflast",vinet_iflast,sizeof(vinet_iflast))' in viptv and
        'rc_wan_nvram_copy_value(wan_nvram_checked_get_callback,NULL,"viptv_ifname",viptv_iflast,sizeof(viptv_iflast))' in viptv,
        "IPTV VLAN must preserve both previous interface identities")
for key, destination in (
    ("viptv_ipaddr", "viptv_addr_setting"),
    ("viptv_netmask", "viptv_mask_setting"),
    ("viptv_gateway", "viptv_gate_setting"),
    ("wan_hwaddr", "wan_hwaddr"),
):
    require(f'"{key}"' in viptv and f'{destination},sizeof({destination})' in viptv,
            "IPTV VLAN setting must be copied before cache reuse: " + key)
assert 'get_wan_unit_value(0,"viflast")' not in viptv
for key in ("vlan_filter", "viptv_ifname", "viptv_ipaddr",
            "viptv_netmask", "viptv_gateway", "wan_hwaddr"):
    assert f'nvram_safe_get("{key}")' not in viptv
require("test_viptv_settings_survive_shared_cache_overwrite" in test,
        "IPTV shared-cache overwrite regression missing")


# The EAPOL read/start failure guard must close before the WAN protocol's
# success path; otherwise start_wan() becomes syntactically unbalanced.# The EAPOL read/start failure guard must close before the WAN protocol's
# success path; otherwise start_wan() becomes syntactically unbalanced.
wan_start = function_body_at_column_zero_terminator(source, "start_wan(void)")
require("if (auth_status != 0) {" in wan_start and
        "continue;\n\t\t\t\t}\n\n\t\t\t/* We are done configuration */" in wan_start,
        "EAPOL failure branch must close before WAN protocol activation")
