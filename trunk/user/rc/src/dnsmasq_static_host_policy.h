#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>

/* A dnsmasq --dhcp-host MAC field must contain exactly twelve hex digits. */
static int
dnsmasq_static_mac_is_valid(const char *mac)
{
	size_t i;

	if (!mac)
		return 0;
	for (i = 0; i < 12; i++) {
		unsigned char value = (unsigned char)mac[i];

		if (!((value >= '0' && value <= '9') ||
		      (value >= 'a' && value <= 'f') ||
		      (value >= 'A' && value <= 'F')))
			return 0;
	}
	return mac[12] == '\0';
}

#endif /* PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H */
