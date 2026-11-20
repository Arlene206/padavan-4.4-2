#ifndef MINIUPNPD_REDIRECT_DESC_POLICY_H
#define MINIUPNPD_REDIRECT_DESC_POLICY_H

#include <errno.h>
#include <stddef.h>

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

#endif
