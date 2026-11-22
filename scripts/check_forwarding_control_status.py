#!/usr/bin/env python3
"""Ensure IPv4/IPv6 forwarding sysctl writes are never silently discarded."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC_DIR = ROOT / "trunk/user/rc/src"
NET = (RC_DIR / "net.c").read_text()
RC = (RC_DIR / "rc.c").read_text()
FIREWALL = (RC_DIR / "firewall_ex.c").read_text()
HEADER = (RC_DIR / "rc.h").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    pos = opening
    while pos < len(source):
        char = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                pos += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                pos += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quote"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:pos]
        elif state == "quote":
            if char == chr(92):
                pos += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            pos += 2
            continue
        pos += 1
    raise AssertionError("unterminated C body: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("forwarding control contract: FAIL: " + message)


setter = body(NET, "set_ipv4_forward(int is_on)")
writer = body(NET, "write_ipv4_forward_sysctl(void *context, int enabled)")
require("return rc_ipv4_forward_apply(is_on, write_ipv4_forward_sysctl, NULL);" in setter,
        "set_ipv4_forward must return the checked policy result")
require('return fput_int("/proc/sys/net/ipv4/ip_forward", enabled);' in writer,
        "writer must return actual kernel sysctl result")
require("int set_ipv4_forward(int is_on);" in HEADER,
        "rc.h must expose the result-bearing setter")

startup = body(RC, "if (!firewall_startup_policy_allows_wan_checked(")
require("ipv4_forward_disable_status = set_ipv4_forward(0);" in startup
        and "ipv4_forward_disable_status);" in startup,
        "boot must capture and report IPv4 forwarding disable failure")
require("ipv6_forward_disable_status = set_interface_conf_int(" in startup
        and "ipv6_forward_disable_status);" in startup,
        "boot must capture and report IPv6 forwarding disable failure")
require("start_wan();" not in startup, "firewall rejection must never start WAN")

shutdown = body(RC, "shutdown_router(int level)")
require("forward_stop_status = set_ipv4_forward(0);" in shutdown
        and "lan_wan_stop_status, forward_stop_status);" in shutdown,
        "shutdown must include forwarding disable in persistence gate")

disable_helper = body(FIREWALL, "firewall_disable_forwarding(const char *operation)")
require("next_status = set_ipv4_forward(0);" in disable_helper
        and 'next_status = set_interface_conf_int("ipv6", "all", "forwarding", 0);' in disable_helper,
        "rollback helper must capture IPv4 and IPv6 forwarding disable failures")
apply = body(FIREWALL, "firewall_apply_core_restore(const char *operation")
require("forward_status = firewall_disable_forwarding(" in apply
        and "result = rc_first_service_start_status(result, forward_status);" in apply,
        "standalone rollback must retain forwarding-disable failure")
firewall_start = body(FIREWALL, "start_firewall_ex_locked(void)")
require(firewall_start.count("forward_status = firewall_disable_forwarding(") == 2,
        "both rebuild rollback branches must check forwarding disable")
require("forward_status = set_ipv4_forward(1);" in firewall_start
        and "status = rc_first_service_start_status(status, forward_status);" in firewall_start,
        "normal firewall setup must report failed IPv4 forwarding enable")

for label, source in (("rc.c", RC), ("firewall_ex.c", FIREWALL)):
    require(not re.findall(r"(?m)^\s*set_ipv4_forward\([^;]+\);", source),
            label + " contains ignored IPv4 forwarding call")
    require(not re.findall(
        r'(?m)^\s*set_interface_conf_int\("ipv6",\s*"all",\s*"forwarding",\s*0\);',
        source),
        label + " contains ignored IPv6 forwarding-disable call")
print("IPv4/IPv6 forwarding sysctl status propagation: PASS")
