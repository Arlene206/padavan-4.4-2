#!/usr/bin/env python3
from pathlib import Path

init = Path("trunk/user/rc/src/init.c").read_text()
rc = Path("trunk/user/rc/src/rc.c").read_text()
policy = Path("trunk/user/rc/src/timezone_env_policy.h").read_text()
regression = Path("trunk/user/rc/src/tests/timezone_env_policy.c").read_text()
workflow = Path(".github/workflows/timezone-env-stability.yml").read_text()


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


timezone = body(rc, "setenv_tz(void)")
compact = " ".join(timezone.split())
assert '#include "timezone_env_policy.h"' in rc
assert 'nvram_get_checked("time_zone_x", &time_zone)' in compact
assert "rc_timezone_env_format_status(" in compact
assert compact.index("if (status != 0)") < compact.index('setenv("TZ", timezone_env + 3, 1)')
assert 'if (setenv("TZ", timezone_env + 3, 1) != 0)' in compact
assert "errno ? errno : EIO" in compact
assert "nvram_safe_get" not in timezone
assert "putenv(" not in timezone

for marker in (
    "read_result < 0",
    "read_result > 1",
    "if (read_result == 0)",
    "if (!time_zone)",
    "value_length > destination_size - 4U",
    'memcpy(destination, "TZ=", 3U)',
):
    assert marker in policy, marker

for marker in (
    "test_present_value_is_formatted_exactly",
    "test_missing_key_preserves_legacy_empty_timezone",
    "test_backend_error_leaves_destination_untouched",
    "test_invalid_read_results_and_present_null_are_rejected",
    "test_timezone_length_boundary_is_checked_without_truncation",
    "RC timezone environment checked-read policy: PASS",
):
    assert marker in regression, marker

paths = (
    ".github/workflows/timezone-env-stability.yml",
    "scripts/check_timezone_env.py",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/timezone_env_policy.h",
    "trunk/user/rc/src/tests/timezone_env_policy.c",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  workflow_dispatch:", 1)[0].split("  pull_request:", 1)[1]
for path in paths:
    assert push.count('"' + path + '"') == 1, path
    assert pr.count('"' + path + '"') == 1, path

recovery_shell = body(init, "run_shell(int timeout, int nowait)")
recovery_compact = "".join(recovery_shell.split())
assert '#include "timezone_env_policy.h"' in init
assert 'nvram_get_checked("time_zone_x",&time_zone)' in recovery_compact
assert "rc_timezone_env_format_status(" in recovery_compact
assert recovery_compact.index("if(tz_status!=0)") < recovery_compact.index("envp[6]=NULL")
assert "time_zone_x_mapping();" in recovery_compact
assert 'nvram_safe_get("time_zone_x")' not in recovery_compact

assert "setenv_tz();" in body(rc, "set_timezone(void)")
print("RC timezone checked-read and failure-preserving integration: PASS")
