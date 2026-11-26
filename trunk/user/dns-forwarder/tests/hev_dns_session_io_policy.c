#include <assert.h>
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
    test_recvmsg_reports_eof_after_peer_shutdown();
    return 0;
}
