#ifndef PADAVAN_VPN_SERVER_CONFIG_POLICY_H
#define PADAVAN_VPN_SERVER_CONFIG_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * The VPN server is started unconditionally by PID 1. A missing enable key
 * means disabled; a failed NVRAM read must not be interpreted as permission
 * to start a remotely reachable service.
 */
static inline int
vpn_server_config_enable_status(int read_status, int read_errno,
                                const char *value, int *enabled)
{
	if (!enabled)
		return EINVAL;

	*enabled = 0;
	if (read_status < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_status == 0)
		return 0;
	if (read_status != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

/*
 * Preserve the historical PPTP default for an absent type key, but reject
 * backend errors, malformed text and unsupported modes instead of silently
 * turning them into type 0.
 */
static inline int
vpn_server_config_type_status(int read_status, int read_errno,
                              const char *value, int *type)
{
	const char *cursor;
	char *end;
	long parsed;

	if (!type)
		return EINVAL;
	if (read_status < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_status == 0) {
		*type = 0;
		return 0;
	}
	if (read_status != 1 || !value)
		return EIO;

	cursor = value;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (!isdigit((unsigned char)*cursor))
		return EINVAL;
	while (isdigit((unsigned char)*cursor))
		++cursor;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (*cursor)
		return EINVAL;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE)
		return ERANGE;
	if (end == value || parsed < 0 || parsed > 2)
		return EINVAL;

	*type = (int)parsed;
	return 0;
}

#endif /* PADAVAN_VPN_SERVER_CONFIG_POLICY_H */
