#!/usr/bin/env python3
"""Guard DeletePortMappingRange against false-success SOAP responses."""

from pathlib import Path


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "quoted":
            if char == "\\":
                index += 1
            elif char == quote:
                state = "code"
        else:
            if char == "/" and next_char == "/":
                state = "line-comment"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block-comment"
                index += 1
            elif char in ('"', "'"):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


root = Path("trunk/user/miniupnpd/miniupnpd-2.x")
soap = (root / "upnpsoap.c").read_text()
policy = (root / "upnpsoap_input_policy.h").read_text()
test = (root / "tests/upnpsoap_input_policy_test.c").read_text()
body = function_body(soap, "DeletePortMappingRange(struct upnphttp * h,")
compact = " ".join(body.split())

assert '#include "upnpsoap_input_policy.h"' in soap
assert "struct miniupnpd_soap_delete_range_status delete_status;" in body
assert "miniupnpd_soap_delete_range_status_init(&delete_status);" in compact
delete_at = compact.index("upnp_delete_redirection(port_list[i], protocol)")
record_at = compact.index("miniupnpd_soap_delete_range_record(&delete_status, r)")
free_at = compact.index("free(port_list)")
error_at = compact.index("soap_error = miniupnpd_soap_delete_range_error(&delete_status)")
error_guard_at = compact.index("if (soap_error != 0)")
soap_error_at = compact.index('SoapError(h, soap_error, "ActionFailed")')
success_at = compact.index("BuildSendAndCloseSoapResp(h, body, bodylen)")
assert delete_at < record_at < free_at < error_at < error_guard_at < soap_error_at < success_at
assert 'r < 0 ? "failed" : "ok"' in body
assert "return;" in compact[error_guard_at:success_at]
assert "return 730;" in policy and "return status->failed ? 501 : 0;" in policy
assert "delete_result < 0" in policy
for case in (
    "test_delete_range_result_status",
    "miniupnpd_soap_delete_range_record(&status, -1)",
    "miniupnpd_soap_delete_range_error(&status) == 501",
):
    assert case in test
print("MiniUPnP DeletePortMappingRange reports backend deletion failures: PASS")
