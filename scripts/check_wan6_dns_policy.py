#!/usr/bin/env python3
"""Guard DHCPv6 automatic DNS withdrawal and bounded option publication."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
policy = (ROOT / "trunk/user/rc/src/wan6_dns_policy.h").read_text()
test = (ROOT / "trunk/user/rc/src/tests/wan6_dns_policy.c").read_text()
busybox = (ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcp6c_script.c").read_text()
workflow = (ROOT / ".github/workflows/wan-dhcp-callback-stability.yml").read_text()

assert '#include "wan6_dns_policy.h"' in source
assert "return strcmp(old_value, new_value) != 0;" in policy
assert "dns6_len = dns6_new ? strlen(dns6_new) : 0;" in source
assert "if (dns6_len >= sizeof(dns6s))" in source
assert "if (dns6_len)\n\t\tmemcpy(dns6s, dns6_new, dns6_len);" in source
assert 'dns6_new = getenv("new_domain_name_servers");' in source
assert "callback_status = store_wan_dns6(dns6_new);" in source
assert "if (callback_status > 0)" in source
assert "if (callback_status < 0)" in source
assert "errno = saved_errno;" in source
assert "return callback_status;" in source

# BusyBox omits new_* variables when the option is absent from a Reply.
script_env = busybox[
    busybox.index("static const envp_list_t client6_envp_list"):
    busybox.index("char **fill_envp_client6")
]
assert '"new_domain_name_servers"' in script_env
fill_env = busybox[
    busybox.index("char **fill_envp_client6"):
    busybox.index("int dhcp6_script")
]
assert "if (elens[i] > 0)" in fill_env

for marker in (
    "test_withdrawn_dns_option_clears_previous_lease",
    "test_absent_dns_option_is_noop_when_already_empty",
    "test_changed_dns_list_is_published",
    "test_unchanged_dns_list_is_not_republished",
    "WAN DHCPv6 DNS withdrawal policy regression: PASS",
):
    assert marker in test, marker

for path in (
    "scripts/check_wan6_dns_policy.py",
    "trunk/user/rc/src/wan6_dns_policy.h",
    "trunk/user/rc/src/tests/wan6_dns_policy.c",
):
    assert workflow.count(f'      - "{path}"') == 2, path

assert "Verify DHCPv6 automatic DNS withdrawal contract" in workflow
assert "Compile and run WAN DHCPv6 DNS withdrawal policy regression" in workflow

print("DHCPv6 DNS withdrawal / response-option contract: PASS")
