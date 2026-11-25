#ifndef DETECT_LINK_NVRAM_POLICY_H
#define DETECT_LINK_NVRAM_POLICY_H

#include <errno.h>

typedef int (*detect_link_wan_status_write_fn)(void *context, int status);

/*
 * A negative link probe is an unknown sample, not link-down. Keep the
 * startup state unknown until the first successful switch read so callers
 * do not publish a fabricated initial zero to NVRAM.
 */
static inline int
detect_link_wan_sample_known(int was_known, int sample_result)
{
	return (was_known != 0 || sample_result >= 0);
}


static inline int
detect_link_publish_wan_status(int status, int *last_published,
			       detect_link_wan_status_write_fn write_status,
			       void *context)
{
	int result;

	if (!last_published || !write_status)
		return EINVAL;
	if (*last_published == status)
		return 0;

	result = write_status(context, status);
	if (result == 0)
		*last_published = status;

	return result;
}

/*
 * Update the cached AP/router topology only after a status-bearing NVRAM
 * read succeeds. On failure the caller keeps its last-known-good mode.
 */
static inline int
detect_link_apply_ap_mode_read(int read_status, int ap_mode,
			       int *cached_ap_mode)
{
	if (!cached_ap_mode)
		return EINVAL;
	if (read_status != 0)
		return read_status;
	*cached_ap_mode = ap_mode ? 1 : 0;
	return 0;
}

#endif
