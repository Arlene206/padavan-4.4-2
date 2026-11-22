#ifndef PADAVAN_OPENVPN_IPV6_PROTOCOL_POLICY_H
#define PADAVAN_OPENVPN_IPV6_PROTOCOL_POLICY_H

#include <errno.h>

/* A read failure must preserve the requested IPv6 protocol. */
static inline int
openvpn_ipv6_protocol_apply_service_state(int *protocol,
                                          int service_status,
                                          int ipv6_type,
                                          int ipv6_disabled)
{
	if (!protocol)
		return EINVAL;
	if (*protocol <= 1)
		return 0;
	if (service_status != 0)
		return service_status;
	if (ipv6_type == ipv6_disabled)
		*protocol &= 1;
	return 0;
}

#endif
