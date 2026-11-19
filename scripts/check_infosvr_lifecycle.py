#!/usr/bin/env python3
"""Verify RC infosvr daemon restart ordering and notification failure handling."""
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


services = read("trunk/user/rc/src/services.c")
policy = read("trunk/user/rc/src/infosvr_daemon_policy.h")
test = read("trunk/user/rc/src/tests/infosvr_daemon_policy.c")
header = read("trunk/user/rc/src/rc.h")
dispatcher = read("trunk/user/rc/src/rc.c")
workflow = read(".github/workflows/infosvr-packet-stability.yml")

assert '#include "infosvr_daemon_policy.h"' in services
assert "process_state == PROCESS_PRESENCE_ABSENT" in policy
assert "infosvr_start_postcondition" in policy
assert "INFOSVR_START_PROBE_ATTEMPTS 4" in policy

start = function_body(services, "start_infosvr(void)")
assert 'optional_daemon_start_status("infosvr", "infosvr")' in start
disabled = start.index('if (nvram_invmatch("adsc_enable", "1"))')
running = start.index("if (start_status > 0)")
unknown = start.index("if (start_status < 0)")
launch_status = start.index('start_status = eval("/usr/sbin/infosvr", IFNAME_BR);')
launch_log = start.index('logmessage("infosvr", "infosvr launch failed (status=%d)",')
launch_return = start.index("return infosvr_start_postcondition(start_status,", launch_log)
launch_failure = start.split("if (start_status != 0)", 1)[1].split("\n\t}", 1)[0]
assert disabled < running < unknown < launch_status < launch_log < launch_return
assert "return 0;" in start[disabled:launch_status]
assert "return 1;" not in start[disabled:launch_status]
assert "if (start_status != 0)" in start
assert "return infosvr_start_postcondition(start_status," in launch_failure
assert "return start_status;" not in launch_failure
postcondition = function_body(policy, "infosvr_start_postcondition(")
assert "if (launch_status != 0)" in postcondition
assert "return -1;" in postcondition
assert "return launch_status;" not in postcondition
assert "test_start_normalizes_launcher_failure_for_pid1" in test
assert 'service_start_probe_wait("infosvr", INFOSVR_START_INITIAL_WAIT_SECONDS)' in start
assert 'service_start_probe_wait("infosvr", INFOSVR_START_RETRY_WAIT_SECONDS)' in start
assert "for (attempt = 0; attempt < INFOSVR_START_PROBE_ATTEMPTS; attempt++)" in start
assert 'process_state = optional_daemon_presence("infosvr", &scan_error);' in start
assert "infosvr_start_postcondition(start_status, process_state)" in start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in start
assert "errno = scan_error > 0 ? scan_error : EIO;" in start
assert "errno = ETIMEDOUT;" in start
assert start.index('eval("/usr/sbin/infosvr", IFNAME_BR);') < start.index('service_start_probe_wait("infosvr", INFOSVR_START_INITIAL_WAIT_SECONDS)')
stop = function_body(services, "stop_infosvr(void)")
assert "return kill_services(svcs, 3, 1);" in stop

restart = function_body(services, "restart_infosvr(void)")
stop_call = restart.index("stop_status = stop_infosvr();")
presence_check = restart.index('process_state = process_inventory_scan("infosvr", &snapshot);')
gate = restart.index("infosvr_restart_can_start(stop_status, process_state)")
stop_error = restart.index("if (stop_status != 0)")
stop_return = restart.index("return stop_status;", stop_error)
busy_errno = restart.index("errno = EBUSY;", stop_return)
failure = restart.index("return -1;", busy_errno)
launch = restart.index("return start_infosvr();")
assert stop_call < presence_check < gate < stop_error < stop_return < busy_errno < failure < launch
assert "refusing infosvr restart: previous daemon absence is unconfirmed" in restart
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in restart
assert "scan_error > 0 ? scan_error : EIO" in restart
assert "process_snapshot_release(&snapshot);" in restart

assert "int stop_infosvr(void);" in header
assert "int restart_infosvr(void);" in header
event = dispatcher.split(
    "else if (strcmp(entry->d_name, RCN_RESTART_ADSC) == 0)", 1
)[1].split("else if", 1)[0]
assert "action_result = restart_infosvr();" in event
assert "if (action_result < 0)" in dispatcher
assert "completion marker retained" in dispatcher

for marker in (
    "test_failed_stop_never_starts_replacement",
    "test_remaining_process_blocks_replacement",
    "test_unknown_inventory_blocks_replacement",
    "test_confirmed_absence_allows_start",
    "test_start_requires_running_postcondition",
    "test_start_normalizes_launcher_failure_for_pid1",
    "infosvr tri-state stop-before-start and daemon-start postcondition policy: PASS",
):
    assert marker in test, marker

paths = (
    "scripts/check_infosvr_lifecycle.py",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/infosvr_daemon_policy.h",
    "trunk/user/rc/src/tests/infosvr_daemon_policy.c",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("RC infosvr stop-before-start and notification failure contracts: PASS")
