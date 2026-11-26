/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_WAN6_ADDRESS_POLICY_H
#define PADAVAN_WAN6_ADDRESS_POLICY_H

#include <netinet/in.h>

/*
 * IPv4-compatible (::/96) and IPv4-mapped (::ffff:0:0/96) values are
 * transition/socket representations, not IPv6 interface or next-hop
 * endpoints. Do not let them pass a generic IPv6 unicast check.
 */
static inline int
wan6_address_has_embedded_ipv4(const struct in6_addr *address)
{
	unsigned int i;

	for (i = 0; i < 10U; ++i) {
		if (address->s6_addr[i] != 0)
			break;
	}
	if (i == 10U && address->s6_addr[10] == 0xff &&
	    address->s6_addr[11] == 0xff)
		return 1;

	for (i = 0; i < 12U; ++i) {
		if (address->s6_addr[i] != 0)
			return 0;
	}
	return 1;
}

/*
 * IPv6 interface addresses and explicit next hops must be usable unicast
 * endpoints. Unspecified, loopback, multicast and embedded-IPv4 forms are
 * not valid for these roles. Link-local and ULA addresses remain valid for
 * directly connected peers.
 */
static inline int
wan6_address_is_usable_unicast(const struct in6_addr *address)
{
	return address != NULL &&
	       !IN6_IS_ADDR_UNSPECIFIED(address) &&
	       !IN6_IS_ADDR_LOOPBACK(address) &&
	       !IN6_IS_ADDR_MULTICAST(address) &&
	       !wan6_address_has_embedded_ipv4(address);
}

#endif /* PADAVAN_WAN6_ADDRESS_POLICY_H */
