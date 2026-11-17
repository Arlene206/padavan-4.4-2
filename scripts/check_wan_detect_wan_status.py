#!/usr/bin/env python3
"""Verify gateway ARP checker launch failures reach the WAN-up result."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()


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
                    return source[opening + 1:index]
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
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("WAN gateway ARP checker status contract: FAIL: " + message)


wan_up = body(WAN, "wan_up_internal(char *wan_ifname, int unit, int is_static)")
scan = wan_up.index('presence = process_inventory_scan("detect_wan", &snapshot);')
release = wan_up.index("process_snapshot_release(&snapshot);", scan)
decision = wan_up.index("action = process_presence_start_action_for_state(presence);", release)
start_gate = wan_up.index("if (action == PROCESS_PRESENCE_START_NEW)", decision)
launch = wan_up.index('route_status = eval("detect_wan");', start_gate)
merge = wan_up.index("status = route_status_accumulate(status, route_status);", launch)
failure_gate = wan_up.index("if (route_status != 0)", merge)
diagnostic = wan_up.index("WAN gateway ARP checker startup incomplete", failure_gate)
unknown_gate = wan_up.index(
    "else if (action == PROCESS_PRESENCE_START_ABORT)", diagnostic
)
require(
    scan < release < decision < start_gate < launch < merge < failure_gate
    < diagnostic < unknown_gate,
    "only a clean ABSENT snapshot may launch; its result must be aggregated and logged",
)
require(
    wan_up.count('eval("detect_wan")') == 1,
    "the gateway ARP launch must be captured exactly once",
)
require(
    "skipping gateway ARP checker start because process inventory is unknown" in wan_up[unknown_gate:],
    "UNKNOWN process inventory must continue to fail closed without launching",
)
require("return status;" in wan_up[unknown_gate:],
        "WAN-up must return the accumulated gateway checker launch result")

wrapper = body(WAN, "wan_up(char *wan_ifname, int unit, int is_static)")
require(
    "wan_up_internal(wan_ifname, unit, is_static)" in wrapper
    and "return status;" in wrapper,
    "the public WAN-up wrapper must preserve the internal status",
)
require(
    WORKFLOW.count('"scripts/check_wan_detect_wan_status.py"') >= 2
    and "python3 scripts/check_wan_detect_wan_status.py" in WORKFLOW,
    "WAN push/pull triggers and regression job must include this checker",
)
require(
    "Compile route status regression with sanitizers" in WORKFLOW
    and "ASAN_OPTIONS=detect_leaks=1 /tmp/route-status-policy" in WORKFLOW,
    "the GNU89/ASan/UBSan first-error aggregation regression must remain enabled",
)

print("WAN gateway ARP checker launch failure propagation: PASS")
