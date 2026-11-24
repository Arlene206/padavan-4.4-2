#!/usr/bin/env python3
"""Verify HTTPD Basic-auth credentials are loaded atomically and fail closed."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/httpd/src/httpd.c").read_text()
WEB_SOURCE = (ROOT / "trunk/user/httpd/src/web_ex.c").read_text()


def require(condition, message):
    if not condition:
        print(f"HTTPD auth NVRAM status contract: FAIL: {message}", file=sys.stderr)
        raise SystemExit(1)


def body(signature, source=SOURCE):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
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
                    return source[opening + 1:index]
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


load = body("load_nvram_auth(void)")
read = body("httpd_auth_nvram_snapshot_credentials(")
require('#include "httpd_auth_nvram_policy.h"' in SOURCE,
        "HTTPD production source must include the tested checked-read policy")
require("httpd_auth_nvram_snapshot_credentials(" in load and
        "nvram_safe_get(" not in load and "nvram_get(" not in load,
        "Basic-auth credentials must not use error-collapsing NVRAM getters")
require("auth_basic_valid = 0;" in load and
        "memset(auth_basic_data, 0, sizeof(auth_basic_data));" in load and
        "if (status != 0)" in load and "auth_basic_valid = 1;" in load,
        "any snapshot/format failure must leave Basic authentication invalid")
require("httpd_auth_nvram_snapshot_credentials" in (ROOT / "trunk/user/httpd/src/httpd_auth_nvram_policy.h").read_text(),
        "the production snapshot helper must exist")
require("nvram_transaction_begin" in SOURCE and
        "nvram_transaction_get" in SOURCE and
        "nvram_transaction_end" in SOURCE,
        "credential reads must use one transaction")
require("auth_check( const char *authorization )" in SOURCE and
        "!auth_basic_valid" in body("auth_check( const char *authorization )"),
        "request authentication must reject an invalid credential snapshot")

WEB_VALIDATE = body("validate_asp_apply(webs_t wp, int sid)", WEB_SOURCE)
WEB_UPDATE = body("update_variables_ex(int eid, webs_t wp, int argc, char **argv)", WEB_SOURCE)
require('#include "httpd_auth_nvram_policy.h"' in WEB_SOURCE and
        "httpd_auth_account_update_status(account_status, errno)" in WEB_VALIDATE,
        "web settings must use the tested account-update status policy")
require("account_status = recreate_passwd_unix(1);" in WEB_VALIDATE and
        "account_status = change_passwd_unix(" in WEB_VALIDATE and
        "if (account_status != 0)" in WEB_VALIDATE and
        "return -1;" in WEB_VALIDATE,
        "passwd/shadow update failures must propagate out of validation")
require(WEB_UPDATE.count("if (result < 0)") == 3 and
        WEB_UPDATE.count("websError(wp, 500") == 3,
        "Save/Apply, Update and Restart handlers must stop on account-update failure")
require(WEB_UPDATE.index("if (result < 0)") < WEB_UPDATE.index("done_committing"),
        "failed account updates must not report a successful commit")

print("HTTPD auth NVRAM status contract: PASS")
print("HTTPD Unix-account update failure propagation: PASS")
