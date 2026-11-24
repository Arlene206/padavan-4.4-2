#ifndef __STORAGE_HOTPLUG_NAME_POLICY_H__
#define __STORAGE_HOTPLUG_NAME_POLICY_H__

#include <stddef.h>

/* Parse a whole unsigned decimal token without accepting signs or suffixes. */
static inline int
storage_hotplug_parse_decimal_token(const char *token, unsigned int minimum,
				    unsigned int maximum, unsigned int *value_out)
{
	const unsigned char *p;
	unsigned int value = 0, digit;

	if (!token || !*token || !value_out || minimum > maximum)
		return -1;
	p = (const unsigned char *)token;
	for (; *p; ++p) {
		if (*p < '0' || *p > '9')
			return -1;
		digit = (unsigned int)(*p - '0');
		if (digit > maximum || value > (maximum - digit) / 10)
			return -1;
		value = value * 10 + digit;
	}
	if (value < minimum)
		return -1;
	*value_out = value;
	return 0;
}

/* snprintf succeeds only when its complete output fits and no error occurred. */
static inline int
storage_hotplug_name_format_status(int format_status, size_t buffer_size)
{
	return (format_status >= 0 && (size_t)format_status < buffer_size) ? 0 : -1;
}

#endif
