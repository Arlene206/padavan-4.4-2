#ifndef LLMNR_QUESTION_POLICY_H
#define LLMNR_QUESTION_POLICY_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* Parse one uncompressed DNS question without reading outside the datagram. */
static inline int
llmnr_parse_question(const uint8_t *packet, size_t packet_len,
                     char *name, size_t name_capacity,
                     uint16_t *qtype, uint16_t *qclass)
{
	const uint8_t *cursor;
	const uint8_t *end;
	size_t name_len = 0;
	size_t wire_len = 0;

	if (!packet || !name || name_capacity == 0 || !qtype || !qclass ||
	    packet_len < 17)
		return -1;

	cursor = packet + 12;
	end = packet + packet_len;
	name[0] = '\0';

	for (;;) {
		size_t label_len;
		size_t separator_len;

		if (cursor >= end)
			return -1;
		label_len = *cursor++;
		if (label_len == 0)
			break;
		if (label_len > 63 || (size_t)(end - cursor) < label_len)
			return -1;
		if (wire_len > 255 - (label_len + 1))
			return -1;
		/* Dotted text form must be unambiguous and cannot contain C-string NUL. */
		{
			size_t i;
			for (i = 0; i < label_len; ++i) {
				if (cursor[i] == '\0' || cursor[i] == '.')
					return -1;
			}
		}
		wire_len += label_len + 1;

		separator_len = name_len ? 1 : 0;
		if (separator_len + label_len >= name_capacity - name_len)
			return -1;
		if (separator_len)
			name[name_len++] = '.';
		memcpy(name + name_len, cursor, label_len);
		name_len += label_len;
		name[name_len] = '\0';
		cursor += label_len;
	}

	/* Include the terminating root label in DNS's 255-octet wire limit. */
	if (wire_len >= 255 || (size_t)(end - cursor) < 4)
		return -1;
	*qtype = (uint16_t)(((uint16_t)cursor[0] << 8) | cursor[1]);
	*qclass = (uint16_t)(((uint16_t)cursor[2] << 8) | cursor[3]);
	return 0;
}

/* gethostname() may omit NUL when the supplied buffer is exactly filled. */
static inline int
llmnr_read_hostname(char *name, size_t capacity)
{
	int status;

	if (!name || capacity == 0)
		return -1;
	status = gethostname(name, capacity);
	name[capacity - 1] = '\0';
	return status;
}

#endif
