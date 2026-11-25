#!/usr/bin/env python3
"""Bind static WAN DNS literal validation to the production publisher."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/net_wan6.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/wan6_dns_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/wan6_dns_policy.c").read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"WAN IPv6 static DNS policy: FAIL: {message}")


start = SOURCE.index("build_dns6_value(")
opening = SOURCE.index("{", start)
depth = 0
end = opening
while end < len(SOURCE):
    if SOURCE[end] == "{":
        depth += 1
    elif SOURCE[end] == "}":
        depth -= 1
        if depth == 0:
            break
    end += 1
require(depth == 0, "build_dns6_value body must be complete")
builder = SOURCE[opening + 1:end]

validation = "status = wan6_dns_validate_static_server(dnsnv[i]);"
assertion = "written = snprintf(dns6s, dns6_size,"
require(validation in builder, "each static DNS slot must be validated")
require(builder.index(validation) < builder.index(assertion),
        "validation must happen before resolver-facing string construction")
require("if (status != 0)" in builder[builder.index(validation):builder.index(assertion)],
        "an invalid static DNS field must abort publication")

require("inet_pton(AF_INET6, address, &parsed) == 1" in POLICY,
        "policy must accept only one IPv6 literal")
require("test_static_dns_server_validation_accepts_ipv6_and_empty_slots" in TEST,
        "valid IPv6 and optional empty slots need regression coverage")
require("test_static_dns_server_validation_rejects_non_literals_and_injection" in TEST,
        "hostnames, IPv4, prefixes and CR/LF injection need regression coverage")
require("2001:db8::53\\nnameserver 192.0.2.1" in TEST,
        "newline injection case must remain in the test")

store_start = SOURCE.index("int store_wan_dns6(")
store_open = SOURCE.index("{", store_start)
depth = 0
store_end = store_open
while store_end < len(SOURCE):
    if SOURCE[store_end] == "{":
        depth += 1
    elif SOURCE[store_end] == "}":
        depth -= 1
        if depth == 0:
            break
    store_end += 1
require(depth == 0, "store_wan_dns6 body must be complete")
store = SOURCE[store_open + 1:store_end]
server_list_validation = "status = wan6_dns_validate_server_list(dns6s);"
read_current = "read_status = get_wan_unit_value_copy_status("
require(server_list_validation in store,
        "DHCPv6-provided DNS servers must be validated")
require(store.index(server_list_validation) < store.index(read_current),
        "invalid DHCPv6 DNS must be rejected before NVRAM access/publication")
require("rejected malformed DHCPv6 DNS server list" in store,
        "invalid DHCPv6 DNS rejection must remain observable")
require("wan6_dns_validate_server_list" in POLICY and
        "inet_pton(AF_INET6, address, &parsed) != 1" in POLICY,
        "DHCPv6 DNS token parser must reject non-IPv6 literals")
require("test_dhcp_dns_server_list_accepts_valid_literals_and_withdrawal" in TEST and
        "test_dhcp_dns_server_list_rejects_malformed_tokens_and_controls" in TEST,
        "DHCPv6 DNS list needs positive and fail-closed tests")

print("WAN IPv6 static and DHCP DNS literal validation contract: PASS")
