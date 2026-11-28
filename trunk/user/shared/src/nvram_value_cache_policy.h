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


/*
 * Returned pointers live in the current cache lap. Once the ring wraps, those
 * pointers are already eligible for reuse; erase the old lap before returning
 * storage to the first slot so shorter values cannot leave credential tails.
 */
static inline int
nvram_value_cache_reserve(char *cache, size_t capacity,
			  size_t min_reservation, size_t requested_bytes,
			  int *next_index)
{
	size_t index;
	size_t remaining;

	if (!cache || !next_index || *next_index < 0 ||
	    min_reservation > capacity || requested_bytes > capacity)
		return -1;

	index = (size_t)*next_index;
	if (index > capacity)
		return -1;

	remaining = capacity - index;
	if (remaining < min_reservation ||
	    (requested_bytes > 0 && requested_bytes > remaining)) {
		nvram_value_cache_secure_clear(cache, capacity, next_index);
		index = 0;
	}

	return (int)index;
}
#endif /* PADAVAN_NVRAM_VALUE_CACHE_POLICY_H */
