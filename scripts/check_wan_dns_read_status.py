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
require("get_usb_modem_wan_status(unit, &is_usb_modem)" in dns_status and
        "if (status != 0)" in dns_status and
        dns_status.index("get_usb_modem_wan_status(unit, &is_usb_modem)") <
        dns_status.index('nvram_get_checked("modem_dnsa", &stored_value)') and
        'nvram_get_checked("modem_dnsa", &stored_value)' in dns_status,
        "USB modem DNS mode must check modem identity before modem-specific NVRAM")
require('get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled)' in dns_status,
        "WAN DNS mode must use checked integer reads")
legacy_dns = body(source, "get_wan_dns_static(void)")
require("get_wan_dns_static_status(&is_static)" in legacy_dns and
        "if (status != 0)" in legacy_dns,
        "legacy DNS selector must retain a compatibility fallback on read errors")

stop_wan = "".join(body(source, "stop_wan_locked(void)").split())
stop_proto_read = "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
stop_protocol_aggregate = "stop_status=wan_teardown_first_error_status(stop_status,proto_status,ifdown_status,callback_cleanup_status)"
stop_callback_cleanup = "callback_cleanup_status=remove_cb_links()"
require(stop_proto_read in stop_wan and
        "if(proto_status!=0)" in stop_wan and
        stop_protocol_aggregate in stop_wan and
        stop_wan.index(stop_callback_cleanup) < stop_wan.index(stop_protocol_aggregate) and
        "if((proto_status!=0||wan_proto==IPV4_WAN_PROTO_IPOE_STATIC)" in stop_wan,
        "WAN stop must retain protocol errors after independent cleanup and use conservative WAN-down cleanup")

wan_down = "".join(body(source, "wan_down_internal(char *wan_ifname, int unit, int is_static)").split())
down_proto_read = "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
require(down_proto_read in wan_down and
        "status=route_status_accumulate(status,proto_status)" in wan_down and
        "(!modem_unit_id&&(proto_status!=0||wan_proto==IPV4_WAN_PROTO_IPOE_DHCP))" in wan_down and
        "if(modem_status!=0||(!modem_unit_id&&(proto_status!=0||wan_proto==IPV4_WAN_PROTO_IPOE_STATIC||wan_proto==IPV4_WAN_PROTO_IPOE_DHCP)))" in wan_down and
        "wan_dns_read_status=route_status_accumulate(proto_status,modem_status)" in wan_down and
        "wan_dns_read_status==0&&!force_clear_wan_dns" in wan_down and
        "if(modem_status==0&&!modem_unit_id&&proto_status==0&&wan_proto==IPV4_WAN_PROTO_IPOE_STATIC)" in wan_down,
        "WAN down must clean lease/auth state conservatively but preserve DNS/IP on unknown protocol")
require("del_dhcp_routes_by_prefix(prefix,wan_ifname,0)" in wan_down and
        wan_down.index(down_proto_read) < wan_down.index("del_dhcp_routes_by_prefix(prefix,wan_ifname,0)") <
        wan_down.index("del_static_wan_routes(wan_ifname)") and
        wan_down.index("del_static_wan_routes(wan_ifname)") < wan_down.index("refresh_resolvconf_nonfatal") <
        wan_down.index("flush_conntrack_table(wan_addr)"),
        "WAN down route, resolver and conntrack cleanup must remain after protocol errors")

resolver = body(source, "update_resolvconf(int is_first_run, int do_not_notify)")
resolver_compact = "".join(resolver.split())
resolver_identity = "modem_identity_status=get_usb_modem_wan_status(0,&is_modem_wan)"
require(resolver_identity in resolver_compact and
        "if(modem_identity_status!=0)" in resolver_compact and
        "resolv_changed=1;" in resolver_compact,
        "resolver publication must force DNSMasq refresh when modem identity is unknown")
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
wan_up_protocol = "".join(body(source, "wan_up_internal(char *wan_ifname, int unit, int is_static)").split())
modem_helper = "".join(body(source, "is_ifunit_modem_status(char *wan_ifname, int unit, int *modem_unit_id)").split())
identity_read = re.search(r"([A-Za-z_]\w*)=is_ifunit_modem_status\(wan_ifname,unit,&modem_unit_id\)", wan_up_protocol)
require(identity_read is not None, "WAN-up must use status-returning modem identity")
identity_status = identity_read.group(1)
identity_error = "if(" + identity_status + "!=0)"
identity_return = "errno=" + identity_status + ";return" + identity_status + ";"
wan_up_proto_read = "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
require("get_usb_modem_wan_status(unit,&is_modem)" in modem_helper and
        "get_wan_unit_value_copy_status(unit,\"ifname_t\"" in modem_helper,
        "modem identity helper must use checked modem_dev and ifname_t reads")
require(identity_error in wan_up_protocol and identity_return in wan_up_protocol and
        wan_up_protocol.index(identity_read.group(0)) < wan_up_protocol.index(identity_error) <
        wan_up_protocol.index(identity_return) <
        wan_up_protocol.index(wan_up_proto_read) <
        wan_up_protocol.index("set_wan_unit_value(unit,\"uptime\"") and
        "is_ifunit_modem(wan_ifname,unit)" not in wan_up_protocol,
        "WAN-up must fail closed on unreadable modem identity before protocol/runtime changes")
print("EAPOL/Kabinet auth-mode reads fail closed and teardown remains defensive: PASS")


# Per-unit WAN protocol must be copied with one checked NVRAM transaction.
# nvram_safe_get() collapses backend errors to an empty string; on a transient
# failure this used to overwrite wan0_proto and potentially select no/wrong WAN.
reset_wan = function_body_at_column_zero_terminator(source, "reset_wan_vars_status(void)")
require('set_wan_unit_param_status(unit, "proto")' in reset_wan,
        "WAN reset must copy wan_proto with the checked transaction helper")
assert 'nvram_safe_get("wan_proto")' not in reset_wan
param_copy = function_body_at_column_zero_terminator(
    source, "set_wan_unit_param_status(int unit, const char *param_name)"
)
param_copy_compact = " ".join(param_copy.split())
for token in (
    "nvram_transaction_begin(&transaction)",
    "nvram_transaction_get(&transaction, wan_param, &source_value)",
    "status = rc_wan_nvram_read_status(read_result, saved_errno)",
    '(read_result == 1) ? source_value : ""',
    "nvram_transaction_set( &transaction, wanN_param",
    "nvram_transaction_end(&transaction)",
):
    require(token in param_copy_compact,
            "WAN protocol copy must remain checked and transactional: " + token)
read_error = param_copy_compact.index("status = rc_wan_nvram_read_status(read_result, saved_errno)")
write_value = param_copy_compact.index("nvram_transaction_set( &transaction, wanN_param")
require(read_error < write_value and
        "if (status == 0) {" in param_copy_compact[read_error:write_value],
        "a failed shared WAN protocol read must stop before wan0_proto is written")
print("WAN protocol reset uses checked transactional NVRAM copy: PASS")


# MAN DNS-route creation is fail-closed; teardown still attempts idempotent removal.
add_man_routes = " ".join(body(source, "add_man_gateway_routes(char *man_ifname, int unit, int metric, int *gateway_route_status)").split())
add_man_routes_compact = "".join(add_man_routes.split())
require('get_wan_unit_value_int_status(unit,"dnsenable_x",&dns_enabled)' in add_man_routes_compact,
        "MAN route creation must use checked DNS-mode reads")
require("is_man_dns_auto=0" in add_man_routes_compact and
        "skippingoptionalDNSroutes" in add_man_routes_compact and
        "wan_man_dns_route_add_allowed(proto_status,wan_proto==IPV4_WAN_PROTO_PPPOE,is_man_dns_auto,is_modem_active)" in add_man_routes_compact and
        "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)" in add_man_routes_compact and
        "status=route_status_accumulate(status,proto_status)" in add_man_routes_compact and
        "*gateway_route_status=route_status_accumulate(*gateway_route_status,proto_status)" in add_man_routes_compact,
        "MAN DNS route creation must skip optional routes when mode or protocol is unknown")
teardown_man_routes = " ".join(body(source, "man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)").split())
teardown_man_routes_compact = "".join(teardown_man_routes.split())
require('get_wan_unit_value_int_status(unit,"dnsenable_x",&dns_enabled)' in teardown_man_routes_compact,
        "MAN route teardown must use checked DNS-mode reads")
require("is_man_dns_auto=1" in teardown_man_routes_compact and
        "attemptingDNSrout eremoval".replace(" ", "") in teardown_man_routes_compact and
        "wan_man_dns_route_remove_allowed(proto_status,wan_proto==IPV4_WAN_PROTO_PPPOE,is_man_dns_auto,is_modem_active)" in teardown_man_routes_compact and
        "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)" in teardown_man_routes_compact and
        "status=route_status_accumulate(status,proto_status)" in teardown_man_routes_compact,
        "MAN route teardown must use checked protocol and best-effort DNS cleanup when unknown")
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
identity_read = "modem_status=get_usb_modem_wan_status(unit,&is_usb_modem)"
identity_error = "if(modem_status!=0)"
require(identity_read in stop_locked_compact and identity_error in stop_locked_compact and
        "returnmodem_status;" in stop_locked_compact and
        "modem_status=get_wan_modem_is_ndis_status(&is_ndis_modem)" in stop_locked_compact,
        "full WAN teardown must distinguish unknown modem identity from non-modem state")
require(stop_locked_compact.index(identity_read) < stop_locked_compact.index(identity_error) <
        stop_locked_compact.index("returnmodem_status;") <
        stop_locked_compact.index("modem_status=get_wan_modem_is_ndis_status(&is_ndis_modem)") <
        stop_locked_compact.index("notify_pause_detect_internet()"),
        "full WAN teardown must verify identity and transport before stopping services")
assert 'nvram_get_int("modem_type")' not in stop_locked_compact

wisp_migration = "".join(body(source, "config_apcli_wisp(void)").split())
wisp_manual_read = "ifname_status=get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))"
wisp_manual_copy = "ifname_status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)"
wisp_manual_read_at = wisp_migration.index(wisp_manual_read)
wisp_manual_read_error_at = wisp_migration.index("if(ifname_status!=0)", wisp_manual_read_at)
wisp_manual_nonempty_at = wisp_migration.index("if(man_ifname_nvram[0])", wisp_manual_read_error_at)
wisp_manual_copy_at = wisp_migration.index(wisp_manual_copy, wisp_manual_nonempty_at)
wisp_manual_copy_error_at = wisp_migration.index("if(ifname_status!=0)", wisp_manual_copy_at)
wisp_old_ifconfig_at = wisp_migration.index("ifconfig(man_ifname,IFUP,\"0.0.0.0\",NULL)")
wisp_ipv6_enable_at = wisp_migration.find("control_if_ipv6(wisp_ifname,1)")
wisp_publish_at = wisp_migration.index("set_wan_unit_value_status(0,\"ifname\",wisp_ifname)")
require(wisp_manual_read_at < wisp_manual_read_error_at < wisp_manual_nonempty_at <
        wisp_manual_copy_at < wisp_manual_copy_error_at < wisp_old_ifconfig_at <
        wisp_publish_at and
        "man_ifname[0]='\\0';" in wisp_migration and
        "errno=ifname_status;" in wisp_migration and
        "get_man_ifname(unit)" not in wisp_migration,
        "AP-client WISP migration must check prior WAN identity before cleanup and publication")
require(wisp_ipv6_enable_at < 0 or wisp_manual_copy_error_at < wisp_ipv6_enable_at,
        "AP-client WISP migration must not change IPv6 before validating the prior WAN interface")
manual_ifname_read = "ifname_status=get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))"
manual_ifname_validate = "ifname_status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)"
manual_ifname_read_at = stop_locked_compact.index(manual_ifname_read)
manual_ifname_read_error_at = stop_locked_compact.index("if(ifname_status!=0)", manual_ifname_read_at)
manual_ifname_validate_gate_at = stop_locked_compact.index("if(man_ifname_nvram[0])", manual_ifname_read_error_at)
manual_ifname_validate_at = stop_locked_compact.index(manual_ifname_validate, manual_ifname_validate_gate_at)
manual_ifname_validate_error_at = stop_locked_compact.index("if(ifname_status!=0)", manual_ifname_validate_at)
manual_ifname_empty_at = stop_locked_compact.index("man_ifname[0]='\\0'", manual_ifname_validate_error_at)
manual_ifname_pause_at = stop_locked_compact.index("notify_pause_detect_internet()", manual_ifname_validate_error_at)
require(manual_ifname_read_at < manual_ifname_read_error_at < manual_ifname_validate_gate_at <
        manual_ifname_validate_at < manual_ifname_validate_error_at < manual_ifname_empty_at <
        manual_ifname_pause_at and
        "errno=ifname_status;" in stop_locked_compact and
        "returnifname_status;" in stop_locked_compact and
        "get_man_ifname(unit)" not in stop_locked_compact,
        "WAN teardown must resolve manual interface before destructive side effects and preserve read errors")

full_restart = "".join(body(source, "full_restart_wan(void)").split())
restart_manual_read = "ifname_status=get_wan_unit_value_copy_status(0,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))"
restart_manual_validate = "ifname_status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)"
restart_manual_read_at = full_restart.index(restart_manual_read)
restart_manual_read_error_at = full_restart.index("if(ifname_status!=0)", restart_manual_read_at)
restart_manual_nonempty_at = full_restart.index("if(man_ifname_nvram[0])", restart_manual_read_error_at)
restart_manual_validate_at = full_restart.index(restart_manual_validate, restart_manual_nonempty_at)
restart_manual_validate_error_at = full_restart.index("if(ifname_status!=0)", restart_manual_validate_at)
restart_mode_update_at = full_restart.index("update_router_mode()")
restart_stop_wan_at = full_restart.index("stop_wan()")
restart_route_use_at = full_restart.index("route_ifnames[0]=man_ifname")
require(restart_manual_read_at < restart_manual_read_error_at < restart_manual_nonempty_at <
        restart_manual_validate_at < restart_manual_validate_error_at <
        restart_mode_update_at < restart_stop_wan_at < restart_route_use_at and
        "man_ifname[0]='\\0';" in full_restart and
        "get_man_ifname(0)" not in full_restart and
        "errno=ifname_status;" in full_restart,
        "full WAN restart must check and preserve route interface identity before stopping WAN")
manual_disconnect = "".join(body(source, "manual_wan_disconnect(void)").split())
manual_modem_read = "status=get_usb_modem_wan_status(0,&is_usb_modem)"
manual_proto_read = "status=get_wan_unit_proto_checked_status(0,&wan_proto);"
require(manual_modem_read in manual_disconnect and manual_proto_read in manual_disconnect and
        "if(status!=0)" in manual_disconnect and
        "errno=status" in manual_disconnect and
        manual_disconnect.index(manual_modem_read) < manual_disconnect.index("if(status!=0)") <
        manual_disconnect.index("returnstatus;") <
        manual_disconnect.index("if(!is_usb_modem)") <
        manual_disconnect.index(manual_proto_read) <
        manual_disconnect.index("status=stop_wan_ppp()") and
        "wan_proto=get_wan_proto(0)" not in manual_disconnect,
        "manual disconnect must check modem identity and protocol before selecting WAN stop path")
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
require("if(ifname==man_ifname)" in wan_ifname and
        "status=get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))" in wan_ifname and
        "status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)" in wan_ifname and
        "get_man_ifname(unit)" not in wan_ifname and
        wan_ifname.index("if(ifname==man_ifname)") < wan_ifname.index("status=get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))") < wan_ifname.index("status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)"),
        "selected management interface name must use checked NVRAM copy and token validation")
require("get_wan_unit_value_copy_status(unit,\"ifname_t\",ifname_temp,sizeof(ifname_temp))" in wan_ifname and
        "status=get_usb_modem_wan_status(unit,&is_usb_modem)" in wan_ifname and
        "if(status!=0){errno=status;returnstatus;}" in wan_ifname,
        "WAN temporary interface name and modem identity must be checked before transport selection")
assert "char*ifname_temp=get_wan_unit_value(unit,\"ifname_t\")" not in wan_ifname

wan_ip4 = "".join(body(source, "get_wan_ip4(int only_broadband_wan)").split())
require("ifname=NULL" in wan_ip4 and
        "get_wan_unit_value_copy_status(unit,\"ifname_t\",ifname_temp,sizeof(ifname_temp))" in wan_ip4 and
        "if(ifname==NULL)" in wan_ip4 and
        "get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))" in wan_ip4 and
        "wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)" in wan_ip4 and
        "ifname=man_ifname" in wan_ip4 and
        "get_man_ifname(unit)" not in wan_ip4 and
        "man_ifname_value" not in wan_ip4,
        "WAN IP lookup must read and validate manual interface only after modem/PPP selection")
ip4_modem_read_at = wan_ip4.index("status=get_usb_modem_wan_status(unit,&is_usb_modem)")
ip4_modem_error_at = wan_ip4.index("if(status!=0){errno=status;returnINADDR_ANY;}", ip4_modem_read_at)
ip4_modem_select_at = wan_ip4.index("if(!only_broadband_wan&&is_usb_modem)")
require(ip4_modem_read_at < ip4_modem_error_at < ip4_modem_select_at,
        "WAN IP lookup must preserve modem identity read errors before selecting the transport")
assert "char*ifname_temp=get_wan_unit_value(unit,\"ifname_t\")" not in wan_ip4
ip4_modem_select_at = wan_ip4.index("if(!only_broadband_wan&&is_usb_modem)")
ip4_proto_read_at = wan_ip4.index("status=get_wan_unit_proto_checked_status(unit,&wan_proto)")
ip4_manual_gate_at = wan_ip4.index("if(ifname==NULL)")
ip4_manual_read_at = wan_ip4.index("get_wan_unit_value_copy_status(unit,\"ifname\",man_ifname_nvram,sizeof(man_ifname_nvram))")
ip4_manual_copy_at = wan_ip4.index("wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram)")
ip4_manual_assign_at = wan_ip4.index("ifname=man_ifname")
ip4_addr_lookup_at = wan_ip4.index("returnget_interface_addr4(ifname)")
require(ip4_modem_select_at < ip4_manual_gate_at < ip4_manual_read_at <
        ip4_manual_copy_at < ip4_manual_assign_at < ip4_addr_lookup_at and
        ip4_proto_read_at < ip4_manual_gate_at and
        "errno=status" in wan_ip4,
        "WAN IP lookup must select modem/PPP before checked manual fallback and preserve read errors")
require("test_wan_interface_names_survive_shared_cache_overwrite" in test,
        "WAN interface-name cache overwrite regression missing")

dns_proto = "get_wan_unit_proto_checked_status(unit, &wan_proto)"
require(dns_proto in dns_status and
        "if (status != 0)" in dns_status and
        dns_status.index(dns_proto) < dns_status.index("get_wan_unit_value_int_status(") and
        "get_wan_proto(unit)" not in dns_status,
        "WAN DNS mode must reject unknown protocol before applying its legacy DNS fallback")

wan_ifname_proto = "".join(body(source, "get_wan_ifname(char wan_ifname[16])").split())
ifname_proto_read = "status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
ifname_proto_error_gate = "if(status!=0){errno=status;returnstatus;}"
ifname_proto_read_at = wan_ifname_proto.index(ifname_proto_read)
ifname_proto_error_at = wan_ifname_proto.index(ifname_proto_error_gate, ifname_proto_read_at)
require(ifname_proto_read_at < ifname_proto_error_at <
        wan_ifname_proto.index("if(wan_proto==", ifname_proto_error_at) and
        "wan_proto=get_wan_proto(unit)" not in wan_ifname_proto,
        "WAN interface selection must not guess the MAN interface on protocol read failure")

wan_ip4 = "".join(body(source, "get_wan_ip4(int only_broadband_wan)").split())
ip4_proto_read = "status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
ip4_proto_read_at = wan_ip4.index(ip4_proto_read)
ip4_proto_error_gate = "if(status!=0){errno=status;returnINADDR_ANY;}"
ip4_proto_error_at = wan_ip4.index(ip4_proto_error_gate, ip4_proto_read_at)
require(ip4_proto_read in wan_ip4 and
        ip4_proto_read_at < ip4_proto_error_at < wan_ip4.index("get_interface_addr4(ifname)") and
        "get_wan_proto(unit)" not in wan_ip4,
        "WAN IP lookup must fail closed when protocol identity is unreadable")

require("get_usb_modem_wan(" not in source,
        "WAN networking code must not consume the lossy legacy modem boolean API")

reconnect = "".join(body(source, "notify_on_internet_state_changed(int has_internet, long elapsed)").split())
reconnect_read = "modem_identity_status=get_usb_modem_wan_status(0,&is_modem_wan)"
require(reconnect_read in reconnect and "if(modem_identity_status!=0)" in reconnect and
        reconnect.index(reconnect_read) < reconnect.index('queue_internet_lost_action("auto_wan_reconnect"'),
        "automatic reconnect must not publish modem priority after an identity read error")

link_restored = "".join(body(source, "notify_on_wan_ether_link_restored(void)").split())
link_proto_read = "status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
require(link_proto_read in link_restored and
        "if(status!=0)" in link_restored and
        "returnstatus;" in link_restored and
        link_restored.index(link_proto_read) < link_restored.index("renew_udhcpc_wan(unit)"),
        "link-restored callback must not renew DHCP on unknown WAN protocol")

deconfig = "".join(body(source, "udhcpc_deconfig(char *wan_ifname, int is_zcip)").split())
deconfig_proto_read = "proto_status=get_wan_unit_proto_checked_status(unit,&wan_proto)"
preserve_underlay = "proto_status!=0||wan_proto==IPV4_WAN_PROTO_PPTP||wan_proto==IPV4_WAN_PROTO_L2TP"
cleanup_call = "man_down_internal(wan_ifname,unit,reset_man_address)"
require(deconfig_proto_read in deconfig and
        "if(proto_status!=0)" in deconfig and
        preserve_underlay in deconfig and
        deconfig.index(deconfig_proto_read) < deconfig.index("ifconfig(wan_ifname,IFUP,") and
        cleanup_call in deconfig and
        deconfig.index(cleanup_call) > deconfig.index("leaseislost") and
        "route_status_accumulate(proto_status" in deconfig,
        "DHCP deconfig must preserve MAN underlay on protocol errors while still cleaning routes")


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


# WAN teardown must not treat a failed global MAN DNS read as an empty DNS
# setting, because that would erase the per-WAN demand-PPP DNS snapshot.
wan_down = body(source, "wan_down_internal(char *wan_ifname, int unit, int is_static)")
wan_down_compact = "".join(wan_down.split())
dns_policy = body(
    (ROOT / "trunk/user/rc/src/wan_teardown_policy.h").read_text(),
    "wan_teardown_should_clear_dns_snapshot(int force_clear",
)
require(
    'rc_wan_nvram_copy_value(wan_nvram_checked_get_callback,NULL,"wanx_dns",wanx_dns,sizeof(wanx_dns))'
    in wan_down_compact,
    "WAN teardown must use a checked bounded global MAN DNS read",
)
require(
    "wan_teardown_should_clear_dns_snapshot(" in wan_down_compact
    and "if(route_status!=0)" in wan_down_compact
    and "status=route_status_accumulate(status,route_status);" in wan_down_compact
    and "elseif(clear_wan_dns)" in wan_down_compact,
    "WAN teardown must preserve/report DNS-read failure and gate indexed DNS removal",
)
assert 'strlen(nvram_safe_get("wanx_dns"))' not in wan_down
assert 'nvram_safe_get("wanx_dns")' not in wan_down
for token in (
    "if (!clear_dns)",
    "*clear_dns = 0;",
    "if (force_clear)",
    "if (read_status != 0)",
    "if (!global_dns)",
    "strlen(global_dns) > 6",
):
    require(token in dns_policy, "WAN DNS teardown decision is incomplete: " + token)
teardown_test = (
    ROOT / "trunk/user/rc/src/tests/wan_teardown_policy.c"
).read_text()
for case in (
    "test_ipoe_dns_snapshot_is_always_cleared",
    "test_ppp_dns_snapshot_is_cleared_only_when_man_dns_is_present",
    "test_ppp_dns_snapshot_survives_global_read_failure",
    "test_dns_snapshot_clear_policy_rejects_invalid_arguments",
):
    require(case in teardown_test, "WAN DNS teardown regression missing " + case)
print("WAN teardown preserves demand-PPP DNS when global MAN DNS cannot be read: PASS")


# PPPoE-MWAN enable is a WAN event gate. A failed exact-string read must not
# collapse to the old nvram_match() false result and silently skip detect.sh.
match_policy = body(
    policy,
    "rc_wan_nvram_match_status(int read_status, const char *value",
)
for token in (
    "if (!matches || !expected)",
    "*matches = 0;",
    "if (read_status != 0)",
    "if (!value)",
    "strcmp(value, expected) == 0",
):
    require(token in match_policy, "exact NVRAM match policy missing " + token)
match_reader = body(
    source,
    "get_nvram_match_checked_status(const char *param_name",
)
for token in (
    "nvram_get_checked(param_name, &stored_value)",
    "rc_wan_nvram_read_status(api_result, saved_errno)",
    "rc_wan_nvram_match_status(read_status, NULL",
    'stored_value = "";',
    "rc_wan_nvram_match_status(0, stored_value",
):
    require(token in match_reader, "checked exact-match reader missing " + token)
for signature, phase in (
    ("wan_up_internal(char *wan_ifname, int unit, int is_static)", "WAN-up"),
    ("wan_down_internal(char *wan_ifname, int unit, int is_static)", "WAN-down"),
):
    event = "".join(body(source, signature).split())
    read = 'get_nvram_match_checked_status("pppoemwan_enable","1",&pppoemwan_enabled);'
    require(read in event, phase + " must read PPPoE-MWAN enable through checked exact match")
    require("if(pppoemwan_status!=0)" in event and
            "status=route_status_accumulate(status,pppoemwan_status);" in event,
            phase + " must propagate PPPoE-MWAN enable read errors")
    require("elseif(pppoemwan_enabled)" in event and
            'eval("/usr/bin/detect.sh")' in event,
            phase + " must preserve enabled launch / disabled no-op behavior")
    require(event.index(read) < event.index("if(pppoemwan_status!=0)") <
            event.index("elseif(pppoemwan_enabled)") <
            event.index('eval("/usr/bin/detect.sh")'),
            phase + " PPPoE-MWAN decision ordering changed")
    assert 'nvram_match("pppoemwan_enable", "1")' not in body(source, signature)
for case in (
    "test_checked_string_match_requires_an_exact_value",
    "test_checked_string_match_preserves_backend_errors",
    "test_checked_string_match_rejects_invalid_arguments",
):
    require(case in test, "exact NVRAM match regression missing " + case)
print("WAN PPPoE-MWAN event gates preserve checked exact-string NVRAM status: PASS")


# Route ownership must use one locked tuple; PPP keeps a checked address for conntrack.
wan_down = body(source, "wan_down_internal(char *wan_ifname, int unit, int is_static)")
wan_down_compact = "".join(wan_down.split())
route_snapshot = body(source, "wan_unit_route_snapshot(int unit,")
route_snapshot_compact = "".join(route_snapshot.split())
for token in (
    '"wan%d_gateway"',
    '"wan%d_ipaddr"',
    '"wan%d_netmask"',
    "rc_wan_nvram_copy_snapshot(",
    "wan_nvram_transaction_begin_callback",
    "wan_nvram_transaction_get_callback",
    "wan_nvram_transaction_end_callback",
):
    require(token in route_snapshot_compact,
            "WAN route snapshot must use one NVRAM transaction: " + token)
require(
    "wan_unit_route_snapshot(unit,wan_gate,sizeof(wan_gate),wan_addr,sizeof(wan_addr),wan_mask,sizeof(wan_mask))"
    in wan_down_compact,
    "non-PPP WAN teardown must use the coherent route tuple",
)
require(
    "wan_addr_status=route_snapshot_status;" in wan_down_compact
    and "wan_mask_status=route_snapshot_status;" in wan_down_compact
    and "if(route_snapshot_status!=0)" in wan_down_compact
    and "status=route_status_accumulate(status,route_snapshot_status);" in wan_down_compact,
    "failed tuple snapshot must be reported and must deny gateway /32 cleanup",
)
require(
    'get_wan_unit_value_copy_status(unit,"gateway",wan_gate,sizeof(wan_gate))' in wan_down_compact
    and 'get_wan_unit_value_copy_status(unit,"ipaddr",wan_addr,sizeof(wan_addr))' in wan_down_compact,
    "WAN teardown must retain checked gateway and PPP-address fallbacks",
)
require(
    'get_wan_unit_value_copy_status(unit,"netmask",wan_mask,sizeof(wan_mask))' not in wan_down_compact,
    "netmask must not be independently read outside the route tuple",
)
require(
    "cannot read coherent WAN route snapshot during teardown" in wan_down
    and "cannot read WAN gateway fallback during teardown" in wan_down
    and "cannot read WAN IPv4 address snapshot during teardown" in wan_down,
    "WAN route snapshot and PPP fallback failures must be visible",
)
ppp_pos = wan_down.index("if (ppp_index < 0) {")
ppp_else = wan_down.index("} else {", ppp_pos)
ppp_branch_compact = "".join(wan_down[ppp_else:].split())
require(
    'get_wan_unit_value_copy_status(unit,"ipaddr",wan_addr,sizeof(wan_addr))' in ppp_branch_compact,
    "PPP teardown must retain a checked conntrack address read",
)
require(
    "if(wan_addr_status==0&&is_valid_ipv4(wan_addr))" in wan_down_compact
    and 'snprintf(wan_addr,sizeof(wan_addr),"0.0.0.0")' in wan_down_compact,
    "conntrack flush must require a checked valid previous WAN address",
)
gateway_policy = body(
    (ROOT / "trunk/user/rc/src/wan_teardown_policy.h").read_text(),
    "wan_teardown_gateway_host_route_required(int gateway_read_status",
)
for token in (
    "gateway_read_status == 0",
    "address_read_status == 0",
    "netmask_read_status == 0",
    "gateway_valid",
    "address_valid",
    "netmask_valid",
    "ppp_index < 0",
    "gateway_off_subnet",
):
    require(token in gateway_policy, "WAN host-route read gate missing " + token)
require(
    "wan_teardown_gateway_host_route_required(" in wan_down
    and "route_status = route_status_idempotent_delete(route_status);" in wan_down,
    "host-route cleanup must retain its checked policy and idempotent delete semantics",
)
require(
    "test_gateway_host_route_requires_a_complete_checked_snapshot" in teardown_test,
    "WAN gateway host-route checked-read regression is missing",
)
require(
    "test_transactional_wan_route_snapshot_reads_one_generation" in
    (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text(),
    "WAN transactional route snapshot regression is missing",
)
print("WAN route ownership uses a coherent tuple while preserving PPP conntrack cleanup: PASS")


# IPv6 service-mode predicates control reset, DNS publication and WAN route
# selection. Do not let a checked NVRAM failure collapse into "IPv6 disabled".
wan6_source = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
ipv6_policy = (ROOT / "trunk/user/rc/src/wan_ipv6_service_policy.h").read_text()
ipv6_test = (ROOT / "trunk/user/rc/src/tests/wan_ipv6_service_policy.c").read_text()
rc_header = (ROOT / "trunk/user/rc/src/rc.h").read_text()

checked_match = body(
    wan6_source,
    "wan6_read_nvram_match_status(const char *param_name, const char *expected,",
)
for token in ("nvram_get_checked(param_name, &value)",
              "rc_wan_nvram_read_status(api_result, saved_errno)",
              "rc_wan_nvram_match_status(0, value, expected, matches)"):
    require(token in checked_match,
            "IPv6 mode option read must preserve NVRAM errors: " + token)

addr_mode = body(wan6_source, "is_wan_addr6_static_status(int *is_static)")
dns_mode = body(wan6_source, "is_wan_dns6_static_status(int *is_static)")
sit_mode = body(wan6_source, "is_wan_ipv6_type_sit_status(int *is_sit)")
for name, mode in (("address", addr_mode), ("DNS", dns_mode), ("SIT", sit_mode)):
    require("get_ipv6_type_status(&ipv6_type)" in mode,
            name + " mode selector must use the checked IPv6 service API")
    require("get_ipv6_type()" not in mode,
            name + " mode selector must not use the legacy error-collapsing getter")

require('wan6_read_nvram_match_status("ip6_6rd_dhcp","0",&sixrd_is_manual)' in
        "".join(addr_mode.split()),
        "6rd address mode must use a checked DHCP-mode read")
require('wan6_read_nvram_match_status("ip6_dns_auto","0",&dns_auto_is_manual)' in
        "".join(dns_mode.split()),
        "native DHCPv6 DNS mode must use a checked auto/manual read")

reset_ipv6 = body(wan6_source, "reset_wan6_vars_status(void)")
require("is_wan_addr6_static_status(&wan_addr6_static)" in reset_ipv6 and
        "if (status != 0)" in reset_ipv6 and
        reset_ipv6.index("if (status != 0)") <
        reset_ipv6.index('rc_wan_nvram_copy_text(nvram_safe_get("ip6_wan_addr")'),
        "IPv6 reset must abort before snapshot work when service mode cannot be read")
dns_export = body(wan6_source, "build_dns6_value(char *dns6s, size_t dns6_size)")
require("is_wan_dns6_static_status(&dns_static)" in dns_export and
        "if (status != 0)" in dns_export and
        dns_export.index("if (status != 0)") <
        dns_export.index('rc_wan_nvram_copy_text(nvram_safe_get("ip6_dns1")'),
        "DNS publication must stop before mode-dependent DNS reads on error")

for name in ("is_wan_addr6_static_status", "is_wan_dns6_static_status",
             "is_wan_ipv6_type_sit_status"):
    require(name + "(int *" in rc_header,
            "checked WAN IPv6 predicate declaration missing: " + name)

for token in ("wan_ipv6_service_is_sit_status",
              "wan_ipv6_service_addr_static_status",
              "wan_ipv6_service_dns_static_status",
              "if (read_status != 0)",
              "sixrd_dhcp_read_status != 0",
              "dns_auto_read_status != 0"):
    require(token in ipv6_policy,
            "WAN IPv6 service policy missing " + token)

for case in ("test_service_read_error_is_not_mapped_to_disabled",
             "test_sit_classification_preserves_all_transport_modes",
             "test_address_static_classification_preserves_legacy_rules",
             "test_dns_static_classification_checks_only_manual_native_dhcp_mode",
             "test_null_outputs_fail_closed"):
    require(case in ipv6_test, "WAN IPv6 service regression missing " + case)

wan_ipv6_callers = (
    ("launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)", "route_status_accumulate"),
    ("start_wan(void)", "rc_first_service_start_status"),
    ("stop_wan_locked(void)", "goto abort_resume_detect"),
    ("wan_up_internal(char *wan_ifname, int unit, int is_static)", "route_status_accumulate"),
    ("wan_down_internal(char *wan_ifname, int unit, int is_static)", "wan6_down"),
)
for signature, required_effect in wan_ipv6_callers:
    caller = "".join(function_body_at_column_zero_terminator(source, signature).split())
    require("is_wan_ipv6_type_sit_status(&ipv6_is_sit)" in caller,
            signature + " must consume the checked service-mode result")
    require("is_wan_ipv6_type_sit()" not in caller,
            signature + " must not use the legacy error-collapsing wrapper")
    require(required_effect.replace(" ", "") in caller,
            signature + " must propagate or handle a mode-read error")
    require("if(ipv6_type_status!=0)" in caller,
            signature + " must branch on a failed mode read")

stop_mode = "".join(
    function_body_at_column_zero_terminator(source, "stop_wan_locked(void)").split()
)
stop_mode_error = stop_mode.index("if(ipv6_type_status!=0)")
stop_mode_fence = stop_mode.index(
    "ipv6_status=wan6_down(man_ifname,unit)", stop_mode_error
)
stop_mode_abort = stop_mode.index("gotoabort_resume_detect;", stop_mode_fence)
require(stop_mode_error < stop_mode_fence < stop_mode_abort,
        "WAN stop must fence DHCPv6 callbacks before aborting on a mode-read error")
down_mode = "".join(
    function_body_at_column_zero_terminator(
        source, "wan_down_internal(char *wan_ifname, int unit, int is_static)"
    ).split()
)
require(down_mode.index("ipv6_status=wan6_down(wan_ifname,unit)") <
        down_mode.index("cannotdetermineWANIPv6transportduringteardown"),
        "WAN-down must attempt best-effort DHCPv6 callback stop on a mode-read error")

require(
    not re.search(r"\bis_wan_(?:addr6_static|dns6_static|ipv6_type_sit)\s*\(",
                  reset_ipv6 + dns_export),
    "state-changing WAN IPv6 paths must use checked mode predicates",
)
require(
    not re.search(r"\bget_ipv6_type\s*\(", wan6_source),
    "net_wan6.c must not consume the legacy IPv6 service getter",
)

wisp_status_api = body(source, "get_wan_wisp_active_status(int *p_is_active, int *p_has_link)")
wisp_status_code = "".join(wisp_status_api.split())
wisp_read = 'read_status=get_wan_unit_value_copy_status(unit,"ifname",man_ifname_nvram,sizeof(man_ifname_nvram));'
wisp_read_at = wisp_status_code.index(wisp_read)
wisp_read_gate_at = wisp_status_code.index("if(read_status==0&&man_ifname_nvram[0])", wisp_read_at)
wisp_validate = "validation_status=wan_ifname_copy(man_ifname,sizeof(man_ifname),man_ifname_nvram);"
wisp_validate_at = wisp_status_code.index(wisp_validate, wisp_read_gate_at)
wisp_classify_at = wisp_status_code.index("is_wisp=is_man_wisp(man_ifname);", wisp_validate_at)
wisp_result = "status=rc_wan_nvram_wisp_active_status(read_status,validation_status,is_wisp,p_is_active);"
wisp_result_at = wisp_status_code.index(wisp_result, wisp_classify_at)
wisp_error_at = wisp_status_code.index("if(status!=0)", wisp_result_at)
wisp_link_at = wisp_status_code.index("*p_has_link=get_apcli_connected(man_ifname);", wisp_error_at)
require(wisp_read_at < wisp_read_gate_at < wisp_validate_at < wisp_classify_at <
        wisp_result_at < wisp_error_at < wisp_link_at and
        "*p_is_active=0;" in wisp_status_code and
        "get_man_ifname" not in wisp_status_api,
        "WISP classification must retain checked MAN-read errors and validate before AP-client link lookup")

legacy_wisp = "".join(body(source, "get_wan_wisp_active(int *p_has_link)").split())
require("get_wan_wisp_active_status(&is_active,p_has_link)" in legacy_wisp and
        "if(status!=0)" in legacy_wisp and "return1;" in legacy_wisp,
        "legacy WISP boolean must conservatively classify unknown interface identity as active")

led_wisp = "".join(body(source, "control_wan_led_isp_state(int is_wan_up, int is_modem_unit)").split())
led_read = "wisp_status=get_wan_wisp_active_status(&is_wisp_active,&wisp_has_link);"
led_error = led_wisp.index("if(wisp_status!=0)")
led_active = led_wisp.index("elseif(is_wisp_active)", led_error)
led_fallback = led_wisp.index("elseif(!is_modem_unit)", led_active)
require(led_read in led_wisp and led_error < led_active < led_fallback and
        "get_wan_wisp_active(" not in led_wisp and "has_link=0;" in led_wisp[led_error:led_active],
        "WAN LED must suppress cached Ethernet fallback when MAN identity cannot be read")

renew_wisp = "".join(body(source, "notify_on_wan_ether_link_restored(void)").split())
renew_read = "status=get_wan_wisp_active_status(&is_wisp_active,NULL);"
renew_read_at = renew_wisp.index(renew_read)
renew_error_at = renew_wisp.index("if(status!=0)", renew_read_at)
renew_active_at = renew_wisp.index("if(is_wisp_active)", renew_error_at)
renew_dhcp_at = renew_wisp.index("status=renew_udhcpc_wan(unit)", renew_active_at)
require(renew_read_at < renew_error_at < renew_active_at < renew_dhcp_at and
        "get_wan_wisp_active(" not in renew_wisp,
        "Ethernet link restoration must not renew DHCP when WISP identity is unknown")

renew_selector = "".join(body(source, "notify_on_wan_ether_link_restored(void)").split())
static_proto_guard = "if(wan_proto==IPV4_WAN_PROTO_IPOE_STATIC)return0;"
selector_read = 'status=get_nvram_match_checked_status("x_DHCPClient","0",&manual_dhcp_disabled);'
selector_read_at = renew_selector.index(selector_read)
selector_error_at = renew_selector.index("if(status!=0)", selector_read_at)
selector_disabled_at = renew_selector.index("if(manual_dhcp_disabled)return0;", selector_error_at)
ifname_temp_read = 'status=get_wan_unit_value_copy_status(unit,"ifname_t",wan_ifname_temp,sizeof(wan_ifname_temp));'
ifname_temp_read_at = renew_selector.index(ifname_temp_read, selector_disabled_at)
ifname_temp_error_at = renew_selector.index("if(status!=0)", ifname_temp_read_at)
usbnet_guard_at = renew_selector.index("if(is_usbnet_interface(wan_ifname_temp))", ifname_temp_error_at)
wisp_read_at = renew_selector.index("status=get_wan_wisp_active_status(", usbnet_guard_at)
dhcp_start_at = renew_selector.index("status=renew_udhcpc_wan(unit)", wisp_read_at)
require(static_proto_guard in renew_selector and
        renew_selector.index(static_proto_guard) < selector_read_at <
        selector_error_at < selector_disabled_at < ifname_temp_read_at <
        ifname_temp_error_at < usbnet_guard_at < wisp_read_at < dhcp_start_at and
        'nvram_match("x_DHCPClient","0")' not in renew_selector and
        'get_wan_unit_value(unit,"ifname_t")' not in renew_selector and
        "cannot read manual WAN DHCP-client selector" in renew_selector and
        "cannot read WAN transport interface" in renew_selector,
        "link-restored DHCP renewal must check both the manual-client selector and transport-interface NVRAM before classifying the device")

require("int get_wan_wisp_active_status(int *p_is_active, int *p_has_link);" in rc_header,
        "checked WISP status API must be declared in rc.h")
require("rc_wan_nvram_wisp_active_status" in policy and
        "test_wisp_active_status_requires_checked_identity" in test and
        "test_wisp_active_status_requires_checked_identity();" in test,
        "checked WISP status mapping must have a registered error-precedence regression")

print("WAN IPv6 and WISP checked interface identity/error propagation: PASS")
