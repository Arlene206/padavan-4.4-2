#!/usr/bin/env python3
"""Verify USB modem WAN launch failures reach boot and RC action results."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
USB_SOURCE = (RC / "usb_modem.c").read_text()
WAN_SOURCE = (RC / "net_wan.c").read_text()
RC_SOURCE = (RC / "rc.c").read_text()
HEADER = (RC / "rc.h").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


def compact(source: str) -> str:
    return re.sub(r"\s+", " ", source).strip()


ndis = compact(body(USB_SOURCE, "launch_wan_usbnet(int unit)"))
migration = "migration_status = check_upnp_wanif_changed(ndis_ifname);"
assert migration in ndis
assert ndis.index(migration) < ndis.index('set_wan_unit_value(unit, "proto_t"')
assert "if (migration_status != 0)" in ndis
migration_failure = ndis.index("return modem_wan_start_status(migration_status, 0);")
dhcp_launch = "dhcp_status = start_udhcpc_wan(ndis_ifname, unit, 0);"
assert migration_failure < ndis.index(dhcp_launch)
assert "return modem_wan_start_status(migration_status, dhcp_status);" in ndis
assert 'set_wan_unit_value_int(unit, "err", dhcp_status != 0);' in ndis

wan_start = compact(body(WAN_SOURCE, "start_wan(void)"))
assert "int modem_start_status = 0;" in wan_start
assert "vlan_result = launch_wan_usbnet(unit);" in wan_start
assert "vlan_result = launch_wan_modem_ras(unit);" in wan_start
aggregate_pattern = r"modem_start_status\s*=\s*rc_first_service_start_status\s*\(\s*modem_start_status\s*,\s*vlan_result\s*\)\s*;"
aggregate_matches = list(re.finditer(aggregate_pattern, wan_start))
assert len(aggregate_matches) == 3
ras_migration = "vlan_result = check_upnp_wanif_changed(ppp_ifname);"
assert ras_migration in wan_start
ras_migration_start = wan_start.index(ras_migration)
assert re.search(aggregate_pattern, wan_start[ras_migration_start:]), "RAS migration failure must be retained"
ras_launch_start = wan_start.index("char *ppp_ifname = IFNAME_RAS;")
ras_launch_end = wan_start.index("vlan_result = launch_wan_modem_ras(unit);", ras_launch_start)
ras_branch = wan_start[ras_launch_start:ras_launch_end]
ras_wanx_status = "modem_start_status = rc_first_service_start_status(modem_start_status, wanx_status);"
assert ras_wanx_status in ras_branch, "RAS physical WAN setup failure must reach modem startup status"
assert ras_branch.index("if (wanx_status != 0)") < ras_branch.index(ras_wanx_status)
eapol_start = wan_start.index("if (auth_status != 0)")
eapol_end = wan_start.index("/* We are done configuration */", eapol_start)
assert "modem_start_status" not in wan_start[eapol_start:eapol_end]
assert wan_start.index("set_passthrough_pppoe(1)") < wan_start.index("return modem_start_status;")

for signature in (
    "full_restart_wan(void)",
    "try_wan_reconnect(int try_use_modem, long pause_in_seconds)",
    "deferred_wan_connect(void)",
):
    function = compact(body(WAN_SOURCE, signature))
    assert "status = start_wan();" in function
    assert "return status;" in function

for signature, call in (
    ("manual_wan_reconnect(void)", "try_wan_reconnect(1, 0)"),
    ("auto_wan_reconnect(void)", "try_wan_reconnect(1, 0)"),
    ("auto_wan_reconnect_pause(void)", 'try_wan_reconnect(1, nvram_get_int("di_recon_pause"))'),
):
    assert f"return {call};" in compact(body(WAN_SOURCE, signature))

init = compact(body(RC_SOURCE, "init_router(void)"))
assert "wan_start_status = start_wan();" in init
assert re.search(
    r"boot_services_status\s*=\s*rc_first_service_start_status\s*\(\s*boot_services_status\s*,\s*wan_start_status\s*\)\s*;",
    init,
)

for call in (
    "full_restart_wan()",
    "deferred_wan_connect()",
    "auto_wan_reconnect()",
    "auto_wan_reconnect_pause()",
    "manual_wan_reconnect()",
):
    notify_pattern = (
        r"rc_notify_merge_action_result\s*\(\s*action_result\s*,\s*"
        + re.escape(call)
        + r"\s*\)"
    )
    assert re.search(notify_pattern, RC_SOURCE), f"RC notification must preserve {call} status"

for declaration in (
    "int start_wan(void);",
    "int full_restart_wan(void);",
    "int try_wan_reconnect(int try_use_modem, long pause_in_seconds);",
    "int deferred_wan_connect(void);",
    "int auto_wan_reconnect(void);",
    "int auto_wan_reconnect_pause(void);",
    "int manual_wan_reconnect(void);",
):
    assert declaration in HEADER

print("USB modem WAN startup status propagation: PASS")
