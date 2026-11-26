#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/busybox/busybox-1.24.x/networking/udhcp"
files_c = (BASE / "files.c").read_text()
io_header = (BASE / "udhcp_lease_file_atomic.h").read_text()
test = (BASE / "tests/udhcp_lease_file_atomic.c").read_text()
workflow = (ROOT / ".github/workflows/udhcp-lease-file-atomic.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

writer_start = files_c.index("void FAST_FUNC write_leases(void)")
writer_end = files_c.index("void FAST_FUNC read_leases(", writer_start)
writer = files_c[writer_start:writer_end]
assert "udhcp_lease_file_write_atomic(" in writer
assert "O_TRUNC" not in writer
assert "full_write(" not in writer
assert writer.index("malloc(total_size)") < writer.index("udhcp_lease_file_write_atomic(")
assert writer.index("udhcp_lease_file_write_atomic(") < writer.index("spawn_and_wait(argv)")

create = io_header.index("fd = ops->mkstemp_fn(temporary_path)")
write = io_header.index("udhcp_lease_file_write_all(fd, buffer, length, ops)")
sync = io_header.index("ops->fsync_fn(fd)")
close = io_header.index("ops->close_fn(fd)")
rename = io_header.index("ops->rename_fn(temporary_path, path)")
dir_open = io_header.index("directory_fd = ops->open_dir_fn(parent_path)")
dir_sync = io_header.index("ops->fsync_dir_fn(directory_fd)")
assert dir_open < create < write < sync < close < rename < dir_sync
assert "UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED" in io_header
assert "write_status < 0" in files_c
assert "write_status == UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED" in files_c

assert "if (written == 0 || (size_t)written > length)" in io_header
assert "if (temporary_created)" in io_header
assert "temporary_created = 1;" in io_header
assert "if (temporary_created)\n        ops->unlink_fn(temporary_path)" in io_header
assert "ops->unlink_fn(temporary_path)" in io_header

for token in (
    "FAIL_MKSTEMP",
    "FAIL_PARTIAL_WRITE",
    "FAIL_FSYNC",
    "FAIL_CLOSE",
    "FAIL_RENAME",
    "FAIL_OPEN_DIR",
    "FAIL_DIR_FSYNC",
    "test_success_replaces_file_atomically",
    "test_directory_open_failure_preserves_old_file",
    "test_directory_fsync_failure_reports_published_file",
    "UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED",
    "test_pre_rename_failures_preserve_old_file",
    'assert_file_contents(path, "old-valid")',
    'assert_file_contents(template_path, "old-valid")',
):
    assert token in test

expected = (
    ".github/workflows/udhcp-lease-file-atomic.yml",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/files.c",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcp_lease_file_atomic.h",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcp_lease_file_atomic.c",
    "scripts/check_udhcp_lease_file_atomic.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in expected:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("missing or duplicate push path", path)
    assert pr.count(quoted) == 1, ("missing or duplicate PR path", path)
for path in expected[1:4]:
    assert xy_workflow.count('"' + path + '"') == 1, ("XY-C1 missing/duplicate source path", path)

print("DHCP lease-file atomic-write integration contract: PASS")
