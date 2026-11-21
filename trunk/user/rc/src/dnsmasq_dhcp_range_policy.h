#ifndef PADAVAN_DNSMASQ_DHCP_RANGE_POLICY_H
#define PADAVAN_DNSMASQ_DHCP_RANGE_POLICY_H

#include <stdint.h>
#include <errno.h>

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


typedef int (*dnsmasq_dhcp_range_nvram_set_fn)(const char *name,
					       const char *value);

/* Update the corrected DHCP pool as a pair. If either NVRAM write fails,
 * restore both original values and report when that rollback is incomplete. */
static int
dnsmasq_dhcp_range_store_nvram_pair(const char *old_start,
				    const char *old_end,
				    const char *new_start,
				    const char *new_end,
				    dnsmasq_dhcp_range_nvram_set_fn set_value,
				    int *rollback_failed)
{
	int saved_errno, rollback_status = 0;

	if (rollback_failed)
		*rollback_failed = 0;
	if (!old_start || !old_end || !new_start || !new_end || !set_value) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if (set_value("dhcp_start", new_start) == 0) {
		errno = 0;
		if (set_value("dhcp_end", new_end) == 0)
			return 0;
	}
	saved_errno = errno ? errno : EIO;

	/* Restore in reverse write order, attempting both even after one failure. */
	errno = 0;
	if (set_value("dhcp_end", old_end) != 0)
		rollback_status = 1;
	errno = 0;
	if (set_value("dhcp_start", old_start) != 0)
		rollback_status = 1;

	if (rollback_failed)
		*rollback_failed = rollback_status;
	errno = rollback_status ? EIO : saved_errno;
	return -1;
}

#endif
