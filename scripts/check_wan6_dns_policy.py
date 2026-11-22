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
wan_source = (ROOT / "trunk/user/rc/src/net_wan.c").read_text()
lease_policy = (ROOT / "trunk/user/rc/src/dhcp_lease_policy.h").read_text()
lease_policy_test = (ROOT / "trunk/user/rc/src/tests/dhcp_lease_policy.c").read_text()
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
assert "dns_status = store_wan_dns6(dns6_new);" in source
assert "if (dns_status > 0)" in source
assert "else if (dns_status < 0)" in source
assert "if (callback_status == 0)" in source
assert "callback_status = dns_status;" in source

# DHCPv6 DNS publication is successful only when the checked NVRAM setter succeeds.
dns_store = body(source, "int store_wan_dns6(")
assert "wan6_dns_store_if_changed" in dns_store
assert "wan6_dns_set_status" in dns_store
assert "errno = -status;" in dns_store
assert 'set_wan_unit_value(0, "dns6", dns6s);' not in dns_store
assert 'set_wan_unit_value_status(0, "dns6", dns_value)' in source
for case in (
    "test_dns_store_publishes_changed_value",
    "test_dns_store_skips_unchanged_value",
    "test_dns_store_publishes_withdrawal",
    "test_dns_store_propagates_nvram_failure",
    "test_dns_store_rejects_missing_writer",
):
    assert case in test and case + "();" in test
assert "if (callback_status < 0)" in source
assert "errno = saved_errno;" in source
assert "return callback_status;" in source

# Reset the checked IPv6 runtime snapshot before WAN activation. Cache-backed
# configuration text is copied before any NVRAM write, and ifname6 is cleared last.
assert '#include "wan_nvram_status_policy.h"' in source
assert "int reset_wan6_vars_status(void);" in header
assert "int reset_wan6_vars(void);" in header
assert "int reset_wan6_vars_status(void)" in source
reset_wrapper = body(source, "reset_wan6_vars(void)")
assert "status = reset_wan6_vars_status();" in reset_wrapper
assert "return status;" in reset_wrapper
reset_v6 = body(source, "int reset_wan6_vars_status(void)")
reset_fields = (
    'reset_values[0].param_name = "addr6";',
    'reset_values[1].param_name = "gate6";',
    'reset_values[2].param_name = "6rd_relay";',
    'reset_values[3].param_name = "6rd_size";',
    'reset_values[4].param_name = "dns6";',
    'reset_values[5].param_name = "ifname6";',
)
reset_positions = [reset_v6.index(item) for item in reset_fields]
assert reset_positions == sorted(reset_positions)
for setting in ("ip6_wan_addr", "ip6_wan_gate", "ip6_6rd_relay", "ip6_6rd_size"):
    assert f'rc_wan_nvram_copy_text(nvram_safe_get("{setting}")' in reset_v6
dns_builder = body(source, "static int\nbuild_dns6_value(")
for dns_key in ("ip6_dns1", "ip6_dns2", "ip6_dns3"):
    assert f'nvram_safe_get("{dns_key}")' in dns_builder
assert "rc_wan_nvram_copy_text" in dns_builder
wan_reset = body(wan_source, "reset_wan_vars_status(void)")
v6_reset_call = wan_reset.index("proto_status = reset_wan6_vars_status();")
v6_failure = wan_reset.index('reset_failure_context = "IPv6 WAN runtime snapshot";', v6_reset_call)
v6_abort = wan_reset.index("goto reset_failed;", v6_failure)
assert v6_reset_call < v6_failure < v6_abort
wan6_start = body(source, "int wan6_up(char *wan_ifname, int unit)")
dns_publish = wan6_start.index("status = publish_dns6_var_status();")
dns_failure = wan6_start.index("if (status != 0)", dns_publish)
dns_return = wan6_start.index("return status;", dns_failure)
assert dns_publish < dns_failure < dns_return
for case in (
    "test_wan6_reset_config_preserves_identity_when_required_write_fails",
    "test_wan6_reset_config_clears_identity_only_after_all_values_succeed",
):
    assert case in (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()

# DHCPv4 option 212 (6rd) is one transactional runtime tuple. A failed
# write must not leave a mixed address/mask/relay tuple for later IPv6 startup.
assert '#include "dhcp_lease_policy.h"' in source
assert "int store_ip6rd_from_dhcp_status(const char *env_value, const char *prefix);" in header
assert "void store_ip6rd_from_dhcp(const char *env_value, const char *prefix);" in header
sixrd_publish = body(source, "static int\nwan6_publish_6rd_snapshot(")
assert "nvram_transaction_begin(&transaction)" in sixrd_publish
assert "nvram_transaction_get(" in sixrd_publish
assert "rc_wan_nvram_copy_text(" in sixrd_publish
assert "dhcp_lease_nvram_publish(" in sixrd_publish
assert "wan6_restore_6rd_values(" in sixrd_publish
sixrd_restore = body(source, "static int\nwan6_restore_6rd_values(")
assert "dhcp_lease_nvram_restore(" in sixrd_restore
sixrd_store = body(source, "int\nstore_ip6rd_from_dhcp_status(")
assert "wan6_publish_6rd_snapshot(" in sixrd_store
assert "status != 0" in sixrd_store
wan_bound = body(wan_source, "static int\nudhcpc_bound(")
sixrd_call = wan_bound.index("snapshot_status = store_ip6rd_from_dhcp_status(value, prefix);")
sixrd_abort = wan_bound.index("goto snapshot_publication_failed;", sixrd_call)
sixrd_publish = wan_bound.index("snapshot_published = 1;")
sixrd_recovery = wan_bound.index("if (snapshot_published)", wan_bound.index("snapshot_publication_failed:"))
sixrd_restore = wan_bound.index("wan_dhcp_nvram_state_publish(", sixrd_recovery)
sixrd_route_abort = wan_bound.index("dhcp_wan_route_transition_abort_snapshot(", sixrd_recovery)
assert sixrd_publish < sixrd_call < sixrd_abort
assert sixrd_recovery < sixrd_restore < sixrd_route_abort
wan_bound_compact = "".join(wan_bound.split())
assert "dhcp_lease_nvram_reverse_snapshot(snapshot_values,snapshot_count,rollback_values)" in wan_bound_compact
assert "dhcp_lease_nvram_reverse_snapshot(" in lease_policy
assert "preserving current interface/routes" in wan_bound
assert "test_dhcp_lease_6rd_snapshot_failure_restores_entire_tuple" in lease_policy_test
assert "test_dhcp_lease_6rd_snapshot_failure_restores_entire_tuple();" in lease_policy_test
assert "dhcp_lease_nvram_publish" in lease_policy

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
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/wan_nvram_status_policy.h",
    "trunk/user/rc/src/tests/wan_nvram_status_policy.c",
):
    assert workflow.count(f'      - "{path}"') == 2, path

assert "Verify DHCPv6 automatic DNS withdrawal contract" in workflow
assert "Compile and run WAN DHCPv6 DNS withdrawal policy regression" in workflow


# DHCPv6 client startup and lease callbacks used to read the auto flags
# through nvram_get_int(), silently mapping backend errors to disabled options.
# Require one checked, coherent transaction before any configuration or lease
# state side effect; missing settings still decode to the legacy zero default.
snapshot_reader = body(source, "wan6_read_dhcp_integer_snapshot(")
assert "rc_wan_nvram_copy_snapshot(" in snapshot_reader
assert "rc_wan_nvram_parse_int_snapshot(" in snapshot_reader
assert "wan6_snapshot_transaction_begin" in snapshot_reader
assert "wan6_snapshot_transaction_get" in snapshot_reader
assert "wan6_snapshot_transaction_end" in snapshot_reader
for key in ("ip6_wan_dhcp", "ip6_dns_auto", "ip6_lan_auto"):
    assert key in source
assert "rc_wan_nvram_parse_int_snapshot" in (
    ROOT / "trunk/user/rc/src/wan_nvram_status_policy.h"
).read_text()
for key in ("ip6_wan_dhcp", "ip6_dns_auto", "ip6_lan_auto"):
    assert f'nvram_get_int("{key}")' not in source

dhcp6_start = body(source, "int start_dhcp6c(char *wan_ifname)")
start_read = dhcp6_start.index(
    "settings_status = wan6_read_dhcp_integer_snapshot("
)
start_error = dhcp6_start.index("if (settings_status != 0)", start_read)
start_noop = dhcp6_start.index("if (!wan6_dhcp && !dns6_auto && !lan6_auto)")
start_lock = dhcp6_start.index("dhcp6c_callback_session_lock(", start_noop)
assert start_read < start_error < start_noop < start_lock
assert "return -1;" in dhcp6_start[start_error:start_noop]
assert "refusing startup" in dhcp6_start[start_error:start_noop]

dhcp6_callback = body(source, "int dhcp6c_main(int argc, char **argv)")
callback_read = dhcp6_callback.index(
    "settings_status = wan6_read_dhcp_integer_snapshot("
)
callback_error = dhcp6_callback.index("if (settings_status != 0)", callback_read)
callback_lan_write = dhcp6_callback.index("store_lan_addr6(", callback_error)
callback_dns_write = dhcp6_callback.index("store_wan_dns6(", callback_error)
assert callback_read < callback_error < callback_lan_write < callback_dns_write
assert "callback_status = -1;" in dhcp6_callback[callback_error:callback_lan_write]
assert "saved_errno = settings_status;" in dhcp6_callback[callback_error:callback_lan_write]
assert "else {" in dhcp6_callback[callback_error:callback_lan_write]
assert "setting_values[0]" in dhcp6_callback
assert "setting_values[1]" in dhcp6_callback

for marker in (
    "test_dhcp6_integer_snapshot_is_transactional_and_all_or_nothing",
    "test_dhcp6_integer_snapshot_is_transactional_and_all_or_nothing();",
):
    assert marker in (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
# Active IPv6 interface identity is required runtime state, not best-effort metadata.
assert '#include "wan6_identity_policy.h"' in source
assert "int set_wan_unit_value_status(int unit, const char *param_name, const char *value);" in header
for marker in (
    "wan6_publish_sit_identity",
    "wan6_clear_runtime_metadata",
    "wan6_clear_native_identity_after_rollback",
    "test_native_identity_is_retained_when_kernel_rollback_incomplete",
    "test_native_identity_is_cleared_after_complete_kernel_rollback",
    "test_native_identity_clear_failure_is_reported",
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
rollback_steps = (
    "if (radv_touched)",
    "if (autoconf_touched)",
    "if (privacy_touched)",
    "if (routes_touched)",
    "if (routes_touched || addresses_touched)",
    "if (addresses_touched)",
)
rollback_positions = [wan6_up.index(step, client_stop_return) for step in rollback_steps]
assert rollback_positions == sorted(rollback_positions)
identity_gate = wan6_up.index("if (identity_publish_failed)", rollback_positions[-1])
identity_clear = wan6_up.index("wan6_clear_native_identity_after_rollback(", identity_gate)
assert client_stop_gate < client_stop_return < rollback_positions[-1] < identity_gate < identity_clear
assert "if (rollback_status != 0)" in identity_policy
assert 'return set_value(context, unit, "ifname6", "");' in identity_policy
assert "retaining native IPv6 interface identity because kernel rollback was incomplete" in wan6_up[identity_gate:]
assert "failed to clear native IPv6 interface identity after startup rollback" in wan6_up[identity_gate:]

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
