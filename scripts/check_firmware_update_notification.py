#!/usr/bin/env python3
"""Verify firmware-update failures remain visible to PID 1 notifications."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC_DIR = ROOT / "trunk/user/rc/src"
rc_source = (RC_DIR / "rc.c").read_text()
policy = (RC_DIR / "rc_notify_batch_policy.h").read_text()
policy_test = (RC_DIR / "tests/rc_notify_batch_policy.c").read_text()
stop_policy = (RC_DIR / "firmware_update_stop_policy.h").read_text()
stop_policy_test = (RC_DIR / "tests/firmware_update_stop_policy.c").read_text()


def block(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:pos]
    raise AssertionError("unterminated block: " + signature)


assert re.search(r"static\s+int\s+flash_firmware\s*\(\s*void\s*\)", rc_source)
assert '#include "rc_notify_batch_policy.h"' in rc_source
flash = block(rc_source, "flash_firmware(void)")
assert not re.search(r"\breturn\s*;", flash), "firmware aborts must not return success implicitly"
assert flash.count("return -1;") >= 10, "all preparation abort paths must return failure"

assert '#include "firmware_update_stop_policy.h"' in rc_source
assert "return operation_errno > 0 ? operation_errno : EIO;" in stop_policy
stop_calls = (
    "eapol_stop_status = stop_auth_eapol();",
    "kabinet_stop_status = stop_auth_kabinet();",
    "auth_stop_status = stop_8021x_all();",
    "pppoe_stop_status = set_passthrough_pppoe(0);",
)
for call in stop_calls:
    call_pos = flash.index(call)
    reset_pos = flash.rfind("errno = 0;", 0, call_pos)
    capture_pos = flash.index(
        "saved_errno = firmware_update_stop_first_errno(", call_pos
    )
    assert reset_pos >= 0 and reset_pos < call_pos < capture_pos, (
        "each firmware stop must clear errno and capture its result"
    )
assert flash.count("saved_errno = firmware_update_stop_first_errno(") == 4
auth_stop_failure = block(
    flash,
    "if (eapol_stop_status != 0 || kabinet_stop_status != 0 ||",
)
assert "if (saved_errno)" in auth_stop_failure
assert "errno = saved_errno;" in auth_stop_failure
assert "first errno=%d" in auth_stop_failure
assert "firmware_update_stop_first_errno(0, EACCES, 0) == 0" in stop_policy_test
assert "firmware_update_stop_first_errno(1, 0, 0) == EIO" in stop_policy_test
assert "first_errno == EBUSY" in stop_policy_test
write_call = 'flash_status = eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME);'
assert write_call in flash
assert "int upnp_stop_status, service_stop_status;" in flash
upnp_stop_call = "upnp_stop_status = stop_upnp();"
services_stop_call = "service_stop_status = stop_services(0);"
assert flash.count(upnp_stop_call) == 1
assert flash.count(services_stop_call) == 1
assert flash.index(upnp_stop_call) < flash.index(services_stop_call)
upnp_failure = block(flash, "if (upnp_stop_status != 0)")
assert "saved_errno = errno ? errno : EIO;" in upnp_failure
assert "MiniUPnPd did not stop" in upnp_failure
service_error_guard = "if (service_stop_status != 0 && saved_errno == 0)\n\t\tsaved_errno = errno ? errno : EIO;"
assert service_error_guard in flash
stop_failure = block(flash, "if (upnp_stop_status != 0 || service_stop_status != 0)")
assert "if (saved_errno)" in stop_failure
assert "errno = saved_errno;" in stop_failure
assert "return -1;" in stop_failure
assert flash.index("if (upnp_stop_status != 0 || service_stop_status != 0)") < flash.index("storage_time_status = storage_save_time(60)")
assert flash.index("if (upnp_stop_status != 0 || service_stop_status != 0)") < flash.index(write_call)
assert '#include "service_wait_policy.h"' in rc_source
assert "sleep(1);" not in flash, "pre-flash wait must not be shortened by EINTR"
pre_flash_wait = "service_wait_seconds(1)"
assert pre_flash_wait in flash
assert flash.index("sync();") < flash.index(pre_flash_wait) < flash.index(write_call)
wait_failure = block(flash, "if (service_wait_seconds(1) != 0)")
assert "int wait_errno = errno ? errno : EIO;" in wait_failure
assert "watchdog_status = start_watchdog();" in wait_failure
assert "errno = wait_errno;" in wait_failure
assert "return -1;" in wait_failure
write_failure = block(flash, "if (flash_status != 0)")
assert "int flash_errno = errno ? errno : EIO;" in write_failure
assert "firmware write command failed (status=%d, errno=%d)" in write_failure
assert "watchdog_status = start_watchdog();" in write_failure
assert "errno = flash_errno;" in write_failure
assert "return -1;" in write_failure
assert flash.rstrip().endswith("return 0;"), "only successful mtd_write may report success"

handler = block(rc_source, "handle_notifications(void)")
firmware_event = block(handler, 'else if (!strcmp(entry->d_name, "flash_firmware"))')
assert "stop_handle = 1;" in firmware_event
assert "action_result = rc_notify_merge_action_result(" in firmware_event
assert "action_result, flash_firmware());" in firmware_event
assert "if (action_result < 0)" in handler
assert "unlinkat(incomplete_dir_fd, entry->d_name, 0);" in handler
assert handler.index("if (action_result < 0)") < handler.index("unlinkat(incomplete_dir_fd, entry->d_name, 0);")

assert "(current_result < 0 || operation_status != 0) ? -1 : 0;" in policy
assert "rc_notify_merge_action_result(0, EBUSY) == -1" in policy_test
assert "rc_notify_merge_action_result(0, EIO) == -1" in policy_test

for relative in (
    ".github/workflows/process-lifecycle-regression.yml",
    ".github/workflows/xy-c1-firmware-validation.yml",
):
    workflow = (ROOT / relative).read_text()
    assert "scripts/check_firmware_update_notification.py" in workflow
    assert "Compile and run RC notification result policy regression" in workflow
    assert "python3 scripts/check_firmware_update_notification.py" in workflow
    assert "trunk/user/rc/src/firmware_update_stop_policy.h" in workflow
    assert "trunk/user/rc/src/tests/firmware_update_stop_policy.c" in workflow
    assert "Compile and run firmware-update first-stop errno policy regression" in workflow

print("Firmware-update abort/write status reaches PID 1 notification incomplete-marker handling: PASS")
