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

print("IPv6 service-type checked-read source contract: PASS")
