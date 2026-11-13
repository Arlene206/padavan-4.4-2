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
assert "return stop_status == 0 && process_present == 0;" in policy

stop = function_body(services, "stop_infosvr(void)")
assert "return kill_services(svcs, 3, 1);" in stop

restart = function_body(services, "restart_infosvr(void)")
stop_call = restart.index("stop_status = stop_infosvr();")
presence_check = restart.index('process_present = pids("infosvr");')
gate = restart.index("infosvr_restart_can_start(stop_status, process_present)")
failure = restart.index("return (stop_status != 0) ? stop_status : EBUSY;")
launch = restart.index("return start_infosvr();")
assert stop_call < presence_check < gate < failure < launch
assert "refusing infosvr restart: previous daemon did not stop" in restart

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
    "test_confirmed_stop_allows_start",
    "infosvr stop-before-start daemon policy: PASS",
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
