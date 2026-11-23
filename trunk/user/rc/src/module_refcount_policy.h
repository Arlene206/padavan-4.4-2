#ifndef MODULE_REFCOUNT_POLICY_H
#define MODULE_REFCOUNT_POLICY_H

#include <errno.h>
#include <limits.h>

/*
 * Parse one /sys/module/<name>/refcnt value. sysfs emits a decimal integer
 * followed by a newline; accept an EOF-terminated value too for testability
 * and older vendor kernels, but never accept a partial or multi-line record.
 */
static inline int
module_refcount_parse(const char *text, int *refcount)
{
	unsigned int value = 0;
	unsigned int digits = 0;
	int digit;

	if (!refcount)
		return EINVAL;
	*refcount = -1;
	if (!text)
		return EINVAL;

	while (*text && *text != '\n') {
		if (*text < '0' || *text > '9')
			return EINVAL;
		if (digits >= 10)
			return ERANGE;
		digit = *text - '0';
		if (value > ((unsigned int)INT_MAX - (unsigned int)digit) / 10U)
			return ERANGE;
		value = value * 10U + (unsigned int)digit;
		++digits;
		++text;
	}
	if (digits == 0)
		return EINVAL;
	if (*text == '\n')
		++text;
	if (*text != '\0')
		return EINVAL;

	*refcount = (int)value;
	return 0;
}

#endif /* MODULE_REFCOUNT_POLICY_H */
