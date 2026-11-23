#!/usr/bin/env python3
"""Ensure firewall IP inputs are checked and copied before rule generation."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/firewall_ex.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/firewall_nvram_match_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/firewall_nvram_match_policy.c").read_text()
MANGLE_POLICY = (ROOT / "trunk/user/rc/src/firewall_mangle_ttl_policy.h").read_text()
MANGLE_TEST = (ROOT / "trunk/user/rc/src/tests/firewall_mangle_ttl_policy.c").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    position = opening
    while position < len(source):
        char = source[position]
        following = source[position + 1] if position + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "*":
                state = "block"
                position += 2
                continue
            if char == "/" and following == "/":
                state = "line"
                position += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quote"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:position]
        elif state == "quote":
            if char == chr(92):
                position += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and following == "/":
            state = "code"
            position += 2
            continue
        position += 1
    raise AssertionError(f"unterminated function: {signature}")


startup = body(SOURCE, "start_firewall_ex_locked(void)")
for key, destination in (
    ("lan_ipaddr", "lan_ip"),
    ("wanx_ipaddr", "man_ip"),
    ("lan_netmask", "lan_netmask"),
):
    call = f'firewall_read_string_setting_copy_status(\n\t\t"{key}", {destination}, sizeof({destination}))'
    position = startup.index(call)
    status_gate = startup.index("if (status != 0)", position)
    assert position < status_gate < startup.index("ipt_raw_rules();")
    assert f'nvram_safe_get("{key}")' not in startup

wan_call = 'get_wan_unit_value_copy_status(\n\t\tunit, "ipaddr", wan_ip, sizeof(wan_ip))'
wan_position = startup.index(wan_call)
wan_failure = startup.index("if (status != 0)", wan_position)
assert wan_position < wan_failure < startup.index("ipt_raw_rules();")
assert 'get_wan_unit_value(unit, "ipaddr")' not in startup

# All four values are copied before the first firewall/rp_filter side effect.
first_network_write = startup.index("set_interface_conf_int(")
assert startup.index('"lan_ipaddr"') < first_network_write
assert startup.index('"wanx_ipaddr"') < first_network_write
assert startup.index('"lan_netmask"') < first_network_write
assert wan_position < first_network_write

assert "firewall_nvram_copy_string_status" in POLICY
for marker in (
    "test_checked_string_copy_preserves_present_and_missing_values",
    "test_checked_string_copy_fails_closed_on_backend_and_invalid_results",
    "test_checked_string_copy_rejects_truncation_and_invalid_arguments",
):
    assert marker in TEST and marker + "();" in TEST
# The configured IPTV interface participates in IPv4 mangle TTL policy.
# Do not silently fall back to MAN when a checked NVRAM read fails.
mangle = body(SOURCE, "ipt_mangle_rules(const char *man_if")
viptv_position = mangle.index("firewall_read_string_setting_copy_status(")
viptv_call_end = mangle.index(";", viptv_position)
viptv_call = mangle[viptv_position:viptv_call_end]
assert '"viptv_ifname"' in viptv_call
assert "viptv_iflast" in viptv_call
assert 'nvram_safe_get("viptv_ifname")' not in mangle
assert "char viptv_iflast[16]" in mangle
assert viptv_position < mangle.index("fput_int(")
assert viptv_position < mangle.index('module_smart_load("iptable_mangle"')
assert viptv_position < mangle.index("firewall_open_rule_file(ipt_file")
viptv_gate = mangle[viptv_position:viptv_position + 240]
assert 'firewall_mangle_snapshot_check("viptv_ifname", status) != 0' in viptv_gate
assert "return;" in viptv_gate
assert "if (*viptv_iflast && is_interface_exist(viptv_iflast))" in mangle
assert ("test_viptv_interface_snapshot_fails_closed_without_changing_legacy_absence" in TEST
        and "test_viptv_interface_snapshot_fails_closed_without_changing_legacy_absence();" in TEST)


# Failed mangle preflights must abort the enclosing multi-table restore.
mangle_snapshot = body(SOURCE, "ipt_mangle_rules(const char *man_if")
snapshot_helper = body(SOURCE, "firewall_mangle_snapshot_check(const char *setting, int read_status)")
assert "firewall_mangle_snapshot_failure_status(" in snapshot_helper
assert "&firewall_core_restore_transaction.failed" in snapshot_helper
assert "errno = status;" in snapshot_helper
mangle_compact = "".join(mangle_snapshot.split())
for read_call, setting in (
    ('firewall_read_integer_setting("wan_ttl_fix",0,&i_wan_ttl_fix)', "wan_ttl_fix"),
    ('firewall_read_integer_setting("wan_ttl_value",0,&i_wan_ttl_value)', "wan_ttl_value"),
    ('firewall_read_match_setting("mr_enable_x","1",0,&is_multicast_enabled)', "mr_enable_x"),
    ('firewall_read_string_setting_copy_status("viptv_ifname",viptv_iflast,sizeof(viptv_iflast))', "viptv_ifname"),
):
    read_at = mangle_compact.index(read_call)
    gate_at = mangle_compact.index(
        f'if(firewall_mangle_snapshot_check("{setting}",status)!=0)return;', read_at)
    assert read_at < gate_at
assert "firewall_mangle_snapshot_failure_status" in MANGLE_POLICY
for marker in (
    "test_snapshot_success_preserves_transaction_state",
    "test_snapshot_read_errors_abort_the_enclosing_transaction",
):
    assert marker in MANGLE_TEST and marker + "();" in MANGLE_TEST
assert "firewall_mangle_snapshot_failure_status(EACCES,&failed)==EACCES" in "".join(MANGLE_TEST.split())
assert "firewall_mangle_snapshot_failure_status(-1,&failed)==EIO" in "".join(MANGLE_TEST.split())
mangle_caller = body(SOURCE, "start_firewall_ex_locked(void)")
mangle_call_at = mangle_caller.index("ipt_mangle_rules(man_if, wan_if, i_use_man)")
mangle_gate_at = mangle_caller.index(
    "if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))", mangle_call_at)
mangle_nat_at = mangle_caller.index("ipt_nat_rules(", mangle_call_at)
assert mangle_call_at < mangle_gate_at < mangle_nat_at


# Virtual-server destination addresses must not turn NVRAM backend failures
# into empty values and silently omit configured port-forward/filter rules.
ip_reader = body(SOURCE, "ip_conv(const char *ip_name, int idx)")
assert "firewall_read_indexed_string_setting(ip_name, idx, &value)" in ip_reader
assert "nvram_safe_get(" not in ip_reader
for signature in (
    "include_vts_filter(FILE *fp, char *lan_ip",
    "include_vts_nat(FILE *fp, int num_entries)",
):
    emitter = body(SOURCE, signature)
    read_value = emitter.index('ip_conv("vts_ipaddr_x", i)')
    null_gate = emitter.index("if (!dstip)", read_value)
    invalid_gate = emitter.index("if (!firewall_ipv4_address_is_safe(dstip))", null_gate)
    assert read_value < null_gate < invalid_gate
    assert "return -1;" in emitter[null_gate:invalid_gate]
    assert "return 0;" in emitter
    assert "refusing incomplete" in emitter[null_gate:invalid_gate]
filter_builder = body(SOURCE, "ipt_filter_rules(char *man_if")
assert filter_builder.count("vts_status = include_vts_filter(") == 2
assert filter_builder.count("if (vts_status != 0)") == 2
for gate in (position for position in range(len(filter_builder))
             if filter_builder.startswith("if (vts_status != 0)", position)):
    cleanup = filter_builder[gate:gate + 650]
    assert "(void)fclose(fp);" in cleanup
    assert "firewall_abort_core_restore_after_staging_failure();" in cleanup
    assert "firewall_rule_file_remove(ipt_file);" in cleanup
    assert "return -1;" in cleanup
nat_builder = body(SOURCE, "ipt_nat_rules(char *man_if")
assert "int vts_status, saved_errno;" in nat_builder
assert "vts_status = include_vts_nat(fp, i_vts_count);" in nat_builder
assert "if (vts_status != 0)" in nat_builder
nat_gate = nat_builder.index("if (vts_status != 0)")
nat_cleanup = nat_builder[nat_gate:nat_gate + 650]
assert "(void)fclose(fp);" in nat_cleanup
assert "firewall_abort_core_restore_after_staging_failure();" in nat_cleanup
assert "firewall_rule_file_remove(ipt_file);" in nat_cleanup
assert "return -1;" in nat_cleanup
assert "firewall_nvram_string_value_status" in POLICY
for marker in (
    "test_checked_string_resolver_preserves_missing_default",
    "test_checked_string_resolver_rejects_backend_and_malformed_results",
):
    assert marker in TEST and marker + "();" in TEST


# VPN server subnet data influences MASQUERADE routes and must be snapshotted
# before opening the NAT rules file; backend errors must not become an empty pool.
nat_builder = body(SOURCE, "ipt_nat_rules(char *man_if")
vpns_read = nat_builder.index("firewall_read_string_setting_copy_status(")
vpns_call_end = nat_builder.index(";", vpns_read)
vpns_call = nat_builder[vpns_read:vpns_call_end]
assert '"vpns_vnet"' in vpns_call and "vpns_vnet" in vpns_call
assert "char vpns_vnet[16]" in nat_builder
assert vpns_read < nat_builder.index('firewall_open_rule_file(ipt_file, "IPv4 NAT")')
vpns_error_gate = nat_builder[vpns_call_end:vpns_call_end + 180]
assert "if (firewall_status != 0)" in vpns_error_gate
assert "return firewall_status;" in vpns_error_gate
assert 'nvram_safe_get("vpns_vnet")' not in nat_builder
assert "ip2class(vpns_vnet, VPN_SERVER_SUBNET_MASK, vpn_net, sizeof(vpn_net));" in nat_builder
vpns_valid_call = "firewall_nvram_ipv4_address_is_valid(vpns_vnet)"
assert vpns_valid_call in nat_builder
vpns_valid_at = nat_builder.index(vpns_valid_call)
vpns_file_open_at = nat_builder.index('firewall_open_rule_file(ipt_file, "IPv4 NAT")')
vpns_ip2class_at = nat_builder.index("ip2class(vpns_vnet, VPN_SERVER_SUBNET_MASK, vpn_net, sizeof(vpn_net));")
assert vpns_valid_at < vpns_file_open_at < vpns_ip2class_at
assert "invalid VPN server subnet address; refusing IPv4 NAT rules" in nat_builder
assert "test_vpns_vnet_requires_strict_ipv4_address" in TEST
assert "test_vpns_vnet_requires_strict_ipv4_address();" in TEST
assert ("test_vpns_vnet_snapshot_preserves_missing_and_rejects_read_errors" in TEST
        and "test_vpns_vnet_snapshot_preserves_missing_and_rejects_read_errors();" in TEST)

print("Firewall IP NVRAM snapshot contract: PASS")
