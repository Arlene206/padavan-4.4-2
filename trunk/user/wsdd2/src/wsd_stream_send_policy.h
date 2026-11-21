#ifndef WSD_STREAM_SEND_POLICY_H
#define WSD_STREAM_SEND_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef ssize_t (*wsd_stream_send_fn)(int fd, const void *buffer,
                                     size_t length, int flags);

/* Complete a TCP message despite short writes and interrupted send calls. */
static inline int
wsd_stream_send_all(int fd, const void *buffer, size_t length,
                    wsd_stream_send_fn send_fn)
{
	const unsigned char *cursor = (const unsigned char *)buffer;
	size_t remaining = length;

	if ((!buffer && length != 0) || !send_fn) {
		errno = EINVAL;
		return -1;
	}

	while (remaining > 0) {
		ssize_t sent = send_fn(fd, cursor, remaining, MSG_NOSIGNAL);

		if (sent < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (sent == 0) {
			errno = EPIPE;
			return -1;
		}
		if ((size_t)sent > remaining) {
			errno = EIO;
			return -1;
		}

		cursor += (size_t)sent;
		remaining -= (size_t)sent;
	}

	return 0;
}

#endif /* WSD_STREAM_SEND_POLICY_H */
