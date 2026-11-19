#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/zerotier/zerotier.sh"
test_path = ROOT / "trunk/user/zerotier/tests/zerotier_lifecycle.sh"
makefile_path = ROOT / "trunk/user/zerotier/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/zerotier-lifecycle.yml"

script = script_path.read_text()
test = test_path.read_text()
makefile = makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"ZeroTier lifecycle source contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    require(start >= 0, f"missing function {signature}")
    opening = source.find("{", start + len(signature))
    require(opening >= 0, f"missing opening brace for {signature}")

    depth = 0
    state = "code"
    pos = opening
    while pos < len(source):
        char = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif char == "/" and nxt == "/":
                state = "line_comment"
                pos += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:pos]
        elif state in ("string", "char"):
            if char == "\\\\":
                pos += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and char == "\\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)

wait_body = function_body(script, "zerotier_wait_stopped()")
kill_body = function_body(script, "kill_z()")
start_body = function_body(script, "start_zero()")
stop_body = function_body(script, "stop_zero()")
start_instance_body = function_body(script, "start_instance()")
interface_body = function_body(script, "zerotier_interface_name()")
route_section = script.split("zerotier_kernel_route_segment() {", 1)[1].split("\nzerotier_wait_interface() {", 1)[0]
interface_wait_body = function_body(script, "zerotier_wait_interface()")
route_wait_body = function_body(script, "zerotier_wait_route()")
rules_section = script.split("rules() {", 1)[1].split("\ndel_rules() {", 1)[0]
firewall_helper_body = function_body(script, "zerotier_add_firewall_rule()")
cleanup_section = script.split("del_rules() {", 1)[1].split("\nzero_route() {", 1)[0]
del_rules_body = function_body(script, "del_rules()")
delete_rule_body = function_body(script, "zerotier_delete_firewall_rule()")
rule_action_body = function_body(script, "zerotier_firewall_rule_action()")
state_save_body = function_body(script, "zerotier_firewall_state_save()")
state_load_body = function_body(script, "zerotier_firewall_state_load()")
static_route_body = function_body(script, "zero_route()")
route_match_body = function_body(script, "zero_route_entry_matches()")

process_state_body = function_body(script, "zerotier_process_state()")
require('while [ "$zerotier_wait" -lt 5 ]; do' in wait_body and
        'zerotier_process_state' in wait_body and 'sleep 1' in wait_body,
        "daemon absence must be checked through five bounded one-second tri-state process polls")
require('pidof zerotier-one' in process_state_body and
        'zt_process_pidof_status' in process_state_body and
        'return 2' in process_state_body and
        'return 1' in process_state_body and
        'return 0' in process_state_body,
        "process inventory must distinguish RUNNING, confirmed ABSENT and UNKNOWN")
require('if [ "$zerotier_stop_state" -ne 1 ]; then' in kill_body and
        kill_body.index('zerotier_wait_stopped') < kill_body.index('if [ "$zerotier_stop_state" -ne 1 ]; then') <
        kill_body.index('killall -9 zerotier-one'),
        "UNKNOWN during graceful stop must block name-wide SIGKILL escalation")
require(kill_body.count('killall zerotier-one') == 1 and
        kill_body.count('killall -9 zerotier-one') == 1,
        "stop must send one TERM and at most one process-name SIGKILL")
require(kill_body.index('killall zerotier-one') < kill_body.index('zerotier_wait_stopped') <
        kill_body.index('killall -9 zerotier-one') <
        kill_body.rindex('zerotier_wait_stopped'),
        "TERM grace period and force-stop absence postcondition are out of order")
require('return 1' not in kill_body or 'zerotier_wait_stopped' in kill_body,
        "stop failure must propagate the bounded absence result")
require('if ! kill_z; then' in start_body and
        start_body.index('if ! kill_z; then') < start_body.index("start_instance 'zerotier'"),
        "start must not launch a new daemon unless the previous daemon is absent")

require("ZEROTIER_RULE_DELETE_LIMIT=32" in script and
        'zerotier_rule_delete_count' in delete_rule_body and
        'zerotier_firewall_rule_action -C "$zerotier_rule_table" "$@"' in delete_rule_body and
        'zerotier_firewall_rule_action -D "$zerotier_rule_table" "$@"' in delete_rule_body and
        'case "$zerotier_rule_table" in' in delete_rule_body and
        'iptables -t "$zerotier_firewall_action_table" "$zerotier_firewall_action" "$@"' in rule_action_body and
        'iptables "$zerotier_firewall_action" "$@"' in rule_action_body,
        "firewall deletion must check rule presence, bound duplicates and put the table selector before -C/-D")
require('1) return 0' in delete_rule_body and
        'zerotier_rule_check_status' in delete_rule_body and
        'return 1' in delete_rule_body,
        "only iptables check status 1 may be treated as an absent rule; other failures must propagate")
require('mktemp "$config_path/.firewall-state.XXXXXX"' in state_save_body and
        'chmod 600' in state_save_body and
        'mv -f "$zt_state_tmp" "$zt_state_file"' in state_save_body,
        "firewall retry identity must be atomically published with private permissions")
require('[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ]' in state_load_body and
        'zt_state_saved_interface' in state_load_body and
        'zt_state_saved_segment' in state_load_body,
        "firewall retry state must reject symlinks and restore the validated interface/network")
require(del_rules_body.count('zerotier_delete_firewall_rule') == 5 and
        'del_rules_status=1' in del_rules_body and
        'return "$del_rules_status"' in del_rules_body,
        "all applicable firewall rules must be attempted and aggregate deletion failures")
require('if ! del_rules; then' in rules_section and
        rules_section.index('if ! del_rules; then') < rules_section.index('zerotier_add_firewall_rule -A INPUT'),
        "new firewall rules must not publish when stale-rule cleanup is unverified")
require('if ! zerotier_firewall_state_save; then' in stop_body and
        stop_body.index('if ! zerotier_firewall_state_save; then') < stop_body.index('if ! kill_z; then') <
        stop_body.index('if ! del_rules; then') < stop_body.index('if ! zero_route "del"; then'),
        "stop must persist cleanup identity first and preserve config/routes until checked firewall deletion succeeds")

require('if [ -z "${zt0:-}" ]; then' in stop_body and
        stop_body.index('zt0="$(zerotier_interface_name)"') <
        stop_body.index('ip_segment="$(zerotier_kernel_route_segment "$zt0")"') <
        stop_body.index('if ! kill_z; then'),
        "stop must capture both the ZeroTier interface and NAT source segment before daemon shutdown")
require('if [ -z "${ip_segment:-}" ]; then' in del_rules_body and
        'ip_segment="$(zerotier_kernel_route_segment "$zt0")"' in del_rules_body,
        "firewall cleanup must reuse a pre-captured NAT source segment after the daemon route disappears")
require('[ "$(nvram get zerotier_nat)" = "1" ] && [ -z "$ip_segment" ]' in stop_body and
        'return 1' in stop_body,
        "NAT-enabled stop must fail before daemon termination when its cleanup route cannot be captured")
require('if ! kill_z; then' in stop_body and
        stop_body.index('if ! kill_z; then') < stop_body.index('del_rules') <
        stop_body.index('if ! zero_route "del"; then') <
        stop_body.index('rm -rf "$config_path"') and
        'return 1' in stop_body[stop_body.index('if ! zero_route "del"; then'):stop_body.index('rm -rf "$config_path"')],
        "failed daemon or route cleanup must preserve persistent identity/config")
require(not re.search(r'\bkill\s+-9\s+"?\$zerotier_process', script),
        "stale numeric PID must never be force-signalled")
require('$(pidof zerotier-one)' not in script,
        "numeric pidof snapshots must not be cached")

require("ZT_READY_TIMEOUT=30" in script,
        "interface and kernel-route readiness must have a bounded 30-second deadline")
require("ifconfig | awk '$1 ~ /^zt/" in interface_body,
        "interface discovery must select actual ZeroTier interface names")
require('awk -v zt_if="$1"' in route_section and
        'dev == zt_if && proto == "kernel"' in route_section,
        "kernel route parsing must match exact interface and protocol fields")
require('while :' in interface_wait_body and
        'zerotier_interface_name' in interface_wait_body and
        'sleep 1' in interface_wait_body and
        '"$zerotier_wait" -ge "$ZT_READY_TIMEOUT"' in interface_wait_body,
        "interface readiness wait must be bounded and re-check at the deadline")
require('while :' in route_wait_body and
        'zerotier_kernel_route_segment "$1"' in route_wait_body and
        'sleep 1' in route_wait_body and
        '"$zerotier_wait" -ge "$ZT_READY_TIMEOUT"' in route_wait_body,
        "kernel-route readiness wait must be bounded and re-check at the deadline")
require('zerotier_wait_interface' in rules_section and
        'zerotier_wait_route "$zt0"' in rules_section and
        rules_section.index('zerotier_wait_route "$zt0"') < rules_section.index('del_rules') <
        rules_section.index('zerotier_add_firewall_rule -A INPUT'),
        "firewall rules must be published only after interface and required route readiness")
require('if iptables "$@"; then' in firewall_helper_body and
        'del_rules' in firewall_helper_body and
        'return 1' in firewall_helper_body,
        "a failed firewall append must trigger rollback and a nonzero result")
for required_rule in (
    'zerotier_add_firewall_rule -A INPUT -i "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -A FORWARD -i "$zt0" -o "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -A FORWARD -i "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -t nat -A POSTROUTING -o "$zt0" -j MASQUERADE',
    'zerotier_add_firewall_rule -t nat -A POSTROUTING -s "$ip_segment" -j MASQUERADE',
):
    require(required_rule in rules_section,
            "every required INPUT/FORWARD/NAT append must use the checked firewall publisher")
require(rules_section.count('zerotier_add_firewall_rule') == 5,
        "the full three-filter/two-NAT firewall ruleset must use checked appends")
require('case "$nat_enable" in' in rules_section and
        '[ "$nat_enable" = "1" ]' in rules_section,
        "NAT configuration must be handled without an unquoted numeric test")
cleanup_branch = start_instance_body.index('if ! kill_z; then')
cleanup_failure_return = start_instance_body.index('return 1', cleanup_branch)
require('if ! rules; then' in start_instance_body and
        start_instance_body.index('if ! rules; then') < cleanup_branch < cleanup_failure_return,
        "daemon startup must fail and attempt cleanup when interface/route readiness times out")
require('if ! zero_route "add"; then' in rules_section and
        'del_rules' in rules_section and
        'zero_route "del"' in rules_section,
        "a failed static-route add must roll back the published firewall/route state")
require("grep -cE '(^-D |^-t nat -D )' in test,
        "firewall regression must count both filter and table-qualified NAT deletes")
require('running-only' in test and
        'firewall cleanup lost the interface after daemon exit' in test and
        'static route cleanup lost the saved interface after daemon exit' in test and
        'daemon-only)' in test and
        'firewall cleanup lost the pre-stop NAT source segment' in test and
        'stop accepted iptables deletion failure' in test and
        'firewall deletion failure did not persist retry state' in test and
        'stop could not retry firewall cleanup from saved state after daemon exit' in test and
        'retry did not check the saved NAT source segment' in test and
        '"-t nat -C POSTROUTING -s 10.147.17.0/24 -j MASQUERADE"' in test and
        'stop accepted NAT-enabled shutdown without a connected route segment' in test,
        "stop regression must model disappearance of both the interface and kernel NAT source route")
missing_nat_route_case = test.split("# Missing NAT route is bounded", 1)[1].split("# Malformed NVRAM NAT state", 1)[0]
require("TEST_NAT_ENABLE=1" in missing_nat_route_case,
        "missing-kernel-route regression must explicitly enable NAT before expecting route readiness to fail")
require('if ! ip route add "$zero_ip" via "$zero_route" dev "$zt0"; then' in static_route_body and
        'return 1' in static_route_body and
        '静态路由安装失败' in static_route_body,
        "static-route add errors must propagate instead of being discarded")
require('ip route show "$zero_ip"' in static_route_body and
        'ip route del "$zero_ip" via "$zero_route" dev "$zt0"' in static_route_body and
        'route_match_status' in static_route_body and
        '静态路由删除失败' in static_route_body and
        '静态路由删除后仍然存在' in static_route_body,
        "static-route cleanup must inspect the exact tuple, propagate delete failure and verify absence")
require('$1 == destination' in route_match_body and
        'via == gateway && dev == interface' in route_match_body,
        "static-route cleanup must match the exact destination, gateway and interface tuple")

require('zerotier_kernel_route_segment "$zt0"' in cleanup_section,
        "firewall cleanup must use the same token-based route parser")

require('$(ROMFSINST) -p +x $(THISDIR)/zerotier.sh /usr/bin/zerotier.sh' in makefile,
        "ROMFS must install the exercised ZeroTier launcher")
start = function_body(services, "int start_zerotier(void)")
require('eval("/usr/bin/zerotier.sh","stop")' in services and
        'return eval("/usr/bin/zerotier.sh","start");' in start and
        'if (zerotier_enable != 1)' in start and
        'int start_zerotier(void);' in rc_header,
        "RC ZeroTier start wrapper must propagate launch status and preserve disabled mode")
stop = function_body(services, "int stop_zerotier(void)")
require('return eval("/usr/bin/zerotier.sh","stop");' in stop and
        'int stop_zerotier(void);' in rc_header,
        "RC ZeroTier stop wrapper must propagate launcher status")
restart_start = services.rfind("int restart_zerotier(void)")
require(restart_start >= 0, "missing checked ZeroTier restart wrapper")
restart = function_body(services[restart_start:], "int restart_zerotier(void)")
restart_compact = " ".join(restart.split())
require('int stop_status = stop_zerotier();' in restart_compact and
        'if (stop_status != 0)' in restart_compact and
        'return stop_status;' in restart_compact and
        'return start_zerotier();' in restart_compact and
        'start_zerotier();' not in restart_compact.replace('return start_zerotier();', '') and
        restart_compact.index('return stop_status;') < restart_compact.index('return start_zerotier();'),
        "RC ZeroTier restart must preserve stop and start launcher statuses")
require('int restart_zerotier(void);' in rc_header and
        'action_result = rc_notify_merge_action_result(action_result, restart_zerotier());' in rc,
        "RC ZeroTier restart status must reach the notification result")
stop_services = function_body(services, "int\nstop_services(int stopall)")
require('record_service_stop_status(&stop_status, "ZeroTier", stop_zerotier())' in stop_services,
        "stop_services must aggregate ZeroTier stop status")

for token in ['graceful', '-9-exits', 'stuck', 'start_zero', 'stop_zero',
              'identity.secret', 'numeric-kill.log', 'five one-second polls',
              'interface readiness', 'kernel route', 'single-space',
              'configured static route add failed', 'zero_route:add',
              'failed static route deletion', 'unreadable static route inventory',
              'route that remained after a successful delete command']:
    require(token in test, f"regression coverage missing {token}")
require('grep -Fqx -- "-t nat -D POSTROUTING -o ztunit -j MASQUERADE" "$TEST_IPTABLES_LOG"' in test,
        "NAT append rollback must assert the table-qualified deletion command")
require("grep -cE '(^| )-A '" in test,
        "static-route rollback regression must count filter and NAT appends whose -A token occurs at different columns")

paths = [
    '.github/workflows/zerotier-lifecycle.yml',
    'scripts/check_zerotier_lifecycle.py',
    'trunk/user/zerotier/zerotier.sh',
    'trunk/user/zerotier/tests/zerotier_lifecycle.sh',
    'trunk/user/zerotier/Makefile',
    'trunk/user/rc/src/services.c',
    'trunk/user/rc/src/rc.h',
    'trunk/user/rc/src/rc.c',
]
push = workflow.split('  push:', 1)[1].split('  pull_request:', 1)[0]
pull = workflow.split('  pull_request:', 1)[1].split('permissions:', 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("ZeroTier bounded lifecycle, persistent-config and RC integration contracts: PASS")
