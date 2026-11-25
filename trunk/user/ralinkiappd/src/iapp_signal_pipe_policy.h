#ifndef IAPP_SIGNAL_PIPE_POLICY_H
#define IAPP_SIGNAL_PIPE_POLICY_H

#include <stddef.h>
#include <errno.h>
#include <sys/types.h>

/*
 * Queue one complete signal context without ever waiting for the event loop
 * that consumes the pipe. The caller supplies the pipe's runtime atomic-write
 * limit and a POSIX write-compatible callback so the policy is testable.
 */
static inline int iapp_signal_pipe_write_record(
	int fd,
	const void *buffer,
	size_t length,
	size_t pipe_buf_size,
	ssize_t (*write_fn)(int, const void *, size_t))
{
	ssize_t written;

	if (fd < 0 || buffer == NULL || length == 0 ||
	    pipe_buf_size == 0 || length > pipe_buf_size || write_fn == NULL) {
		errno = EINVAL;
		return -1;
	}

	do {
		written = write_fn(fd, buffer, length);
	} while (written < 0 && errno == EINTR);

	if (written < 0)
		return -1;

	/* An atomic <= PIPE_BUF nonblocking write must not be split. */
	if ((size_t)written != length) {
		errno = EIO;
		return -1;
	}

	return 0;
}

#endif /* IAPP_SIGNAL_PIPE_POLICY_H */
