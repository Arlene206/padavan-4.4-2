#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/u2ec/src"
source = (BASE / "usbsock.c").read_text()
usb_test_source = (BASE / "testusb.c").read_text()
policy = (BASE / "u2ec_socket_io_policy.h").read_text()
test = (BASE / "tests/u2ec_socket_io_policy_test.c").read_text()
makefile = (BASE / "Makefile").read_text()
workflow = (ROOT / ".github/workflows/u2ec-socket-io-stability.yml").read_text()

# The package build invokes make with no explicit target. Keep "all" as
# the default goal so its first object dependency cannot suppress the link.
assert makefile.index(".DEFAULT_GOAL := all") < makefile.index("usbsock.o: u2ec_socket_io_policy.h")
assert "all: $(PROGS)" in makefile
assert "testusb.o: u2ec_discovery_snapshot_policy.h" in makefile
assert "trunk/user/u2ec/src/testusb.c" in workflow
assert "$(PROGS): $(OBJS)" in makefile
assert "OBJS\t= usbsock.o testusb.o" in makefile
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
daemon_main = body(source, "int main(int argc, char *argv[])")
establish = body(source, "int establish_connection(int sockfd)")
handle_fifo = body(source, "static int handle_fifo(")
update_device = body(source, "static void update_device()")
publish_snapshot = body(source, "u2ec_publish_discovery_snapshot(void)")
read_snapshot = body(source, "u2ec_read_discovery_snapshot(")
usb_connection = body(source, "int usb_connection(int sockfd)")
lock_acquire = body(source, "u2ec_acquire_printer_lock(void)")
handle_urb32 = body(source, "int handleURB(PIRP_SAVE")
handle_urb64 = body(source, "int handleURB_64(PIRP_SAVE")
exchange = body(source, "int exchangeIRP(")
establish = body(source, "int establish_connection(int sockfd)")
send_policy = body(policy, "u2ec_send_all_no_sigpipe(")
recv_policy = body(policy, "u2ec_recv_all(")
send_timeout_policy = body(policy, "u2ec_send_all_no_sigpipe_timeout(")
send_policy = body(policy, "u2ec_send_all_no_sigpipe(")
recv_timeout_policy = body(policy, "u2ec_recv_all_timeout(")
select_policy = body(policy, "u2ec_select_readable_retry_with(")

assert '#include "u2ec_socket_io_policy.h"' in source
assert '#include "u2ec_discovery_snapshot_policy.h"' in source
assert "fd >= 0 && fd < FD_SETSIZE" in policy
assert "(size_t)fd < tracked_capacity" in policy
assert "u2ec_fd_is_usable(find_fd, sizeof(fd_in_use))" in add_remote
assert "u2ec_fd_is_usable(conf_fd, sizeof(fd_in_use))" in add_remote
assert add_remote.index("u2ec_fd_is_usable(find_fd") < add_remote.index("FD_SET(find_fd")
assert add_remote.index("u2ec_fd_is_usable(conf_fd") < add_remote.index("FD_SET(conf_fd")
assert "u2ec_fd_is_usable(sockfd, sizeof(fd_in_use))" in usb_connection
assert "u2ec_fd_is_usable(new_fd, sizeof(fd_in_use))" in establish
assert establish.index("u2ec_fd_is_usable(new_fd") < establish.index("u2ec_send_all_no_sigpipe")
assert "u2ec_fd_is_usable(fifo_fd, sizeof(fd_in_use))" in daemon_main
assert daemon_main.index("u2ec_fd_is_usable(fifo_fd") < daemon_main.index("FD_SET(fifo_fd")
assert "u2ec_fd_is_usable(conn_fd, sizeof(fd_in_use))" in daemon_main
assert daemon_main.index("u2ec_fd_is_usable(conn_fd") < daemon_main.index("fd_in_use[conn_fd]")
assert "u2ec_fd_is_usable(new_fd, sizeof(fd_in_use))" in daemon_main
assert daemon_main.index("u2ec_fd_is_usable(new_fd") < daemon_main.index("fd_in_use[new_fd]") < daemon_main.index("FD_SET(new_fd")
rejected_new_fd_start = daemon_main.index("if (!u2ec_fd_is_usable(new_fd")
rejected_new_fd_end = daemon_main.index("continue;", rejected_new_fd_start)
rejected_new_fd = daemon_main[rejected_new_fd_start:rejected_new_fd_end]
assert "fd_in_use[new_fd]" not in rejected_new_fd
assert "u2ec_fd_is_usable(*fd, sizeof(fd_in_use))" in handle_fifo
assert handle_fifo.index("u2ec_fd_is_usable(*fd") < handle_fifo.index("FD_SET(*fd")
assert "return -1;" in handle_fifo
assert "if (rtvl < 0)" in daemon_main
assert "MSG_NOSIGNAL" in send_timeout_policy
assert "errno == EINTR" in send_timeout_policy
assert "return (ssize_t)sent;" in send_timeout_policy
assert "U2EC_CLIENT_FRAME_TIMEOUT_SECONDS 10U" in policy
assert "U2EC_CLIENT_SEND_TIMEOUT_SECONDS 10U" in policy
assert "u2ec_timeval_remaining(&deadline, &remaining)" in send_timeout_policy
assert "select(fd + 1, NULL, &writefds, NULL, &wait_time)" in send_timeout_policy
assert "MSG_NOSIGNAL | MSG_DONTWAIT" in send_timeout_policy
assert "errno = ETIMEDOUT" in send_timeout_policy
assert "u2ec_send_all_no_sigpipe_timeout(" in send_policy
assert "U2EC_CLIENT_SEND_TIMEOUT_SECONDS" in send_policy
assert "test_timed_send_completes_a_ready_field" in test
assert "test_timed_send_expires_when_peer_does_not_read" in test

assert "gettimeofday(&deadline, NULL)" in recv_timeout_policy
assert "u2ec_timeval_remaining(&deadline, &remaining)" in recv_timeout_policy
assert "select(fd + 1, &readfds, NULL, NULL, &wait_time)" in recv_timeout_policy
assert "MSG_DONTWAIT" in recv_timeout_policy
assert "errno = ETIMEDOUT" in recv_timeout_policy
assert "fd >= FD_SETSIZE" in recv_timeout_policy
assert "u2ec_recv_all_timeout(sockfd, (char *)&tcp_pack" in usb_connection
assert "u2ec_recv_all_timeout(sockfd, (char *)pirp_save" in usb_connection
assert "U2EC_CLIENT_FRAME_TIMEOUT_SECONDS" in usb_connection
assert "test_timed_recv_completes_a_ready_field" in test
assert "test_timed_recv_expires_after_a_partial_field" in test
assert "errno = ECONNRESET;" in recv_policy
assert "errno == EINTR" in recv_policy
assert "nfds > FD_SETSIZE" in select_policy
assert "*readfds = *master;" in select_policy
assert "result < 0 && errno == EINTR" in select_policy
assert "u2ec_select_readable_retry_with(nfds, master, readfds, select)" in policy
assert "u2ec_select_readable_retry(fdmax + 1, &master_fds" in add_remote
select_error = add_remote[add_remote.index("if (ret < 0)"):add_remote.index("/* handle GETIP", add_remote.index("if (ret < 0)"))]
assert "u2ec_close_failed_socket(conf_fd)" in select_error
assert "u2ec_close_failed_socket(find_fd)" in select_error
assert "u2ec_pselect_readable_retry(fdmax + 1, &master_fds" in daemon_main
assert "while ((ret = select(" not in add_remote
assert "while ((rtvl = select(" not in daemon_main
assert "if (ret < 0)" in add_remote
assert "if (rtvl < 0)" in daemon_main
assert "frame_size >= 0" in policy
assert "minimum <= capacity" in policy
assert "(size_t)frame_size >= minimum" in policy
assert "(size_t)frame_size <= capacity" in policy
assert "u2ec_urb_buffer_bounds_are_valid" in policy
assert "buffer_size < 0" in policy
assert "urb_length > (unsigned long)buffer_size" in policy
assert "transfer_length <= (unsigned long)buffer_size - urb_length" in policy

assert "u2ec_urb32_buffer_bounds_valid(pirp_saverw)" in handle_urb32
assert "u2ec_urb64_buffer_bounds_valid(pirp_saverw)" in handle_urb64
assert handle_urb32.index("u2ec_urb32_buffer_bounds_valid") < handle_urb32.index("size = pirp_saverw->BufferSize -")
assert handle_urb64.index("u2ec_urb64_buffer_bounds_valid") < handle_urb64.index("size = pirp_saverw->BufferSize -")
assert "urb_status < 0" in exchange and "return -1;" in exchange

assert "u2ec_usb_configuration_index_from_value" in source
assert "u2ec_usb_index_is_valid" in policy
assert "u2ec_urb_interface_record_bounds_valid" in policy
assert "u2ec_usb_interface_target_valid" in source
assert "dev->descriptor.bNumConfigurations" in source
assert "USB_DT_CONFIG_SIZE" in handle_urb32 and "USB_DT_CONFIG_SIZE" in handle_urb64
assert "u2ec_usb_interface_target_valid(tmp_config, tmp_int, tmp_alt)" in handle_urb32
assert "u2ec_usb_interface_target_valid(tmp_config, tmp_int, tmp_alt)" in handle_urb64
assert "offsetof(struct _URB_SELECT_CONFIGURATION, Interface)" in handle_urb32
assert "offsetof(struct _URB_SELECT_CONFIGURATION_64, Interface)" in handle_urb64
assert "interface_info->Length" in handle_urb32 and "interface_info->Length" in handle_urb64
assert "tmp_len -= interface_info->Length" in handle_urb32
assert "tmp_len -= interface_info->Length" in handle_urb64

assert add_remote.count("u2ec_send_all_no_sigpipe") == 2
assert "testusb();" in update_device
assert update_device.index("testusb();") < update_device.index("u2ec_publish_discovery_snapshot();")
assert "pthread_mutex_lock(&u2ec_discovery_snapshot_mutex)" in publish_snapshot
assert "u2ec_copy_discovery_product" in publish_snapshot
assert "pthread_mutex_lock(&u2ec_discovery_snapshot_mutex)" in read_snapshot
assert "u2ec_copy_discovery_product" in read_snapshot
assert "u2ec_read_discovery_snapshot" in add_remote
assert "if (dev)" not in add_remote
assert "str_product" not in add_remote
assert "u2ec_discovery_snapshot_policy.h" in makefile
assert "u2ec_discovery_snapshot_policy.h" in workflow
assert "test_discovery_product_snapshot_copy_is_bounded" in test
assert '#include "u2ec_discovery_snapshot_policy.h"' in usb_test_source
assert "sizeof(string) - 1" in usb_test_source
assert "u2ec_copy_discovery_product(1, string, str_product" in usb_test_source
assert "strcpy(str_product, string)" not in usb_test_source
assert "SO_RCVTIMEO" in add_remote and "command_timeout.tv_sec = 1" in add_remote
assert "u2ec_recv_all(new_fd, message, 7U)" in add_remote
assert "u2ec_recv_all(new_fd, message + 7, 2U)" in add_remote
assert "recv(new_fd, message, 9, 0)" not in add_remote
assert "u2ec_config_command_kind(message, 7U)" in add_remote
assert "u2ec_config_command_kind(message, 9U)" in add_remote
assert "U2EC_CONFIG_COMMAND_GETCONFIG" in add_remote
assert "U2EC_CONFIG_COMMAND_GETNAME" in add_remote
assert "length == 7U && memcmp(message, \"GETNAME\", 7U) == 0" in policy
assert "length == 9U && memcmp(message, \"GETCONFIG\", 9U) == 0" in policy
assert "u2ec_recv_all_timeout(sockfd, (char *)&tcp_pack" in usb_connection
assert "u2ec_frame_size_is_valid((long)tcp_pack.SizeBuffer" in usb_connection
assert "u2ec_irp_frame_buffer_bounds_valid(" in usb_connection
assert "offsetof(IRP_SAVE, Buffer)" in usb_connection
assert "sizeof(data_buf) - offsetof(IRP_SAVE, Buffer)" in usb_connection
assert "received_payload <= buffer_capacity" in policy
assert "(size_t)buffer_size <= received_payload" in policy
assert usb_connection.index("pirp_save->BufferSize = SWAP32") < usb_connection.index("if (!u2ec_irp_frame_buffer_bounds_valid(")
assert usb_connection.index("if (!u2ec_irp_frame_buffer_bounds_valid(") < usb_connection.index("purb->UrbHeader.Length = SWAP16")
assert "sizeof(IRP_SAVE), sizeof(data_buf)" in usb_connection
assert "u2ec_frame_size_is_valid((long)pirp_saverw->NeedSize" in usb_connection
assert "sizeof(IRP_SAVE), sizeof(data_bufrw)" in usb_connection
assert usb_connection.index("u2ec_frame_size_is_valid((long)tcp_pack.SizeBuffer") < usb_connection.index("bzero(data_buf, sizeof(data_buf))") < usb_connection.index("u2ec_recv_all_timeout(sockfd, (char *)pirp_save")
assert "sizeof(data_buf)" in usb_connection
assert "u2ec_recv_all_timeout(sockfd, (char *)pirp_save" in usb_connection
assert "for (i = 0, j = 0; i < tcp_pack.SizeBuffer; i += j)" not in usb_connection
assert usb_connection.count("u2ec_send_all_no_sigpipe") == 3
assert "goto CLOSE_CONN;" in usb_connection
assert "u2ec_send_all_no_sigpipe(new_fd, \"\\0\\0\", 2U)" in establish
assert "close(new_fd);" in establish
assert "send(new_fd," not in source and "send(sockfd," not in source

for signature in ("int dgram_sock_init(int port)",
                  "int stream_sock_init(int port)"):
    initializer = body(source, signature)
    socket_check = initializer.index("if ((sockfd = socket(")
    post_socket = initializer[socket_check:]
    assert post_socket.count("return u2ec_close_failed_socket(sockfd);") == 4
    assert "return -1;" not in post_socket.split("if (setsockopt", 1)[-1]
assert "u2ec_close_failed_socket" in policy
assert "test_socket_initialization_failure_closes_descriptor_and_preserves_errno" in test
assert "errno = EADDRINUSE;" in test and "errno == EBADF" in test

fifo_reader = body(source, "static int handle_fifo(")
assert "u2ec_read_fifo_event(*fd, &c)" in fifo_reader
assert fifo_reader.index("if (fifo_status < 0)") < fifo_reader.index("if (c == 'a')")
assert fifo_reader.index("if (fifo_status == 0)") < fifo_reader.index("lock = u2ec_acquire_printer_lock();")
assert "O_RDWR|O_NONBLOCK" in fifo_reader
assert "O_RDWR|O_NONBLOCK" in daemon_main
assert "u2ec_read_fifo_event" in policy
assert "u2ec_release_lock_once(&lock, file_unlock)" in fifo_reader
assert "file_unlock(lock)" not in fifo_reader
cancel_at = fifo_reader.index("CANCEL:")
reopen_at = fifo_reader.index("REOPEN_FIFO:", cancel_at)
assert "u2ec_release_lock_once(&lock, file_unlock)" in fifo_reader[cancel_at:reopen_at]
assert "u2ec_release_lock_once" in policy
assert 'file_lock("printer")' in lock_acquire
assert "errno = 0" in lock_acquire
assert "lock_errno = errno" in lock_acquire
assert "u2ec_lock_acquisition_is_valid" in lock_acquire
assert "lock_errno != 0 ? lock_errno : EIO" in policy

lock_sites = []
search_from = 0
while True:
    site = usb_connection.find("lock = u2ec_acquire_printer_lock();", search_from)
    if site < 0:
        break
    guard_end = usb_connection.find("if (lock < 0)", site)
    state_read = usb_connection.find("MFP_state(MFP_GET_STATE)", site)
    assert guard_end >= 0 and (state_read < 0 or guard_end < state_read)
    assert "goto CLOSE_CONN;" in usb_connection[guard_end:guard_end + 100]
    lock_sites.append(site)
    search_from = site + 1
assert len(lock_sites) == 2
fifo_lock = handle_fifo.index("lock = u2ec_acquire_printer_lock();")
fifo_guard = handle_fifo.index("if (lock < 0)", fifo_lock)
fifo_mutation = handle_fifo.index("/* Handle hotplug usb. */", fifo_lock)
assert fifo_lock < fifo_guard < fifo_mutation
assert "return -1;" in handle_fifo[fifo_guard:fifo_guard + 100]
startup_lock = daemon_main.index("lock = u2ec_acquire_printer_lock();")
startup_guard = daemon_main.index("if (lock < 0)", startup_lock)
startup_mutation = daemon_main.index('nvram_set_int_temp("MFP_busy", 0)', startup_lock)
assert startup_lock < startup_guard < startup_mutation
assert "return 1;" in daemon_main[startup_guard:startup_guard + 80]
assert "u2ec_lock_acquisition_is_valid" in test
assert "test_printer_lock_acquisition_failure_is_normalized" in test
alarm_handler = body(source, "static void multi_client(int sig)")
alarm_tick = body(source, "static void multi_client_tick(void)")
assert "multi_client_alarm_pending = 1" in alarm_handler
for token in ("conn_info_list", "last_busy_conn", "PDEBUG", "inet_ntoa",
              "time(", "alarm(", "open(", "write(", "close("):
    assert token not in alarm_handler, token
assert "if ((PCONNECTION_INFO)pos == last_busy_conn)" in alarm_tick
assert "if (!last_busy_live)" in alarm_tick
assert "last_busy_conn = NULL" in alarm_tick
assert "static void u2ec_forget_last_busy_connection(PCONNECTION_INFO connection)" in source
assert "if (last_busy_conn == connection)" in source
assert "u2ec_forget_last_busy_connection(conn_curt);" in usb_connection
assert usb_connection.index("u2ec_forget_last_busy_connection(conn_curt);") < usb_connection.index("u2ec_list_del((struct u2ec_list_head*)conn_curt);")
assert handle_fifo.count("free(pos);") == 4
forget_pos = "u2ec_forget_last_busy_connection((PCONNECTION_INFO)pos);"
assert handle_fifo.count(forget_pos) == handle_fifo.count("free(pos);")
free_at = handle_fifo.find("free(pos);")
while free_at >= 0:
    assert handle_fifo.rfind(forget_pos, 0, free_at) > handle_fifo.rfind("free(pos);", 0, free_at)
    free_at = handle_fifo.find("free(pos);", free_at + 1)
assert "last_busy_live->time" in alarm_tick
assert "u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK)" in alarm_tick
assert "if (u2ec_fifo < 0)" in alarm_tick
time_check_at = alarm_tick.index("if (now == (time_t)-1")
open_fifo_at = alarm_tick.index("u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK)")
write_event_at = alarm_tick.index('write(u2ec_fifo, "c", 1)')
assert time_check_at < alarm_tick.index("alarm(1)") < open_fifo_at
assert alarm_tick.count("alarm(1)") == 3
open_failure = alarm_tick[alarm_tick.index("if (u2ec_fifo < 0)"):write_event_at]
write_failure_start = alarm_tick.index("if (write(u2ec_fifo")
write_failure = alarm_tick[write_failure_start:alarm_tick.index("return;", write_failure_start)]
assert "alarm(1)" in open_failure
assert "alarm(1)" in write_failure
successful_delivery = alarm_tick[write_event_at:alarm_tick.index("close(u2ec_fifo);", write_event_at)]
assert "alarm(1)" not in successful_delivery and "alarm(0)" not in successful_delivery
assert "pthread_sigmask(SIG_BLOCK, &sigs_to_catch" in daemon_main
assert "sigaction(SIGALRM, &alarm_action, NULL)" in daemon_main
assert daemon_main.index("pthread_sigmask(SIG_BLOCK, &sigs_to_catch") < daemon_main.index("pthread_create(")
assert daemon_main.index("pthread_create(") < daemon_main.index("u2ec_pselect_readable_retry(")
assert "sigprocmask(SIG_UNBLOCK" not in daemon_main
assert "u2ec_pselect_readable_retry(fdmax + 1, &master_fds" in daemon_main
assert "multi_client_alarm_pending = 0" in daemon_main
assert "multi_client_tick();" in daemon_main
assert "u2ec_pselect_readable_retry_with" in policy
assert "result < 0 && errno == EINTR && !*interrupt_pending" in policy
assert "test_pselect_alarm_interrupt_is_not_swallowed" in test
assert "u2ec_pselect_readable_retry_with" in test
assert "pthread_sigmask(SIG_BLOCK" in daemon_main
assert "sigdelset(&select_sigmask, SIGALRM)" in daemon_main
assert "test_lock_fd_is_released_exactly_once" in test
for token in (
    "test_fifo_idle_readiness_and_event_bytes_are_safe",
    "A read-only FIFO becomes EOF-readable after a writer has connected,",
    "writer = open(path, O_WRONLY | O_NONBLOCK)",
    "Holding the FIFO RDWR prevents idle EOF and select() busy looping.",
    "u2ec_read_fifo_event(fd, &event) == 1",
):
    assert token in test, token

for token in (
    "test_send_completes_a_tcp_stream_write",
    "test_disconnected_peer_returns_epipe_without_sigpipe",
    "test_mid_frame_eof_is_reported_as_reset",
    "test_irp_buffer_size_must_fit_actual_received_frame",
    "test_frame_size_is_bounded_and_positive",
    "test_urb_buffer_bounds_reject_underflow_and_oversized_transfers",
    "test_usb_selection_indices_and_variable_interface_records_are_bounded",
    "test_discovery_commands_are_read_as_complete_stream_tokens",
    "test_select_retries_restore_fdsets_and_preserve_errors",
    "test_fd_set_descriptors_must_fit_both_capacities",
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
