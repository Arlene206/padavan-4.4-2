#ifndef HTTPD_ACL_NVRAM_POLICY_H
#define HTTPD_ACL_NVRAM_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/*
 * Preserve nvram_get_int()'s empty/missing-key default, but distinguish a
 * backend failure from the configured "ACL disabled" mode.
 */
static inline int
httpd_acl_nvram_mode_status(int read_result, int saved_errno,
                            const char *value, int *mode, int *valid)
{
	if (!mode || !valid)
		return EINVAL;

	*mode = 0;
	*valid = 0;
	if (read_result == 0) {
		*valid = 1;
		return 0;
	}
	if (read_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (read_result != 1 || !value)
		return EIO;

	/*
	 * Preserve decimal integer settings, but do not let corrupt text (for
	 * example, "invalid") turn the ACL off through atoi()'s zero fallback.
	 * Whitespace and an optional sign remain accepted for compatibility.
	 */
	{
		char *end;
		long parsed;

		errno = 0;
		parsed = strtol(value, &end, 10);
		if (end == value)
			return EINVAL;
		while (*end && isspace((unsigned char)*end))
			++end;
		if (*end != '\\0' || parsed < 0)
			return EINVAL;
		if (errno == ERANGE || parsed > INT_MAX)
			return ERANGE;
		*mode = (int)parsed;
	}
	*valid = 1;
	return 0;
}

/* An unknown ACL setting denies remote clients but preserves loopback access. */
static inline int
httpd_acl_client_allowed(int configuration_valid, int is_loopback)
{
	return configuration_valid || is_loopback;
}

#endif /* HTTPD_ACL_NVRAM_POLICY_H */
