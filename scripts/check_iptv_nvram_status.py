#!/usr/bin/env python3
"""Require IPTV proxy transitions to fail closed on unreadable NVRAM identity/policy."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/net.c").read_text()


def require(condition, message):
    if not condition:
        print(f"IPTV NVRAM status contract: FAIL: {message}", file=sys.stderr)
        raise SystemExit(1)


def body(signature):
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(SOURCE):
        char = SOURCE[index]
        nxt = SOURCE[index + 1] if index + 1 < len(SOURCE) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return SOURCE[opening + 1:index]
        elif state == "quoted":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line_comment":
            if char == chr(10):
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


copy_helper = body("net_nvram_copy_setting_status(")
match_helper = body("net_nvram_match_setting_status(")
stop = body("stop_igmpproxy(char *wan_ifname)")
start = body("start_igmpproxy(char *wan_ifname)")
restart = body("restart_iptv(int is_ap_mode)")

require("rc_wan_nvram_copy_value(net_nvram_get_checked" in copy_helper,
        "cache-backed values must be copied through the checked NVRAM reader")
require("nvram_get_checked(param_name, &value)" in match_helper and
        "rc_wan_nvram_read_status(read_result, saved_errno)" in match_helper and
        "rc_wan_nvram_match_status(0, value, expected, matches)" in match_helper,
        "setting comparisons must distinguish missing keys from read/close failures")

require('net_nvram_copy_setting_status(\n\t\t\t"viptv_ifname"' in stop and
        "if (nvram_status != 0)" in stop and
        "return nvram_status;" in stop,
        "proxy stop must preserve an unreadable IPTV VLAN identity before kill_services")
require('net_nvram_copy_setting_status(\n\t\t"viptv_ifname"' in start and
        'net_nvram_match_setting_status(\n\t\t"mr_enable_x", "1", "", &mr_enabled)' in start and
        'net_nvram_copy_setting_status(\n\t\t\t"mr_altnet_x"' in start and
        'net_nvram_match_setting_status(\n\t\t\t"mr_qleave_x", "0", "", &quickleave_is_zero)' in start,
        "proxy startup must check VLAN, enable, altnet and quickleave values")
for marker in ("stop_igmpproxy(wan_ifname)", "start_udpxy(wan_ifname)",
               "igmpproxy_config_publish(igmpproxy_conf, &config_values)"):
    require(marker in start, "expected proxy lifecycle stage is missing: " + marker)
for marker in ('nvram_safe_get("viptv_ifname")', 'nvram_get("mr_altnet_x")',
               'nvram_invmatch("mr_enable_x", "1")',
               'nvram_invmatch("mr_qleave_x", "0")'):
    require(marker not in start, "unchecked IPTV policy read remains: " + marker)

require(restart.index('net_nvram_copy_setting_status(\n            "viptv_ifname"') <
        restart.index("config_bridge(is_ap_mode)") and
        "if (nvram_status != 0)" in restart and
        restart.index("return nvram_status;") < restart.index("config_bridge(is_ap_mode)"),
        "restart must preflight IPTV VLAN identity before bridge/proxy changes")
require('nvram_safe_get("viptv_ifname")' not in restart,
        "restart must not collapse an unreadable VLAN identity into an empty default")

# Existing policy regression exercises checked-copy and missing/error semantics.
policy = (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
for marker in ("test_checked_nvram_copy_treats_missing_key_as_empty",
               "test_checked_nvram_copy_preserves_backend_failure",
               "test_checked_nvram_copy_rejects_truncation",
               "test_checked_string_match_requires_an_exact_value"):
    require(marker in policy, "checked NVRAM regression is missing: " + marker)

print("IPTV NVRAM status contract: PASS")
