#!/usr/bin/env python3
"""Ensure firewall IP inputs are checked and copied before rule generation."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/firewall_ex.c").read_text()
POLICY = (ROOT / "trunk/user/rc/src/firewall_nvram_match_policy.h").read_text()
TEST = (ROOT / "trunk/user/rc/src/tests/firewall_nvram_match_policy.c").read_text()


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
print("Firewall IP NVRAM snapshot contract: PASS")
