#ifndef NVRAM_COMPACTION_POLICY_H
#define NVRAM_COMPACTION_POLICY_H

#ifndef __KERNEL__
#include <stddef.h>
#include <string.h>
#endif

/*
 * Return the length of one NUL-terminated value wholly inside the used part
 * of the backing pool. The length excludes the terminator.
 */
static inline int
nvram_compaction_value_length(const char *pool, size_t pool_used,
                              size_t value_offset, size_t *value_length)
{
	size_t remaining, length;

	if (!pool || !value_length || value_offset >= pool_used)
		return -1;

	remaining = pool_used - value_offset;
	length = strnlen(pool + value_offset, remaining);
	if (length >= remaining)
		return -1;

	*value_length = length;
	return 0;
}

/* An arena copy consumes the value bytes plus their terminating NUL. */
static inline int
nvram_compaction_fits(size_t capacity, size_t used, size_t value_length)
{
	if (used > capacity || value_length == (size_t)-1)
		return 0;

	return value_length + 1 <= capacity - used;
}

/*
 * A flash entry is "name=value\0". Check addition overflow as well as the
 * remaining image space; callers reserve the final double-NUL separately.
 */
static inline int
nvram_generate_entry_size(size_t available, size_t name_length,
                          size_t value_length, size_t *entry_length)
{
	size_t maximum = (size_t)-1;
	size_t required;

	if (!entry_length || value_length > maximum - 2 ||
	    name_length > maximum - value_length - 2)
		return 0;

	required = name_length + value_length + 2;
	if (required > available)
		return 0;

	*entry_length = required;
	return 1;
}

#endif
