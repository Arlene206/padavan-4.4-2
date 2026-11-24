#ifndef __UQMI_CLIENT_ID_POLICY_H
#define __UQMI_CLIENT_ID_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/* QMI service client IDs are one-octet values; reject truncation and aliases. */
static inline int
uqmi_parse_client_id(const char *text, int *client_id)
{
	const char *number;
	char *end;
	unsigned long value;

	if (!text || !client_id)
		return EINVAL;

	number = text;
	while (isspace((unsigned char)*number))
		number++;
	if (!*number || *number == '-')
		return EINVAL;

	errno = 0;
	value = strtoul(text, &end, 0);
	if (end == text || errno == ERANGE || value > UCHAR_MAX || *end)
		return EINVAL;

	*client_id = (int)value;
	return 0;
}

#endif
