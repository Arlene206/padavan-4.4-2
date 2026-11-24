#!/usr/bin/env python3
"""Check the production LLDT and conformance-tool config parsers share bounded policies."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
src = ROOT / "trunk/user/lldt/src"
main = (src / "main.c").read_text(encoding="utf-8")
ctmain = (src / "ctmain.c").read_text(encoding="utf-8")
test = (src / "tests/lldt_config_path_policy.c").read_text(encoding="utf-8")
workflow = (ROOT / ".github/workflows/lldt-config-parser.yml").read_text(encoding="utf-8")
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text(encoding="utf-8")


def function_body(source, signature):
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


for label, source in (("production lld2d", main), ("optional lld2test", ctmain)):
    body = function_body(source, "init_from_conf_file()")
    assert '#include "lldt_config_parse_policy.h"' in source, label
    assert "lldt_config_parse_assignment(" in body, label
    assert "sscanf(" not in body, label + " retains unsafe sscanf"
    assert "memchr(" not in body, label + " bypasses length-aware parser"

assert "lldt_config_path_copy(val, path, path_capacity)" in function_body(main, "init_from_conf_file()")
assert "lldt_config_path_copy(val, path, path_capacity)" in function_body(ctmain, "init_from_conf_file()")
ct_body = function_body(ctmain, "init_from_conf_file()")
assert "atoi(" not in ct_body
assert "strncpy(cur" not in ct_body
assert "lldt_config_parse_unsigned(val, 100000UL" in ct_body
assert "lldt_config_parse_unsigned(val, 1000UL" in ct_body

for marker in ("embedded_nul", "ENAMETOOLONG", "100000UL", "100001",
               "999999999999999999999999999999999", "missing equals",
               "LLTD bounded config parser and icon path regression: PASS"):
    assert marker in test, "missing config regression: " + marker

tracked = ("trunk/user/lldt/src/main.c",
           "trunk/user/lldt/src/lldt_config_parse_policy.h",
           "trunk/user/lldt/src/lldt_config_path_policy.h",
           "trunk/user/lldt/src/tests/lldt_config_path_policy.c",
           "scripts/check_lldt_config_parser.py",
           ".github/workflows/lldt-config-parser.yml")
for path in tracked:
    assert workflow.count('      - "' + path + '"') >= 2, (
        "missing push/PR workflow trigger: " + path
    )
assert xy_workflow.count('      - "trunk/user/lldt/src/lldt_config_parse_policy.h"') >= 1
print("LLDT production/conformance config parser source contract: PASS")
