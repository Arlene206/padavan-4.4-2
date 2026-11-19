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
 * The SET ABI uses a NULL value pointer with zero length only for unset.
 * A non-NULL pointer with zero length is a valid empty-string assignment.
 */
static inline int
nvram_ioctl_set_value_request_valid(int length, int maximum_length,
                                    int has_pointer, int is_temp)
{
	if (length < 0 || maximum_length < 1 || length >= maximum_length ||
	    is_temp < 0 || is_temp > 1)
		return 0;

	return has_pointer || length == 0;
}

/*
 * GETALL is represented by exactly (length=0, pointer=NULL). A named
 * request must carry a pointer and a non-empty, NUL-free byte span. This
 * prevents malformed requests from silently widening into a full dump.
 */
static inline int
nvram_ioctl_param_request_valid(int length, int maximum_length,
                               int has_pointer, int has_embedded_nul)
{
	if (length < 0 || maximum_length < 1 || length >= maximum_length ||
	    has_embedded_nul)
		return 0;
	if (length == 0)
		return !has_pointer;
	return has_pointer;
}

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
