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

    assert(u2ec_frame_size_is_valid(1L, capacity));
    assert(u2ec_frame_size_is_valid((long)capacity, capacity));
    assert(!u2ec_frame_size_is_valid(0L, capacity));
    assert(!u2ec_frame_size_is_valid(-1L, capacity));
    assert(!u2ec_frame_size_is_valid((long)capacity + 1L, capacity));
    assert(!u2ec_frame_size_is_valid(1L, 0U));
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
    test_invalid_buffers_are_rejected();
    puts("U2EC bounded TCP framing and SIGPIPE-safe I/O policy: PASS");
    return 0;
}
