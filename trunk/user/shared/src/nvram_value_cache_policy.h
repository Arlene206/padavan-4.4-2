#ifndef PADAVAN_NVRAM_VALUE_CACHE_POLICY_H
#define PADAVAN_NVRAM_VALUE_CACHE_POLICY_H

#include <stddef.h>

/* Transaction values can contain credentials. Erase the complete cache with
 * volatile stores when its transaction lease ends, then reset its ring cursor. */
static inline void
nvram_value_cache_secure_clear(char *cache, size_t capacity, int *next_index)
{
	volatile unsigned char *cursor;
	size_t remaining;

	if (cache) {
		cursor = (volatile unsigned char *)cache;
		remaining = capacity;
		while (remaining > 0) {
			*cursor++ = 0;
			--remaining;
		}
	}
	if (next_index)
		*next_index = 0;
}

#endif /* PADAVAN_NVRAM_VALUE_CACHE_POLICY_H */
