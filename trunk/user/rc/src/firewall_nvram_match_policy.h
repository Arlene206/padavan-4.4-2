#ifndef PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H
#define PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H

#include <errno.h>
#include <string.h>

/* nvram_get_checked(): 1=value present, 0=key absent, -1=read/backend error.
 * Missing values use the explicit legacy default; errors never become false. */
static inline int
firewall_nvram_match_status(int api_result, int saved_errno,
			    const char *value, const char *expected,
			    int missing_match, int *matches)
{
	if (!expected || !matches)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*matches = missing_match != 0;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*matches = strcmp(value, expected) == 0;
	return 0;
}

#endif /* PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H */
