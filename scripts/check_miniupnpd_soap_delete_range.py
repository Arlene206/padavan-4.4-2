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
free_at = compact.index("free(port_list)", record_at)
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


single_body = function_body(soap, "DeletePortMapping(struct upnphttp * h,")
single_compact = " ".join(single_body.split())
single_guard = single_compact.index("if(GETFLAG(SECUREMODEMASK))")
single_lookup = single_compact.index("upnp_get_redirection_infos(eport, protocol", single_guard)
single_lookup_failure = single_compact.index("if(r != 0)", single_lookup)
single_parse = single_compact.index("inet_pton(AF_INET, int_ip, &int_ip_addr)", single_lookup_failure)
single_peer = single_compact.index("miniupnpd_soap_ipv4_peer_matches(&h->clientaddr, &int_ip_addr)", single_parse)
single_denied = single_compact.index('SoapError(h, 606, "Action not authorized")', single_peer)
single_delete = single_compact.index("upnp_delete_redirection(eport, protocol)")
assert single_guard < single_lookup < single_lookup_failure < single_parse < single_peer < single_denied < single_delete
single_preflight = single_compact[single_guard:single_delete]
assert single_preflight.count('SoapError(h, 501, "ActionFailed")') == 2
assert single_preflight.count("ClearNameValueList(&data)") == 3
assert "return;" in single_preflight

secure_start = compact.index("if(GETFLAG(SECUREMODEMASK))")
lookup_at = compact.index("upnp_get_redirection_infos(port_list[i], protocol", secure_start)
parse_at = compact.index("inet_pton(AF_INET, int_ip, &int_ip_addr)", lookup_at)
peer_at = compact.index("miniupnpd_soap_ipv4_peer_matches(&h->clientaddr, &int_ip_addr)", parse_at)
unauthorized_at = compact.index('SoapError(h, 606, "Action not authorized")', peer_at)
first_delete_at = compact.index("upnp_delete_redirection(port_list[i], protocol)")
assert secure_start < lookup_at < parse_at < peer_at < unauthorized_at < first_delete_at
secure_preflight = compact[secure_start:first_delete_at]
assert "if(r != 0)" in secure_preflight
assert secure_preflight.count('SoapError(h, 501, "ActionFailed")') == 2
assert secure_preflight.count("free(port_list)") == 3
assert secure_preflight.count("ClearNameValueList(&data)") == 3
assert "return;" in secure_preflight
assert "miniupnpd_soap_ipv4_peer_matches" in policy
assert "sender->s_addr == internal->s_addr" in policy
assert "test_ipv4_peer_identity_comparison" in test
assert "MiniUPnP SOAP unsigned input and range-delete result policy: PASS" in test

print("MiniUPnP single/range DeletePortMapping secure-mode owner checks and backend failures: PASS")
