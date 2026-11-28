#ifndef NVRAM_SENSITIVE_BUFFER_POLICY_H
#define NVRAM_SENSITIVE_BUFFER_POLICY_H

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <stddef.h>
#endif

/* NVRAM staging buffers can contain wireless keys, VPN secrets and other data. */
#ifndef NVRAM_SENSITIVE_BUFFER_ZERO
#ifdef __KERNEL__
#define NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length) \
	memzero_explicit((buffer), (length))
#else
#include <string.h>
#define NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length) \
	memset((buffer), 0, (length))
#endif
#endif

static inline void
nvram_sensitive_buffer_clear(void *buffer, size_t length)
{
	if (buffer && length)
		NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length);
}

#endif
