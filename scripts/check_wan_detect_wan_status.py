#!/usr/bin/env python3
"""Verify gateway ARP checker inventory and launch failures reach WAN-up."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
POLICY = (RC / "process_presence_start_policy.h").read_text()
TEST = (RC / "tests/process_presence_start_policy.c").read_text()
DETACHED_TEST = (RC / "tests/detached_daemon_start_policy.c").read_text()
ROUTE_STATUS_TEST = (RC / "tests/route_status_policy.c").read_text()
RC_SOURCE = (RC / "rc.c").read_text()
RC_HEADER = (RC / "rc.h").read_text()
NETUTILS = (ROOT / "trunk/user/shared/src/netutils.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
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
        raise SystemExit("WAN gateway ARP checker status contract: FAIL: " + message)


copy_ifname = body(NETUTILS, "copy_ifname(")
require(
    "errno = EINVAL;" in copy_ifname
    and "len == 0 || len >= IFNAMSIZ || len >= dest_size" in copy_ifname,
    "invalid interface names must leave an errno for callers that distinguish missing IP from local lookup failure",
)
interface_addr = body(NETUTILS, "get_interface_addr4(const char *ifname)")
require(
    "lookup_error = errno ? errno : EIO;" in interface_addr
    and "if (close(sockfd) != 0 && lookup_error == 0)" in interface_addr
    and "if (ipv4_addr == INADDR_ANY && lookup_error != 0)" in interface_addr
    and "errno = lookup_error;" in interface_addr,
    "interface-address ioctl errors must survive descriptor close and remain visible to WAN health checks",
)

wan_up = body(WAN, "wan_up_internal_with_route_status(char *wan_ifname, int unit,")
gateway_scope = wan_up.rindex(
    "if (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)"
)
gateway_read = wan_up.index(
    "gateway_arp_status = get_nvram_match_checked_status("
)
gateway_error = wan_up.index("if (gateway_arp_status != 0)", gateway_read)
gateway_merge = wan_up.index(
    "status = route_status_accumulate(status, gateway_arp_status);",
    gateway_error,
)
gateway_log = wan_up.index(
    "cannot read gateway ARP checker enable setting", gateway_error
)
gateway_enabled = wan_up.index("else if (gateway_arp_enabled)", gateway_error)
scan = wan_up.index(
    'presence = process_inventory_scan("detect_wan", &snapshot);', gateway_enabled
)
require(
    gateway_scope < gateway_read < gateway_error < gateway_merge
    < gateway_log < gateway_enabled < scan,
    "DHCP gateway ARP must use checked enable state and preserve read errors before process inventory",
)
require(
    'nvram_match("gw_arp_ping", "1")' not in wan_up,
    "gateway ARP enable must not collapse NVRAM read errors to a disabled result",
)
release = wan_up.index("process_snapshot_release(&snapshot);", scan)
decision = wan_up.index("action = process_presence_start_action_for_state(presence);", release)
start_gate = wan_up.index("if (action == PROCESS_PRESENCE_START_NEW)", decision)
launch = wan_up.index("route_status = start_detect_wan_with_readiness();", start_gate)
status_map = wan_up.index(
    "route_status = process_presence_start_status_for_action(", launch
)
failure_gate = wan_up.index("if (route_status != 0)", status_map)
merge = wan_up.index("status = route_status_accumulate(status, route_status);", failure_gate)
unknown_gate = wan_up.index("if (action == PROCESS_PRESENCE_START_ABORT)", failure_gate)
unknown_diagnostic = wan_up.index(
    "skipping gateway ARP checker start because process inventory is unknown",
    unknown_gate,
)
launch_diagnostic = wan_up.index(
    "WAN gateway ARP checker startup incomplete", unknown_diagnostic
)
require(
    scan < release < decision < start_gate < launch < status_map < failure_gate
    < merge < unknown_gate < unknown_diagnostic < launch_diagnostic,
    "inventory and launch outcomes must be mapped, aggregated and logged in order",
)
require(WAN.count('eval("detect_wan")') == 1,
        "the daemonizing gateway ARP launcher must be invoked exactly once")
require("action, scan_error, route_status" in wan_up[status_map:merge],
        "UNKNOWN status mapping must receive scan error and launch result")
require(
    "route_status = 0;" in wan_up[decision:start_gate]
    and "if (action == PROCESS_PRESENCE_START_NEW)" in wan_up[decision:launch],
    "RUNNING and UNKNOWN must not launch another monitor",
)
require("route_status);" in wan_up[unknown_diagnostic:launch_diagnostic],
        "UNKNOWN inventory must report its effective error")

# detect_wan() daemonizes before PID-file publication; its parent exit is not
# proof that the detached monitor is alive and ready.
presence_scan = body(WAN, "detect_wan_startup_presence(void *context, int *scan_error)")
startup_wait = body(WAN, "detect_wan_startup_wait(void *context, unsigned int seconds)")
startup = body(WAN, "start_detect_wan_with_readiness(void)")
presence_compact = "".join(presence_scan.split())
require('#include "detached_daemon_start_policy.h"' in WAN
        and 'process_pidfile_read_identity("/var/run/detect_wan.pid","detect_wan",&identity)' in presence_compact
        and "presence==PROCESS_PRESENCE_UNKNOWN" in presence_compact
        and "scan_errno>0?scan_errno:EIO" in presence_compact
        and "*scan_error=0;" in presence_compact,
        "gateway monitor readiness must require the published PID/start-time identity")
wait_compact = "".join(startup_wait.split())
require("remaining=sleep(remaining);" in wait_compact
        and "errno!=EINTR" in wait_compact,
        "readiness waits must preserve the full interval across EINTR")
require('launch_status = eval("detect_wan");' in startup
        and "detached_daemon_start_postcondition_with_ops(" in startup
        and "detect_wan_startup_presence" in startup
        and "detect_wan_startup_wait" in startup
        and "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in startup
        and "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in startup
        and "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in startup,
        "detached detect_wan startup must require a bounded RUNNING postcondition")
require("test_delayed_daemon_appearance_is_accepted" in DETACHED_TEST
        and "test_absent_child_times_out" in DETACHED_TEST
        and "test_unknown_inventory_fails_closed" in DETACHED_TEST
        and "test_wait_interruption_and_invalid_arguments_fail" in DETACHED_TEST,
        "detached-daemon regression must cover delayed, absent, UNKNOWN and interrupted states")
require(WORKFLOW.count('"trunk/user/rc/src/tests/detached_daemon_start_policy.c"') >= 2
        and "Compile and run shared detached-daemon startup readiness regression" in WORKFLOW,
        "WAN push/pull validation must compile the detached-daemon readiness policy")

post_hook = wan_up.index("/* Only invoke the optional hook with a complete, stable IPv4 snapshot. */")
post_hook_guard = wan_up.index("if (check_if_file_exist(script_postw))", post_hook)
post_hook_eval = wan_up.index(
    'route_status = eval(script_postw, "up", wan_ifname, wan_addr);',
    post_hook_guard,
)
post_hook_merge = wan_up.index(
    "status = route_status_accumulate(status, route_status);", post_hook_eval
)
post_hook_failure = wan_up.index("if (route_status != 0)", post_hook_merge)
post_hook_log = wan_up.index("WAN post-up script incomplete", post_hook_failure)
later_zerotier = wan_up.index("#if defined(APP_ZEROTIER)", post_hook_log)
require(
    post_hook < post_hook_guard < post_hook_eval < post_hook_merge
    < post_hook_failure < post_hook_log < later_zerotier,
    "post-WAN hook exit status must be captured, aggregated and logged before later independent starts",
)
require(wan_up.count('eval(script_postw, "up", wan_ifname, wan_addr)') == 1,
        "post-WAN hook must execute exactly once")
require("test_wan_post_script_failure_is_reported_and_later_steps_continue" in ROUTE_STATUS_TEST,
        "post-WAN script failure must have a first-error/later-step regression")

vpn_notify = wan_up.index("/* Queue the deferred VPN-client startup and preserve notification errors. */")
vpn_call = wan_up.index('route_status = notify_rc_status("start_vpn_client");', vpn_notify)
vpn_merge = wan_up.index("status = route_status_accumulate(status, route_status);", vpn_call)
vpn_failure = wan_up.index("if (route_status != 0)", vpn_merge)
vpn_log = wan_up.index("WAN VPN client start notification incomplete", vpn_failure)
later_gateway = wan_up.index("A failed gateway-ARP enable read", vpn_log)
require(
    vpn_notify < vpn_call < vpn_merge < vpn_failure < vpn_log < later_gateway,
    "WAN deferred VPN-client notification failure must be preserved before subsequent checks",
)
require('notify_rc("start_vpn_client")' not in wan_up,
        "WAN VPN client startup notification must not discard queue status")
require(
    "test_wan_vpn_start_notification_failure_is_reported_without_skipping_later_actions" in ROUTE_STATUS_TEST,
    "deferred VPN-client notification failure needs a regression",
)

down_hook = body(WAN, "wan_down_internal(char *wan_ifname, int unit, int is_static)")
down_hook_guard = down_hook.index("if (check_if_file_exist(script_postw))")
down_hook_eval = down_hook.index(
    'route_status = eval(script_postw, "down", wan_ifname, wan_addr);',
    down_hook_guard,
)
down_hook_merge = down_hook.index(
    "status = route_status_accumulate(status, route_status);", down_hook_eval
)
down_hook_failure = down_hook.index("if (route_status != 0)", down_hook_merge)
down_hook_log = down_hook.index("WAN post-down script incomplete", down_hook_failure)
later_mwan_read = down_hook.index(
    "pppoemwan_status = get_nvram_match_checked_status(", down_hook_log
)
later_mwan_error = down_hook.index(
    "if (pppoemwan_status != 0)", later_mwan_read
)
later_mwan_error_merge = down_hook.index(
    "status = route_status_accumulate(status, pppoemwan_status);",
    later_mwan_error,
)
later_mwan_enabled = down_hook.index(
    "else if (pppoemwan_enabled)", later_mwan_error
)
later_mwan_eval = down_hook.index(
    'route_status = eval("/usr/bin/detect.sh");', later_mwan_enabled
)
down_return = down_hook.index("return status;", later_mwan_eval)
require(
    'nvram_match("pppoemwan_enable", "1")' not in down_hook
    and down_hook_log < later_mwan_read < later_mwan_error
    < later_mwan_error_merge < later_mwan_enabled < later_mwan_eval < down_return,
    "post-down hook failure must be aggregated before checked PPPoE-MWAN detection and WAN return",
)


static_down_start = down_hook.index(
    "if (modem_status == 0 && !modem_unit_id && proto_status == 0 &&"
)
static_down = down_hook[static_down_start:].split(
    "/* Flush conntrack only when the checked previous WAN address is valid. */", 1
)[0]
reset_wan = body(WAN, "reset_wan_vars_status(void)")
for name, initializer, invalid_assignment in (
    ("man_addr", 'char man_addr[INET_ADDRSTRLEN] = "0.0.0.0";', 'man_addr = "0.0.0.0";'),
    ("man_mask", 'char man_mask[INET_ADDRSTRLEN] = "0.0.0.0";', 'man_mask = "0.0.0.0";'),
    ("man_gate", 'char man_gate[INET_ADDRSTRLEN] = "0.0.0.0";', 'man_gate = "0.0.0.0";'),
    ("man_mtu", 'char man_mtu[16] = "";', 'man_mtu = "";'),
):
    assert initializer in reset_wan, f"{name} must retain its bounded initialized buffer"
    assert invalid_assignment not in reset_wan, f"{name} array must not be assigned as a pointer"

static_clear = static_down.index(
    'route_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);'
)
static_accumulate = static_down.index(
    "status = route_status_accumulate(status, route_status);", static_clear
)
force_down_gate = static_down.index(
    "if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, route_status))",
    static_accumulate,
)
force_down_call = static_down.index(
    "interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);",
    force_down_gate,
)
force_down_accumulate = static_down.index(
    "status = route_status_accumulate(status, interface_cleanup_status);",
    force_down_call,
)
force_down_failure = static_down.index(
    "if (interface_cleanup_status != 0)", force_down_accumulate
)
require(
    static_clear < static_accumulate < force_down_gate < force_down_call
    < force_down_accumulate < force_down_failure,
    "static WAN teardown address-clear failure must force interface-down and aggregate the result",
)
require(
    "failed to clear static WAN address on %s (error=%d); forcing interface down" in static_down
    and "failed to take static WAN interface %s down after address cleanup failure (status=%d)" in static_down,
    "static WAN teardown clear and force-down failures must both be diagnosed",
)
require(
    "test_static_ipoe_teardown_clear_failure_forces_interface_down" in ROUTE_STATUS_TEST
    and "test_static_ipoe_teardown_clear_failure_forces_interface_down();" in ROUTE_STATUS_TEST,
    "static WAN teardown failure must have a registered policy regression",
)

require(
    down_hook_guard < down_hook_eval < down_hook_merge < down_hook_failure
    < down_hook_log < later_mwan_read < later_mwan_error
    < later_mwan_error_merge < later_mwan_enabled < later_mwan_eval < down_return,
    "post-down hook failure must be captured and aggregated without skipping later PPPoE-MWAN reconfiguration",
)
require(down_hook.count('eval(script_postw, "down", wan_ifname, wan_addr)') == 1,
        "post-down hook must execute exactly once")
require(
    "test_wan_post_down_script_failure_is_reported_and_later_steps_continue" in ROUTE_STATUS_TEST,
    "post-down script failure must have a first-error/later-action regression",
)

require("return status;" in wan_up[launch_diagnostic:],
        "WAN-up must return the accumulated gateway checker result")

status_policy = body(
    POLICY,
    "process_presence_start_status_for_action(",
)
for expected in (
    "case PROCESS_PRESENCE_START_NEW:",
    "return launch_status;",
    "case PROCESS_PRESENCE_START_NOTIFY:",
    "return 0;",
    "case PROCESS_PRESENCE_START_ABORT:",
    "return inventory_error > 0 ? inventory_error : EIO;",
    "default:",
    "return EIO;",
):
    require(expected in status_policy, "process status policy missing " + expected)

for token in (
    "PROCESS_PRESENCE_START_NOTIFY, EIO, EACCES) == 0",
    "PROCESS_PRESENCE_START_ABORT, ENOMEM, 0) == ENOMEM",
    "PROCESS_PRESENCE_START_ABORT, 0, 0) == EIO",
    "(enum process_presence_start_action)7, 0, 0) == EIO",
):
    require(token in TEST, "tri-state unit test missing " + token)
require(
    "trunk/user/rc/src/tests/process_presence_start_policy.c" in PROCESS_WORKFLOW
    and "Compile and run process presence start/notify tri-state regression" in PROCESS_WORKFLOW,
    "process workflow must compile and run the changed policy regression",
)

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

rc_main = body(RC_SOURCE, "main(int argc, char **argv)")
stop_wan_command = rc_main.split(
    'else if (!strcmp(base, "stop_wan"))', 1
)[1].split('else if (!strcmp(base, "restart_wan"))', 1)[0]
restart_wan_command = rc_main.split(
    'else if (!strcmp(base, "restart_wan"))', 1
)[1].split('else if (!strcmp(base, "restart_dns"))', 1)[0]
require(
    'ret = notify_rc_status("manual_wan_disconnect");' in stop_wan_command
    and 'notify_rc("manual_wan_disconnect")' not in stop_wan_command,
    "stop_wan must return the manual disconnect enqueue result",
)
require(
    'ret = notify_rc_status("manual_wan_reconnect");' in restart_wan_command
    and 'notify_rc("manual_wan_reconnect")' not in restart_wan_command,
    "restart_wan must return the manual reconnect enqueue result",
)
require("return ret;" in rc_main,
        "manual WAN notification failures must reach the rc command exit status")

manual_disconnect = body(WAN, "manual_wan_disconnect(void)")
require(
    "int\nmanual_wan_disconnect(void)" in WAN
    and "int manual_wan_disconnect(void);" in RC_HEADER,
    "manual WAN disconnect must return its teardown status through the public declaration",
)
require(
    "status = stop_wan();" in manual_disconnect
    and "status = stop_wan_ppp();" in manual_disconnect
    and "if (status != 0)" in manual_disconnect
    and "manual WAN disconnect failed (status=%d)" in manual_disconnect
    and "return status;" in manual_disconnect,
    "manual WAN disconnect must report stop_wan/stop_wan_ppp failures",
)
require(
    "ap_mode_status = get_ap_mode_status(&is_ap_mode);" in manual_disconnect
    and "if (ap_mode_status != 0)" in manual_disconnect
    and "return ap_mode_status;" in manual_disconnect
    and "if (is_ap_mode)" in manual_disconnect
    and "return 0;" in manual_disconnect,
    "manual WAN disconnect must fail closed on unknown mode and remain a no-op in AP mode",
)
manual_disconnect_dispatch = body(RC_SOURCE, "handle_notifications(void)")
manual_disconnect_action = manual_disconnect_dispatch.split(
    'else if(!strcmp(entry->d_name, "manual_wan_disconnect"))', 1
)[1].split('else if(!strcmp(entry->d_name, "manual_wisp_reassoc"))', 1)[0]
require(
    "rc_notify_merge_action_result(" in manual_disconnect_action
    and "manual_wan_disconnect()" in manual_disconnect_action,
    "PID 1 must retain the manual disconnect queue marker when WAN teardown fails",
)
manual_wisp = body(WAN, "manual_wisp_reassoc(void)")
require(
    "int\nmanual_wisp_reassoc(void)" in WAN
    and "int manual_wisp_reassoc(void);" in RC_HEADER,
    "manual WISP reassociation must return status through the public declaration",
)
require(
    "if (!wisp_ifname)" in manual_wisp
    and "return 0;" in manual_wisp
    and "reconnect_status = reconnect_apcli(wisp_ifname, 1);" in manual_wisp
    and "if (reconnect_status != 0)" in manual_wisp
    and "manual WISP reassociation failed on %s (status=%d)" in manual_wisp
    and "return reconnect_status;" in manual_wisp,
    "manual WISP reassociation must expose APCLI reconnect status and preserve no-interface no-op",
)
manual_wisp_dispatch = body(RC_SOURCE, "handle_notifications(void)")
manual_wisp_action = manual_wisp_dispatch.split(
    'else if(!strcmp(entry->d_name, "manual_wisp_reassoc"))', 1
)[1].split('else if(!strcmp(entry->d_name, RCN_MANUAL_DDNS_UPDATE))', 1)[0]
require(
    "rc_notify_merge_action_result(" in manual_wisp_action
    and "manual_wisp_reassoc()" in manual_wisp_action,
    "PID 1 must retain the manual WISP notification marker when reassociation fails",
)

print("WAN gateway ARP inventory and launch status propagation: PASS")
