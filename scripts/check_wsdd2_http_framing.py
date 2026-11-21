#!/usr/bin/env python3
"""Enforce safe, bounded WSD HTTP request framing in the production parser."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsd.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_http_header_policy.h").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-http-framing-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def compact(text):
    return re.sub(r"\s+", " ", text).strip()


assert '#include "wsd_http_header_policy.h"' in SOURCE
start = SOURCE.index("static int wsd_parse_http_header(")
end = SOURCE.index("static char hostname[", start)
parser = compact(SOURCE[start:end])
assert "wsd_http_read_until_crlf(fd, buf, &len, bsize)" in parser
assert parser.count("wsd_http_read_until_crlf(fd, buf, &len, bsize)") == 2
assert "eol = wsd_http_find_crlf(p, len)" in parser
assert "char *eol = strstr(p" not in parser
assert "wsd_http_parse_content_length(val, &contentlength)" in parser
assert "Invalid or duplicate Content-Length" in parser
assert "atoi(val)" not in parser
assert "wsd_http_read_exact(fd, buf, &len, contentlength)" in parser
assert re.search(r"memmove\(buf, p, len\); p = buf;", parser), parser
assert "errno == EMSGSIZE" in parser

policy = compact(POLICY)
for token in (
    "i + 1 < length",
    "buffer[i] == '\\r' && buffer[i + 1] == '\\n'",
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
print("wsdd2 bounded HTTP framing source contract: PASS")
