#ifndef MINIUPNPD_PRESENTATION_URL_H
#define MINIUPNPD_PRESENTATION_URL_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

/* Port 0 and the default HTTP port (80) intentionally omit the suffix. */
static inline int
miniupnpd_format_lan_url(const char *lan_addr, int http_port,
			 char *buffer, size_t buffer_size)
{
	int length;

	if (buffer == NULL || buffer_size == 0) {
		errno = EINVAL;
		return -1;
	}
	buffer[0] = '\0';
	if (lan_addr == NULL || http_port < 0 || http_port > 65535) {
		errno = EINVAL;
		return -1;
	}

	if (http_port == 0 || http_port == 80)
		length = snprintf(buffer, buffer_size, "%s", lan_addr);
	else
		length = snprintf(buffer, buffer_size, "%s:%d", lan_addr, http_port);

	if (length < 0) {
		buffer[0] = '\0';
		errno = EIO;
		return -1;
	}
	if ((size_t)length >= buffer_size) {
		buffer[0] = '\0';
		errno = ENOSPC;
		return -1;
	}
	return 0;
}

#endif
