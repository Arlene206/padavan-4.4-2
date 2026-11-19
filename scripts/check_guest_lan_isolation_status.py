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
assert "wireless_guest_isolation_ebtables_rollback_required(" in isolation_body
assert "ebtables_failures_before = status.failure_count;" in isolation_body
ebtables_baseline = isolation_body.index("ebtables_failures_before = status.failure_count;")
ebtables_module_load = isolation_body.index('(void)module_smart_load("ebtable_filter", NULL);')
ebtables_rollback = isolation_body.index("wireless_guest_isolation_ebtables_rollback_required(")
assert ebtables_baseline < ebtables_module_load < ebtables_rollback
assert 'wireless_guest_ap_rollback(IFNAME_5G_GUEST, IFNAME_5G_GUEST, &status);' in isolation_body
assert 'wireless_guest_ap_rollback(wireless_guest_rt_interface_ifname(),' in isolation_body
assert 'taking configured guest APs offline' in isolation_body
assert 'is_need_ebtables |= 0x10;' in isolation_body
assert 'is_need_ebtables |= 0x01;' in isolation_body
assert 'is_module_loaded("ebtables")' not in isolation_body
assert '(void)module_smart_load("ebtable_filter", NULL);' in isolation_body
enable_load_pos = isolation_body.index('(void)module_smart_load("ebtable_filter", NULL);')
enable_probe_pos = isolation_body.index('ebtables_module_status = is_module_loaded_status("ebtable_filter");')
enable_error_pos = isolation_body.index("wireless_guest_isolation_module_presence_error(ebtables_module_status)", enable_probe_pos)
enable_record_pos = isolation_body.index('"load ebtables filter module"', enable_error_pos)
enable_guard_pos = isolation_body.index("if (ebtables_module_error != 0)", enable_record_pos)
enable_else_pos = isolation_body.index("} else {", enable_guard_pos)
enable_flush_pos = isolation_body.index('doSystem("ebtables %s", "-F")', enable_else_pos)
assert enable_load_pos < enable_probe_pos < enable_error_pos < enable_record_pos < enable_guard_pos < enable_else_pos < enable_flush_pos

disable_probe_pos = isolation_body.index('ebtables_module_status = is_module_loaded_status("ebtables");')
disable_action_pos = isolation_body.index("wireless_guest_isolation_ebtables_disable_action(ebtables_module_status)", disable_probe_pos)
disable_guard_pos = isolation_body.index("if (ebtables_disable_action != WIRELESS_GUEST_EBTABLES_DISABLE_NONE)", disable_action_pos)
disable_flush_pos = isolation_body.index('doSystem("ebtables %s", "-F")', disable_guard_pos)
disable_cleanup_pos = isolation_body.index("if (cleanup_status == 0)", disable_flush_pos)
unload_gate_pos = isolation_body.index(
    "if (ebtables_disable_action == WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_AND_UNLOAD)",
    disable_cleanup_pos)
assert disable_probe_pos < disable_action_pos < disable_guard_pos < disable_flush_pos < disable_cleanup_pos < unload_gate_pos
assert "WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY" in isolation_body
assert "inspect ebtables module state before cleanup" in isolation_body
assert "retaining ebtables modules because their loaded state is unknown" in isolation_body
assert "test_ebtables_enable_requires_confirmed_module_presence" in regression
assert "test_ebtables_disable_preserves_unknown_module_state" in regression
assert "wireless_guest_isolation_module_presence_error" in policy
assert "wireless_guest_isolation_ebtables_disable_action" in policy
assert "return wireless_guest_isolation_status_result(&status);" in isolation_body
assert isolation_body.count("wireless_guest_interface_state_checked(") == 3
assert "is_interface_up(" not in isolation_body
for diagnostic in (
    "inspect 5GHz guest AP state for LAN isolation",
    "inspect 2.4GHz guest AP state for LAN isolation",
    "inspect 2.4GHz/iNIC guest bridge port state",
):
    assert diagnostic in isolation_body
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

bring_down = function_body(wifi, "wireless_guest_rollback_bring_down(void *context)")
verify_down = function_body(wifi, "wireless_guest_rollback_verify_down(void *context)")
assert "wireless_guest_rollback_should_request_down(status, is_up)" in bring_down
assert "if (!wif_control_success(rollback->ifname_ap, 0))" in bring_down
assert "return status != 0 ? status : -EIO;" in bring_down
assert "wireless_guest_interface_state_checked(rollback->ifname_ap," in bring_down
assert "wireless_guest_rollback_should_request_down(status, is_up)" in bring_down
assert "if (!wif_control_success(rollback->ifname_ap, 0))" in bring_down
assert "wireless_guest_interface_state_checked(rollback->ifname_ap," in verify_down
assert "if (status != 0)" in verify_down
assert "is_interface_up(" not in bring_down
assert "is_interface_up(" not in verify_down

checked_state = function_body(wifi, "wireless_guest_interface_state_checked(const char *ifname, int *is_up)")
assert "errno = 0;" in checked_state
assert "interface_error = errno;" in checked_state
assert "get_interface_flags(ifname)" in checked_state
assert "wireless_guest_isolation_link_state(interface_flags, interface_error," in checked_state
assert "ENODEV, IFF_UP, is_up" in checked_state

for signature, main_if in (
    ("control_guest_wl(int guest_on, int manual)", "IFNAME_5G_MAIN"),
    ("control_guest_rt(int guest_on, int manual)", "IFNAME_2G_MAIN"),
):
    control = function_body(wifi, signature)
    assert "wireless_guest_interface_state_checked(" + main_if in control
    assert "wireless_guest_interface_state_checked(ifname_ap, &guest_is_up)" in control
    assert "if (state_status != 0 && guest_on)" in control
    assert "is_interface_up(" not in control
    assert "if (state_status != 0 || guest_is_up)" in control

rollback_helper = function_body(wifi, "static void\nwireless_guest_ap_rollback(")
assert "wireless_guest_isolation_status_rollback(status, &ops, &context)" in rollback_helper
assert "br_add_del_if(IFNAME_BR, rollback->bridge_ifname, 0)" in wifi
rt_interface = function_body(wifi, "wireless_guest_rt_interface_ifname(void)")
assert "return IFNAME_INIC_GUEST;" in rt_interface
assert "return IFNAME_2G_GUEST;" in rt_interface
boot_rollback = function_body(wifi, "wireless_guest_init_bridge_fail_closed(int rt_guest_was_up,")
assert "wireless_guest_startup_rollback_candidate(" in boot_rollback
assert "rt_guest_was_up, rt_guest_start_attempted" in boot_rollback
assert "wl_guest_was_up, wl_guest_start_attempted" in boot_rollback
assert "wireless_guest_rt_interface_ifname()" in boot_rollback
assert "wireless_guest_rt_bridge_ifname(get_ap_mode())" in boot_rollback
assert "restart_guest_lan_isolation()" in boot_rollback
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

init_bridge = function_body(lan, "int\ninit_bridge(int is_ap_mode, int *switch_vlan_status)")
assert 'rc_bridge_init_record(&bridge_status, "guest LAN isolation", guest_isolation_status);' in init_bridge
init_bridge_compact = " ".join(init_bridge.split())
assert "if (switch_vlan_status) *switch_vlan_status = 0;" in init_bridge_compact
assert "*switch_vlan_status = operation_status;" in init_bridge_compact
vlan_config = init_bridge_compact.index("operation_status = switch_config_vlan(1);")
vlan_capture = init_bridge_compact.index("*switch_vlan_status = operation_status;", vlan_config)
vlan_record = init_bridge_compact.index('rc_bridge_init_record(&bridge_status, "switch VLAN setup", operation_status);', vlan_capture)
assert vlan_config < vlan_capture < vlan_record
assert "rt_guest_state_status = wireless_guest_rt_guest_is_up(&rt_guest_was_up);" in init_bridge
assert "wl_guest_state_status = wireless_guest_wl_guest_is_up(&wl_guest_was_up);" in init_bridge
assert '"inspect 2.4GHz guest AP before startup"' in init_bridge
assert '"inspect 5GHz guest AP before startup"' in init_bridge
assert "if (rt_guest_state_status == 0)" in init_bridge
assert "if (wl_guest_state_status == 0)" in init_bridge
assert "rt_guest_start_attempted = rt_radio_on != 0;" in init_bridge
assert "wl_guest_start_attempted = wl_radio_on != 0;" in init_bridge
assert "rt_ap_start_status = start_wifi_ap_rt(rt_radio_on);" in init_bridge
assert "wl_ap_start_status = start_wifi_ap_wl(wl_radio_on);" in init_bridge
assert "wireless_guest_init_bridge_fail_closed(" in init_bridge
rt_snapshot_pos = init_bridge.index(
    "rt_guest_state_status = wireless_guest_rt_guest_is_up(&rt_guest_was_up);")
wl_snapshot_pos = init_bridge.index(
    "wl_guest_state_status = wireless_guest_wl_guest_is_up(&wl_guest_was_up);")
rt_start_pos = init_bridge.index("rt_ap_start_status = start_wifi_ap_rt(rt_radio_on);")
wl_start_pos = init_bridge.index("wl_ap_start_status = start_wifi_ap_wl(wl_radio_on);")
isolation_pos = init_bridge.index("guest_isolation_status = restart_guest_lan_isolation();")
rollback_pos = init_bridge.index("wireless_guest_init_bridge_fail_closed(")
assert rt_snapshot_pos < rt_start_pos < isolation_pos < rollback_pos
assert wl_snapshot_pos < wl_start_pos < isolation_pos < rollback_pos

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
    "test_startup_rollback_tracks_only_interfaces_started_here",
    "test_unknown_guest_link_state_still_requests_down",
    "test_ebtables_setup_failure_requires_rollback_of_enabled_guest_aps",
    "test_null_status_is_fail_safe",
):
    assert test_name in regression
assert "wireless_guest_isolation_status_record" in policy
assert "wireless_guest_isolation_status_rollback" in policy
assert "wireless_guest_rollback_should_request_down" in policy
assert "wireless_guest_isolation_ebtables_rollback_required" in policy
assert "wireless_guest_startup_rollback_candidate" in policy
assert "interface_error != absent_error" in policy


for signature, operations in (
    ("start_wifi_ap_wl(int radio_on)", (
        "wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_MAIN))",
        "wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_GUEST))",
    )),
    ("start_wifi_ap_rt(int radio_on)", (
        "wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_MAIN))",
        "wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_GUEST))",
    )),
):
    startup = function_body(wifi, signature)
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
