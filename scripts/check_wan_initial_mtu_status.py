#!/usr/bin/env python3
"""Verify initial physical WAN MTU failures reach start_wan() status."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
POLICY = (RC / "wan_mtu_setup_policy.h").read_text()
TEST = (RC / "tests/service_start_status.c").read_text()
WAN_WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
PROCESS_WORKFLOW = (ROOT / ".github/workflows/process-lifecycle-regression.yml").read_text()


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
        raise SystemExit("WAN initial MTU status contract: FAIL: " + message)


start_wan = body(WAN, "start_wan(void)")
require("int unit, wan_proto, wan_mtu;" in start_wan,
        "configured WAN MTU must be available to both initial and static setup")
proto = start_wan.index("wan_proto = get_wan_proto(unit);")
read_mtu = start_wan.index('wan_mtu = get_wan_unit_value_int(unit, "mtu");', proto)
prepare = start_wan.index("if (wan_default_mtu_should_be_set(", read_mtu)
default_set = start_wan.index("mtu_status = set_interface_mtu(wan_ifname, 1500);", prepare)
failure = start_wan.index("if (mtu_status != 0)", default_set)
aggregate = start_wan.index(
    "wan_start_status = rc_first_service_start_status(", failure
)
diagnostic = start_wan.index("WAN interface %s default MTU setup incomplete", aggregate)
ifup = start_wan.index(
    'interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);',
    diagnostic,
)
require(
    proto < read_mtu < prepare < default_set < failure < aggregate < diagnostic < ifup,
    "read protocol/MTU, attempt default, aggregate and log failure before interface bring-up",
)
default_block = start_wan[prepare:ifup]
require(
    "wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, wan_mtu" in default_block,
    "default MTU suppression must apply only to static IPoE configured MTU",
)
require("continue;" not in default_block,
        "default MTU failure must not skip required physical interface bring-up")

static_branch = start_wan.index(
    "if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)", ifup
)
static_end = start_wan.index(
    "/* re-build iptables rules (final stage for IPoE static) */", static_branch
)
static = start_wan[static_branch:static_end]
custom_set = static.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);")
custom_failure = static.index("wan_start_status = rc_first_service_start_status(", custom_set)
address_gate = static.index("if (!wan_err)", custom_failure)
address_set = static.index(
    "address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);",
    address_gate,
)
require(
    "if (wan_mtu >= 1300 && wan_mtu < 1500)" in static
    and custom_set < custom_failure < address_gate < address_set,
    "static custom MTU must still be applied and its failure must block address publication",
)
require("int wan_mtu =" not in static,
        "static path must use the same configured MTU value as the initial decision")
require(
    "return rc_first_service_start_status(modem_start_status, wan_start_status);" in start_wan,
    "start_wan must return the accumulated physical MTU failure",
)

policy_body = body(POLICY, "wan_default_mtu_should_be_set(")
require(
    "is_static_ipoe && configured_mtu >= 1300 && configured_mtu < 1500" in policy_body,
    "only configured static-IPoE MTUs from 1300 through 1499 suppress default MTU",
)
for token in (
    "wan_default_mtu_should_be_set(1, 1299)",
    "wan_default_mtu_should_be_set(1, 1300)",
    "wan_default_mtu_should_be_set(1, 1499)",
    "wan_default_mtu_should_be_set(1, 1500)",
    "wan_default_mtu_should_be_set(0, 1400)",
    "physical_ifup_attempts == 1",
    "protocol_start_attempts == 1",
    "assert(status == -1)",
):
    require(token in TEST, "service startup regression missing " + token)
require(
    "test_wan_default_mtu_respects_static_custom_override();" in TEST
    and "test_wan_default_mtu_failure_is_reported_while_startup_continues();" in TEST,
    "both WAN MTU policy regressions must run from main()",
)

for path in (
    "scripts/check_wan_initial_mtu_status.py",
    "trunk/user/rc/src/wan_mtu_setup_policy.h",
    "trunk/user/rc/src/tests/service_start_status.c",
):
    require(
        WAN_WORKFLOW.count('"' + path + '"') >= 2,
        "WAN push and pull_request triggers must include " + path,
    )
require(
    "python3 scripts/check_wan_initial_mtu_status.py" in WAN_WORKFLOW,
    "WAN regression job must run the initial MTU contract checker",
)
for path in (
    "trunk/user/rc/src/wan_mtu_setup_policy.h",
    "trunk/user/rc/src/tests/service_start_status.c",
):
    require(
        PROCESS_WORKFLOW.count('"' + path + '"') >= 2,
        "Process push and pull_request triggers must include " + path,
    )
require(
    "Compile and run RC boot service status aggregation regression" in PROCESS_WORKFLOW
    and 'trunk/user/rc/src/tests/service_start_status.c' in PROCESS_WORKFLOW,
    "Process CI must compile and run the regression with the new policy header",
)

print("WAN initial MTU failure propagation and static override policy: PASS")
