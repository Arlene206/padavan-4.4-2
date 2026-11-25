#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/u2ec/src"
source = (BASE / "usbsock.c").read_text()
policy = (BASE / "u2ec_socket_io_policy.h").read_text()
test = (BASE / "tests/u2ec_socket_io_policy_test.c").read_text()
makefile = (BASE / "Makefile").read_text()
workflow = (ROOT / ".github/workflows/u2ec-socket-io-stability.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


add_remote = body(source, "void add_remote_device()")
usb_connection = body(source, "int usb_connection(int sockfd)")
establish = body(source, "int establish_connection(int sockfd)")
send_policy = body(policy, "u2ec_send_all_no_sigpipe(")
recv_policy = body(policy, "u2ec_recv_all(")

assert '#include "u2ec_socket_io_policy.h"' in source
assert "MSG_NOSIGNAL" in send_policy
assert "errno == EINTR" in send_policy
assert "return (ssize_t)sent;" in send_policy
assert "errno = ECONNRESET;" in recv_policy
assert "errno == EINTR" in recv_policy
assert "frame_size > 0" in policy
assert "(size_t)frame_size <= capacity" in policy

assert "u2ec_send_all_no_sigpipe" not in add_remote or add_remote.count("u2ec_send_all_no_sigpipe") == 2
assert add_remote.count("u2ec_send_all_no_sigpipe") == 2
assert "u2ec_recv_all(sockfd, (char *)&tcp_pack, sizeof(tcp_pack))" in usb_connection
assert "u2ec_frame_size_is_valid((long)tcp_pack.SizeBuffer" in usb_connection
assert "sizeof(IRP_SAVE), sizeof(data_buf)" in usb_connection
assert "u2ec_frame_size_is_valid((long)pirp_saverw->NeedSize" in usb_connection
assert "sizeof(IRP_SAVE), sizeof(data_bufrw)" in usb_connection
assert usb_connection.index("u2ec_frame_size_is_valid((long)tcp_pack.SizeBuffer") < usb_connection.index("bzero(data_buf, sizeof(data_buf))") < usb_connection.index("u2ec_recv_all(sockfd, (char *)pirp_save")
assert "sizeof(data_buf)" in usb_connection
assert "u2ec_recv_all(sockfd, (char *)pirp_save" in usb_connection
assert "for (i = 0, j = 0; i < tcp_pack.SizeBuffer; i += j)" not in usb_connection
assert usb_connection.count("u2ec_send_all_no_sigpipe") == 3
assert "goto CLOSE_CONN;" in usb_connection
assert "u2ec_send_all_no_sigpipe(new_fd, \"\\0\\0\", 2U)" in establish
assert "close(new_fd);" in establish
assert "send(new_fd," not in source and "send(sockfd," not in source

for token in (
    "test_send_completes_a_tcp_stream_write",
    "test_disconnected_peer_returns_epipe_without_sigpipe",
    "test_mid_frame_eof_is_reported_as_reset",
    "test_frame_size_is_bounded_and_positive",
    "test_invalid_buffers_are_rejected",
):
    assert token in test, token
assert "usbsock.o: u2ec_socket_io_policy.h" in makefile

paths = (
    ".github/workflows/u2ec-socket-io-stability.yml",
    "trunk/user/u2ec/src/usbsock.c",
    "trunk/user/u2ec/src/u2ec_socket_io_policy.h",
    "trunk/user/u2ec/src/tests/u2ec_socket_io_policy_test.c",
    "trunk/user/u2ec/src/Makefile",
    "scripts/check_u2ec_socket_io.py",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted) == 1, ("pull_request path missing/duplicated", path)
assert xy_workflow.count('"trunk/user/u2ec/**"') == 1

print("U2EC bounded TCP framing, partial-read handling and SIGPIPE-safe writes: PASS")
