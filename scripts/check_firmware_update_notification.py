#!/usr/bin/env python3
"""Verify firmware-update failures remain visible to PID 1 notifications."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC_DIR = ROOT / "trunk/user/rc/src"
rc_source = (RC_DIR / "rc.c").read_text()
policy = (RC_DIR / "rc_notify_batch_policy.h").read_text()
policy_test = (RC_DIR / "tests/rc_notify_batch_policy.c").read_text()


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
write_call = 'flash_status = eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME);'
assert write_call in flash
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

print("Firmware-update abort/write status reaches PID 1 notification incomplete-marker handling: PASS")
