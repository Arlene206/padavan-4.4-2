#ifndef SWITCH_IPV4_PARSE_POLICY_H
#define SWITCH_IPV4_PARSE_POLICY_H

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

#endif /* SWITCH_IPV4_PARSE_POLICY_H */
