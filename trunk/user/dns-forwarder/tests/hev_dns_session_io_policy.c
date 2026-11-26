#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../dns-forwarder-1.2.1/hev-dns-session-io-policy.h"

static void
test_sendmsg_to_closed_peer_does_not_raise_sigpipe(void)
{
    int sockets[2];
    struct msghdr message;
    struct iovec vector;
    char payload = 'x';

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(close(sockets[1]) == 0);

    memset(&message, 0, sizeof(message));
    vector.iov_base = &payload;
    vector.iov_len = sizeof(payload);
    message.msg_iov = &vector;
    message.msg_iovlen = 1;

    errno = 0;
    assert(hev_dns_session_sendmsg_no_sigpipe(sockets[0], &message) == -1);
    assert(errno == EPIPE);
    assert(close(sockets[0]) == 0);
}

static void
test_truncated_udp_datagrams_are_detected(void)
{
    int receiver, sender;
    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);
    struct msghdr message;
    struct iovec vector;
    char received[4];
    const char oversized[] = "eight-bytes";
    const char small[] = "dns";

    receiver = socket(AF_INET, SOCK_DGRAM, 0);
    sender = socket(AF_INET, SOCK_DGRAM, 0);
    assert(receiver >= 0 && sender >= 0);
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    assert(bind(receiver, (struct sockaddr *)&address, sizeof(address)) == 0);
    assert(getsockname(receiver, (struct sockaddr *)&address,
                       &address_length) == 0);

    assert(sendto(sender, oversized, sizeof(oversized) - 1, 0,
                  (struct sockaddr *)&address, sizeof(address)) ==
           (ssize_t)(sizeof(oversized) - 1));
    memset(&message, 0, sizeof(message));
    vector.iov_base = received;
    vector.iov_len = sizeof(received);
    message.msg_iov = &vector;
    message.msg_iovlen = 1;
    assert(recvmsg(receiver, &message, 0) == (ssize_t)sizeof(received));
    assert(hev_dns_session_datagram_truncated(&message));

    assert(sendto(sender, small, sizeof(small) - 1, 0,
                  (struct sockaddr *)&address, sizeof(address)) ==
           (ssize_t)(sizeof(small) - 1));
    memset(&message, 0, sizeof(message));
    message.msg_iov = &vector;
    message.msg_iovlen = 1;
    assert(recvmsg(receiver, &message, 0) == (ssize_t)(sizeof(small) - 1));
    assert(!hev_dns_session_datagram_truncated(&message));

    assert(close(sender) == 0);
    assert(close(receiver) == 0);
}

static void
test_dns_message_size_policy(void)
{
    assert(!hev_dns_session_message_size_valid(0U));
    assert(!hev_dns_session_message_size_valid(11U));
    assert(hev_dns_session_message_size_valid(12U));
    assert(hev_dns_session_message_size_valid(512U));
    assert(hev_dns_session_message_size_valid(4096U));
    assert(!hev_dns_session_message_size_valid(4097U));
}

static void
test_tcp_dns_frame_length_handles_ring_wrap(void)
{
    struct iovec vectors[2];
    unsigned char first = 0x00;
    unsigned char second = 0x0c;
    unsigned char contiguous[] = { 0x10, 0x00 };
    size_t frame_length = 0;

    vectors[0].iov_base = &first;
    vectors[0].iov_len = 1;
    vectors[1].iov_base = &second;
    vectors[1].iov_len = 1;
    assert(hev_dns_session_read_frame_length(vectors, 2, &frame_length) == 1);
    assert(frame_length == 12U);

    vectors[0].iov_base = contiguous;
    vectors[0].iov_len = sizeof(contiguous);
    vectors[1].iov_base = NULL;
    vectors[1].iov_len = 0;
    assert(hev_dns_session_read_frame_length(vectors, 2, &frame_length) == 1);
    assert(frame_length == 4096U);

    vectors[0].iov_base = NULL;
    vectors[0].iov_len = 0;
    vectors[1].iov_base = contiguous;
    vectors[1].iov_len = sizeof(contiguous);
    assert(hev_dns_session_read_frame_length(vectors, 2, &frame_length) == 1);
    assert(frame_length == 4096U);

    vectors[0].iov_base = &first;
    vectors[0].iov_len = 1;
    vectors[1].iov_base = NULL;
    vectors[1].iov_len = 0;
    errno = 0;
    assert(hev_dns_session_read_frame_length(vectors, 2, &frame_length) == 0);
    assert(errno == EAGAIN);

    vectors[1].iov_base = NULL;
    vectors[1].iov_len = 1;
    errno = 0;
    assert(hev_dns_session_read_frame_length(vectors, 2, &frame_length) == -1);
    assert(errno == EINVAL);
    errno = 0;
    assert(hev_dns_session_read_frame_length(NULL, 0, &frame_length) == -1);
    assert(errno == EINVAL);
}

static void
test_dns_response_frame_capacity_policy(void)
{
    assert(HEV_DNS_SESSION_BUFFER_SIZE == 4098U);
    assert(hev_dns_session_response_action(14U, 13U,
               HEV_DNS_SESSION_BUFFER_SIZE) ==
           HEV_DNS_SESSION_RESPONSE_WAIT);
    assert(hev_dns_session_response_action(14U, 14U,
               HEV_DNS_SESSION_BUFFER_SIZE) ==
           HEV_DNS_SESSION_RESPONSE_READY);
    assert(hev_dns_session_response_action(4098U, 4098U,
               HEV_DNS_SESSION_BUFFER_SIZE) ==
           HEV_DNS_SESSION_RESPONSE_READY);
    assert(hev_dns_session_response_action(4099U, 4098U,
               HEV_DNS_SESSION_BUFFER_SIZE) ==
           HEV_DNS_SESSION_RESPONSE_TOO_LARGE);
    assert(hev_dns_session_response_action(65537U, 4098U,
               HEV_DNS_SESSION_BUFFER_SIZE) ==
           HEV_DNS_SESSION_RESPONSE_TOO_LARGE);
}

static void
test_recvmsg_reports_eof_after_peer_shutdown(void)
{
    int sockets[2];
    struct msghdr message;
    struct iovec vector;
    char payload;

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(close(sockets[1]) == 0);

    memset(&message, 0, sizeof(message));
    vector.iov_base = &payload;
    vector.iov_len = sizeof(payload);
    message.msg_iov = &vector;
    message.msg_iovlen = 1;

    assert(hev_dns_session_recvmsg_retry_eintr(sockets[0], &message) == 0);
    assert(close(sockets[0]) == 0);
}

int
main(void)
{
    test_sendmsg_to_closed_peer_does_not_raise_sigpipe();
    test_truncated_udp_datagrams_are_detected();
    test_dns_response_frame_capacity_policy();
    test_tcp_dns_frame_length_handles_ring_wrap();
    test_dns_message_size_policy();
    test_recvmsg_reports_eof_after_peer_shutdown();
    return 0;
}
