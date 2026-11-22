#ifndef __LAN_ADDR6_RELOAD_POLICY_H__
#define __LAN_ADDR6_RELOAD_POLICY_H__

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


/*
 * Return whether the selected LAN IPv6 service uses a configured static
 * address. Disabled/6to4/6rd modes do not use this address; native-static
 * and 6in4 force it. Other modes retain nvram_match("ip6_lan_auto","0").
 */
static inline int
rc_lan_addr6_static_mode_status(int disabled, int excludes_lan_address,
				int forces_static, int auto_read_status,
				const char *auto_value, int *is_static)
{
	if (!is_static)
		return EINVAL;
	*is_static = 0;
	if (disabled || excludes_lan_address)
		return 0;
	if (forces_static) {
		*is_static = 1;
		return 0;
	}
	if (auto_read_status != 0)
		return auto_read_status;
	if (!auto_value)
		return EIO;

	*is_static = (strcmp(auto_value, "0") == 0);
	return 0;
}

/*
 * nvram_get_int() historically behaves like atoi(); missing, empty,
 * malformed and out-of-range prefix sizes all fall back to /64. Retain
 * that compatibility, but keep backend failures distinct from bad values.
 */
static inline int
rc_lan_addr6_legacy_prefix_size(int read_status, const char *text,
				int *prefix_size)
{
	char *end;
	long parsed = 0;

	if (!prefix_size)
		return EINVAL;
	if (read_status != 0)
		return read_status;

	if (text && text[0]) {
		errno = 0;
		parsed = strtol(text, &end, 10);
		if (end == text || errno == ERANGE ||
		    parsed < INT_MIN || parsed > INT_MAX)
			parsed = 0;
	}
	if (parsed < 48 || parsed > 64)
		parsed = 64;

	*prefix_size = (int)parsed;
	return 0;
}

typedef int (*rc_lan_addr6_clear_fn)(void);

/* NVRAM setters report failure through errno; never hide a failed publish. */
static inline int
rc_lan_addr6_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}
typedef int (*rc_lan_addr6_add_fn)(void *context);

/* Format address/prefix without overlapping input and output storage. */
static inline int
rc_lan_addr6_format_prefix(const char *address, int prefix_length,
                           char *output, size_t output_size)
{
    int written;

    if (!address || !address[0] || prefix_length < 0 || prefix_length > 128 ||
        !output || output_size == 0) {
        if (output && output_size > 0)
            output[0] = '\0';
        errno = EINVAL;
        return -1;
    }

    written = snprintf(output, output_size, "%s/%d", address, prefix_length);
    if (written < 0 || (size_t)written >= output_size) {
        output[0] = '\0';
        errno = written < 0 ? EIO : ENAMETOOLONG;
        return -1;
    }
    return 0;
}

/* Clear stale state before publishing a replacement; preserve either error. */
static inline int
rc_lan_addr6_reload_apply(int has_address,
                          rc_lan_addr6_clear_fn clear_address,
                          rc_lan_addr6_add_fn add_address,
                          void *context)
{
	int status;

	if (!clear_address || (has_address && !add_address))
		return EINVAL;

	status = clear_address();
	if (status != 0 || !has_address)
		return status;

	return add_address(context);
}

#endif
