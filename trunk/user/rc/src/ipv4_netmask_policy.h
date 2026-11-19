#ifndef PADAVAN_IPV4_NETMASK_POLICY_H
#define PADAVAN_IPV4_NETMASK_POLICY_H

#include <arpa/inet.h>
#include <stdint.h>

/* Require a nonzero, contiguous dotted-IPv4 prefix before ip2class(). */
static int
ipv4_netmask_is_contiguous(const char *value)
{
	struct in_addr mask;
	uint32_t host_mask;

	if (!value || inet_pton(AF_INET, value, &mask) != 1)
		return 0;

	host_mask = ntohl(mask.s_addr);
	return host_mask != 0U &&
	       (host_mask | (host_mask - 1U)) == UINT32_MAX;
}

#endif /* PADAVAN_IPV4_NETMASK_POLICY_H */
