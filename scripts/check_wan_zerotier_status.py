#!/usr/bin/env python3
"""Verify configured ZeroTier startup errors reach the WAN-up status."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
SERVICES = (RC / "services.c").read_text()
HEADER = (RC / "rc.h").read_text()
TEST = (ROOT / "trunk/user/zerotier/tests/zerotier_lifecycle.sh").read_text()
WORKFLOW = (ROOT / ".github/workflows/zerotier-lifecycle.yml").read_text()
WAN_WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    i = opening
    while i < len(source):
        char = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quote"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:i]
        elif state == "quote":
            if char == chr(92):
                i += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            i += 2
            continue
        i += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("WAN ZeroTier status contract: FAIL: " + message)


wan_up = body(WAN, "wan_up_internal(char *wan_ifname, int unit, int is_static)")
gate = wan_up.index('if (nvram_match("zerotier_enable", "1"))')
start = wan_up.index("route_status = start_zerotier();", gate)
merge = wan_up.index(
    "status = route_status_accumulate(status, route_status);", start
)
log = wan_up.index("WAN ZeroTier startup incomplete", merge)
require(
    "#if defined(APP_ZEROTIER)" in wan_up[:gate]
    and gate < start < merge < log
    and "if (route_status != 0)" in wan_up[merge:log],
    "enabled ZeroTier start must be checked, aggregated and logged",
)
require(
    'doSystem("/usr/bin/zerotier.sh start")' not in wan_up
    and "return status;" in wan_up[log:],
    "WAN-up must return the accumulated ZeroTier startup result",
)
mwan = wan_up.index('if (nvram_match("pppoemwan_enable", "1"))', log)
require(log < mwan, "ZeroTier result must be processed before WAN-up returns")

start_wrapper = body(SERVICES, "int start_zerotier(void)")
require(
    'nvram_get_int("zerotier_enable")' in start_wrapper
    and "if (zerotier_enable != 1)" in start_wrapper
    and "return 0;" in start_wrapper
    and 'return eval("/usr/bin/zerotier.sh","start");' in start_wrapper,
    "RC wrapper must preserve disabled no-op and launcher exit status",
)
require("int start_zerotier(void);" in HEADER, "RC header must declare the wrapper")

for token in (
    'return "$TEST_START_INSTANCE_STATUS"',
    "TEST_START_INSTANCE_STATUS=23",
    "if start_zero; then",
    "start reported success after start_instance failed",
):
    require(token in TEST, "shell regression lacks " + token)

for workflow, label in ((WORKFLOW, "ZeroTier"), (WAN_WORKFLOW, "WAN route")):
    for path in ("trunk/user/rc/src/net_wan.c", "scripts/check_wan_zerotier_status.py"):
        require(f'"{path}"' in workflow, label + " push/pull trigger missing " + path)
    require("python3 scripts/check_wan_zerotier_status.py" in workflow,
            label + " workflow does not run the caller contract")

print("Configured ZeroTier startup failure propagation into WAN-up: PASS")
