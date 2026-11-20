#ifndef __LAN_ADDR6_RELOAD_POLICY_H__
#define __LAN_ADDR6_RELOAD_POLICY_H__

#include <errno.h>
#include <stdio.h>
#include <stddef.h>

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
