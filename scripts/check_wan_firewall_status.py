#!/usr/bin/env python3
"""Ensure WAN firewall rebuild and PPP recovery failures reach start_wan()."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"


def require(condition, message):
    if not condition:
        print(f"WAN firewall status contract: FAIL: {message}", file=sys.stderr)
        raise SystemExit(1)


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quoted":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line_comment":
            if char == chr(10):
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        index += 1
    raise AssertionError(f"unterminated C function: {signature}")


def positions(source, marker):
    found = []
    offset = 0
    while True:
        position = source.find(marker, offset)
        if position < 0:
            return found
        found.append(position)
        offset = position + len(marker)


wan = (RC / "net_wan.c").read_text()
wan_renew = body(wan, "udhcpc_renew(char *wan_ifname)")
require("wan_dhcp_nvram_state_publish(" in wan_renew and
        "struct dhcp_lease_nvram_value renew_values[4];" in wan_renew and
        "return nvram_status;" in wan_renew,
        "WAN DHCP renewal fields must be checked as one runtime snapshot")
wan_renew_publisher = body(wan, "wan_dhcp_nvram_state_publish(")
require("dhcp_lease_nvram_publish(" in wan_renew_publisher and
        "wan_dhcp_nvram_restore_values(" in wan_renew_publisher and
        "nvram_transaction_begin(&transaction)" in wan_renew_publisher,
        "WAN DHCP renewal snapshot needs transaction and full rollback")
for marker in (
    'nvram_set_temp(param, lease_dns)',
    'nvram_set_temp(param, lease_domain)',
    'nvram_set_temp(strcat_r(prefix, "wins", tmp), lease_wins)',
    'nvram_set_int_temp(strcat_r(prefix, "lease", tmp), (int)parsed_value)',
):
    require(marker not in wan_renew,
            "unchecked DHCP renewal option write remains: " + marker)

wan_bound = body(wan, "udhcpc_bound(char *wan_ifname, int is_renew_mode)")
require("wan_dhcp_nvram_state_publish(" in wan_bound and
        "snapshot_values[WAN_DHCP_BOUND_SNAPSHOT_COUNT]" in wan_bound and
        "previous_text[WAN_DHCP_BOUND_SNAPSHOT_COUNT]" in wan_bound,
        "WAN DHCP bound must stage the complete address-change lease tuple")
require(wan_bound.index("wan_dhcp_nvram_state_publish(") <
        wan_bound.index("if (!i_err)") and
        "dhcp_wan_route_transition_abort_snapshot(" in wan_bound,
        "DHCP route setup must wait for snapshot publication and roll back on failure")
require("foreach(route, routes_rfc_parse, value)" in wan_bound and
        "lease_routes_rfc" in wan_bound,
        "DHCP default-gateway discovery must consume the staged classless-route value")
for marker in (
    'nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), lease_ip)',
    'nvram_set_temp(strcat_r(prefix, "netmask", tmp), lease_netmask)',
    'nvram_set_temp(strcat_r(prefix, "dns", tmp), lease_dns)',
    'nvram_set_temp(strcat_r(prefix, "wins", tmp), lease_wins)',
    'nvram_set_temp(strcat_r(prefix, "routes", tmp), route_value)',
    'nvram_set_temp(strcat_r(prefix, "routes_ms", tmp), route_value)',
    'nvram_set_temp(strcat_r(prefix, "routes_rfc", tmp), route_value)',
    'nvram_set_temp(strcat_r(prefix, "domain", tmp), lease_domain)',
    'nvram_set_int_temp(strcat_r(prefix, "lease", tmp), lease_dur)',
    'nvram_set_temp(strcat_r(prefix, "gateway", tmp), gateway_buf)',
):
    require(marker not in wan_bound,
            "unchecked DHCP bound snapshot field write remains: " + marker)

wan_route_test = (RC / "tests/route_status_policy.c").read_text()

net_core = (RC / "net.c").read_text()
ppp = (RC / "net_ppp.c").read_text()
ppp_ipup_policy = (RC / "ppp_wan_ipup_policy.h").read_text()
ppp_ipup_test = (RC / "tests/ppp_wan_ipup_policy.c").read_text()
header = (RC / "rc.h").read_text()
policy = (RC / "service_start_status.h").read_text()
policy_test = (RC / "tests/service_start_status.c").read_text()
route_policy = (RC / "wan_route_flush_policy.h").read_text()
route_policy_test = (RC / "tests/wan_route_flush_policy.c").read_text()
nvram_policy = (RC / "wan_nvram_status_policy.h").read_text()
nvram_policy_test = (RC / "tests/wan_nvram_status_policy.c").read_text()
require("rc_wan_firewall_start_allows_protocol" in policy,
        "WAN protocol start policy must require a successful firewall rebuild")
require("test_wan_firewall_failure_blocks_protocol_launch" in policy_test and
        "test_wan_firewall_failure_blocks_protocol_launch();" in policy_test,
        "WAN firewall failure gate must have a registered policy regression")
require("test_static_ipoe_eapol_cleanup_failure_forces_interface_down" in policy_test and
        "test_static_ipoe_eapol_cleanup_failure_forces_interface_down();" in policy_test,
        "static IPoE EAPOL address cleanup must have a registered policy regression")
require("test_static_ipoe_address_cleanup_failure_blocks_eapol" in policy_test and
        "test_static_ipoe_address_cleanup_failure_blocks_eapol();" in policy_test,
        "partial static IPoE address cleanup must have a registered protocol-gating regression")
require("int start_firewall_ex(void);" in header,
        "firewall rebuild API must expose its status")

restore = body(wan, "restore_wan_after_ppp_start_failure(int unit, char *wan_ifname)")
firewall_recovery = body(wan, "restore_wan_after_firewall_failure(int unit, char *wan_ifname)")
for marker in (
    "restore_status = restore_wan_after_ppp_start_failure(unit, wan_ifname);",
    "if (restore_status == 0)",
    "down_status = ifconfig(wan_ifname, 0, NULL, NULL);",
    "return rc_first_service_start_status(restore_status, down_status);",
):
    require(marker in firewall_recovery,
            f"failed physical-WAN firewall restoration must fail closed: {marker}")

for marker in (
    "upnp_status = check_upnp_wanif_changed(wan_ifname);",
    "if (upnp_status != 0)",
    "return upnp_status;",
    "firewall_status = start_firewall_ex();",
    "return firewall_status;",
):
    require(marker in restore, f"PPP physical-WAN recovery must preserve {marker}")

start = body(wan, "start_wan(void)")

# Protocol selection must never consume a stale wanN_proto after a failed
# copy from the configured wan_proto key. Preserve the public void
# reset_wan_vars() ABI, but retain a private failure latch for boot callers.
reset = body(wan, "reset_wan_vars_status(void)")
proto_write = 'proto_status = set_wan_unit_value_status(\n\t\tunit, "proto", nvram_safe_get("wan_proto"));'
proto_failure = "if (proto_status != 0)"
proto_read = "wan_proto = get_wan_proto(unit);"
require(proto_write in reset and proto_failure in reset and proto_read in reset,
        "WAN reset must check the configured protocol copy before reading it")
proto_write_at = reset.index(proto_write)
proto_failure_at = reset.index(proto_failure, proto_write_at)
proto_return_at = reset.index("return proto_status;", proto_failure_at)
proto_read_at = reset.index(proto_read)
require(reset.index("wan_vars_reset_status = 0;") < proto_write_at <
        proto_failure_at < proto_return_at < proto_read_at,
        "failed WAN protocol publication must latch and return before protocol selection")
for marker in (
    "wan_vars_reset_status = proto_status;",
    "errno = proto_status;",
    "cannot stage configured WAN protocol before reset",
):
    require(marker in reset, f"failed protocol staging must preserve diagnosis: {marker}")
reset_wrapper = body(wan, "reset_wan_vars(void)")
require("(void)reset_wan_vars_status();" in reset_wrapper,
        "legacy void reset_wan_vars wrapper must delegate to status implementation")
require("void reset_wan_vars(void);" in header and
        "int reset_wan_vars_status" not in header,
        "preserve public reset_wan_vars ABI")

reset_gate = "if (wan_vars_reset_status != 0)"
sysctl_start = start.index("sysctl_status = set_tcp_syncookies(")
require(reset_gate in start and start.index(reset_gate) < sysctl_start and
        "return wan_start_status;" in body(start[start.index(reset_gate):], reset_gate),
        "WAN startup must stop before any setup side effect when protocol staging failed")
restore_policy = body(route_policy, "rc_wan_restore_lan_routes_after_failed_reset(")
for marker in (
    "if (reset_status == 0)",
    "route_status = restore(lan_ifname);",
    "if (restore_status)",
    "return rc_first_service_start_status(reset_status, route_status);",
):
    require(marker in restore_policy,
            f"failed protocol reset route-recovery policy must preserve {marker}")
require("test_failed_reset_restores_lan_routes_and_preserves_first_error" in route_policy_test and
        "test_failed_reset_restores_lan_routes_and_preserves_first_error();" in route_policy_test,
        "WAN reset route recovery must have a registered injected regression")

full_restart = body(wan, "full_restart_wan(void)")
reset_proto_at = full_restart.index("reset_status = reset_wan_vars_status();")
reset_proto_gate = full_restart.index("if (reset_status != 0)", reset_proto_at)
lan_restore = full_restart.index("status = rc_wan_restore_lan_routes_after_failed_reset(", reset_proto_gate)
lan_restore_check = full_restart.index("if (static_lan_route_status != 0)", lan_restore)
reset_abort = full_restart.index("return status;", lan_restore_check)
normal_lan_restore = full_restart.index("status = add_static_lan_routes(IFNAME_BR);", reset_abort)
wan_reactivation = full_restart.index("select_usb_modem_to_wan();", reset_abort)
require(
    reset_proto_at < reset_proto_gate < lan_restore < lan_restore_check <
    reset_abort < normal_lan_restore < wan_reactivation,
    "protocol staging failure must restore static LAN routes, preserve first error and abort WAN restart",
)
require(
    "static LAN route restoration also failed after WAN protocol staging error" in
    full_restart[lan_restore_check:reset_abort] and
    "aborting full WAN restart because configured WAN protocol staging failed" in
    full_restart[lan_restore_check:reset_abort],
    "failed route restoration and primary reset error must both be logged",
)
reconnect = body(wan, "try_wan_reconnect(int try_use_modem, long pause_in_seconds)")
require("status = reset_wan_vars_status();" in reconnect and
        reconnect.index("status = reset_wan_vars_status();") <
        reconnect.index("if (pause_in_seconds > 0)") and
        "if (status != 0)" in reconnect,
        "WAN reconnect must propagate protocol staging failure")

# The firewall consumes wanN_ifname_t directly. Never rebuild rules or launch a
# client while that publication is stale: restore the physical WAN and abort
# the current protocol branch when any of the three protocol paths cannot set it.
for marker, expected_count in (
    ('vlan_result = set_wan_unit_value_status(unit, "ifname_t", ppp_ifname);', 2),
    ('vlan_result = set_wan_unit_value_status(unit, "ifname_t", wan_ifname);', 1),
):
    publications = positions(start, marker)
    require(len(publications) == expected_count,
            f"checked active-WAN publication count mismatch for {marker}")
    for position in publications:
        gate = "if (vlan_result != 0)"
        gate_at = start.find(gate, position + len(marker))
        require(gate_at >= 0,
                "failed active-WAN publication must be checked immediately")
        failure = body(start[gate_at:], gate)
        for required in (
            "wan_start_status = rc_first_service_start_status(",
            "restore_wan_after_firewall_failure(unit, wan_ifname);",
            "continue;",
        ):
            require(required in failure,
                    f"failed active-WAN publication must abort and restore: {required}")
        next_firewall = start.find("vlan_result = start_firewall_ex();", position)
        require(next_firewall < 0 or gate_at < next_firewall,
                "active-WAN publication failure must be handled before firewall rebuild")

firewall_marker = "vlan_result = start_firewall_ex();"
firewall_sites = positions(start, firewall_marker)
require(len(firewall_sites) == 3,
        "USB RAS, PPP and IPoE firewall rebuilds must capture their return value")
for index, position in enumerate(firewall_sites):
    follow = start[position:position + 3200]
    gate = "if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {"
    require(gate in follow,
            "every WAN protocol path must stop after a failed firewall rebuild")

    ordered = [gate]
    if index == 0:
        ordered.extend((
            "vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);",
            "continue;",
            "vlan_result = launch_wan_modem_ras(unit);",
        ))
    elif index == 1:
        ordered.extend((
            "vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);",
            "continue;",
            "vlan_result = launch_wan_pppd(unit, wan_proto);",
        ))
    else:
        ordered.extend((
            "wan_err = 1;",
            "if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {",
            'firewall_cleanup_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);',
            "if (firewall_cleanup_status != 0)",
            "interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);",
            'set_wan_unit_value_int(unit, "err", 1);',
            "continue;",
            "/* Start eapol authenticator */",
        ))

    cursor = 0
    for marker in ordered:
        found = follow.find(marker, cursor)
        require(found >= 0,
                f"WAN firewall gate ordering missing or reversed: {marker}")
        cursor = found + len(marker)

    if index == 0:
        require("wan_start_status = rc_first_service_start_status(" in follow,
                "USB RAS firewall failure must reach shared WAN first-error status")
    else:
        require("wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);" in follow,
                "PPP/IPoE firewall failure must reach wan_start_status")


# All ordinary PPP recovery failures must use the wrapper that takes the
# physical WAN interface down if miniupnp/firewall restoration is incomplete.
ppp_physical_setup_failure = body(start, "if (!i_pppoe && wanx_status != 0)")
require("restore_wan_after_firewall_failure(unit, wan_ifname);" in ppp_physical_setup_failure,
        "PPTP/L2TP physical setup failure must use fail-closed WAN recovery")
require("restore_wan_after_ppp_start_failure(unit, wan_ifname);" not in start,
        "start_wan must not bypass physical-WAN force-down on failed recovery")

pppd_launch_at = start.index("vlan_result = launch_wan_pppd(unit, wan_proto);")
pppd_launch_failure = body(start[pppd_launch_at:], "if (vlan_result != 0)")
require("restore_wan_after_firewall_failure(unit, wan_ifname);" in pppd_launch_failure,
        "pppd launch failure must use fail-closed physical-WAN recovery")
require("restore_wan_after_ppp_start_failure(unit, wan_ifname);" not in pppd_launch_failure,
        "pppd launch failure must not call the non-fail-closed recovery helper")

demand_wait_at = start.index("vlan_result = wait_ppp_up(ppp_ifname, unit);")
demand_wait_failure = body(start[demand_wait_at:], "if (vlan_result != 0)")
require("restore_wan_after_firewall_failure(unit, wan_ifname);" in demand_wait_failure,
        "demand PPP readiness failure must use fail-closed physical-WAN recovery")
require("restore_wan_after_ppp_start_failure(unit, wan_ifname);" not in demand_wait_failure,
        "demand PPP readiness failure must not call the non-fail-closed recovery helper")

# A failed static address setup must not leave a stale address exposed.
static_address = start.split("/* Assign static IP address to i/f */", 1)[1].split(
    "/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */", 1)[0]
require("int static_address_cleanup_failed = 0;" in start.split(
    "for (unit = 0; unit < 1; unit++)", 1)[1][:180],
    "static address cleanup gate must be reset for every WAN unit")
address_cleanup_order = (
    'clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);',
    "if (rc_wan_static_ipoe_clear_failure_requires_ifdown(",
    "static_address_cleanup_failed = 1;",
    "wan_start_status = rc_first_service_start_status(",
    "wan_start_status, clear_status);",
    "interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);",
    "if (interface_cleanup_status != 0)",
    "wan_start_status, interface_cleanup_status);",
)
address_cursor = 0
for marker in address_cleanup_order:
    position = static_address.find(marker, address_cursor)
    require(position >= 0,
            f"partial static address cleanup/force-down sequence missing or reversed: {marker}")
    address_cursor = position + len(marker)
require("failed to clear partially configured static WAN address on %s (status=%d); forcing interface down" in static_address,
        "partial static address cleanup failure must be logged")
require("failed to take static WAN interface %s down after partial address cleanup failure (status=%d)" in static_address,
        "failed force-down after partial address cleanup must be logged")
protocol_gate_at = start.find("if (static_address_cleanup_failed)")
eapol_at = start.find("/* Start eapol authenticator */")
firewall_gate_at = start.find("/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */")
require(firewall_gate_at >= 0 and firewall_gate_at < protocol_gate_at < eapol_at,
        "partial address cleanup must gate protocol activation after firewall setup and before EAPOL")
protocol_gate = body(start, "if (static_address_cleanup_failed)")
require("continue;" in protocol_gate and "start_auth_eapol" not in protocol_gate,
        "failed partial address cleanup must skip EAPOL and the remaining IPoE protocol path")
require("skipping IPoE protocol activation after static address cleanup failure on %s" in protocol_gate,
        "protocol activation skip must be diagnosed")

# A failed EAPOL launch must not leave a stale static IPoE address exposed.
require("rc_wan_static_ipoe_clear_failure_requires_ifdown" in policy and
        "return is_static_ipoe && clear_status != 0;" in policy,
        "static IPoE clear failure must trigger a fail-closed interface-down decision")
eapol = start.split("/* Start eapol authenticator */", 1)[1]
require("auth_status = start_auth_eapol(wan_ifname, unit, wan_auth_mode - 2);" in eapol,
        "EAPOL startup result must be captured")
eapol_order = (
    "wan_start_status = rc_first_service_start_status(",
    "wan_start_status, auth_status);",
    'set_wan_unit_value_int(unit, "err", 1);',
    "static_address_clear_status = 0;",
    'static_address_clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);',
    "if (rc_wan_static_ipoe_clear_failure_requires_ifdown(",
    "wan_start_status, static_address_clear_status);",
    "interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);",
    "if (interface_cleanup_status != 0)",
    "wan_start_status, interface_cleanup_status);",
    "continue;",
)
eapol_cursor = 0
for marker in eapol_order:
    position = eapol.find(marker, eapol_cursor)
    require(position >= 0,
            f"EAPOL static-IP cleanup/forced-down sequence missing or out of order: {marker}")
    eapol_cursor = position + len(marker)
require("failed to clear static WAN address after EAPOL startup failure on %s (status=%d); forcing interface down" in eapol,
        "EAPOL static-IP clear failure must be logged with its status")
require("failed to take static WAN interface %s down after EAPOL address cleanup failure (status=%d)" in eapol,
        "forced-down failure must be logged with its status")

direct_recovery_marker = "vlan_result = restore_wan_after_ppp_start_failure(unit, wan_ifname);"
require(direct_recovery_marker not in start,
        "normal PPP startup must not bypass fail-closed physical-WAN recovery")
recovery_marker = "vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);"
recovery_sites = positions(start, recovery_marker)
require(len(recovery_sites) >= 6,
        "all RAS and ordinary PPP failure paths must use fail-closed recovery")
for position in recovery_sites:
    follow = start[position + len(recovery_marker):position + len(recovery_marker) + 260]
    require("wan_start_status = rc_first_service_start_status(" in follow,
            "fail-closed PPP recovery failure must be aggregated before continuing")

require("modem_start_status" not in start,
        "start_wan() must use one chronological first-error accumulator")
require("return wan_start_status;" in start,
        "start_wan() must return the chronological first WAN error")
bare_calls = [line for line in wan.splitlines() if line.strip() == "start_firewall_ex();"]
require(not bare_calls, "no firewall rebuild result may be discarded")
require(len(positions(wan, "start_firewall_ex()")) == 4,
        "the contract must cover all four current firewall rebuild sites")

# Static IPoE must not be activated with stale address/routing settings when
# any required per-WAN NVRAM copy fails.
required_param_policy = body(nvram_policy, "rc_wan_nvram_set_required_params(")
require("if ((!param_names && count != 0) || !set_param)" in required_param_policy,
        "required WAN parameter copy policy must reject invalid callbacks/arrays")
require("if (status != 0)" in required_param_policy and "return status;" in required_param_policy,
        "required WAN parameter copies must stop and return the first write error")
for case in (
    "test_successful_required_param_sequence",
    "test_required_param_sequence_stops_at_first_error",
    "test_invalid_required_param_arguments_fail_closed",
):
    require(case in nvram_policy_test and case + "();" in body(nvram_policy_test, "main(void)"),
            f"required WAN parameter copy regression is missing or unregistered: {case}")

param_copy = body(wan, "set_wan_unit_param_status(int unit, const char *param_name)")
require('snprintf(wan_param, sizeof(wan_param), "wan_%s", param_name)' in param_copy,
        "checked WAN parameter copy must read the unindexed source key")
require("return set_wan_unit_value_status(unit, param_name," in param_copy and
        "nvram_safe_get(wan_param)" in param_copy,
        "checked WAN parameter copy must propagate the per-WAN NVRAM setter status")
param_callback = body(wan, "set_wan_unit_param_status_callback(")
require("return set_wan_unit_param_status(*unit, param_name);" in param_callback,
        "static IPoE copy callback must propagate the checked NVRAM write status")
reset = body(wan, "reset_wan_vars_status(void)")
static_ipoe = body(reset, "if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)")
require("rc_wan_nvram_set_required_params(" in static_ipoe and
        "set_wan_unit_param_status_callback" in static_ipoe,
        "static IPoE reset must use the checked required-parameter policy")
require("if (proto_status != 0)" in static_ipoe and
        'reset_failure_context = "static IPoE address settings";' in static_ipoe and
        "goto reset_failed;" in static_ipoe,
        "static IPoE copy failure must abort the WAN reset")
require("reset_failed:" in reset and "wan_vars_reset_status = proto_status;" in reset and
        "return proto_status;" in reset,
        "required WAN snapshot failure must latch reset status and block activation")

manual_order = (
    'set_wan_unit_value_status(unit, "man_ipaddr", man_addr)',
    'set_wan_unit_value_status(unit, "man_netmask", man_mask)',
    'set_wan_unit_value_status(unit, "man_gateway", man_gate)',
    'set_wan_unit_value_status(unit, "man_mtu", man_mtu)',
)
manual_cursor = 0
for marker in manual_order:
    position = reset.find(marker, manual_cursor)
    require(position >= 0, f"manual WAN endpoint write missing/out of order: {marker}")
    manual_cursor = position + len(marker)
require('reset_failure_context = "manual WAN endpoint settings";' in reset and
        "goto reset_failed;" in reset[manual_cursor:],
        "manual WAN endpoint write failure must abort the WAN reset")

credential_params = ('"pppoe_username"', '"pppoe_passwd"')
option_params = (
    '"pppoe_txonly_x"', '"pppoe_service"', '"pppoe_ac"', '"pppoe_man"',
    '"pppoe_mtu"', '"pppoe_mru"', '"pptp_mtu"', '"pptp_mru"',
    '"l2tp_mtu"', '"l2tp_mru"', '"ppp_peer"', '"ppp_auth"', '"ppp_mppe"',
    '"ppp_lcp"', '"ppp_alcp"', '"ppp_pppd"',
)
for array_name, expected in (
    ("ppp_credential_params[] = {", credential_params),
    ("ppp_option_params[] = {", option_params),
):
    array_start = wan.index(array_name)
    array_end = wan.index("};", array_start)
    array_text = wan[array_start:array_end]
    cursor = 0
    for marker in expected:
        position = array_text.find(marker, cursor)
        require(position >= 0, f"{array_name} missing or reorders required field {marker}")
        cursor = position + len(marker)

require("rc_wan_nvram_set_required_params(\n\t\t\tppp_credential_params," in reset and
        "rc_wan_nvram_set_required_params(\n\t\t\tppp_option_params," in reset,
        "PPP credential and negotiation option copies must use the checked sequence policy")
require('set_wan_unit_param_status(unit, "pppoe_idletime")' in reset and
        'set_wan_unit_value_status(\n\t\t\t\tunit, "pppoe_idletime", "0")' in reset,
        "PPP idle-time snapshot must use checked NVRAM writes for both protocol branches")
require('reset_failure_context = "PPP authentication credentials";' in reset and
        'reset_failure_context = "PPP peer and negotiation options";' in reset,
        "PPP credential/options failures must latch reset status with a diagnostic context")


runtime_clear = reset.index("runtime_clear_values[] = {")
runtime_clear_end = reset.index("};", runtime_clear)
runtime_clear_text = reset[runtime_clear:runtime_clear_end]
runtime_clear_expected = (
    '{ "dns", "" }', '{ "wins", "" }', '{ "lease", "" }',
    '{ "domain", "" }', '{ "routes", "" }', '{ "routes_ms", "" }',
    '{ "routes_rfc", "" }', '{ "err", "0" }',
)
cursor = 0
for marker in runtime_clear_expected:
    position = runtime_clear_text.find(marker, cursor)
    require(position >= 0, f"runtime WAN snapshot clear missing/out of order: {marker}")
    cursor = position + len(marker)
require("rc_wan_nvram_set_required_values(\n\t\truntime_clear_values," in reset and
        'reset_failure_context = "WAN runtime DNS and route snapshot";' in reset and
        "goto reset_failed;" in reset,
        "WAN DNS/lease/domain/route snapshot clears must be checked before protocol selection")
for marker in (
    'set_wan_unit_value(unit, "dns", "")',
    'set_wan_unit_value(unit, "routes", "")',
    'set_wan_unit_value_int(unit, "err", 0)',
):
    require(marker not in reset, f"unchecked runtime snapshot clear remains: {marker}")

nonstatic_clear = reset.index("nonstatic_address_clear_values[] = {")
nonstatic_clear_end = reset.index("};", nonstatic_clear)
nonstatic_clear_text = reset[nonstatic_clear:nonstatic_clear_end]
for marker in ('{ "ipaddr", "0.0.0.0" }', '{ "netmask", "0.0.0.0" }',
               '{ "gateway", "0.0.0.0" }', '{ "mtu", "" }'):
    require(marker in nonstatic_clear_text,
            f"non-static WAN address snapshot clear missing: {marker}")
require("rc_wan_nvram_set_required_values(\n\t\t\tnonstatic_address_clear_values," in reset and
        'reset_failure_context = "non-static WAN address snapshot";' in reset and
        "goto reset_failed;" in reset,
        "non-static WAN address snapshot clear failures must abort reset before activation")
require("set_wan_unit_value_status(*unit, param_name, value)" in wan,
        "runtime snapshot writes must use the checked indexed NVRAM setter")

for marker in ("test_required_value_sequence_success",
               "test_required_value_sequence_stops_at_first_error",
               "test_required_value_sequence_rejects_invalid_values"):
    require(marker in nvram_policy_test,
            "required runtime-value policy regression missing: " + marker)
require("rc_wan_nvram_set_required_values" in nvram_policy_test and
        "fake_set_value" in nvram_policy_test,
        "runtime NVRAM value policy must be exercised through an injected writer")

man_reset = body(wan, "reset_man_vars_status(void)")
empty_man_keys = (
    '"wanx_ipaddr"', '"wanx_netmask"', '"wanx_gateway"', '"wanx_dns"',
    '"wanx_lease"', '"wanx_domain"', '"wanx_mtu"', '"wanx_routes"',
    '"wanx_routes_ms"', '"wanx_routes_rfc"',
    '"manv_ipaddr"', '"manv_netmask"', '"manv_gateway"', '"manv_mtu"',
    '"manv_routes"', '"manv_routes_ms"', '"manv_routes_rfc"',
)
zero_man_keys = ('"wanx_err"', '"manv_err"')
for array_name, expected_keys in (("empty_params[] = {", empty_man_keys),
                                  ("zero_params[] = {", zero_man_keys)):
    start = man_reset.index(array_name)
    end = man_reset.index("};", start)
    values = man_reset[start:end]
    cursor = 0
    for marker in expected_keys:
        position = values.find(marker, cursor)
        require(position >= 0, f"MAN snapshot reset missing/out of order: {marker}")
        cursor = position + len(marker)
require("rc_wan_nvram_set_required_params(" in man_reset and
        "set_nvram_named_value_status" in man_reset and "return status;" in man_reset,
        "MAN/virtual IPTV clears must use checked first-error NVRAM writes")
require("reset_man_vars_status();" in reset and
        'reset_failure_context = "MAN/virtual IPTV route snapshot";' in reset and
        "goto reset_failed;" in reset,
        "WAN activation must stop if MAN route snapshot clearing fails")
man_publish = reset.split('if (wan_proto == IPV4_WAN_PROTO_PPPOE ||', 1)[1].split("mac_buf[0] = 0;", 1)[0]
for marker in (
    'set_nvram_value_status("wanx_ipaddr", man_addr)',
    'set_nvram_value_status("wanx_netmask", man_mask)',
    'set_nvram_value_status("wanx_gateway", man_gate)',
):
    require(marker in man_publish, f"global MAN endpoint publication must check NVRAM write: {marker}")
require('reset_failure_context = "global MAN route endpoint snapshot";' in man_publish and
        "goto reset_failed;" in man_publish,
        "global MAN route endpoint write failure must block protocol activation")
auth_array = reset.index('auth_params[] = {')
auth_end = reset.index("};", auth_array)
auth_text = reset[auth_array:auth_end]
cursor = 0
for marker in ('"auth_mode"', '"auth_user"', '"auth_pass"'):
    position = auth_text.find(marker, cursor)
    require(position >= 0, f"EAPOL authentication field missing/out of order: {marker}")
    cursor = position + len(marker)
require("rc_wan_nvram_set_required_params(\n\t\tauth_params," in reset and
        'reset_failure_context = "WAN authentication mode and credentials";' in reset,
        "EAPOL mode/user/password must be copied with checked writes before WAN startup")
require('set_wan_unit_param(unit, "auth_mode")' not in reset and
        'set_wan_unit_param(unit, "auth_user")' not in reset and
        'set_wan_unit_param(unit, "auth_pass")' not in reset,
        "legacy unchecked EAPOL authentication copies must be removed")

mac_start = reset.index("wan_hwaddr = (strlen(mac_buf)")
mac_end = reset.index("#if defined (USE_IPV6)", mac_start)
mac_snapshot = reset[mac_start:mac_end]
require('set_nvram_persistent_value_status("wan_hwaddr", wan_hwaddr)' in mac_snapshot and
        'set_wan_unit_value_status(unit, "hwaddr", wan_hwaddr)' in mac_snapshot,
        "global and indexed WAN interface MAC snapshots must both use checked NVRAM writes")
require('reset_failure_context = "WAN interface hardware address";' in mac_snapshot and
        "goto reset_failed;" in mac_snapshot,
        "WAN hardware address staging failure must abort before interface/VLAN activation")
require('nvram_set("wan_hwaddr", mac_buf)' not in reset and
        'set_wan_unit_param(unit, "hwaddr")' not in reset,
        "legacy unchecked WAN MAC copies must be removed")

dhcp_params_start = reset.index('dhcp_ppp_params[] = {')
dhcp_params_end = reset.index("};", dhcp_params_start)
dhcp_params = reset[dhcp_params_start:dhcp_params_end]
cursor = 0
for marker in ('"dnsenable_x"', '"vci"', '"hostname"'):
    position = dhcp_params.find(marker, cursor)
    require(position >= 0, f"critical DHCP/PPP option missing or out of order: {marker}")
    cursor = position + len(marker)
require("rc_wan_nvram_set_required_params(\n\t\tdhcp_ppp_params," in reset and
        'reset_failure_context = "WAN DHCP/DNS client options";' in reset,
        "DHCP hostname, VCI and DNS mode snapshot must use checked fail-fast writes")
for marker in ('"dnsenable_x"', '"vci"', '"hostname"'):
    require(f'set_wan_unit_param(unit, {marker})' not in reset,
            f"unchecked DHCP/PPP parameter copy remains: {marker}")
param_list_start = wan.index("static_ipoe_params[] = {")
param_list_end = wan.index("};", param_list_start)
param_list = wan[param_list_start:param_list_end]
param_cursor = 0
for param in ('"ipaddr"', '"netmask"', '"gateway"', '"mtu"'):
    position = param_list.find(param, param_cursor)
    require(position >= 0, f"static IPoE required parameter missing/out of order: {param}")
    param_cursor = position + len(param)


man_down = body(wan, "man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)")
clear_runtime = man_down.index("clear_runtime_values[] = {")
clear_runtime_end = man_down.index("};", clear_runtime)
clear_runtime_text = man_down[clear_runtime:clear_runtime_end]
for marker in ('{ "wanx_gateway", "" }', '{ "wanx_dns", "" }',
               '{ "wanx_domain", "" }', '{ "wanx_routes", "" }',
               '{ "wanx_routes_ms", "" }', '{ "wanx_routes_rfc", "" }'):
    require(marker in clear_runtime_text,
            f"MAN runtime snapshot clear missing: {marker}")
clear_address = man_down.index("clear_address_values[] = {")
clear_address_end = man_down.index("};", clear_address)
require('{ "wanx_ipaddr", "0.0.0.0" }' in man_down[clear_address:clear_address_end],
        "DHCP deconfig must retain its conditional MAN IP snapshot clear")
require("if (clear_dhcp_address)" in man_down and
        "route_status = rc_wan_nvram_set_all_values(" in man_down and
        "set_nvram_snapshot_value_status" in man_down,
        "MAN teardown snapshot clears must use the checked attempt-all writer")
snapshot_writer = body(wan, "set_nvram_snapshot_value_status(void *context, const char *param_name,")
require("nvram_set_temp(param_name, value)" in snapshot_writer and
        "rc_wan_nvram_write_status(api_result, saved_errno)" in snapshot_writer,
        "MAN runtime snapshot clears must retain temporary NVRAM semantics and report errno")
require(man_down.index("if (status == 0)") < man_down.index("route_status = rc_wan_nvram_set_all_values(", man_down.index("/* Attempt every independent clear")) and
        man_down.index("MAN runtime DNS/route snapshot clear incomplete") <
        man_down.index('refresh_resolvconf_nonfatal("MAN down", 0)'),
        "MAN snapshot is cleared only after route teardown and before post-teardown notifications")
for marker in (
    'nvram_set_temp("wanx_ipaddr", "0.0.0.0")',
    'nvram_set_temp("wanx_gateway", "")',
    'nvram_set_temp("wanx_dns", "")',
    'nvram_set_temp("wanx_routes", "")',
):
    require(marker not in man_down,
            "unchecked MAN teardown NVRAM snapshot write remains: " + marker)
require("test_all_value_sequence_attempts_every_write_and_preserves_first_error" in nvram_policy_test and
        "test_all_value_sequence_validates_before_writing" in nvram_policy_test,
        "MAN teardown attempt-all helper must have injected host regressions")
udhcpc_deconfig = body(wan, "udhcpc_deconfig(char *wan_ifname, int is_zcip)")
require("cleanup_status = man_down_internal(wan_ifname, unit, reset_man_address);" in udhcpc_deconfig and
        "return route_status_accumulate(" in udhcpc_deconfig and
        "cleanup_status);" in udhcpc_deconfig,
        "MAN deconfig must return the snapshot-clear failure to its caller")



wan_up = body(wan, "wan_up_internal(char *wan_ifname, int unit, int is_static)")
gateway_publish = 'route_status = set_nvram_snapshot_value_status(\n\t\t\tNULL, "wanx_gateway",'
gateway_gate = 'if (route_status != 0)'
require(gateway_publish in wan_up and gateway_gate in wan_up,
        "IPoE WAN-up must check the volatile MAN gateway snapshot write")
gateway_publish_at = wan_up.index(gateway_publish)
gateway_gate_at = wan_up.index(gateway_gate, gateway_publish_at)
skip_log_at = wan_up.index("skipping dependent static MAN routes", gateway_gate_at)
dependent_route_at = wan_up.index("route_status = add_static_man_routes(wan_ifname);", gateway_gate_at)
require(gateway_publish_at < gateway_gate_at < skip_log_at < dependent_route_at and
        "status = route_status_accumulate(status, route_status);" in
        wan_up[gateway_publish_at:dependent_route_at],
        "failed gateway publication must be reported and block dependent static route creation")
require("nvram_set_temp(\"wanx_gateway\"" not in wan_up,
        "unchecked MAN gateway snapshot publication remains in WAN-up")
require('gateway = nvram_safe_get("wanx_gateway");' in net_core and
        "if (!is_valid_ipv4(gateway))" in net_core,
        "static-route implementation must continue to identify the global MAN gateway fallback dependency")


wait_ppp = body(wan, "wait_ppp_up(char *ppp_ifname, int unit)")
ppp_snapshot = wait_ppp.index('ppp_snapshot_values[0].param_name = "ipaddr";')
snapshot_markers = (
    "ppp_snapshot_values[0].value = ppp_addr;",
    'ppp_snapshot_values[1].param_name = "netmask";',
    "ppp_snapshot_values[1].value = ppp_mask;",
    'ppp_snapshot_values[2].param_name = "gateway";',
    "ppp_snapshot_values[2].value = ppp_gate;",
)
cursor = ppp_snapshot
for marker in snapshot_markers:
    position = wait_ppp.find(marker, cursor)
    require(position >= 0, "PPP demand snapshot fields missing/out of order: " + marker)
    cursor = position + len(marker)
require("SIOCGIFADDR" in wait_ppp and "SIOCGIFDSTADDR" in wait_ppp and
        'snprintf(ppp_addr, sizeof(ppp_addr), "%s", "0.0.0.0");' in wait_ppp and
        "ppp_gate[0] =" in wait_ppp,
        "PPP demand startup must clear unavailable local/peer tuple fields rather than retain old values")
require("rc_wan_nvram_set_required_values(\n\t\tppp_snapshot_values, 3, set_wan_unit_value_status_callback, &unit)" in wait_ppp and
        "if (route_status != 0)" in wait_ppp and
        "return route_status;" in wait_ppp,
        "PPP demand route setup must stop when a runtime address snapshot write fails")
require(wait_ppp.index("rc_wan_nvram_set_required_values(") <
        wait_ppp.index("route_status = preset_wan_ppp_routes(ppp_ifname, unit);"),
        "PPP demand routes must only be preset after the complete address tuple is published")
for marker in (
    'set_wan_unit_value(unit, "ipaddr", inet_ntoa(sin_addr(&ifr.ifr_addr)))',
    'set_wan_unit_value(unit, "netmask", "255.255.255.255")',
    'set_wan_unit_value(unit, "gateway", ppp_gate)',
):
    require(marker not in wait_ppp, "unchecked PPP runtime address copy remains: " + marker)


ipup_publish = body(ppp, "ppp_wan_ipup_publish_address(void *opaque, const char *ipaddr,")
ipup_writer = body(ppp, "ppp_wan_ipup_set_nvram_value_status(void *opaque, const char *param_name,")
ipup_clear = body(ppp, "ppp_wan_ipup_clear_interface_address(void *opaque)")
for marker in (
    'param_names[] = {\n\t\t"ipaddr", "netmask", "ifname_t", "gateway", "dns"',
    'values[0].value = ipaddr;',
    'values[1].value = netmask;',
    'values[2].value = context->ifname;',
    'values[3].value = context->remote_ip;',
    'values[4].value = context->dns;',
    "dhcp_lease_nvram_publish(",
    "ppp_wan_ipup_set_nvram_value_status",
    "values[i].previous_value = previous_text[i];",
    "dhcp_lease_nvram_restore(",
):
    require(marker in ipup_publish, "PPP ip-up complete snapshot/restore missing: " + marker)
require("nvram_transaction_set(context->transaction, wan_param" in ipup_writer and
        "saved_errno ? saved_errno : EIO" in ipup_writer,
        "PPP ip-up snapshot writer must check transaction result and errno")
require("ifconfig(context->ifname, 0, NULL, NULL)" in ipup_clear,
        "PPP ip-up snapshot failure must take the configured interface down")
ipup_policy_body = body(ppp_ipup_policy, "ppp_wan_ipup_apply_address(")
require("status = ops->publish_address(context, ipaddr, netmask);" in ipup_policy_body and
        "ops->clear_interface_address(context)" in ipup_policy_body and
        "return status;" in ipup_policy_body,
        "PPP ip-up publication failure must abort and roll back the interface")
ipup = body(ppp, "ipup_main(int argc, char **argv)")
require("interface_context.remote_ip = remote_ip;" in ipup and
        "interface_context.dns = buf;" in ipup,
        "PPP ip-up must stage gateway and DNS in the same runtime snapshot as its address")
require(ipup.index("ppp_wan_ipup_apply_address(") < ipup.index("route_status = wan_up(ppp_ifname, unit, 0);"),
        "PPP WAN-up routes must not run until the complete ip-up NVRAM snapshot succeeds")
for marker in (
    'set_wan_unit_value(unit, "ifname_t", ppp_ifname)',
    'set_wan_unit_value(unit, "gateway", remote_ip)',
    'set_wan_unit_value(unit, "dns", buf)',
):
    require(marker not in ipup, "unchecked PPP ip-up runtime NVRAM publication remains: " + marker)
for case in (
    "test_publication_failure_rolls_back_interface_and_preserves_primary_error",
    "test_rollback_failure_does_not_hide_publication_error",
):
    require(case in ppp_ipup_test and case + "();" in ppp_ipup_test,
            "PPP ip-up publication rollback regression missing: " + case)


wan_down = body(wan, "wan_down_internal(char *wan_ifname, int unit, int is_static)")
vpn_dns_clear = 'route_status = set_nvram_value_status("vpnc_dns_t", "");'
vpn_dns_at = wan_down.index(vpn_dns_clear)
vpn_dns_next = wan_down.index("if (!is_static)", vpn_dns_at)
vpn_dns_block = wan_down[vpn_dns_at:vpn_dns_next]
require("status = route_status_accumulate(status, route_status);" in vpn_dns_block and
        "failed to clear VPN resolver snapshot during WAN teardown" in vpn_dns_block,
        "WAN teardown must report a failed VPN resolver-marker clear without skipping later cleanup")
wan_dns_guard = wan_down.index('strlen(nvram_safe_get("wanx_dns")) > 6)')
wan_dns_clear = 'route_status = set_wan_unit_value_status(unit, "dns", "");'
wan_dns_at = wan_down.index(wan_dns_clear, wan_dns_guard)
wan_dns_refresh = wan_down.index('refresh_resolvconf_nonfatal("WAN down", 0);', wan_dns_at)
wan_dns_if = wan_down.rindex("if (wan_proto", 0, wan_dns_guard + 1)
wan_dns_block = wan_down[wan_dns_if:wan_dns_refresh]
require("if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC" in wan_dns_block and
        "status = route_status_accumulate(status, route_status);" in wan_dns_block and
        "failed to clear indexed WAN DNS snapshot" in wan_dns_block,
        "conditional indexed WAN DNS clear must be checked and included in teardown status")
require(wan_dns_at < wan_dns_refresh and
        '\n\t}\n\n\t/* Resolver refresh failure must not block WAN teardown/cleanup. */\n\t'
        'refresh_resolvconf_nonfatal("WAN down", 0);' in wan_down[wan_dns_at:],
        "resolver refresh must still run after a failed DNS snapshot clear")
require('nvram_set_temp("vpnc_dns_t", "")' not in wan_down and
        'set_wan_unit_value(unit, "dns", "")' not in wan_down,
        "unchecked VPN/WAN resolver snapshot clears remain")
require("test_wan_resolver_snapshot_clear_failures_are_reported_while_teardown_continues" in wan_route_test and
        "test_wan_resolver_snapshot_clear_failures_are_reported_while_teardown_continues();" in wan_route_test,
        "WAN resolver snapshot cleanup failure regression is missing")
print("WAN firewall rebuild fail-closed protocol gates and PPP recovery: PASS")
