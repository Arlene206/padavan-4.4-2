#ifndef PADAVAN_IPV6_SERVICE_TYPE_POLICY_H
#define PADAVAN_IPV6_SERVICE_TYPE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <string.h>

/* nvram_get_checked(): 1=present, 0=missing, -1=backend/read/close error. */
static inline int
ipv6_service_type_parse_status(int api_result, int saved_errno,
                               const char *value,
                               const char *const *service_names,
                               unsigned int service_count,
                               int disabled_value,
                               int first_enabled_value,
                               int *service_type)
{
	unsigned int i;

	if (!service_type)
		return EINVAL;
	*service_type = disabled_value;
	if (api_result == 0)
		return 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result != 1 || !value)
		return EIO;
	if (!service_names || service_count == 0)
		return EINVAL;

	for (i = 0; i < service_count; ++i) {
		if (service_names[i] && strcmp(value, service_names[i]) == 0) {
			if (i > (unsigned int)(INT_MAX - first_enabled_value))
				return ERANGE;
			*service_type = first_enabled_value + (int)i;
			return 0;
		}
	}
	/* Unknown legacy values continue to mean "IPv6 disabled". */
	return 0;
}

#endif
