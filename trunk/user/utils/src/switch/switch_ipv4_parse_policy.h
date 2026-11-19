#ifndef SWITCH_IPV4_PARSE_POLICY_H
#define SWITCH_IPV4_PARSE_POLICY_H

#include <string.h>

/*
 * Parse a strict dotted-decimal IPv4 address without temporary buffers.
 * Leading zeroes remain accepted for compatibility with the previous CLI.
 * On failure, the caller's output value is left untouched.
 */
static int
switch_parse_ipv4_address(const char *text, unsigned int *address)
{
	const unsigned char *cursor;
	unsigned int octets[4];
	unsigned int count = 0;
	unsigned int octet;
	unsigned int digit;
	unsigned int parsed;

	if (text == 0 || address == 0 || *text == '\0')
		return -1;

	cursor = (const unsigned char *)text;
	while (*cursor != '\0') {
		if (*cursor < '0' || *cursor > '9')
			return -1;

		octet = 0;
		while (*cursor >= '0' && *cursor <= '9') {
			digit = (unsigned int)(*cursor - '0');
			if (octet > 25U || (octet == 25U && digit > 5U))
				return -1;
			octet = octet * 10U + digit;
			cursor++;
		}

		if (count >= 4U)
			return -1;
		octets[count++] = octet;

		if (*cursor == '\0')
			break;
		if (*cursor != '.' || count >= 4U)
			return -1;
		cursor++;
		if (*cursor == '\0')
			return -1;
	}

	if (count != 4U)
		return -1;

	parsed = (octets[0] << 24) | (octets[1] << 16) |
		 (octets[2] << 8) | octets[3];
	*address = parsed;
	return 0;
}

/*
 * The legacy map has eight characters; bits 0..6 encode switch ports and
 * bit 7 is reserved. Validate all input while preserving the seven-bit map.
 */
static int
switch_parse_portmap(const char *text, unsigned int *portmap)
{
	unsigned int i;
	unsigned int parsed = 0;

	if (!text || !portmap || strlen(text) != 8U)
		return -1;

	for (i = 0; i < 8U; ++i) {
		if (text[i] != '0' && text[i] != '1')
			return -1;
		if (i < 7U)
			parsed |= (unsigned int)(text[i] - '0') << i;
	}

	*portmap = parsed;
	return 0;
}

#endif /* SWITCH_IPV4_PARSE_POLICY_H */
