#ifndef __HEV_DNS_SESSION_IO_POLICY_H__
#define __HEV_DNS_SESSION_IO_POLICY_H__

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

/* DNS messages have a 12-byte header; support EDNS(0) payloads up to 4096. */
#define HEV_DNS_SESSION_MIN_MESSAGE_SIZE 12U
#define HEV_DNS_SESSION_MAX_MESSAGE_SIZE 4096U

static inline int
hev_dns_session_message_size_valid(size_t message_size)
{
    return message_size >= HEV_DNS_SESSION_MIN_MESSAGE_SIZE &&
           message_size <= HEV_DNS_SESSION_MAX_MESSAGE_SIZE;
}


/*
 * Read the network-order TCP DNS frame length without assuming the two-byte
 * prefix is contiguous. A ring-buffer read can expose the prefix in two
 * separate iovecs when its read pointer is at the final byte.
 */
static inline int
hev_dns_session_read_frame_length(const struct iovec *vectors,
                                  size_t vector_count,
                                  size_t *frame_length)
{
    unsigned char prefix[2];
    size_t copied = 0;
    size_t index;

    if (!vectors || !frame_length) {
        errno = EINVAL;
        return -1;
    }

    for (index = 0; index < vector_count && copied < sizeof(prefix); index++) {
        size_t amount;

        if (vectors[index].iov_len == 0)
            continue;
        if (!vectors[index].iov_base) {
            errno = EINVAL;
            return -1;
        }

        amount = sizeof(prefix) - copied;
        if (amount > vectors[index].iov_len)
            amount = vectors[index].iov_len;
        memcpy(prefix + copied, vectors[index].iov_base, amount);
        copied += amount;
    }

    if (copied != sizeof(prefix)) {
        errno = EAGAIN;
        return 0;
    }

    *frame_length = ((size_t)prefix[0] << 8) | (size_t)prefix[1];
    return 1;
}

/* Two-byte TCP length prefix plus one full EDNS(0) DNS message. */
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
