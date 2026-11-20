#ifndef MINIUPNPD_SOAP_INPUT_POLICY_H
#define MINIUPNPD_SOAP_INPUT_POLICY_H

#include <errno.h>
#include <limits.h>
#include <netinet/in.h>

/* Parse a non-empty unsigned decimal value with an explicit upper bound. */
static inline int
miniupnpd_soap_parse_unsigned(const char *text, unsigned long maximum,
                              unsigned long *value_out)
{
	const unsigned char *cursor;
	unsigned long value = 0;

	if (!text || text[0] == '\0' || !value_out) {
		errno = EINVAL;
		return -1;
	}

	cursor = (const unsigned char *)text;
	for (; *cursor != '\0'; ++cursor) {
		unsigned int digit;

		if (*cursor < '0' || *cursor > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(*cursor - '0');
		if ((unsigned long)digit > maximum ||
		    value > (maximum - (unsigned long)digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + (unsigned long)digit;
	}

	*value_out = value;
	return 0;
}

static inline int
miniupnpd_soap_parse_port(const char *text, unsigned short *port_out)
{
	unsigned long value;

	if (!port_out) {
		errno = EINVAL;
		return -1;
	}
	if (miniupnpd_soap_parse_unsigned(text, 65535UL, &value) != 0)
		return -1;
	*port_out = (unsigned short)value;
	return 0;
}

static inline int
miniupnpd_soap_parse_int(const char *text, int *value_out)
{
	unsigned long value;

	if (!value_out) {
		errno = EINVAL;
		return -1;
	}
	if (miniupnpd_soap_parse_unsigned(text, (unsigned long)INT_MAX,
	                                  &value) != 0)
		return -1;
	*value_out = (int)value;
	return 0;
}


static inline int
miniupnpd_soap_parse_nonzero_port(const char *text, unsigned short *port_out)
{
	unsigned short value;

	if (!port_out) {
		errno = EINVAL;
		return -1;
	}
	if (miniupnpd_soap_parse_port(text, &value) != 0)
		return -1;
	if (value == 0) {
		errno = EINVAL;
		return -1;
	}
	*port_out = value;
	return 0;
}

static inline int
miniupnpd_soap_parse_protocol_number(const char *text, int *protocol_out)
{
	unsigned long value;

	if (!protocol_out) {
		errno = EINVAL;
		return -1;
	}
	if (miniupnpd_soap_parse_unsigned(text, 65535UL, &value) != 0)
		return -1;
	*protocol_out = (int)value;
	return 0;
}

static inline int
miniupnpd_soap_ascii_equal_ci(const char *left, const char *right)
{
	while (*left && *right) {
		unsigned char a = (unsigned char)*left++;
		unsigned char b = (unsigned char)*right++;

		if (a >= 'a' && a <= 'z')
			a = (unsigned char)(a - 'a' + 'A');
		if (b >= 'a' && b <= 'z')
			b = (unsigned char)(b - 'a' + 'A');
		if (a != b)
			return 0;
	}
	return *left == '\0' && *right == '\0';
}

/* Required SOAP string arguments must exist; empty description remains valid. */
static inline int
miniupnpd_soap_required_text_present(const char *text)
{
	return text != NULL;
}

static inline int
miniupnpd_soap_protocol_valid(const char *protocol)
{
	return protocol &&
	       (miniupnpd_soap_ascii_equal_ci(protocol, "TCP") ||
	        miniupnpd_soap_ascii_equal_ci(protocol, "UDP")
#ifdef IPPROTO_UDPLITE
	        || miniupnpd_soap_ascii_equal_ci(protocol, "UDPLITE")
#endif
	       );
}

#endif /* MINIUPNPD_SOAP_INPUT_POLICY_H */
