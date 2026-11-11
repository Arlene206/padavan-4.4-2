#!/usr/bin/env python3
"""Verify udhcpc event-script status propagation and lease-state gating."""
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
        next_ch = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and next_ch == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and next_ch == "/":
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
            if ch == "*" and next_ch == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


udhcp_dir = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"
dhcpc = read(udhcp_dir + "dhcpc.c")
policy = read(udhcp_dir + "udhcpc_script_policy.h")
test = read(udhcp_dir + "tests/udhcpc_script_policy.c")
spawn = read("trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c")
libbb = read("trunk/user/busybox/busybox-1.24.x/include/libbb.h")
workflow = read(".github/workflows/udhcpc-script-status-stability.yml")

assert '#include "udhcpc_script_policy.h"' in dhcpc
runner = function_body(dhcpc, "static int udhcp_run_script")
assert "int status;" in runner
assert "status = spawn_and_wait(argv);" in runner
assert "if (status != 0)" in runner
assert "DHCP event script %s failed with status %d" in runner
assert "return status;" in runner
assert "static void udhcp_run_script" not in dhcpc

spawn_body = function_body(spawn, "int FAST_FUNC spawn_and_wait(char **argv)")
assert "return wait4pid(rc);" in spawn_body
assert "int spawn_and_wait(char **argv) FAST_FUNC;" in libbb
assert "Returns sig + 0x180 if child is killed by signal." in libbb

ack_start = dhcpc.index("if (*message == DHCPACK)")
nak_start = dhcpc.index("if (*message == DHCPNAK)", ack_start)
ack = dhcpc[ack_start:nak_start]
script_call = ack.index("script_status = udhcp_run_script")
failure_gate = ack.index("if (script_status != 0)", script_call)
retry = ack.index("udhcp_script_failure_prepare_retry", failure_gate)
retry_continue = ack.index("continue;", retry)
bound_publish = ack.index("state = BOUND;", retry_continue)
assert script_call < failure_gate < retry < retry_continue < bound_publish
assert "script_status != 0" in ack[failure_gate:retry_continue]
assert "state == REQUESTING" in ack[retry:retry_continue]
assert "tryagain_timeout" in ack[retry:retry_continue]
assert "script_elapsed" in ack[retry:retry_continue]
assert ack.index("requested_ip = packet.yiaddr;", retry_continue) > retry_continue
assert "already_waited_sec = script_elapsed;" in ack
assert "DHCPACK for %s" in ack

assert "if (acquiring)" in policy
assert "*timeout = retry_timeout;" in policy
assert "*packet_num = 0;" in policy
assert "script_elapsed >= remaining" in policy
assert "*already_waited_sec += script_elapsed;" in policy

for marker in (
    "test_initial_acquisition_retries_selected_request",
    "test_failed_renewal_preserves_existing_lease_deadline",
    "test_script_runtime_cannot_extend_expired_lease",
    "test_exact_lease_deadline_is_expired",
    "test_already_expired_lease_is_not_resurrected",
    "udhcpc script failure retry policy: PASS",
):
    assert marker in test, marker

paths = (
    ".github/workflows/udhcpc-script-status-stability.yml",
    "scripts/check_udhcpc_script_status.py",
    udhcp_dir + "dhcpc.c",
    udhcp_dir + "udhcpc_script_policy.h",
    udhcp_dir + "tests/udhcpc_script_policy.c",
    "trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c",
    "trunk/user/busybox/busybox-1.24.x/include/libbb.h",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("udhcpc event-script status and lease-state source contracts: PASS")
