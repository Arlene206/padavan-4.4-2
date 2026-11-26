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

/*
 * Derive a remaining preferred lifetime from elapsed monotonic milliseconds.
 * Infinite lifetimes never decay; finite lifetimes saturate at zero. Wall-clock
 * adjustments therefore cannot extend or prematurely expire an IA's T1/T2.
 */
static inline uint32_t
dhcp6c_ia_preferred_lifetime_remaining(uint32_t lifetime,
				      unsigned long long elapsed_ms,
				      uint32_t infinite_lifetime)
{
	unsigned long long elapsed_seconds;

	if (lifetime == infinite_lifetime)
		return infinite_lifetime;

	elapsed_seconds = elapsed_ms / 1000ULL;
	if (elapsed_seconds >= (unsigned long long)lifetime)
		return 0U;
	return lifetime - (uint32_t)elapsed_seconds;
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
dhcp6c_ia_operation_error(int status, int saved_errno)
{
	if (status == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

static inline int
dhcp6c_ia_nobinding_requires_request(int is_renew_or_rebind,
				     int contains_nobinding)
{
	return is_renew_or_rebind && contains_nobinding;
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
/* Drain kernel cleanup retries before a SIGTERM shutdown may exit. */
static inline int dhcp6c_ia_shutdown_ready(int pending_ias, int pending_events)
{
	return pending_ias == 0 && pending_events == 0;
}

/* Do not create a new Solicit/Information Request after shutdown begins. */
static inline int dhcp6c_ia_client_start_allowed(int shutting_down)
{
	return !shutting_down;
}

#endif /* DHCP6C_IA_POLICY_H */
