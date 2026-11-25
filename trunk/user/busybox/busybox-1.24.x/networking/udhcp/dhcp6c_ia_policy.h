/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_IA_POLICY_H
#define DHCP6C_IA_POLICY_H

#include <errno.h>
#include <stdint.h>

#define DHCP6C_IA_CLEANUP_RETRY_DELAY_MS 5000ULL

/*
 * Calculate the 4/5 and 5/8 timer ratios without overflowing 32-bit DHCP
 * lifetime fields before division. Quotient/remainder arithmetic is exact.
 */
static inline uint32_t dhcp6c_ia_t2_from_duration(uint32_t duration)
{
	return (duration / 5U) * 4U + ((duration % 5U) * 4U) / 5U;
}

static inline uint32_t dhcp6c_ia_t1_from_t2(uint32_t t2)
{
	return (t2 / 8U) * 5U + ((t2 % 8U) * 5U) / 8U;
}
static inline int
dhcp6c_ia_pd_target_needs_apply(int is_dhcp_interface, int already_installed)
{
	return !is_dhcp_interface && !already_installed;
}
static inline int
dhcp6c_ia_first_error(int first_error, int next_error)
{
	return first_error != 0 ? first_error : next_error;
}
static inline int
dhcp6c_ia_retry_required(int status)
{
	return status != 0;
}

/* A duplicate add or deletion of an absent address already has the target state. */
static inline int
dhcp6c_ia_ifaddr_change_complete(int is_add, int error, int already_owned)
{
	if (error == 0)
		return 1;
	if (is_add)
		return error == EEXIST && already_owned;
	return error == ENOENT || error == EADDRNOTAVAIL;
}

static inline int
dhcp6c_ia_na_address_needs_remove(int address_owned)
{
	return address_owned != 0;
}

/* Only finite, nonzero valid lifetimes need an expiry timer. */
static inline int
dhcp6c_ia_prefix_expiry_timer_required(unsigned long valid_lifetime,
				       unsigned long infinite_lifetime)
{
	return valid_lifetime != 0UL && valid_lifetime != infinite_lifetime;
}
#endif /* DHCP6C_IA_POLICY_H */
