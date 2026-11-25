#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "u2ec_socket_io_policy.h"

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
    assert(u2ec_recv_all(sockets[1], command, 7U) == 7);
    assert(memcmp(command, "GETCONF", 7U) == 0);
    assert(u2ec_config_command_kind(command, 7U) ==
           U2EC_CONFIG_COMMAND_UNKNOWN);
    assert(u2ec_recv_all(sockets[1], command + 7, 2U) == 2);
    assert(u2ec_config_command_kind(command, 9U) ==
           U2EC_CONFIG_COMMAND_GETCONFIG);
    close(sockets[0]);
    close(sockets[1]);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(write(sockets[0], "GET", 3U) == 3);
    assert(write(sockets[0], "NAME", 4U) == 4);
    memset(command, 0, sizeof(command));
    assert(u2ec_recv_all(sockets[1], command, 7U) == 7);
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

int
main(void)
{
    test_send_completes_a_tcp_stream_write();
    test_disconnected_peer_returns_epipe_without_sigpipe();
    test_mid_frame_eof_is_reported_as_reset();
    test_frame_size_is_bounded_and_positive();
    test_irp_buffer_size_must_fit_actual_received_frame();
    test_urb_buffer_bounds_reject_underflow_and_oversized_transfers();
    test_usb_selection_indices_and_variable_interface_records_are_bounded();
    test_discovery_commands_are_read_as_complete_stream_tokens();
    test_invalid_buffers_are_rejected();
    puts("U2EC bounded TCP framing, discovery command reads and SIGPIPE-safe I/O policy: PASS");
    return 0;
}
