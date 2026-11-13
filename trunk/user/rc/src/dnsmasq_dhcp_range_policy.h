#ifndef PADAVAN_DNSMASQ_DHCP_RANGE_POLICY_H
#define PADAVAN_DNSMASQ_DHCP_RANGE_POLICY_H

#include <stdint.h>

static int
dnsmasq_dhcp_range_mask_valid(uint32_t netmask)
{
	uint32_t hostmask;

	if (netmask == 0U)
		return 0;
	hostmask = ~netmask;
	return hostmask >= 3U && (hostmask & (hostmask + 1U)) == 0U;
}

static int
dnsmasq_dhcp_range_pool_is_valid(uint32_t lan_ip, uint32_t netmask,
				 uint32_t pool_start, uint32_t pool_end)
{
	uint32_t network, broadcast, hostmask;

	if (!dnsmasq_dhcp_range_mask_valid(netmask))
		return 0;
	hostmask = ~netmask;
	network = lan_ip & netmask;
	broadcast = network | hostmask;
	if (lan_ip <= network || lan_ip >= broadcast)
		return 0;
	if (pool_start <= network || pool_start >= broadcast ||
	    pool_end <= network || pool_end >= broadcast)
		return 0;
	if (pool_start > pool_end)
		return 0;
	if (lan_ip >= pool_start && lan_ip <= pool_end)
		return 0;
	return 1;
}

static int
dnsmasq_dhcp_range_default_pool(uint32_t lan_ip, uint32_t netmask,
				uint32_t *pool_start, uint32_t *pool_end)
{
	uint32_t network, broadcast, hostmask;
	uint32_t first, last, below, above;

	if (!pool_start || !pool_end ||
	    !dnsmasq_dhcp_range_mask_valid(netmask))
		return 0;
	hostmask = ~netmask;
	network = lan_ip & netmask;
	broadcast = network | hostmask;
	if (lan_ip <= network || lan_ip >= broadcast)
		return 0;

	first = network + 1U;
	last = broadcast - 1U;
	if (lan_ip >= first && lan_ip <= last) {
		below = lan_ip - first;
		above = last - lan_ip;
		if (above > below)
			first = lan_ip + 1U;
		else
			last = lan_ip - 1U;
	}
	if (first > last)
		return 0;
	*pool_start = first;
	*pool_end = last;
	return 1;
}

#endif
