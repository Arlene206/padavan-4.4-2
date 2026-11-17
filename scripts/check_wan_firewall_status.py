#!/usr/bin/env python3
"""Ensure WAN firewall rebuild and PPP recovery failures reach start_wan()."""
from pathlib import Path
import re
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


wan = (RC / "net_wan.c").read_text()
header = (RC / "rc.h").read_text()
require("int start_firewall_ex(void);" in header,
        "firewall rebuild API must expose its status")

restore = body(wan, "restore_wan_after_ppp_start_failure(int unit, char *wan_ifname)")
for marker in (
    "upnp_status = check_upnp_wanif_changed(wan_ifname);",
    "if (upnp_status != 0)",
    "return upnp_status;",
    "firewall_status = start_firewall_ex();",
    "return firewall_status;",
):
    require(marker in restore, f"PPP physical-WAN recovery must preserve {marker}")

start = body(wan, "start_wan(void)")
firewall_sites = [m.start() for m in re.finditer(
    r"vlan_result = start_firewall_ex\\(\\);", start
)]
require(len(firewall_sites) == 3,
        "USB RAS, PPP and IPoE firewall rebuilds must capture their return value")
for index, position in enumerate(firewall_sites):
    follow = start[position:position + 420]
    if index == 0:
        require("modem_start_status = rc_first_service_start_status(" in follow and
                "modem_start_status, vlan_result);" in follow,
                "USB modem RAS firewall error must reach modem_start_status")
    else:
        require("wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);" in follow,
                "PPP/IPoE firewall error must reach wan_start_status")

recovery_calls = list(re.finditer(
    r"vlan_result = restore_wan_after_ppp_start_failure\\(unit, wan_ifname\\);", start
))
require(len(recovery_calls) == 3,
        "all PPP startup/recovery failure branches must capture recovery status")
for match in recovery_calls:
    follow = start[match.end():match.end() + 260]
    require("wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);" in follow,
            "PPP recovery failure must be aggregated before continuing")

require("return rc_first_service_start_status(modem_start_status, wan_start_status);" in start,
        "start_wan() must return the first modem/WAN error")
require(re.search(r"(?m)^[ \\t]*start_firewall_ex\\(\\);[ \\t]*$", wan) is None,
        "no firewall rebuild result may be discarded")
require(len(re.findall(r"start_firewall_ex\\(\\)", wan)) == 4,
        "the contract must cover all four current firewall rebuild sites")
print("WAN firewall rebuild and PPP recovery status propagation: PASS")
