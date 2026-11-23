#!/usr/bin/env python3
"""Verify LLDT pidfile locking spans the entire existing check-and-publish path."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / "trunk/user/lldt/src/osl-linux.c").read_text(encoding="utf-8")
policy = (ROOT / "trunk/user/lldt/src/lldt_pidfile_policy.h").read_text(encoding="utf-8")
test = (ROOT / "trunk/user/lldt/src/tests/lldt_pidfile_policy.c").read_text(encoding="utf-8")

signature = "osl_write_pidfile(osl_t *osl)"
start = source.index(signature)
opening = source.index("{", start + len(signature))
depth = 0
state = "code"
end = None
pos = opening
while pos < len(source):
    char = source[pos]
    nxt = source[pos + 1] if pos + 1 < len(source) else ""
    if state == "code":
        if char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "/" and nxt == "*":
            state = "block"
            pos += 1
        elif char == "/" and nxt == "/":
            state = "line"
            pos += 1
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                end = pos
                break
    elif state in ("string", "char"):
        if char == "\\\\":
            pos += 1
        elif state == "string" and char == '"':
            state = "code"
        elif state == "char" and char == "'":
            state = "code"
    elif state == "block" and char == "*" and nxt == "/":
        state = "code"
        pos += 1
    elif state == "line" and char == "\\n":
        state = "code"
    pos += 1
assert end is not None, "unterminated osl_write_pidfile()"
writer = source[opening + 1:end]
lock_open = writer.index("osl_open_pidfile_lock(pidfile)")
first_pid_open = writer.index("open(pidfile")
last_pid_open = writer.rindex("open(pidfile")
lock_close = writer.index("close(pidfile_lock_fd)")
assert lock_open < first_pid_open < last_pid_open < lock_close
assert "O_NOFOLLOW" in source and "O_NONBLOCK" in source
assert "st_uid != geteuid()" in source
assert "st_nlink != 1" in source
assert "S_ISREG(lock_stat.st_mode)" in source
assert "flock(fd, LOCK_EX | LOCK_NB)" in policy
assert "test_pidfile_lock_serializes_competing_open" in test
print("LLDT pidfile concurrency contract: PASS")
