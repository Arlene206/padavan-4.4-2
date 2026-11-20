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
header = (RC / "rc.h").read_text()
policy = (RC / "service_start_status.h").read_text()
policy_test = (RC / "tests/service_start_status.c").read_text()
route_policy = (RC / "wan_route_flush_policy.h").read_text()
route_policy_test = (RC / "tests/wan_route_flush_policy.c").read_text()
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
require(reset.index("wan_vars_reset_status = 0;") < reset.index(proto_write) <
        reset.index(proto_failure) < reset.index("return proto_status;") <
        reset.index(proto_read),
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
print("WAN firewall rebuild fail-closed protocol gates and PPP recovery: PASS")
