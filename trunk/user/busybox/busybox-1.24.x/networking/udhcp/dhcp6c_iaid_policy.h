/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_IAID_POLICY_H
#define DHCP6C_IAID_POLICY_H

#include <errno.h>
#include <stdint.h>

/* IAIDs in the client configuration are unsigned 32-bit decimal values.
 * Unlike atoi(), this rejects signs, trailing data and values that overflow.
 */
static inline int
dhcp6c_iaid_parse(const char *text, uint32_t *iaid)
{
	const unsigned char *p;
	unsigned long value, digit;

	if (!text || !text[0] || !iaid) {
		errno = EINVAL;
		return -1;
	}

	value = 0;
	for (p = (const unsigned char *)text; *p; ++p) {
		if (*p < '0' || *p > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned long)(*p - '0');
		if (value > (0xffffffffUL - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}

	*iaid = (uint32_t)value;
	return 0;
}

#endif /* DHCP6C_IAID_POLICY_H */
