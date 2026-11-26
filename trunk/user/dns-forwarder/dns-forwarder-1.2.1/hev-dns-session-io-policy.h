#ifndef __HEV_DNS_SESSION_IO_POLICY_H__
#define __HEV_DNS_SESSION_IO_POLICY_H__

/* Two-byte TCP length prefix plus one full EDNS(0) 4096-byte DNS message. */
#define HEV_DNS_SESSION_BUFFER_SIZE (4096U + 2U)

enum {
    HEV_DNS_SESSION_RESPONSE_WAIT = 0,
    HEV_DNS_SESSION_RESPONSE_READY = 1,
    HEV_DNS_SESSION_RESPONSE_TOO_LARGE = -1
};

/* Distinguish an incomplete stream frame from one that cannot fit the buffer. */
static inline int
hev_dns_session_response_action(size_t frame_size, size_t received_size,
                                size_t buffer_capacity)
{
    if (frame_size <= received_size)
        return HEV_DNS_SESSION_RESPONSE_READY;
    if (received_size >= buffer_capacity)
        return HEV_DNS_SESSION_RESPONSE_TOO_LARGE;
    return HEV_DNS_SESSION_RESPONSE_WAIT;
}

#include <errno.h>
#include <stddef.h>
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
