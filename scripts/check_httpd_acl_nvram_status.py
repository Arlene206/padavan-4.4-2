#!/usr/bin/env python3
"""Verify HTTPD remote admission fails closed when its ACL NVRAM read fails."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/httpd/src/httpd.c").read_text()


def require(condition, message):
    if not condition:
        print(f"HTTPD ACL NVRAM status contract: FAIL: {message}", file=sys.stderr)
        raise SystemExit(1)


def body(signature):
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(SOURCE):
        char = SOURCE[index]
        nxt = SOURCE[index + 1] if index + 1 < len(SOURCE) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return SOURCE[opening + 1:index]
        elif state == "quoted":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line_comment":
            if char == chr(10):
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


load = body("httpd_load_acl_mode(void)")
gate = body("is_http_client_allowed(const usockaddr *usa)")
require('#include "httpd_acl_nvram_policy.h"' in SOURCE,
        "production HTTPD must include the tested ACL NVRAM status policy")
require("nvram_get_checked(\"http_access\", &value)" in load and
        "httpd_acl_nvram_mode_status(" in load and
        "nvram_get_int(\"http_access\")" not in load,
        "http_access must use a checked read rather than defaulting I/O errors to zero")
require("httpd_acl_client_allowed(http_acl_config_valid" in gate and
        "is_uaddr_localhost(&uip)" in gate and
        gate.index("httpd_acl_client_allowed(") < gate.index("if (http_acl_mode < 1)"),
        "unreadable ACL config must deny remote clients before disabled-mode handling")
policy = (ROOT / "trunk/user/httpd/src/httpd_acl_nvram_policy.h").read_text()
tests = (ROOT / "trunk/user/httpd/src/tests/httpd_acl_nvram_policy.c").read_text()
require("saved_errno > 0 ? saved_errno : EIO" in policy and
        "return configuration_valid || is_loopback;" in policy,
        "ACL policy must preserve read errors and keep loopback recovery access")
require("parsed = strtol(value, &end, 10)" in policy and
        "if (*end != '\\0' || parsed < 0)" in policy and
        "parsed > INT_MAX" in policy,
        "malformed or negative ACL modes must not be parsed as disabled")
for name in ("test_missing_key_preserves_disabled_acl_default",
             "test_malformed_or_negative_acl_values_fail_closed",
             "test_backend_failure_does_not_disable_acl",
             "test_failure_without_errno_and_unexpected_result_fail_closed"):
    require(name in tests, "ACL NVRAM regression is missing: " + name)

print("HTTPD ACL NVRAM status contract: PASS")
