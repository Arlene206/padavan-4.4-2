#!/usr/bin/env python3
"""Require explicit ownership transfer after MiniUPnP IPTC deletion helpers."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()


def function_body(signature):
    start = SOURCE.index(signature)
    brace = SOURCE.index("{", start)
    depth = 0
    state = "code"
    index = brace
    while index < len(SOURCE):
        char = SOURCE[index]
        next_char = SOURCE[index + 1] if index + 1 < len(SOURCE) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and next_char == "*":
                state = "block_comment"
                index += 1
            elif char == "/" and next_char == "/":
                state = "line_comment"
                index += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return SOURCE[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\\\":
                index += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "line_comment" and char == "\\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


delete_helper = function_body("delete_rule_and_commit(unsigned int index, IPTC_HANDLE h,")
delete_rules = function_body(
    "delete_redirect_and_filter_rules(unsigned short eport, int proto)"
)

# The helper closes its by-value handle on every result. Each caller must
# clear its own copy before the later conditional cleanup can inspect it.
assert "iptc_free(h);" in delete_helper
assert "iptc_free(&h);" in delete_helper
assert "IPTC_HANDLE h" in SOURCE[SOURCE.index("delete_rule_and_commit(unsigned int index,") : SOURCE.index("{", SOURCE.index("delete_rule_and_commit(unsigned int index,"))]

calls = list(re.finditer(r"\\b(?:r|r2)\\s*=\\s*delete_rule_and_commit\\s*\\(", delete_rules))
assert len(calls) == 4, "expected four IPTC deletion-helper call sites"
for call in calls:
    semicolon = delete_rules.find(";", call.end())
    assert semicolon >= 0, "unterminated delete_rule_and_commit() call"
    ownership = re.match(r"\\s*h\\s*=\\s*NULL\\s*;", delete_rules[semicolon + 1 :])
    assert ownership, "caller must clear consumed IPTC handle immediately after delete_rule_and_commit()"

print("MiniUPnP IPTC delete-handle ownership contract: PASS")
