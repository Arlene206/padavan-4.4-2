#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_xid_policy.h").read_text()
test = (BASE / "tests/dhcprelay_xid_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-xid-policy.yml").read_text()
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


xid_add = body(relay, "xid_add(uint32_t xid")
xid_expire = body(relay, "xid_expire(void)")
xid_find = body(relay, "xid_find(const struct dhcp_packet *p)")
xid_del_item = body(relay, "xid_del_item(struct xid_item *target)")
pass_to_server = body(relay, "pass_to_server(struct dhcp_packet *p")
pass_to_client = body(relay, "pass_to_client(struct dhcp_packet *p")

assert "UDHCP_RELAY_MAX_PENDING_XIDS 1024U" in policy
assert "budget->pending >= UDHCP_RELAY_MAX_PENDING_XIDS" in policy
assert "++budget->pending" in policy
assert "--budget->pending" in policy
assert "udhcp_relay_xid_reserve(&xid_budget)" in xid_add
assert "udhcp_relay_xid_release(&xid_budget)" in xid_expire
assert "udhcp_relay_xid_release(&xid_budget)" in xid_del_item
assert "gateway_nip" in xid_add and "chaddr" in xid_add
assert "memcmp(item->chaddr, p->chaddr" in xid_add
assert "gateway_nip == p->gateway_nip" in xid_find
assert "memcmp(item->chaddr, p->chaddr" in xid_find
assert "xid_add(p->xid, p, client_addr, client, &created)" in pass_to_server
assert "if (NULL == item)" in pass_to_server
assert "!udhcp_relay_send_result_ok(sendto_ip4(fds[0], p, packet_len, server_addr)) && created" in pass_to_server
assert "xid_del_item(item)" in pass_to_server
assert "xid_find(p)" in pass_to_client
assert "udhcp_relay_send_result_ok(sendto_ip4(fds[item->client], p, packet_len, &item->ip))" in pass_to_client
assert "xid_del_item(item)" in pass_to_client
assert "Unable to forward DHCP reply; retaining transaction for retry" in pass_to_client
assert "xid_del(p->xid)" not in relay

for token in (
    "test_reservation_limit",
    "test_release_allows_next_transaction",
    "test_send_result_status",
    "UDHCP_RELAY_MAX_PENDING_XIDS",
    "budget.pending == 0U",
):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-xid-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_xid_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_xid_policy.c",
    "scripts/check_udhcp_relay_xid_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)

for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate production path", path)

print("DHCP relay bounded XID transaction integration contract: PASS")
