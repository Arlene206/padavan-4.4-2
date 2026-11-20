#ifndef MINIUPNPD_LEASE_FILE_POLICY_H
#define MINIUPNPD_LEASE_FILE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX 63

/* Keep lease records single-line and within reload_from_lease_file()'s
 * 128-byte input buffer. Description text is otherwise opaque. */
static inline int
miniupnpd_lease_record_format(char *buffer, size_t buffer_size,
                              const char *protocol, unsigned short eport,
                              const char *iaddr, unsigned short iport,
                              unsigned int timestamp, const char *description)
{
	char safe_description[MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX + 1];
	size_t description_length = 0;
	int length;

	if (buffer && buffer_size > 0)
		buffer[0] = '\0';
	if (!buffer || buffer_size == 0 || !protocol || !iaddr) {
		errno = EINVAL;
		return -1;
	}

	if (description) {
		while (description[description_length] != '\0' &&
		       description_length < MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX) {
			char ch = description[description_length];

			if (ch == '\r' || ch == '\n')
				ch = ' ';
			safe_description[description_length] = ch;
			++description_length;
		}
	}
	safe_description[description_length] = '\0';

	length = snprintf(buffer, buffer_size, "%s:%hu:%s:%hu:%u:%s\n",
	                  protocol, eport, iaddr, iport, timestamp,
	                  safe_description);
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
	return length;
}

#endif /* MINIUPNPD_LEASE_FILE_POLICY_H */
