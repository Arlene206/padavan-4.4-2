#!/usr/bin/env python3
"""Verify configured ZeroTier startup errors reach the WAN-up status."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
SERVICES = (RC / "services.c").read_text()
ZEROTIER_SCRIPT = (ROOT / "trunk/user/zerotier/zerotier.sh").read_text()
ZEROTIER_TEST = (ROOT / "trunk/user/zerotier/tests/zerotier_lifecycle.sh").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
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
start = wan_up.index("route_status = start_zerotier();")
merge = wan_up.index(
    "status = route_status_accumulate(status, route_status);", start
)
log = wan_up.index("WAN ZeroTier startup incomplete", merge)
guard = wan_up.rfind("#if defined(APP_ZEROTIER)", 0, start)
require(
    guard >= 0
    and guard < start < merge < log
    and "if (route_status != 0)" in wan_up[merge:log]
    and 'nvram_match("zerotier_enable", "1")' not in wan_up,
    "WAN-up must always call the checked RC wrapper under APP_ZEROTIER and aggregate/log its status",
)
require(
    'doSystem("/usr/bin/zerotier.sh start")' not in wan_up
    and "return status;" in wan_up[log:],
    "WAN-up must return the accumulated ZeroTier startup result",
)
mwan_read = wan_up.index(
    'pppoemwan_status = get_nvram_match_checked_status(',
    log,
)
mwan_error = wan_up.index("if (pppoemwan_status != 0)", mwan_read)
mwan_enabled = wan_up.index("else if (pppoemwan_enabled)", mwan_error)
mwan_launch = wan_up.index('eval("/usr/bin/detect.sh")', mwan_enabled)
require(
    log < mwan_read < mwan_error < mwan_enabled < mwan_launch
    and "status = route_status_accumulate(status, pppoemwan_status);" in wan_up[mwan_error:mwan_enabled]
    and "cannot read PPPoE-MWAN enable setting during WAN-up" in wan_up[mwan_error:mwan_enabled]
    and 'nvram_match("pppoemwan_enable", "1")' not in wan_up,
    "ZeroTier status must precede checked PPPoE-MWAN read; PPPoE-MWAN errors must be accumulated before detect.sh",
)

start_wrapper = body(SERVICES, "int start_zerotier(void)")
compact_start_wrapper = "".join(start_wrapper.split())
require(
    'service_nvram_match_status("zerotier_enable","1",0,&zerotier_enabled);' in compact_start_wrapper
    and "if(config_status!=0)returnconfig_status;" in compact_start_wrapper
    and "if(!zerotier_enabled)return0;" in compact_start_wrapper
    and 'returneval("/usr/bin/zerotier.sh","start");' in compact_start_wrapper
    and "nvram_get_int(" not in start_wrapper
    and "nvram_match(" not in start_wrapper
    and "nvram_safe_get(" not in start_wrapper,
    "RC wrapper must propagate checked configuration errors and preserve exact disabled no-op / launcher status",
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

start_zero = body(ZEROTIER_SCRIPT, "start_zero()")
stop_zero = body(ZEROTIER_SCRIPT, "stop_zero()")
route_preflight = body(ZEROTIER_SCRIPT, "zerotier_static_route_state_preflight_cleanup()")
replay = body(ZEROTIER_SCRIPT, "zerotier_replay_firewall_state()")
rules = body(ZEROTIER_SCRIPT, "rules()")
route_state_load = body(ZEROTIER_SCRIPT, "zerotier_static_route_state_load()")
route_state_save = body(ZEROTIER_SCRIPT, "zerotier_static_route_state_save()")
route_state_mark = body(ZEROTIER_SCRIPT, "zerotier_static_route_state_mark_owned()")
route_state_remove = body(ZEROTIER_SCRIPT, "zerotier_static_route_state_remove_pending()")
zero_route_body = body(ZEROTIER_SCRIPT, "zero_route()")
append = body(ZEROTIER_SCRIPT, "zerotier_add_firewall_rule()")
require(
    start_zero.index("zerotier_static_route_state_preflight_cleanup")
    < start_zero.index("kill_z")
    and stop_zero.index("zerotier_static_route_state_preflight_cleanup")
    < stop_zero.index("kill_z")
    and "zero_route_entry_matches" in route_preflight
    and 'zt_route_record_state=P' in route_preflight,
    "ambiguous pending routes must block start/stop before the managed daemon is signalled",
)
require(
    start_zero.index("kill_z") < start_zero.index("zerotier_replay_firewall_state")
    < start_zero.index("start_instance"),
    "stale firewall/route cleanup must complete after old-daemon shutdown and before replacement launch",
)
require(
    "zerotier_firewall_state_load" in replay
    and 'zt0="$zt_state_saved_interface"' in replay
    and 'ip_segment="$zt_state_saved_segment"' in replay
    and "if ! del_rules;" in replay
    and 'if ! zero_route "del";' in replay
    and "zerotier_firewall_state_clear" in replay,
    "stale network replay must use saved interface/segment and clear state only after firewall and route cleanup",
)
require(
    rules.index("zerotier_firewall_state_save") < rules.index("zerotier_add_firewall_rule"),
    "successful ZeroTier rule publication must persist a cleanup checkpoint before appending rules",
)
require(
    'zt_route_record_state=P' in route_state_load
    and 'zt_route_record_state=A' in route_state_load
    and 'zt_route_record_state=P' in route_state_save
    and 'grep -Fqx "P $zt_route_ip $zt_route_gateway"' in route_state_save
    and 'found != 1' in route_state_mark
    and 'removed != 1' in route_state_remove
    and 'zerotier_static_route_state_mark_owned "$zero_ip" "$zero_route"' in zero_route_body
    and 'if [ "$zt_route_record_state" = P ]; then' in zero_route_body
    and 'zerotier_static_route_state_remove_pending "$zero_ip" "$zero_route"' in zero_route_body,
    "only a successful route add may promote a pending ZeroTier tuple to owned state",
)
require(
    "failed add attempted to delete an unconfirmed route" in ZEROTIER_TEST
    and "unconfirmed route did not retain a pending ownership marker" in ZEROTIER_TEST
    and "pending exact match is unowned" in ZEROTIER_TEST
    and "route cleanup preflight signalled a running daemon" in ZEROTIER_TEST,
    "static-route EEXIST race regression must preserve unconfirmed routes and the running daemon",
)
require(
    "elif ! zerotier_firewall_state_clear" in append,
    "failed rule append may clear its checkpoint only after complete firewall rollback",
)
require(
    "TEST_ASSERT_CLEAN_BEFORE_START=1" in ZEROTIER_TEST
    and "replacement daemon started despite incomplete stale firewall cleanup" in ZEROTIER_TEST
    and "successful stale-state replay left old firewall rules" in ZEROTIER_TEST
    and "successful stale-state replay left the old configured route" in ZEROTIER_TEST,
    "shell regression must block replacement startup on failed replay and verify successful retry",
)
require('"trunk/user/zerotier/**"' in XY_WORKFLOW,
        "XY-C1 must rebuild when ZeroTier production or lifecycle test inputs change")

print("Configured ZeroTier startup failure propagation into WAN-up and stale-state replay: PASS")
