#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/miniupnpd/miniupnpd-2.x"
source = (BASE / "upnphttp.c").read_text()
policy = (BASE / "miniupnpd_http_request_policy.h").read_text()
test = (BASE / "tests/miniupnpd_http_request_policy_test.c").read_text()
linux_makefile = (BASE / "Makefile.linux").read_text()
nft_makefile = (BASE / "Makefile.linux_nft").read_text()
workflow = (ROOT / ".github/workflows/miniupnpd-http-request-stability.yml").read_text()


def body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


headers = body(source, "ParseHttpHeaders(struct upnphttp * h)")
query = body(source, "ProcessHttpQuery_upnphttp(struct upnphttp * h)")
process = body(source, "Process_upnphttp(struct upnphttp * h)")
header_too_large = body(source, "SendRequestHeaderTooLarge(struct upnphttp * h)")

assert '#include "miniupnpd_http_request_policy.h"' in source
assert "int\nParseHttpHeaders(struct upnphttp * h)" in source
assert "content_length_seen" in headers
assert "miniupnpd_http_record_content_length(" in headers
assert "miniupnpd_http_parse_content_length(" in policy
assert "miniupnpd_http_record_content_length(" in headers
assert "strncasecmp(line, \"Transfer-Encoding\", 17)==0" in headers
assert "return EINVAL;" in headers
assert "header_status = ParseHttpHeaders(h);" in query
assert "header_status == ERANGE" in query
assert "BuildResp2_upnphttp(h, 413, \"Payload Too Large\", NULL, 0)" in query
assert "BuildResp2_upnphttp(h, 400, \"Bad Request\", NULL, 0)" in query

assert "MINIUPNPD_HTTP_MAX_HEADER_BYTES" in process
assert "Request Header Fields Too Large" in header_too_large
assert "h->HttpVer[0]" in header_too_large
assert 'memcpy(h->HttpVer, "HTTP/1.0", sizeof("HTTP/1.0"))' in header_too_large
assert "BuildResp2_upnphttp(h, 431," in header_too_large
assert process.index("findendheaders(") < process.index("ProcessHttpQuery_upnphttp(h)")
assert "MINIUPNPD_HTTP_MAX_REQUEST_BYTES" in process
assert "miniupnpd_http_request_append_fits(" in process
assert "body_remaining" in process and "append_length" in process
assert "current_length + append_length + 1U" in process
assert "h->req_buf[h->req_buflen] = '\\0';" in process
assert "memcpy(h->req_buf + h->req_buflen, buf, append_length);" in process

for makefile in (linux_makefile, nft_makefile):
    assert "upnphttp.o: upnpevents.h upnputils.h miniupnpd_http_request_policy.h" in makefile

for token in (
    "MINIUPNPD_HTTP_MAX_HEADER_BYTES 8192U",
    "MINIUPNPD_HTTP_MAX_BODY_BYTES 65536U",
    "errno = ERANGE;",
    "maximum - current",
):
    assert token in policy, token
for token in (
    "test_content_length_boundaries",
    "test_content_length_rejects_ambiguous_or_malformed_values",
    "test_duplicate_content_length_rejected",
    "test_invalid_destination_is_rejected",
    "test_request_buffer_growth_boundaries",
):
    assert token in test, token

paths = (
    ".github/workflows/miniupnpd-http-request-stability.yml",
    "trunk/user/miniupnpd/miniupnpd-2.x/upnphttp.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_http_request_policy.h",
    "trunk/user/miniupnpd/miniupnpd-2.x/tests/miniupnpd_http_request_policy_test.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux_nft",
    "scripts/check_miniupnpd_http_request.py",
)
push, pr = workflow.split("  pull_request:", 1)[0], workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    quoted_path = '"' + path + '"'
    assert push.count(quoted_path) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted_path) == 1, ("pull_request path missing/duplicated", path)

print("MiniUPnP HTTP framing, duplicate Content-Length and bounded-buffer source contract: PASS")
