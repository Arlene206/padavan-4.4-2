#!/usr/bin/env python3
"""Verify WAN DNS-mode reads fail closed before resolver publication."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / "trunk/user/rc/src/net_wan.c").read_text()
policy = (ROOT / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
test = (ROOT / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()


def body(text, signature):
    start = text.find(signature)
    if start < 0:
        raise AssertionError("missing function: " + signature)
    opening = text.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError("missing body: " + signature)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise AssertionError(message)


checked_int = body(
    source,
    "get_wan_unit_value_int_status(int unit, const char *param_name, int *value)",
)
require("nvram_get_checked(wanN_param, &stored_value)" in checked_int,
        "WAN integer getter must use the checked NVRAM API")
require("rc_wan_nvram_read_int_status(api_result, saved_errno" in checked_int,
        "WAN integer getter must preserve missing/error states")

parse_int = body(policy, "rc_wan_nvram_parse_int_value(const char *text, int *value)")
for token in ("strtol(text, &end, 10)", "errno == ERANGE", "parsed < INT_MIN",
              "parsed > INT_MAX", "if (*end)"):
    require(token in parse_int, "strict integer parser missing " + token)

read_int = body(policy, "rc_wan_nvram_read_int_status(int api_result, int saved_errno")
for token in ("rc_wan_nvram_read_status(api_result, saved_errno)",
              "if (api_result == 0)", "if (!text)",
              "rc_wan_nvram_parse_int_value(text, value)"):
    require(token in read_int, "checked read policy missing " + token)

dns_status = body(source, "get_wan_dns_static_status(int *is_static)")
require('nvram_get_checked("modem_dnsa", &stored_value)' in dns_status,
        "USB modem DNS mode must use checked reads")
require('get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled)' in dns_status,
        "WAN DNS mode must use checked integer reads")
legacy_dns = body(source, "get_wan_dns_static(void)")
require("get_wan_dns_static_status(&is_static)" in legacy_dns and
        "if (status != 0)" in legacy_dns,
        "legacy DNS selector must retain a compatibility fallback on read errors")

resolver = body(source, "update_resolvconf(int is_first_run, int do_not_notify)")
for token in ("get_wan_dns_static_status(&dns_static)", "file_unlock(lock)",
              "errno = dns_static_status", "return dns_static_status"):
    require(token in resolver, "resolver error path missing " + token)
require(resolver.index("return dns_static_status") <
        resolver.index("dnsmasq_file_open(resolv_temp)"),
        "resolver staging must not start before DNS mode is confirmed")

for case in ("test_checked_integer_read_preserves_missing_default",
             "test_checked_integer_read_accepts_complete_decimal_values",
             "test_checked_integer_read_rejects_malformed_and_overflow_values",
             "test_checked_integer_read_propagates_backend_errors"):
    require(case in test, "WAN integer regression missing " + case)

print("WAN DNS-mode checked-read and resolver fail-closed contract: PASS")
