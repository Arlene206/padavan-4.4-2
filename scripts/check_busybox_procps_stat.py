#!/usr/bin/env python3
"""Source contract for bounded BusyBox /proc/PID/stat parsing."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "trunk/user/busybox/busybox-1.24.x/libbb/procps.c"
HEADER = ROOT / "trunk/user/busybox/busybox-1.24.x/libbb/procps_stat_policy.h"
TEST = ROOT / "trunk/user/busybox/busybox-1.24.x/libbb/tests/procps_stat_policy.c"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"BusyBox procps stat contract: FAIL: {message}")


source = SOURCE.read_text()
header = HEADER.read_text()
test = TEST.read_text()

require('#include "procps_stat_policy.h"' in source,
        "production procps scanner must include the shared stat parser")
require('#include "procps_io_policy.h"' in source,
        "production procps scanner must include the bounded read retry policy")
read_to_buf_start = source.index("static int read_to_buf(")
read_to_buf_end = source.index("\\nstatic procps_status_t*", read_to_buf_start)
read_to_buf = source[read_to_buf_start:read_to_buf_end]
require("procps_read_with_retry(fd, buf, PROCPS_BUFSIZE-1)" in read_to_buf,
        "procfs reads must retry transient EINTR instead of dropping the PID")
io_header = (ROOT / "trunk/user/busybox/busybox-1.24.x/libbb/procps_io_policy.h").read_text()
io_test = (ROOT / "trunk/user/busybox/busybox-1.24.x/libbb/tests/procps_io_policy.c").read_text()
require("PROCPS_READ_MAX_INTERRUPTS 8U" in io_header and
        "interruptions >= PROCPS_READ_MAX_INTERRUPTS" in io_header,
        "procfs read retries must remain bounded under a signal storm")
for token in ("test_retries_after_interrupted_reads",
              "test_interrupt_retries_are_bounded",
              "test_non_interrupted_errors_are_not_retried",
              "test_eof_is_returned_without_retry"):
    require(token in io_test, f"interrupted-read regression must retain {token}")
scan_start = source.index("procps_status_t* FAST_FUNC procps_scan")
read_start = source.index("n = read_to_buf(filename, buf);", scan_start)
parse_end = source.index("#if !ENABLE_FEATURE_FAST_TOP", read_start)
read_block = source[read_start:parse_end]

require("if (n <= 0)" in read_block and
        "procps_stat_comm_bounds(buf, (size_t)n, &comm1, &cp)" in read_block,
        "empty reads and malformed stat records must be skipped before parsing")
require(read_block.index("procps_stat_comm_bounds") <
        read_block.index("*cp = '\\0';") <
        read_block.index("safe_strncpy(sp->comm, comm1 + 1"),
        "command field must be bounded before it is copied or fields are parsed")
require("/*if (!cp" not in read_block and "/*if (comm1)" not in read_block,
        "disabled NULL checks must not return to the procps parser")

for token in (
    "memchr(buffer, '\\0', length)",
    "open_paren[-1] != ' '",
    "cursor < open_paren - 1",
    "(size_t)(end - close_paren) < 4U",
    "close_paren[3] != ' '",
):
    require(token in header, f"bounded stat parsing must retain {token}")

for token in (
    "normal /proc stat record",
    "comm containing nested and unmatched right parentheses",
    "empty read is rejected",
    "missing closing parenthesis",
    "non-decimal PID prefix",
    "embedded NUL in proc stat read",
    "NULL input clears outputs and is rejected",
):
    require(token in test, f"host regression must retain {token}")

print("BusyBox procps stat source contract: PASS")
