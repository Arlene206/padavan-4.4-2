#!/usr/bin/env python3
"""Verify PPPoE-MWAN detect.sh failures reach WAN-up and WAN-down results."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
FIREWALL = (RC / "firewall_ex.c").read_text()
WAN = (RC / "net_wan.c").read_text()
PPP = (RC / "net_ppp.c").read_text()
XL2TP_POLICY = (RC / "xl2tpd_start_policy.h").read_text()
XL2TP_TEST = (RC / "tests/xl2tpd_start_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
DETECT = (ROOT / "trunk/user/pppoe/detect.sh").read_text()
DETECT_TEST = (ROOT / "trunk/user/pppoe/tests/detect_status.sh").read_text()
MWAN = (ROOT / "trunk/user/pppoe/mwan.sh").read_text()
MWAN_TEST = (ROOT / "trunk/user/pppoe/tests/mwan_lifecycle.sh").read_text()


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
    read = caller.index("pppoemwan_status = get_nvram_match_checked_status(")
    read_error = caller.index("if (pppoemwan_status != 0)", read)
    read_merge = caller.index(
        "status = route_status_accumulate(status, pppoemwan_status);",
        read_error,
    )
    read_log = caller.index(
        "cannot read PPPoE-MWAN enable setting during " + direction,
        read_error,
    )
    enabled = caller.index("else if (pppoemwan_enabled)", read_error)
    launch = caller.index('route_status = eval("/usr/bin/detect.sh");', enabled)
    merge = caller.index("status = route_status_accumulate(status, route_status);", launch)
    failure_check = caller.index("if (route_status != 0)", merge)
    log = caller.index(diagnostic, failure_check)
    require(
        read < read_error < read_merge < read_log < enabled < launch
        < merge < failure_check < log,
        direction + " must report checked configuration errors and aggregate detector status in order",
    )
    require(
        'nvram_match("pppoemwan_enable", "1")' not in caller
        and 'doSystem("/usr/bin/detect.sh")' not in caller
        and "return status;" in caller[log:],
        direction + " must not use an unchecked gate and must return accumulated status",
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

# Distinguish expected PPP disappearance from a real inspection failure.
require(
    'MWAN_SYS_CLASS_NET="${MWAN_SYS_CLASS_NET:-/sys/class/net}"' in DETECT
    and 'mwan_get_ppp_ipv4()' in DETECT
    and '[ -d "$MWAN_SYS_CLASS_NET/$iface" ] || return 1' in DETECT
    and 'if [ -d "$MWAN_SYS_CLASS_NET/$iface" ]; then' in DETECT
    and 'mwan_fail "$rc" "ifconfig $iface"' in DETECT
    and 'return 0\n\telse\n\t\trc=$?\n\tfi' in DETECT[
        DETECT.index("mwan_get_ppp_ipv4() {"):DETECT.index("mwan_parse_rule()")
    ]
    and DETECT.count('if mwan_get_ppp_ipv4 "ppp$count"; then') == 2,
    "PPP enumeration must distinguish absent interfaces from ifconfig errors",
)
require(
    "PPPoE-MWAN reports ifconfig errors on existing PPP interfaces and continues independent peers: PASS" in DETECT_TEST
    and "ifconfig ran for an interface absent from sysfs" in DETECT_TEST,
    "PPP interface presence/error regression cases are missing",
)

# The firewall rebuild also invokes this detector before WAN protocol clients
# are launched. Its command result must not disappear at this third call site.
firewall_reconcile = function_body(FIREWALL, "start_firewall_ex_locked(void)")
mwan_gate = 'if (nvram_match("pppoemwan_enable", "1"))'
mwan_call = 'hook_error = firewall_log_command_failure("PPPoE-MWAN detection",'
mwan_merge = "status = rc_first_service_start_status(status, hook_error);"
firewall_entry = function_body(FIREWALL, "start_firewall_ex(void)")
firewall_lock = firewall_entry.index("firewall_lifecycle_lock_acquire()")
firewall_worker = firewall_entry.index("status = start_firewall_ex_locked();")
firewall_unlock = firewall_entry.index("firewall_lifecycle_lock_release(lock_fd)")
require(firewall_lock < firewall_worker < firewall_unlock, "firewall rebuild must serialize the entire worker transaction")
mwan_call_pos = firewall_reconcile.find(mwan_call)
mwan_merge_pos = firewall_reconcile.find(mwan_merge, mwan_call_pos)
require(
    mwan_gate in firewall_reconcile
    and mwan_call_pos >= 0
    and mwan_merge_pos > mwan_call_pos
    and firewall_reconcile.index(mwan_gate) < mwan_call_pos,
    "firewall rebuild must decode and aggregate the PPPoE-MWAN detector command result",
)
require(
    'doSystem("/usr/bin/detect.sh");' not in firewall_reconcile,
    "firewall rebuild must not discard a PPPoE-MWAN detector result",
)

flush_helper_start = DETECT.index("mwan_flush_fail()")
flush_helper_end = DETECT.index(chr(10), flush_helper_start)
require(
    '[ "$rc" -ne 0 ] || return 0' in DETECT[flush_helper_start:flush_helper_end]
    and "MWAN_FLUSH_FAILED=1" in DETECT[flush_helper_start:flush_helper_end]
    and 'mwan_flush_fail "$rc" "iptables-save -t $table"' in DETECT
    and 'mwan_flush_fail 2 "cannot safely parse $table iptables-save rule"' in DETECT
    and 'mwan_flush_fail "$rc" "iptables -t $table rule deletion"' in DETECT,
    "snapshot, parse and old-rule deletion failures must poison prior-policy cleanup",
)
flush_start = DETECT.index("mwan_flush_iptables mangle ctstate")
flush_gate = DETECT.index('if [ "$MWAN_FLUSH_FAILED" -ne 0 ]; then', flush_start)
new_policy_start = DETECT.index(chr(10) + "last=1", flush_start)
require(
    DETECT.index("mwan_flush_iptables mangle br0", flush_start) < flush_gate
    and DETECT.index("mwan_flush_iptables nat ppp", flush_start) < flush_gate
    and flush_gate < DETECT.index('mwan_run ip route flush cache', flush_gate)
    and DETECT.index('exit "$MWAN_STATUS"', flush_gate) < new_policy_start,
    "all independent cleanup must precede cache flush and abort before new policy",
)
require(
    "PPPoE-MWAN mangle snapshot failure blocks new policy: PASS" in DETECT_TEST
    and "PPPoE-MWAN NAT snapshot failure blocks new policy: PASS" in DETECT_TEST
    and "PPPoE-MWAN old-rule deletion failure blocks new policy: PASS" in DETECT_TEST
    and "PPPoE-MWAN malformed saved rule blocks new policy: PASS" in DETECT_TEST,
    "PPPoE-MWAN cleanup fail-closed regression matrix is incomplete",
)

require(
    "MWAN_LOCK_WAIT_LIMIT=30" in DETECT
    and 'if [ "$MWAN_LOCK_WAITED" -ge "$MWAN_LOCK_WAIT_LIMIT" ]; then' in DETECT
    and 'timed out after ${MWAN_LOCK_WAIT_LIMIT}s' in DETECT
    and 'if [ "$MWAN_LOCK_OWNED" -eq 1 ]' in DETECT
    and "PPPoE-MWAN lock contention is bounded and foreign lock preserved: PASS" in DETECT_TEST,
    "PPPoE-MWAN lock contention must be bounded and a foreign/stale lock left untouched",
)

# Keep shell-generated PPP options consistent with the safe RC C writer.
require(
    "umask 077" in MWAN
    and "mwan_pppd_quote()" in MWAN
    and 'options_file="/tmp/ppp/options.wan$count"' in MWAN
    and 'options_tmp="/tmp/ppp/.options.wan$count.$$"' in MWAN
    and 'if [ -L "$options_file" ]; then' in MWAN
    and 'chmod 600 "$options_tmp"' in MWAN
    and 'mv -f "$options_tmp" "$options_file"' in MWAN
    and "printf 'user %s\\n' \"$quoted_user\"" in MWAN
    and "printf 'password %s\\n' \"$quoted_pass\"" in MWAN
    and 'if /usr/sbin/pppd file "$options_file"; then :; else' in MWAN
    and 'base_mac_suffix=$((0x${base_mac##*:}))' in MWAN
    and 'mac_suffix_value=$(((mac_suffix_value + 1) % 256))' in MWAN
    and '[ "$mac_suffix_value" -eq "$base_mac_suffix" ]' in MWAN
    and 'echo "user $userid' not in MWAN
    and "/tmp/ppp/options.wan0" not in MWAN,
    "multi-WAN must quote credentials, atomically publish private per-session files, and retain command failures",
)
require(
    "PPPoE-MWAN quotes PPP credentials without altering their value: PASS" in MWAN_TEST
    and "PPPoE-MWAN rejects line-injected PPP credentials before launch: PASS" in MWAN_TEST
    and "PPPoE-MWAN avoids reusing a parent MAC suffix: PASS" in MWAN_TEST
    and "PPPoE-MWAN preserves the virtual MAC sequence for an unused parent suffix: PASS" in MWAN_TEST
    and "PPPoE-MWAN preserves first PPPD launch failure after later success: PASS" in MWAN_TEST
    and "PPPoE-MWAN refuses symlink credential files: PASS" in MWAN_TEST,
    "multi-WAN regression must cover credential escaping/injection, partial launch and symlink paths",
)

rpl2tp_start = function_body(PPP, "start_rpl2tp(int unit)")
session_start = 'status = eval("/usr/sbin/l2tp-control", "start-session 0.0.0.0");'
session_gate = rpl2tp_start.index(session_start)
cleanup_gate = rpl2tp_start.index("if (xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(status))", session_gate)
cleanup_call = rpl2tp_start.index("stop_status = kill_services(svcs_l2tp, 5, 1);", cleanup_gate)
cleanup_result = rpl2tp_start.index("xl2tpd_start_policy_rpl2tp_failure_status(status, stop_status)", cleanup_call)
restore_errno = rpl2tp_start.index("errno = saved_errno;", cleanup_result)
require(
    session_gate < cleanup_gate < cleanup_call < cleanup_result < restore_errno
    and "if (stop_status != 0)" in rpl2tp_start[cleanup_call:cleanup_result]
    and "rp-l2tp daemon rollback after session failure was incomplete" in rpl2tp_start,
    "RPL2TP session-start failure must stop its newly launched l2tpd, report rollback errors and preserve the primary errno",
)
require(
    "xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(1)" in XL2TP_TEST
    and "xl2tpd_start_policy_rpl2tp_failure_status(7, EBUSY) == 7" in XL2TP_TEST
    and "xl2tpd_start_policy_rpl2tp_failure_status(0, EBUSY) == EBUSY" in XL2TP_TEST
    and "xl2tpd_start_policy_rpl2tp_failure_requires_cleanup" in XL2TP_POLICY,
    "RPL2TP rollback policy must cover success, session failure and cleanup failure",
)

ppp_launch = function_body(PPP, "launch_wan_pppd_locked(int unit, int wan_proto)")
launch_failure_start = ppp_launch.index("if (launch_status != 0)")
launch_failure_open = ppp_launch.index("{", launch_failure_start)
launch_failure, _ = braced_body(ppp_launch, launch_failure_open)
invalidate = launch_failure.index("ppp_wan_session_generation_change(NULL)")
cleanup_gate = launch_failure.index('if (wan_proto != IPV4_WAN_PROTO_L2TP &&')
cleanup_call = launch_failure.index("cleanup_status = kill_services(svcs_pppd, 5, 1);")
cleanup_log = launch_failure.index("failed to stop partial multi-WAN PPP sessions after launch failure")
restore_errno = launch_failure.index("errno = saved_errno;")
require(
    invalidate < cleanup_gate < cleanup_call < cleanup_log < restore_errno,
    "partial multi-WAN launch failure must invalidate callback identity then stop all partial PPPD sessions",
)

for marker in (
    "trunk/user/rc/src/net_ppp.c",
    "scripts/check_pppoe_mwan_status.py",
    "trunk/user/pppoe/mwan.sh",
    "trunk/user/pppoe/tests/mwan_lifecycle.sh",
):
    require(WORKFLOW.count(f'"{marker}"') >= 2,
            "WAN push/pull triggers are missing " + marker)
require(
    'bash trunk/user/pppoe/tests/mwan_lifecycle.sh "$GITHUB_WORKSPACE"' in WORKFLOW
    and "python3 scripts/check_pppoe_mwan_status.py" in WORKFLOW
    and "bash trunk/user/pppoe/tests/detect_status.sh" in WORKFLOW,
    "WAN workflow must run detector + multi-WAN regressions and source contracts",
)
for marker in (
    "trunk/user/rc/src/net_ppp.c",
    "trunk/user/pppoe/detect.sh",
    "trunk/user/pppoe/tests/detect_status.sh",
    "scripts/check_pppoe_mwan_status.py",
    "trunk/user/pppoe/mwan.sh",
    "trunk/user/pppoe/tests/mwan_lifecycle.sh",
):
    require(f'"{marker}"' in XY_WORKFLOW,
            "XY-C1 push paths must include multi-WAN source and regression")
require(
    "bash -n trunk/user/pppoe/detect.sh" in XY_WORKFLOW
    and "bash trunk/user/pppoe/tests/detect_status.sh" in XY_WORKFLOW
    and 'bash trunk/user/pppoe/tests/mwan_lifecycle.sh "$GITHUB_WORKSPACE"' in XY_WORKFLOW
    and "python3 scripts/check_pppoe_mwan_status.py" in XY_WORKFLOW,
    "XY-C1 workflow must run detector and multi-WAN shell regressions/contracts before firmware build",
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
