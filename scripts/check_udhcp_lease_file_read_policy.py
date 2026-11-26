#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
files_c = (BASE / "files.c").read_text()
policy = (BASE / "udhcp_lease_file_read_policy.h").read_text()
test = (BASE / "tests/udhcp_lease_file_read_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-lease-file-read-policy.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

read_start = files_c.index("void FAST_FUNC read_leases(const char *file)")
read_body = files_c[read_start:]
header_read = read_body.index("read_count = full_read(fd, &written_at")
header_status = read_body.index("udhcp_lease_file_read_status(read_count", header_read)
header_error = read_body.index("empty or truncated lease file header", header_status)
record_read = read_body.index("read_count = full_read(fd, &lease", header_error)
record_status = read_body.index("udhcp_lease_file_read_status(read_count", record_read)
record_eof = read_body.index("UDHCP_LEASE_FILE_READ_EOF", record_status)
record_truncated = read_body.index("truncated lease record in %s", record_eof)
assert header_read < header_status < header_error < record_read < record_status
assert record_eof < record_truncated
assert "while (full_read(fd, &lease, sizeof(lease))" not in read_body
assert "lease file has invalid or stale timestamp" in read_body

for token in (
    "UDHCP_LEASE_FILE_READ_ERROR = -1",
    "UDHCP_LEASE_FILE_READ_EOF = 0",
    "UDHCP_LEASE_FILE_READ_COMPLETE = 1",
    "if ((size_t)bytes_read != expected_size)",
):
    assert token in policy

for token in (
    "test_complete_header_and_record",
    "test_eof_is_distinct_from_truncation",
    "test_read_errors_are_rejected",
    "udhcp_lease_file_read_status(31, 32)",
    "udhcp_lease_file_read_status(-1, 8)",
):
    assert token in test

expected = (
    ".github/workflows/udhcp-lease-file-read-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/files.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_lease_file_read_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcp_lease_file_read_policy.c",
    "scripts/check_udhcp_lease_file_read_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate source path", path)

print("DHCP lease-file read framing integration contract: PASS")
