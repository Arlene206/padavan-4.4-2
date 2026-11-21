#ifndef PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H
#define PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H

#include <errno.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

/*
 * nvram_get_checked() returns 1 for a present value, 0 for a missing key,
 * and -1 for a backend/read/close failure. Preserve nvram_match(..., "1")
 * semantics for present values while keeping missing-key defaults explicit.
 */
static inline int
dnsmasq_nvram_option_status(int api_result, int saved_errno,
			    const char *value, int missing_enabled,
			    int *enabled)
{
	if (!enabled)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*enabled = missing_enabled != 0;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

static inline int
dnsmasq_nvram_invmatch_status(int api_result, int saved_errno,
			      const char *value, const char *match_value,
			      int missing_matches, int *matches)
{
	if (!matches || !match_value ||
	    (missing_matches != 0 && missing_matches != 1))
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*matches = missing_matches;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*matches = strcmp(value, match_value) != 0;
	return 0;
}

static inline int
dnsmasq_nvram_integer_boolean_status(int api_result, int saved_errno,
				     const char *value, int missing_enabled,
				     int *enabled)
{
	int parsed, status;

	if (!enabled || (missing_enabled != 0 && missing_enabled != 1))
		return EINVAL;

	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					      value, missing_enabled, &parsed);
	if (status != 0)
		return status;

	*enabled = parsed != 0;
	return 0;
}

/*
 * Parse an integer option completely and enforce its documented live range.
 * Missing keys retain the caller's historical default, even if it is outside
 * the range for a present value; malformed present values never fall back.
 */
static inline int
dnsmasq_nvram_integer_range_status(int api_result, int saved_errno,
				   const char *text, int missing_value,
				   int minimum, int maximum, int *result)
{
	int parsed, status;

	if (!result || minimum > maximum)
		return EINVAL;
	if (api_result == 0) {
		if (text)
			return EIO;
		*result = missing_value;
		return 0;
	}
	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					     text, missing_value, &parsed);
	if (status != 0)
		return status;
	if (parsed < minimum || parsed > maximum)
		return ERANGE;
	*result = parsed;
	return 0;
}

static inline int
dnsmasq_nvram_string_status(int api_result, int saved_errno,
			    const char *value, const char *missing_value,
			    char *destination, size_t destination_size)
{
	size_t value_length;

	if (!destination || destination_size == 0)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		value = missing_value ? missing_value : "";
	} else if (api_result != 1 || !value) {
		return EIO;
	}

	value_length = strlen(value);
	if (value_length >= destination_size)
		return ENAMETOOLONG;

	memcpy(destination, value, value_length + 1);
	return 0;
}

#endif /* PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H */
