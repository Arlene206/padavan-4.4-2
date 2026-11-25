#ifndef PADAVAN_IPTV_PROXY_ALTNET_POLICY_H
#define PADAVAN_IPTV_PROXY_ALTNET_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

#define IPTV_PROXY_ALTN_DEFAULT "0.0.0.0/0"

/* Strict dotted-decimal IPv4/CIDR accepted by the bundled inet_addr(). */
static inline int
iptv_proxy_altnet_is_valid(const char *value)
{
	const unsigned char *cursor = (const unsigned char *)value;
	unsigned int octet_index, octet, digits, prefix;
	int all_ones = 1;

	if (cursor == NULL || *cursor == '\0')
		return 0;

	for (octet_index = 0; octet_index < 4; ++octet_index) {
		octet = 0;
		digits = 0;
		/* BusyBox/inet_addr treats multi-digit leading-zero octets as octal. */
		if (*cursor == '0' && cursor[1] >= '0' && cursor[1] <= '9')
			return 0;
		while (*cursor >= '0' && *cursor <= '9') {
			unsigned int digit = (unsigned int)(*cursor - '0');

			if (octet > 25 || (octet == 25 && digit > 5))
				return 0;
			octet = octet * 10 + digit;
			++digits;
			++cursor;
		}
		if (digits == 0)
			return 0;
		if (octet != 255U)
			all_ones = 0;
		if (octet_index < 3) {
			if (*cursor != '.')
				return 0;
			++cursor;
		} else {
			if (*cursor != '/')
				return 0;
			++cursor;
		}
	}

	prefix = 0;
	digits = 0;
	while (*cursor >= '0' && *cursor <= '9') {
		unsigned int digit = (unsigned int)(*cursor - '0');

		if (prefix > 3 || (prefix == 3 && digit > 2))
			return 0;
		prefix = prefix * 10 + digit;
		++digits;
		++cursor;
	}
	/* inet_addr() uses INADDR_NONE for 255.255.255.255; reject that alias. */
	return digits != 0 && *cursor == '\0' && !all_ones;
}

/* An empty optional setting keeps the historical all-addresses default. */
static inline int
iptv_proxy_altnet_normalize(char *value, size_t capacity)
{
	static const char default_value[] = IPTV_PROXY_ALTN_DEFAULT;
	size_t length = sizeof(default_value);

	if (value == NULL || capacity == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	if (value[0] == '\0') {
		if (length > capacity) {
			errno = ENAMETOOLONG;
			return ENAMETOOLONG;
		}
		memcpy(value, default_value, length);
		return 0;
	}
	if (!iptv_proxy_altnet_is_valid(value)) {
		errno = EINVAL;
		return EINVAL;
	}
	return 0;
}

#endif /* PADAVAN_IPTV_PROXY_ALTNET_POLICY_H */
