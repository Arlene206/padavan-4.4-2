#!/usr/bin/env python3
"""Verify detached startup readiness for UDPXY, MSD Lite and IGMPproxy."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
net = (RC / "net.c").read_text()
policy = (RC / "detached_daemon_start_policy.h").read_text()
test = (RC / "tests/detached_daemon_start_policy.c").read_text()
udpxy = (ROOT / "trunk/user/udpxy/src/udpxy.c").read_text()


def function_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    index = brace
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


presence = function_body(net, "iptv_proxy_process_presence(void *context, int *scan_error)")
assert "process_inventory_scan(process_name, &snapshot)" in presence
assert "process_snapshot_release(&snapshot)" in presence
assert "PROCESS_PRESENCE_UNKNOWN" in presence
wait = function_body(net, "iptv_proxy_process_wait(void *context, unsigned int seconds)")
assert "sleep(seconds)" in wait
assert "errno = EINTR;" in wait
confirm = function_body(net, "iptv_proxy_confirm_started(const char *process_name, int launch_status)")
assert "detached_daemon_start_postcondition_with_ops(" in confirm
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in confirm
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in confirm
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in confirm

startup = function_body(net, "start_udpxy(char *wan_ifname)")
assert 'process_name = "msd_lite";' in startup
assert 'process_name = "udpxy";' in startup
assert "iptv_proxy_process_presence((void *)process_name, &scan_error)" in startup
assert "PROCESS_PRESENCE_UNKNOWN" in startup
assert "process_state != PROCESS_PRESENCE_ABSENT" in startup
assert startup.index("iptv_proxy_process_presence(") < startup.index("msd_lite_config_publish(")
assert startup.index("iptv_proxy_process_presence(") < startup.index('eval("/usr/sbin/udpxy"')
assert 'eval("/usr/bin/msd_lite", "-d", "-c", dst_conf)' in startup
assert 'eval("/usr/sbin/udpxy"' in startup
assert "launch_status = iptv_proxy_confirm_started(process_name, launch_status);" in startup
assert "return launch_status;" in startup

# The upstream daemon defaults to detaching when root; -T explicitly opts out.
assert "custom_log = 0, no_daemon = 0;" in udpxy
assert "case 'T': no_daemon = 1;" in udpxy
assert 'if( 0 == geteuid() )' in udpxy
assert "if( !no_daemon )" in udpxy
assert "daemonize(0, g_flog)" in udpxy
assert "test_delayed_daemon_appearance_is_accepted" in test
assert "test_absent_child_times_out" in test
assert "test_unknown_inventory_fails_closed" in test
assert "test_wait_interruption_and_invalid_arguments_fail" in test
assert "Detached-daemon startup postcondition regression: PASS" in test
assert "detached_daemon_start_rollback_with_ops" in policy
assert "test_failed_start_rolls_back_and_preserves_primary_error" in test
assert "test_running_child_after_failed_cleanup_keeps_primary_status" in test
assert "test_unknown_rollback_inventory_preserves_primary_status" in test
rollback_stop = function_body(net, "iptv_proxy_process_stop(void *context)")
rollback = function_body(net, "iptv_proxy_rollback_failed_start(")
assert "kill_services(service_names, 3, 1)" in rollback_stop
assert "detached_daemon_start_rollback_with_ops(" in rollback
assert "PROCESS_PRESENCE_ABSENT" in rollback
assert "iptv_proxy_rollback_failed_start(" in startup
assert "iptv_proxy_rollback_failed_start(" in igmpproxy_start
xupnpd_start = function_body(net, "start_xupnpd(char *wan_ifname)")
assert "iptv_proxy_rollback_failed_start(" in xupnpd_start

igmpproxy_start = function_body(net, "start_igmpproxy(char *wan_ifname)")
assert "PKG_VERSION:=0.3" in (ROOT / "trunk/user/igmpproxy/Makefile").read_text()
assert 'iptv_proxy_process_presence((void *)"igmpproxy", &scan_error)' in igmpproxy_start
assert "PROCESS_PRESENCE_UNKNOWN" in igmpproxy_start
assert "process_state != PROCESS_PRESENCE_ABSENT" in igmpproxy_start
assert igmpproxy_start.index("iptv_proxy_process_presence(") < igmpproxy_start.index("igmpproxy_config_publish(")
assert 'eval("/bin/igmpproxy", igmpproxy_conf)' in igmpproxy_start
assert 'iptv_proxy_confirm_started("igmpproxy", launch_status)' in igmpproxy_start
assert "return rc_first_service_start_status(start_status, launch_status);" in igmpproxy_start

print("UDPXY, MSD Lite and IGMPproxy detached startup readiness/fail-closed preflight: PASS")
