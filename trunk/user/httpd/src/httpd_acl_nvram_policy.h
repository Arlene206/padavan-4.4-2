#ifndef HTTPD_ACL_NVRAM_POLICY_H
#define HTTPD_ACL_NVRAM_POLICY_H

#include <errno.h>
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

	/* Keep the legacy atoi() interpretation for readable NVRAM values. */
	*mode = atoi(value);
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
