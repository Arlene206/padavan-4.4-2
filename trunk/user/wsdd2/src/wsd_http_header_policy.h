#ifndef WSD_HTTP_HEADER_POLICY_H
#define WSD_HTTP_HEADER_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <sys/socket.h>

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

#endif
