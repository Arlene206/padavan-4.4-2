#ifndef WSD_HTTP_HEADER_POLICY_H
#define WSD_HTTP_HEADER_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stddef.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>

/* Locate CRLF without reading beyond the bytes currently buffered. */
static inline char *
wsd_http_find_crlf(char *buffer, size_t length)
{
	size_t i;

	if (!buffer)
		return NULL;
	for (i = 0; i + 1 < length; ++i) {
		if (buffer[i] == '\r' && buffer[i + 1] == '\n')
			return buffer + i;
	}
	return NULL;
}


#define WSD_HTTP_REQUEST_TIMEOUT_MS 5000U

/*
 * Use one absolute connection deadline across the request line, all headers
 * and the body. SO_RCVTIMEO alone is a per-recv timeout and can be defeated
 * by a peer that trickles one byte before each timeout expires.
 */
static inline int
wsd_http_deadline_after_ms(struct timespec *deadline, unsigned int timeout_ms)
{
	if (!deadline) {
		errno = EINVAL;
		return -1;
	}
	/* Wall-clock corrections must not extend or prematurely expire a request. */
	if (clock_gettime(CLOCK_MONOTONIC, deadline) != 0)
		return -1;
	deadline->tv_sec += (time_t)(timeout_ms / 1000U);
	deadline->tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
	if (deadline->tv_nsec >= 1000000000L) {
		deadline->tv_sec++;
		deadline->tv_nsec -= 1000000000L;
	}
	return 0;
}

static inline int
wsd_http_wait_readable_until(int fd, const struct timespec *deadline)
{
	struct pollfd descriptor;

	if (fd < 0 || !deadline) {
		errno = EINVAL;
		return -1;
	}
	descriptor.fd = fd;
	descriptor.events = POLLIN;
	descriptor.revents = 0;

	for (;;) {
		struct timespec now;
		long long remaining_ns, remaining_ms;
		int status;

		if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
			return -1;
		remaining_ns =
			((long long)deadline->tv_sec - (long long)now.tv_sec) *
			1000000000LL +
			((long long)deadline->tv_nsec - (long long)now.tv_nsec);
		if (remaining_ns <= 0) {
			errno = ETIMEDOUT;
			return -1;
		}
		remaining_ms = (remaining_ns + 999999LL) / 1000000LL;
		if (remaining_ms > INT_MAX)
			remaining_ms = INT_MAX;

		descriptor.revents = 0;
		status = poll(&descriptor, 1, (int)remaining_ms);
		if (status < 0 && errno == EINTR)
			continue;
		if (status < 0)
			return -1;
		if (status == 0) {
			errno = ETIMEDOUT;
			return -1;
		}
		if (descriptor.revents & POLLNVAL) {
			errno = EBADF;
			return -1;
		}
		if (descriptor.revents & (POLLIN | POLLHUP | POLLERR))
			return 0;
		errno = EIO;
		return -1;
	}
}

static inline ssize_t
wsd_http_recv_until(int fd, void *buffer, size_t length,
                    const struct timespec *deadline)
{
	if ((!buffer && length != 0) || !deadline) {
		errno = EINVAL;
		return -1;
	}
	if (length == 0)
		return 0;

	for (;;) {
		ssize_t received;

		if (wsd_http_wait_readable_until(fd, deadline) != 0)
			return -1;
		received = recv(fd, buffer, length, MSG_DONTWAIT);
		if (received < 0 &&
		    (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			continue;
		return received;
	}
}

static inline int
wsd_http_read_until_crlf_until(int fd, char *buffer, size_t *length,
                               size_t capacity,
                               const struct timespec *deadline)
{
	if (!buffer || !length || capacity == 0 || *length >= capacity ||
	    !deadline) {
		errno = EINVAL;
		return -1;
	}

	buffer[*length] = '\0';
	while (!wsd_http_find_crlf(buffer, *length)) {
		ssize_t received;

		if (*length >= capacity - 1) {
			errno = EMSGSIZE;
			return -1;
		}
		received = wsd_http_recv_until(
			fd, buffer + *length, capacity - *length - 1, deadline);
		if (received == 0) {
			errno = ECONNRESET;
			return -1;
		}
		if (received < 0)
			return -1;
		*length += (size_t)received;
		buffer[*length] = '\0';
	}
	return 0;
}

static inline int
wsd_http_read_exact_until(int fd, char *buffer, size_t *length,
                          size_t expected,
                          const struct timespec *deadline)
{
	if (!buffer || !length || *length > expected || !deadline) {
		errno = EINVAL;
		return -1;
	}

	while (*length < expected) {
		ssize_t received = wsd_http_recv_until(
			fd, buffer + *length, expected - *length, deadline);

		if (received == 0) {
			errno = ECONNRESET;
			return -1;
		}
		if (received < 0)
			return -1;
		*length += (size_t)received;
	}
	return 0;
}

/* Read one complete CRLF-terminated line without overrunning the buffer. */
static inline int
wsd_http_read_until_crlf(int fd, char *buffer, size_t *length,
                         size_t capacity)
{
	if (!buffer || !length || capacity == 0 || *length >= capacity) {
		errno = EINVAL;
		return -1;
	}

	buffer[*length] = '\0';
	while (!wsd_http_find_crlf(buffer, *length)) {
		ssize_t received;

		if (*length >= capacity - 1) {
			errno = EMSGSIZE;
			return -1;
		}
		received = recv(fd, buffer + *length,
		                capacity - *length - 1, 0);
		if (received < 0 && errno == EINTR)
			continue;
		if (received == 0) {
			errno = ECONNRESET;
			return -1;
		}
		if (received < 0)
			return -1;
		*length += (size_t)received;
		buffer[*length] = '\0';
	}
	return 0;
}

/* Receive exactly expected bytes, preserving bytes already buffered. */
static inline int
wsd_http_read_exact(int fd, char *buffer, size_t *length, size_t expected)
{
	if (!buffer || !length || *length > expected) {
		errno = EINVAL;
		return -1;
	}

	while (*length < expected) {
		ssize_t received = recv(fd, buffer + *length,
		                        expected - *length, 0);
		if (received < 0 && errno == EINTR)
			continue;
		if (received == 0) {
			errno = ECONNRESET;
			return -1;
		}
		if (received < 0)
			return -1;
		*length += (size_t)received;
	}
	return 0;
}

/* Content-Length is a non-zero decimal field, not atoi()-style prefix data. */
static inline int
wsd_http_parse_content_length(const char *value, size_t *length)
{
	const unsigned char *cursor = (const unsigned char *)value;
	size_t parsed = 0;

	if (!cursor || !length) {
		errno = EINVAL;
		return -1;
	}
	while (*cursor == ' ' || *cursor == '\t')
		++cursor;
	if (!isdigit(*cursor)) {
		errno = EINVAL;
		return -1;
	}
	do {
		unsigned int digit = (unsigned int)(*cursor - '0');
		if (parsed > ((size_t)-1 - digit) / 10) {
			errno = ERANGE;
			return -1;
		}
		parsed = parsed * 10 + digit;
		++cursor;
	} while (isdigit(*cursor));
	while (*cursor == ' ' || *cursor == '\t')
		++cursor;
	if (*cursor || parsed == 0) {
		errno = EINVAL;
		return -1;
	}
	*length = parsed;
	return 0;
}
/* This endpoint accepts bounded Content-Length bodies, not transfer coding. */
static inline int
wsd_http_is_transfer_encoding_header(const char *line)
{
	static const char field[] = "Transfer-Encoding:";

	return line && strncasecmp(line, field, sizeof(field) - 1) == 0;
}

#endif
