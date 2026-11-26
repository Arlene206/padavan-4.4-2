#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_select_error_policy.h").read_text()
test = (BASE / "tests/dhcprelay_select_error_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-select-error-policy.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


loop = body(relay, "while (1) {")
assert '"udhcp_relay_select_error_policy.h"' in relay
assert "select_result = select(max_socket + 1" in loop
assert "if (select_result < 0)" in loop
assert "int select_errno = errno" in loop
assert "udhcp_relay_select_was_interrupted(select_errno)" in loop
assert "errno = select_errno" in loop
assert 'bb_perror_msg_and_die("select")' in loop
assert loop.index("if (select_result < 0)") < loop.index("if (select_result > 0)")
assert "return error_number == EINTR" in policy

for token in ("EINTR", "EBADF", "EINVAL", "ENOMEM"):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-select-error-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_select_error_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_select_error_policy.c",
    "scripts/check_udhcp_relay_select_error_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing/duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing/duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate path", path)

print("DHCP relay select error handling integration contract: PASS")
