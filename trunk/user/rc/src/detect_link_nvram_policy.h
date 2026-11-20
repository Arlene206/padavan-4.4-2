#ifndef DETECT_LINK_NVRAM_POLICY_H
#define DETECT_LINK_NVRAM_POLICY_H

#include <errno.h>

typedef int (*detect_link_wan_status_write_fn)(void *context, int status);

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

#endif
