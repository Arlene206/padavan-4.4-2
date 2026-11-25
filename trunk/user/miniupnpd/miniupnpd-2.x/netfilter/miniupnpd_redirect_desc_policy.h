#ifndef MINIUPNPD_REDIRECT_DESC_POLICY_H
#define MINIUPNPD_REDIRECT_DESC_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

typedef void *(*miniupnpd_redirect_desc_allocator)(size_t);

static inline void *
miniupnpd_redirect_desc_allocate(size_t header_size, size_t text_size,
                                  miniupnpd_redirect_desc_allocator allocator)
{
	if (!allocator || text_size == 0) {
		errno = EINVAL;
		return NULL;
	}
	if (text_size > (size_t)-1 - header_size) {
		errno = EOVERFLOW;
		return NULL;
	}
	return allocator(header_size + text_size);
}

/*
 * SOAP-supplied descriptions may exceed callers' fixed-size output buffers.
 * Reserve a terminator byte even when the source must be truncated.
 */
static inline int
miniupnpd_redirect_desc_copy_bounded(char *destination,
                                     size_t destination_size,
                                     const char *source)
{
	size_t copy_size;

	if (!destination || destination_size == 0 || !source) {
		errno = EINVAL;
		return -1;
	}
	copy_size = strlen(source);
	if (copy_size >= destination_size)
		copy_size = destination_size - 1;
	if (copy_size > 0)
		memcpy(destination, source, copy_size);
	destination[copy_size] = '\0';
	return 0;
}

#endif
