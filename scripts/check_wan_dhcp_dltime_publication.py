#!/usr/bin/env python3
"""Ensure WAN DHCP timestamps are committed only after lease acceptance."""

from pathlib import Path

source = (Path("trunk/user/rc/src") / "net_wan.c").read_text()
test_source = (Path("trunk/user/rc/src/tests") / "dhcp_lease_policy.c").read_text()
route_source = (Path("trunk/user/rc/src") / "dhcp_wan_route_transition_policy.h").read_text()
route_test = (Path("trunk/user/rc/src/tests") / "dhcp_wan_route_transition_policy.c").read_text()


def function_body(text, signature):
    search_from = 0
    while True:
        start = text.index(signature, search_from)
        brace = text.index("{", start)
        terminator = text.find(";", start)
        if terminator >= 0 and terminator < brace:
            search_from = start + len(signature)
            continue
        break

    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


bound = function_body(
    source, "udhcpc_bound(char *wan_ifname, int is_renew_mode)"
)
renew = function_body(source, "udhcpc_renew(char *wan_ifname)")

assert "WAN_DHCP_BOUND_DLTIME" in source
assert '"gateway", "dltime"' in source
assert 'set_wan_unit_value(unit, "dltime", tmp)' not in bound
assert "if (is_man && i == WAN_DHCP_BOUND_DLTIME)" in bound

previous_snapshot = bound.index("rc_wan_nvram_copy_snapshot(")
interface_apply = bound.index("dhcp_wan_interface_apply(")
assert previous_snapshot < interface_apply
assert "struct rc_wan_nvram_snapshot_field previous_snapshot_fields[WAN_DHCP_BOUND_SNAPSHOT_COUNT];" in bound
assert "wan_nvram_transaction_begin_callback" in bound[previous_snapshot:]
assert "wan_nvram_transaction_get_callback" in bound[previous_snapshot:]
assert "wan_nvram_transaction_end_callback" in bound[previous_snapshot:]
assert "cannot snapshot previous DHCP NVRAM tuple" in bound
assert "nvram_safe_get(bound_names[i])" not in bound

bound_dltime = bound.index("bound_names[WAN_DHCP_BOUND_DLTIME]")
bound_publish = bound.index("snapshot_status = wan_dhcp_nvram_state_publish(")
assert bound.rfind("if (!is_man)", 0, bound_dltime) >= 0
assert bound_dltime < bound_publish
assert "dltime_text" in bound[bound_dltime:bound_publish]
assert "previous_text[WAN_DHCP_BOUND_DLTIME]" in bound[bound_dltime:bound_publish]

assert 'set_wan_unit_value(unit, "dltime", tmp)' not in renew
renew_dltime = renew.index("wan_dhcp_nvram_publish_values(")
renew_options = renew.index("if (renew_count != 0)")
renew_dns = renew.index("if (resolver_changed)")
assert renew.rfind("if (!is_man)", 0, renew_dltime) >= 0
assert renew_options < renew_dltime
assert renew_dns < renew_dltime
assert "dltime_names, dltime_values, 1" in renew[renew_dltime:]
assert "if (nvram_status != 0)" in renew[renew_dltime:]
assert "return nvram_status;" in renew[renew_dltime:]
assert renew_dltime < renew.rindex("return 0;")

assert "wan0_dltime" in test_source
assert "test_dhcp_lease_fast_renew_publishes_dltime_as_one_checked_value" in test_source
assert '"ABCDEFGHIJKLlkjihgfedcba"' in test_source


# Previous lease/route/MTU state must be complete before renew can skip a bind.
for signature, bound_call in (
    ("udhcpc_renew(char *wan_ifname)", "returnudhcpc_bound("),
    ("udhcpc_viptv_renew(char *man_ifname)", "returnudhcpc_viptv_bound("),
):
    renew_body = function_body(source, signature)
    compact = "".join(renew_body.split())
    read_tokens = (
        'get_nvram_int_checked_status(strcat_r(prefix,"err",tmp),&lease_error);',
        'get_nvram_int_checked_status(strcat_r(prefix,"route_err",tmp),&route_error);',
        'get_nvram_int_checked_status(strcat_r(prefix,"mtu_err",tmp),&mtu_error);',
    )
    read_positions = [compact.index(token) for token in read_tokens]
    decision = (
        "dhcp_wan_bound_retry_decide(lease_error_read_status,"
        "route_error_read_status,mtu_error_read_status,lease_error,"
        "route_error,mtu_error,&retry_required);"
    )
    assert decision in compact
    decision_pos = compact.index(decision)
    assert read_positions == sorted(read_positions)
    assert read_positions[-1] < decision_pos
    error_gate = compact.index("if(nvram_status!=0)", decision_pos)
    retry_gate = compact.index("if(retry_required)", error_gate)
    retry_call = compact.index(bound_call, retry_gate)
    assert decision_pos < error_gate < retry_gate < retry_call
    assert "cannot verify previous DHCP retry state" in renew_body

wan_renew = function_body(source, "udhcpc_renew(char *wan_ifname)")
viptv_renew = function_body(source, "udhcpc_viptv_renew(char *man_ifname)")
assert 'get_nvram_int_checked_status(strcat_r(prefix,"mtu",tmp),&stored_mtu);' in "".join(wan_renew.split())
assert 'get_nvram_int_checked_status("manv_mtu",&stored_mtu);' in "".join(viptv_renew.split())
assert "cannot verify stored DHCP MTU" in wan_renew
assert "cannot verify stored IPTV MTU" in viptv_renew
assert "dhcp_wan_bound_retry_decide" in route_source
assert "test_retry_decision_fails_closed_on_each_status_read_error" in route_test
assert "test_retry_decision_requires_a_complete_checked_tuple" in route_test

print("WAN DHCP checked-renew state and dltime publication contracts: PASS")
