#ifndef __HEV_DNS_FORWARDER_POLICY_H__
#define __HEV_DNS_FORWARDER_POLICY_H__

#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>

enum {
	HEV_DNS_FORWARDER_UDP_DRAIN_ERROR = -1,
	HEV_DNS_FORWARDER_UDP_DRAIN_STOP = 0,
	HEV_DNS_FORWARDER_UDP_DRAIN_PACKET = 1,
	HEV_DNS_FORWARDER_UDP_DRAIN_RETRY = 2
};

/* Interpret nonblocking UDP readiness, including EINTR and empty datagrams. */
static inline int
hev_dns_forwarder_udp_drain_action (ssize_t result, int error)
{
	if (0 <= result)
		return HEV_DNS_FORWARDER_UDP_DRAIN_PACKET;
	if (EINTR == error)
		return HEV_DNS_FORWARDER_UDP_DRAIN_RETRY;
	if (EAGAIN == error || EWOULDBLOCK == error)
		return HEV_DNS_FORWARDER_UDP_DRAIN_STOP;
	return HEV_DNS_FORWARDER_UDP_DRAIN_ERROR;
}

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
