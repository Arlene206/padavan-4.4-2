#ifndef __HEV_DNS_SESSION_IO_POLICY_H__
#define __HEV_DNS_SESSION_IO_POLICY_H__

#include <errno.h>
#include <sys/socket.h>

/* Reject truncated UDP query datagrams instead of forwarding malformed DNS. */
static inline int
hev_dns_session_datagram_truncated(const struct msghdr *message)
{
    return message != NULL && (message->msg_flags & MSG_TRUNC) != 0;
}

/* A closed upstream must not deliver SIGPIPE to the whole DNS daemon. */
static inline ssize_t
hev_dns_session_sendmsg_no_sigpipe(int fd, const struct msghdr *message)
{
    ssize_t result;

    do {
        result = sendmsg(fd, message, MSG_NOSIGNAL);
    } while (result < 0 && errno == EINTR);

    return result;
}

/* Signals are transient; let the caller handle only the final I/O result. */
static inline ssize_t
hev_dns_session_recvmsg_retry_eintr(int fd, struct msghdr *message)
{
    ssize_t result;

    do {
        result = recvmsg(fd, message, 0);
    } while (result < 0 && errno == EINTR);

    return result;
}

#endif /* __HEV_DNS_SESSION_IO_POLICY_H__ */
