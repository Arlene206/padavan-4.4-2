#ifndef IAPP_SIGNAL_PIPE_POLICY_H
#define IAPP_SIGNAL_PIPE_POLICY_H

#include <stddef.h>
#include <errno.h>
#include <string.h>
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


/*
 * Read exactly one length-prefixed record. Separate pipe writes may be
 * coalesced by read(), so the reader must consume the prefix and payload
 * lengths explicitly instead of passing an aggregate byte count downstream.
 */
static inline int iapp_signal_pipe_read_record(
	int fd,
	void *buffer,
	size_t capacity,
	size_t *record_length,
	ssize_t (*read_fn)(int, void *, size_t))
{
	unsigned char header[sizeof(size_t)];
	size_t payload_length;
	size_t received = 0;
	ssize_t bytes_read;

	if (fd < 0 || buffer == NULL || capacity == 0 ||
	    record_length == NULL || read_fn == NULL) {
		errno = EINVAL;
		return -1;
	}

	while (received < sizeof(header)) {
		bytes_read = read_fn(fd, header + received,
				     sizeof(header) - received);
		if (bytes_read < 0 && errno == EINTR)
			continue;
		if (bytes_read <= 0) {
			if (bytes_read == 0)
				errno = EPIPE;
			return -1;
		}
		if ((size_t)bytes_read > sizeof(header) - received) {
			errno = EIO;
			return -1;
		}
		received += (size_t)bytes_read;
	}

	memcpy(&payload_length, header, sizeof(payload_length));
	if (payload_length == 0) {
		errno = EINVAL;
		return -1;
	}
	if (payload_length > capacity) {
		errno = EMSGSIZE;
		return -1;
	}

	received = 0;
	while (received < payload_length) {
		bytes_read = read_fn(fd, (unsigned char *)buffer + received,
				     payload_length - received);
		if (bytes_read < 0 && errno == EINTR)
			continue;
		if (bytes_read <= 0) {
			if (bytes_read == 0)
				errno = EPIPE;
			return -1;
		}
		if ((size_t)bytes_read > payload_length - received) {
			errno = EIO;
			return -1;
		}
		received += (size_t)bytes_read;
	}

	*record_length = payload_length;
	return 0;
}

#endif /* IAPP_SIGNAL_PIPE_POLICY_H */
