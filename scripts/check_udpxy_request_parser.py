#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/udpxy/src"
parser = (BASE / "rparse.c").read_text()
caller = (BASE / "udpxy.c").read_text()
test = (BASE / "test/rparse_request_bounds.c").read_text()
workflow = (ROOT / ".github/workflows/udpxy-request-parser-stability.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


get_request = body(parser, "get_request( const char* src")
parse_param = body(parser, "parse_param( const char* s")
parse_udprelay = body(parser, "parse_udprelay( const char* opt")
read_command = body(caller, "read_command( int sockfd")

assert "memchr(src, '\\0', srclen)" in get_request
assert "memcmp(src + i, HEAD, head_len)" in get_request
assert "memchr(p, ' ', remaining)" in get_request
for token in ("strstr(", "strcspn(", "strlen(", "strncpy("):
    assert token not in get_request, token

assert "memchr(s, '\\0', slen)" in parse_param
assert "memchr(s + i, '?', len - i)" in parse_param
assert "(i < len) && (s[i] == DLM)" in parse_param
assert "memcpy(tail, query, copy_len)" in parse_param
for token in ("strcspn(", "strncpy(", "strlen("):
    assert token not in parse_param, token

assert "memchr(opt, '\\0', optlen)" in parse_udprelay
assert "at >= s_addrlen" in parse_udprelay
assert "host_len >= addrlen" in parse_udprelay
assert "host[i] < '0' || host[i] > '9'" in parse_udprelay
assert "s_addr[0] = '\\0'" in parse_udprelay
for token in ("strncpy(", "strcspn(", "atoi(", "strlen("):
    assert token not in parse_udprelay, token

assert "recv( sockfd, httpbuf, sizeof(httpbuf), 0 )" in read_command
assert "get_request( httpbuf, (size_t)hlen, request, &rlen )" in read_command

for token in (
    "test_unterminated_request_buffer",
    "test_length_delimited_parameters",
    "test_source_address_bounds_and_port",
    "5000junk",
    "source_len",
):
    assert token in test, token

paths = (
    ".github/workflows/udpxy-request-parser-stability.yml",
    "trunk/user/udpxy/src/rparse.c",
    "trunk/user/udpxy/src/rparse.h",
    "trunk/user/udpxy/src/udpxy.c",
    "trunk/user/udpxy/src/test/rparse_request_bounds.c",
    "scripts/check_udpxy_request_parser.py",
)
push, pr = workflow.split("  pull_request:", 1)[0], workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted) == 1, ("pull_request path missing/duplicated", path)

assert xy_workflow.count('"trunk/user/udpxy/**"') == 1

print("UDPXY request parsing source and workflow contract: PASS")
