#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
files_c = (BASE / "files.c").read_text()
policy = (BASE / "udhcp_lease_file_time_policy.h").read_text()
test = (BASE / "tests/udhcp_lease_file_time_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-lease-file-clock-policy.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

read_start = files_c.index("void FAST_FUNC read_leases(")
read_body = files_c[read_start:]
time_gate = read_body.index(
    "udhcp_lease_file_time_delta_valid(time(NULL), written_at, &time_passed)"
)
lease_age_gate = read_body.index("goto ret;", time_gate)
record_loop = read_body.index("while (full_read(fd, &lease, sizeof(lease))", time_gate)
assert time_gate < lease_age_gate < record_loop
assert "(uint64_t)time_passed" not in read_body

assert "if (now < 0 || written_at < 0)" in policy
assert "if (written_at - now > UDHCP_LEASE_FILE_MAX_CLOCK_SKEW)" in policy
assert "*time_passed = 0;" in policy
assert "return *time_passed <= UDHCP_LEASE_FILE_MAX_CLOCK_SKEW;" in policy

for token in (
    "test_elapsed_time_within_existing_age_limit",
    "test_bounded_backward_clock_correction",
    "test_invalid_timestamps_are_rejected",
    "assert(elapsed == 0);",
    "UDHCP_LEASE_FILE_MAX_CLOCK_SKEW",
):
    assert token in test

expected = (
    ".github/workflows/udhcp-lease-file-clock-policy.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/files.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_lease_file_time_policy.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcp_lease_file_time_policy.c",
    "scripts/check_udhcp_lease_file_time_policy.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate source path", path)

print("DHCP lease-file clock integration contract: PASS")
