#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    i = opening
    while i < len(source):
        if source.startswith("//", i):
            end = source.find("\n", i + 2)
            i = len(source) if end < 0 else end + 1
            continue
        if source.startswith("/*", i):
            end = source.find("*/", i + 2)
            i = len(source) if end < 0 else end + 2
            continue
        char = source[i]
        if char in ("'", '"'):
            quote = char
            i += 1
            while i < len(source):
                if source[i] == "\\":
                    i += 2
                    continue
                if source[i] == quote:
                    i += 1
                    break
                i += 1
            continue
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:i]
        i += 1
    raise AssertionError("unterminated function: " + signature)


netutils = (ROOT / "trunk/user/shared/src/netutils.c").read_text()
netutils_h = (ROOT / "trunk/user/shared/src/netutils.h").read_text()
net6 = (ROOT / "trunk/user/rc/src/net6.c").read_text()
wan6 = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
rc = (ROOT / "trunk/user/rc/src/rc.c").read_text()
net_lan6 = (ROOT / "trunk/user/rc/src/net_lan6.c").read_text()
net_lan = (ROOT / "trunk/user/rc/src/net_lan.c").read_text()
storage = (ROOT / "trunk/user/rc/src/services_stor.c").read_text()

reader = function_body(netutils, "int get_ipv6_type_status(")
assert 'nvram_get_checked("ip6_service", &service_value)' in reader
assert "ipv6_service_type_parse_status(" in reader
assert "nvram_safe_get(" not in reader
legacy = function_body(netutils, "int get_ipv6_type(void)")
assert "get_ipv6_type_status(&ipv6_type)" in legacy
assert "errno = status;" in legacy
assert "get_ipv6_type_status(int *ipv6_type)" in netutils_h

init = function_body(net6, "int init_ipv6(void)")
ir = init.index("get_ipv6_type_status(")
ie = init.index("if (ipv6_type_status != 0)", ir)
isys = init.index("control_if_ipv6_all(", ie)
igai = init.index("set_libc_gai(", ie)
assert ir < ie < isys < igai
assert "return ipv6_type_status;" in init[ie:isys]

restart = function_body(net6, "full_restart_ipv6(int ipv6_type_old)")
rr = restart.index("get_ipv6_type_status(")
re = restart.index("if (ipv6_type_status != 0)", rr)
rl = restart.index("dnsmasq_lifecycle_lock_acquire(", re)
assert rr < re < rl

up = function_body(wan6, "int wan6_up(char *wan_ifname, int unit)")
ur = up.index("get_ipv6_type_status(")
ue = up.index("if (status != 0)", ur)
us = up.index("stop_dhcp6c(", ue)
assert ur < ue < us and "return status;" in up[ue:us]

down = function_body(wan6, "int wan6_down(char *wan_ifname, int unit)")
ds = down.index("stop_dhcp6c(")
dr = down.index("get_ipv6_type_status(", ds)
de = down.index("if (status != 0)", dr)
di = down.index('get_wan_unit_value(unit, "ifname6")', de)
assert ds < dr < de < di and "return status;" in down[de:di]

callback = function_body(wan6, "int dhcp6c_main(int argc, char **argv)")
cr = callback.index("get_ipv6_type_status(")
ce = callback.index("if (settings_status != 0)", cr)
cu = callback.index("dhcp6c_callback_session_unlock(callback_fd)", ce)
cf = callback.index("wan6_read_dhcp_integer_snapshot(", ce)
assert cr < ce < cu < cf and "return -1;" in callback[ce:cf]

restore = function_body(rc, "nvram_restore_defaults(void)")
ri = restore.index("get_ipv6_type_status(&nvram_ipv6_type)")
assert "return -1;" in restore[ri:]
assert "ipv6_disable_required = (nvram_ipv6_type == IPV6_DISABLED);" in rc
assert "type_read_status = get_ipv6_type_status(&updated_ipv6_type);" in rc
assert 'nvram_safe_get("ip6_service")' not in netutils


lan6_reader = function_body(net_lan6, "get_lan6_service_type(")
assert "get_ipv6_type_status(ipv6_type)" in lan6_reader
assert "if (status != 0)" in lan6_reader
assert "errno = status;" in lan6_reader
assert "return status;" in lan6_reader
assert "get_ipv6_type()" not in net_lan6

reset_lan6 = function_body(net_lan6, "int reset_lan6_vars(void)")
service_read = reset_lan6.index("get_lan6_service_type(&ipv6_type)")
service_error = reset_lan6.index("if (service_status != 0)", service_read)
service_return = reset_lan6.index("return service_status;", service_error)
lan_config_read = reset_lan6.index('nvram_safe_get("ip6_lan_addr")')
lan_publish = reset_lan6.index('nvram_set_temp("lan_addr6", addr6s)')
assert service_read < service_error < service_return < lan_config_read < lan_publish
assert "lan_addr6_static_for_type(ipv6_type)" in reset_lan6

for signature in (
    "int is_lan_radv_on(void)",
    "int is_lan_addr6_static(void)",
    "int get_lan_dhcp6s_mode(void)",
):
    getter = function_body(net_lan6, signature)
    read = getter.index("get_lan6_service_type(&ipv6_type)")
    error = getter.index("if (status != 0", read)
    assert read < error
    assert "return -1;" in getter[error:]
assert "lan_addr6_static_for_type(ipv6_type)" in function_body(
    net_lan6, "int is_lan_addr6_static(void)"
)


firewall = (ROOT / "trunk/user/rc/src/firewall_ex.c").read_text()
ipv4_filter = function_body(firewall, "ipt_filter_rules(char *man_if")
ipv4_read = ipv4_filter.index("get_ipv6_type_status(&ipv6_type)")
ipv4_error = ipv4_filter.index("if (ipv6_type_status != 0)", ipv4_read)
ipv4_file = ipv4_filter.index('firewall_open_rule_file(ipt_file, "IPv4 filter")')
assert ipv4_read < ipv4_error < ipv4_file
assert "return ipv6_type_status;" in ipv4_filter[ipv4_error:ipv4_file]
assert "get_ipv6_type()" not in ipv4_filter

ipv6_filter = function_body(firewall, "ip6t_filter_rules(char *man_if")
ipv6_read = ipv6_filter.index("get_ipv6_type_status(&ipv6_type)")
ipv6_error = ipv6_filter.index("if (ret != 0)", ipv6_read)
ipv6_file = ipv6_filter.index('firewall_open_rule_file(ipt_file, "IPv6 filter")')
assert ipv6_read < ipv6_error < ipv6_file
assert "return ret;" in ipv6_filter[ipv6_error:ipv6_file]
assert "get_ipv6_type()" not in ipv6_filter

nat_rules = function_body(firewall, "ipt_nat_rules(char *man_if")
nat_read = nat_rules.index("get_ipv6_type_status(&ipv6_type)")
nat_guard = nat_rules.rfind("if (is_use_dmz) {", 0, nat_read)
nat_error = nat_rules.index("if (firewall_status != 0)", nat_read)
nat_file = nat_rules.index('firewall_open_rule_file(ipt_file, "IPv4 NAT")')
assert 0 <= nat_guard < nat_read < nat_error < nat_file
assert "return firewall_status;" in nat_rules[nat_error:nat_file]
assert "get_ipv6_type()" not in nat_rules


lan_hosts = function_body(net_lan, "create_hosts_lan(")
hosts_read = lan_hosts.index("get_ipv6_type_status(&ipv6_type)")
hosts_error = lan_hosts.index("if (ipv6_type_status != 0)", hosts_read)
hosts_return = lan_hosts.index("return ipv6_type_status;", hosts_error)
hosts_open = lan_hosts.index("dnsmasq_file_open(hostname_temp)")
hosts_ipv6 = lan_hosts.index("if (ipv6_type != IPV6_DISABLED)")
assert hosts_read < hosts_error < hosts_return < hosts_open < hosts_ipv6
assert "get_ipv6_type()" not in lan_hosts

lan_start = function_body(net_lan, "start_lan(int is_ap_mode")
start_read = lan_start.index("get_ipv6_type_status(&ipv6_type)")
start_error = lan_start.index("if (ipv6_type_status != 0)", start_read)
start_reload = lan_start.index("reload_lan_addr6()", start_error)
start_aggregate = lan_start.index(
    "start_status = rc_first_service_start_status(start_status, operation_status);",
    start_reload,
)
start_bridge = lan_start.index("config_bridge(is_ap_mode)", start_aggregate)
assert start_read < start_error < start_reload < start_aggregate < start_bridge
assert "operation_status = ipv6_type_status;" in lan_start[start_error:start_reload]
assert "get_ipv6_type()" not in lan_start


ftp_config = function_body(storage, "write_vsftpd_conf(void)")
ftp_read = ftp_config.index("get_ipv6_type_status(&ipv6_type)")
ftp_error = ftp_config.index("if (ipv6_type_status != 0)", ftp_read)
ftp_return = ftp_config.index("return -1;", ftp_error)
ftp_stage = ftp_config.index("ftp_config_file_open(", ftp_return)
ftp_listener = ftp_config.index("if (ipv6_type != IPV6_DISABLED)")
assert ftp_read < ftp_error < ftp_return < ftp_stage < ftp_listener
assert "get_ipv6_type()" not in ftp_config

nfs_exports = function_body(storage, "write_nfsd_exports(void)")
nfs_override = nfs_exports.index("if (exports_override_state == 1)")
nfs_read = nfs_exports.index("get_ipv6_type_status(&ipv6_type)")
nfs_error = nfs_exports.index("if (ipv6_type_status != 0)", nfs_read)
nfs_return = nfs_exports.index("return -1;", nfs_error)
nfs_mounts = nfs_exports.index("procpt = fopen(PROC_MOUNTS_FILE")
nfs_acl = nfs_exports.index("if (ipv6_type != IPV6_DISABLED)", nfs_mounts)
assert nfs_override < nfs_read < nfs_error < nfs_return < nfs_mounts < nfs_acl
assert "get_ipv6_type()" not in nfs_exports

print("IPv6 service-type checked-read source contract: PASS")
