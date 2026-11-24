#ifndef STORAGE_TIME_FILE_POLICY_H
#define STORAGE_TIME_FILE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <time.h>

/*
 * The persistent writer stores an unsigned decimal epoch with no suffix.
 * Accept one optional final LF for old/manual files, but reject signs,
 * base prefixes, embedded whitespace, trailing data and time_t overflow.
 */
static inline int
rc_storage_time_parse_status(const char *buffer, size_t length,
                             time_t *storage_time)
{
	unsigned long parsed = 0;
	time_t converted;
	size_t i;

	if (!buffer || !storage_time)
		return EINVAL;
	if (length > 0 && buffer[length - 1] == '\n')
		length--;
	if (length == 0)
		return EINVAL;

	for (i = 0; i < length; i++) {
		unsigned int digit;

		if (buffer[i] < '0' || buffer[i] > '9')
			return EINVAL;
		digit = (unsigned int)(buffer[i] - '0');
		if (parsed > (ULONG_MAX - digit) / 10UL)
			return EOVERFLOW;
		parsed = parsed * 10UL + digit;
	}
	if (parsed == 0)
		return EINVAL;

	converted = (time_t)parsed;
	if (converted <= (time_t)0 || (unsigned long)converted != parsed)
		return EOVERFLOW;
	*storage_time = converted;
	return 0;
}

#endif /* STORAGE_TIME_FILE_POLICY_H */
