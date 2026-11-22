#ifndef PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H
#define PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

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
    if (api_result == 0)
        return value ? EIO : 0;
    if (api_result != 1 || !value)
        return EIO;
    return 0;
}

/* Copy a checked string while the shared NVRAM cache pointer is still valid.
 * A missing key retains the empty-string default; read failures and truncation
 * remain distinguishable from a genuinely empty setting. */
static inline int
firewall_nvram_copy_string_status(int api_result, int saved_errno,
                                  const char *value, char *destination,
                                  size_t destination_size)
{
    const char *source;
    size_t length;
    int status;

    if (!destination || destination_size == 0)
        return EINVAL;
    destination[0] = '\0';

    status = firewall_nvram_read_status(api_result, saved_errno, value);
    if (status != 0)
        return status;

    source = (api_result == 0) ? "" : value;
    length = strlen(source);
    if (length >= destination_size)
        return ENAMETOOLONG;

    memcpy(destination, source, length + 1);
    return 0;
}

/* Copy an optional IPv4 NVRAM value without losing backend errors. Missing
 * or overlong values remain disabled, matching the legacy DMZ address gate. */
static inline int
firewall_nvram_optional_ipv4_copy_status(int api_result, int saved_errno,
					 const char *value, char *destination,
					 size_t destination_size)
{
	int status;
	size_t value_length;

	if (!destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';

	status = firewall_nvram_read_status(api_result, saved_errno, value);
	if (status != 0 || api_result == 0)
		return status;

	value_length = strlen(value);
	if (value_length >= destination_size)
		return 0;
	memcpy(destination, value, value_length + 1);
	return 0;
}

/* Keep the protocol tokens expected by the VTS filter/NAT emitters. */
static inline const char *
firewall_vts_protocol_name(const char *value)
{
    if (!value)
        return NULL;
    if (!strncasecmp(value, "Both", 4))
        return "both";
    if (!strncasecmp(value, "TCP", 3))
        return "tcp";
    if (!strncasecmp(value, "UDP", 3))
        return "udp";
    if (!strncasecmp(value, "OTHER", 5))
        return "other";
    return "tcp";
}
#endif /* PADAVAN_FIREWALL_NVRAM_MATCH_POLICY_H */
