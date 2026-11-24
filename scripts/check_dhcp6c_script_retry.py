#!/usr/bin/env python3
"""Verify DHCPv6 Reply script failures retain and retry their event."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

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
                state = "block"
                pos += 1
            elif ch == "/" and nxt == "/":
                state = "line"
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
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif state == "block" and ch == "*" and nxt == "/":
            state = "code"
            pos += 1
        elif state == "line" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)

base = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"
dhcp6c = (ROOT / (base + "dhcp6c.c")).read_text()
script_runner = (ROOT / (base + "dhcp6c_script.c")).read_text()
policy = (ROOT / (base + "dhcp6c_script_retry_policy.h")).read_text()
test = (ROOT / (base + "tests/dhcp6c_script_retry_policy.c")).read_text()
workflow = (ROOT / ".github/workflows/dhcp6c-script-retry-stability.yml").read_text()

assert '#include "dhcp6c_script_retry_policy.h"' in dhcp6c
runner = function_body(script_runner, "int dhcp6_script(const char *scriptpath, char **envp)")
assert "status = spawn_and_wait(argv);" in runner
assert "return status;" in runner
assert "free(envp);" in runner

reply = function_body(dhcp6c, "static int client6_recvreply(struct dhcp6_if *ifp, struct dhcp6_event *ev,")
assert "int state, script_status = 0;" in reply
script_call = reply.index("script_status = dhcp6_script(")
retry_gate = reply.index("if (dhcp6c_script_retry_required(script_status))")
assert script_call < retry_gate
retry_budget = reply.index("ev->timeouts = 0;", retry_gate)
retry_params = reply.index("dhcp6_set_timeoparam(ev);", retry_budget)
retry_timer = reply.index("dhcp6_timer_set(DHCP6C_SCRIPT_RETRY_DELAY_MS, ev->timer);", retry_params)
retry_return = reply.index("return -1;", retry_timer)
refresh_timer = reply.index("if (state == DHCP6S_INFOREQ)", retry_return)
remove_event = reply.index("dhcp6_remove_event(ev);", refresh_timer)
assert script_call < retry_gate < retry_budget < retry_params < retry_timer < retry_return < refresh_timer < remove_event
assert "if (script_status < 0)" in reply[retry_gate:retry_return]
assert "retrying event" in reply[retry_gate:retry_return]
assert "DHCPv6 configuration script exited with status %d; retrying event" in reply[retry_gate:retry_return]
assert "#define DHCP6C_SCRIPT_RETRY_DELAY_MS 5000ULL" in policy
assert "return script_status != 0;" in policy
assert "DHCP6C_SCRIPT_RETRY_DELAY_MS == 5000ULL" in test

for marker in (
    "test_success_does_not_retry",
    "test_positive_exit_status_retries",
    "test_spawn_failure_retries",
    "test_retry_delay_is_bounded_and_nonzero",
    "DHCPv6 configuration-script retry policy: PASS",
):
    assert marker in test, marker

workflow_push, pr_part = workflow.split("  pull_request:", 1)
workflow_pr = pr_part.split("  workflow_dispatch:", 1)[0]
paths = (
    ".github/workflows/dhcp6c-script-retry-stability.yml",
    "scripts/check_dhcp6c_script_retry.py",
    base + "dhcp6c.c",
    base + "dhcp6c_script.c",
    base + "dhcp6c_common.c",
    base + "dhcp6c_script_retry_policy.h",
    base + "tests/dhcp6c_script_retry_policy.c",
)
for path in paths:
    quoted = '"' + path + '"'
    assert workflow_push.count(quoted) == 1, "push path: " + path
    assert workflow_pr.count(quoted) == 1, "pull_request path: " + path

print("DHCPv6 Reply callback failure and event retry source contract: PASS")
