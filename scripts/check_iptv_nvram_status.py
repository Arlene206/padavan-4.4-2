#!/usr/bin/env python3
"""Require IPTV proxy transitions to fail closed on unreadable NVRAM identity/policy."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/rc/src/net.c").read_text()
IGMP_CONFIG_POLICY = (ROOT / "trunk/user/rc/src/igmpproxy_config_file.h").read_text()
IPTV_NVRAM_POLICY = (ROOT / "trunk/user/rc/src/iptv_proxy_nvram_policy.h").read_text()
IPTV_ALTNET_POLICY = (ROOT / "trunk/user/rc/src/iptv_proxy_altnet_policy.h").read_text()
IGMP_CONFIG_TEST = (ROOT / "trunk/user/rc/src/tests/igmpproxy_config_file.c").read_text()


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


stop = body("stop_igmpproxy(char *wan_ifname)")
start = body("start_igmpproxy(char *wan_ifname)")
restart = body("restart_iptv(int is_ap_mode)")
settings_reader = body("net_iptv_proxy_nvram_settings_read(")
start_internal = body("start_igmpproxy_with_settings(char *wan_ifname")
stop_internal = body("stop_igmpproxy_with_viptv(")

require('#include "iptv_proxy_nvram_policy.h"' in SOURCE,
        "production source must include the tested settings policy")
require('#include "iptv_proxy_altnet_policy.h"' in IPTV_NVRAM_POLICY and
        "iptv_proxy_altnet_normalize(settings->altnet" in IPTV_NVRAM_POLICY,
        "the checked snapshot must normalize empty altnet and reject invalid IPv4/CIDR before proxy transitions")
require('match_value(context, name, "0", missing_value, &matches_zero)' in IPTV_NVRAM_POLICY and
        'match_value(context, name, "1", missing_value, &matches_one)' in IPTV_NVRAM_POLICY and
        '"mr_enable_x", "1", &enabled' in IPTV_NVRAM_POLICY and
        '"mr_qleave_x", "0",' in IPTV_NVRAM_POLICY and
        "settings->quickleave = quickleave;" in IPTV_NVRAM_POLICY and
        'match_value(context, "mr_enable_x", "1"' not in IPTV_NVRAM_POLICY and
        'match_value(context, "mr_qleave_x", "0"' not in IPTV_NVRAM_POLICY,
        "IPTV selectors must accept exact 0/1 with legacy missing-key defaults")
require('#include "iptv_proxy_altnet_policy.h"' in IGMP_CONFIG_POLICY and
        "iptv_proxy_altnet_is_valid(value)" in IGMP_CONFIG_POLICY and
        "IPTV_PROXY_ALTN_DEFAULT" in IPTV_ALTNET_POLICY and
        "if (prefix > 3 || (prefix == 3 && digit > 2))" in IPTV_ALTNET_POLICY and
        "prefix = prefix * 10 + digit;" in IPTV_ALTNET_POLICY,
        "IGMP writer and NVRAM preflight must share strict inet_addr-compatible IPv4/CIDR validation")
require("iptv_proxy_nvram_settings_read(" in settings_reader and
        "net_nvram_transaction_begin(&transaction)" in settings_reader and
        "net_nvram_transaction_end(&transaction)" in settings_reader and
        "end_status != 0 && status == 0" in settings_reader,
        "all proxy settings must be snapshotted and transaction close checked")

require('net_nvram_copy_setting_status(\n\t\t"viptv_ifname"' in stop and
        "if (nvram_status != 0)" in stop and
        "return nvram_status;" in stop,
        "standalone proxy stop must fail closed when VLAN identity cannot be read")
require("net_iptv_proxy_nvram_settings_read(&settings)" in start and
        "start_igmpproxy_with_settings(wan_ifname, &settings)" in start,
        "standalone proxy start must use one checked settings snapshot")
require("settings->enabled" in start_internal and
        "settings->altnet" in start_internal and
        "settings->quickleave" in start_internal and
        "stop_igmpproxy_with_viptv(" in start_internal and
        "nvram_" not in start_internal,
        "proxy start side effects must not perform additional NVRAM reads")
require('#include "igmpproxy_config_file.h"' in SOURCE and
        "igmpproxy_config_publish(igmpproxy_conf, &config_values)" in start_internal,
        "the validated IGMP config helper must be used by the production proxy launch path")
require('#include "iptv_proxy_altnet_policy.h"' in IGMP_CONFIG_POLICY and
        "iptv_proxy_altnet_is_valid(value)" in IGMP_CONFIG_POLICY and
        "if (*cursor == '0' && cursor[1] >= '0' && cursor[1] <= '9')" in IPTV_ALTNET_POLICY and
        "if (octet != 255U)" in IPTV_ALTNET_POLICY and
        "&& !all_ones;" in IPTV_ALTNET_POLICY,
        "IGMP altnet validation must share the strict inet_addr-compatible octal and INADDR_NONE policy")
require('"010.0.0.0/8"' in IGMP_CONFIG_TEST and
        '"192.168.001.1/24"' in IGMP_CONFIG_TEST and
        '"255.255.255.255/32"' in IGMP_CONFIG_TEST and
        'igmpproxy_config_altnet_token("255.255.255.254/32")' in IGMP_CONFIG_TEST,
        "IGMP config regression must cover octal aliases and the inet_addr broadcast sentinel")
require("kill_services(svcs, 3, 1)" in stop_internal,
        "snapshot-aware stop must retain the original service stop operation")

bridge = restart.index("config_bridge(is_ap_mode)")
vlan_checks = [i for i in range(len(restart))
               if restart.startswith("is_interface_exist(proxy_settings.viptv_ifname)", i)]
require(restart.index("net_iptv_proxy_nvram_settings_read(&proxy_settings)") < bridge and
        restart.index("return operation_status;") < bridge,
        "restart must preflight every proxy setting before bridge changes")
require(restart.index("get_man_ifname(0)") < bridge and
        "if (!wan_ifname || !wan_ifname[0])" in restart,
        "restart must resolve a usable fallback WAN interface before bridge changes")
require(len(vlan_checks) == 2 and vlan_checks[0] < bridge < vlan_checks[1],
        "restart must re-check a configured IPTV VLAN after bridge setup may create it")
require("start_igmpproxy_with_settings(" in restart and
        "start_igmpproxy(wan_ifname)" not in restart,
        "restart must reuse its original snapshot and avoid a second NVRAM read")

policy_test = (ROOT / "trunk/user/rc/src/tests/iptv_proxy_nvram_policy.c").read_text()
for marker in ("test_missing_keys_preserve_legacy_defaults",
               "test_configured_values_are_copied_to_one_snapshot",
               "test_disabled_proxy_skips_dependent_reads",
               "test_any_required_read_failure_invalidates_snapshot",
               "test_invalid_altnet_values_fail_closed",
               "test_invalid_boolean_selectors_fail_closed",
               "test_oversized_values_fail_closed"):
    require(marker in policy_test, "IPTV snapshot regression is missing: " + marker)
require('"239.0.0.0/9"' in policy_test and
        'assert(igmpproxy_config_altnet_token("192.0.2.1/9"));' in IGMP_CONFIG_TEST,
        "single-digit CIDR prefixes within the valid range must remain accepted")

print("IPTV NVRAM status contract: PASS")
