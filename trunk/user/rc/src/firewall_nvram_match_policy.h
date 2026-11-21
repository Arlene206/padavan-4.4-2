#ifndef PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H
#define PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
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

/* Strict integer counterpart for numeric firewall modes such as macfilter. */
static inline int
firewall_nvram_int_status(int api_result, int saved_errno,
			  const char *value, int missing_value, int *number)
{
	char *end;
	long parsed;

	if (!number)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*number = missing_value;
		return 0;
	}
	if (api_result != 1 || !value || !value[0])
		return EIO;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
		return ERANGE;
	if (errno != 0)
		return errno;
	if (end == value || *end != '\0')
		return EINVAL;

	*number = (int)parsed;
	return 0;
}

/* Checked counterpart of legacy nvram_invmatch(key, "0"): an absent
 * value remains enabled, while backend/close and malformed-result errors
 * are propagated instead of being mistaken for an enabled setting. */
static inline int
firewall_nvram_not_zero_status(int api_result, int saved_errno,
                               const char *value, int missing_enabled,
                               int *enabled)
{
    int matches_zero, status;

    if (!enabled)
        return EINVAL;
    status = firewall_nvram_match_status(api_result, saved_errno,
                                         value, "0",
                                         missing_enabled ? 0 : 1,
                                         &matches_zero);
    if (status != 0)
        return status;

    *enabled = !matches_zero;
    return 0;
}

/* Classify nvram_get_checked() results without converting an ioctl/close
 * failure into a legacy empty-string value. Missing keys remain valid. */
static inline int
firewall_nvram_read_status(int api_result, int saved_errno, const char *value)
{
    if (api_result < 0)
        return saved_errno ? saved_errno : EIO;
    if (api_result > 1 || (api_result == 1 && !value))
        return EIO;
    return 0;
}
#endif /* PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H */
