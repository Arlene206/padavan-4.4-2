#!/usr/bin/env python3
"""Verify modem driver alternatives are attempted and readiness is propagated."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
USB = (RC / "usb_modem.c").read_text()
RC_SOURCE = (RC / "rc.c").read_text()
HEADER = (RC / "rc.h").read_text()
POLICY = (RC / "modem_module_reload_policy.h").read_text()
TEST = (RC / "tests/modem_module_reload_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/usb-modem-identity-stability.yml").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


reload = body(USB, "reload_modem_modules(int modem_type, int reload)")
assert "static const char *const ndis_modules[]" in reload
assert "static const char *const ndis_params[]" in reload
for driver in ("cdc_ether", "rndis_host", "qmi_wwan", "cdc_ncm",
               "huawei_cdc_ncm", "cdc_mbim", "sierra_net"):
    assert f'"{driver}"' in reload
assert '"prefer_mbim=0"' in reload
for driver in ("cdc_acm", "qcserial", "sierra", "option"):
    assert f'"{driver}"' in reload
assert 'load_modem_module_group("NDIS"' in reload
assert 'load_modem_module_group("RAS"' in reload
assert "return status;" in reload
assert "if (reload)" in reload and "sleep(1);" in reload

loader = body(USB, "load_modem_module_group(")
assert "module_smart_load" in loader and "is_module_loaded_status" in loader
assert "is_module_loaded(module_name)" not in loader
assert "rc_modem_module_load_group_verified" in loader

assert "int  reload_modem_modules(int modem_type, int reload);" in HEADER
boot = body(RC_SOURCE, "load_usb_modules(")
assert "rc_first_service_start_status" in boot
boot_reader = body(
    RC_SOURCE,
    "rc_boot_nvram_read_int_status(const char *name, int missing_value, int *value)",
)
assert "nvram_get_checked(name, &text)" in boot_reader
assert "rc_wan_nvram_read_int_status(api_result, saved_errno" in boot_reader
assert "saved_errno = errno;" in boot_reader
assert "errno = status;" in boot_reader

assert 'rc_boot_nvram_read_int_status("modem_rule", 0,' in boot
assert 'rc_boot_nvram_read_int_status("modem_type", 0,' in boot
assert 'reload_modem_modules(modem_type, 0)' in boot
assert 'nvram_get_int("modem_rule")' not in boot
assert 'nvram_get_int("modem_type")' not in boot
restore = body(RC_SOURCE, "nvram_restore_defaults(void)")
assert "rc_modem_read_nvram_settings_status(" in restore
assert "cannot read USB modem configuration; aborting router startup" in restore
assert 'nvram_get_int("modem_rule")' not in restore
assert 'nvram_get_int("modem_type")' not in restore
assert "rc_modem_read_nvram_settings_status" in POLICY
for marker in (
    "test_modem_settings_snapshot_reads_complete_pair",
    "test_modem_settings_first_read_error_publishes_no_values",
    "test_modem_settings_second_read_error_discards_partial_pair",
    "test_modem_settings_absent_keys_keep_legacy_defaults",
    "test_modem_settings_reject_invalid_arguments",
    "test_module_reload_cache_published_only_after_success",
):
    assert marker in TEST

rule_read = boot.index('rc_boot_nvram_read_int_status("modem_rule", 0,')
rule_error = boot.index("if (nvram_status != 0)", rule_read)
rule_enable = boot.index("else if (modem_rule > 0)", rule_error)
type_read = boot.index('rc_boot_nvram_read_int_status("modem_type", 0,', rule_enable)
type_error = boot.index("if (nvram_status != 0)", type_read)
reload_call = boot.index("reload_modem_modules(modem_type, 0)", type_error)
usb_host = boot.index("/* start usb host */")
assert rule_read < rule_error < rule_enable < type_read < type_error < reload_call < usb_host
assert "rc_first_service_start_status(start_status" in boot[rule_error:type_read]
assert "rc_first_service_start_status(start_status" in boot[type_error:reload_call]
assert "nvram_get_int(" not in boot[rule_read:usb_host]


remove = body(USB, "safe_remove_usb_modem(void)")
transport_reader = body(USB, "usb_modem_read_transport_status(int *is_ndis)")
assert 'nvram_get_checked("modem_type", &value)' in transport_reader
assert "rc_wan_nvram_read_int_status(api_result, saved_errno" in transport_reader
assert "rc_wan_nvram_modem_is_ndis_status(status, modem_type" in transport_reader
assert "if (status != 0)" in transport_reader and "errno = status;" in transport_reader
assert "usb_modem_read_transport_status(&is_ndis)" in remove
transport_failure = remove.index("if (modem_type_status != 0)")
transport_return = remove.index("return -1;", transport_failure)
ndis_branch = remove.index("if (is_ndis)")
assert transport_failure < transport_return < ndis_branch
assert transport_return < remove.index("stop_udhcpc_wan(0)")
assert transport_return < remove.index('kill_services_with_pidfiles(NULL, pidfiles, "pppd"')
assert 'nvram_get_int("modem_type")' not in remove

notify_start = RC_SOURCE.index('else if (!strcmp(entry->d_name, RCN_RESTART_MODEM))')
notify_end = RC_SOURCE.index('else if (strcmp(entry->d_name, RCN_RESTART_SPOOLER)', notify_start)
notify = RC_SOURCE[notify_start:notify_end]
notify_config = notify.index("rc_modem_read_nvram_settings_status(")
notify_config_error = notify.index("if (modem_settings_status != 0)", notify_config)
notify_identity = notify.index("if (!transition_failed)", notify_config_error)
assert notify_config < notify_config_error < notify_identity
assert 'nvram_get_int("modem_rule")' not in notify
assert 'nvram_get_int("modem_type")' not in notify
assert "if (!transition_failed && nvram_modem_rule != modem_rule)" in notify
assert notify.count("if (!modem_wan_restart_allowed(modem_identity_status))") == 1
assert notify.count("modem_identity_status = get_usb_modem_wan_status(") == 1
assert notify.index("if (!transition_failed && nvram_modem_rule != modem_rule)") > notify_config_error
assert "stop_failed" not in notify
assert notify.count("rc_modem_module_reload_cache_publish(") == 2
assert notify.count("reload_modem_modules(modem_type, 1)") == 2

rule_change = notify.index("if (!transition_failed && nvram_modem_rule != modem_rule)")
rule_publish = notify.index("rc_modem_module_reload_cache_publish(", rule_change)
rule_failure = notify.index("if (module_reload_status != 0)", rule_publish)
rule_failed = notify.index("transition_failed = 1;", rule_failure)
rule_ready = notify.index("modules_reloaded = 1;", rule_failed)
rule_unload = notify.index("unload_modem_modules();", rule_change)
rule_disabled_cache = notify.index("nvram_modem_rule = modem_rule;", rule_unload)
assert rule_change < rule_publish < rule_failure < rule_failed < rule_ready
assert rule_unload < rule_disabled_cache and rule_disabled_cache > rule_ready
assert "reload_modem_modules(modem_type, 1)" in notify[rule_publish:rule_failure]
assert "action_result = rc_notify_merge_action_result(" in notify[rule_failure:rule_ready]

type_change = notify.index("if (!transition_failed && nvram_modem_type != modem_type)")
type_publish = notify.index("rc_modem_module_reload_cache_publish(", type_change)
type_failure = notify.index("if (module_reload_status != 0)", type_publish)
type_failed = notify.index("transition_failed = 1;", type_failure)
type_cache = notify.index("nvram_modem_type = modem_type;", type_failed)
wan_restart = notify.index("if (!transition_failed && need_restart_wan)")
assert type_change < type_publish < type_failure < type_failed < type_cache < wan_restart
assert "reload_modem_modules(modem_type, 1)" in notify[type_publish:type_failure]
assert "retaining previous cache for retry" in notify
cache_publish = body(POLICY, "rc_modem_module_reload_cache_publish(")
assert "if (!cached_value)" in cache_publish and "return EINVAL;" in cache_publish
assert "if (reload_status != 0)" in cache_publish
assert "return reload_status > 0 ? reload_status : EIO;" in cache_publish
assert "*cached_value = requested_value;" in cache_publish
assert notify.index("if (!transition_failed && need_restart_wan)") > type_failed

assert "if (probe_status == 1)" in POLICY
assert "probe_status < 0" in POLICY
assert "probe_status > 1" in POLICY
assert "first_probe_error" in POLICY
assert "return first_probe_error;" in POLICY
assert "return ENODEV;" in POLICY
assert "Validate the entire group before invoking any loader." in POLICY
for marker in (
    "test_one_available_alternative_is_sufficient",
    "test_already_loaded_alternative_counts_as_ready",
    "test_all_missing_alternatives_fail_the_group",
    "test_unknown_probe_is_not_counted_as_available",
    "test_invalid_probe_value_fails_closed",
    "test_driver_specific_parameter_is_forwarded",
    "test_invalid_group_fails_before_side_effects",
    "USB modem module alternative readiness policy: PASS",
):
    assert marker in TEST

FILTERS = WORKFLOW.split("permissions:", 1)[0]
for path in (
    "scripts/check_modem_module_reload.py",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/modem_module_reload_policy.h",
    "trunk/user/rc/src/tests/modem_module_reload_policy.c",
):
    assert FILTERS.count(path) == 2, f"push/pull_request path filter missing: {path}"
assert "python3 scripts/check_modem_module_reload.py" in WORKFLOW

print("USB modem module alternatives and readiness propagation contract: PASS")
