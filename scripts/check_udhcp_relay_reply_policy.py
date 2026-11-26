#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_reply_policy.h").read_text()
test = (BASE / "tests/udhcp_relay_reply_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-reply-policy.yml").read_text()
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


reply = body(relay, "pass_to_client(struct dhcp_packet *p")
opcode_guard = reply.index("udhcp_relay_is_bootreply(p->op)")
message_type = reply.index("get_dhcp_packet_type(p)")
transaction_lookup = reply.index("xid_find(p)")
assert opcode_guard < message_type < transaction_lookup
assert "Ignoring non-BOOTREPLY packet" in reply
assert "return opcode == UDHCP_RELAY_BOOTREPLY_OPCODE" in policy
assert "#define UDHCP_RELAY_BOOTREPLY_OPCODE 2U" in policy
assert '"udhcp_relay_reply_policy.h"' in relay

for token in ("udhcp_relay_is_bootreply(2U)",
              "udhcp_relay_is_bootreply(1U)",
              "udhcp_relay_is_bootreply(255U)"):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-reply-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_reply_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcp_relay_reply_policy.c",
    "scripts/check_udhcp_relay_reply_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing/duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing/duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate path", path)

print("DHCP relay BOOTREPLY source integration contract: PASS")
