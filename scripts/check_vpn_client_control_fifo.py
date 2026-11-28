#!/usr/bin/env python3
"""Source and CI contract for bounded xl2tpd control FIFO operations."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

def body(source, signature):
    start = source.find(signature)
    assert start >= 0, "missing function: " + signature
    brace = source.find("{", start + len(signature))
    assert brace >= 0, "missing body: " + signature
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)

client = read("trunk/user/rc/src/vpn_client.c")
header = read("trunk/user/rc/src/vpn_client_control_fifo.h")
test = read("trunk/user/rc/src/tests/vpn_client_control_fifo.c")
workflow = read(".github/workflows/vpn-server-stability.yml")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
xy_test = read("scripts/tests/test_xy_c1_firmware_inputs.py")

control = body(client, "control_xl2tpd(const char *cmd, const char *lac)")
assert '#include "vpn_client_control_fifo.h"' in client
assert "vpn_client_control_fifo_open_writer_wait(" in control
assert "VPN_CLIENT_CONTROL_FIFO_OPEN_ATTEMPTS" in control
assert "VPN_CLIENT_CONTROL_FIFO_RETRY_DELAY_US" in control
assert "vpn_client_control_fifo_write_fd(" in control
assert "close(control_fd) != 0" in control
assert "open(" not in control and "write(" not in control

open_writer = body(header, "vpn_client_control_fifo_open_writer(const char *path)")
open_wait = body(header, "vpn_client_control_fifo_open_writer_wait(const char *path")
write_fd = body(header, "vpn_client_control_fifo_write_fd(int fd")
assert "O_NONBLOCK | O_NOFOLLOW" in open_writer
assert "fstat(fd, &st)" in open_writer and "S_ISFIFO(st.st_mode)" in open_writer
assert "ENOENT" in header and "ENXIO" in header and "EINTR" in header
assert "nanosleep(&delay, &delay)" in open_wait
assert "sigprocmask(SIG_BLOCK" in write_fd
assert "sigpending(&pending)" in write_fd
assert "sigtimedwait(&set, NULL, &no_wait)" in write_fd
assert "sigprocmask(SIG_SETMASK" in write_fd and "PIPE_BUF" in write_fd

for marker in ("test_orphaned_fifo_open_is_nonblocking",
               "test_delayed_reader_startup",
               "test_fifo_type_and_symlink",
               "test_epipe_does_not_kill_process",
               "test_preexisting_sigpipe_is_preserved",
               "test_atomic_fifo_write_and_bounds"):
    assert marker in test, "missing FIFO regression: " + marker

for path in ("trunk/user/rc/src/vpn_client_control_fifo.h",
             "trunk/user/rc/src/tests/vpn_client_control_fifo.c",
             "scripts/check_vpn_client_control_fifo.py"):
    push, pull = workflow.split("  pull_request:", 1)
    assert push.count("      - '" + path + "'") == 1, "missing push filter: " + path
    assert pull.count("      - '" + path + "'") == 1, "missing PR filter: " + path
assert "ASAN_OPTIONS=detect_leaks=1 /tmp/vpn-client-control-fifo" in workflow
assert "scripts/check_vpn_client_control_fifo.py" in workflow

xy_path = "trunk/user/rc/src/vpn_client_control_fifo.h"
assert xy_workflow.count('      - "' + xy_path + '"') == 1
assert "test_vpn_client_control_fifo_header_triggers_xy_c1_build" in xy_test
print("VPN client control FIFO source and CI contract: PASS")
