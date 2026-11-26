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
iptv_bound = function_body(
    source, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)"
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

# Validate the complete Router option before any interface mutation or old
# route cleanup. Invalid non-empty input must not be published as withdrawal.
router_preflight = bound.index(
    "route_result = dhcp_lease_router_option_parse("
)
router_validation = bound.index("if (route_result != 0)", router_preflight)
router_reject = bound.index("return route_result;", router_validation)
route_options_preflight = bound.index('value = getenv("routes")')
route_options_ms_preflight = bound.index('value = getenv("msstaticroutes")')
route_options_rfc_preflight = bound.index('value = getenv("staticroutes")')
route_options_reject = bound.index("rejected invalid DHCP routes option", route_options_preflight)
route_options_ms_reject = bound.index("rejected invalid DHCP Microsoft routes option", route_options_ms_preflight)
route_options_rfc_reject = bound.index("rejected invalid DHCP classless routes option", route_options_rfc_preflight)
previous_snapshot_pos = bound.index("rc_wan_nvram_copy_snapshot(")
interface_apply_pos = bound.index("dhcp_wan_interface_apply(")
route_cleanup_pos = bound.index("dhcp_wan_route_transition_apply(")
assert router_preflight < router_validation < router_reject
assert route_options_preflight < route_options_reject < route_options_ms_preflight
assert route_options_ms_preflight < route_options_ms_reject < route_options_rfc_preflight
assert route_options_rfc_preflight < route_options_rfc_reject < previous_snapshot_pos
assert router_reject < route_options_preflight < previous_snapshot_pos
assert previous_snapshot_pos < interface_apply_pos < route_cleanup_pos
assert "ignoring invalid DHCP router list" not in bound
assert "ignoring invalid DHCP route option" not in bound
assert "ignoring invalid DHCP Microsoft route option" not in bound
assert "ignoring invalid DHCP classless route option" not in bound
post_preflight = bound[router_reject:bound.index(
    "snapshot_status = wan_dhcp_nvram_state_publish("
)]
assert "gateway_buf[0] = '\\0';" not in post_preflight
assert "dhcp_lease_router_option_parse" in (
    Path("trunk/user/rc/src/dhcp_lease_policy.h").read_text()
)
assert 'dhcp_lease_router_option_parse("192.0.2.1 999.1.1.1"' in test_source

# DHCP route-list validation mirrors the exact token grammar consumed by
# control_dhcp_routes(); reject partial or malformed inventories before any
# add/delete operation can begin.
lease_policy = Path("trunk/user/rc/src/dhcp_lease_policy.h").read_text()
assert "dhcp_lease_route_separators_are_valid(value)" in lease_policy
assert "dhcp_lease_legacy_route_list_is_valid" in lease_policy
assert "dhcp_lease_classless_route_list_is_valid" in lease_policy
assert '"0.0.0.0/198.51.100.1"' in test_source
assert '"192.0.2.0/198.51.100.1  192.0.3.0/198.51.100.2"' in test_source
assert '"192.0.2.0/24  198.51.100.1"' in test_source
assert '"192.0.2.0/24\\t198.51.100.1"' in test_source
control_body = function_body(source, "control_dhcp_routes(")
legacy_validation = control_body.index(
    "dhcp_lease_legacy_route_list_is_valid("
)
classless_validation = control_body.index(
    "dhcp_lease_classless_route_list_is_valid("
)
route_add = control_body.index("dhcp_route_add_owned(")
route_cleanup = control_body.index("dhcp_route_cleanup_owned(owner)")
assert legacy_validation < route_add
assert classless_validation < route_add
assert route_cleanup < route_add
assert "route_del(" not in control_body
assert "dhcp_route_cleanup_owned(owner)" in control_body

# IPTV must reject malformed route and Router options before the previous
# interface/route inventory can be modified or its replacement published.
iptv_router_preflight = iptv_bound.index(
    "route_result = dhcp_lease_router_option_parse("
)
iptv_router_reject = iptv_bound.index("return route_result;", iptv_router_preflight)
iptv_legacy_routes = iptv_bound.index(
    "dhcp_lease_legacy_route_list_is_valid("
)
iptv_legacy_reject = iptv_bound.index(
    "rejected invalid IPTV DHCP route option", iptv_legacy_routes
)
iptv_ms_routes = iptv_bound.index(
    "dhcp_lease_classless_route_list_is_valid(", iptv_legacy_reject
)
iptv_ms_reject = iptv_bound.index(
    "rejected invalid IPTV DHCP Microsoft route option", iptv_ms_routes
)
iptv_rfc_routes = iptv_bound.index(
    "dhcp_lease_classless_route_list_is_valid(", iptv_ms_reject
)
iptv_rfc_reject = iptv_bound.index(
    "rejected invalid IPTV DHCP classless route option", iptv_rfc_routes
)
iptv_previous_snapshot = iptv_bound.index("rc_wan_nvram_copy_snapshot(")
iptv_interface_change = min(
    iptv_bound.index("wan_dhcp_interface_clear("),
    iptv_bound.index("dhcp_wan_interface_apply(")
)
iptv_route_teardown = iptv_bound.index("dhcp_wan_route_transition_apply(")
iptv_publish = iptv_bound.index("wan_dhcp_nvram_state_publish(")
assert iptv_router_preflight < iptv_router_reject < iptv_legacy_routes
assert iptv_legacy_routes < iptv_legacy_reject < iptv_ms_routes
assert iptv_ms_routes < iptv_ms_reject < iptv_rfc_routes
assert iptv_rfc_routes < iptv_rfc_reject < iptv_previous_snapshot
assert iptv_previous_snapshot < iptv_interface_change < iptv_route_teardown < iptv_publish
for ignored in (
    "ignoring invalid DHCP router list",
    "ignoring invalid DHCP IPTV route option",
    "ignoring invalid DHCP IPTV Microsoft route option",
    "ignoring invalid DHCP IPTV classless route option",
):
    assert ignored not in iptv_bound

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


# DHCP renewal decisions must consume one coherent, checked snapshot. Missing
# keys remain empty/zero defaults; read or close failures abort before a bind
# decision or lease/resolver publication.
snapshot = function_body(
    source, "wan_dhcp_renew_state_snapshot(const char *prefix, int include_lease_options"
)
for key in ('"err", lease_error', '"route_err", route_error',
            '"mtu_err", mtu_error', '"netmask", netmask',
            '"gateway", gateway', '"ipaddr", ipaddr',
            '"routes", routes', '"routes_ms", routes_ms',
            '"routes_rfc", routes_rfc', '"mtu", mtu',
            '"dns", dns', '"domain", domain', '"wins", wins', '"lease", lease'):
    assert key in snapshot, "DHCP renewal snapshot omits " + key
assert "rc_wan_nvram_copy_snapshot(" in snapshot
assert "wan_nvram_transaction_begin_callback" in snapshot
assert "wan_nvram_transaction_get_callback" in snapshot
assert "wan_nvram_transaction_end_callback" in snapshot

for signature, bound_call, include_options in (
    ("udhcpc_renew(char *wan_ifname)", "returnudhcpc_bound(", True),
    ("udhcpc_viptv_renew(char *man_ifname)", "returnudhcpc_viptv_bound(", False),
):
    renew_body = function_body(source, signature)
    compact = "".join(renew_body.split())
    snapshot_call = (
        "wan_dhcp_renew_state_snapshot(prefix,1,&previous_state);"
        if include_options else
        "wan_dhcp_renew_state_snapshot(prefix,0,&previous_state);"
    )
    assert snapshot_call in compact
    snapshot_pos = compact.index(snapshot_call)
    decision = (
        "dhcp_wan_bound_retry_decide(lease_error_read_status,"
        "route_error_read_status,mtu_error_read_status,lease_error,"
        "route_error,mtu_error,&retry_required);"
    )
    assert decision in compact
    decision_pos = compact.index(decision)
    assert snapshot_pos < decision_pos
    error_gate = compact.index("if(nvram_status!=0)", decision_pos)
    retry_gate = compact.index("if(retry_required)", error_gate)
    retry_call = compact.index(bound_call, retry_gate)
    assert snapshot_pos < decision_pos < error_gate < retry_gate < retry_call
    for forbidden in ("nvram_invmatch(", "nvram_safe_get(",
                      "get_nvram_int_checked_status("):
        assert forbidden not in renew_body, "unchecked DHCP renewal read remains: " + forbidden
    for field in ("netmask", "gateway", "ipaddr", "routes", "routes_ms",
                  "routes_rfc", "mtu"):
        assert "previous_state." + field in renew_body
    if include_options:
        for field in ("dns", "domain", "wins", "lease"):
            assert "previous_state." + field in renew_body
    assert "rc_wan_nvram_parse_int_value(" in renew_body

wan_renew = function_body(source, "udhcpc_renew(char *wan_ifname)")
assert "if (resolver_changed)" in wan_renew
assert wan_renew.index("if (resolver_changed)") < wan_renew.index("wan_dhcp_nvram_publish_values(")
for field in ("dns", "domain", "wins", "lease"):
    assert "renew_values[renew_count].previous_value = previous_state." + field in wan_renew

dhcp_snapshot_test = Path(
    "trunk/user/rc/src/tests/wan_nvram_status_policy.c"
).read_text()
assert "DHCP_RENEW_SNAPSHOT_TEST_COUNT 14" in dhcp_snapshot_test
assert "test_transactional_dhcp_renew_snapshot_copies_complete_state" in dhcp_snapshot_test
assert "dhcp_wan_bound_retry_decide" in route_source
assert "test_retry_decision_fails_closed_on_each_status_read_error" in route_test
assert "test_retry_decision_requires_a_complete_checked_tuple" in route_test

# Also preserve a coherent rollback tuple for IPTV DHCP and ZeroConf before
# any live interface changes.
for signature in (
    "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)",
    "zcip_bound(char *man_ifname)",
    "zcip_viptv_bound(char *man_ifname)",
):
    callback = function_body(source, signature)
    snapshot = callback.index("rc_wan_nvram_copy_snapshot(")
    interface_apply = callback.index("dhcp_wan_interface_apply(")
    assert snapshot < interface_apply
    assert "wan_nvram_transaction_begin_callback" in callback[snapshot:]
    assert "wan_nvram_transaction_get_callback" in callback[snapshot:]
    assert "wan_nvram_transaction_end_callback" in callback[snapshot:]
    assert "previous_value = nvram_safe_get(" not in callback
    assert "cannot snapshot previous" in callback

print("WAN DHCP/IPTV/ZeroConf rollback snapshots are transactionally coherent: PASS")
