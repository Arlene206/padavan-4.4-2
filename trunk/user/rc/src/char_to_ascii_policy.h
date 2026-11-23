#ifndef PADAVAN_RC_CHAR_TO_ASCII_POLICY_H
#define PADAVAN_RC_CHAR_TO_ASCII_POLICY_H

#include <errno.h>
#include <stddef.h>

#define RC_WLAN_SSID_MAX_LEN 32U
#define RC_CHAR_TO_ASCII_MAX_OUTPUT_SIZE (RC_WLAN_SSID_MAX_LEN * 3U + 1U)

static inline int
rc_char_to_ascii_is_unescaped(unsigned char value)
{
	return (value >= '0' && value <= '9') ||
	       (value >= 'A' && value <= 'Z') ||
	       (value >= 'a' && value <= 'z') ||
	       value == '!' || value == '*' ||
	       value == '(' || value == ')' ||
	       value == '_' || value == '-' ||
	       value == '\'' || value == '.';
}

/*
 * Encode one WLAN SSID as uppercase %HH escapes. Input and output must not
 * overlap. The caller supplies capacity explicitly; on failure output is
 * unchanged. SSIDs longer than the 802.11 32-octet limit are rejected.
 */
static inline int
rc_char_to_ascii_encode(char *output, size_t output_size, const char *input)
{
	static const char hex[] = "0123456789ABCDEF";
	const unsigned char *source;
	char *destination;
	size_t input_length = 0;
	size_t required = 1;

	if (!output || !input || output_size == 0) {
		errno = EINVAL;
		return -1;
	}

	for (source = (const unsigned char *)input; *source; source++) {
		size_t encoded_length;

		if (input_length == RC_WLAN_SSID_MAX_LEN) {
			errno = EMSGSIZE;
			return -1;
		}
		input_length++;
		encoded_length = rc_char_to_ascii_is_unescaped(*source) ? 1U : 3U;
		if (required > (size_t)-1 - encoded_length) {
			errno = EOVERFLOW;
			return -1;
		}
		required += encoded_length;
	}
	if (required > output_size) {
		errno = ENOSPC;
		return -1;
	}

	destination = output;
	for (source = (const unsigned char *)input; *source; source++) {
		unsigned char value = *source;

		if (rc_char_to_ascii_is_unescaped(value)) {
			*destination++ = (char)value;
		} else {
			*destination++ = '%';
			*destination++ = hex[value >> 4];
			*destination++ = hex[value & 0x0f];
		}
	}
	*destination = '\0';
	return 0;
}

#endif /* PADAVAN_RC_CHAR_TO_ASCII_POLICY_H */
