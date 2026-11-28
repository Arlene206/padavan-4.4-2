#!/usr/bin/env python3
"""Enforce safe, bounded WSD HTTP request framing in the production parser."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsd.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_http_header_policy.h").read_text()
REQUEST_POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_xml_request_policy.h").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-http-framing-regression.yml").read_text()
TEST = (ROOT / "trunk/user/wsdd2/src/tests/wsd_http_header_policy_test.c").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def compact(text):
    return re.sub(r"\s+", " ", text).strip()


assert '#include "wsd_http_header_policy.h"' in SOURCE
start = SOURCE.index("static int wsd_parse_http_header(")
end = SOURCE.index("static char hostname[", start)
parser = compact(SOURCE[start:end])
assert "wsd_http_read_until_crlf_until(fd, buf, &len, bsize, deadline)" in parser
assert parser.count("wsd_http_read_until_crlf_until(fd, buf, &len, bsize, deadline)") == 2
assert "wsd_http_read_exact_until(fd, buf, &len, contentlength, deadline)" in parser
assert "const struct timespec *deadline" in parser
assert "eol = wsd_http_find_crlf(p, len)" in parser
assert "char *eol = NULL;" in parser
assert parser.index("char *eol = NULL;") < parser.index("while ((eol =")
assert "while (*p &&" not in parser
assert "char *eol = strstr(p" not in parser
assert parser.count("wsd_http_line_contains_nul(p, (size_t)(eol - p))") == 2
assert "Embedded NUL in HTTP request line" in parser
assert "Embedded NUL in HTTP header" in parser
assert "wsd_http_parse_content_length(val, &contentlength)" in parser
assert "wsd_http_is_transfer_encoding_header(p)" in parser
assert "Transfer-Encoding is unsupported" in parser
assert parser.index("wsd_http_is_transfer_encoding_header(p)") < parser.index("wsd_http_parse_content_length(val, &contentlength)")
assert "wsd_http_is_transfer_encoding_header" in POLICY
assert "Invalid or duplicate Content-Length" in parser
assert "atoi(val)" not in parser
assert "wsd_http_read_exact(fd, buf, &len, contentlength)" not in parser
assert re.search(r"memmove\(buf, p, len\); p = buf;", parser), parser
assert "errno == EMSGSIZE" in parser
assert re.search(r'case\s+408:\s*status_str = "408 Request Timeout";', SOURCE)
assert re.search(r'case\s+413:\s*status_str = "413 Payload Too Large";', SOURCE)
tag_finder = compact(SOURCE[
    SOURCE.index("static const char *wsd_tag_find("):
    SOURCE.index("static struct wsd_req_info *wsd_req_parse(")
])
assert "const char *xml, size_t length" in tag_finder
assert "while (tag[tag_count][0]) ++tag_count;" in tag_finder
assert "wsd_xml_find_simple_text(xml, length, tag, tag_count, len)" in tag_finder
assert "strstr(" not in tag_finder
assert "wsd_xml_find_sequence" in REQUEST_POLICY
assert '"<!--"' in REQUEST_POLICY
assert '"<![CDATA["' in REQUEST_POLICY
assert '"?>"' in REQUEST_POLICY
assert "memchr(xml, '\\0', length)" in REQUEST_POLICY

request_parser = SOURCE[SOURCE.index("static struct wsd_req_info *wsd_req_parse("):
                         SOURCE.index("static void wsd_req_destruct(")]
assert "action_len == 0 || msgid_len == 0" in request_parser
assert "if (!info->action || !info->msgid)" in request_parser
assert "free(info->action);" in request_parser
assert "free(info->msgid);" in request_parser

policy = compact(POLICY)
for token in (
    "i + 1 < length",
    "buffer[i] == '\\r' && buffer[i + 1] == '\\n'",
    "WSD_HTTP_REQUEST_TIMEOUT_MS 5000U",
    "clock_gettime(CLOCK_MONOTONIC, deadline)",
    "poll(&descriptor, 1, (int)remaining_ms)",
    "clock_gettime(CLOCK_MONOTONIC, &now)",
    "1000000000LL",
    "MSG_DONTWAIT",
    "errno = ETIMEDOUT",
    "capacity - *length - 1",
    "received < 0 && errno == EINTR",
    "ECONNRESET",
    "parsed > ((size_t)-1 - digit) / 10",
):
    assert token in policy, token

for path in (
    "trunk/user/wsdd2/src/wsd.c",
    "trunk/user/wsdd2/src/wsd_http_header_policy.h",
    "trunk/user/wsdd2/src/tests/wsd_http_header_policy_test.c",
    "scripts/test_wsdd2_http_framing.sh",
    "scripts/check_wsdd2_http_framing.py",
):
    assert path in WORKFLOW, path
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW

recv_handler = SOURCE[SOURCE.index("int wsd_recv(struct endpoint *ep)"):]
recv_compact = compact(recv_handler)
assert "wsd_http_deadline_after_ms( &deadline, WSD_HTTP_REQUEST_TIMEOUT_MS)" in recv_compact
assert "wsd_http_recv_until(fd, buf, sizeof buf - 1, &deadline)" in recv_compact
assert "SO_RCVTIMEO" not in recv_handler
assert "accepted_client = 1;" in recv_handler
assert "close(fd); return 0;" in recv_compact
assert "wsd_parse_http_header(fd, ep, buf, len, sizeof buf, &deadline)" in recv_compact
assert "test_http_lines_reject_embedded_nul" in TEST
assert "wsd_http_line_contains_nul(embedded, sizeof(embedded)) == 1" in TEST
assert "test_absolute_deadline_rejects_slow_trickle" in TEST
assert "test_idle_client_deadline" in TEST
print("wsdd2 bounded HTTP framing and absolute client deadline source contract: PASS")
