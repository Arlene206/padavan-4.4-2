#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
dhcpd = (BASE / "dhcpd.c").read_text()
policy = (BASE / "udhcp_server_message_policy.h").read_text()
test = (BASE / "tests/udhcp_server_message_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-server-message-policy.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

decline_start = dhcpd.index("case DHCPDECLINE:")
release_start = dhcpd.index("case DHCPRELEASE:", decline_start)
inform_start = dhcpd.index("case DHCPINFORM:", release_start)
decline = dhcpd[decline_start:release_start]
release = dhcpd[release_start:inform_start]

decline_guard = decline.index("udhcp_server_decline_ciaddr_valid(packet.ciaddr)")
decline_mutation = decline.index("memset(lease->lease_mac")
assert decline_guard < decline_mutation
assert "server_id_opt" in decline and "requested_ip_opt" in decline

release_guard = release.index(
    "udhcp_server_release_has_no_requested_ip(requested_ip_opt != NULL)"
)
release_mutation = release.index("lease->expires = time(NULL)")
assert release_guard < release_mutation
assert "packet.ciaddr == lease->lease_nip" in release
assert "server_id_opt" in release

assert "return ciaddr == 0U" in policy
assert "return requested_ip_present == 0" in policy
for token in (
    "test_decline_requires_zero_ciaddr",
    "test_release_rejects_requested_ip_option",
    "0xffffffffU",
):
    assert token in test

expected = (
    ".github/workflows/udhcp-server-message-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpd.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_server_message_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcp_server_message_policy.c",
    "scripts/check_udhcp_server_message_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate source path", path)

print("DHCP server DECLINE/RELEASE integration contract: PASS")
