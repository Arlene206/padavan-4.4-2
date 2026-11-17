#!/usr/bin/env python3
"""Verify PPPoE-MWAN detect.sh failures reach WAN-up and WAN-down results."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
DETECT = (ROOT / "trunk/user/pppoe/detect.sh").read_text()
DETECT_TEST = (ROOT / "trunk/user/pppoe/tests/detect_status.sh").read_text()


def braced_body(source, opening):
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quote"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index], index + 1
        elif state == "quote":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 2
            continue
        index += 1
    raise AssertionError("unterminated C block")


def function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    return braced_body(source, opening)[0]


def require(condition, message):
    if not condition:
        raise SystemExit("PPPoE-MWAN WAN lifecycle contract: FAIL: " + message)


def check_caller(signature, direction, diagnostic):
    caller = function_body(WAN, signature)
    gate = caller.index('if (nvram_match("pppoemwan_enable", "1"))')
    opening = caller.index("{", gate)
    block, end = braced_body(caller, opening)
    launch = block.index('route_status = eval("/usr/bin/detect.sh");')
    merge = block.index("status = route_status_accumulate(status, route_status);", launch)
    failure_check = block.index("if (route_status != 0)", merge)
    log = block.index(diagnostic, failure_check)
    require(
        launch < merge < failure_check < log,
        direction + " detector result must be captured, aggregated and logged in order",
    )
    require(
        'doSystem("/usr/bin/detect.sh")' not in caller
        and "return status;" in caller[end:],
        direction + " must return the accumulated detector status",
    )


check_caller(
    "wan_up_internal(char *wan_ifname, int unit, int is_static)",
    "WAN-up",
    "WAN PPPoE-MWAN detection incomplete",
)
check_caller(
    "wan_down_internal(char *wan_ifname, int unit, int is_static)",
    "WAN-down",
    "WAN PPPoE-MWAN reconfiguration incomplete",
)

require(
    "MWAN_LOCK_WAIT_LIMIT=30" in DETECT
    and 'if [ "$MWAN_LOCK_WAITED" -ge "$MWAN_LOCK_WAIT_LIMIT" ]; then' in DETECT
    and 'timed out after ${MWAN_LOCK_WAIT_LIMIT}s' in DETECT
    and 'if [ "$MWAN_LOCK_OWNED" -eq 1 ]' in DETECT
    and "PPPoE-MWAN lock contention is bounded and foreign lock preserved: PASS" in DETECT_TEST,
    "PPPoE-MWAN lock contention must be bounded and a foreign/stale lock left untouched",
)

# An IPv6 teardown error must not short-circuit independent IPv4 route/service cleanup.
wan_down_lifecycle = function_body(
    WAN, "wan_down_internal(char *wan_ifname, int unit, int is_static)"
)
ipv6_call = "ipv6_status = wan6_down(wan_ifname, unit);"
ipv6_merge = "status = route_status_accumulate(status, ipv6_status);"
ipv6_error_check = "if (ipv6_status != 0)"
ipv4_teardown = "wan_proto = get_wan_proto(unit);"
require(
    ipv6_call in wan_down_lifecycle
    and ipv6_merge in wan_down_lifecycle
    and ipv6_error_check in wan_down_lifecycle
    and ipv4_teardown in wan_down_lifecycle
    and wan_down_lifecycle.index(ipv6_call)
        < wan_down_lifecycle.index(ipv6_merge)
        < wan_down_lifecycle.index(ipv6_error_check)
        < wan_down_lifecycle.index(ipv4_teardown),
    "WAN-down must retain IPv6 failure and continue to independent IPv4 cleanup",
)
require(
    "return ipv6_status;" not in wan_down_lifecycle,
    "WAN-down must not abandon IPv4 cleanup after an IPv6 teardown failure",
)

for signature, internal in (
    ("wan_up(char *wan_ifname, int unit, int is_static)", "wan_up_internal"),
    ("wan_down(char *wan_ifname, int unit, int is_static)", "wan_down_internal"),
):
    wrapper = function_body(WAN, signature)
    require(internal + "(wan_ifname, unit, is_static)" in wrapper
            and "return status;" in wrapper,
            internal + " result must be returned by its public wrapper")

for marker in (
    "trunk/user/rc/src/net_wan.c",
    "scripts/check_pppoe_mwan_status.py",
):
    require(WORKFLOW.count(f'"{marker}"') >= 2,
            "WAN push/pull triggers are missing " + marker)
require(
    "python3 scripts/check_pppoe_mwan_status.py" in WORKFLOW
    and "bash trunk/user/pppoe/tests/detect_status.sh" in WORKFLOW,
    "WAN CI must execute both detector and caller regressions",
)
print("PPPoE-MWAN WAN-up/WAN-down failure propagation: PASS")
