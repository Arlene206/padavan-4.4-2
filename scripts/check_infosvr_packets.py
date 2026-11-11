#!/usr/bin/env python3
"""Check infosvr packet bounds and SIGTERM signal-lifecycle contracts."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return (ROOT / relative).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and nxt == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state in ("string", "char"):
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


source = read("trunk/user/infosvr/src/infosvr.c")
asus = read("trunk/user/infosvr/src/asus.c")
policy = read("trunk/user/infosvr/src/infosvr_packet_policy.h")
test = read("trunk/user/infosvr/src/tests/infosvr_packet_policy.c")
signal_policy = read("trunk/user/infosvr/src/infosvr_signal_policy.h")
signal_test = read("trunk/user/infosvr/src/tests/infosvr_signal_policy.c")
ibox = read("trunk/user/shared/src/include/ibox.h")
workflow = read(".github/workflows/infosvr-packet-stability.yml")
handoff = read("SESSION_HANDOFF.md")

for required_heading in (
    "## Current repository state",
    "## Active task",
    "## Verification boundary",
    "## Exact first action for the next GPT",
):
    assert required_heading in handoff, required_heading

for token in ("memset(packet, 0, size)", "errno = EINVAL",
              "received >= required", "required != 0"):
    assert token in policy, token

reader = function_body(source, "read_packet(int fd, void *packet, int size)")
prepare = reader.index("infosvr_packet_buffer_prepare(packet, (size_t)size)")
receive = reader.index("recvfrom(fd, packet, (size_t)size")
assert prepare < receive
assert "sizeof(*packet)" not in reader
assert "return res;" in reader
assert "return (res == size);" not in reader

processor = function_body(
    source, "process_packet(int fd, void *packet, int packet_len, char *ifname)"
)
base_guard = processor.index("infosvr_packet_length_valid((size_t)packet_len, sizeof(IBOX_COMM_PKT_HDR))")
service_read = processor.index("hdr->ServiceID")
assert base_guard < service_read
extended_guard = processor.index("offsetof(IBOX_COMM_PKT_HDR_EX, Password)")
mac_read = processor.index("hdr->MacAddress")
assert service_read < extended_guard < mac_read
assert "packet_len < 0" in processor

main = function_body(source, "main(int argc, char *argv[])")
receive_call = main.index("res = read_packet(g_fd4, packet, sizeof(packet))")
minimum_check = main.index("res < (int)sizeof(IBOX_COMM_PKT_HDR)", receive_call)
process_call = main.index("process_packet(g_fd4, packet, res, ifname)", minimum_check)
assert receive_call < minimum_check < process_call

# SIGTERM only records intent in signal context; all cleanup remains in main().
signal_handler = function_body(source, "catch_sig_infosvr(int sig)")
assert re.fullmatch(
    r"\s*\(void\)sig;\s*INFOSVR_SIGNAL_REQUEST_STOP\(g_stop_requested\);\s*",
    signal_handler,
)
assert "res_cleanup" not in signal_handler
assert "exit(" not in signal_handler

socket_open = function_body(source, "open_socket4(char *ifname)")
assert "SO_RCVTIMEO" in socket_open
assert "SO_SNDTIMEO" in socket_open
assert "receive_timeout.tv_sec = INFOSVR_SOCKET_TIMEOUT_SECONDS" in socket_open
assert "receive_timeout.tv_usec = 0" in socket_open

reader = function_body(source, "read_packet(int fd, void *packet, int size)")
assert "INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)" in reader
assert "errno = EINTR" in reader
assert "errno == EINTR &&" in reader
assert "!INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)" in reader

sender = function_body(source, "send_packet(int fd, void *packet, int size)")
assert "errno == EINTR &&" in sender
assert "!INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)" in sender
assert "if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))" in sender

sigempty = main.index("sigemptyset(&sigterm_action.sa_mask)")
sig_handler_install = main.index("sigaction(SIGTERM, &sigterm_action, NULL)")
daemon_call = main.index("daemon(0, 0)")
assert main.index("sigterm_action.sa_handler = catch_sig_infosvr") < sigempty
assert sigempty < sig_handler_install < daemon_call
assert "sigterm_action.sa_flags = 0" in main
assert "sigterm_action.sa_flags = SA_RESTART" not in main

loop_guard = main.index("while (!INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))")
timeout_set = main.index("select_timeout.tv_sec = INFOSVR_SELECT_TIMEOUT_SECONDS", loop_guard)
select_call = main.index("select(max_fd + 1, &active_rfds, NULL, NULL, &select_timeout)", timeout_set)
stop_after_select = main.index("if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))", select_call)
read_call = main.index("res = read_packet(g_fd4, packet, sizeof(packet))", stop_after_select)
stop_after_read = main.index("if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))", read_call)
process_call = main.index("process_packet(g_fd4, packet, res, ifname)", stop_after_read)
stop_after_process = main.index("if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))", process_call)
cleanup_call = main.index("res_cleanup();", stop_after_process)
assert loop_guard < timeout_set < select_call < stop_after_select
assert stop_after_select < read_call < stop_after_read < process_call
assert process_call < stop_after_process < cleanup_call

for token in (
    "single sig_atomic_t store",
    "INFOSVR_SELECT_TIMEOUT_SECONDS 1",
    "INFOSVR_SOCKET_TIMEOUT_SECONDS 1",
):
    assert token in signal_policy, token
assert "INFOSVR_SIGNAL_REQUEST_STOP(stop_requested);" in signal_test
assert "infosvr SIGTERM signal deferral policy: PASS" in signal_test

mac_source = function_body(asus, "get_hwaddr(char *ifname, void *buf, int size)")
assert "size < 6" in mac_source
assert "!ether_atoe(macaddr, buf)" in mac_source
use = processor.index("if (!get_hwaddr(ifname, hwaddr, sizeof(hwaddr)))")
compare = processor.index("memcmp(hdr->MacAddress, hwaddr")
assert use < compare

assert "BYTE Password[32]" in ibox
assert "BYTE MacAddress[6]" in ibox
for token in (
    "full receive buffer is initialized",
    "unused tail are zero",
    "truncated base IBOX header is rejected",
    "truncated extended MAC prefix is rejected",
    "NULL receive buffer is rejected",
    "infosvr packet initialization and length policy: PASS",
):
    assert token in test, token

paths = (
    ".github/workflows/infosvr-packet-stability.yml",
    "SESSION_HANDOFF.md",
    "scripts/check_infosvr_packets.py",
    "trunk/user/infosvr/src/infosvr.c",
    "trunk/user/infosvr/src/asus.c",
    "trunk/user/infosvr/src/infosvr_packet_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_packet_policy.c",
    "trunk/user/infosvr/src/infosvr_signal_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_signal_policy.c",
    "trunk/user/shared/src/include/ibox.h",
)
pre_jobs = workflow.split("jobs:", 1)[0]
push, pull = pre_jobs.split("  pull_request:", 1)
for path in paths:
    item = '"' + path + '"'
    assert push.count(item) == 1, "push path filter: " + path
    assert pull.count(item) == 1, "pull_request path filter: " + path

print("infosvr packet bounds and SIGTERM signal-lifecycle source contracts: PASS")
