#!/usr/bin/env python3
"""Guard the enabled-DDNS WAN-up result contract and disabled/AP no-op cases."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN_SOURCE = (RC / "net_wan.c").read_text()
SERVICES_SOURCE = (RC / "services_ex.c").read_text()
POLICY_SOURCE = (RC / "inadyn_daemon_policy.h").read_text()
POLICY_TEST = (RC / "tests/inadyn_daemon_policy.c").read_text()
CONFIG_TEST = (RC / "tests/inadyn_config_file.c").read_text()
NVRAM_POLICY = (RC / "wan_nvram_status_policy.h").read_text()
WAN_NVRAM_POLICY = (RC / "wan_nvram_status_policy.h").read_text()
WAN_NVRAM_TEST = (RC / "tests/wan_nvram_status_policy.c").read_text()
DDNS_ENABLE_POLICY = (RC / "ddns_enabled_policy.h").read_text()
RC_SOURCE = (RC / "rc.c").read_text()
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
enable_read = notify.index('nvram_get_checked("ddns_enable_x", &enabled_value)')
enable_check = notify.index("inadyn_ddns_enabled_status(", enable_read)
gate = notify.index("inadyn_update_should_run(", enable_check)
inventory = notify.index('process_inventory_scan("inadyn", &snapshot)')
require(mode_read < mode_error < enable_read < enable_check < gate < inventory and
        'nvram_match("ddns_enable_x", "1")' not in notify and
        "return ap_mode_status;" in notify[mode_error:enable_read] and
        "return enabled_status;" in notify[enable_check:gate] and
        "return 0;" in notify[gate:inventory],
        "DDNS updater must check enable-state reads and preserve disabled/AP no-op behavior")

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

writer = function_body(SERVICES_SOURCE, "write_inadyn_conf(const char *conf_file)")
snapshot_call = writer.index("rc_wan_nvram_copy_snapshot(")
parse_first = writer.index('inadyn_parse_config_integer("ddns_period"')
config_open = writer.index("inadyn_config_file_open(conf_file")
require(snapshot_call < parse_first < config_open,
        "Inadyn writer must snapshot configuration before validating/staging")
for key in (
    "ddns_period", "ddns_forced", "ddns_source", "ddns_ipv6", "ddns_ssl",
    "ddns_wildcard_x", "ddns2_ssl", "ddns2_wildcard_x", "ddns_checkip",
    "ddns2_checkip", "ddns_server_x", "ddns_hostname_x", "ddns_hostname2_x",
    "ddns_hostname3_x", "ddns_username_x", "ddns_passwd_x", "ddns_cst_svr",
    "ddns_cst_url", "ddns2_server", "ddns2_hname", "ddns2_user", "ddns2_pass",
):
    require(f'"{key}"' in SERVICES_SOURCE, f"Inadyn config snapshot omits {key}")
require("nvram_safe_get(" not in writer and
        "nvram_safe_get_int(" not in writer and
        "inadyn_read_nvram_integer(" not in writer,
        "Inadyn writer must avoid lossy getters and rotating cache pointers")
for signature, api_call in (
    ("inadyn_nvram_snapshot_begin(void *context)", "nvram_transaction_begin"),
    ("inadyn_nvram_snapshot_get(void *context, const char *name, char **value)",
     "nvram_transaction_get"),
    ("inadyn_nvram_snapshot_end(void *context)", "nvram_transaction_end"),
):
    require(api_call in function_body(SERVICES_SOURCE, signature),
            f"Inadyn snapshot adapter must call {api_call}")
for marker in (
    "test_nvram_snapshot_copies_rotating_cache_and_invalidates_failed_tuple",
    "fake_nvram_snapshot_get", "fake.end_status = EBUSY",
    "fake.get_failure_status = EACCES",
):
    require(marker in CONFIG_TEST, f"Inadyn snapshot regression missing {marker}")
require("rc_wan_nvram_copy_snapshot(" in NVRAM_POLICY and
        "fields[i].destination[0] = '\\0';" in NVRAM_POLICY,
        "NVRAM snapshot helper must clear partial tuple data on error")
require("inadyn_ddns_enabled_status" in DDNS_ENABLE_POLICY,
        "DDNS enable-state parser must preserve backend errors")
ddns_start = RC_SOURCE.index('else if (strcmp(entry->d_name, RCN_RESTART_DDNS) == 0)')
ddns_end = RC_SOURCE.index('else if (strcmp(entry->d_name, RCN_RESTART_DI)', ddns_start)
ddns_restart = RC_SOURCE[ddns_start:ddns_end]
ddns_restart_compact = " ".join(ddns_restart.split())
require('nvram_get_checked("ddns_enable_x", &ddns_enabled_value)' in ddns_restart_compact and
        "inadyn_ddns_enabled_status(" in ddns_restart_compact and
        "ddns_status = ddns_enabled_status;" in ddns_restart_compact and
        'nvram_match("ddns_enable_x", "1")' not in ddns_restart_compact,
        "RC DDNS restart must preserve state on a backend error")

for marker in (
    '"trunk/user/rc/src/services_ex.c"',
    '"trunk/user/rc/src/inadyn_daemon_policy.h"',
    '"trunk/user/rc/src/tests/inadyn_daemon_policy.c"',
    '"trunk/user/rc/src/inadyn_config_file.h"',
    '"trunk/user/rc/src/tests/inadyn_config_file.c"',
    '"trunk/user/rc/src/wan_nvram_status_policy.h"',
    '"trunk/user/rc/src/ddns_enabled_policy.h"',
    '"trunk/user/rc/src/tests/wan_nvram_status_policy.c"',
    "tests/wan_nvram_status_policy.c",
    '"scripts/check_wan_ddns_status.py"',
    "python3 scripts/check_wan_ddns_status.py",
    "tests/inadyn_daemon_policy.c",
    "tests/inadyn_config_file.c",
):
    require(marker in WORKFLOW, f"WAN CI trigger or regression step is missing {marker}")
for path in ("trunk/user/rc/src/wan_nvram_status_policy.h",
             "trunk/user/rc/src/ddns_enabled_policy.h",
             "trunk/user/rc/src/tests/wan_nvram_status_policy.c"):
    require(WORKFLOW.count('"' + path + '"') == 2,
            f"WAN CI must watch {path} on push and pull_request")
for path in ("trunk/user/rc/src/inadyn_config_file.h",
             "trunk/user/rc/src/tests/inadyn_config_file.c"):
    require(WORKFLOW.count('"' + path + '"') == 2,
            f"WAN CI must watch {path} on push and pull_request")

print("WAN-up DDNS failure propagation and disabled/AP no-op contracts: PASS")
