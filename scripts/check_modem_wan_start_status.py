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

vinet_setup = compact(body(WAN_SOURCE, "config_vinet_wan(void)"))
parent_ipv6_call = vinet_setup.index("ret = control_if_ipv6(ifname_wan_cpu, 0);")
parent_ipv6_guard = vinet_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, parent_ipv6_call)
parent_ipv6_end = vinet_setup.index("ret = create_vlan_iface(", parent_ipv6_call)
parent_ipv6 = vinet_setup[parent_ipv6_guard:parent_ipv6_end]
assert parent_ipv6_guard >= 0
assert re.search(r"ret\s*=\s*control_if_ipv6\(ifname_wan_cpu,\s*0\)\s*;", parent_ipv6)
assert re.search(r"if\s*\(ret\s*!=\s*0\)\s*\{[^}]*return\s+ret\s*;", parent_ipv6), (
    "WAN VLAN creation must stop when required IPv6 disable on the parent fails"
)

vinet_publish = vinet_setup.index('set_wan_unit_value(0, "ifname", vinet_ifname);')
vinet_enable_call = vinet_setup.index("ret = control_if_ipv6(vinet_ifname, 1);")
vinet_enable_guard = vinet_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, vinet_enable_call)
vinet_enable = vinet_setup[vinet_enable_guard:vinet_publish]
assert vinet_enable_guard >= 0
assert re.search(r"if\s*\(ret\s*!=\s*0\)\s*\{[^}]*return\s+ret\s*;", vinet_enable), (
    "WAN interface publication must wait for successful IPv6 enable"
)

wisp_setup = compact(body(WAN_SOURCE, "config_apcli_wisp(void)"))
wisp_publish = wisp_setup.index('set_wan_unit_value(0, "ifname", wisp_ifname);')
wisp_enable_call = wisp_setup.index("int ipv6_status = control_if_ipv6(wisp_ifname, 1);")
wisp_enable_guard = wisp_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, wisp_enable_call)
wisp_enable = wisp_setup[wisp_enable_guard:wisp_publish]
assert wisp_enable_guard >= 0
assert re.search(r"if\s*\(ipv6_status\s*!=\s*0\)\s*\{[^}]*return\s+ipv6_status\s*;", wisp_enable), (
    "AP-client WAN publication must wait for successful IPv6 enable"
)

wan_start = compact(body(WAN_SOURCE, "start_wan(void)"))
assert "int modem_start_status = 0;" in wan_start
assert "wan_up_status = wan_up(wan_ifname, unit, 1);" in wan_start
assert re.search(r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*wan_up_status\s*\)\s*;", wan_start)
interface_failure_start = wan_start.index("if (interface_status != 0)")
interface_failure_end = wan_start.index("/* perform ApCli reconnect */", interface_failure_start)
interface_failure = wan_start[interface_failure_start:interface_failure_end]
interface_error_record = re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*interface_status\s*\)\s*;",
    interface_failure,
)
assert interface_error_record, "physical WAN IFUP failure must reach start_wan() status"
assert interface_failure.index(interface_error_record.group(0)) < interface_failure.index(
    "interface_cleanup_status = ifconfig("
), "best-effort interface cleanup must not replace the original IFUP failure"
assert "continue;" in interface_failure
assert "return rc_first_service_start_status(modem_start_status, wan_start_status);" in wan_start
assert re.search(r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*ipv6_status\s*\)\s*;", wan_start)
assert "vlan_result = launch_wan_usbnet(unit);" in wan_start
assert "vlan_result = launch_wan_modem_ras(unit);" in wan_start

iptv_call = "vlan_result = launch_viptv_wan();"
assert iptv_call in wan_start
iptv_call_pos = wan_start.index(iptv_call)
iptv_error_pos = wan_start.index("if (vlan_result != 0)", iptv_call_pos)
iptv_log_pos = wan_start.index("IPTV VLAN or service setup incomplete", iptv_error_pos)
assert re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*vlan_result\s*\)\s*;",
    wan_start[iptv_error_pos:iptv_log_pos],
), "configured IPTV startup failure must be returned without aborting WAN core startup"

passthrough_call = "int passthrough_status = set_passthrough_pppoe(1);"
assert passthrough_call in wan_start
passthrough_pos = wan_start.index(passthrough_call)
passthrough_error_pos = wan_start.index("if (passthrough_status != 0)", passthrough_pos)
passthrough_status_pos = wan_start.index("wan_start_status = rc_first_service_start_status(", passthrough_error_pos)
passthrough_status_end = wan_start.index(";", passthrough_status_pos)
assert "wan_start_status, passthrough_status" in wan_start[passthrough_status_pos:passthrough_status_end]
assert passthrough_status_pos < wan_start.index("return rc_first_service_start_status(modem_start_status, wan_start_status);")

wait_ppp = compact(body(WAN_SOURCE, "wait_ppp_up(char *ppp_ifname, int unit)"))
assert "failure_status = (interface_status < 0 && errno != 0) ?" in wait_ppp
assert "return failure_status;" in wait_ppp
assert "socket_status = errno ? errno : EIO;" in wait_ppp
assert "return socket_status;" in wait_ppp
assert "return route_status ? route_status : EIO;" in wait_ppp
assert "return 0;" in wait_ppp[wait_ppp.index('doSystem("ping -c1 %s", ppp_gate);'):]
ppp_branch_start = wan_start.index("char *proto_desc, *ppp_ifname = IFNAME_PPP;")
ipoe_branch_start = wan_start.index("else if (wan_proto == IPV4_WAN_PROTO_IPOE_DHCP", ppp_branch_start)
ppp_branch = wan_start[ppp_branch_start:ipoe_branch_start]
for operation in (
    "vlan_result = check_upnp_wanif_changed(ppp_ifname);",
    "vlan_result = launch_wan_pppd(unit, wan_proto);",
    "vlan_result = wait_ppp_up(ppp_ifname, unit);",
):
    operation_pos = ppp_branch.index(operation)
    next_continue = ppp_branch.index("continue;", operation_pos)
    failure_path = ppp_branch[operation_pos:next_continue]
    assert "wan_start_status = rc_first_service_start_status" in failure_path, (
        operation + " failures must reach the WAN startup result"
    )
wanx_failure_pos = ppp_branch.index("if (!i_pppoe && wanx_status != 0)")
wanx_failure_end = ppp_branch.index("continue;", wanx_failure_pos)
assert "wan_start_status = rc_first_service_start_status" in ppp_branch[wanx_failure_pos:wanx_failure_end]

ipoe_end = wan_start.index("return rc_first_service_start_status(modem_start_status, wan_start_status);", ipoe_branch_start)
ipoe_branch = wan_start[ipoe_branch_start:ipoe_end]
migration_pos = ipoe_branch.index("vlan_result = check_upnp_wanif_changed(wan_ifname);")
migration_end = ipoe_branch.index("continue;", migration_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[migration_pos:migration_end]
static_config_start = ipoe_branch.index("if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)")
static_config_end = ipoe_branch.index('set_wan_unit_value_int(unit, "err", wan_err);', static_config_start)
static_config = compact(ipoe_branch[static_config_start:static_config_end])
invalid_config = "if (!dhcp_lease_ipv4_is_valid(wan_addr) || !dhcp_lease_netmask_is_valid(wan_mask))"
assert invalid_config in static_config, "static WAN IPoE must strictly validate address and netmask"
invalid_config_start = static_config.index(invalid_config)
subnet_conflict_start = static_config.index("else if (is_same_subnet2(wan_addr, lan_addr, wan_mask, lan_mask))")
assert invalid_config_start < subnet_conflict_start
invalid_config_path = static_config[invalid_config_start:subnet_conflict_start]
assert "wan_err = 1;" in invalid_config_path
assert re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*EINVAL\s*\)\s*;",
    invalid_config_path,
), "invalid static WAN config must not be reported as successful startup"
assert static_config.index("address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);") > subnet_conflict_start

address_pos = ipoe_branch.index("if (address_status != 0)")
address_end = ipoe_branch.index("/* re-build iptables rules (final stage for IPoE static) */", address_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[address_pos:address_end]
auth_pos = ipoe_branch.index("if (auth_status != 0)")
auth_end = ipoe_branch.index("/* We are done configuration */", auth_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[auth_pos:auth_end]
dhcp_pos = ipoe_branch.index("if (dhcp_status != 0)")
dhcp_end = ipoe_branch.index('set_wan_unit_value_int(unit, "err", wan_err);', dhcp_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[dhcp_pos:dhcp_end]

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
ras_wanx_pattern = r"modem_start_status\s*=\s*rc_first_service_start_status\s*\(\s*modem_start_status\s*,\s*wanx_status\s*\)\s*;"
ras_wanx_match = re.search(ras_wanx_pattern, ras_branch)
assert ras_wanx_match, "RAS physical WAN setup failure must reach modem startup status"
assert ras_branch.index("if (wanx_status != 0)") < ras_wanx_match.start()
eapol_start = wan_start.index("if (auth_status != 0)")
eapol_end = wan_start.index("/* We are done configuration */", eapol_start)
assert "modem_start_status" not in wan_start[eapol_start:eapol_end]
assert wan_start.index("set_passthrough_pppoe(1)") < wan_start.index("return rc_first_service_start_status(modem_start_status, wan_start_status);")

l2tp_start_merge = compact(
    body(WAN_SOURCE, "merge_l2tp_server_start_status(int wan_status)")
)
assert 'nvram_match("l2tp_srv_t", "1")' in l2tp_start_merge
assert "xl2tp_status = safe_start_xl2tpd();" in l2tp_start_merge
assert "wan_status == 0 && xl2tp_status < 0" in l2tp_start_merge
assert "return xl2tp_status;" in l2tp_start_merge
assert "return wan_status;" in l2tp_start_merge

for signature in (
    "full_restart_wan(void)",
    "try_wan_reconnect(int try_use_modem, long pause_in_seconds)",
    "deferred_wan_connect(void)",
):
    function = compact(body(WAN_SOURCE, signature))
    assert "status = start_wan();" in function
    assert "return merge_l2tp_server_start_status(status);" in function

wan_stop = compact(body(WAN_SOURCE, "stop_wan_locked(void)"))
assert "stop_status = wan_down(man_ifname, unit, 1);" in wan_stop
assert "static WAN route teardown incomplete" in wan_stop
assert "int modem_stop_status = stop_wan_usbnet();" in wan_stop
assert "rc_first_service_start_status(stop_status, modem_stop_status)" in wan_stop
assert wan_stop.index("stop_status = wan_down(man_ifname, unit, 1);") < wan_stop.index("int modem_stop_status = stop_wan_usbnet();")

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
