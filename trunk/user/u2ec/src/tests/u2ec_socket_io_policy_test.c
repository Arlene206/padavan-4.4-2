#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

#include "u2ec_socket_io_policy.h"
#include "u2ec_discovery_snapshot_policy.h"


static void
test_discovery_product_snapshot_copy_is_bounded(void)
{
    char product[8];

    assert(u2ec_copy_discovery_product(1, "Printer", product,
                                       sizeof(product)) == 0);
    assert(strcmp(product, "Printer") == 0);

    assert(u2ec_copy_discovery_product(1, "Long printer name", product,
                                       sizeof(product)) == 0);
    assert(strcmp(product, "Long pr") == 0);

    assert(u2ec_copy_discovery_product(0, "stale", product,
                                       sizeof(product)) == 0);
    assert(product[0] == '\0');

    errno = 0;
    assert(u2ec_copy_discovery_product(1, NULL, product,
                                       sizeof(product)) == -1);
    assert(errno == EINVAL);

    errno = 0;
    assert(u2ec_copy_discovery_product(1, "Printer", product, 0U) == -1);
    assert(errno == EINVAL);

    errno = 0;
    assert(u2ec_copy_discovery_product(1, "Printer", NULL, 1U) == -1);
    assert(errno == EINVAL);
}

static void
test_send_completes_a_tcp_stream_write(void)
{
    int sockets[2];
    char received[16];
    const char payload[] = "u2ec-frame";

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(u2ec_send_all_no_sigpipe(sockets[0], payload,
                                    sizeof(payload) - 1U) ==
           (ssize_t)(sizeof(payload) - 1U));
    assert(u2ec_recv_all(sockets[1], received, sizeof(payload) - 1U) ==
           (ssize_t)(sizeof(payload) - 1U));
    assert(memcmp(received, payload, sizeof(payload) - 1U) == 0);
    close(sockets[0]);
    close(sockets[1]);
}

static void
test_disconnected_peer_returns_epipe_without_sigpipe(void)
{
    int sockets[2];
    const char payload[] = "printer reply";

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    close(sockets[1]);

    errno = 0;
    assert(u2ec_send_all_no_sigpipe(sockets[0], payload,
                                    sizeof(payload) - 1U) == -1);
    assert(errno == EPIPE);
    close(sockets[0]);
}

static void
test_monotonic_timeout_arithmetic(void)
{
    struct timespec started, deadline, now;
    struct timeval remaining;

    started.tv_sec = 100;
    started.tv_nsec = 900000000L;
    assert(u2ec_timespec_deadline_after_seconds(&started, 1UL,
                                                &deadline) == 0);
    assert(deadline.tv_sec == 101 && deadline.tv_nsec == 900000000L);

    now.tv_sec = 101;
    now.tv_nsec = 400000000L;
    assert(u2ec_timespec_remaining(&deadline, &now, &remaining) == 1);
    assert(remaining.tv_sec == 0 && remaining.tv_usec == 500000);

    now.tv_nsec = 899999999L;
    assert(u2ec_timespec_remaining(&deadline, &now, &remaining) == 1);
    assert(remaining.tv_sec == 0 && remaining.tv_usec == 1);
    now.tv_nsec = 900000000L;
    errno = 0;
    assert(u2ec_timespec_remaining(&deadline, &now, &remaining) == 0);
    assert(errno == ETIMEDOUT);

    now.tv_sec = 99;
    now.tv_nsec = 900000000L;
    assert(u2ec_timespec_remaining(&deadline, &now, &remaining) == 1);
    assert(remaining.tv_sec == 2 && remaining.tv_usec == 0);

    now.tv_nsec = 1000000000L;
    errno = 0;
    assert(u2ec_timespec_remaining(&deadline, &now, &remaining) == -1);
    assert(errno == EINVAL);
    errno = 0;
    assert(u2ec_timespec_deadline_after_seconds(NULL, 1UL,
                                                &deadline) == -1);
    assert(errno == EINVAL);

    started.tv_sec = (time_t)LONG_MAX;
    started.tv_nsec = 0;
    errno = 0;
    assert(u2ec_timespec_deadline_after_seconds(&started, 1UL,
                                                &deadline) == -1);
    assert(errno == EOVERFLOW);
}

static void
test_timed_send_completes_a_ready_field(void)
{
    int sockets[2];
    char received[5];
    const char payload[] = "u2ec";

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(u2ec_send_all_no_sigpipe_timeout(sockets[0], payload,
                                           sizeof(payload), 1U) ==
           (ssize_t)sizeof(payload));
    assert(read(sockets[1], received, sizeof(payload)) ==
           (ssize_t)sizeof(payload));
    assert(memcmp(received, payload, sizeof(payload)) == 0);
    close(sockets[0]);
    close(sockets[1]);
}

static void
test_timed_send_expires_when_peer_does_not_read(void)
{
    int sockets[2];
    int send_buffer = 4096;
    unsigned char *payload;
    size_t payload_length = 1024U * 1024U;

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &send_buffer,
                      sizeof(send_buffer)) == 0);
    payload = (unsigned char *)malloc(payload_length);
    assert(payload != NULL);
    memset(payload, 0x5a, payload_length);
    errno = 0;
    assert(u2ec_send_all_no_sigpipe_timeout(sockets[0], payload,
                                           payload_length, 1U) == -1);
    assert(errno == ETIMEDOUT);
    free(payload);
    close(sockets[0]);
    close(sockets[1]);
}
static void
test_timed_recv_completes_a_ready_field(void)
{
    int sockets[2];
    char received[4];

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(write(sockets[0], "u2ec", 4U) == 4);
    assert(u2ec_recv_all_timeout(sockets[1], received, sizeof(received),
                                 1U) == (ssize_t)sizeof(received));
    assert(memcmp(received, "u2ec", sizeof(received)) == 0);
    close(sockets[0]);
    close(sockets[1]);
}

static void
test_timed_recv_expires_after_a_partial_field(void)
{
    int sockets[2];
    char received[2];

    assert(U2EC_DISCOVERY_COMMAND_TIMEOUT_SECONDS == 1U);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(write(sockets[0], "x", 1U) == 1);
    errno = 0;
    assert(u2ec_recv_all_timeout(
               sockets[1], received, sizeof(received),
               U2EC_DISCOVERY_COMMAND_TIMEOUT_SECONDS) == -1);
    assert(errno == ETIMEDOUT);
    assert(received[0] == 'x');
    close(sockets[0]);
    close(sockets[1]);
}
static void
test_mid_frame_eof_is_reported_as_reset(void)
{
    int sockets[2];
    char received[8];

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(write(sockets[0], "abc", 3U) == 3);
    assert(shutdown(sockets[0], SHUT_WR) == 0);

    errno = 0;
    assert(u2ec_recv_all(sockets[1], received, sizeof(received)) == -1);
    assert(errno == ECONNRESET);
    assert(memcmp(received, "abc", 3U) == 0);
    close(sockets[0]);
    close(sockets[1]);
}

static void
test_frame_size_is_bounded_and_positive(void)
{
    const size_t capacity = 262144U;

    assert(u2ec_frame_size_is_valid(104L, 104U, capacity));
    assert(u2ec_frame_size_is_valid((long)capacity, 104U, capacity));
    assert(!u2ec_frame_size_is_valid(103L, 104U, capacity));
    assert(!u2ec_frame_size_is_valid(0L, 104U, capacity));
    assert(!u2ec_frame_size_is_valid(-1L, 104U, capacity));
    assert(!u2ec_frame_size_is_valid((long)capacity + 1L, 104U, capacity));
    assert(!u2ec_frame_size_is_valid(1L, 104U, 0U));
}

static void
test_urb_buffer_bounds_reject_underflow_and_oversized_transfers(void)
{
    assert(u2ec_urb_buffer_bounds_are_valid(256L, 64UL, 64UL,
                                             192UL, 256U));
    assert(u2ec_urb_buffer_bounds_are_valid(256L, 64UL, 64UL,
                                             0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(-1L, 64UL, 64UL,
                                              0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(257L, 64UL, 64UL,
                                              0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(63L, 64UL, 64UL,
                                              0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(256L, 63UL, 64UL,
                                              0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(256L, 64UL, 64UL,
                                            193UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(256L, 64UL, 0UL,
                                              0UL, 256U));
    assert(!u2ec_urb_buffer_bounds_are_valid(256L, 64UL, 257UL,
                                              0UL, 256U));
}

static void
test_irp_buffer_size_must_fit_actual_received_frame(void)
{
    const size_t prefix = 96U;
    const size_t capacity = 262144U;

    assert(u2ec_irp_frame_buffer_bounds_valid(96L, 0L, prefix, capacity));
    assert(u2ec_irp_frame_buffer_bounds_valid(112L, 16L, prefix, capacity));
    assert(u2ec_irp_frame_buffer_bounds_valid(112L, 8L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(112L, 17L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(112L, -1L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(-1L, 0L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(95L, 0L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(96L, 1L, prefix, capacity));
    assert(!u2ec_irp_frame_buffer_bounds_valid(96L, 0L, prefix - 1U, 0U));
    assert(!u2ec_irp_frame_buffer_bounds_valid(96L, 1L, prefix, 0U));
}

static void
test_usb_selection_indices_and_variable_interface_records_are_bounded(void)
{
    assert(u2ec_usb_index_is_valid(0UL, 1UL));
    assert(u2ec_usb_index_is_valid(2UL, 3UL));
    assert(!u2ec_usb_index_is_valid(0UL, 0UL));
    assert(!u2ec_usb_index_is_valid(3UL, 3UL));

    assert(u2ec_urb_interface_record_bounds_valid(40U, 40U, 2UL, 8U, 16U));
    assert(u2ec_urb_interface_record_bounds_valid(8U, 40U, 0UL, 8U, 16U));
    assert(!u2ec_urb_interface_record_bounds_valid(7U, 40U, 0UL, 8U, 16U));
    assert(!u2ec_urb_interface_record_bounds_valid(41U, 40U, 1UL, 8U, 16U));
    assert(!u2ec_urb_interface_record_bounds_valid(23U, 40U, 2UL, 8U, 16U));
    assert(!u2ec_urb_interface_record_bounds_valid(40U, 40U, 1UL, 8U, 0U));
}

static void
test_fd_set_descriptors_must_fit_both_capacities(void)
{
    assert(u2ec_fd_is_usable(0, 1U));
    assert(u2ec_fd_is_usable(FD_SETSIZE - 1, (size_t)FD_SETSIZE));
    assert(!u2ec_fd_is_usable(-1, 8U));
    assert(!u2ec_fd_is_usable(FD_SETSIZE, (size_t)FD_SETSIZE + 1U));
    assert(!u2ec_fd_is_usable(8, 8U));
    assert(!u2ec_fd_is_usable(0, 0U));
}

static int u2ec_test_select_calls;

static int
u2ec_test_select_interrupt_once(int nfds, fd_set *readfds,
                                fd_set *writefds, fd_set *exceptfds,
                                struct timeval *timeout)
{
    (void)writefds;
    (void)exceptfds;
    (void)timeout;
    assert(nfds == 8);
    assert(FD_ISSET(3, readfds));
    if (u2ec_test_select_calls++ == 0) {
        FD_ZERO(readfds);
        errno = EINTR;
        return -1;
    }
    return 1;
}

static int
u2ec_test_select_hard_failure(int nfds, fd_set *readfds,
                              fd_set *writefds, fd_set *exceptfds,
                              struct timeval *timeout)
{
    (void)nfds;
    (void)readfds;
    (void)writefds;
    (void)exceptfds;
    (void)timeout;
    u2ec_test_select_calls++;
    errno = EINVAL;
    return -1;
}

static volatile sig_atomic_t *u2ec_test_alarm_pending;
static int u2ec_test_pselect_calls;

static int
u2ec_test_pselect_alarm_interrupt(int nfds, fd_set *readfds,
                                  fd_set *writefds, fd_set *exceptfds,
                                  const struct timespec *timeout,
                                  const sigset_t *wait_mask)
{
    (void)writefds;
    (void)exceptfds;
    (void)timeout;
    assert(nfds == 8);
    assert(wait_mask != NULL);
    assert(FD_ISSET(3, readfds));
    assert(!FD_ISSET(7, readfds));
    u2ec_test_pselect_calls++;
    FD_ZERO(readfds);
    *u2ec_test_alarm_pending = 1;
    errno = EINTR;
    return -1;
}

static void
test_pselect_alarm_interrupt_is_not_swallowed(void)
{
    fd_set master, readfds;
    sigset_t wait_mask;
    volatile sig_atomic_t alarm_pending = 0;
    int result;

    sigemptyset(&wait_mask);
    FD_ZERO(&master);
    FD_SET(3, &master);
    FD_ZERO(&readfds);
    FD_SET(7, &readfds);

    u2ec_test_alarm_pending = &alarm_pending;
    u2ec_test_pselect_calls = 0;
    errno = 0;
    result = u2ec_pselect_readable_retry_with(
        8, &master, &readfds, &wait_mask, &alarm_pending,
        u2ec_test_pselect_alarm_interrupt);
    assert(result == -1);
    assert(errno == EINTR);
    assert(alarm_pending == 1);
    assert(u2ec_test_pselect_calls == 1);
    assert(!FD_ISSET(7, &readfds));
    u2ec_test_alarm_pending = NULL;

    errno = 0;
    result = u2ec_pselect_readable_retry_with(
        FD_SETSIZE + 1, &master, &readfds, &wait_mask, &alarm_pending,
        u2ec_test_pselect_alarm_interrupt);
    assert(result == -1);
    assert(errno == EINVAL);
    assert(u2ec_test_pselect_calls == 1);
}

static void
test_select_retries_restore_fdsets_and_preserve_errors(void)
{
    fd_set master, readfds;
    int result;

    FD_ZERO(&master);
    FD_SET(3, &master);
    FD_ZERO(&readfds);

    u2ec_test_select_calls = 0;
    result = u2ec_select_readable_retry_with(
        8, &master, &readfds, u2ec_test_select_interrupt_once);
    assert(result == 1);
    assert(u2ec_test_select_calls == 2);
    assert(FD_ISSET(3, &readfds));

    u2ec_test_select_calls = 0;
    errno = 0;
    result = u2ec_select_readable_retry_with(
        8, &master, &readfds, u2ec_test_select_hard_failure);
    assert(result == -1);
    assert(errno == EINVAL);
    assert(u2ec_test_select_calls == 1);

    u2ec_test_select_calls = 0;
    errno = 0;
    result = u2ec_select_readable_retry_with(
        FD_SETSIZE + 1, &master, &readfds,
        u2ec_test_select_hard_failure);
    assert(result == -1);
    assert(errno == EINVAL);
    assert(u2ec_test_select_calls == 0);
}

static void
test_discovery_commands_are_read_as_complete_stream_tokens(void)
{
    int sockets[2];
    char command[10] = {0};

    assert(u2ec_config_command_kind("GETNAME", 7U) ==
           U2EC_CONFIG_COMMAND_GETNAME);
    assert(u2ec_config_command_kind("GETCONFIG", 9U) ==
           U2EC_CONFIG_COMMAND_GETCONFIG);
    assert(u2ec_config_command_kind("GETCONF", 7U) ==
           U2EC_CONFIG_COMMAND_UNKNOWN);
    assert(u2ec_config_command_kind("GETCONFIG", 8U) ==
           U2EC_CONFIG_COMMAND_UNKNOWN);
    assert(u2ec_config_command_kind(NULL, 7U) ==
           U2EC_CONFIG_COMMAND_UNKNOWN);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    /* Two writes model a command split across TCP segments. */
    assert(write(sockets[0], "GETCON", 6U) == 6);
    assert(write(sockets[0], "FIG", 3U) == 3);
    assert(u2ec_recv_all_timeout(sockets[1], command, 7U,
                                 U2EC_DISCOVERY_COMMAND_TIMEOUT_SECONDS) == 7);
    assert(memcmp(command, "GETCONF", 7U) == 0);
    assert(u2ec_config_command_kind(command, 7U) ==
           U2EC_CONFIG_COMMAND_UNKNOWN);
    assert(u2ec_recv_all_timeout(sockets[1], command + 7, 2U,
                                 U2EC_DISCOVERY_COMMAND_TIMEOUT_SECONDS) == 2);
    assert(u2ec_config_command_kind(command, 9U) ==
           U2EC_CONFIG_COMMAND_GETCONFIG);
    close(sockets[0]);
    close(sockets[1]);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(write(sockets[0], "GET", 3U) == 3);
    assert(write(sockets[0], "NAME", 4U) == 4);
    memset(command, 0, sizeof(command));
    assert(u2ec_recv_all_timeout(sockets[1], command, 7U,
                                 U2EC_DISCOVERY_COMMAND_TIMEOUT_SECONDS) == 7);
    assert(u2ec_config_command_kind(command, 7U) ==
           U2EC_CONFIG_COMMAND_GETNAME);
    close(sockets[0]);
    close(sockets[1]);
}

static void
test_invalid_buffers_are_rejected(void)
{
    int sockets[2];

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    errno = 0;
    assert(u2ec_send_all_no_sigpipe(sockets[0], NULL, 1U) == -1);
    assert(errno == EINVAL);
    errno = 0;
    assert(u2ec_recv_all(sockets[0], NULL, 1U) == -1);
    assert(errno == EINVAL);
    assert(u2ec_send_all_no_sigpipe(sockets[0], NULL, 0U) == 0);
    assert(u2ec_recv_all(sockets[0], NULL, 0U) == 0);
    close(sockets[0]);
    close(sockets[1]);
}



static int u2ec_test_unlock_calls;
static int u2ec_test_unlocked_fd;

static void
u2ec_test_unlock_record(int fd)
{
    u2ec_test_unlock_calls++;
    u2ec_test_unlocked_fd = fd;
}


static void
test_printer_lock_acquisition_failure_is_normalized(void)
{
    errno = 0;
    assert(u2ec_lock_acquisition_is_valid(12, 0));

    errno = 0;
    assert(!u2ec_lock_acquisition_is_valid(-1, EACCES));
    assert(errno == EACCES);

    errno = 0;
    assert(!u2ec_lock_acquisition_is_valid(-1, 0));
    assert(errno == EIO);
}

static void
test_lock_fd_is_released_exactly_once(void)
{
    int lock_fd = 19;

    u2ec_test_unlock_calls = 0;
    u2ec_test_unlocked_fd = -1;
    u2ec_release_lock_once(&lock_fd, u2ec_test_unlock_record);
    assert(lock_fd == -1);
    assert(u2ec_test_unlock_calls == 1);
    assert(u2ec_test_unlocked_fd == 19);

    u2ec_release_lock_once(&lock_fd, u2ec_test_unlock_record);
    assert(u2ec_test_unlock_calls == 1);
    lock_fd = -1;
    u2ec_release_lock_once(&lock_fd, u2ec_test_unlock_record);
    assert(u2ec_test_unlock_calls == 1);
    u2ec_release_lock_once(&lock_fd, NULL);
    assert(u2ec_test_unlock_calls == 1);
}

static void
test_fifo_idle_readiness_and_event_bytes_are_safe(void)
{
    char path[128];
    char event = '?';
    int fd, old_read_fd, writer, ready;
    fd_set readfds;
    struct timeval timeout;

    assert(snprintf(path, sizeof(path), "/tmp/u2ec-fifo-test.%ld",
                    (long)getpid()) > 0);
    (void)unlink(path);
    assert(mkfifo(path, 0600) == 0);

    /*
     * A read-only FIFO becomes EOF-readable after a writer has connected,
     * sent an event and closed. An unopened writer does not necessarily
     * produce POLLHUP, so exercise the real notification lifecycle.
     */
    old_read_fd = open(path, O_RDONLY | O_NONBLOCK);
    assert(old_read_fd >= 0);
    writer = open(path, O_WRONLY | O_NONBLOCK);
    assert(writer >= 0);
    assert(write(writer, "r", 1U) == 1);
    assert(close(writer) == 0);
    assert(read(old_read_fd, &event, 1U) == 1);
    assert(event == 'r');

    FD_ZERO(&readfds);
    FD_SET(old_read_fd, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    ready = select(old_read_fd + 1, &readfds, NULL, NULL, &timeout);
    assert(ready == 1 && FD_ISSET(old_read_fd, &readfds));
    assert(read(old_read_fd, &event, 1U) == 0);
    assert(close(old_read_fd) == 0);

    /* Holding the FIFO RDWR prevents idle EOF and select() busy looping. */
    fd = open(path, O_RDWR | O_NONBLOCK);
    assert(fd >= 0);
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    assert(ready == 0);
    event = '?';
    assert(u2ec_read_fifo_event(fd, &event) == 0);
    assert(event == '?');

    writer = open(path, O_WRONLY | O_NONBLOCK);
    assert(writer >= 0);
    assert(write(writer, "c", 1U) == 1);
    assert(close(writer) == 0);
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    assert(ready == 1 && FD_ISSET(fd, &readfds));
    assert(u2ec_read_fifo_event(fd, &event) == 1);
    assert(event == 'c');
    assert(u2ec_read_fifo_event(fd, &event) == 0);
    assert(event == 'c');

    assert(close(fd) == 0);
    assert(unlink(path) == 0);
}


static void
test_socket_initialization_failure_closes_descriptor_and_preserves_errno(void)
{
    int sockets[2];

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    errno = EADDRINUSE;
    assert(u2ec_close_failed_socket(sockets[0]) == -1);
    assert(errno == EADDRINUSE);

    errno = 0;
    assert(fcntl(sockets[0], F_GETFD) == -1);
    assert(errno == EBADF);
    close(sockets[1]);
}


int
main(void)
{
    test_printer_lock_acquisition_failure_is_normalized();
    test_lock_fd_is_released_exactly_once();
    test_discovery_product_snapshot_copy_is_bounded();
    test_fifo_idle_readiness_and_event_bytes_are_safe();
    test_socket_initialization_failure_closes_descriptor_and_preserves_errno();
    test_send_completes_a_tcp_stream_write();
    test_disconnected_peer_returns_epipe_without_sigpipe();
    test_monotonic_timeout_arithmetic();
    test_timed_send_completes_a_ready_field();
    test_timed_send_expires_when_peer_does_not_read();
    test_timed_recv_completes_a_ready_field();
    test_timed_recv_expires_after_a_partial_field();
    test_mid_frame_eof_is_reported_as_reset();
    test_frame_size_is_bounded_and_positive();
    test_irp_buffer_size_must_fit_actual_received_frame();
    test_urb_buffer_bounds_reject_underflow_and_oversized_transfers();
    test_usb_selection_indices_and_variable_interface_records_are_bounded();
    test_discovery_commands_are_read_as_complete_stream_tokens();
    test_select_retries_restore_fdsets_and_preserve_errors();
    test_pselect_alarm_interrupt_is_not_swallowed();
    test_fd_set_descriptors_must_fit_both_capacities();
    test_invalid_buffers_are_rejected();
    puts("U2EC TCP framing, fd_set bounds, discovery commands, select retry and SIGPIPE-safe I/O policy: PASS");
    return 0;
}
