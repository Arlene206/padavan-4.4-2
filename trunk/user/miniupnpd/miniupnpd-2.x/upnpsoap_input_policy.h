#ifndef MINIUPNPD_SOAP_INPUT_POLICY_H
#define MINIUPNPD_SOAP_INPUT_POLICY_H

#include <errno.h>
#include <limits.h>

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

#endif /* MINIUPNPD_SOAP_INPUT_POLICY_H */
