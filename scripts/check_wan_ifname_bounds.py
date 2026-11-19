#!/usr/bin/env python3
"""Check bounded WAN interface-name handling at every RC call site."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"

policy = (RC / "wan_ifname_policy.h").read_text()
net_wan = (RC / "net_wan.c").read_text()
services = (RC / "services_ex.c").read_text()
firewall = (RC / "firewall_ex.c").read_text()
rc_header = (RC / "rc.h").read_text()

assert "wan_ifname_copy(char *destination, size_t destination_size" in policy
assert "length == destination_size" in policy
assert "c == '_' || c == '-' || c == '.'" in policy

wan_start = net_wan.index("get_wan_ifname(char wan_ifname[16])")
wan_end = net_wan.index("\n}\n", wan_start)
wan_get = net_wan[wan_start:wan_end]
assert "wan_ifname_copy(wan_ifname, 16, ifname)" in wan_get
assert "strcpy(wan_ifname" not in wan_get
assert "int get_wan_ifname(char wan_ifname[16]);" in rc_header

assert "ret = get_wan_ifname(wan_ifname);" in services
assert "miniupnpd start aborted: invalid WAN interface name" in services
assert "config.wan_ifname = wan_ifname;" in services
assert "interface_status = get_wan_ifname(wan_ifname);" in services
assert "wan_ifname_copy(wan_ifname, sizeof(wan_ifname), management_ifname)" in services
assert "strcpy(wan_ifname, get_man_ifname(0))" not in services
assert services.count("if (interface_status != 0)") >= 2

assert "status = get_wan_ifname(wan_if);" in firewall
assert "No safe WAN interface token; refusing firewall rebuild (status=%d)" in firewall
assert "if (status != 0)" in firewall
print("WAN interface-name bounded-copy integration contract: PASS")
