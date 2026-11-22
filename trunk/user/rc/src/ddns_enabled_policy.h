#ifndef PADAVAN_DDNS_ENABLED_POLICY_H
#define PADAVAN_DDNS_ENABLED_POLICY_H

#include <errno.h>
#include <string.h>

/* A failed enable-key read must not be mistaken for DDNS being disabled. */
static inline int
inadyn_ddns_enabled_status(int api_result, int saved_errno,
                           const char *value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	*enabled = 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !value)
		return EIO;
	*enabled = (strcmp(value, "1") == 0);
	return 0;
}

#endif /* PADAVAN_DDNS_ENABLED_POLICY_H */
