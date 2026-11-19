#!/usr/bin/env python3
"""Check bounded WAN interface-name handling at every RC call site."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"

policy = (RC / "wan_ifname_policy.h").read_text()
net_wan = (RC / "net_wan.c").read_text()
services = (RC / "services_ex.c").read_text()
firewall = (RC / "firewall_ex.c").read_text()
network = (RC / "net.c").read_text()
ppp = (RC / "net_ppp.c").read_text()
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
services_compact = " ".join(services.split())
assert "wan_ifname_copy(wan_ifname, sizeof(wan_ifname), management_ifname)" in services_compact
assert "strcpy(wan_ifname, get_man_ifname(0))" not in services
assert services.count("if (interface_status != 0)") >= 2

assert "status = get_wan_ifname(wan_if);" in firewall
assert "No safe WAN interface token; refusing firewall rebuild (status=%d)" in firewall
assert "if (status != 0)" in firewall

assert '#include "wan_ifname_policy.h"' in ppp
ppp_start = ppp.index("launch_wan_pppd_locked(int unit, int wan_proto)")
ppp_launch = ppp[ppp_start:]
assert "char pppoe_ifname[16];" in ppp_launch
ppp_compact = " ".join(ppp_launch.split())
assert "wan_ifname_copy(pppoe_ifname, sizeof(pppoe_ifname), get_man_ifname(unit))" in ppp_compact
assert ppp_launch.index("wan_ifname_copy(pppoe_ifname") < ppp_launch.index("ppp_wan_session_generation_change(")
assert ppp_launch.index("wan_ifname_copy(pppoe_ifname") < ppp_launch.index("ppp_options_file_open(options)")
assert 'fprintf(fp, "nic-%s\\n", pppoe_ifname);' in ppp_launch
assert 'fprintf(fp, "nic-%s\\n", get_man_ifname(unit));' not in ppp_launch

assert '#include "wan_ifname_policy.h"' in network
relay_start = network.index("set_passthrough_pppoe(int is_on)")
relay_end = network.index("\\n}\\n", relay_start)
relay = network[relay_start:relay_end]
relay_compact = " ".join(relay.split())
assert "wan_ifname_copy(relay_ifname, sizeof(relay_ifname), get_man_ifname(0))" in relay_compact
assert 'eval("/usr/sbin/pppoe-relay", "-C", IFNAME_BR, "-S", relay_ifname)' in relay_compact
assert 'eval("/usr/sbin/pppoe-relay", "-C", IFNAME_BR, "-S", get_man_ifname(0))' not in relay_compact
assert relay.index("wan_ifname_copy(relay_ifname") < relay.index('eval("/usr/sbin/pppoe-relay"')
print("WAN interface-name bounds and PPPoE configuration/argv contracts: PASS")
