#ifndef PADAVAN_STATIC_ROUTE_POLICY_H
#define PADAVAN_STATIC_ROUTE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

/* Advanced_GWStaticRoute_Content.asp exposes at most 64 configured routes. */
#define STATIC_ROUTE_MAX_ENTRIES 64

static int
static_route_entry_count_is_safe(int count)
{
	return count >= 0 && count <= STATIC_ROUTE_MAX_ENTRIES;
}

/*
 * Empty metrics retain the old atoi("") == 0 behavior. Non-empty values must
 * be unsigned decimal and leave room for route_add()'s metric + 1 adjustment.
 */
static int
static_route_metric_parse(const char *value, int *metric)
{
	unsigned int parsed = 0;
	size_t i;

	if (!value || !metric)
		return EINVAL;
	*metric = 0;
	if (!value[0])
		return 0;

	for (i = 0; value[i]; ++i) {
		unsigned int digit;

		if (value[i] < '0' || value[i] > '9')
			return EINVAL;
		digit = (unsigned int)(value[i] - '0');
		if (parsed > ((unsigned int)INT_MAX - 1U - digit) / 10U)
			return ERANGE;
		parsed = parsed * 10U + digit;
	}

	*metric = (int)parsed;
	return 0;
}

/* Keep delimiter/control bytes out of the colon-separated route snapshot. */
static int
static_route_ipv4_field_is_safe(const char *value)
{
	size_t i, length;

	if (!value)
		return 0;
	length = strlen(value);
	if (length > 15)
		return 0;
	for (i = 0; i < length; ++i) {
		if (!((value[i] >= '0' && value[i] <= '9') || value[i] == '.'))
			return 0;
	}
	return 1;
}

static int
static_route_interface_is_safe(const char *value)
{
	if (!value)
		return 0;
	return value[0] == '\0' || strcmp(value, "LAN") == 0 ||
	       strcmp(value, "MAN") == 0 || strcmp(value, "WAN") == 0;
}

#endif /* PADAVAN_STATIC_ROUTE_POLICY_H */
