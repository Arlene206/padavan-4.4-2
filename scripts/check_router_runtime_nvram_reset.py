#!/usr/bin/env python3
"""Verify fail-closed production wiring for per-boot runtime NVRAM resets."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:position]
    raise AssertionError("unterminated function: " + signature)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> None:
    rc_path = ROOT / "trunk/user/rc/src/rc.c"
    policy_path = ROOT / "trunk/user/rc/src/router_runtime_nvram_reset_policy.h"
    test_path = ROOT / "trunk/user/rc/src/tests/router_runtime_nvram_reset_policy.c"
    workflow_path = ROOT / ".github/workflows/router-runtime-nvram-reset.yml"

    rc = rc_path.read_text()
    policy = policy_path.read_text()
    test = test_path.read_text()
    workflow = workflow_path.read_text()

    require('#include "router_runtime_nvram_reset_policy.h"' in rc,
            "rc.c must include the tested batch-reset policy")
    setter = function_body(rc, "rc_router_runtime_nvram_set(void *context,")
    require("nvram_set_int_temp(name, integer_value)" in setter and
            "nvram_set_temp(name, string_value)" in setter,
            "production writer must use the matching integer/string NVRAM APIs")

    table = function_body(rc, "reset_router_runtime_nvram_state(int sw_mode)")
    keys = (
        "login_timestamp", "networkmap_fullscan", "link_internet", "link_wan",
        "led_front_t", "led_ether_t", "reload_svc_wl", "reload_svc_rt",
        "usb_hotplug_ms", "usb_hotplug_lp", "usb_hotplug_md", "usb_unplug_lp",
        "usb_unplug_md", "usb_opt_start", "l2tp_wan_t", "l2tp_cli_t",
        "l2tp_srv_t", "vpnc_state_t", "deferred_wanup_t", "vpnc_dns_t",
        "vpnc_dom_t", "viptv_ifname", "RSTATS_NVKEY_24", "RSTATS_NVKEY_DM",
    )
    for key in keys:
        require(key in table, "runtime reset table is missing " + key)
    require("router_runtime_nvram_reset_apply(" in table,
            "runtime reset table must use the tested all-attempts helper")

    converter = function_body(rc, "nvram_convert_misc_values(void)")
    reset_call = converter.find("status = reset_router_runtime_nvram_state(sw_mode);")
    reset_check = converter.find("if (status != 0)", reset_call)
    reset_return = converter.find("return status;", reset_check)
    wan_start = converter.find("status = set_wan0_vars();")
    require(reset_call >= 0 and reset_check > reset_call and
            reset_return > reset_check and wan_start > reset_return,
            "startup must abort before WAN/service setup after any runtime reset failure")

    require("first_status = status;" in policy and
            "first_errno = saved_errno;" in policy and
            "errno = first_errno;" in policy,
            "batch helper must preserve first status and errno")
    for marker in (
        "test_all_resets_are_attempted_and_first_error_wins",
        "test_first_error_with_zero_errno_does_not_leak_later_errno",
        "test_invalid_batch_has_no_side_effects",
    ):
        require(marker in test, "missing regression case " + marker)
    require("python3 scripts/check_router_runtime_nvram_reset.py" in workflow,
            "workflow must execute production source contract")
    for path in (
        ".github/workflows/router-runtime-nvram-reset.yml",
        "scripts/check_router_runtime_nvram_reset.py",
        "trunk/user/rc/src/rc.c",
        "trunk/user/rc/src/router_runtime_nvram_reset_policy.h",
        "trunk/user/rc/src/tests/router_runtime_nvram_reset_policy.c",
    ):
        require(f'"{path}"' in workflow, "workflow path filter is missing " + path)

    print("Router runtime NVRAM reset fail-closed source contract: PASS")


if __name__ == "__main__":
    main()
