#ifndef __HEV_DNS_FORWARDER_POLICY_H__
#define __HEV_DNS_FORWARDER_POLICY_H__

#include <errno.h>
#include <stdlib.h>

/* Accept only an ASCII-decimal UDP port in the inclusive range 1..65535. */
static inline int
hev_dns_forwarder_parse_port (const char *text, unsigned short *port)
{
	const char *cursor = NULL;
	char *end = NULL;
	unsigned long value = 0;

	if (!text || !port || !*text)
		return EINVAL;

	for (cursor=text; *cursor; cursor++) {
		if (*cursor < '0' || *cursor > '9')
			return EINVAL;
	}

	errno = 0;
	value = strtoul (text, &end, 10);
	if (ERANGE == errno)
		return ERANGE;
	if (end == text || !end || *end || 0 == value || value > 65535)
		return EINVAL;

	*port = (unsigned short) value;
	return 0;
}

#endif /* __HEV_DNS_FORWARDER_POLICY_H__ */
