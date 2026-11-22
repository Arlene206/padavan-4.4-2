#ifndef DETECT_INTERNET_HOST_POLICY_H
#define DETECT_INTERNET_HOST_POLICY_H

#include <errno.h>
#include <stddef.h>

#include "wan_nvram_status_policy.h"

typedef int (*detect_internet_nvram_get_fn)(void *context,
					     const char *param_name,
					     char **value);

/*
 * Preserve nvram_safe_get_int()'s missing/malformed/out-of-range fallback
 * while keeping a backend read error distinct from an ordinary default.
 */
static inline int
detect_internet_read_int_setting(detect_internet_nvram_get_fn get_value,
				 void *context, const char *param_name,
				 int minimum, int maximum, int fallback,
				 int *value)
{
	char *text = NULL;
	int read_result, saved_errno, status, parsed;

	if (!get_value || !param_name || !param_name[0] || !value)
		return EINVAL;
	*value = fallback;

	errno = 0;
	read_result = get_value(context, param_name, &text);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(read_result, saved_errno);
	if (status != 0)
		return status;
	if (read_result == 0)
		parsed = fallback;
	else {
		if (!text)
			return EIO;
		status = rc_wan_nvram_parse_int_value(text, &parsed);
		if (status != 0)
			parsed = fallback;
	}

	return rc_wan_nvram_int_range_status(
		0, parsed, minimum, maximum, fallback, value);
}

/*
 * Read a probe address and port as one caller-owned endpoint. The address is
 * copied before the port lookup can recycle the shared NVRAM cache. If either
 * read fails, invalidate both outputs so callers cannot consume half a pair.
 */
static inline int
detect_internet_read_endpoint(detect_internet_nvram_get_fn get_value,
			      void *context, const char *address_key,
			      const char *port_key, char *address,
			      size_t address_size, int *port)
{
	int status;

	if (!get_value || !address_key || !address_key[0] ||
	    !port_key || !port_key[0] || !address || address_size == 0 ||
	    !port)
		return EINVAL;
	address[0] = '\0';
	*port = 0;

	status = rc_wan_nvram_copy_value(
		get_value, context, address_key, address, address_size);
	if (status != 0)
		return status;

	status = detect_internet_read_int_setting(
		get_value, context, port_key, 1, 65535, 53, port);
	if (status != 0) {
		address[0] = '\0';
		*port = 0;
	}
	return status;
}

/*
 * select() requires every descriptor passed to FD_SET() to be less than
 * FD_SETSIZE. Reject out-of-range descriptors before touching the fd_set.
 */
static inline int
detect_internet_select_fd_status(int fd, int fd_set_size)
{
	if (fd < 0 || fd_set_size <= 0)
		return EINVAL;
	if (fd >= fd_set_size)
		return EOVERFLOW;
	return 0;
}

#endif /* DETECT_INTERNET_HOST_POLICY_H */
