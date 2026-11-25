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
