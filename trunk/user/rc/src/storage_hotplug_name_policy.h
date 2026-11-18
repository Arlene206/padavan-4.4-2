#ifndef __STORAGE_HOTPLUG_NAME_POLICY_H__
#define __STORAGE_HOTPLUG_NAME_POLICY_H__

#include <stddef.h>

/* snprintf succeeds only when its complete output fits and no error occurred. */
static inline int
storage_hotplug_name_format_status(int format_status, size_t buffer_size)
{
	return (format_status >= 0 && (size_t)format_status < buffer_size) ? 0 : -1;
}

#endif
