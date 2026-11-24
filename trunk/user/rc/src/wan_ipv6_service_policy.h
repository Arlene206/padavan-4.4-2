#ifndef PADAVAN_WAN_IPV6_SERVICE_POLICY_H
#define PADAVAN_WAN_IPV6_SERVICE_POLICY_H

#include <errno.h>

/*
 * Preserve the distinction between a disabled service and an unreadable
 * service-type setting. The output uses -1 for disabled/unknown, 0 for false,
 * and 1 for true; a non-zero return always means the decision is unavailable.
 */
static inline int
wan_ipv6_service_is_sit_status(int read_status, int service_type,
                               int disabled_type, int sixin4_type,
                               int six2four_type, int sixrd_type,
                               int *is_sit)
{
	if (!is_sit)
		return EINVAL;
	*is_sit = -1;
	if (read_status != 0)
		return read_status;
	if (service_type == disabled_type)
		return 0;

	*is_sit = (service_type == sixin4_type ||
	           service_type == six2four_type ||
	           service_type == sixrd_type) ? 1 : 0;
	return 0;
}

static inline int
wan_ipv6_service_addr_static_status(int read_status, int service_type,
                                    int disabled_type, int native_static_type,
                                    int native_dhcp_type, int sixin4_type,
                                    int six2four_type, int sixrd_type,
                                    int sixrd_dhcp_read_status,
                                    int sixrd_is_manual, int *is_static)
{
	if (!is_static)
		return EINVAL;
	*is_static = -1;
	if (read_status != 0)
		return read_status;
	if (service_type == disabled_type)
		return 0;

	if (service_type == native_dhcp_type ||
	    service_type == six2four_type) {
		*is_static = 0;
		return 0;
	}
	if (service_type == native_static_type ||
	    service_type == sixin4_type) {
		*is_static = 1;
		return 0;
	}
	if (service_type == sixrd_type) {
		if (sixrd_dhcp_read_status != 0)
			return sixrd_dhcp_read_status;
		*is_static = sixrd_is_manual ? 1 : 0;
		return 0;
	}

	*is_static = 0;
	return 0;
}

static inline int
wan_ipv6_service_dns_static_status(int read_status, int service_type,
                                   int disabled_type, int native_static_type,
                                   int native_dhcp_type, int sixin4_type,
                                   int six2four_type, int sixrd_type,
                                   int dns_auto_read_status,
                                   int dns_auto_is_manual, int *is_static)
{
	if (!is_static)
		return EINVAL;
	*is_static = -1;
	if (read_status != 0)
		return read_status;
	if (service_type == disabled_type)
		return 0;

	if (service_type == native_dhcp_type) {
		if (dns_auto_read_status != 0)
			return dns_auto_read_status;
		*is_static = dns_auto_is_manual ? 1 : 0;
		return 0;
	}
	if (service_type == native_static_type ||
	    service_type == sixin4_type ||
	    service_type == six2four_type ||
	    service_type == sixrd_type)
		*is_static = 1;
	else
		*is_static = 0;
	return 0;
}

#endif /* PADAVAN_WAN_IPV6_SERVICE_POLICY_H */
