#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>

/* A dnsmasq --dhcp-host MAC field must contain exactly twelve hex digits. */
static int
dnsmasq_static_mac_is_valid(const char *mac)
{
	size_t i;
	unsigned int first_octet = 0;
	unsigned int digit;
	int nonzero = 0;

	if (!mac)
		return 0;
	for (i = 0; i < 12; i++) {
		unsigned char value = (unsigned char)mac[i];

		if (value >= '0' && value <= '9')
			digit = value - '0';
		else if (value >= 'a' && value <= 'f')
			digit = value - 'a' + 10U;
		else if (value >= 'A' && value <= 'F')
			digit = value - 'A' + 10U;
		else
			return 0;

		if (i == 0)
			first_octet = digit << 4;
		else if (i == 1)
			first_octet |= digit;
		if (digit != 0)
			nonzero = 1;
	}
	if (mac[12] != '\0')
		return 0;

	/* DHCP client hardware addresses must be nonzero unicast Ethernet MACs. */
	return nonzero && !(first_octet & 1U);
}

#endif /* PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H */
