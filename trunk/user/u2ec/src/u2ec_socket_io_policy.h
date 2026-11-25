#ifndef U2EC_SOCKET_IO_POLICY_H
#define U2EC_SOCKET_IO_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
 * Complete a TCP write without allowing a disconnected client to deliver
 * SIGPIPE to the whole printer daemon. The U2EC TCP sockets are blocking;
 * retry interruptions and treat zero progress as a broken stream.
 */
static inline ssize_t
u2ec_send_all_no_sigpipe(int fd, const void *buffer, size_t length)
{
    const unsigned char *bytes = (const unsigned char *)buffer;
    size_t sent = 0;

    if (!buffer && length != 0U) {
        errno = EINVAL;
        return -1;
    }

    while (sent < length) {
        ssize_t result = send(fd, bytes + sent, length - sent, MSG_NOSIGNAL);

        if (result > 0) {
            sent += (size_t)result;
        } else if (result < 0 && errno == EINTR) {
            continue;
        } else {
            if (result == 0)
                errno = EPIPE;
            return -1;
        }
    }

    return (ssize_t)sent;
}

/* Read one complete TCP frame field; EOF in the middle is a broken frame. */
static inline ssize_t
u2ec_recv_all(int fd, void *buffer, size_t length)
{
    unsigned char *bytes = (unsigned char *)buffer;
    size_t received = 0;

    if (!buffer && length != 0U) {
        errno = EINVAL;
        return -1;
    }

    while (received < length) {
        ssize_t result = recv(fd, bytes + received,
                              length - received, 0);

        if (result > 0) {
            received += (size_t)result;
        } else if (result == 0) {
            errno = ECONNRESET;
            return -1;
        } else if (errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return (ssize_t)received;
}

/* SizeBuffer is a signed, peer-controlled byte count for a fixed buffer. */
static inline int
u2ec_frame_size_is_valid(long frame_size, size_t minimum,
                           size_t capacity)
{
    return frame_size >= 0 && minimum <= capacity &&
           (size_t)frame_size >= minimum &&
           (size_t)frame_size <= capacity;
}

/*
 * Validate a USB URB's advertised payload before deriving a pointer or
 * passing a transfer length to libusb. All lengths are peer-controlled.
 */
static inline int
u2ec_urb_buffer_bounds_are_valid(long buffer_size,
                                 unsigned long urb_length,
                                 unsigned long minimum_urb_length,
                                 unsigned long transfer_length,
                                 size_t capacity)
{
    if (buffer_size < 0 || minimum_urb_length == 0UL ||
        (unsigned long)buffer_size > (unsigned long)capacity ||
        urb_length < minimum_urb_length ||
        urb_length > (unsigned long)buffer_size)
        return 0;

    return transfer_length <= (unsigned long)buffer_size - urb_length;
}

#endif /* U2EC_SOCKET_IO_POLICY_H */
