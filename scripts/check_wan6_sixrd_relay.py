#!/usr/bin/env python3
"""Bind the RFC 5969 multi-border-relay parser to its regression test."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/wan6_sixrd_option_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/wan6_sixrd_option_policy.c").read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"WAN 6rd multi-relay regression: FAIL: {message}")


require('#include "wan6_sixrd_option_policy.h"' in SOURCE,
        "net_wan6 must include the relay-list policy")
require("char ip6rd[WAN6_SIXRD_OPTION_VALUE_MAX];" in SOURCE,
        "DHCP 6rd text buffer must hold the complete maximum-length option")
require("char *value, *values[3], *end;" in SOURCE,
        "the fixed fields must be parsed separately from the relay list")
require("wan6_sixrd_relay_list_first_valid(" in SOURCE and
        "value, first_relay, sizeof(first_relay), is_valid_ipv4" in SOURCE,
        "all relays must be validated and the first selected")
require("prefix, addr6, values[0], first_relay, &rollback_status" in SOURCE,
        "only the validated first relay may be published")

require("length >= sizeof(candidate)" in POLICY and
        "!validate_ipv4(candidate)" in POLICY and
        "relay_count >= WAN6_SIXRD_MAX_RELAY_COUNT" in POLICY and
        "relay_count != 0" in POLICY,
        "relay parser must cap, validate and require at least one address")
require("at most 59 IPv4 relay addresses" in POLICY,
        "text buffer bound must document the DHCP option's maximum relay count")

require("test_multiple_relays_select_first_and_validate_all" in TEST,
        "multiple relay behavior must have a regression test")
require("999.1.1.1" in TEST and "198.51.100.256" in TEST,
        "malformed trailing relay addresses must be rejected")
require("test_accept_maximum_relay_text" in TEST and
        "WAN6_SIXRD_MAX_RELAY_COUNT" in TEST,
        "maximum-size DHCP relay list must be covered")
require("test_reject_relay_count_above_dhcp_limit" in TEST and
        "WAN6_SIXRD_MAX_RELAY_COUNT + 1U" in TEST,
        "the first over-limit relay count must be rejected")

print("WAN 6rd multi-border-relay integration regression: PASS")
