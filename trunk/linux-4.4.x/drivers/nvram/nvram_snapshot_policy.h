#ifndef NVRAM_SNAPSHOT_POLICY_H
#define NVRAM_SNAPSHOT_POLICY_H

#ifndef __KERNEL__
#include <stddef.h>
#endif

#define NVRAM_GET_SNAPSHOT_RETRIES 3

#define NVRAM_SNAPSHOT_COPY_INVALID (-1)
#define NVRAM_SNAPSHOT_COPY_READY 0
#define NVRAM_SNAPSHOT_COPY_RETRY 1

/*
 * A scan stopped at source_remaining means there was no terminator before
 * the backing pool ended. A scan stopped at destination_size means the
 * value grew after the first sizing pass and must be retried.
 */
static inline int
nvram_snapshot_copy_decision(size_t source_remaining,
                             size_t destination_size,
                             size_t observed_length)
{
	if (observed_length >= source_remaining)
		return NVRAM_SNAPSHOT_COPY_INVALID;
	if (observed_length >= destination_size)
		return NVRAM_SNAPSHOT_COPY_RETRY;

	return NVRAM_SNAPSHOT_COPY_READY;
}

#endif
