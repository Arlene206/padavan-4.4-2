#!/usr/bin/env python3
"""Check SCUT crontab result semantics and RC boot persistence propagation."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
rc = (root / "trunk/user/rc/src/rc.c").read_text()
script = (root / "trunk/user/scripts/files/sbin/check_crontab.sh").read_text()
shell_test = (root / "trunk/user/scripts/tests/check_crontab_status.sh").read_text()
policy = (root / "trunk/user/rc/src/crontab_update_status.h").read_text()
regression = (root / "trunk/user/rc/src/tests/crontab_update_status.c").read_text()
workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def c_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError("unterminated C body: " + signature)


def block_body(source, marker):
    start = source.index(marker)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError("unterminated C block: " + marker)


assert '#include "crontab_update_status.h"' in rc
crontab = c_body(rc, "init_crontab(void)")
assert "command_status = system(" in crontab
assert "command_errno = errno;" in crontab
assert "rc_crontab_update_status_from_wait(command_status, command_errno)" in crontab
assert "rc_crontab_update_status_merge(ret, update_status)" in crontab
assert "ret |= system(" not in crontab

router = c_body(rc, "init_router(void)")
assert "int crontab_status = init_crontab();" in router
assert "rc_crontab_update_requires_persistence(crontab_status)" in router
persist = block_body(router, "if (rc_crontab_update_requires_persistence(crontab_status))")
normalized = " ".join(persist.split())
ordered = (
    "storage_save_status = write_storage_to_mtd();",
    "boot_services_status = rc_first_service_start_status( boot_services_status, storage_save_status);",
    "crond_status = restart_crond();",
)
positions = [normalized.index(token) for token in ordered]
assert positions == sorted(positions), "MTD persistence error must be recorded before crond restart"
assert "MTD storage persistence failed" in persist
error_branch = block_body(router, "else if (crontab_status != 0)")
assert "boot_services_status = rc_first_service_start_status(" in error_branch
assert "initial crontab update failed" in error_branch

for required in (
    "cd /etc/storage || exit 2",
    "[ ! -L cron ] || exit 2",
    "[ ! -L cron/crontabs ] || exit 2",
    '[ ! -L "$cron_file" ] || exit 2',
    'grep -F -- "$script_path" "$cron_file"',
    "grep_status=$?",
    "1) ;;",
    "*) exit 2 ;;",
    '>> "$cron_file" || exit 2',
    "exit 1",
):
    assert required in script, "missing check_crontab result contract: " + required
assert script.index("grep_status=$?") < script.index('>> "$cron_file"')
assert "run_check 1" in shell_test and "run_check 0" in shell_test
assert "TEST_GREP_STATUS=2" in shell_test
assert 'ln -s "$WORK/outside" "$TEST_STORAGE_ROOT/cron"' in shell_test
assert "SIGTERM" in regression and "WIFEXITED" in policy
assert "wait_errno > 0 ? wait_errno : -1" in policy
assert "return status == 1;" in policy

for path in (
    "scripts/check_crontab_startup_status.py",
    "trunk/user/rc/src/crontab_update_status.h",
    "trunk/user/rc/src/tests/crontab_update_status.c",
    "trunk/user/scripts/files/sbin/check_crontab.sh",
    "trunk/user/scripts/tests/check_crontab_status.sh",
):
    path_pattern = re.compile(r'^      - "' + re.escape(path) + r'"$', re.MULTILINE)
    assert len(path_pattern.findall(workflow)) == 2, "push/PR path filters must include " + path
assert "python3 scripts/check_crontab_startup_status.py" in workflow
assert "trunk/user/rc/src/tests/crontab_update_status.c" in workflow
assert "trunk/user/scripts/tests/check_crontab_status.sh" in workflow
assert '"trunk/user/scripts/files/sbin/check_crontab.sh"' in xy_workflow

print("SCUT crontab update status, persistence and CI contract: PASS")
