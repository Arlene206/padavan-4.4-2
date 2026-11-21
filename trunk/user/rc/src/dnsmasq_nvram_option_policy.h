#ifndef PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H
#define PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H

#include <errno.h>
#include <string.h>

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

#endif /* PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H */
