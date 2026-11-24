#ifndef INFOSVR_PACKET_POLICY_H
#define INFOSVR_PACKET_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Initialize the entire receive buffer, including bytes not supplied by UDP. */
static inline int
infosvr_packet_buffer_prepare(void *packet, size_t size)
{
	if (!packet || size == 0) {
		errno = EINVAL;
		return -1;
	}

	memset(packet, 0, size);
	return 0;
}

/* Reject a datagram whose real wire length exceeds its receive buffer. */
static inline int
infosvr_packet_length_fits(size_t received, size_t capacity)
{
	return capacity != 0 && received <= capacity;
}

/* Validate the minimum wire prefix before a caller dereferences it. */
static inline int
infosvr_packet_length_valid(size_t received, size_t required)
{
	return required != 0 && received >= required;
}

#endif
