#!/usr/bin/env python3
"""Ensure WAN DHCP timestamps are committed only after lease acceptance."""

from pathlib import Path

source = (Path("trunk/user/rc/src") / "net_wan.c").read_text()
test_source = (Path("trunk/user/rc/src/tests") / "dhcp_lease_policy.c").read_text()


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

print("WAN DHCP dltime accepted-lease publication contract: PASS")
