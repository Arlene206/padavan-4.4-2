#!/usr/bin/env python3
from pathlib import Path

rc = Path("trunk/user/rc/src/rc.c").read_text()
policy = Path("trunk/user/rc/src/console_loglevel_policy.h").read_text()
regression = Path("trunk/user/rc/src/tests/console_loglevel_policy.c").read_text()
workflow = Path(".github/workflows/rc-console-loglevel.yml").read_text()


def body(source, signature):
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
    raise AssertionError("unterminated function: " + signature)


restore = body(rc, "nvram_restore_defaults(void)")
compact = "".join(restore.split())
assert '#include "console_loglevel_policy.h"' in rc
assert 'nvram_get_checked("console_loglevel",&console_loglevel_text)' in compact
assert "rc_console_loglevel_parse_status(" in compact
assert compact.index("if(console_loglevel_status!=0)") < compact.index("klogctl(8,NULL,console_loglevel)")
assert "retaining kernel default" in restore
assert "if(klogctl(8,NULL,console_loglevel)<0)" in compact
assert 'nvram_get_int("console_loglevel")' not in restore

for marker in (
    "read_result < 0",
    "read_result > 1",
    "read_result == 1 && !value",
    "read_result == 1) ? atoi(value) : 0",
):
    assert marker in policy, marker

for marker in (
    "test_present_console_loglevel_keeps_legacy_integer_conversion",
    "test_missing_console_loglevel_keeps_legacy_zero_default",
    "test_backend_errors_do_not_publish_a_new_level",
    "test_invalid_results_and_arguments_fail_closed",
    "RC console loglevel checked-read policy: PASS",
):
    assert marker in regression, marker

paths = (
    ".github/workflows/rc-console-loglevel.yml",
    "scripts/check_console_loglevel.py",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/console_loglevel_policy.h",
    "trunk/user/rc/src/tests/console_loglevel_policy.c",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  workflow_dispatch:", 1)[0].split("  pull_request:", 1)[1]
for path in paths:
    assert push.count('"' + path + '"') == 1, path
    assert pr.count('"' + path + '"') == 1, path

print("RC console loglevel checked-read and kernel-default preservation: PASS")
