#!/usr/bin/env python3
"""Ensure XUPnPD private storage setup fails before daemon launch."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
source = (RC / "net.c").read_text()
header = (RC / "xupnpd_config_file.h").read_text()
test = (RC / "tests/xupnpd_config_file.c").read_text()
daemon_policy = (RC / "xupnpd_daemon_policy.h").read_text()
detached_policy = (RC / "detached_daemon_start_policy.h").read_text()
daemon_test = (RC / "tests/xupnpd_daemon_policy.c").read_text()


def function_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    index = brace
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


startup = function_body(source, "start_xupnpd(char *wan_ifname)")
assert "xupnpd_config_ensure_directory(dir_dst, 0755)" in startup
assert "xupnpd_config_ensure_directory(tmp2, 0755)" in startup
assert "copy_storage = get_mtd_size(\"Storage\") > 0x10000;" in startup
assert "xupnpd_config_subdirectory_validate(" in startup

copy_tree = 'copy_status = doSystem("cp -rf %s %s", tmp1, tmp2);'
copy_lua = 'copy_status = doSystem("cp -f %s %s", tmp1, tmp2);'
assert startup.count(copy_tree) == 1
assert startup.count(copy_lua) == 1
assert re.search(r'(?m)^\s*doSystem\("cp -rf %s %s"', startup) is None
assert re.search(r'(?m)^\s*doSystem\("cp -f %s %s"', startup) is None

copy_tree_failure = startup.split(copy_tree, 1)[1].split("} else if", 1)[0]
assert "if (copy_status != 0)" in copy_tree_failure
assert "return copy_status;" in copy_tree_failure
copy_lua_failure = startup.split(copy_lua, 1)[1].split("\n\t\t}\n", 1)[0]
assert "if (copy_status != 0)" in copy_lua_failure
assert "return copy_status;" in copy_lua_failure

for marker in (
    "symlink(tmp1, tmp2) != 0 && errno != EEXIST",
    'unlink("/var/run/xupnpd.pid") != 0 && errno != ENOENT',
    "unlink(tmp2) != 0 && errno != ENOENT",
    "lstat(tmp2, &lua_stat)",
    "S_ISLNK(lua_stat.st_mode)",
    "xupnpd_config_directory_validate(tmp1)",
):
    assert marker in startup, marker

assert "mkdir(tmp2, 0755)" not in startup
assert "symlink(tmp1, tmp2);" not in startup
for marker in (
    "test_ensure_directory_creates_and_rejects_unsafe_paths",
    "test_subdirectory_requires_expected_source_link",
    "xupnpd_config_subdirectory_validate(expected_link, source, 1)",
    "xupnpd_config_subdirectory_validate(wrong_link, source, 1)",
    "XUPnPD storage directory preparation and config-file safety regression: PASS",
):
    assert marker in test, marker

assert startup.index("xupnpd_config_subdirectory_validate(") < startup.index("if (has_daemon)")

presence_scan = function_body(source, "xupnpd_process_presence(void *context, int *scan_error)")
assert 'process_inventory_scan("xupnpd", &snapshot)' in presence_scan
assert "process_snapshot_release(&snapshot)" in presence_scan
startup = function_body(source, "start_xupnpd(char *wan_ifname)")
assert startup.index("xupnpd_process_presence(NULL, &scan_error)") < startup.index('unlink("/var/run/xupnpd.pid")')
assert "process_state != PROCESS_PRESENCE_ABSENT" in startup
assert startup.count("xupnpd_process_presence(NULL, &scan_error)") == 2
assert "launch_status = xupnpd_confirm_started(launch_status);" in startup
assert "detached_daemon_start_postcondition_with_ops(" in daemon_policy
assert "detached_daemon_start_postcondition_with_ops(" in detached_policy
assert "process_presence_start_postcondition(0, state)" in detached_policy
for marker in (
    "test_delayed_daemon_appearance_is_accepted",
    "test_absent_child_times_out",
    "test_unknown_inventory_fails_closed",
    "test_wait_interruption_and_invalid_arguments_fail",
    "XUPnPD detached-daemon startup postcondition regression: PASS",
):
    assert marker in daemon_test, marker

print("XUPnPD storage, config copy, cleanup and detached startup readiness gates: PASS")
