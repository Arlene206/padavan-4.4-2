#!/usr/bin/env python3
"""Verify modem driver alternatives are attempted and readiness is propagated."""

from pathlib import Path
import re

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
assert "module_smart_load" in loader and "is_module_loaded" in loader
assert "rc_modem_module_load_group_verified" in loader

assert "int  reload_modem_modules(int modem_type, int reload);" in HEADER
boot = body(RC_SOURCE, "load_usb_modules(")
assert "rc_first_service_start_status" in boot
assert 'reload_modem_modules(nvram_get_int("modem_type"), 0)' in boot

notify_start = RC_SOURCE.index('else if (!strcmp(entry->d_name, RCN_RESTART_MODEM))')
notify_end = RC_SOURCE.index('else if (strcmp(entry->d_name, RCN_RESTART_SPOOLER)', notify_start)
notify = RC_SOURCE[notify_start:notify_end]
notify_pattern = (
    r"rc_notify_merge_action_result\s*\(\s*action_result\s*,\s*"
    r"reload_modem_modules\(modem_type, 1\)\s*\)"
)
assert len(re.findall(notify_pattern, notify)) == 2

assert "available > 0 ? 0 : ENODEV" in POLICY
assert "Validate the entire group before invoking any loader." in POLICY
for marker in (
    "test_one_available_alternative_is_sufficient",
    "test_already_loaded_alternative_counts_as_ready",
    "test_all_missing_alternatives_fail_the_group",
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
