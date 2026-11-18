#!/usr/bin/env python3
"""Verify AP-client association gates both physical and IPoE WAN activation."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
HEADER = (RC / "apcli_wait_policy.h").read_text()
TEST = (RC / "tests/apcli_wait_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and following == "*":
                state = "block"
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
        elif state == "block" and char == "*" and following == "/":
            state = "code"
            index += 2
            continue
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("AP-client WAN readiness contract: FAIL: " + message)


launch_wanx = body(WAN, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)")
launch_wait = "launch_status = rc_wan_apcli_wait_status(\n\t\t\twait_apcli_connected(man_ifname, 3));"
require(launch_wait in launch_wanx, "launch_wanx must convert an AP-client timeout to ETIMEDOUT")
launch_failure = launch_wanx.index("if (launch_status != 0)")
launch_return = launch_wanx.index("return launch_status;", launch_failure)
for later_start in ("start_udhcpc_wan(", "start_zcip_wan(", "ifconfig("):
    require(launch_return < launch_wanx.index(later_start), "AP-client timeout must abort before " + later_start)

wan_start = body(WAN, "start_wan(void)")
ipoe_wait = "vlan_result = rc_wan_apcli_wait_status(\n\t\t\t\t\twait_apcli_connected(wan_ifname, 3));"
require(ipoe_wait in wan_start, "IPoE startup must check the AP-client wait result")
failure = wan_start.index("if (vlan_result != 0)", wan_start.index(ipoe_wait))
failure_end = wan_start.index("continue;", failure)
failure_body = wan_start[failure:failure_end]
failure_compact = "".join(failure_body.split())
require("rc_first_service_start_status(wan_start_status,vlan_result)" in failure_compact,
        "IPoE timeout must reach the WAN startup result")
require('set_wan_unit_value_int(unit,"err",1)' in failure_compact,
        "IPoE timeout must mark the WAN unit in error")
require("aborting IPoE WAN activation" in failure_body,
        "IPoE timeout must be diagnosed")
require(failure_end < wan_start.index("ifconfig(wan_ifname, IFUP, wan_addr, wan_mask)"),
        "IPoE static address publication must remain behind the AP-client gate")
require(WAN.count("wait_apcli_connected(") == 3,
        "every wait_apcli_connected use must remain visible to this contract")
require(WAN.count("rc_wan_apcli_wait_status(\n") == 2,
        "both wait call sites must convert timeout into a failure status")
require('#include "apcli_wait_policy.h"' in WAN, "production source must include the tested policy")
wait_fn = body(WAN, "wait_apcli_connected(const char *ifname, int wait_sec)")
require("service_wait_microseconds(500000U) != 0" in wait_fn,
        "AP-client polling must preserve the full wait interval across EINTR")
require("return -1;" in wait_fn and "usleep(" not in wait_fn,
        "AP-client wait errors must not be mistaken for association")
require("if (wait_result > 0)" in HEADER and "if (wait_result == 0)" in HEADER,
        "connected and timeout results must remain distinct")
require("return errno > 0 ? errno : EIO;" in HEADER,
        "wait failures must preserve errno with an EIO fallback")
require("rc_wan_apcli_wait_status(1) == 0" in TEST, "regression must cover connected state")
require("rc_wan_apcli_wait_status(0) == ETIMEDOUT" in TEST, "regression must cover timeout")
require("rc_wan_apcli_wait_status(-1) == EACCES" in TEST and
        "rc_wan_apcli_wait_status(-1) == EIO" in TEST,
        "regression must cover wait failure and missing errno")
require('#include "service_wait_policy.h"' in WAN,
        "production AP-client polling must use the shared wait policy")

required_paths = (
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/apcli_wait_policy.h",
    "trunk/user/rc/src/service_wait_policy.h",
    "trunk/user/rc/src/tests/apcli_wait_policy.c",
    "scripts/check_apcli_wait_status.py",
)
before_jobs = WORKFLOW.split("jobs:", 1)[0]
push_and_pr = before_jobs.split("  pull_request:", 1)
require(len(push_and_pr) == 2, "WAN route workflow must define push and pull_request filters")
for trigger, name in ((push_and_pr[0], "push"), (push_and_pr[1], "pull_request")):
    for path in required_paths:
        require(f'"{path}"' in trigger, "WAN route " + name + " filter misses " + path)
require("apcli-wait-policy" in WORKFLOW, "WAN workflow must compile the host regression")
require("python3 scripts/check_apcli_wait_status.py" in WORKFLOW,
        "WAN workflow must run the caller contract")

xy_filters = XY_WORKFLOW.split("  workflow_dispatch:", 1)[0]
require('"trunk/user/rc/src/net_wan.c"' in xy_filters,
        "XY-C1 product build must watch net_wan.c")
require('"trunk/user/rc/src/apcli_wait_policy.h"' in xy_filters,
        "XY-C1 product build must watch the included AP-client policy")
require('"trunk/user/rc/src/service_wait_policy.h"' in xy_filters,
        "XY-C1 product build must watch the shared interrupt-safe wait policy")

print("AP-client WISP timeout blocks physical and IPoE WAN activation: PASS")
