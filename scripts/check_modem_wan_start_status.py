#!/usr/bin/env python3
"""Verify USB modem WAN launch failures reach boot and RC action results."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
USB_SOURCE = (RC / "usb_modem.c").read_text()
WAN_SOURCE = (RC / "net_wan.c").read_text()
STORAGE_SOURCE = (RC / "services_stor.c").read_text()
SERVICES_USB_SOURCE = (RC / "services_usb.c").read_text()
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
wanx = compact(body(WAN_SOURCE, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)"))
static_man_validation = "if (!dhcp_lease_ipv4_is_valid(man_addr) || !dhcp_lease_netmask_is_valid(man_mask))"
assert static_man_validation in wanx, "static MAN must reject malformed IPv4 address/netmask"
man_invalid_pos = wanx.index(static_man_validation)
man_subnet_pos = wanx.index("else if (is_same_subnet2(man_addr, lan_addr, man_mask, lan_mask))")
man_mtu_pos = wanx.index("mtu_status = set_interface_mtu(man_ifname, man_mtu);")
man_address_pos = wanx.index("address_status = ifconfig(man_ifname, IFUP, man_addr, man_mask);")
assert man_invalid_pos < man_subnet_pos < man_mtu_pos < man_address_pos
man_invalid_path = wanx[man_invalid_pos:man_subnet_pos]
assert "man_err = 1;" in man_invalid_path
assert "launch_status = EINVAL;" in man_invalid_path
assert "invalid static WAN IPv4 address or netmask" in man_invalid_path
assert 'logmessage(LOGNAME, "MAN interface %s MTU setup incomplete (status=%d)"' in wanx
assert "if (mtu_status != 0)" in wanx
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
assert "mtu_status = set_interface_mtu(wan_ifname, wan_mtu);" in static_config
assert 'logmessage(LOGNAME, "static WAN interface %s MTU setup incomplete (status=%d)"' in static_config
assert static_config.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);") > subnet_conflict_start
assert static_config.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);") < static_config.index("address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);")

wan_dhcp_bound = compact(body(WAN_SOURCE, "udhcpc_bound(char *wan_ifname, int is_renew_mode)"))
assert "mtu_status = set_interface_mtu(wan_ifname, dhcp_mtu);" in wan_dhcp_bound
assert 'logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)"' in wan_dhcp_bound
wan_mtu_failure = wan_dhcp_bound[wan_dhcp_bound.index("if (mtu_status != 0)"):wan_dhcp_bound.index("wan_gw = nvram_safe_get", wan_dhcp_bound.index("if (mtu_status != 0)"))]
assert "i_err =" not in wan_mtu_failure and "route_setup_status =" not in wan_mtu_failure and "return " not in wan_mtu_failure

iptv_dhcp_bound = compact(body(WAN_SOURCE, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)"))
assert "int interface_status, rollback_status, route_setup_status, mtu_status;" in iptv_dhcp_bound
assert "mtu_status = set_interface_mtu(man_ifname, dhcp_mtu);" in iptv_dhcp_bound
assert 'logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)"' in iptv_dhcp_bound
iptv_mtu_failure = iptv_dhcp_bound[iptv_dhcp_bound.index("if (mtu_status != 0)"):iptv_dhcp_bound.index("logmessage(log_prefix,", iptv_dhcp_bound.index("if (mtu_status != 0)"))]
assert "i_err =" not in iptv_mtu_failure and "route_setup_status =" not in iptv_mtu_failure and "return " not in iptv_mtu_failure

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


deferred_hotplug = compact(body(STORAGE_SOURCE, "on_deferred_hotplug_dev(void)"))
modem_start = compact(body(SERVICES_USB_SOURCE, "try_start_usb_modem_to_wan(void)"))
assert "return try_wan_reconnect(1, 0);" in modem_start
assert "return;" not in modem_start

unplug_event = deferred_hotplug.index('if (nvram_match("usb_unplug_md", "1"))')
plug_event = deferred_hotplug.index('if (nvram_match("usb_hotplug_md", "1"))')
unplug_attempt = deferred_hotplug.index("if (unplug_modem)")
plug_attempt = deferred_hotplug.index("if (plug_modem && !unplug_modem)")
retry_policy = deferred_hotplug.index("modem_wan_hotplug_retry_mask(")
assert unplug_event < plug_event < unplug_attempt < plug_attempt < retry_policy

plug_capture = deferred_hotplug[plug_event:unplug_attempt]
assert "plug_modem = 1;" in plug_capture
assert "if (!unplug_modem)" in plug_capture
assert plug_capture.index("if (!unplug_modem)") < plug_capture.index('nvram_set_int_temp("usb_hotplug_md", 0)')
assert "modem_unplug_status = try_wan_reconnect(1, 0);" in deferred_hotplug
assert "modem_plug_status = try_start_usb_modem_to_wan();" in deferred_hotplug
assert "MODEM_WAN_HOTPLUG_RETRY_UNPLUG" in deferred_hotplug
assert "MODEM_WAN_HOTPLUG_RETRY_PLUG" in deferred_hotplug
assert 'nvram_set_int_temp("usb_unplug_md", 1)' in deferred_hotplug
assert 'nvram_set_int_temp("usb_hotplug_md", 1)' in deferred_hotplug
assert "if (modem_retry_mask != 0)" in deferred_hotplug
assert "alarm(5);" in deferred_hotplug
assert "test_modem_hotplug_retry_mask_preserves_failed_and_coalesced_events" in (
    RC / "tests/modem_wan_start_policy.c"
).read_text()

print("USB modem WAN startup status propagation: PASS")
