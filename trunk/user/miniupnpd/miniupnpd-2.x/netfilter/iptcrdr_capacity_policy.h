#ifndef MINIUPNPD_IPTCRDR_CAPACITY_POLICY_H
#define MINIUPNPD_IPTCRDR_CAPACITY_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>

/* Double an element capacity without overflowing size_t or unsigned int. */
static inline int
miniupnpd_portmapping_capacity_next(unsigned int current, size_t *next)
{
	size_t maximum_elements;

	if (!next || current == 0 || current > UINT_MAX / 2) {
		errno = EOVERFLOW;
		return -1;
	}
	maximum_elements = ((size_t)-1) / sizeof(unsigned short);
	if ((size_t)current > maximum_elements / 2) {
		errno = EOVERFLOW;
		return -1;
	}
	*next = (size_t)current * 2;
	return 0;
}

#endif
