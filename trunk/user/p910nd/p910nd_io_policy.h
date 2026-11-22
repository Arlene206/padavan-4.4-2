#ifndef P910ND_IO_POLICY_H
#define P910ND_IO_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

/* Retry interrupted stream operations; leave EAGAIN/EWOULDBLOCK to select(). */
static inline ssize_t
p910nd_read_retry_eintr(int fd, void *buffer, size_t length,
                        ssize_t (*read_fn)(int, void *, size_t))
{
	ssize_t result;

	do {
		result = read_fn(fd, buffer, length);
	} while (result < 0 && errno == EINTR);

	return result;
}

static inline ssize_t
p910nd_write_retry_eintr(int fd, const void *buffer, size_t length,
                         ssize_t (*write_fn)(int, const void *, size_t))
{
	ssize_t result;

	do {
		result = write_fn(fd, buffer, length);
	} while (result < 0 && errno == EINTR);

	return result;
}

static inline int
p910nd_io_would_block(int error)
{
	return error == EAGAIN || error == EWOULDBLOCK;
}

/* A positive request followed by a zero-byte write cannot make progress. */
static inline int
p910nd_write_no_progress(ssize_t result, size_t requested)
{
	return requested > 0 && result == 0;
}

#endif
