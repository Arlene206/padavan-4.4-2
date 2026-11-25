#ifndef LANAUTH_SOCKET_IO_POLICY_H
#define LANAUTH_SOCKET_IO_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

enum lanauth_socket_io_stage {
    LANAUTH_SOCKET_IO_STAGE_NONE = 0,
    LANAUTH_SOCKET_IO_STAGE_WAIT,
    LANAUTH_SOCKET_IO_STAGE_READ,
    LANAUTH_SOCKET_IO_STAGE_WRITE,
    LANAUTH_SOCKET_IO_STAGE_TIMEOUT
};

typedef int (*lanauth_wait_readable_fn)(void *context, int fd,
                                        int timeout_seconds);
typedef ssize_t (*lanauth_read_fn)(void *context, int fd, void *buffer,
                                   size_t size);
typedef ssize_t (*lanauth_write_fn)(void *context, int fd, const void *buffer,
                                    size_t size);

static int
lanauth_socket_io_fail(int status, int stage, int *failure_stage,
                       size_t completed, size_t *completed_out)
{
    if (failure_stage)
        *failure_stage = stage;
    if (completed_out)
        *completed_out = completed;
    errno = status;
    return status;
}

/* Read one complete protocol frame. After the first byte, each additional
 * fragment must arrive within fragment_timeout_seconds. */
static int
lanauth_read_exact_with_ops(int fd, void *buffer, size_t size,
                            int first_timeout_seconds,
                            int fragment_timeout_seconds,
                            lanauth_wait_readable_fn wait_readable,
                            lanauth_read_fn read_data, void *context,
                            int *failure_stage, size_t *completed_out)
{
    size_t completed = 0;

    if (failure_stage)
        *failure_stage = LANAUTH_SOCKET_IO_STAGE_NONE;
    if (completed_out)
        *completed_out = 0;
    if (fd < 0 || !buffer || size == 0 || first_timeout_seconds < 0 ||
        fragment_timeout_seconds < 0 || !wait_readable || !read_data)
        return lanauth_socket_io_fail(EINVAL, LANAUTH_SOCKET_IO_STAGE_NONE,
                                      failure_stage, completed, completed_out);

    while (completed < size) {
        int timeout_seconds, ready;
        ssize_t count;

        timeout_seconds = completed == 0 ? first_timeout_seconds :
                          fragment_timeout_seconds;
        errno = 0;
        ready = wait_readable(context, fd, timeout_seconds);
        if (ready == 0)
            return lanauth_socket_io_fail(ETIMEDOUT,
                LANAUTH_SOCKET_IO_STAGE_TIMEOUT, failure_stage, completed,
                completed_out);
        if (ready < 0)
            return lanauth_socket_io_fail(errno ? errno : EIO,
                LANAUTH_SOCKET_IO_STAGE_WAIT, failure_stage, completed,
                completed_out);

        errno = 0;
        count = read_data(context, fd, (char *)buffer + completed,
                          size - completed);
        if (count < 0)
            return lanauth_socket_io_fail(errno ? errno : EIO,
                LANAUTH_SOCKET_IO_STAGE_READ, failure_stage, completed,
                completed_out);
        if (count == 0)
            return lanauth_socket_io_fail(ECONNRESET,
                LANAUTH_SOCKET_IO_STAGE_READ, failure_stage, completed,
                completed_out);
        if ((size_t)count > size - completed)
            return lanauth_socket_io_fail(EIO, LANAUTH_SOCKET_IO_STAGE_READ,
                failure_stage, completed, completed_out);
        completed += (size_t)count;
    }

    if (completed_out)
        *completed_out = completed;
    return 0;
}

/* Write one complete protocol frame. An interruption after a partial write
 * returns EINTR so the caller can discard this connection and reconnect;
 * sending the remainder on a later connection would corrupt framing. */
static int
lanauth_write_all_with_ops(int fd, const void *buffer, size_t size,
                           lanauth_write_fn write_data, void *context,
                           size_t *completed_out)
{
    size_t completed = 0;

    if (completed_out)
        *completed_out = 0;
    if (fd < 0 || !buffer || size == 0 || !write_data)
        return lanauth_socket_io_fail(EINVAL, LANAUTH_SOCKET_IO_STAGE_NONE,
                                      NULL, completed, completed_out);

    while (completed < size) {
        ssize_t count;

        errno = 0;
        count = write_data(context, fd, (const char *)buffer + completed,
                           size - completed);
        if (count < 0)
            return lanauth_socket_io_fail(errno ? errno : EIO,
                LANAUTH_SOCKET_IO_STAGE_WRITE, NULL, completed, completed_out);
        if (count == 0)
            return lanauth_socket_io_fail(EIO, LANAUTH_SOCKET_IO_STAGE_WRITE,
                NULL, completed, completed_out);
        if ((size_t)count > size - completed)
            return lanauth_socket_io_fail(EIO, LANAUTH_SOCKET_IO_STAGE_WRITE,
                NULL, completed, completed_out);
        completed += (size_t)count;
    }

    if (completed_out)
        *completed_out = completed;
    return 0;
}

#endif /* LANAUTH_SOCKET_IO_POLICY_H */
