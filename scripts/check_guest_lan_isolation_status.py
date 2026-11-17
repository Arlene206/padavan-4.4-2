#!/usr/bin/env python3
"""Verify guest-LAN isolation command results reach boot, Wi-Fi and PID-1 callers."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"

wifi = (RC / "net_wifi.c").read_text()
lan = (RC / "net_lan.c").read_text()
rc_source = (RC / "rc.c").read_text()
rc_header = (RC / "rc.h").read_text()
policy = (RC / "wireless_guest_isolation_policy.h").read_text()
regression = (RC / "tests/wireless_guest_isolation_policy.c").read_text()


def function_body(source, signature, last=False):
    start = source.rfind(signature) if last else source.find(signature)
    if start < 0:
        raise AssertionError("missing function: " + signature)
    brace = source.find("{", start + len(signature))
    if brace < 0:
        raise AssertionError("missing function body: " + signature)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


assert '#include "wireless_guest_isolation_policy.h"' in wifi
assert "int restart_guest_lan_isolation(void);" in rc_header
assert "void restart_guest_lan_isolation(void);" not in rc_header

record = function_body(wifi, "static void\nwireless_guest_isolation_record(", last=True)
assert "wireless_guest_isolation_status_record(status, operation, operation_status);" in record
filter_body = function_body(wifi, "static void\nebtables_filter_guest_ap(")
assert filter_body.count("doSystem(") == filter_body.count("wireless_guest_isolation_record(")
isolation_body = function_body(wifi, "int\nrestart_guest_lan_isolation(void)")
for token in (
    'operation_status = brport_set_param_int(wl_ifname_guest, "isolate_mode", bp_isolate);',
    'wireless_guest_isolation_record(&status, "set 5GHz guest bridge isolate_mode"',
    'operation_status = brport_set_param_int(rt_ifname_guest, "isolate_mode", bp_isolate);',
    'wireless_guest_isolation_record(&status, "set 2.4GHz/iNIC guest bridge isolate_mode"',
    'wireless_guest_isolation_record(&status, "load ebtables filter module"',
    'wireless_guest_isolation_record(&status, "flush ebtables rules before guest isolation"',
    'wireless_guest_isolation_record(&status, "delete ebtables user chains before guest isolation"',
    'wireless_guest_isolation_record(&status, "flush ebtables rules while disabling guest isolation"',
    'wireless_guest_isolation_record(&status, "delete ebtables user chains while disabling guest isolation"',
    'wireless_guest_isolation_record(&status, "unload ebt_ip after clearing guest isolation"',
    'wireless_guest_isolation_record(&status, "unload ebtables filter after clearing guest isolation"',
    'wireless_guest_isolation_record(&status, "unload ebtables after clearing guest isolation"',
):
    assert token in isolation_body, token
assert "if (cleanup_status == 0)" in isolation_body
assert "return wireless_guest_isolation_status_result(&status);" in isolation_body
assert "failed_operations=%u" in isolation_body

for signature, finish_label in (
    ("int\ncontrol_guest_wl(int guest_on, int manual)", "finish_wl"),
    ("int\ncontrol_guest_rt(int guest_on, int manual)", "finish_rt"),
):
    body = function_body(wifi, signature)
    assert "br_add_del_if(" in body
    assert "wireless_guest_isolation_status_record(&control_status" in body
    assert "restart_guest_lan_isolation()" in body
    assert "rollback_candidate = 1;" in body
    assert "if (guest_on && rollback_candidate && control_status.first_status != 0)" in body
    assert "wireless_guest_ap_rollback(ifname_ap, bridge_ifname, &control_status);" in body
    assert "wireless_guest_isolation_control_result(is_ap_changed, &control_status)" in body
    down_gate = "if (!wif_control_success(ifname_ap, 0))"
    down_pos = body.index(down_gate)
    down_record = body.index("wireless_guest_isolation_status_record(&control_status", down_pos)
    detach_pos = body.index("br_add_del_if(", down_record)
    assert down_pos < down_record < detach_pos
    assert "goto " + finish_label + ";" not in body[down_record:detach_pos]

rollback_helper = function_body(wifi, "static void\nwireless_guest_ap_rollback(")
assert "wireless_guest_isolation_status_rollback(status, &ops, &context)" in rollback_helper
assert "br_add_del_if(IFNAME_BR, rollback->bridge_ifname, 0)" in wifi
rt_bridge = function_body(wifi, "static const char *\nwireless_guest_rt_bridge_ifname(int is_ap_mode)")
assert "if (!is_ap_mode)" in rt_bridge and "IFNAME_INIC_GUEST_VLAN" in rt_bridge

restart_rollback = function_body(wifi, "static int\nwireless_guest_restart_fail_closed(")
assert "wireless_guest_ap_rollback(IFNAME_5G_GUEST" in restart_rollback
assert "wireless_guest_ap_rollback(IFNAME_2G_GUEST" in restart_rollback
assert "restart_guest_lan_isolation()" in restart_rollback

for signature in ("int\nrestart_wifi_wl(int radio_on, int need_reload_conf)",
                  "int\nrestart_wifi_rt(int radio_on, int need_reload_conf)"):
    body = function_body(wifi, signature)
    assert "wireless_guest_restart_fail_closed(" in body
assert "return brport_set_m2u(wifname, i_m2u);" in wifi

for signature in ("int\nrestart_wifi_wl(int radio_on, int need_reload_conf)",
                  "int\nrestart_wifi_rt(int radio_on, int need_reload_conf)"):
    body = function_body(wifi, signature)
    assert "isolation_status = restart_guest_lan_isolation();" in body
    assert "return isolation_status;" in body

init_bridge = function_body(lan, "int\ninit_bridge(int is_ap_mode)")
assert 'rc_bridge_init_record(&bridge_status, "guest LAN isolation", operation_status);' in init_bridge
assert "operation_status = restart_guest_lan_isolation();" in init_bridge

notification_body = function_body(rc_source, "handle_notifications(void)")
for call in (
    "control_guest_wl(guest_on, 1)",
    "control_guest_rt(guest_on, 1)",
    "control_guest_wl(1, 0)",
    "control_guest_wl(0, 0)",
    "control_guest_rt(1, 0)",
    "control_guest_rt(0, 0)",
):
    assert call + " < 0" in notification_body, call

for test_name in (
    "test_empty_and_success_status",
    "test_first_failure_and_independent_later_operations",
    "test_positive_system_wait_status_is_a_failure",
    "test_later_failures_do_not_replace_first_error",
    "test_control_result_preserves_changed_boolean_only_on_success",
    "test_fail_closed_rollback_attempts_every_step",
    "test_fail_closed_rollback_success_and_invalid_ops",
    "test_null_status_is_fail_safe",
):
    assert test_name in regression
assert "wireless_guest_isolation_status_record" in policy
assert "wireless_guest_isolation_status_rollback" in policy


for signature, operations in (
    ("int\\nstart_wifi_ap_wl(int radio_on)", (
        "wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_MAIN))",
        "wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_GUEST))",
    )),
    ("int\\nstart_wifi_ap_rt(int radio_on)", (
        "wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_MAIN))",
        "wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_GUEST))",
    )),
):
    startup = body(wifi, signature)
    for operation in operations:
        assert operation in startup, operation
        assert "wireless_radio_startup_status(all_started," in startup

guest_commands = {
    'radio2_guest_enable': 'ret = manual_change_guest_rt(1) < 0 ? -1 : 0;',
    'radio2_guest_disable': 'ret = manual_change_guest_rt(0) < 0 ? -1 : 0;',
    'radio5_guest_enable': 'ret = manual_change_guest_wl(1) < 0 ? -1 : 0;',
    'radio5_guest_disable': 'ret = manual_change_guest_wl(0) < 0 ? -1 : 0;',
}
for command, status_assignment in guest_commands.items():
    branch = rc_source.index('!strcmp(base, "' + command + '")')
    next_branch = rc_source.find('\n\telse if ', branch)
    if next_branch < 0:
        next_branch = len(rc_source)
    assert status_assignment in rc_source[branch:next_branch], command
assert "return ret;" in function_body(rc_source, "main(int argc, char **argv)")
print("Guest-LAN bridge, ebtables and PID-1 status propagation contract: PASS")
