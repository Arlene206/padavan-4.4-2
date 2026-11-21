#!/usr/bin/env python3
"""Guard the enabled-DDNS WAN-up result contract and disabled/AP no-op cases."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN_SOURCE = (RC / "net_wan.c").read_text()
SERVICES_SOURCE = (RC / "services_ex.c").read_text()
POLICY_SOURCE = (RC / "inadyn_daemon_policy.h").read_text()
POLICY_TEST = (RC / "tests/inadyn_daemon_policy.c").read_text()
WAN_NVRAM_POLICY = (RC / "wan_nvram_status_policy.h").read_text()
WAN_NVRAM_TEST = (RC / "tests/wan_nvram_status_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()


def function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and next_char == "/":
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
                    return source[opening + 1:index]
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
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        index += 1
    raise AssertionError(f"unterminated C function: {signature}")


def require(condition, message):
    if not condition:
        raise SystemExit(f"WAN DDNS status contract: FAIL: {message}")


policy = function_body(POLICY_SOURCE, "inadyn_update_should_run(")
require("return ddns_enabled && !is_ap_mode;" in policy,
        "DDNS should run only when enabled and not in AP mode")

notify = function_body(SERVICES_SOURCE, "notify_ddns_update_locked(void)")
mode_read = notify.index("get_ap_mode_status(&is_ap_mode)")
mode_error = notify.index("if (ap_mode_status != 0)", mode_read)
gate = notify.index("inadyn_update_should_run(")
inventory = notify.index('process_inventory_scan("inadyn", &snapshot)')
require(mode_read < mode_error < gate < inventory and
        'nvram_match("ddns_enable_x", "1")' in notify and
        "is_ap_mode" in notify and
        "return ap_mode_status;" in notify[mode_error:gate] and
        "return 0;" in notify[gate:inventory],
        "DDNS updater must fail closed on mode-read errors and keep disabled/AP mode as a successful pre-probe no-op")

wan_up = function_body(
    WAN_SOURCE, "wan_up_internal(char *wan_ifname, int unit, int is_static)"
)
notify_at = wan_up.index("route_status = notify_ddns_update();")
merge_at = wan_up.index(
    "status = route_status_accumulate(status, route_status);", notify_at
)
log_at = wan_up.index("WAN DDNS client update incomplete", merge_at)
require(notify_at < merge_at < log_at and
        "if (route_status != 0)" in wan_up[merge_at:log_at],
        "WAN-up must merge and log the DDNS notifier result instead of discarding it")
require("return status;" in wan_up[log_at:],
        "WAN-up must return the accumulated status after DDNS notification")


restore_ppp = function_body(
    WAN_SOURCE, "restore_wan_after_ppp_start_failure(int unit, char *wan_ifname)"
)
nvram_publish = function_body(
    WAN_SOURCE, "set_wan_unit_value_status(int unit, const char *param_name, const char *value)"
)
firewall_restore = function_body(
    WAN_SOURCE, "restore_wan_after_firewall_failure(int unit, char *wan_ifname)"
)
publish_at = restore_ppp.index('set_wan_unit_value_status(unit, "ifname_t", wan_ifname)')
publish_check_at = restore_ppp.index("if (nvram_status != 0)", publish_at)
firewall_at = restore_ppp.index("firewall_status = start_firewall_ex();")
require(publish_at < publish_check_at < firewall_at and
        "return nvram_status;" in restore_ppp[publish_check_at:firewall_at],
        "physical WAN interface must be published successfully before firewall rebuild")
require("down_status = ifconfig(wan_ifname, 0, NULL, NULL);" in firewall_restore,
        "failed physical-WAN restoration must keep the interface down")
errno_clear_at = nvram_publish.index("errno = 0;")
nvram_write_at = nvram_publish.index("api_result = nvram_set_temp(wanN_param, value);")
errno_capture_at = nvram_publish.index("saved_errno = errno;")
require(errno_clear_at < nvram_write_at < errno_capture_at and
        "rc_wan_nvram_write_status(api_result, saved_errno)" in nvram_publish,
        "checked WAN NVRAM helper must preserve errno and normalize API failure")
require("return saved_errno > 0 ? saved_errno : EIO;" in WAN_NVRAM_POLICY,
        "NVRAM failure without errno must map to EIO")
for marker in ("test_success_ignores_stale_errno",
               "test_failure_preserves_errno",
               "test_failure_without_errno_falls_back_to_eio"):
    require(marker in WAN_NVRAM_TEST, f"missing WAN NVRAM test: {marker}")

wan_wrapper = function_body(
    WAN_SOURCE, "wan_up(char *wan_ifname, int unit, int is_static)"
)
require("wan_up_internal(wan_ifname, unit, is_static)" in wan_wrapper and
        "return status;" in wan_wrapper,
        "wan_up must preserve the internal WAN-up result for its caller")

for marker in (
    "test_update_configuration_gate",
    "inadyn_update_should_run(1, 0)",
    "inadyn_update_should_run(0, 0)",
    "inadyn_update_should_run(1, 1)",
    "inadyn_update_should_run(0, 1)",
):
    require(marker in POLICY_TEST, f"host regression is missing {marker}")

for marker in (
    '"trunk/user/rc/src/services_ex.c"',
    '"trunk/user/rc/src/inadyn_daemon_policy.h"',
    '"trunk/user/rc/src/tests/inadyn_daemon_policy.c"',
    '"trunk/user/rc/src/wan_nvram_status_policy.h"',
    '"trunk/user/rc/src/tests/wan_nvram_status_policy.c"',
    "tests/wan_nvram_status_policy.c",
    '"scripts/check_wan_ddns_status.py"',
    "python3 scripts/check_wan_ddns_status.py",
    "tests/inadyn_daemon_policy.c",
):
    require(marker in WORKFLOW, f"WAN CI trigger or regression step is missing {marker}")
for path in ("trunk/user/rc/src/wan_nvram_status_policy.h",
             "trunk/user/rc/src/tests/wan_nvram_status_policy.c"):
    require(WORKFLOW.count('"' + path + '"') == 2,
            f"WAN CI must watch {path} on push and pull_request")

print("WAN-up DDNS failure propagation and disabled/AP no-op contracts: PASS")
