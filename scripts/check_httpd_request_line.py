#!/usr/bin/env python3
"""Check HTTPD's bounded request/header-line reader integration."""

from pathlib import Path

root = Path("trunk/user/httpd/src")
source = (root / "httpd.c").read_text()
policy = (root / "httpd_request_line_policy.h").read_text()
test = (root / "tests/httpd_request_line_policy.c").read_text()
workflow = Path(".github/workflows/httpd-security.yml").read_text()

start = source.index("handle_request(FILE *conn_fp")
end = source.index("\nstatic void\nreset_lang_dict", start)
request = source[start:end]

assert '#include "httpd_request_line_policy.h"' in source
assert request.count("httpd_request_line_read(") == 2
assert "httpd_request_line_read(conn_fp, line, sizeof(line))" in request
assert "httpd_request_line_read(" in request[
    request.index("while (cur < end)"):
    request.index('if (strcmp(cur, "\\n")')
]
assert "fgets(line, sizeof(line), conn_fp)" not in request
assert "fgets(cur, line + sizeof(line) - cur, conn_fp)" not in request
assert "line_status == HTTPD_REQUEST_LINE_READ_ERROR" in request
assert "Invalid or incomplete HTTP header line." in request

for marker in (
    "if (ch == '\\0')",
    "errno = EPROTO;",
    "errno = EMSGSIZE;",
    "if (ferror(stream))",
    "buffer[length] = '\\0';",
):
    assert marker in policy, marker

for marker in (
    "test_complete_lf_and_crlf_lines_are_preserved",
    "test_embedded_nul_is_rejected_in_request_and_header_lines",
    "test_overlong_and_incomplete_lines_fail_closed",
    "test_clean_eof_and_invalid_arguments_are_distinct",
):
    assert marker in test, marker

filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
required_paths = (
    ".github/workflows/httpd-security.yml",
    "trunk/user/httpd/src/httpd.c",
    "trunk/user/httpd/src/httpd_request_line_policy.h",
    "trunk/user/httpd/src/tests/httpd_request_line_policy.c",
    "scripts/check_httpd_request_line.py",
)
for path in required_paths:
    assert push_filters.count(path) == 1, f"push path missing/duplicated: {path}"
    assert pr_filters.count(path) == 1, f"pull_request path missing/duplicated: {path}"

assert "tests/httpd_request_line_policy.c" in workflow
assert "python3 scripts/check_httpd_request_line.py" in workflow
print("HTTPD bounded byte-safe request/header-line integration: PASS")
