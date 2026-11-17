#!/usr/bin/env python3
"""Ensure XUPnPD private storage directories are ready before daemon launch."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
source = (RC / "net.c").read_text()
header = (RC / "xupnpd_config_file.h").read_text()
test = (RC / "tests/xupnpd_config_file.c").read_text()


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
assert "copy_status = doSystem(\"cp -rf %s %s\", tmp1, tmp2);" in startup
copy_at = startup.index('copy_status = doSystem("cp -rf %s %s", tmp1, tmp2);')
copy_failure_at = startup.index("if (copy_status != 0)", copy_at)
copy_return_at = startup.index("return copy_status;", copy_failure_at)
assert copy_at < copy_failure_at < copy_return_at
assert "symlink(tmp1, tmp2) != 0 && errno != EEXIST" in startup
assert "xupnpd_config_subdirectory_validate(" in startup[startup.index("symlink(tmp1, tmp2)"):startup.index("for (i=0; xlua[i]; i++)")]
assert startup.index("xupnpd_config_subdirectory_validate(") < startup.index('if (has_daemon)')
assert "mkdir(tmp2, 0755)" not in startup
assert "doSystem(\"cp -rf %s %s\", tmp1, tmp2);" not in startup
assert "symlink(tmp1, tmp2);" not in startup
assert "static inline int\nxupnpd_config_ensure_directory" in header
assert "static inline int\nxupnpd_config_subdirectory_validate" in header
for marker in (
    "test_ensure_directory_creates_and_rejects_unsafe_paths",
    "test_subdirectory_requires_expected_source_link",
    "xupnpd_config_subdirectory_validate(expected_link, source, 1)",
    "xupnpd_config_subdirectory_validate(wrong_link, source, 1)",
    "XUPnPD storage directory preparation and config-file safety regression: PASS",
):
    assert marker in test, marker

print("XUPnPD storage setup and failure-before-launch contract: PASS")
