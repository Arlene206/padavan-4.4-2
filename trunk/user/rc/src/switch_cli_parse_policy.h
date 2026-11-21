#ifndef SWITCH_CLI_PARSE_POLICY_H
#define SWITCH_CLI_PARSE_POLICY_H

#include <errno.h>
#include <limits.h>

/*
 * Parse an unsigned CLI number without accepting signs, whitespace, trailing
 * junk or overflow. Decimal is the default; legacy switch operands may also
 * use an explicit 0x/0X hexadecimal prefix. Output is untouched on failure.
 */
static inline int
switch_cli_parse_uint(const char *text, int allow_hex_prefix,
                      unsigned int *value)
{
	const unsigned char *cursor;
	unsigned int base = 10;
	unsigned int parsed = 0;

	if (!text || !text[0] || !value)
		return EINVAL;

	cursor = (const unsigned char *)text;
	if (allow_hex_prefix && cursor[0] == '0' &&
	    (cursor[1] == 'x' || cursor[1] == 'X')) {
		base = 16;
		cursor += 2;
		if (!cursor[0])
			return EINVAL;
	}

	for (; *cursor; ++cursor) {
		unsigned int digit;

		if (*cursor >= '0' && *cursor <= '9')
			digit = (unsigned int)(*cursor - '0');
		else if (base == 16 && *cursor >= 'a' && *cursor <= 'f')
			digit = (unsigned int)(*cursor - 'a') + 10U;
		else if (base == 16 && *cursor >= 'A' && *cursor <= 'F')
			digit = (unsigned int)(*cursor - 'A') + 10U;
		else
			return EINVAL;

		if (digit >= base)
			return EINVAL;
		if (parsed > (UINT_MAX - digit) / base)
			return ERANGE;
		parsed = parsed * base + digit;
	}

	*value = parsed;
	return 0;
}

#endif /* SWITCH_CLI_PARSE_POLICY_H */
