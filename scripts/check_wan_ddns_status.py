#!/usr/bin/env python3
"""Guard the enabled-DDNS WAN-up result contract and disabled/AP no-op cases."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN_SOURCE = (RC / "net_wan.c").read_text()
SERVICES_SOURCE = (RC / "services_ex.c").read_text()
POLICY_SOURCE = (RC / "inadyn_daemon_policy.h").read_text()
POLICY_TEST = (RC / "tests/inadyn_daemon_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()


def function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and next_char == "/":
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
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        index += 1
    raise AssertionError(f"unterminated C function: {signature}")


def require(condition, message):
    if not condition:
        raise SystemExit(f"WAN DDNS status contract: FAIL: {message}")


policy = function_body(POLICY_SOURCE, "inadyn_update_should_run(")
require("return ddns_enabled && !is_ap_mode;" in policy,
        "DDNS should run only when enabled and not in AP mode")

notify = function_body(SERVICES_SOURCE, "notify_ddns_update_locked(void)")
gate = notify.index("inadyn_update_should_run(")
inventory = notify.index('process_inventory_scan("inadyn", &snapshot)')
require(gate < inventory and
        'nvram_match("ddns_enable_x", "1")' in notify and
        "get_ap_mode()" in notify and
        "return 0;" in notify[gate:inventory],
        "disabled DDNS and AP mode must return a successful no-op before process probing or daemon start")

wan_up = function_body(
    WAN_SOURCE, "wan_up_internal(char *wan_ifname, int unit, int is_static)"
)
notify_at = wan_up.index("route_status = notify_ddns_update();")
merge_at = wan_up.index(
    "status = route_status_accumulate(status, route_status);", notify_at
)
log_at = wan_up.index("WAN DDNS client update incomplete", merge_at)
require(notify_at < merge_at < log_at and
        "if (route_status != 0)" in wan_up[merge_at:log_at],
        "WAN-up must merge and log the DDNS notifier result instead of discarding it")
require("return status;" in wan_up[log_at:],
        "WAN-up must return the accumulated status after DDNS notification")

wan_wrapper = function_body(
    WAN_SOURCE, "wan_up(char *wan_ifname, int unit, int is_static)"
)
require("wan_up_internal(wan_ifname, unit, is_static)" in wan_wrapper and
        "return status;" in wan_wrapper,
        "wan_up must preserve the internal WAN-up result for its caller")

for marker in (
    "test_update_configuration_gate",
    "inadyn_update_should_run(1, 0)",
    "inadyn_update_should_run(0, 0)",
    "inadyn_update_should_run(1, 1)",
    "inadyn_update_should_run(0, 1)",
):
    require(marker in POLICY_TEST, f"host regression is missing {marker}")

for marker in (
    '"trunk/user/rc/src/services_ex.c"',
    '"trunk/user/rc/src/inadyn_daemon_policy.h"',
    '"trunk/user/rc/src/tests/inadyn_daemon_policy.c"',
    '"scripts/check_wan_ddns_status.py"',
    "python3 scripts/check_wan_ddns_status.py",
    "tests/inadyn_daemon_policy.c",
):
    require(marker in WORKFLOW, f"WAN CI trigger or regression step is missing {marker}")

print("WAN-up DDNS failure propagation and disabled/AP no-op contracts: PASS")
