#ifndef DETECT_INTERNET_NVRAM_POLICY_H
#define DETECT_INTERNET_NVRAM_POLICY_H

#include <errno.h>

typedef int (*detect_internet_status_write_fn)(void *context, int status);

static inline int
detect_internet_publish_status(int status, int *last_published,
			       detect_internet_status_write_fn write_status,
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
