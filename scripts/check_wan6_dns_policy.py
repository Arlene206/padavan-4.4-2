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
assert "wan6_dns_copy_config_snapshot" in policy
assert "rc_wan_nvram_copy_snapshot" in policy
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
assert "wan6_dns_store_after_checked_read" in dns_store
assert "wan6_dns_store_if_changed" in policy
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
    "test_dns_config_snapshot_copies_one_coherent_cache_backed_tuple",
    "test_dns_config_snapshot_keeps_missing_values_empty",
    "test_dns_config_snapshot_discards_partial_values_on_read_failure",
    "test_dns_config_snapshot_discards_values_on_transaction_close_failure",
    "test_dns_config_snapshot_begin_failure_does_not_read_or_close",
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
assert "rc_wan_nvram_copy_snapshot(" in reset_v6
assert "wan6_snapshot_transaction_begin" in reset_v6
assert "wan6_snapshot_transaction_get" in reset_v6
assert "wan6_snapshot_transaction_end" in reset_v6
assert "rc_wan_nvram_parse_int_snapshot(" in reset_v6
assert "nvram_safe_get(" not in reset_v6
assert "nvram_get_int(" not in reset_v6
for setting in ("ip6_wan_addr", "ip6_wan_size", "ip6_wan_gate",
                "ip6_6rd_relay", "ip6_6rd_size"):
    assert f'"{setting}"' in reset_v6
dns_builder = body(source, "static int\nbuild_dns6_value(")
assert "wan6_dns_copy_config_snapshot(" in dns_builder
assert "nvram_safe_get(" not in dns_builder
for dns_key in ("ip6_dns1", "ip6_dns2", "ip6_dns3"):
    assert f'"{dns_key}"' in dns_builder
assert "wan6_snapshot_transaction_begin" in dns_builder
assert "wan6_snapshot_transaction_get" in dns_builder
assert "wan6_snapshot_transaction_end" in dns_builder
assert "rc_wan_nvram_copy_snapshot" in policy
assert "rc_wan_nvram_copy_snapshot(" in policy
snapshot_policy = (ROOT / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
assert "fields[i].destination[0] = '\\0';" in snapshot_policy
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


# DHCPv6 DUID/IAID hardware address must distinguish missing MAC from NVRAM faults.
duid_source = open("trunk/user/rc/src/net_wan6.c", encoding="utf-8").read()
duid_start = duid_source.index("int start_dhcp6c(")
duid_end = duid_source.find("\nint ", duid_start + 1)
assert duid_end > duid_start
duid_body = duid_source[duid_start:duid_end]
assert 'wan6_hwaddr_field.param_name = "wan_hwaddr";' in duid_body
assert "rc_wan_nvram_copy_snapshot(" in duid_body
assert "wan6_snapshot_transaction_begin" in duid_body
assert "wan6_snapshot_transaction_get" in duid_body
assert "wan6_snapshot_transaction_end" in duid_body
assert "return wan6_hwaddr_status;" in duid_body
assert 'nvram_safe_get("wan_hwaddr")' not in duid_body

# Preserve the DHCPv6 no-op path before reading optional DUID identity.
duid_ifname_guard = duid_body.index("if (!wan_ifname")
duid_settings_read = duid_body.index("settings_status = wan6_read_dhcp_integer_snapshot(")
duid_noop = duid_body.index("if (!wan6_dhcp && !dns6_auto && !lan6_auto)")
duid_hwaddr_read = duid_body.index('wan6_hwaddr_field.param_name = "wan_hwaddr";')
duid_hwaddr_error = duid_body.index("if (wan6_hwaddr_status != 0)")
duid_session_lock = duid_body.index("session_fd = dhcp6c_callback_session_lock(")
assert duid_ifname_guard < duid_settings_read < duid_noop < duid_hwaddr_read < duid_hwaddr_error < duid_session_lock

# DHCPv6 6rd options must fail closed when the mode selector is unreadable.
sixrd_start = duid_source.index("store_ip6rd_from_dhcp_status(")
sixrd_end = duid_source.index("\nvoid\nstore_ip6rd_from_dhcp(", sixrd_start)
sixrd_body = duid_source[sixrd_start:sixrd_end]
assert "nvram_match(" not in sixrd_body
assert "wan6_read_nvram_match_status(" in sixrd_body
assert '"ip6_6rd_dhcp", "0", &sixrd_dhcp_disabled' in sixrd_body
assert sixrd_body.index("if (!env_value || !prefix || !*prefix)") < sixrd_body.index("wan6_read_nvram_match_status(")
assert sixrd_body.index("if (inet_pton(AF_INET6") < sixrd_body.index("wan6_read_nvram_match_status(")
assert sixrd_body.index("if (status != 0)") < sixrd_body.index("wan6_publish_6rd_snapshot(")
assert sixrd_body.index("if (sixrd_dhcp_disabled)") < sixrd_body.index("wan6_publish_6rd_snapshot(")
nvram_test_source = open("trunk/user/rc/src/tests/wan_nvram_status_policy.c", encoding="utf-8").read()
assert "test_dhcp6_6rd_selector_preserves_missing_value_and_backend_errors" in nvram_test_source
assert "test_dhcp6_6rd_selector_preserves_missing_value_and_backend_errors();" in nvram_test_source

# Native DHCPv6 RA/privacy selectors must be checked before client stop or runtime mutation.
native_start = duid_source.index("int wan6_up(")
native_end = duid_source.index("\nint start_dhcp6c(", native_start)
native_body = duid_source[native_start:native_end]
assert "nvram_invmatch(" not in native_body
assert 'nvram_match("ip6_wan_priv"' not in native_body
native_dhcp_read = native_body.index('wan6_read_nvram_match_status(\n\t\t\t"ip6_wan_dhcp", "1", &wan_dhcp_enabled)')
native_ra_error = native_body.index("if (status != 0)", native_dhcp_read)
native_privacy_read = native_body.index('wan6_read_nvram_match_status(\n\t\t\t\t"ip6_wan_priv", "1", &wan_priv_enabled)')
native_privacy_error = native_body.index("if (status != 0)", native_privacy_read)
native_stop_client = native_body.index("status = stop_dhcp6c();")
native_dns_publish = native_body.index("status = publish_dns6_var_status();")
assert native_dhcp_read < native_ra_error < native_privacy_read < native_privacy_error < native_stop_client < native_dns_publish
assert "allow_ra = !wan_dhcp_enabled;" in native_body
assert "privacy_enabled = allow_ra && wan_priv_enabled;" in native_body
assert "control_if_ipv6_privacy(wan_ifname, privacy_enabled)" in native_body
assert "control_if_ipv6_autoconf(wan_ifname, allow_ra)" in native_body
assert "test_checked_string_match_requires_an_exact_value();" in nvram_test_source
assert "test_checked_string_match_preserves_backend_errors();" in nvram_test_source

# PPP/non-PPP WAN interface selection is a status-bearing NVRAM input.
nvram_policy = (ROOT / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
nvram_status_test = (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
ppp_status_body = body(source, "is_wan_ipv6_if_ppp_status(int *is_ppp)")
ppp_wrapper_body = body(source, "is_wan_ipv6_if_ppp(void)")
ppp_policy_body = body(nvram_policy, "rc_wan_nvram_ipv6_if_ppp_status(int api_result, int saved_errno,")
ppp_launch_body = body(wan_source, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)")
assert "nvram_get_checked(\"ip6_wan_if\", &value)" in ppp_status_body
assert "rc_wan_nvram_ipv6_if_ppp_status(" in ppp_status_body
assert "rc_wan_nvram_read_int_status(" in ppp_policy_body
assert "*is_ppp = -1;" in ppp_policy_body
assert "*is_ppp = (mode == 0);" in ppp_policy_body
assert "status = is_wan_ipv6_if_ppp_status(&is_ppp);" in ppp_wrapper_body
assert "return 1;" in ppp_wrapper_body
assert "int is_wan_ipv6_if_ppp_status(int *is_ppp);" in header
assert "is_wan_ipv6_if_ppp_status(&ipv6_is_ppp)" in ppp_launch_body
assert "if (ipv6_if_status != 0)" in ppp_launch_body
assert "route_status_accumulate(launch_status, ipv6_if_status)" in ppp_launch_body
assert "else if (!ipv6_is_ppp)" in ppp_launch_body
assert "is_wan_ipv6_if_ppp()" not in ppp_launch_body
assert "test_ipv6_if_ppp_status_preserves_missing_default_and_errors" in nvram_status_test
assert "test_ipv6_if_ppp_status_preserves_missing_default_and_errors();" in nvram_status_test

# SIT tunnel configuration must be read with checked NVRAM APIs before replacing the existing tunnel.
nvram_status_test = (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
sit_body = body(source, "int start_sit_tunnel(")
sit_copy_helper = body(source, "wan6_read_nvram_copy_status(const char *param_name,")
sit_int_helper = body(source, "wan6_read_nvram_int_status(const char *param_name,")
for forbidden in ("nvram_safe_get(", "nvram_get_int(", "get_wan_unit_value(", "get_wan_unit_value_int("):
    assert forbidden not in sit_body
for required in ("rc_wan_nvram_copy_value(", "nvram_get_checked(", "rc_wan_nvram_read_int_status("):
    assert required in sit_copy_helper + sit_int_helper
for required in ('"ip6_6in4_remote"', '"ip6_6to4_relay"', '"6rd_size"',
                 '"6rd_relay"', '"gate6"', '"ip6_sit_mtu"', '"ip6_sit_ttl"'):
    assert required in sit_body
for required in ("get_wan_unit_value_int_status(", "get_wan_unit_value_copy_status(",
                 "WAN6_SIT_READ_CHECK("):
    assert required in sit_body
assert "#define WAN6_SIT_READ_CHECK" in source and "errno = status;" in source
sit_first_mutation = sit_body.index("if (is_interface_exist(IFNAME_SIT))")
assert all(sit_body.index(token) < sit_first_mutation for token in (
    '"ip6_6in4_remote"', '"ip6_6to4_relay"', '"6rd_size"', '"6rd_relay"',
    '"gate6"', '"ip6_sit_mtu"', '"ip6_sit_ttl"'))
for test_name in ("test_checked_nvram_copy_treats_missing_key_as_empty",
                  "test_checked_nvram_copy_preserves_backend_failure",
                  "test_checked_nvram_copy_rejects_truncation",
                  "test_checked_integer_read_preserves_missing_default",
                  "test_checked_integer_read_propagates_backend_errors"):
    assert test_name in nvram_status_test and test_name + "();" in nvram_status_test

# wan6_up must obtain per-unit address settings as one checked tuple before stopping clients.
unit_snapshot_body = body(source, "wan6_read_unit_address_snapshot(int unit, int ipv6_type,")
wan6_up_body = body(source, "int wan6_up(")
for token in ("rc_wan_nvram_copy_snapshot(", "wan6_snapshot_transaction_begin",
              "wan6_snapshot_transaction_get", "wan6_snapshot_transaction_end",
              '"wan%d_%s"', "return ENAMETOOLONG"):
    assert token in unit_snapshot_body
assert "wan6_read_unit_address_snapshot(" in wan6_up_body
assert "errno = status;" in wan6_up_body
assert wan6_up_body.index("wan6_read_unit_address_snapshot(") < wan6_up_body.index("status = stop_dhcp6c();")
assert wan6_up_body.index("format_ipv6_literal(wan_addr6, addr6_text") < wan6_up_body.index("status = stop_dhcp6c();")
for lossy in ('get_wan_unit_value(unit, "ipaddr")',
              'get_wan_unit_value(unit, "gateway")',
              'get_wan_unit_value(unit, "addr6")',
              'get_wan_unit_value(unit, "gate6")'):
    assert lossy not in wan6_up_body
for test_name in ("test_wan6_unit_address_snapshot_copies_one_coherent_tuple",
                  "test_wan6_unit_address_snapshot_discards_partial_tuple_on_each_read_error"):
    assert test_name in nvram_status_test and test_name + "();" in nvram_status_test

# DHCPv6 DNS withdrawal must not treat a failed old-value read as an empty list.
dns_store_body = body(source, "int store_wan_dns6(char *dns6_new)")
dns_policy = (ROOT / "trunk/user/rc/src/wan6_dns_policy.h").read_text()
dns_policy_test = (ROOT / "trunk/user/rc/src/tests/wan6_dns_policy.c").read_text()
assert 'get_wan_unit_value_copy_status(\n\t\t0, "dns6", dns6_old, sizeof(dns6_old))' in dns_store_body
assert "wan6_dns_store_after_checked_read(" in dns_store_body
assert "get_wan_unit_value(0, \"dns6\")" not in dns_store_body
assert "if (read_status != 0)" in dns_policy
assert "test_dns_store_fails_closed_when_current_value_read_fails" in dns_policy_test
assert "test_dns_store_fails_closed_when_current_value_read_fails();" in dns_policy_test
assert "test_dns_store_treats_missing_current_value_as_empty();" in dns_policy_test

# IPv6 teardown must not substitute a fallback interface when the previous identity read failed.
wan6_down_body = body(source, "int wan6_down(char *wan_ifname, int unit)")
identity_read = wan6_down_body.index('get_wan_unit_value_copy_status(\n\t\tunit, "ifname6", previous_ifname_buffer')
identity_error = wan6_down_body.index("if (status != 0)", identity_read)
first_interface_probe = wan6_down_body.index("is_interface_exist(previous_ifname)")
assert identity_read < identity_error < first_interface_probe
assert 'get_wan_unit_value(unit, "ifname6")' not in wan6_down_body



# DHCPv6 lease callback must not report success when getifaddrs failed.
lan_lookup = body(source, "wan6_lookup_lan_addr6_status(void *context,")
lan_publish = body(source, "wan6_publish_lan_addr6_status(void *context,")
callback = body(source, "int dhcp6c_main(int argc, char **argv)")
assert "errno = 0;" in lan_lookup
assert "result = get_ifaddr6(IFNAME_BR, 0, address);" in lan_lookup
assert "return errno ? errno : 0;" in lan_lookup
assert "store_lan_addr6((char *)address)" in lan_publish
assert "wan6_identity_publish_lan_addr6(" in callback
assert "if (lan_addr_status != 0)" in callback
assert "callback_status = -1;" in callback
assert "get_ifaddr6(IFNAME_BR, 0, addr6s)" not in callback
for case in ("test_lan_addr6_read_error_is_not_a_successful_noop",
             "test_lan_addr6_clean_miss_preserves_last_published_value",
             "test_lan_addr6_success_publishes_one_complete_address",
             "test_lan_addr6_publish_error_is_propagated",
             "test_lan_addr6_policy_rejects_invalid_arguments"):
    assert case in identity_test and case + "();" in identity_test

# OpenVPN WAN IPv6 endpoint lookup keeps read failures separate from clean misses.
host_addr_body = body(source, "get_wan_addr6_host_status(char *p_addr6s, const char **p_host)")
assert "get_wan_unit_value_copy_status(" in host_addr_body
assert '"ifname6"' in host_addr_body
assert "wan6_identity_resolve_addr6_host(" in host_addr_body
lookup_body = body(source, "wan6_lookup_ifaddr6_status(void *context, const char *ifname,")
assert "errno = 0;" in lookup_body
assert "result = get_ifaddr6(ifname, 0, address);" in lookup_body
assert "return errno ? errno : 0;" in lookup_body
legacy_body = body(source, "char *get_wan_addr6_host(char *p_addr6s)")
assert "get_wan_addr6_host_status(p_addr6s, &host)" in legacy_body
assert "errno = status;" in legacy_body
identity_policy = Path("trunk/user/rc/src/wan6_identity_policy.h").read_text()
identity_test = Path("trunk/user/rc/src/tests/wan6_identity_policy.c").read_text()
host_policy = body(identity_policy, "wan6_identity_resolve_addr6_host(int ifname_status,")
for token in ("if (ifname_status != 0)", "if (!ifname[0])",
              "if (status != 0)", "if (!found)", "strchr(address, '/')"):
    assert token in host_policy
for case in ("test_wan6_address_host_read_error_fails_closed",
             "test_wan6_address_host_missing_interface_keeps_fallback_eligible",
             "test_wan6_address_host_extracts_prefix_free_public_address",
             "test_wan6_address_host_does_not_hide_address_enumeration_error",
             "test_wan6_address_host_no_address_is_a_non_error",
             "test_wan6_address_host_rejects_invalid_arguments"):
    assert case in identity_test and case + "();" in identity_test
vpn_source = Path("trunk/user/rc/src/vpn_openvpn.c").read_text()
vpn_export = body(vpn_source, "ovpn_server_expcli_main(int argc, char **argv)")
assert "wan_addr6_status = get_wan_addr6_host_status(addr6s, &wan_addr);" in vpn_export
assert "if (wan_addr6_status != 0)" in vpn_export and "goto failed;" in vpn_export
assert vpn_export.index("if (wan_addr6_status != 0)") < vpn_export.index("get_lan_addr6_host(addr6s)")
