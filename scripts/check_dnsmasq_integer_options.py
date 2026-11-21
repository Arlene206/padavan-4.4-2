#!/usr/bin/env python3
"""Pin checked DHCP integer snapshots before DNSMasq mutates generated files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SERVICES = (ROOT / "trunk/user/rc/src/services_ex.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/dnsmasq_nvram_option_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/dnsmasq-lifecycle.yml").read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


reader = function_body(
    SERVICES, "dnsmasq_read_integer_range_option(const char *param_name")
assert "nvram_get_checked(param_name, &text)" in reader
assert "dnsmasq_nvram_integer_range_status(" in reader
assert "errno = status;" in reader

prepare = function_body(
    SERVICES, "prepare_dns_dhcpd(int is_ap_mode, int *service_enabled)")
compact = "".join(prepare.split())
assert 'dnsmasq_read_integer_range_option("dhcp_verbose",0,0,3,&i_verbose)' in compact
assert 'dnsmasq_read_integer_range_option("dhcp_lease",0,120,604800,&dhcp_lease)' in compact
assert 'nvram_get_int("dhcp_verbose")' not in prepare
assert 'nvram_get_int("dhcp_lease")' not in prepare
assert prepare.index('"dhcp_verbose"') < prepare.index("fill_static_ethers(")
assert prepare.index('"dhcp_lease"') < prepare.index("fill_static_ethers(")
assert "dhcp_lease);" in prepare

for token in (
    "api_result == 0",
    "rc_wan_nvram_read_int_status(",
    "parsed < minimum || parsed > maximum",
    "return ERANGE;",
):
    assert token in POLICY, token

for token in (
    "test_checked_integer_range_options",
    '"120", 0, 120, 604800',
    '"604800", 0, 120, 604800',
    '"604801", 0, 120, 604800',
    'test_checked_string_option_and_output_preservation',
):
    assert token in TEST, token

assert "scripts/check_dnsmasq_integer_options.py" in WORKFLOW
assert "Verify DHCP integer snapshots precede generated-file changes" in WORKFLOW
print("DNSMasq checked DHCP integer options source contract: PASS")
