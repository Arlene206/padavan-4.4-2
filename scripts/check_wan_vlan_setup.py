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
lan = (rc / "net_lan.c").read_text()
bridge_policy = (shared / "bridge_membership_policy.h").read_text()
bridge_test = (shared / "tests/bridge_membership_policy.c").read_text()
rc_header = (rc / "rc.h").read_text()
netutils = (shared / "netutils.c").read_text()
netutils_header = (shared / "netutils.h").read_text()
vlan_policy = (rc / "vlan_interface_policy.h").read_text()
vlan_test = (rc / "tests/vlan_interface_policy.c").read_text()
wan_nvram_policy = (rc / "wan_nvram_status_policy.h").read_text()
wan_nvram_test = (rc / "tests/wan_nvram_status_policy.c").read_text()
viptv_policy = (rc / "wan_viptv_static_policy.h").read_text()
workflow = (ROOT / ".github/workflows/wan-route-status.yml").read_text()

require('#include "vlan_interface_policy.h"' in net,
        "net.c must use the tested VLAN interface error policy")
inspect_vlan = body(net, "inspect_vlan_iface(")
require("get_interface_flags(vlan_ifname)" in inspect_vlan and
        "vlan_interface_probe_result(flags, probe_errno, exists)" in inspect_vlan and
        "errno = status;" in inspect_vlan,
        "VLAN inspection must distinguish absence from probe errors and preserve errno")
flags_reader = body(netutils, "get_interface_flags(const char *ifname)")
require("saved_errno = errno;" in flags_reader and
        "errno = saved_errno;" in flags_reader and
        "close(sockfd);" in flags_reader,
        "get_interface_flags() must preserve ioctl/copy errno across socket close")

create_vlan = body(net, "create_vlan_iface_internal(")
for marker in ("vconfig_add_if(", "vconfig_egress_map(", "set_interface_mtu(",
               "set_interface_hwaddr(", "ifconfig(vlan_ifname, IFUP",
               "ifconfig(vlan_ifname, 0", "vconfig_del_if(", "return ret;"):
    require(marker in create_vlan, f"create_vlan_iface_internal() must propagate/rollback {marker}")
require("if (created_out)\n\t\t*created_out = 0;" in create_vlan and
        "if (created_out)\n\t\t*created_out = created;" in create_vlan and
        "*created_out = 1;" in create_vlan and
        "vlan_interface_retain_ownership(created, cleanup_status)" in create_vlan and
        "vlan_interface_cleanup_status(cleanup_down_status, cleanup_delete_status)" in " ".join(create_vlan.split()),
        "tracked VLAN creation must publish surviving ownership when self-rollback is incomplete")
require("inspect_vlan_iface(vlan_ifname, &vlan_exists)" in create_vlan and
        "is_interface_exist(vlan_ifname)" not in create_vlan,
        "VLAN creation must fail closed on unknown interface state")
require("int\ncreate_vlan_iface(" in net and
        "int create_vlan_iface(" in rc_header and
        "int create_vlan_iface_tracked(" in rc_header and
        "create_vlan_iface_internal(" in body(net, "create_vlan_iface_tracked("),
        "public and tracked VLAN creation declarations must return status")
remove_vlan = body(net, "remove_vlan_iface(const char *vlan_ifname")
require("ifconfig(vlan_ifname, 0" in remove_vlan and
        "return vconfig_del_if(vlan_ifname);" in remove_vlan and
        "int remove_vlan_iface(" in rc_header and
        "inspect_vlan_iface(vlan_ifname, &vlan_exists)" in remove_vlan and
        "is_interface_exist(vlan_ifname)" not in remove_vlan,
        "remove_vlan_iface must distinguish absent interfaces from inspection errors and report detach/delete errors")
for test_name in ("check_probe(0, 0, 0, 1)",
                  "check_probe(-1, ENODEV, 0, 0)",
                  "check_probe(-1, ENXIO, 0, 0)",
                  "check_probe(-1, EPERM, EPERM, 0)",
                  "check_probe(-1, 0, EIO, 0)",
                  "vlan_interface_cleanup_status(EIO, EBUSY)",
                  "vlan_interface_retain_ownership(1, EIO)",
                  "vlan_interface_retain_ownership(0, EIO)"):
    require(test_name in vlan_test, f"VLAN interface policy regression missing {test_name}")
require("vlan-interface-policy" in workflow and
        "tests/vlan_interface_policy.c" in workflow and
        "ASAN_OPTIONS=detect_leaks=1 /tmp/vlan-interface-policy" in workflow,
        "WAN workflow must compile and run VLAN interface sanitizer tests")

for name in ("br_set_param_int", "br_set_stp", "br_set_fd",
             "brport_set_param_int", "brport_set_m2u"):
    require(f"int\n{name}(" in netutils and
            f"extern int       {name}(" in netutils_header,
            f"{name} must return the sysfs write status")
require("return fput_int(bridge_path, value);" in body(netutils, "br_set_param_int("),
        "bridge sysfs writes must propagate fput_int failure")
require("return fput_int(brport_path, value);" in body(netutils, "brport_set_param_int("),
        "bridge-port sysfs writes must propagate fput_int failure")
bridge_internal = body(netutils, "br_add_del_if_internal(")
require("*changed_out = 0;" in bridge_internal and
        "bridge_membership_operation_changed(is_add, membership, ret)" in bridge_internal,
        "tracked bridge operation must report only a membership change made by its ioctl")
changed_pos = bridge_internal.index("*changed_out = bridge_membership_operation_changed")
race_pos = bridge_internal.index("if (ret < 0 && bridge_membership_race_is_idempotent(")
require(changed_pos < race_pos,
        "a raced-to-idempotent bridge request must not claim ownership of the change")
tracked_bridge = body(netutils, "br_add_del_if_tracked(")
require("if (!changed_out)" in tracked_bridge and
        "br_add_del_if_internal(" in tracked_bridge and
        "extern int       br_add_del_if_tracked(" in netutils_header,
        "tracked bridge membership API must validate and expose ownership")
require("bridge_membership_operation_changed" in bridge_policy and
        "expect_membership_change(" in bridge_test,
        "bridge membership ownership policy must have regression coverage")
m2u = body(netutils, "brport_set_m2u(")
require(m2u.count("brport_set_param_int(") == 2 and "return status;" in m2u,
        "multicast bridge-port configuration must report either sysfs failure")
fd = body(netutils, "br_set_fd(")
require("INT_MAX / 100" in fd and "return br_set_param_int(" in fd,
        "bridge forward-delay conversion must reject integer overflow")

wan_start = body(wan, "start_wan(void)")
static_mtu_start = wan_start.index("if (wan_mtu >= 1300 && wan_mtu < 1500)")
static_mtu_end = wan_start.index('set_wan_unit_error_status(unit, wan_err);',
                                static_mtu_start)
static_mtu = wan_start[static_mtu_start:static_mtu_end]
mtu_failure = static_mtu.index("if (mtu_status != 0)")
mtu_error_flag = static_mtu.index("wan_err = 1;", mtu_failure)
mtu_status_merge = static_mtu.index("wan_start_status = rc_first_service_start_status(",
                                    mtu_failure)
address_gate = static_mtu.index("if (!wan_err) {")
address_setup = static_mtu.index("address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);")
require(mtu_failure < mtu_error_flag < mtu_status_merge < address_gate < address_setup,
        "failed configured static MTU must set WAN error, preserve first status, and block address/route startup")

range_policy = body(wan_nvram_policy, "rc_wan_nvram_int_range_status(")
require("if (read_status != 0)" in range_policy and "*normalized = value;" in range_policy,
        "WAN checked range policy must preserve read errors and publish only validated output")
for test_name in ("test_checked_integer_range_preserves_valid_values_and_legacy_fallback",
                  "test_checked_integer_range_preserves_read_errors_and_output"):
    require(test_name in wan_nvram_test and test_name + "();" in wan_nvram_test,
            "WAN bridge range-policy test missing " + test_name)

mode_reader = body(wan, "get_wan_bridge_mode_status(")
mode_reader_compact = " ".join(mode_reader.split())
require('get_nvram_int_checked_status("wan_stb_x", &configured_mode)' in mode_reader_compact and
        "rc_wan_nvram_int_range_status(" in mode_reader,
        "WAN bridge mode must use checked reads and retain the disabled fallback")
iso_reader = body(wan, "get_wan_bridge_iso_mode_status(")
iso_reader_compact = " ".join(iso_reader.split())
require('get_nvram_int_checked_status("wan_stb_iso", &configured_isolation)' in iso_reader_compact and
        "rc_wan_nvram_int_range_status(" in iso_reader,
        "WAN bridge isolation must use checked reads and retain the no-isolation fallback")

soft_lan = body(wan, "config_soft_bridge_lan(void)")
soft_lan_code = " ".join(soft_lan.split())
igmp_read = 'get_nvram_int_checked_status("ether_igmp", &igmp_snoop)'
m2u_read = 'get_nvram_int_checked_status("ether_m2u", &wired_m2u)'
igmp_pos = soft_lan_code.index(igmp_read)
igmp_gate = soft_lan_code.index("if (ret != 0)", igmp_pos)
igmp_return = soft_lan_code.index("return ret;", igmp_gate)
m2u_pos = soft_lan_code.index(m2u_read)
m2u_gate = soft_lan_code.index("if (ret != 0)", m2u_pos)
m2u_return = soft_lan_code.index("return ret;", m2u_gate)
mode_read_pos = soft_lan_code.index("get_wan_bridge_mode_status(&bridge_mode)")
mode_error_return = soft_lan_code.index("return ret;", mode_read_pos)
first_lan_probe = soft_lan_code.index("get_interface_flags(lan_ifname)")
require(igmp_pos < igmp_gate < igmp_return < m2u_pos < m2u_gate < m2u_return <
        mode_read_pos < mode_error_return < first_lan_probe,
        "LAN bridge multicast settings and WAN bridge mode must all be checked before interface probes/topology changes")
require('nvram_get_int("ether_igmp")' not in soft_lan_code and
        'nvram_get_int("ether_m2u")' not in soft_lan_code,
        "LAN bridge multicast configuration must not collapse NVRAM read errors")

soft_wan = body(wan, "config_soft_bridge_wan_internal(")
iso_read_pos = soft_wan.index("get_wan_bridge_iso_mode_status(&bwan_isolation)")
iso_error_return = soft_wan.index("return ret;", iso_read_pos)
first_wan_probe = soft_wan.index("get_interface_flags(br_ifname)")
require(iso_read_pos < iso_error_return < first_wan_probe and
        "if (bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)" in soft_wan and
        'nvram_get_int("wan_stb_iso")' not in soft_wan,
        "WAN bridge isolation read failures must abort before topology mutations")

# The HW-NAT WAN VID resolver must not interpret an unreadable filter/VID
# as disabled or silently substitute the physical switch PVID.
wan_vid = body(wan, "get_vlan_vid_wan(void)")
wan_vid_code = " ".join(wan_vid.split())
filter_status_read = 'get_nvram_match_checked_status("vlan_filter", "1", &is_vlan_filter)'
configured_vid_read = 'get_nvram_int_checked_status("vlan_vid_cpu", &vlan_vid)'
filter_pos = wan_vid_code.index(filter_status_read)
filter_failure = wan_vid_code.index("if (status != 0)", filter_pos)
filter_error_return = wan_vid_code.index("wan_vlan_vid_policy_read_failure_result(status)", filter_failure)
vid_pos = wan_vid_code.index(configured_vid_read)
vid_failure = wan_vid_code.index("if (status != 0)", vid_pos)
vid_error_return = wan_vid_code.index("wan_vlan_vid_policy_read_failure_result(status)", vid_failure)
pvid_fallback = wan_vid_code.index("phy_vlan_pvid_wan_get()")
require(filter_pos < filter_failure < filter_error_return < vid_pos < vid_failure < vid_error_return < pvid_fallback,
        "HW-NAT WAN VID must abort on selector/VID read errors before any physical-PVID fallback")
require('nvram_match("vlan_filter", "1")' not in wan_vid_code and
        'nvram_get_int("vlan_vid_cpu")' not in wan_vid_code and
        "wan_vlan_vid_policy_read_failure_result" in wan_vid_code,
        "HW-NAT WAN VID lookup must use checked selectors and preserve negative errno")
wan_vid_policy = (rc / "wan_vlan_vid_policy.h").read_text()
wan_vid_test = (rc / "tests/wan_vlan_vid_policy.c").read_text()
require("return read_status > 0 ? -read_status : read_status;" in wan_vid_policy and
        "wan_vlan_vid_policy_read_failure_result(EIO) == -EIO" in wan_vid_test and
        "wan_vlan_vid_policy_read_failure_result(ENODEV) == -ENODEV" in wan_vid_test,
        "WAN VLAN ID read-error mapping must have negative-errno regression coverage")

vinet = body(wan, "config_vinet_wan(void)")
vinet_code = " ".join(vinet.split())
mode_read_pos = vinet_code.index("get_wan_bridge_mode_status(&bridge_mode)")
mode_error_return = vinet_code.index("return ret;", mode_read_pos)
filter_read = 'get_nvram_match_checked_status("vlan_filter", "1", &is_vlan_filter)'
filter_read_pos = vinet_code.index(filter_read)
filter_error_gate = vinet_code.index("if (ret != 0)", filter_read_pos)
filter_error_return = vinet_code.index("return ret;", filter_error_gate)
cpu_vid_read = 'get_nvram_int_checked_status("vlan_vid_cpu", &vlan_vid[0])'
iptv_vid_read = 'get_nvram_int_checked_status("vlan_vid_iptv", &vlan_vid[1])'
cpu_pri_read = 'get_nvram_int_checked_status("vlan_pri_cpu", &vlan_pri)'
cpu_vid_pos = vinet_code.index(cpu_vid_read)
iptv_vid_pos = vinet_code.index(iptv_vid_read)
cpu_pri_pos = vinet_code.index(cpu_pri_read)
first_vlan_change = vinet_code.index("hw_vlan_tx_map(6, vlan_vid[0])")
require(mode_read_pos < mode_error_return < filter_read_pos < filter_error_gate < filter_error_return and
        filter_error_return < cpu_vid_pos < iptv_vid_pos < cpu_pri_pos < first_vlan_change and
        "if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE)" in vinet_code and
        "get_wan_bridge_mode()" not in vinet_code,
        "WAN VLAN setup must check the filter and all enabled VLAN inputs before any VLAN/interface mutation")
for key in ("vlan_vid_cpu", "vlan_vid_iptv", "vlan_pri_cpu"):
    require(f'nvram_get_int("{key}")' not in vinet_code,
            f"WAN VLAN setup must not collapse read errors for {key}")
require('nvram_match("vlan_filter", "1")' not in vinet_code,
        "WAN VLAN enable must not collapse a checked NVRAM backend error")

config_bridge = body(lan, "config_bridge(int is_ap_mode)")
config_bridge_code = " ".join(config_bridge.split())
igmp_read = 'lan_nvram_get_int_status("ether_igmp", &igmp_snoop)'
m2u_read = 'lan_nvram_get_int_status("ether_m2u", &wired_m2u)'
mr_read = 'lan_nvram_get_match_status("mr_enable_x", "1", &mr_enable)'
igmp_static_port_read = 'lan_nvram_get_int_status("ether_uport", &igmp_static_port)'
igmp_pos = config_bridge_code.index(igmp_read)
m2u_pos = config_bridge_code.index(m2u_read)
mr_pos = config_bridge_code.index(mr_read)
first_apply = config_bridge_code.index("rc_bridge_config_apply(")
require(igmp_pos < m2u_pos < mr_pos < first_apply and
        config_bridge_code.index("if (status != 0)", igmp_pos) < config_bridge_code.index("return status;", igmp_pos) and
        config_bridge_code.index("if (status != 0)", m2u_pos) < config_bridge_code.index("return status;", m2u_pos) and
        config_bridge_code.index("if (status != 0)", mr_pos) < config_bridge_code.index("return status;", mr_pos),
        "LAN bridge IGMP, multicast-to-unicast and multicast-router settings must fail before bridge configuration")
require(igmp_static_port_read in config_bridge_code and
        config_bridge_code.index("if (status != 0)", config_bridge_code.index(igmp_static_port_read)) <
        config_bridge_code.index("return status;", config_bridge_code.index(igmp_static_port_read)) <
        first_apply,
        "AP LAN bridge static port read must be checked before bridge configuration")
require('lan_wan_bridge_mode_status(&bridge_mode)' in config_bridge_code and
        'get_wan_bridge_mode()' not in config_bridge_code,
        "GMAC2 LAN bridge port selection must use checked WAN bridge mode")
for old_read in ('nvram_get_int("ether_igmp")','nvram_get_int("ether_m2u")',
                 'nvram_get_int("ether_uport")','nvram_match("mr_enable_x", "1")'):
    require(old_read not in config_bridge_code,
            f"LAN bridge must not collapse NVRAM errors for {old_read}")

switch_vlan = body(lan, "switch_config_vlan(int first_call)")
switch_vlan_code = " ".join(switch_vlan.split())
# Match the wrapped C calls independent of the line break after the opening parenthesis.
switch_vlan_code = switch_vlan_code.replace(
    "lan_nvram_get_int_before_vlan_apply( ",
    "lan_nvram_get_int_before_vlan_apply("
)
require("int status;" in switch_vlan_code,
        "switch VLAN checked-read status must be declared locally")
mode_read = 'lan_wan_bridge_mode_status(&bridge_mode)'
isolation_read = 'lan_wan_bridge_iso_mode_status(bridge_mode, &bwan_isolation)'
filter_read = 'lan_nvram_get_match_status("vlan_filter", "1", &is_vlan_filter)'
vid_reads = [
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_cpu", &vlan_vid[SWAPI_VLAN_RULE_WAN_INET])',
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_iptv", &vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV])',
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_lan1", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN1])',
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_lan2", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN2])',
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_lan3", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN3])',
    'lan_nvram_get_int_before_vlan_apply("vlan_vid_lan4", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN4])',
]
pri_reads = [
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_cpu", &vlan_pri[SWAPI_VLAN_RULE_WAN_INET])',
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_iptv", &vlan_pri[SWAPI_VLAN_RULE_WAN_IPTV])',
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_lan1", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN1])',
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_lan2", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN2])',
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_lan3", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN3])',
    'lan_nvram_get_int_before_vlan_apply("vlan_pri_lan4", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN4])',
]
tag_reads = [
    'lan_nvram_get_int_before_vlan_apply("vlan_tag_lan1", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN1])',
    'lan_nvram_get_int_before_vlan_apply("vlan_tag_lan2", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN2])',
    'lan_nvram_get_int_before_vlan_apply("vlan_tag_lan3", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN3])',
    'lan_nvram_get_int_before_vlan_apply("vlan_tag_lan4", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN4])',
]
read_order = [mode_read, isolation_read, filter_read] + vid_reads + pri_reads + tag_reads
read_positions = [switch_vlan_code.index(item) for item in read_order]
first_rule_apply = switch_vlan_code.index("phy_vlan_rule_set(")
require(read_positions == sorted(read_positions) and read_positions[-1] < first_rule_apply,
        "switch VLAN mode, selector, VIDs, priorities and tag flags must all be checked before programming any VLAN rule")
priority_rules = ("WAN_INET", "WAN_IPTV", "WAN_LAN1", "WAN_LAN2", "WAN_LAN3", "WAN_LAN4")
priority_masks = [
    f"vlan_pri[SWAPI_VLAN_RULE_{rule}] &= 0x07;"
    for rule in priority_rules
]
require(all(mask in switch_vlan_code for mask in priority_masks),
        "all six VLAN priorities must retain their legacy three-bit normalization")
priority_mask_positions = [switch_vlan_code.index(mask) for mask in priority_masks]
priority_read_positions = [switch_vlan_code.index(read) for read in pri_reads]
require(
    all(read < mask for read, mask in zip(priority_read_positions, priority_mask_positions)) and
    priority_mask_positions == sorted(priority_mask_positions) and
    priority_mask_positions[-1] < first_rule_apply,
    "each checked VLAN priority must be normalized after its read and before hardware programming"
)
require("if (status != 0) return status;" in switch_vlan_code and
        "lan_wan_bridge_mode_status" in switch_vlan_code and
        "lan_wan_bridge_iso_mode_status" in switch_vlan_code,
        "switch VLAN read failures must abort before hardware VLAN/bridge programming")
for key in ("vlan_vid_cpu","vlan_vid_iptv","vlan_vid_lan1","vlan_vid_lan2",
            "vlan_vid_lan3","vlan_vid_lan4","vlan_pri_cpu","vlan_pri_iptv",
            "vlan_pri_lan1","vlan_pri_lan2","vlan_pri_lan3","vlan_pri_lan4",
            "vlan_tag_lan1","vlan_tag_lan2","vlan_tag_lan3","vlan_tag_lan4"):
    require(f'nvram_get_int("{key}")' not in switch_vlan_code,
            f"switch VLAN setup must not collapse NVRAM errors for {key}")
require('nvram_match("vlan_filter", "1")' not in switch_vlan_code and
        "get_wan_bridge_mode()" not in switch_vlan_code and
        "get_wan_bridge_iso_mode(" not in switch_vlan_code,
        "switch VLAN selector/bridge modes must not use lossy getters")

storm = body(lan, "switch_config_storm(void)")
storm_code = " ".join(storm.split())
storm_keys = (
    "controlrate_unknown_unicast",
    "controlrate_unknown_multicast",
    "controlrate_multicast",
    "controlrate_broadcast",
)
storm_positions = [
    storm_code.index(f'lan_nvram_get_int_range_logged_status( "{key}", 1, 1024, 1024,')
    for key in storm_keys
]
first_storm_apply = storm_code.index("phy_storm_unicast_unknown(")
require(storm_positions == sorted(storm_positions) and
        storm_positions[-1] < first_storm_apply and
        "if (read_status != 0) return read_status;" in storm_code,
        "all storm-control rates must be checked and range-normalized before any switch storm-control operation")
require("rc_wan_nvram_int_range_status( status, configured_value, minimum, maximum, fallback, normalized)" in
        " ".join(body(lan, "lan_nvram_get_int_range_logged_status(").split()),
        "storm-control rate helper must preserve legacy fallback only for successfully read out-of-range values")
for key in storm_keys:
    require(f'nvram_get_int("{key}")' not in storm_code,
            f"storm-control rate {key} must not collapse NVRAM read errors")

switch_base = body(lan, "switch_config_base(void)")
switch_base_code = " ".join(switch_base.split())
base_keys = ("ether_jumbo", "ether_green", "ether_eee")
base_positions = [
    switch_base_code.index(f'lan_nvram_get_int_before_switch_apply( "{key}",')
    for key in base_keys
]
first_base_apply = switch_base_code.index("phy_jumbo_frames(")
require(base_positions == sorted(base_positions) and
        base_positions[-1] < first_base_apply and
        "if (read_status != 0) return read_status;" in switch_base_code,
        "jumbo/green/EEE settings must all be checked before the first switch hardware configuration")
for key in base_keys:
    require(f'nvram_get_int("{key}")' not in switch_base_code,
            f"switch base {key} must not collapse NVRAM read errors")

other = body(wan, "config_soft_bridges_other(")
port_reader = body(wan, "read_soft_bridge_port_vlan_config(")
require('get_nvram_int_checked_status(param_name, &configured_vid)' in port_reader and
        'get_nvram_int_checked_status(param_name, &configured_prio)' in port_reader,
        "active additional bridge ports must check both VID and priority")
first_port_probe = other.index("get_interface_flags(port_ifname)")
for port in range(1, 5):
    call = f"read_soft_bridge_port_vlan_config({port},"
    if call in other:
        read_pos = other.index(call)
        read_return = other.index("return ret;", read_pos)
        require(read_pos < read_return < first_port_probe,
                f"LAN bridge port {port} VLAN read errors must abort before interface probes")
require('nvram_get_int("vlan_vid_lan' not in other and
        'nvram_get_int("vlan_pri_lan' not in other,
        "additional WAN bridge ports must not collapse NVRAM errors into fallback values")

soft_lan = body(wan, "config_soft_bridge_lan(void)")
for marker in ("create_vlan_iface_tracked(", "br_add_del_if_tracked(",
               "remove_vlan_iface(", "brport_set_m2u(", "rollback:",
               "bridge_rollback_can_remove_created_vlan(",
               "bridge_rollback_can_restore_interface("):
    require(marker in soft_lan, f"LAN bridge setup omits transactional rollback marker {marker}")
soft_lan_rollback = soft_lan[soft_lan.index("rollback:"):]
require("if (vlan_attached)" in soft_lan_rollback and
        "if (physical_detached && vlan_state_safe)" in soft_lan_rollback and
        "if (physical_attached)" in soft_lan_rollback and
        "if (vlan_detached && physical_state_safe)" in soft_lan_rollback,
        "LAN bridge rollback must reverse only memberships changed by this call")
require("vlan_attach_ambiguous" in soft_lan_rollback and
        "physical_attach_ambiguous" in soft_lan_rollback and
        "errno = saved_errno;" in soft_lan_rollback and
        "return ret;" in soft_lan_rollback,
        "LAN bridge rollback must preserve ambiguous state and the original error")
require("bridge_rollback_can_remove_created_vlan" in bridge_policy and
        "bridge_rollback_can_restore_interface" in bridge_policy and
        "bridge_rollback_can_remove_created_vlan" in bridge_test and
        "bridge_rollback_can_restore_interface" in bridge_test,
        "LAN bridge rollback ownership gates require host regression coverage")
soft_wan = body(wan, "config_soft_bridge_wan_internal(")
require('#include "bridge_membership_policy.h"' in wan and
        "config_soft_bridge_wan_internal(" in wan and
        "config_soft_bridge_wan(" in wan,
        "WAN bridge transaction policy must be available to the RC batch coordinator")
require("config_soft_bridge_wan_internal(br_id, vid, prio, wan_ifname," in wan and
        "wan_hwaddr, NULL);" in wan,
        "legacy WAN bridge callers must retain the compatibility wrapper")
for marker in ("br_add_del_bridge(", "br_set_stp(", "br_set_fd(",
               "br_set_param_int(", "set_interface_hwaddr(", "set_interface_mtu(",
               "create_vlan_iface_tracked(", "br_add_del_if_tracked(", "brport_set_param_int(",
               "ret = ifconfig(br_ifname, IFUP"):
    require(marker in soft_wan, f"WAN bridge setup omits {marker}")
for marker in ("ret = br_add_del_bridge(", "ret = br_set_stp(", "ret = br_set_fd(",
               "ret = br_set_param_int(", "ret = set_interface_hwaddr(",
               "ret = set_interface_mtu(", "ret = create_vlan_iface_tracked(",
               "ret = br_add_del_if_tracked(", "ret = brport_set_param_int("):
    require(marker in soft_wan, f"WAN bridge setup discards {marker}")
require(soft_wan.count("if (ret != 0)") >= 10,
        "WAN bridge setup does not fail closed on each setup stage")
activation_pos = soft_wan.index('ret = ifconfig(br_ifname, IFUP, "0.0.0.0", NULL);')
activation_gate = soft_wan.index("if (ret != 0)", activation_pos)
require(activation_pos < activation_gate < soft_wan.index("goto rollback;", activation_gate),
        "failed final bridge activation must enter rollback")
rollback = soft_wan[soft_wan.index("rollback:"):]
require("if (bwan_port_added)" in rollback and
        "if (wan_port_added)" in rollback and
        rollback.index("if (bwan_port_added)") < rollback.index("if (wan_port_added)"),
        "owned bridge ports must roll back in reverse attachment order")
require("if (bwan_attach_attempted && !bwan_port_added)" in rollback and
        "vlan_remove_safe = 0;" in rollback and
        "if (vlan_created)" in rollback and
        "if (vlan_remove_safe)" in rollback,
        "uncertain attachment ownership must prevent VLAN deletion")
require("if (bridge_created)" in rollback and
        "if (bridge_remove_safe &&" in rollback and
        "bridge_batch_can_remove_created_bridge(bridge_created, bridge_empty)" in rollback and
        "br_bridge_is_empty(br_ifname)" in rollback and
        "br_add_del_bridge(br_ifname, 0)" in rollback,
        "new bridge removal must be ownership-gated by an empty inventory")
require("else if (bridge_was_up)" in rollback and
        "ifconfig(br_ifname, IFUP" in rollback,
        "pre-existing UP bridge must be restored on failed setup")
require("saved_errno = errno;" in rollback and
        "errno = saved_errno;\n\treturn ret;" in rollback,
        "rollback must preserve the original setup failure")
require("ownership->wan_port_added = wan_port_added" in soft_wan and
        "ownership->bwan_port_added = bwan_port_added" in soft_wan and
        "ownership->bridge_created = 1" in soft_wan and
        "ownership->bwan_vlan_created = vlan_created" in soft_wan,
        "WAN bridge setup must publish per-call bridge, VLAN and membership ownership")
require("ownership->wan_port_detached_safe = 1" in rollback and
        "ownership->bwan_port_detached_safe = 1" in rollback and
        "bridge_batch_can_remove_created_bridge(bridge_created, bridge_empty)" in rollback,
        "WAN bridge local rollback must report detached memberships and require an empty new bridge")

soft_other = body(wan, "config_soft_bridges_other(")
require("int\nconfig_soft_bridges_other(" in wan and
        "struct soft_bridge_other_entry entries[4];" in soft_other and
        "create_vlan_iface_tracked(" in soft_other and
        "config_soft_bridge_wan_internal(" in soft_other and
        "rollback:" in soft_other and
        "while (entry_count > 0)" in soft_other and
        "rollback_soft_bridge_other_entry(&entries[--entry_count])" in soft_other and
        "errno = saved_errno;" in soft_other and
        "return ret;" in soft_other,
        "additional LAN bridge setup must track and reverse the current batch")
other_rollback = body(wan, "rollback_soft_bridge_other_entry(")
require("static int\nrollback_soft_bridge_other_entry(" in wan and
        "bridge_batch_membership_detached_safe(" in other_rollback and
        "br_add_del_if(" in other_rollback and
        "bridge_batch_can_remove_created_bridge(" in other_rollback and
        "br_bridge_is_empty(" in other_rollback and
        "if (entry->port_vlan_created)" in other_rollback and
        "if (bridge->bwan_vlan_created)" in other_rollback and
        "bridge->bridge_existed" in other_rollback and
        "return status;" in other_rollback and
        "if (bwan_detached_safe)" in other_rollback and
        "if (wan_detached_safe)" in other_rollback and
        "bridge_empty_errno = bridge_empty < 0" in other_rollback and
        "route_status_accumulate(status, bridge_empty_errno)" in other_rollback and
        "br_del_all_ifs(" not in other_rollback,
        "batch rollback must report cleanup status and never restore reused VLAN/bridge state while membership is uncertain")
detach_bwan_pos = other_rollback.index("if (bridge->bwan_port_added")
detach_wan_pos = other_rollback.index("if (bridge->wan_port_added")
remove_bwan_vlan_pos = other_rollback.index("if (bridge->bwan_vlan_created)")
remove_port_vlan_pos = other_rollback.index("if (entry->port_vlan_created)")
remove_bridge_pos = other_rollback.index("if (bridge->bridge_name[0])")
require(detach_bwan_pos < detach_wan_pos < remove_bwan_vlan_pos <
        remove_port_vlan_pos < remove_bridge_pos,
        "batch rollback must compensate ports and VLANs in reverse construction order")
bridge_empty = body(netutils, "br_bridge_is_empty(")
require("BRCTL_GET_PORT_LIST" in bridge_empty and
        "bridge_port_list_is_empty(" in bridge_empty and
        "return -1;" in bridge_empty and
        "br_bridge_is_empty(const char *brname)" in netutils_header,
        "bridge removal must use a complete fail-closed port inventory")
require("bridge_port_list_is_empty" in bridge_policy and
        "bridge_batch_membership_detached_safe" in bridge_policy and
        "bridge_batch_can_remove_created_bridge" in bridge_policy and
        "expect_port_list_empty(" in bridge_test and
        "expect_batch_membership_safe(" in bridge_test and
        "expect_batch_bridge_removal(" in bridge_test,
        "batch rollback policies need empty/ownership/failure sanitizer regression coverage")
vinet = body(wan, "config_vinet_wan(void)")
require("ret = create_vlan_iface(" in vinet and
        "ret = config_soft_bridges_other(" in vinet and
        "ret = config_soft_bridge_wan(" in vinet and
        "return ret;" in vinet,
        "primary WAN VLAN/bridge failures must abort config_vinet_wan")
viflast_publish = vinet.index('ret = set_wan_unit_value_status(0, "viflast", vlan_iflast);')
viflast_gate = vinet.index("if (ret != 0)", viflast_publish)
create_vlan = vinet.index("ret = create_vlan_iface(", viflast_gate)
require(vinet.index("ret = remove_vlan_iface(vinet_iflast);") < viflast_publish <
        viflast_gate < create_vlan,
        "WAN VLAN cleanup identity must be checked and recorded after old VLAN removal but before creating its replacement")
ifname_publish = vinet.index('ret = set_wan_unit_value_status(0, "ifname", vinet_ifname);')
ifname_gate = vinet.index("if (ret != 0)", ifname_publish)
require(create_vlan < ifname_publish < ifname_gate < vinet.index("return 0;", ifname_gate) and
        'set_wan_unit_value(0, "ifname", vinet_ifname)' not in vinet,
        "WAN activation interface identity must be checked after VLAN/bridge/IPv6 setup and before returning success")
wisp = body(wan, "config_apcli_wisp(void)")
wisp_publish = wisp.index('publication_status = set_wan_unit_value_status(0, "ifname", wisp_ifname);')
wisp_gate = wisp.index("if (publication_status != 0)", wisp_publish)
require(wisp_publish < wisp_gate < wisp.index("return 0;", wisp_gate) and
        'set_wan_unit_value(0, "ifname", wisp_ifname)' not in wisp,
        "AP-client WISP migration must fail closed when its active interface identity cannot be published")
viptv = body(wan, "launch_viptv_wan(void)")
iptv_start = viptv.index("/* VLAN interface for IPTV */")
iptv_setup = viptv[iptv_start:]
mode_read_pos = viptv.index('ret = get_nvram_int_checked_status("viptv_mode", &viptv_mode);')
mode_error_gate_pos = viptv.index("if (ret != 0)", mode_read_pos)
mode_error_return_pos = viptv.index("return ret;", mode_error_gate_pos)
static_mode_gate_pos = viptv.index("if (viptv_mode == 2)", mode_error_return_pos)
static_addr_read_pos = viptv.index('"viptv_ipaddr", viptv_addr_setting,', static_mode_gate_pos)
static_mask_read_pos = viptv.index('"viptv_netmask",', static_addr_read_pos)
static_pair_error_pos = viptv.index("if (ret != 0)", static_mask_read_pos)
static_pair_return_pos = viptv.index("return ret;", static_pair_error_pos)
static_validation_pos = viptv.index("if (!is_valid_ipv4(viptv_addr_setting)", static_pair_return_pos)
static_addr_copy_pos = viptv.index("strcpy(viptv_addr, viptv_addr_setting);", static_validation_pos)
static_mask_copy_pos = viptv.index("strcpy(viptv_mask, viptv_mask_setting);", static_addr_copy_pos)
static_gateway_read_pos = viptv.index('"viptv_gateway", viptv_gate_setting,', static_mask_copy_pos)
static_gateway_error_pos = viptv.index("if (ret != 0)", static_gateway_read_pos)
static_gateway_return_pos = viptv.index("return ret;", static_gateway_error_pos)
static_gateway_copy_pos = viptv.index("strcpy(viptv_gate, viptv_gate_setting);", static_gateway_return_pos)
old_vlan_removal_pos = viptv.index("/* Remove a no-longer-used VLAN", static_gateway_copy_pos)
require(mode_read_pos < mode_error_gate_pos < mode_error_return_pos <
        static_mode_gate_pos < static_addr_read_pos < static_mask_read_pos <
        static_pair_error_pos < static_pair_return_pos < static_validation_pos <
        static_addr_copy_pos < static_mask_copy_pos < static_gateway_read_pos <
        static_gateway_error_pos < static_gateway_return_pos <
        static_gateway_copy_pos < old_vlan_removal_pos,
        "static IPTV mode/address/mask/gateway must be checked, copied, validated and committed to local buffers before VLAN teardown")
for key in ("viptv_ipaddr", "viptv_netmask", "viptv_gateway"):
    assert f'nvram_safe_get("{key}")' not in viptv

snapshot_pos = iptv_setup.index("vlan_flags = get_interface_flags(viptv_ifname);")
create_pos = iptv_setup.index("ret = create_vlan_iface_tracked(")
bridge_pos = iptv_setup.index("ret = config_soft_bridge_wan_internal(")
rp_filter_pos = iptv_setup.index('ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);')
old_vlan_branch = iptv_setup.index("/* Remove a no-longer-used VLAN")
old_vlan_replace = iptv_setup.index("ret = wan_viptv_ifname_remove_old_vlan(", old_vlan_branch)
old_vlan_plain_remove = iptv_setup.index("ret = remove_vlan_iface(viptv_iflast);", old_vlan_branch)
old_vlan_policy = body(viptv_policy, "wan_viptv_ifname_remove_old_vlan(")
publish_pos = iptv_setup.index("ret = wan_viptv_ifname_publish_or_rollback(", rp_filter_pos)
require(snapshot_pos < old_vlan_branch < old_vlan_replace < old_vlan_plain_remove < create_pos < bridge_pos < rp_filter_pos < publish_pos,
        "IPTV startup must snapshot replacement VLAN ownership, clear old VLAN identity before removal, then commit the new identity after setup")
require(old_vlan_policy.index("ops->clear_identity(context)") <
        old_vlan_policy.index("ops->remove_vlan(context)") <
        old_vlan_policy.index("ops->restore_identity(context)"),
        "old IPTV VLAN removal must restore its recovery identity if interface removal fails")
for operation in ("ret = create_vlan_iface_tracked(",
                  "ret = config_soft_bridge_wan_internal(",
                  'ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);'):
    operation_pos = iptv_setup.index(operation)
    error_gate = iptv_setup.index("if (ret != 0)", operation_pos)
    rollback_call = iptv_setup.index("rollback_status = rollback_viptv_vlan_setup(&rollback_entry);",
                                     error_gate)
    error_return = iptv_setup.index("return ret;", rollback_call)
    require(operation_pos < error_gate < rollback_call < error_return < publish_pos,
            f"IPTV setup must transactionally roll back before returning after {operation}")
ownership_struct_pos = wan.index("struct soft_bridge_wan_ownership {")
other_entry_struct_pos = wan.index("struct soft_bridge_other_entry {")
gmac_guard_pos = wan.index("#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)")
require(ownership_struct_pos < other_entry_struct_pos < gmac_guard_pos,
        "IPTV rollback ownership records must be declared for all board configurations")
require("}\n#endif\n\nstatic int\nwan_viptv_ifname_publish_status" in wan and
        wan.index("static int\nwan_viptv_ifname_publish_status") <
        wan.index("static int\nrollback_viptv_vlan_setup"),
        "GMAC-only bridge batch must close its preprocessor guard before checked IPTV identity publication and rollback adapters")
viptv_rollback = body(wan, "rollback_viptv_vlan_setup(struct soft_bridge_other_entry *entry)")
require("#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)" in viptv_rollback and
        "rollback_soft_bridge_other_entry(entry);" in viptv_rollback and
        "entry->port_vlan_created" in viptv_rollback and
        "remove_vlan_iface(entry->port_ifname)" in viptv_rollback and
        "wan_viptv_ifname_publish_status(" in viptv_rollback and
        "failed to publish IPTV VLAN recovery interface identity" in viptv_rollback and
        "entry->bridge.bridge_existed" in viptv_rollback and
        "get_interface_flags(entry->bridge.bridge_name)" in viptv_rollback and
        "Boards without GMAC2 bridge support" in viptv_rollback,
        "IPTV rollback must compile on GMAC2 and VLAN-only boards and retain only surviving recovery interfaces")
require("char viptv_ifname[24], viptv_addr[16], viptv_mask[16], viptv_gate[16];" in viptv and
        "strcpy(viptv_addr, viptv_addr_setting);" in viptv and
        "strcpy(viptv_mask, viptv_mask_setting);" in viptv and
        "strcpy(viptv_gate, viptv_gate_setting);" in viptv and
        "viptv_addr = nvram_safe_get" not in viptv and
        "viptv_mask = nvram_safe_get" not in viptv and
        "viptv_gate = nvram_safe_get" not in viptv,
        "static IPTV callbacks must use bounded local copies, not borrowed NVRAM pointers")

ifname_publish = body(wan, "wan_viptv_ifname_publish_status(const char *ifname)")
require('nvram_set_temp("viptv_ifname", ifname)' in ifname_publish and
        "rc_wan_nvram_write_status(api_result, saved_errno)" in ifname_publish,
        "IPTV recovery interface identity must use the checked NVRAM write status")
ifname_publish_policy = body(viptv_policy, "wan_viptv_ifname_publish_or_rollback(")
require("status = ops->publish(context);" in ifname_publish_policy and
        "rollback_status = ops->rollback_vlan(context);" in ifname_publish_policy and
        "return route_status_accumulate(status, rollback_status);" in ifname_publish_policy,
        "IPTV interface identity publication failure must trigger VLAN rollback")
require("wan_viptv_ifname_publish_or_rollback(" in iptv_setup and
        "VLAN rollback attempted" in iptv_setup,
        "new IPTV VLAN must be rolled back if its recovery identity cannot be committed")
for case in ("test_ifname_publish_success_keeps_vlan_and_skips_rollback",
             "test_ifname_publish_failure_rolls_back_vlan_and_preserves_primary_error",
             "test_ifname_rollback_failure_does_not_mask_publish_error",
             "test_ifname_publish_policy_rejects_incomplete_callbacks"):
    require(case in (rc / "tests/wan_viptv_static_policy.c").read_text(),
            "IPTV interface identity policy regression missing " + case)

snapshot_absolute = iptv_start + snapshot_pos
old_vlan_branch = iptv_setup.index("/* Remove a no-longer-used VLAN")
old_vlan_replace = iptv_setup.index("ret = wan_viptv_ifname_remove_old_vlan(", old_vlan_branch)
old_vlan_plain_remove = iptv_setup.index("ret = remove_vlan_iface(viptv_iflast);", old_vlan_branch)
old_vlan_policy = body(viptv_policy, "wan_viptv_ifname_remove_old_vlan(")
require(snapshot_pos < old_vlan_branch < old_vlan_replace < old_vlan_plain_remove < create_pos,
        "snapshot replacement VLAN before retiring the old VLAN identity and removing its interface")
require(old_vlan_policy.index("ops->clear_identity(context)") <
        old_vlan_policy.index("ops->remove_vlan(context)") <
        old_vlan_policy.index("ops->restore_identity(context)"),
        "old IPTV VLAN removal must clear identity first and restore it if interface removal fails")
restart = body(wan, "full_restart_wan(void)")
lan_bridge_call = restart.index("status = config_soft_bridge_lan();")
lan_bridge_gate = restart.index("if (status != 0)", lan_bridge_call)
lan_bridge_log = restart.index("LAN bridge VLAN setup failed (status=%d)", lan_bridge_gate)
lan_bridge_return = restart.index("return status;", lan_bridge_log)
wan_restart = restart.index("status = start_wan();")
require(lan_bridge_call < lan_bridge_gate < lan_bridge_log < lan_bridge_return < wan_restart,
        "full WAN restart must preserve LAN-side VLAN bridge setup failure before WAN startup")

push, pr = workflow.split("  pull_request:", 1)
pr = pr.split("\njobs:", 1)[0]
for path in ("trunk/user/shared/src/netutils.c",
             "trunk/user/shared/src/netutils.h",
             "trunk/user/rc/src/vlan_interface_policy.h",
             "trunk/user/rc/src/tests/vlan_interface_policy.c",
             "trunk/user/shared/src/bridge_membership_policy.h",
             "trunk/user/shared/src/tests/bridge_membership_policy.c",
             "trunk/user/rc/src/net_wan.c",
             "scripts/check_wan_vlan_setup.py"):
    require(push.count(f'"{path}"') == 1 and pr.count(f'"{path}"') == 1,
            f"WAN workflow push/PR paths must include {path}")
require("python3 scripts/check_wan_vlan_setup.py" in workflow,
        "WAN workflow must execute the VLAN/bridge source contract")

viptv_test = (rc / "tests/wan_viptv_static_policy.c").read_text()
viptv_test_main = body(viptv_test, "main(void)")
for case in ("test_old_vlan_metadata_is_cleared_before_vlan_removal",
             "test_old_vlan_clear_failure_preserves_vlan_and_skips_removal",
             "test_old_vlan_remove_failure_restores_recovery_identity",
             "test_old_vlan_restore_failure_preserves_remove_error",
             "test_old_vlan_transition_rejects_incomplete_callbacks"):
    require(case in viptv_test and case + "();" in viptv_test,
            "old IPTV VLAN replacement regression missing " + case)
for case in ("test_old_vlan_metadata_is_cleared_before_vlan_removal",
             "test_old_vlan_clear_failure_preserves_vlan_and_skips_removal",
             "test_old_vlan_remove_failure_restores_recovery_identity",
             "test_old_vlan_restore_failure_preserves_remove_error",
             "test_old_vlan_transition_rejects_incomplete_callbacks"):
    require(case in viptv_test and case + "();" in viptv_test,
            "old IPTV VLAN replacement regression missing " + case)
static_multicast = body(wan, "wan_viptv_static_start_multicast(void *opaque)")
require(("static int" + chr(10) + "wan_viptv_static_start_multicast") in wan and
        "return start_igmpproxy(context->ifname);" in static_multicast,
        "static IPTV must return its IGMP proxy startup result")
static_start = body(viptv_policy, "wan_viptv_static_start(")
static_publish = body(wan, "wan_viptv_static_publish_snapshot(void *opaque)")
static_clear = body(wan, "wan_viptv_static_clear_address(void *opaque)")
require("int (*publish_snapshot)(void *context);" in viptv_policy and
        "int (*clear_address)(void *context);" in viptv_policy and
        "status = ops->publish_snapshot(context);" in static_start and
        "rollback_status = ops->clear_address(context);" in static_start and
        static_start.index("ops->publish_snapshot(context)") <
        static_start.index("ops->clear_address(context)") <
        static_start.index("ops->add_default_route(context)"),
        "static IPTV must clear its configured address and stop route/multicast startup after publication failure")
for key in ("manv_ipaddr", "manv_netmask", "manv_gateway"):
    require('"' + key + '"' in static_publish,
            "static IPTV snapshot is missing " + key)
require("wan_dhcp_nvram_publish_values(names, values, 3)" in static_publish and
        'ifconfig(context->ifname, IFUP, "0.0.0.0", NULL)' in static_clear,
        "static IPTV address/mask/gateway publication must be compensatable and have address cleanup")
require("multicast_status = ops->start_multicast(context);" in static_start and
        "return route_status_accumulate(status, multicast_status);" in static_start,
        "static IPTV policy must aggregate proxy status and still run multicast after route failure")
for test_name in ("test_snapshot_failure_clears_address_and_skips_routes_and_multicast",
                  "test_snapshot_failure_preserves_primary_error_when_address_clear_fails",
                  "test_multicast_failure_is_returned_without_gateway",
                  "test_first_route_error_is_preserved_when_multicast_also_fails"):
    require(test_name in viptv_test and test_name + "();" in viptv_test,
            f"static IPTV regression missing {test_name}")
for signature, operation, description in (
    ("man_up_internal(char *man_ifname, int unit, int is_static, int *gateway_route_status)", "start_igmpproxy(man_ifname)", "MAN startup"),
    ("man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)", "stop_igmpproxy(man_ifname)", "MAN teardown"),
    ("wan_up_internal(char *wan_ifname, int unit, int is_static)", "start_igmpproxy(wan_ifname)", "WAN startup"),
    ("wan_down_internal(char *wan_ifname, int unit, int is_static)", "stop_igmpproxy(wan_ifname)", "WAN teardown"),
):
    function = body(wan, signature)
    require(f"route_status = {operation};" in function and
            "status = route_status_accumulate(status, route_status);" in function,
            f"{description} must preserve IGMP proxy status")
dynamic_policy = body(viptv_policy, "wan_viptv_dynamic_start_with_rollback(")
dynamic_policy_code = " ".join(dynamic_policy.split())
require(dynamic_policy_code.index("ops->start(context)") <
        dynamic_policy_code.index("ops->stop_client(context)") <
        dynamic_policy_code.index("ops->rollback_vlan(context)") and
        "if (result->stop_status == 0)" in dynamic_policy_code and
        "errno = start_errno;" in dynamic_policy_code,
        "dynamic IPTV must stop the child before VLAN rollback and preserve the primary error")
dynamic_start_callback = body(wan, "wan_viptv_dynamic_start_callback(void *opaque)")
dynamic_stop_callback = body(wan, "wan_viptv_dynamic_stop_callback(void *opaque)")
dynamic_rollback_callback = body(wan, "wan_viptv_dynamic_rollback_callback(void *opaque)")
require("start_zcip_viptv(" in dynamic_start_callback and
        "start_udhcpc_viptv(" in dynamic_start_callback and
        "stop_udhcpc_viptv()" in dynamic_stop_callback and
        '"zcip"' in dynamic_stop_callback and
        "kill_services(services, 3, 1)" in dynamic_stop_callback and
        "rollback_viptv_vlan_setup(context->rollback_entry)" in dynamic_rollback_callback,
        "dynamic IPTV cleanup must use client stop and owned VLAN rollback")
dynamic_launch = body(wan, "launch_viptv_wan(void)")
require(dynamic_launch.index("wan_viptv_ifname_publish_or_rollback(") <
        dynamic_launch.index("wan_viptv_dynamic_start_with_rollback(") and
        "dynamic_context.rollback_entry = &rollback_entry;" in dynamic_launch and
        "retaining VLAN recovery identity" in dynamic_launch,
        "IPTV must publish recovery identity before starting and retain it on uncertain stop")
for case in (
    "test_dynamic_start_success_does_not_stop_or_rollback",
    "test_dynamic_start_failure_stops_before_vlan_rollback",
    "test_dynamic_stop_failure_preserves_vlan_recovery_handle",
    "test_dynamic_rollback_failure_preserves_start_error",
    "test_dynamic_start_rejects_incomplete_callbacks",
):
    require(case in viptv_test and case + "();" in viptv_test_main,
            "dynamic IPTV regression missing or unregistered: " + case)

iptv_dhcp = body(wan, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)")
require("multicast_status = start_igmpproxy(man_ifname);" in iptv_dhcp and
        "callback_status = route_status_accumulate(route_setup_status, multicast_status);" in iptv_dhcp and
        'wan_nvram_int_temp_status("manv_route_err", route_setup_status)' in iptv_dhcp and
        'wan_nvram_int_temp_status("manv_err", i_err)' in iptv_dhcp and
        "callback_status = route_status_accumulate(callback_status, publish_status);" in iptv_dhcp and
        "return callback_status;" in iptv_dhcp and
        "nvram_set_int_temp(" not in iptv_dhcp,
        "IPTV DHCP must return proxy and NVRAM status while keeping route retry state independent")
iptv_zcip = body(wan, "zcip_viptv_bound(char *man_ifname)")
require("return start_igmpproxy(man_ifname);" in iptv_zcip,
        "ZeroConf IPTV bound must return proxy startup status")
iptv_zcip_down = body(wan, "zcip_viptv_deconfig(char *man_ifname)")
normalized_iptv_zcip_down = " ".join(iptv_zcip_down.split())
require("multicast_status = stop_igmpproxy(man_ifname);" in iptv_zcip_down and
        "return route_status_accumulate( route_status_accumulate( route_status_accumulate(interface_status, route_status), metadata_status), multicast_status);" in normalized_iptv_zcip_down,
        "ZeroConf IPTV deconfig must return proxy stop failure")
for path in ("trunk/user/rc/src/wan_viptv_static_policy.h",
             "trunk/user/rc/src/tests/wan_viptv_static_policy.c"):
    require(path in workflow, f"WAN workflow path missing {path}")

print("WAN VLAN/bridge and IGMP proxy result-propagation contracts: PASS")


# DHCP dynamic-route selector reads must preserve backend errors in both producers and route consumer.
route_add = body(wan, "control_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms")
wan_dhcp = body(wan, "start_udhcpc_wan(char *wan_ifname, int unit, int wait_lease)")
iptv_dhcp = body(wan, "start_udhcpc_viptv(char *man_ifname)")
route_add_code = " ".join(route_add.split())
wan_dhcp_code = " ".join(wan_dhcp.split())
iptv_dhcp_code = " ".join(iptv_dhcp.split())
checked_dr_read = 'get_nvram_match_checked_status( "dr_enable_x", "1", &dr_enable_value)'
for name, code in (("route-add", route_add_code),
                   ("WAN DHCP", wan_dhcp_code),
                   ("IPTV DHCP", iptv_dhcp_code)):
    require(checked_dr_read in code and
            "if (dr_enable_status != 0)" in code and
            "return dr_enable_status;" in code and
            'nvram_match("dr_enable_x", "1")' not in code,
            f"{name} must preserve dynamic-route NVRAM read errors")
require(route_add_code.index("if (is_add)") < route_add_code.index(checked_dr_read) <
        route_add_code.index("if (!dr_enable_value)") and
        "if (!is_add)" not in route_add_code[:route_add_code.index(checked_dr_read)],
        "DHCP route cleanup must remain independent of the dynamic-route add selector")
for code in (wan_dhcp_code, iptv_dhcp_code):
    require(code.index(checked_dr_read) < code.index("if (dr_enable_value)"),
            "DHCP clients must check the dynamic-route selector before adding option requests")

# Protocol identity is a required reset input, not a lossy legacy nvram_safe_get().
wan_proto_reader = body(
    wan,
    "get_wan_unit_proto_checked_status(int unit, int *wan_proto)"
)
wan_proto_reader_code = " ".join(wan_proto_reader.split())
require('get_wan_unit_value_copy_status( unit, "proto", proto_text, sizeof(proto_text))' in wan_proto_reader_code and
        all(f'strcmp(proto_text, "{proto}") == 0' in wan_proto_reader_code
            for proto in ("pppoe", "pptp", "l2tp", "dhcp", "static")) and
        "return EINVAL;" in wan_proto_reader_code,
        "WAN reset protocol parser must use checked text and reject unknown values")
wan_reset = body(wan, "reset_wan_vars_status(void)")
wan_reset_code = " ".join(wan_reset.split())
checked_proto_read = "get_wan_unit_proto_checked_status(unit, &wan_proto)"
require(checked_proto_read in wan_reset_code and
        'reset_failure_context = "WAN protocol snapshot";' in wan_reset_code and
        wan_reset_code.index(checked_proto_read) <
        wan_reset_code.index("if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)") and
        "wan_proto = get_wan_proto(unit);" not in wan_reset_code,
        "WAN reset must abort on protocol-read errors before branching on protocol")

# WAN interface identity must not collapse custom/factory NVRAM read failures.
wan_mac_reader = body(
    wan,
    "wan_hardware_address_copy_status(char *hardware_address,"
)
wan_mac_reader_code = " ".join(wan_mac_reader.split())
custom_mac_read = 'wan_nvram_checked_get_callback( NULL, "wan_hwaddr_x", &custom_mac)'
fallback_mac_read = 'rc_wan_nvram_copy_value( wan_nvram_checked_get_callback, NULL, "il1macaddr", factory_mac, sizeof(factory_mac))'
require(custom_mac_read in wan_mac_reader_code and
        'rc_wan_nvram_format_mac_override( custom_mac, formatted_mac, sizeof(formatted_mac))' in wan_mac_reader_code and
        fallback_mac_read in wan_mac_reader_code and
        wan_mac_reader_code.index(custom_mac_read) <
        wan_mac_reader_code.index(fallback_mac_read) and
        "if (formatted_mac[0] != '\\0')" in wan_mac_reader_code,
        "WAN custom MAC must be checked and retain legacy factory fallback")
require('wan_hardware_address_copy_status( mac_buf, sizeof(mac_buf))' in wan_reset_code and
        'reset_failure_context = "WAN hardware address configuration";' in wan_reset_code and
        wan_reset_code.index("wan_hardware_address_copy_status(") <
        wan_reset_code.index('set_nvram_persistent_value_status("wan_hwaddr", wan_hwaddr)') and
        'mac_conv("wan_hwaddr_x"' not in wan_reset_code and
        'nvram_safe_get("il1macaddr")' not in wan_reset_code,
        "WAN reset must check MAC source reads before persistent/runtime identity publication")

# Manual WAN underlay selectors and endpoint values are required startup inputs.
manual_snapshot = body(
    wan,
    "wan_manual_endpoint_snapshot(char *wan_addr, size_t wan_addr_size"
)
manual_snapshot_code = " ".join(manual_snapshot.split())
manual_keys = ("wan_ipaddr", "wan_netmask", "wan_gateway", "wan_mtu")
manual_key_positions = [
    manual_snapshot_code.index(f'fields[{index}].param_name = "{key}"')
    for index, key in enumerate(manual_keys)
]
require(manual_key_positions == sorted(manual_key_positions) and
        "return rc_wan_nvram_copy_snapshot(" in manual_snapshot_code and
        "fields, 4, wan_nvram_transaction_begin_callback" in manual_snapshot_code and
        "wan_nvram_transaction_get_callback" in manual_snapshot_code and
        "wan_nvram_transaction_end_callback" in manual_snapshot_code,
        "manual WAN underlay address/netmask/gateway/MTU must be one checked transactional snapshot")

wan_reset = body(wan, "reset_wan_vars_status(void)")
wan_reset_code = " ".join(wan_reset.split())
manual_mode_read = 'get_nvram_match_checked_status( "x_DHCPClient", "0", &manual_endpoint_enabled)'
manual_pppoe_read = 'get_nvram_match_checked_status( "wan_pppoe_man", "1", &manual_pppoe_enabled)'
require(manual_mode_read in wan_reset_code and
        manual_pppoe_read in wan_reset_code and
        "if (proto_status != 0)" in wan_reset_code and
        'reset_failure_context = "manual WAN address-mode selector";' in wan_reset_code and
        'reset_failure_context = "manual PPPoE selector";' in wan_reset_code,
        "manual WAN selector backend errors must abort WAN reset instead of selecting empty defaults")
require(wan_reset_code.index(manual_mode_read) <
        wan_reset_code.index("wan_manual_endpoint_snapshot(") <
        wan_reset_code.index('set_wan_unit_value_status(unit, "man_ipaddr", man_addr)'),
        "manual WAN settings must be snapshotted before publishing the unit endpoint")
for lossy_read in (
    'nvram_match("x_DHCPClient", "0")',
    'nvram_match("wan_pppoe_man", "1")',
    'nvram_safe_get("wan_ipaddr")',
    'nvram_safe_get("wan_netmask")',
    'nvram_safe_get("wan_gateway")',
    'nvram_safe_get("wan_mtu")',
):
    require(lossy_read not in wan_reset_code,
            f"manual WAN reset must not collapse NVRAM errors for {lossy_read}")
