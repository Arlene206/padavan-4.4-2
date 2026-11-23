#!/usr/bin/env python3
"""Check infosvr packet bounds, command dispatch and SIGTERM lifecycle contracts."""
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
dispatch_test = read("trunk/user/infosvr/src/tests/infosvr_dispatch_test.c")
signal_policy = read("trunk/user/infosvr/src/infosvr_signal_policy.h")
interface_policy = read("trunk/user/infosvr/src/infosvr_interface_policy.h")
interface_test = read("trunk/user/infosvr/src/tests/infosvr_interface_policy.c")
signal_test = read("trunk/user/infosvr/src/tests/infosvr_signal_policy.c")
io_policy = read("trunk/user/infosvr/src/infosvr_io_policy.h")
io_test = read("trunk/user/infosvr/src/tests/infosvr_io_policy.c")
pidfile_policy = read("trunk/user/infosvr/src/infosvr_pidfile_policy.h")
pidfile_test = read("trunk/user/infosvr/src/tests/infosvr_pidfile_policy.c")
ibox = read("trunk/user/shared/src/include/ibox.h")
workflow = read(".github/workflows/infosvr-packet-stability.yml")

for token in ("memset(packet, 0, size)", "errno = EINVAL",
              "received >= required", "required != 0",
              "received <= capacity", "capacity != 0"):
    assert token in policy, token
assert "recvfrom(receiver, receive_buffer, sizeof(receive_buffer)" in test
assert "MSG_TRUNC" in test
assert "MSG_TRUNC reports original datagram length" in test

reader = function_body(source, "read_packet(int fd, void *packet, int size)")
prepare = reader.index("infosvr_packet_buffer_prepare(packet, (size_t)size)")
receive = reader.index("recvfrom(fd, packet, (size_t)size, MSG_TRUNC")
assert prepare < receive
assert "sizeof(*packet)" not in reader
assert "return res;" in reader
assert "return (res == size);" not in reader

processor = function_body(
    source, "process_packet(int fd, void *packet, int packet_len, char *ifname)"
)
assert "hdr->OpCode == hdr_res->OpCode" not in processor
supported_guard = processor.index("if (hdr->OpCode != NET_CMD_ID_GETINFO && hdr->OpCode != NET_CMD_ID_GETINFO_MANU)")
response_reset = processor.index("memset(packet_res, 0, sizeof(packet_res_storage.bytes))")
response_header = processor.index("hdr_res->ServiceID =")
assert supported_guard < response_reset < response_header
assert "only the implemented read-only opcodes" in processor.lower().lower()
base_guard = processor.index("infosvr_packet_length_valid((size_t)packet_len, sizeof(IBOX_COMM_PKT_HDR))")
service_read = processor.index("hdr->ServiceID")
assert base_guard < service_read
extended_guard = processor.index("offsetof(IBOX_COMM_PKT_HDR_EX, Password)")
assert service_read < extended_guard < supported_guard
assert "hdr->MacAddress" not in processor
assert "get_hwaddr(ifname, hwaddr" not in processor
assert "get_hwaddr(ifname, info->MacAddress" in processor
assert "packet_len < 0" in processor
assert "memcpy(&header, packet, sizeof(header))" in processor
assert "IBOX_COMM_PKT_HDR *hdr = &header" in processor
assert "IBOX_COMM_PKT_RES_EX alignment;" in processor
assert "char bytes[512];" in processor
assert "(IBOX_COMM_PKT_HDR_EX *)packet" not in processor

main = function_body(source, "main(int argc, char *argv[])")
io_policy_test_body = function_body(io_test, "main(void)")
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
assert "infosvr_interface_name_copy(" in socket_open
assert socket_open.index("infosvr_interface_name_copy(") < socket_open.index("socket(AF_INET")
assert "strncpy(ifr.ifr_name" not in socket_open
for token in (
    "length >= capacity",
    "errno = ENAMETOOLONG",
    "memcpy(destination, source, length + 1)",
):
    assert token in interface_policy, token
for token in (
    "1234567890123456",
    "errno == ENAMETOOLONG",
    "NULL) == -1",
    "infosvr interface-name bounded copy policy: PASS",
):
    assert token in interface_test, token

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
pidfile_call = main.index("if (acquire_pidfile() != 0)")
loop_start = main.index("while (!INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))")
assert main.index("sigterm_action.sa_handler = catch_sig_infosvr") < sigempty
assert sigempty < sig_handler_install < daemon_call < pidfile_call < loop_start
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

select_error = main.index("if (res < 0 && infosvr_io_error_retryable(errno))")
select_fatal = main.index('perror("select")', select_error)
read_call = main.index("res = read_packet(g_fd4, packet, sizeof(packet))")
oversize_gate = main.index("infosvr_packet_length_fits((size_t)res, sizeof(packet))", read_call)
min_header_gate = main.index("if (res < (int)sizeof(IBOX_COMM_PKT_HDR))", oversize_gate)
assert oversize_gate < min_header_gate
assert "MSG_TRUNC" in reader
recv_error = main.index("if (res < 0) {", read_call)
recv_retry = main.index("if (infosvr_io_error_retryable(errno))", recv_error)
recv_fatal = main.index('perror("recvfrom")', recv_retry)
exit_cleanup = main.index("res_cleanup();")
exit_return = main.index("return exit_status;", exit_cleanup)
assert select_error < select_fatal < read_call < recv_error < recv_retry < recv_fatal < exit_cleanup < exit_return
assert "exit_status = 1;" in main
assert "errno = saved_errno ? saved_errno : EIO;" in main
assert "return 0;" not in main[exit_cleanup:]

for token in ("EINTR", "EAGAIN", "EWOULDBLOCK"):
    assert token in io_policy
for token in (
    "infosvr_io_error_retryable(EINTR)",
    "infosvr_io_error_retryable(EAGAIN)",
    "infosvr_io_error_retryable(EWOULDBLOCK)",
    "infosvr_io_error_retryable(EIO)",
    "infosvr_io_error_retryable(EBADF)",
    "infosvr_io_error_retryable(0)",
    "infosvr socket I/O retry policy: PASS",
):
    assert token in io_test
assert "infosvr_io_error_retryable(errno)" in main
assert "infosvr_io_error_retryable(0)" in io_policy_test_body

for token in (
    "single sig_atomic_t store",
    "INFOSVR_SELECT_TIMEOUT_SECONDS 1",
    "INFOSVR_SOCKET_TIMEOUT_SECONDS 1",
):
    assert token in signal_policy, token
assert "INFOSVR_SIGNAL_REQUEST_STOP(stop_requested);" in signal_test
assert "infosvr SIGTERM signal deferral policy: PASS" in signal_test

for token in (
    "flock(fd, LOCK_EX | LOCK_NB)",
    "ftruncate(fd, 0)",
    "written < 0 && errno == EINTR",
    "infosvr_pidfile_matches",
):
    assert token in pidfile_policy, token

pidfile_acquire = function_body(source, "acquire_pidfile(void)")
assert 'open(g_pidfile, O_RDWR | O_CREAT | O_NOFOLLOW, 0644)' in pidfile_acquire
assert "infosvr_pidfile_try_exclusive_lock(fd)" in pidfile_acquire
assert pidfile_acquire.index("infosvr_pidfile_try_exclusive_lock(fd)") < pidfile_acquire.index("fstat(fd, &status)")
assert pidfile_acquire.index("fstat(fd, &status)") < pidfile_acquire.index("infosvr_pidfile_write(fd, getpid())")
assert "S_ISREG(status.st_mode)" in pidfile_acquire
assert "status.st_nlink != 1" in pidfile_acquire
open_flags = re.search(r'open\s*\(\s*g_pidfile\s*,([^)]*)\)', pidfile_acquire)
assert open_flags is not None
assert "O_TRUNC" not in open_flags.group(1)
cleanup = function_body(source, "res_cleanup(void)")
assert "infosvr_pidfile_matches(g_pidfile_fd, getpid()) == 1" in cleanup
assert "opened_status.st_dev == path_status.st_dev" in cleanup
assert "opened_status.st_ino == path_status.st_ino" in cleanup
assert cleanup.index("infosvr_pidfile_matches") < cleanup.index("unlink(g_pidfile)") < cleanup.index("close(g_pidfile_fd)")
assert "remove(g_pidfile)" not in source
assert "g_pidfile_fd = -1" in cleanup

for token in (
    "contender must not truncate",
    "EWOULDBLOCK || errno == EAGAIN",
    'infosvr_pidfile_write(contender_fd, (pid_t)9876)',
    'infosvr_pidfile_matches(contender_fd, (pid_t)4321) == 0',
    "infosvr PID-file exclusive lock, complete publication and owner cleanup policy: PASS",
):
    assert token in pidfile_test, token

mac_source = function_body(asus, "get_hwaddr(char *ifname, void *buf, int size)")
assert "size < 6" in mac_source
assert "!ether_atoe(macaddr, buf)" in mac_source
assert "get_hwaddr(ifname, info->MacAddress, sizeof(info->MacAddress))" in processor

assert "BYTE Password[32]" in ibox
assert "BYTE MacAddress[6]" in ibox
for token in (
    "full receive buffer is initialized",
    "unused tail are zero",
    "truncated base IBOX header is rejected",
    "truncated extended MAC prefix is rejected",
    "NULL receive buffer is rejected",
    "infosvr packet initialization, length and UDP truncation policy: PASS",
):
    assert token in test, token

paths = (
    ".github/workflows/infosvr-packet-stability.yml",
    "scripts/check_infosvr_packets.py",
    "trunk/user/infosvr/src/infosvr.c",
    "trunk/user/infosvr/src/asus.c",
    "trunk/user/infosvr/src/infosvr_packet_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_packet_policy.c",
    "trunk/user/infosvr/src/tests/infosvr_dispatch_test.c",
    "trunk/user/infosvr/src/infosvr_signal_policy.h",
    "trunk/user/infosvr/src/infosvr_io_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_io_policy.c",
    "trunk/user/infosvr/src/infosvr_pidfile_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_pidfile_policy.c",
    "trunk/user/infosvr/src/tests/infosvr_signal_policy.c",
    "trunk/user/shared/src/include/ibox.h",
)
pre_jobs = workflow.split("jobs:", 1)[0]
push, pull = pre_jobs.split("  pull_request:", 1)
for path in paths:
    item = '"' + path + '"'
    assert push.count(item) == 1, "push path filter: " + path
    assert pull.count(item) == 1, "pull_request path filter: " + path

assert "NET_CMD_ID_SETINFO" in dispatch_test
assert "duplicate unsupported command" in dispatch_test
assert "response->Info == 0" in dispatch_test
assert "memcmp(response->MacAddress, info->PrinterInfo, sizeof(response->MacAddress)) == 0" in dispatch_test
assert "infosvr rejects duplicate unsupported opcodes and clears cached replies: PASS" in dispatch_test

print("infosvr packet bounds, opcode dispatch and SIGTERM signal-lifecycle source contracts: PASS")
