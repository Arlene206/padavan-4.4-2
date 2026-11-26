#!/usr/bin/env python3
from pathlib import Path

root = Path(".")
base = root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (base / "dhcprelay.c").read_text()
policy = (base / "udhcp_relay_client_dest_policy.h").read_text()
test = (base / "tests/dhcprelay_client_dest_policy.c").read_text()
workflow = (root / ".github/workflows/udhcp-relay-client-dest-policy.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


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
assert '"udhcp_relay_client_dest_policy.h"' in relay
dest = reply.index("udhcp_relay_client_destination(")
send = reply.index("sendto_ip4(fds[item->client], p, packet_len, &client_dest)")
assert dest < send
assert "client_dest.sin_port = htons(CLIENT_PORT)" in reply
assert "item->ip.sin_addr.s_addr, p->ciaddr, p->yiaddr" in reply
assert "ntohs(p->flags) & 0x8000" in reply
assert "(DHCPNAK == type)" in reply
assert "if (broadcast || nak)" in policy
assert "if (ciaddr != 0U)" in policy
assert "if (yiaddr != 0U)" in policy
assert "if (source_addr != 0U)" in policy
for token in ("== bcast", "== ciaddr", "== yiaddr", "== source"):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-client-dest-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_client_dest_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_client_dest_policy.c",
    "scripts/check_udhcp_relay_client_dest_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for item in expected:
    quoted = '"' + item + '"'
    assert push.count(quoted) == 1 and pr.count(quoted) == 1, item
for item in expected[1:4]:
    assert xy.count('"' + item + '"') == 1, ("XY-C1 missing/duplicate path", item)

print("DHCP relay client destination integration contract: PASS")
