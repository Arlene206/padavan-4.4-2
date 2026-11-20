#!/usr/bin/env python3
"""Guard DHCPv6 automatic DNS withdrawal and bounded option publication."""

from pathlib import Path


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError("unterminated function: " + signature)


ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
policy = (ROOT / "trunk/user/rc/src/wan6_dns_policy.h").read_text()
test = (ROOT / "trunk/user/rc/src/tests/wan6_dns_policy.c").read_text()
identity_policy = (ROOT / "trunk/user/rc/src/wan6_identity_policy.h").read_text()
identity_test = (ROOT / "trunk/user/rc/src/tests/wan6_identity_policy.c").read_text()
header = (ROOT / "trunk/user/rc/src/rc.h").read_text()
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


# Active IPv6 interface identity is required runtime state, not best-effort metadata.
assert '#include "wan6_identity_policy.h"' in source
assert "int set_wan_unit_value_status(int unit, const char *param_name, const char *value);" in header
for marker in (
    "wan6_publish_sit_identity",
    "wan6_clear_runtime_metadata",
    "test_sit_identity_failure_removes_tunnel_and_clears_partial_identity",
    "test_sit_identity_failure_retains_state_when_tunnel_removal_fails",
    "test_metadata_clear_attempts_both_fields_and_preserves_first_error",
    "WAN IPv6 identity publication policy: PASS",
):
    assert marker in identity_policy + identity_test, marker

wan6_up = body(source, "int wan6_up(char *wan_ifname, int unit)")
sit_publish = wan6_up.index("status = wan6_publish_sit_identity(")
sit_publish_gate = wan6_up.index("if (status != 0)", sit_publish)
sit_return = wan6_up.index("return status;", sit_publish_gate)
assert sit_publish < sit_publish_gate < sit_return
assert "SIT startup aborted because the active IPv6 interface identity could not be published" in wan6_up
native_publish = wan6_up.index('status = set_wan_unit_value_status(unit, "ifname6", wan_ifname);')
native_gate = wan6_up.index("if (status != 0)", native_publish)
native_failed = wan6_up.index("native_failed:", native_gate)
assert native_publish < native_gate < native_failed
assert "identity_publish_failed = 1;" in wan6_up[native_gate:native_failed]
assert 'failure = "publish native IPv6 WAN interface identity";' in wan6_up[native_gate:native_failed]
client_stop_gate = wan6_up.index("if (dhcp6_start_attempted)", native_failed)
client_stop_return = wan6_up.index("return operation_status;", client_stop_gate)
rollback_gate = wan6_up.index("if (rollback_status == 0)", client_stop_return)
identity_clear = wan6_up.index("wan6_clear_runtime_metadata(", rollback_gate)
assert client_stop_gate < client_stop_return < rollback_gate < identity_clear
assert "retaining native IPv6 interface identity because kernel rollback was incomplete" in wan6_up[rollback_gate:]

wan6_down = body(source, "int wan6_down(char *wan_ifname, int unit)")
no_interface = wan6_down.index("if (!wan6_ifname)")
no_interface_clear = wan6_down.index("wan6_clear_runtime_metadata(", no_interface)
no_interface_return = wan6_down.index("return clear_status;", no_interface_clear)
assert no_interface < no_interface_clear < no_interface_return
successful_teardown = wan6_down.index("if (status == 0)")
metadata_clear = wan6_down.index("wan6_clear_runtime_metadata(", successful_teardown)
assert successful_teardown < metadata_clear
assert "status = route_status_accumulate(status, operation_status);" in wan6_down[metadata_clear:]
assert "retaining IPv6 metadata after incomplete teardown" in wan6_down

print("DHCPv6 DNS withdrawal / response-option contract: PASS")
