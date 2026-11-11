#!/usr/bin/env python3
"""Source, lifecycle and integration contracts for networkmap signal handling."""
from pathlib import Path
import re

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


source = read("trunk/user/networkmap/src/networkmap.c")
policy = read("trunk/user/networkmap/src/networkmap_signal_policy.h")
test = read("trunk/user/networkmap/src/tests/networkmap_signal_policy.c")
shared = read("trunk/user/networkmap/src/function.c")
src_makefile = read("trunk/user/networkmap/src/Makefile")
package_makefile = read("trunk/user/networkmap/Makefile")
user_makefile = read("trunk/user/Makefile")
services = read("trunk/user/rc/src/services.c")
rc = read("trunk/user/rc/src/rc.c")
rc_header = read("trunk/user/rc/src/rc.h")
workflow = read(".github/workflows/networkmap-signal-stability.yml")

for token in (
    "#define networkmap_signal_record", "volatile sig_atomic_t",
    "SIGUSR1", "SIGTERM", "*(refresh_state) = 1", "*(exit_state) = 1",
    "SIG_BLOCK", "SIG_SETMASK", "*refresh_requested = 0",
):
    assert token in policy, token

for signature in ("sig_refresh(int sig)", "sig_exit(int sig)"):
    body = function_body(source, signature)
    compact = re.sub(r"\s+", "", body)
    assert compact == (
        "networkmap_signal_record(&networkmap_refresh_requested,&daemon_exit,sig);"
    ), signature + " must only record a signal-safe event"

assert "static volatile sig_atomic_t daemon_exit = 0;" in source
assert "static volatile sig_atomic_t networkmap_refresh_requested = 0;" in source
assert "unsigned int scan_now = 0;" in source
assert "static int networkmap_fullscan = 1;" in source
assert "volatile unsigned int scan_now" not in source
assert "static volatile int networkmap_fullscan" not in source

take = function_body(policy, "networkmap_signal_take_refresh(")
block = take.index("sigprocmask(SIG_BLOCK")
snapshot = take.index("requested = *refresh_requested")
clear = take.index("*refresh_requested = 0")
restore = take.index("sigprocmask(SIG_SETMASK")
assert block < snapshot < clear < restore
assert "if (!refresh_requested)" in take
assert "errno = EINVAL" in take
assert "if (sigprocmask(SIG_SETMASK" in take
assert "*refresh_requested = 1;" in take[restore:]

main = function_body(source, "main(int argc, char *argv[])")
loop = main.index("while (!daemon_exit)")
consume = main.index("networkmap_signal_take_refresh(&networkmap_refresh_requested)", loop)
mask_failure = main.index("if (refresh < 0)", consume)
refresh_gate = main.index("if (refresh)", mask_failure)
fullscan = main.index("networkmap_fullscan = 1;", refresh_gate)
scan_reset = main.index("scan_now = 0;", fullscan)
iterate = main.index("nmap_iterate();", scan_reset)
cleanup = main.index("clear_resources();", iterate)
assert loop < consume < mask_failure < refresh_gate < fullscan < scan_reset < iterate < cleanup
assert "exit_status = 1;" in main
assert "return exit_status;" in main
for signature in ("sig_refresh(int sig)", "sig_exit(int sig)"):
    handler = function_body(source, signature)
    assert "clear_resources();" not in handler and "exit(" not in handler

receive = function_body(source, "nmap_receive_arp(void)")
assert "while (!daemon_exit)" in receive
assert "recvfrom(arp_sockfd" in receive
init = function_body(source, "nmap_init(void)")
assert "struct timeval arp_timeout = {2, 0};" in init
assert "SO_RCVTIMEO" in init
assert "struct timeval arp_timeout = {2, 0};" in source[source.index("nmap_iterate(void)"):]

assert "extern unsigned int scan_now;" in shared
assert "OBJS = function.o networkmap.o" in src_makefile
assert "$(eval $(call BuildPackage,networkmap))" in package_makefile
assert "$(INSTALL_BIN) $(PKG_BUILD_DIR)/networkmap $(ROMFSDIR)/usr/sbin/networkmap" in package_makefile
assert "+= networkmap" in user_makefile

start = function_body(services, "start_networkmap(int first_call)")
stop = function_body(services, "stop_networkmap(void)")
restart = function_body(services, "restart_networkmap(void)")
assert 'eval("/usr/sbin/networkmap"' in start
assert '"networkmap"' in stop and "kill_services(" in stop
assert '"-SIGUSR1", "networkmap"' in restart
assert "start_networkmap(0);" in restart
assert "start_networkmap(1);" in services
assert "stop_networkmap();" in services
assert "restart_networkmap();" in rc
assert "int start_networkmap(int first_call);" in rc_header
assert "void stop_networkmap(void);" in rc_header
assert "void restart_networkmap(void);" in rc_header

paths = (
    ".github/workflows/networkmap-signal-stability.yml",
    "scripts/check_networkmap_signals.py",
    "trunk/user/networkmap/src/networkmap.c",
    "trunk/user/networkmap/src/networkmap_signal_policy.h",
    "trunk/user/networkmap/src/tests/networkmap_signal_policy.c",
    "trunk/user/networkmap/src/function.c",
    "trunk/user/networkmap/src/networkmap.h",
    "trunk/user/networkmap/src/Makefile",
    "trunk/user/networkmap/Makefile",
    "trunk/user/Makefile",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/Makefile",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

for expected in (
    "SIGUSR1 requests refresh only",
    "repeated refresh requests coalesce",
    "SIGTERM requests shutdown",
    "unmanaged signals are ignored",
    "pending SIGUSR1",
    "networkmap signal deferral policy: PASS",
):
    assert expected in test, expected

print("networkmap signal deferral and lifecycle source contracts: PASS")
