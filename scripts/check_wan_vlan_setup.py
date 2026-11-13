#!/usr/bin/env python3
"""Static contract checks for WAN VLAN and bridge configuration error handling."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        print(f"WAN VLAN/bridge contract: FAIL: {message}", file=sys.stderr)
        raise SystemExit(1)


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError(f"unterminated function: {signature}")


rc = ROOT / "trunk/user/rc/src"
shared = ROOT / "trunk/user/shared/src"
net = (rc / "net.c").read_text()
wan = (rc / "net_wan.c").read_text()
rc_header = (rc / "rc.h").read_text()
netutils = (shared / "netutils.c").read_text()
netutils_header = (shared / "netutils.h").read_text()
workflow = (ROOT / ".github/workflows/wan-route-status.yml").read_text()

create_vlan = body(net, "create_vlan_iface(const char *dev_ifname")
for marker in ("vconfig_add_if(", "vconfig_egress_map(", "set_interface_mtu(",
               "set_interface_hwaddr(", "ifconfig(vlan_ifname, IFUP",
               "ifconfig(vlan_ifname, 0", "vconfig_del_if(", "return ret;"):
    require(marker in create_vlan, f"create_vlan_iface() must propagate/rollback {marker}")
require("int\ncreate_vlan_iface(" in net and
        "int create_vlan_iface(" in rc_header and
        "void create_vlan_iface(" not in rc_header,
        "create_vlan_iface declaration/definition must return status")
remove_vlan = body(net, "remove_vlan_iface(const char *vlan_ifname")
require("ifconfig(vlan_ifname, 0" in remove_vlan and
        "return vconfig_del_if(vlan_ifname);" in remove_vlan and
        "int remove_vlan_iface(" in rc_header,
        "remove_vlan_iface must report detach/delete errors")

for name in ("br_set_param_int", "br_set_stp", "br_set_fd",
             "brport_set_param_int", "brport_set_m2u"):
    require(f"int\n{name}(" in netutils and
            f"extern int       {name}(" in netutils_header,
            f"{name} must return the sysfs write status")
require("return fput_int(bridge_path, value);" in body(netutils, "br_set_param_int("),
        "bridge sysfs writes must propagate fput_int failure")
require("return fput_int(brport_path, value);" in body(netutils, "brport_set_param_int("),
        "bridge-port sysfs writes must propagate fput_int failure")
m2u = body(netutils, "brport_set_m2u(")
require(m2u.count("brport_set_param_int(") == 2 and "return status;" in m2u,
        "multicast bridge-port configuration must report either sysfs failure")
fd = body(netutils, "br_set_fd(")
require("INT_MAX / 100" in fd and "return br_set_param_int(" in fd,
        "bridge forward-delay conversion must reject integer overflow")

soft_lan = body(wan, "config_soft_bridge_lan(void)")
for marker in ("create_vlan_iface(", "br_add_del_if(", "remove_vlan_iface(",
               "brport_set_m2u(", "if (ret != 0)", "return ret;"):
    require(marker in soft_lan, f"LAN bridge setup ignores {marker}")
soft_wan = body(wan, "config_soft_bridge_wan(")
for marker in ("br_add_del_bridge(", "br_set_stp(", "br_set_fd(",
               "br_set_param_int(", "set_interface_hwaddr(", "set_interface_mtu(",
               "create_vlan_iface(", "br_add_del_if(", "brport_set_param_int(",
               "return ifconfig(br_ifname, IFUP"):
    require(marker in soft_wan, f"WAN bridge setup omits {marker}")
for marker in ("ret = br_add_del_bridge(", "ret = br_set_stp(", "ret = br_set_fd(",
               "ret = br_set_param_int(", "ret = set_interface_hwaddr(",
               "ret = set_interface_mtu(", "ret = create_vlan_iface(",
               "ret = br_add_del_if(", "ret = brport_set_param_int("):
    require(marker in soft_wan, f"WAN bridge setup discards {marker}")
require(soft_wan.count("if (ret != 0)") >= 9,
        "WAN bridge setup does not fail closed on each setup stage")

soft_other = body(wan, "config_soft_bridges_other(")
require("int\nconfig_soft_bridges_other(" in wan and
        "ret = create_vlan_iface(" in soft_other and
        "ret = config_soft_bridge_wan(" in soft_other and
        "return ret;" in soft_other,
        "additional LAN bridge VLAN failures must reach primary WAN startup")
vinet = body(wan, "config_vinet_wan(void)")
require("ret = create_vlan_iface(" in vinet and
        "ret = config_soft_bridges_other(" in vinet and
        "ret = config_soft_bridge_wan(" in vinet and
        "return ret;" in vinet,
        "primary WAN VLAN/bridge failures must abort config_vinet_wan")
require(vinet.index("ret = create_vlan_iface(") < vinet.index('set_wan_unit_value(0, "viflast"'),
        "last-good WAN VLAN name must not be published before VLAN creation")
viptv = body(wan, "launch_viptv_wan(void)")
iptv_start = viptv.index("/* VLAN interface for IPTV */")
iptv_setup = viptv[iptv_start:]
bridge_pos = iptv_setup.index("ret = config_soft_bridge_wan(")
rp_filter_pos = iptv_setup.index('ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);')
publish_pos = iptv_setup.index('nvram_set_temp("viptv_ifname", viptv_ifname);')
require(iptv_setup.index("ret = create_vlan_iface(") < bridge_pos < rp_filter_pos < publish_pos,
        "IPTV VLAN, bridge and rp_filter setup must complete before publishing its interface")
for operation in ("ret = create_vlan_iface(", "ret = config_soft_bridge_wan(",
                  'ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);'):
    operation_pos = iptv_setup.index(operation)
    error_gate = iptv_setup.index("if (ret != 0)", operation_pos)
    error_return = iptv_setup.index("return ret;", error_gate)
    require(operation_pos < error_gate < error_return < publish_pos,
            f"IPTV setup must abort before interface publication after {operation}")
restart = body(wan, "full_restart_wan(void)")
require("config_soft_bridge_lan() != 0" in restart,
        "full WAN restart must stop when LAN-side VLAN bridge setup fails")

for path in ("trunk/user/shared/src/netutils.c",
             "trunk/user/shared/src/netutils.h",
             "scripts/check_wan_vlan_setup.py"):
    push, pr = workflow.split("  pull_request:", 1)
    require(push.count(f'"{path}"') == 1 and pr.count(f'"{path}"') == 1,
            f"WAN workflow push/PR paths must include {path}")
require("python3 scripts/check_wan_vlan_setup.py" in workflow,
        "WAN workflow must execute the VLAN/bridge source contract")
print("WAN VLAN and bridge error-propagation contracts: PASS")
