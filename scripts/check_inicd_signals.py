#!/usr/bin/env python3
"""Source and lifecycle contracts for deferred iNIC health-monitor signals."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return (ROOT / relative).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and nxt == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state in ("string", "char"):
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


source = read("trunk/user/rc/src/inicd.c")
policy = read("trunk/user/rc/src/inicd_signal_policy.h")
test = read("trunk/user/rc/src/tests/inicd_signal_policy.c")
wifi = read("trunk/user/rc/src/net_wifi.c")
rc = read("trunk/user/rc/src/rc.c")
rc_header = read("trunk/user/rc/src/rc.h")
pidfile_header = read("trunk/user/shared/src/process_pidfile.h")
src_makefile = read("trunk/user/rc/src/Makefile")
package_makefile = read("trunk/user/rc/Makefile")
workflow = read(".github/workflows/inicd-signal-stability.yml")

handler = function_body(source, "catch_sig_inicd(int sig)")
assert "inicd_record_signal(&pending_events, sig)" in handler
for forbidden in (
    "check_inic_mii_rebooted(", "doSystem(", "logmessage(",
    "inicd_alarmtimer(", "remove(", "exit(",
):
    assert forbidden not in handler, forbidden

main = function_body(source, "inicd_main(int argc, char *argv[])")
block = main.index("sigprocmask(SIG_BLOCK, &managed_signals, &previous_mask)")
install_alarm = main.index("sigaction(SIGALRM, &sa, NULL)")
install_term = main.index("sigaction(SIGTERM, &sa, NULL)")
assert block < install_alarm < install_term
assert "sa.sa_mask = managed_signals" in main
assert "sigdelset(&wait_mask, SIGALRM)" in main
assert "sigdelset(&wait_mask, SIGTERM)" in main
assert "sigsuspend(&wait_mask)" in main
assert "pause()" not in main
ignore_actions = main.index("if (sigaction(SIGPIPE, &sa, NULL) < 0")
daemon_start = main.index("if (daemon(0, 0) < 0)")
assert ignore_actions < daemon_start
for signum in ("SIGPIPE", "SIGHUP", "SIGUSR1", "SIGUSR2"):
    assert f"sigaction({signum}, &sa, NULL)" in main[ignore_actions:daemon_start]
assert "inicd_signal_action_error(-1, errno)" in main[ignore_actions:daemon_start]
assert "(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);" in main[ignore_actions:daemon_start]
assert "Failed to install auxiliary signal dispositions" in main[ignore_actions:daemon_start]
take = main.index("inicd_take_pending_events(&pending_events)")
pending = main.index("sigpending(&pending_signals)")
pending_stop = main.index("sigismember(&pending_signals, SIGTERM)")
stop = main.index("inicd_event_should_stop(events)")
poll = main.index("if (events & INICD_EVENT_POLL)")
callback = main.index("poll_status = check_inic_mii_rebooted();")
poll_error = main.index("if (poll_status != 0)", callback)
poll_log = main.index('"iNIC health check incomplete (status=%d); retrying at next poll"', poll_error)
assert take < pending < pending_stop < stop < poll < callback < poll_error < poll_log
assert "int poll_status;" in main
assert "events == 0" in main
retry_wait = main.index("if (events == 0 &&", poll_error)
assert poll_log < retry_wait
assert "break;" not in main[poll_error:retry_wait]
publish_pidfile = main.index('process_pidfile_write(INICD_PID_FILE, "inicd")')
publish_failure = main.index("if (process_pidfile_write(INICD_PID_FILE, \"inicd\") != 0)")
timer_start = main.index("timer_status = inicd_alarmtimer(INICD_POLL_INTERVAL)")
assert publish_failure < timer_start
assert "Failed to publish identity PID file" in main[publish_failure:timer_start]
assert "return saved_errno;" in main[publish_failure:timer_start]
assert main.index("inicd_cleanup();", main.index("for (;;)")) > callback
assert "if (timer_status < 0)" in main

cleanup = function_body(source, "inicd_cleanup(void)")
assert "inicd_alarmtimer(0)" in cleanup
assert "remove(INICD_PID_FILE)" not in cleanup
assert "replacement daemon may already have atomically published a newer one" in source

pidfile_stop = function_body(source, "stop_inicd(void)")
assert 'char *services[] = { "inicd", NULL };' in pidfile_stop
assert 'const char *pidfiles[] = { INICD_PID_FILE, NULL };' in pidfile_stop
assert 'kill_services_with_pidfiles(services, pidfiles, "inicd", 0, 3, 1)' in pidfile_stop
assert "killall" not in pidfile_stop
assert 'int process_pidfile_write(const char *path, const char *process_name);' in pidfile_header
assert "return -errno;" in function_body(source, "inicd_alarmtimer(unsigned long sec)")

for token in (
    "volatile sig_atomic_t *pending_events",
    "INICD_EVENT_POLL", "INICD_EVENT_STOP",
    "inicd_record_signal", "inicd_take_pending_events",
    "inicd_event_should_stop",
):
    assert token in policy, token
for token in (
    "test_signal_mapping",
    "test_signal_coalescing_and_stop_precedence",
    "test_null_event_storage",
    "iNIC signal deferral policy: PASS",
):
    assert token in test, token

assert '#if defined (USE_RT3352_MII)' in rc
assert '{ "inicd",\t\tinicd_main' in rc
assert '#if defined (USE_RT3352_MII)' in rc_header
assert "int inicd_main(int argc, char *argv[]);" in rc_header
assert "int start_inicd(void);" in rc_header
assert "int stop_inicd(void);" in rc_header
assert "int check_inic_mii_rebooted(void);" in rc_header
assert "OBJS += inicd.o" in src_makefile
assert "$(LN) rc $(ROMFSDIR)/sbin/inicd" in package_makefile
assert "start_inicd()" in function_body(wifi, "start_wifi_ap_rt(int radio_on)")
stop_wifi = function_body(wifi, "stop_wifi_all_rt(void)")
stop_daemon = stop_wifi.index("if (stop_inicd() != 0)")
isolate_hardware = stop_wifi.index("phy_isolate_inic(1)")
assert stop_daemon < isolate_hardware
assert "return -1;" in stop_wifi[stop_daemon:isolate_hardware]
assert "iNIC health monitor did not stop" in stop_wifi[stop_daemon:isolate_hardware]
assert "int\ncheck_inic_mii_rebooted(void)" in wifi
inic_check = function_body(wifi, "check_inic_mii_rebooted(void)")
radio_read_call = 'api_result = nvram_get_checked("mlme_radio_rt", &radio_value);'
radio_parse_call = "radio_read_status = wireless_radio_nvram_flag_status("
radio_error_gate = "if (radio_read_status != 0)"
disabled_gate = "if (!radio_enabled)"
disable_call = 'status = doSystem("iwpriv %s set %s=%d", IFNAME_INIC_MAIN,'
disable_error = "if (status != 0)"
disable_return = "return wireless_radio_restart_status(0, status);"
mode_read_call = 'api_result = nvram_get_checked("rt_mode_x", &mode_value);'
mode_parse_call = "mode_read_status = wireless_radio_nvram_int_status("
mode_error_gate = "if (mode_read_status != 0)"
mode_gate = "if (rt_mode_x != 1 && rt_mode_x != 3)"
inic_compact = " ".join(inic_check.split())
assert radio_read_call in inic_compact
assert radio_parse_call in inic_compact
assert radio_error_gate in inic_compact
assert "return radio_read_status;" in inic_check
assert disabled_gate in inic_compact
assert disable_call in inic_compact
assert "get_mlme_radio_rt()" not in inic_check
assert "get_mode_radio_rt()" not in inic_check
radio_read_pos = inic_compact.index(radio_read_call)
radio_parse_pos = inic_compact.index(radio_parse_call)
radio_error_pos = inic_compact.index(radio_error_gate)
disabled_pos = inic_compact.index(disabled_gate)
disable_pos = inic_compact.index("status = doSystem(")
assert radio_read_pos < radio_parse_pos < radio_error_pos < disabled_pos < disable_pos
assert inic_compact.index("saved_errno = errno;", radio_read_pos) < radio_parse_pos
assert mode_read_call in inic_compact
assert mode_parse_call in inic_compact
assert mode_error_gate in inic_compact
assert "return mode_read_status;" in inic_check
mode_read_pos = inic_compact.index(mode_read_call)
mode_parse_pos = inic_compact.index(mode_parse_call)
mode_error_pos = inic_compact.index(mode_error_gate)
mode_gate_pos = inic_compact.index(mode_gate)
guest_state_pos = inic_compact.index("status = wireless_guest_interface_state_checked(")
guest_action_pos = inic_compact.index("status = wif_control(IFNAME_INIC_GUEST, 1)")
assert mode_read_pos < mode_parse_pos < mode_error_pos < mode_gate_pos < guest_state_pos < guest_action_pos
assert "saved_errno = errno;" in inic_check[mode_read_pos:mode_parse_pos]
guest_state_check = "status = wireless_guest_interface_state_checked(IFNAME_INIC_GUEST,"
guest_state_error = "if (status != 0)"
guest_gate = "if (!guest_is_up && is_guest_allowed_rt())"
assert guest_state_check in " ".join(inic_check.split())
assert guest_state_error in inic_check
assert "iNIC guest AP state is unknown; recovery deferred until the next health check" in inic_check
assert guest_gate in inic_check
assert "is_interface_up(IFNAME_INIC_GUEST)" not in inic_check
guest_state_call_pos = inic_check.index("wireless_guest_interface_state_checked(")
guest_state_error_pos = inic_check.index(guest_state_error, guest_state_call_pos)
guest_state_return_pos = inic_check.index("return status;", guest_state_error_pos)
guest_state_gate_pos = inic_check.index(guest_gate)
assert guest_state_call_pos < guest_state_error_pos < guest_state_return_pos < guest_state_gate_pos
assert "test_unknown_guest_link_state_still_requests_down" in read("trunk/user/rc/src/tests/wireless_guest_isolation_policy.c")
assert "wireless_guest_isolation_link_state(-1, EACCES, ENODEV," in read("trunk/user/rc/src/tests/wireless_guest_isolation_policy.c")
assert "status = wif_control(IFNAME_INIC_GUEST, 1);" in inic_check
assert "wireless_radio_status_success(status)" in inic_check
assert "wif_control_success(IFNAME_INIC_GUEST, 1)" not in inic_check
assert inic_check.index(guest_gate) < inic_check.index("status = wif_control(IFNAME_INIC_GUEST, 1);")
assert inic_check.index("wireless_radio_status_success(status)") < inic_check.rindex(disable_return)
assert "iNIC guest AP recovery deferred until the next health check (status=%d)" in inic_check
assert inic_check.rstrip().endswith("return 0;")
assert "wireless_radio_restart_status(0, 256)" in read("trunk/user/rc/src/tests/wireless_radio_policy.c")

paths = (
    ".github/workflows/inicd-signal-stability.yml",
    "scripts/check_inicd_signals.py",
    "trunk/user/rc/src/inicd.c",
    "trunk/user/rc/src/inicd_signal_policy.h",
    "trunk/user/rc/src/tests/inicd_signal_policy.c",
    "trunk/user/rc/src/net_wifi.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/Makefile",
    "trunk/user/rc/Makefile",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("iNIC signal deferral and daemon lifecycle source contracts: PASS")
