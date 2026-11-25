#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_select_fd_policy.h").read_text()
test = (BASE / "tests/dhcprelay_select_fd_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-select-fd-policy.yml").read_text()
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


init_sockets = body(relay, "init_sockets(char **iface_list")
assert '"udhcp_relay_select_fd_policy.h"' in relay
assert "fd >= 0 && fd_set_size > 0 && fd < fd_set_size" in policy
assert "udhcp_relay_fd_fits_select(fds[i], FD_SETSIZE)" in init_sockets
assert "for (j = 0; j <= i; j++)" in init_sockets
assert "close(fds[j])" in init_sockets
assert "bb_error_msg_and_die(" in init_sockets
assert init_sockets.index("udhcp_relay_fd_fits_select") < init_sockets.index("if (n < fds[i])")
assert "FD_SET(fds[i], &rfds)" in relay

for token in ("fd_set_size - 1", "fd_set_size, fd_set_size",
              "fd_set_size + 1", "udhcp_relay_fd_fits_select(-1"):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-select-fd-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_select_fd_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_select_fd_policy.c",
    "scripts/check_udhcp_relay_select_fd_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate path", path)

print("DHCP relay select descriptor integration contract: PASS")
