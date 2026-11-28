#ifndef PADAVAN_IPTV_PROXY_ENABLE_PORT_POLICY_H
#define PADAVAN_IPTV_PROXY_ENABLE_PORT_POLICY_H

#include <errno.h>

/*
 * udpxy and xupnpd use their *_enable_x NVRAM selectors as HTTP ports.
 * Preserve the legacy disabled range (0..1023), but do not silently
 * reinterpret negative values as disabled configuration.
 */
#define IPTV_PROXY_ENABLE_PORT_MIN 1024
#define IPTV_PROXY_ENABLE_PORT_MAX 65535

static inline int
iptv_proxy_enable_port_status(int configured_port, int *enabled_out)
{
	if (!enabled_out)
		return EINVAL;

	*enabled_out = 0;
	if (configured_port < 0 ||
	    configured_port > IPTV_PROXY_ENABLE_PORT_MAX)
		return ERANGE;

	*enabled_out = configured_port >= IPTV_PROXY_ENABLE_PORT_MIN;
	return 0;
}

#endif /* PADAVAN_IPTV_PROXY_ENABLE_PORT_POLICY_H */
