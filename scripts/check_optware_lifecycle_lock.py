#!/usr/bin/env python3
"""Source contracts for the pre-provisioned Optware lifecycle lock."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HELPER = ROOT / "trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh"
DEV_INIT = ROOT / "trunk/user/scripts/files/sbin/dev_init.sh"
DEV_INIT_CHECK = ROOT / "scripts/check_dev_init_boot.py"
LOCK_TEST = ROOT / "trunk/user/scripts/tests/optware_lifecycle_lock.sh"
START_TEST = ROOT / "trunk/user/scripts/tests/optware_lifecycle_startup.sh"
STOP_TEST = ROOT / "trunk/user/scripts/tests/optware_lifecycle_teardown.sh"
START_CALLER = ROOT / "trunk/user/optware/opt-start.sh"
MOUNT_CALLER = ROOT / "trunk/user/optware/opt-mount.sh"
STOP_CALLER = ROOT / "trunk/user/optware/opt-umount.sh"
WORKFLOW = ROOT / ".github/workflows/process-lifecycle-regression.yml"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(message)


helper = HELPER.read_text()
dev_init = DEV_INIT.read_text()
dev_check = DEV_INIT_CHECK.read_text()
lock_test = LOCK_TEST.read_text()
start_test = START_TEST.read_text()
stop_test = STOP_TEST.read_text()
start_caller = START_CALLER.read_text()
mount_caller = MOUNT_CALLER.read_text()
stop_caller = STOP_CALLER.read_text()
workflow = WORKFLOW.read_text()

start = helper.index("_optware_lifecycle_lock_open()")
end = helper.index("\n}\n\noptware_lifecycle_lock_acquire()", start)
open_policy = helper[start:end]

require("optware_lock_file=" + chr(36) + "{1:-/tmp/optware-lifecycle.lock}" in open_policy,
        "Optware default lock must live in sticky /tmp")
require('case "$optware_lock_file" in\n\t/*)' in open_policy,
        "Optware lock path must be absolute")
for token in (
    '[ -L "$optware_lock_dir" ]',
    '[ ! -d "$optware_lock_dir" ]',
    '[ -L "$optware_lock_file" ]',
    '[ ! -f "$optware_lock_file" ]',
    'id -u',
    'stat -c %u "$optware_lock_dir"',
    'stat -c %a "$optware_lock_dir"',
    'optware_lock_dir_mode_value & 0022',
    'optware_lock_dir_mode_value & 01000',
    'stat -c %u "$optware_lock_file"',
    'stat -c %h "$optware_lock_file"',
    'stat -c %a "$optware_lock_file"',
    '[ "$optware_lock_links" != 1 ]',
    '[ "$optware_lock_mode" != 600 ]',
    'exec 9>>"$optware_lock_file"',
):
    require(token in open_policy, "missing Optware lock safety check: " + token)
require(': >> "$optware_lock_file"' not in open_policy and
        'umask 077' not in open_policy,
        "Optware caller must not create or chmod the shared lock")
require('_optware_lifecycle_lock_open "$1"' in helper and
        'flock 9' in helper and 'flock -n 9' in helper,
        "blocking start and nonblocking teardown must share one checked lock opener")

tmp_setup = dev_init.index("required_dir 1777 /tmp || exit 1")
optware_setup = dev_init.index(
    "required_lock_file /tmp/optware-lifecycle.lock || exit 1")
mdev_scan = dev_init.index("mdev -s || exit 1")
require(tmp_setup < optware_setup < mdev_scan,
        "dev_init must pre-create Optware's private lock in sticky /tmp before mdev")

for label, content in (
    ("lock", lock_test),
    ("startup", start_test),
    ("teardown", stop_test),
):
    require('umask 077; : > "$' in content,
            label + " test must provision an owner-only lock fixture")
require('optware_lifecycle_lock_acquire "$lock_file"' in lock_test,
        "lock regression must exercise blocking acquisition")
for marker in (
    "Optware lifecycle lock followed a symbolic link",
    "Optware lifecycle lock accepted a multiply-linked inode",
    "Optware lifecycle lock accepted a non-private file mode",
    "Optware lifecycle lock implicitly created a missing file",
    "Optware lifecycle lock accepted a writable non-sticky directory",
):
    require(marker in lock_test, "missing Optware lock regression: " + marker)

require('. /sbin/optware_lifecycle_lock.sh || exit 1' in start_caller and
        'optware_lifecycle_lock_acquire' in start_caller,
        "Optware boot startup must use the checked blocking lock")
require('. /sbin/optware_lifecycle_lock.sh || exit 1' in mount_caller and
        'optware_lifecycle_lock_acquire' in mount_caller,
        "Optware mount must serialize with service startup")
require('. /sbin/optware_lifecycle_lock.sh || exit 1' in stop_caller and
        'optware_lifecycle_lock_try_acquire' in stop_caller,
        "Optware teardown must retain the nonblocking lock-order policy")

require("optware_lock_provision" in dev_check and
        "required_lock_file /tmp/optware-lifecycle.lock || exit 1" in dev_check,
        "dev_init source checker must pin Optware lock provisioning order")
for path in (
    "scripts/check_optware_lifecycle_lock.py",
    "trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh",
    "trunk/user/scripts/tests/optware_lifecycle_lock.sh",
    "trunk/user/scripts/tests/optware_lifecycle_startup.sh",
    "trunk/user/scripts/tests/optware_lifecycle_teardown.sh",
):
    require(workflow.count('      - "' + path + '"') == 2,
            "both Process workflow events must watch " + path)
require("sh trunk/user/scripts/tests/optware_lifecycle_lock.sh" in workflow and
        "sh trunk/user/scripts/tests/optware_lifecycle_startup.sh" in workflow and
        "sh trunk/user/scripts/tests/optware_lifecycle_teardown.sh" in workflow,
        "Process workflow must run all three Optware lifecycle regressions")

print("Optware pre-provisioned private lifecycle lock source contract: PASS")
