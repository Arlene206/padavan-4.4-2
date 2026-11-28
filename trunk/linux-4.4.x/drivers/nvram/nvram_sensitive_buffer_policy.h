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


/*
 * A value slot reserves value_capacity payload bytes plus one NUL byte.
 * Shorter replacements retain the slot capacity for future updates, so wipe
 * the bytes after the new terminator. Full release wipes the entire slot.
 */
static inline void
nvram_sensitive_value_clear_tail(char *value, size_t value_capacity,
                                  size_t value_length)
{
	if (!value || value_length >= value_capacity)
		return;

	nvram_sensitive_buffer_clear(value + value_length + 1,
				     value_capacity - value_length);
}

static inline void
nvram_sensitive_value_clear(char *value, size_t value_capacity)
{
	if (!value || value_capacity == (size_t)-1)
		return;

	nvram_sensitive_buffer_clear(value, value_capacity + 1);
}

#endif
