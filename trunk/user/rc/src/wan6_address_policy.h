/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_WAN6_ADDRESS_POLICY_H
#define PADAVAN_WAN6_ADDRESS_POLICY_H

#include <netinet/in.h>

/*
 * IPv6 interface addresses and explicit next hops must be usable unicast
 * endpoints. Unspecified and multicast addresses are not valid addresses
 * for these roles, and loopback is only meaningful on the local host.
 * Link-local and ULA addresses remain valid for directly connected peers.
 */
static inline int
wan6_address_is_usable_unicast(const struct in6_addr *address)
{
	return address != NULL &&
	       !IN6_IS_ADDR_UNSPECIFIED(address) &&
	       !IN6_IS_ADDR_LOOPBACK(address) &&
	       !IN6_IS_ADDR_MULTICAST(address);
}

#endif /* PADAVAN_WAN6_ADDRESS_POLICY_H */
