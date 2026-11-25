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

/*
 * Strict 12-digit MAC parser. Outputs remain untouched on failure.
 */
static int
switch_parse_mac_address(const char *text, unsigned int *high,
                         unsigned int *low)
{
    unsigned int i, digit, parsed_high = 0, parsed_low = 0;
    char c;

    if (!text || !high || !low || strlen(text) != 12U)
        return -1;
    for (i = 0; i < 12U; ++i) {
        c = text[i];
        if (c >= '0' && c <= '9')
            digit = (unsigned int)(c - '0');
        else if (c >= 'a' && c <= 'f')
            digit = (unsigned int)(c - 'a') + 10U;
        else if (c >= 'A' && c <= 'F')
            digit = (unsigned int)(c - 'A') + 10U;
        else
            return -1;
        if (i < 8U)
            parsed_high = (parsed_high << 4) | digit;
        else
            parsed_low = (parsed_low << 4) | digit;
    }
    *high = parsed_high;
    *low = parsed_low;
    return 0;
}

/* Strict base-0 unsigned parse; rejects signs, whitespace and partial tokens. */
static int
switch_parse_uint_range(const char *text, unsigned int minimum,
                        unsigned int maximum, unsigned int *value)
{
    const unsigned char *p;
    unsigned int base = 10U, digit, parsed = 0, digits = 0;

    if (!text || !value || !*text || maximum < minimum ||
        *text == '+' || *text == '-' || *text == ' ' || *text == '\t' ||
        *text == '\n' || *text == '\r')
        return -1;
    p = (const unsigned char *)text;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        base = 16U;
        p += 2;
    } else if (p[0] == '0' && p[1] != '\0') {
        base = 8U;
        ++p;
    }
    while (*p) {
        if (*p >= '0' && *p <= '9')
            digit = (unsigned int)(*p - '0');
        else if (*p >= 'a' && *p <= 'f')
            digit = (unsigned int)(*p - 'a') + 10U;
        else if (*p >= 'A' && *p <= 'F')
            digit = (unsigned int)(*p - 'A') + 10U;
        else
            return -1;
        if (digit >= base || digit > maximum ||
            parsed > (maximum - digit) / base)
            return -1;
        parsed = parsed * base + digit;
        ++digits;
        ++p;
    }
    if (digits == 0U || parsed < minimum)
        return -1;
    *value = parsed;
    return 0;
}

/* Strict decimal parser; leading zeroes retain decimal CLI semantics. */ 
static int
switch_parse_decimal_range(const char *text, unsigned int minimum,
                           unsigned int maximum, unsigned int *value)
{
    const unsigned char *p;
    unsigned int digit, parsed = 0, digits = 0;

    if (!text || !value || !*text || maximum < minimum ||
        *text == '+' || *text == '-' || *text == ' ' || *text == '\t' ||
        *text == '\n' || *text == '\r')
        return -1;

    p = (const unsigned char *)text;
    while (*p) {
        if (*p < '0' || *p > '9')
            return -1;
        digit = (unsigned int)(*p - '0');
        if (digit > maximum || parsed > (maximum - digit) / 10U)
            return -1;
        parsed = parsed * 10U + digit;
        ++digits;
        ++p;
    }

    if (digits == 0U || parsed < minimum)
        return -1;

    *value = parsed;
    return 0;
}

/* Strict hexadecimal parser with optional 0x prefix and bounded output. */
static int
switch_parse_hex_range(const char *text, unsigned int maximum,
                       unsigned int *value)
{
    const unsigned char *p;
    unsigned int digit, parsed = 0, digits = 0;

    if (!text || !value || !*text ||
        *text == '+' || *text == '-' || *text == ' ' || *text == '\t' ||
        *text == '\n' || *text == '\r')
        return -1;

    p = (const unsigned char *)text;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
        p += 2;

    while (*p) {
        if (*p >= '0' && *p <= '9')
            digit = (unsigned int)(*p - '0');
        else if (*p >= 'a' && *p <= 'f')
            digit = (unsigned int)(*p - 'a') + 10U;
        else if (*p >= 'A' && *p <= 'F')
            digit = (unsigned int)(*p - 'A') + 10U;
        else
            return -1;

        if (digit > maximum || parsed > (maximum - digit) / 16U)
            return -1;
        parsed = parsed * 16U + digit;
        ++digits;
        ++p;
    }

    if (digits == 0U)
        return -1;

    *value = parsed;
    return 0;
}

/* Accept documented seven-bit maps and the legacy eight-character form. */
static int
switch_parse_table_portmap(const char *text, unsigned int *portmap)
{
    unsigned int i, parsed = 0;
    size_t length;

    if (!text || !portmap)
        return -1;
    length = strlen(text);
    if (length != 7U && length != 8U)
        return -1;
    for (i = 0; i < 7U; ++i) {
        if (text[i] != '0' && text[i] != '1')
            return -1;
        parsed |= (unsigned int)(text[i] - '0') << i;
    }
    if (length == 8U && text[7] != '0' && text[7] != '1')
        return -1;
    *portmap = parsed;
    return 0;
}

#endif /* SWITCH_IPV4_PARSE_POLICY_H */
