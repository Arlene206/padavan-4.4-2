#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
relay = (BASE / "dhcprelay.c").read_text()
policy = (BASE / "udhcp_relay_bootrequest_policy.h").read_text()
test = (BASE / "tests/dhcprelay_bootrequest_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-relay-bootrequest-policy.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

client_start = relay.index("/* clients */")
opcode = relay.index("if (!udhcp_relay_is_bootrequest(dhcp_msg.op))", client_start)
hop = relay.index("udhcp_relay_increment_hops(&dhcp_msg.hops)", client_start)
giaddr = relay.index("if (dhcp_msg.gateway_nip == 0)", hop)
forward = relay.index("pass_to_server(&dhcp_msg", giaddr)
legacy_unconditional = relay.find(
    "udhcp_read_interface(iface_list[i], NULL, &dhcp_msg.gateway_nip", client_start
)

assert '"udhcp_relay_bootrequest_policy.h"' in relay
assert opcode < hop < giaddr < forward
assert "if (!udhcp_relay_is_bootrequest(dhcp_msg.op))" in relay[client_start:hop]
assert "log1(\\"Ignoring non-BOOTREQUEST packet on relay client socket\\")" in relay[opcode:hop]
assert "#define UDHCP_RELAY_BOOTREQUEST_OPCODE 1U" in policy
assert "return opcode == UDHCP_RELAY_BOOTREQUEST_OPCODE" in policy
assert "test_bootrequest_opcode" in test
assert legacy_unconditional == -1
assert "if (!udhcp_relay_increment_hops(&dhcp_msg.hops))" in relay[hop - 100:hop + 180]
assert "log1(" in relay[hop:giaddr]
assert "udhcp_relay_select_giaddr(" in relay[giaddr:forward]
assert "#define UDHCP_RELAY_MAX_HOPS 16U" in policy
assert "if (existing_giaddr != 0U)" in policy
assert "if (interface_addr != 0U)" in policy
assert "*hops > UDHCP_RELAY_MAX_HOPS" in policy
for token in (
    "0x01020304U",
    "UDHCP_RELAY_MAX_HOPS - 1U",
    "hops == UDHCP_RELAY_MAX_HOPS + 1U",
    "(UDHCP_RELAY_MAX_HOPS + 1U)",
    "hops == 255U",
    "udhcp_relay_increment_hops(NULL)",
):
    assert token in test, token

expected = (
    ".github/workflows/udhcp-relay-bootrequest-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcprelay.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_relay_bootrequest_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/dhcprelay_bootrequest_policy.c",
    "scripts/check_udhcp_relay_bootrequest_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing/duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing/duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate production path", path)

print("DHCP relay giaddr/hop integration contract: PASS")
