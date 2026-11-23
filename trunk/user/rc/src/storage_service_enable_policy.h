#ifndef PADAVAN_STORAGE_SERVICE_ENABLE_POLICY_H
#define PADAVAN_STORAGE_SERVICE_ENABLE_POLICY_H

#include <errno.h>
#include <string.h>

/* Preserve nvram_match(key, "1")'s exact-string semantics while retaining
 * backend failures instead of silently treating them as a disabled service. */
static inline int
storage_service_enable_value_status(int read_status, const char *stored_value,
                                    int *enabled)
{
	if (!enabled || !stored_value) {
		errno = EINVAL;
		return EINVAL;
	}
	*enabled = 0;
	if (read_status != 0) {
		errno = read_status > 0 ? read_status : EIO;
		return errno;
	}
	*enabled = (strcmp(stored_value, "1") == 0);
	return 0;
}

#endif /* PADAVAN_STORAGE_SERVICE_ENABLE_POLICY_H */
