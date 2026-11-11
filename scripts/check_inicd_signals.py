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
take = main.index("inicd_take_pending_events(&pending_events)")
pending = main.index("sigpending(&pending_signals)")
pending_stop = main.index("sigismember(&pending_signals, SIGTERM)")
stop = main.index("inicd_event_should_stop(events)")
poll = main.index("if (events & INICD_EVENT_POLL)")
callback = main.index("check_inic_mii_rebooted();")
assert take < pending < pending_stop < stop < poll < callback
assert "events == 0" in main
assert main.index("inicd_cleanup();", main.index("for (;;)")) > callback
assert "if (timer_status < 0)" in main

cleanup = function_body(source, "inicd_cleanup(void)")
assert "inicd_alarmtimer(0)" in cleanup
assert "remove(INICD_PID_FILE)" in cleanup
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
assert "void check_inic_mii_rebooted(void);" in rc_header
assert "OBJS += inicd.o" in src_makefile
assert "$(LN) rc $(ROMFSDIR)/sbin/inicd" in package_makefile
assert "start_inicd()" in function_body(wifi, "start_wifi_ap_rt(int radio_on)")
assert "stop_inicd();" in function_body(wifi, "stop_wifi_all_rt(void)")
assert "check_inic_mii_rebooted(void)" in wifi

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
