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
require("rc_wan_firewall_start_allows_protocol" in policy,
        "WAN protocol start policy must require a successful firewall rebuild")
require("test_wan_firewall_failure_blocks_protocol_launch" in policy_test and
        "test_wan_firewall_failure_blocks_protocol_launch();" in policy_test,
        "WAN firewall failure gate must have a registered policy regression")
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
        require("modem_start_status = rc_first_service_start_status(" in follow,
                "USB RAS firewall failure must reach modem_start_status")
    else:
        require("wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);" in follow,
                "PPP/IPoE firewall failure must reach wan_start_status")

recovery_marker = "vlan_result = restore_wan_after_ppp_start_failure(unit, wan_ifname);"
recovery_sites = positions(start, recovery_marker)
require(len(recovery_sites) == 3,
        "all PPP startup/recovery failure branches must capture recovery status")
for position in recovery_sites:
    follow = start[position + len(recovery_marker):position + len(recovery_marker) + 260]
    require("wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);" in follow,
            "PPP recovery failure must be aggregated before continuing")

require("return rc_first_service_start_status(modem_start_status, wan_start_status);" in start,
        "start_wan() must return the first modem/WAN error")
bare_calls = [line for line in wan.splitlines() if line.strip() == "start_firewall_ex();"]
require(not bare_calls, "no firewall rebuild result may be discarded")
require(len(positions(wan, "start_firewall_ex()")) == 4,
        "the contract must cover all four current firewall rebuild sites")
print("WAN firewall rebuild fail-closed protocol gates and PPP recovery: PASS")
