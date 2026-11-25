#ifndef U2EC_SOCKET_IO_POLICY_H
#define U2EC_SOCKET_IO_POLICY_H

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define U2EC_CONFIG_COMMAND_UNKNOWN    0
#define U2EC_CONFIG_COMMAND_GETNAME    1
#define U2EC_CONFIG_COMMAND_GETCONFIG  2

/* Close a socket whose initialization failed without losing the cause. */
static inline int
u2ec_close_failed_socket(int fd)
{
    int saved_errno = errno;

    if (fd >= 0)
        (void)close(fd);
    errno = saved_errno;
    return -1;
}

/*
 * The discovery protocol uses fixed command tokens (7 and 9 bytes), not
 * NUL-terminated strings. Do not accept an incomplete TCP read as a command.
 */
static inline int
u2ec_config_command_kind(const char *message, size_t length)
{
    if (!message)
        return U2EC_CONFIG_COMMAND_UNKNOWN;
    if (length == 7U && memcmp(message, "GETNAME", 7U) == 0)
        return U2EC_CONFIG_COMMAND_GETNAME;
    if (length == 9U && memcmp(message, "GETCONFIG", 9U) == 0)
        return U2EC_CONFIG_COMMAND_GETCONFIG;
    return U2EC_CONFIG_COMMAND_UNKNOWN;
}

/*
 * The daemon mirrors descriptors in fd_set and fd_in_use. Both have finite
 * capacities; check both before indexing or passing a descriptor to FD_SET.
 */
static inline int
u2ec_fd_is_usable(int fd, size_t tracked_capacity)
{
    return fd >= 0 && fd < FD_SETSIZE && (size_t)fd < tracked_capacity;
}

/*
 * select() may modify the supplied fd_sets even when interrupted. Always
 * rebuild the working set from the immutable master set before retrying.
 * Returning the actual select result (rather than its EINTR comparison)
 * also preserves non-EINTR errors for the caller.
 */
typedef int (*u2ec_select_call_fn)(int, fd_set *, fd_set *, fd_set *,
                                   struct timeval *);

static inline int
u2ec_select_readable_retry_with(int nfds, const fd_set *master,
                                fd_set *readfds,
                                u2ec_select_call_fn select_call)
{
    int result;

    if (nfds < 0 || nfds > FD_SETSIZE || !master || !readfds ||
        !select_call) {
        errno = EINVAL;
        return -1;
    }

    do {
        *readfds = *master;
        result = select_call(nfds, readfds, NULL, NULL, NULL);
    } while (result < 0 && errno == EINTR);

    return result;
}

static inline int
u2ec_select_readable_retry(int nfds, const fd_set *master, fd_set *readfds)
{
    return u2ec_select_readable_retry_with(nfds, master, readfds, select);
}

/*
 * pselect atomically applies wait_mask while waiting. When the requested
 * signal sets interrupt_pending, return EINTR to the event loop so it can
 * perform the work in ordinary thread context; unrelated EINTRs are retried.
 */
typedef int (*u2ec_pselect_call_fn)(int, fd_set *, fd_set *, fd_set *,
                                    const struct timespec *, const sigset_t *);

static inline int
u2ec_pselect_readable_retry_with(int nfds, const fd_set *master,
                                 fd_set *readfds, const sigset_t *wait_mask,
                                 const volatile sig_atomic_t *interrupt_pending,
                                 u2ec_pselect_call_fn pselect_call)
{
    int result;

    if (nfds < 0 || nfds > FD_SETSIZE || !master || !readfds ||
        !wait_mask || !interrupt_pending || !pselect_call) {
        errno = EINVAL;
        return -1;
    }

    do {
        *readfds = *master;
        result = pselect_call(nfds, readfds, NULL, NULL, NULL, wait_mask);
    } while (result < 0 && errno == EINTR && !*interrupt_pending);

    return result;
}

static inline int
u2ec_pselect_readable_retry(int nfds, const fd_set *master, fd_set *readfds,
                            const sigset_t *wait_mask,
                            const volatile sig_atomic_t *interrupt_pending)
{
    return u2ec_pselect_readable_retry_with(nfds, master, readfds, wait_mask,
                                            interrupt_pending, pselect);
}

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

/*
 * Bound a complete TCP field, not just each individual recv() call. A peer
 * that trickles bytes must not keep the single U2EC event loop blocked forever.
 */
#define U2EC_CLIENT_FRAME_TIMEOUT_SECONDS 10U

static inline int
u2ec_timeval_remaining(const struct timeval *deadline,
                       struct timeval *remaining)
{
    struct timeval now;

    if (!deadline || !remaining) {
        errno = EINVAL;
        return -1;
    }
    if (gettimeofday(&now, NULL) < 0)
        return -1;

    remaining->tv_sec = deadline->tv_sec - now.tv_sec;
    remaining->tv_usec = deadline->tv_usec - now.tv_usec;
    if (remaining->tv_usec < 0) {
        remaining->tv_sec--;
        remaining->tv_usec += 1000000;
    }
    if (remaining->tv_sec < 0 ||
        (remaining->tv_sec == 0 && remaining->tv_usec <= 0)) {
        errno = ETIMEDOUT;
        return 0;
    }
    return 1;
}

static inline ssize_t
u2ec_recv_all_timeout(int fd, void *buffer, size_t length,
                      unsigned int timeout_seconds)
{
    unsigned char *bytes = (unsigned char *)buffer;
    size_t received = 0;
    struct timeval deadline, remaining, wait_time;
    fd_set readfds;
    int remaining_status, ready;
    ssize_t result;

    if (length == 0U)
        return 0;
    if (fd < 0 || fd >= FD_SETSIZE || !buffer || timeout_seconds == 0U) {
        errno = EINVAL;
        return -1;
    }
    if (gettimeofday(&deadline, NULL) < 0)
        return -1;
    deadline.tv_sec += (time_t)timeout_seconds;

    while (received < length) {
        remaining_status = u2ec_timeval_remaining(&deadline, &remaining);
        if (remaining_status <= 0)
            return -1;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        wait_time = remaining;
        ready = select(fd + 1, &readfds, NULL, NULL, &wait_time);
        if (ready < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (ready == 0) {
            errno = ETIMEDOUT;
            return -1;
        }

        result = recv(fd, bytes + received, length - received, MSG_DONTWAIT);
        if (result > 0) {
            received += (size_t)result;
        } else if (result == 0) {
            errno = ECONNRESET;
            return -1;
        } else if (errno == EINTR || errno == EAGAIN ||
                   errno == EWOULDBLOCK) {
            continue;
        } else {
            return -1;
        }
    }

    return (ssize_t)received;
}
/*
 * A FIFO with no writer reports readable EOF when opened read-only. U2EC
 * keeps an RDWR descriptor to suppress that idle wakeup; still treat EOF and
 * nonblocking races as "no event", never as an initialized control byte.
 */
static inline int
u2ec_read_fifo_event(int fd, char *event)
{
    ssize_t result;

    if (fd < 0 || !event) {
        errno = EINVAL;
        return -1;
    }

    do {
        result = read(fd, event, 1U);
    } while (result < 0 && errno == EINTR);

    if (result == 1)
        return 1;
    if (result == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
        return 0;
    return -1;
}

/*
 * Normalize file_lock() failures. Some legacy failure paths, including the
 * same-process re-entry guard, return -1 without assigning errno.
 */
static inline int
u2ec_lock_acquisition_is_valid(int lock_fd, int lock_errno)
{
    if (lock_fd >= 0)
        return 1;

    errno = lock_errno != 0 ? lock_errno : EIO;
    return 0;
}

/*
 * Release an owned lock descriptor at most once. Invalidate the caller's
 * handle before close so later control-flow paths cannot release a reused fd.
 */
typedef void (*u2ec_lock_release_fn)(int);

static inline void
u2ec_release_lock_once(int *lock_fd, u2ec_lock_release_fn release)
{
    int owned_fd;

    if (!lock_fd || *lock_fd < 0 || !release)
        return;

    owned_fd = *lock_fd;
    *lock_fd = -1;
    release(owned_fd);
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

/*
 * Ensure that the peer's IRP BufferSize is backed by bytes in the actual TCP
 * frame, not merely by the zero-filled capacity of the daemon's staging area.
 * frame_size includes the fixed IRP prefix before Buffer.
 */
static inline int
u2ec_irp_frame_buffer_bounds_valid(long frame_size, long buffer_size,
                                   size_t buffer_offset,
                                   size_t buffer_capacity)
{
    size_t received_payload;

    if (frame_size < 0 || buffer_size < 0 ||
        buffer_offset > (size_t)frame_size)
        return 0;

    received_payload = (size_t)frame_size - buffer_offset;
    return received_payload <= buffer_capacity &&
           (size_t)buffer_size <= received_payload;
}

/* Validate an index before using it against an externally sized USB array. */
static inline int
u2ec_usb_index_is_valid(unsigned long index, unsigned long count)
{
    return count > 0UL && index < count;
}

/*
 * Validate a variable-length USBD_INTERFACE_INFORMATION record before
 * reading or writing its pipe array and before advancing to the next record.
 */
static inline int
u2ec_urb_interface_record_bounds_valid(size_t record_length,
                                       size_t remaining_length,
                                       unsigned long pipe_count,
                                       size_t pipe_offset,
                                       size_t pipe_size)
{
    if (pipe_size == 0U || record_length < pipe_offset ||
        record_length > remaining_length)
        return 0;

    return pipe_count <= (record_length - pipe_offset) / pipe_size;
}

#endif /* U2EC_SOCKET_IO_POLICY_H */
