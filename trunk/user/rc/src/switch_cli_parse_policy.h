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

static inline int
switch_cli_parse_uint_max(const char *text, int allow_hex_prefix,
			 unsigned int maximum, unsigned int *value)
{
	unsigned int parsed;
	int status;

	if (!value)
		return EINVAL;

	status = switch_cli_parse_uint(text, allow_hex_prefix, &parsed);
	if (status != 0)
		return status;
	if (parsed > maximum)
		return ERANGE;

	*value = parsed;
	return 0;
}

static inline int
switch_cli_parse_ioctl_command(const char *text, unsigned int command_bits,
			       unsigned int *command)
{
	const unsigned int word_bits = (unsigned int)(sizeof(unsigned int) * CHAR_BIT);

	if (!command || command_bits == 0U || command_bits >= word_bits)
		return EINVAL;

	return switch_cli_parse_uint_max(text, 0,
					 (1U << command_bits) - 1U, command);
}

static inline int
switch_cli_parse_ioctl_parameter(const char *text, unsigned int command_bits,
				 int allow_hex_prefix, unsigned int *parameter)
{
	const unsigned int word_bits = (unsigned int)(sizeof(unsigned int) * CHAR_BIT);

	if (!parameter || command_bits == 0U || command_bits >= word_bits)
		return EINVAL;

	return switch_cli_parse_uint_max(text, allow_hex_prefix,
					 UINT_MAX >> command_bits, parameter);
}

#endif /* SWITCH_CLI_PARSE_POLICY_H */
