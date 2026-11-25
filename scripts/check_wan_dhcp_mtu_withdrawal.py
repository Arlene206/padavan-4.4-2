#!/usr/bin/env python3
"""Pin DHCP option-26 withdrawal to the default MTU for WAN and IPTV."""

from pathlib import Path

source = Path("trunk/user/rc/src/net_wan.c").read_text()
policy = Path("trunk/user/rc/src/dhcp_lease_policy.h").read_text()
test_source = Path("trunk/user/rc/src/tests/dhcp_lease_policy.c").read_text()


def function_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


assert "#define DHCP_LEASE_DEFAULT_MTU 1500" in policy
assert "if (!option)" in policy
assert "*mtu = DHCP_LEASE_DEFAULT_MTU;" in policy
assert "dhcp_lease_decimal_is_valid(option, 576U, 1500U" in policy
assert "*supported = 0;" in policy
assert "previous_mtu = DHCP_LEASE_DEFAULT_MTU;" in policy
assert "*should_reapply = previous_mtu != next_mtu;" in policy

physical_bound = function_body(
    source, "udhcpc_bound(char *wan_ifname, int is_renew_mode)"
)
physical_renew = function_body(source, "udhcpc_renew(char *wan_ifname)")
iptv_bound = function_body(
    source, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)"
)
iptv_renew = function_body(source, "udhcpc_viptv_renew(char *man_ifname)")

physical_mtu_start = physical_bound.index('value = getenv("mtu");')
physical_route_snapshot = physical_bound.index(
    "bound_names[WAN_DHCP_BOUND_ROUTES],", physical_mtu_start
)
physical_mtu = physical_bound[physical_mtu_start:physical_route_snapshot]
assert "dhcp_lease_mtu_option_value(" in physical_mtu
assert "bound_names[WAN_DHCP_BOUND_MTU]" in physical_mtu
assert "if (value) {" not in physical_mtu
assert "wan_dhcp_mtu_is_supported(dhcp_mtu)" in physical_bound
assert "set_interface_mtu(wan_ifname, dhcp_mtu)" in physical_bound

iptv_mtu = iptv_bound[
    iptv_bound.index('value = getenv("mtu");'):
    iptv_bound.index("lan_config_status = rc_wan_nvram_copy_snapshot(")
]
assert "mtu_seen = 1;" in iptv_mtu
assert "dhcp_lease_mtu_option_value(" in iptv_mtu
assert "if (mtu_seen)" in iptv_bound
assert "wan_dhcp_viptv_snapshot_params[6]" in iptv_bound
assert "wan_dhcp_mtu_is_supported(dhcp_mtu)" in iptv_bound
assert "set_interface_mtu(man_ifname, dhcp_mtu)" in iptv_bound

for callback, bound_call in (
    (physical_renew, "return udhcpc_bound(wan_ifname, 1);"),
    (iptv_renew, "return udhcpc_viptv_bound(man_ifname, 1);"),
):
    parse = callback.index("dhcp_lease_mtu_option_value(")
    compare = callback.index("dhcp_lease_mtu_should_reapply(")
    changed = callback.index("if (mtu_changed)", compare)
    rebind = callback.index(bound_call, changed)
    assert parse < compare < changed < rebind
    assert "previous_state.mtu[0]" in callback[compare:changed]
    assert "DHCP_LEASE_DEFAULT_MTU" in callback

assert "test_dhcp_lease_mtu_option_withdrawal_restores_default" in test_source
assert "dhcp_lease_mtu_option_value(NULL" in test_source
assert "dhcp_lease_mtu_should_reapply(1400, 1, 1500" in test_source

print("DHCP option-26 withdrawal restores default MTU for WAN and IPTV: PASS")
