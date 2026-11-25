/* SPDX-License-Identifier: GPL-2.0 */
#ifndef WAN6_SIXRD_OPTION_POLICY_H
#define WAN6_SIXRD_OPTION_POLICY_H

#include <stddef.h>
#include <string.h>

/*
 * A DHCP option has at most 255 payload octets. After the 18-octet
 * 6rd header, at most 59 IPv4 relay addresses remain. The text formatter
 * expands them to at most 993 bytes, including separators and prefix text.
 */
#define WAN6_SIXRD_OPTION_VALUE_MAX 1024U
#define WAN6_SIXRD_IPV4_TEXT_SIZE 16U

typedef int (*wan6_sixrd_ipv4_validator_fn)(const char *address);

static inline int
wan6_sixrd_is_separator(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

/*
 * RFC 5969 defines one or more border-relay IPv4 addresses. Validate every
 * supplied token, but return the first because the current NVRAM/runtime
 * route model stores one relay. Never truncate an address or accept an
 * invalid trailing relay while silently using an earlier one.
 */
static inline int
wan6_sixrd_relay_list_first_valid(
	const char *relay_list, char *first_relay, size_t first_relay_size,
	wan6_sixrd_ipv4_validator_fn validate_ipv4)
{
	const char *cursor;
	char candidate[WAN6_SIXRD_IPV4_TEXT_SIZE];
	unsigned int relay_count = 0;

	if (first_relay && first_relay_size)
		first_relay[0] = '\0';
	if (!relay_list || !first_relay || first_relay_size == 0 ||
	    !validate_ipv4)
		return 0;

	cursor = relay_list;
	while (*cursor) {
		const char *start;
		size_t length;

		while (*cursor && wan6_sixrd_is_separator(*cursor))
			++cursor;
		if (!*cursor)
			break;

		start = cursor;
		while (*cursor && !wan6_sixrd_is_separator(*cursor))
			++cursor;
		length = (size_t)(cursor - start);
		if (length == 0 || length >= sizeof(candidate))
			goto invalid;

		memcpy(candidate, start, length);
		candidate[length] = '\0';
		if (!validate_ipv4(candidate))
			goto invalid;

		if (relay_count == 0) {
			if (length >= first_relay_size)
				goto invalid;
			memcpy(first_relay, candidate, length + 1);
		}
		++relay_count;
	}

	return relay_count != 0;

invalid:
	first_relay[0] = '\0';
	return 0;
}

#endif /* WAN6_SIXRD_OPTION_POLICY_H */
