#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_packet_bounds.h").read_text()
test = (BASE / "tests/dhcprelay_packet_bounds.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-packet-bounds.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

server_start = relay.index("/* server */")
client_start = relay.index("/* clients */", server_start)
server_loop = relay[server_start:client_start]
server_memset = server_loop.index("memset(&dhcp_msg, 0, sizeof(dhcp_msg))")
server_receive = server_loop.index("packlen = recvfrom(")
server_length_check = server_loop.index("udhcp_relay_packet_length_valid(")
server_cookie_check = server_loop.index("dhcp_msg.cookie == htonl(DHCP_MAGIC)")
server_pass_reply = server_loop.index("pass_to_client(&dhcp_msg")

assert server_memset < server_receive < server_length_check < server_cookie_check < server_pass_reply
assert "sizeof(dhcp_msg), MSG_TRUNC" in server_loop
assert "offsetof(struct dhcp_packet, options) + 4U" in server_loop
assert "udhcp_recv_kernel_packet" not in server_loop

start = relay.index("/* clients */")
end = relay.index("\n\t\t\t}", start)
client_loop = relay[start:end]
memset = client_loop.index("memset(&dhcp_msg, 0, sizeof(dhcp_msg))")
receive = client_loop.index("packlen = recvfrom(")
length_check = client_loop.index("udhcp_relay_packet_length_valid(")
pass_request = client_loop.index("pass_to_server(&dhcp_msg")

assert memset < receive < length_check < pass_request
assert "sizeof(dhcp_msg), MSG_TRUNC" in client_loop
assert "offsetof(struct dhcp_packet, options) + 4U" in client_loop
assert "addr_size != sizeof(client_addr)" in client_loop
assert "client_addr.sin_family != AF_INET" in client_loop
assert "dhcp_msg.cookie != htonl(DHCP_MAGIC)" in client_loop
assert '"udhcp_relay_packet_bounds.h"' in relay

# A valid opcode is not enough: message type TLV length must be exactly one.
packet_type_start = relay.index("static int get_dhcp_packet_type(")
packet_type_end = relay.index("\n/**", packet_type_start)
packet_type_fn = relay[packet_type_start:packet_type_end]
packet_type_compact = "".join(packet_type_fn.split())
type_lookup = packet_type_compact.index(
    "udhcp_get_option_with_length(p,DHCP_MESSAGE_TYPE,&op,&option_length)"
)
type_gate = packet_type_compact.index(
    "udhcp_relay_message_type_length_valid(option_status,option_length)",
    type_lookup,
)
type_value = packet_type_compact.index("returnop[0];", type_gate)
assert type_lookup < type_gate < type_value
assert "udhcp_get_option(p,DHCP_MESSAGE_TYPE)" not in packet_type_compact
assert "test_message_type_option_requires_exactly_one_byte" in test
assert "udhcp_relay_message_type_length_valid(1, 255U)" in test

assert "received_length < 0" in policy
assert "(size_t)received_length > packet_capacity" in policy
assert "(size_t)received_length < minimum_length" in policy
for token in (
    "udhcp_relay_packet_length_valid(-1",
    "(int)minimum - 1",
    "(int)capacity + 1",
    "capacity + 1U",
):
    assert token in test, token

expected_paths = (
    ".github/workflows/udhcp-relay-packet-bounds.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_packet_bounds.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_packet_bounds.c",
    "scripts/check_udhcp_relay_packet_bounds.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected_paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted) == 1, ("pull_request path missing/duplicated", path)
    assert xy_workflow.count(quoted if path.startswith("trunk/") else "NOT_A_XY_SOURCE") == (1 if path.startswith("trunk/") else 0)
assert '"trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c"' in xy_workflow
assert '"trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_packet_bounds.h"' in xy_workflow
assert '"trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_packet_bounds.c"' in xy_workflow

print("DHCP relay inbound packet bounds integration contract: PASS")
