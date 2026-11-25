/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_PD_SLA_POLICY_H
#define DHCP6C_PD_SLA_POLICY_H

#include <errno.h>
#include <stdint.h>

/* SLA IDs are right-aligned unsigned integers placed immediately after the
 * delegated prefix. The configured field must fit the 32-bit SLA ID.
 */
static inline int
dhcp6c_pd_sla_id_valid(int sla_len, uint32_t sla_id)
{
	if (sla_len < 0 || sla_len > 32 ||
	    (sla_len == 0 && sla_id != 0) ||
	    (sla_len > 0 && sla_len < 32 && (sla_id >> sla_len) != 0)) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

/* Canonicalize a delegated prefix before adding SLA and interface-ID bits. */
static inline int
dhcp6c_pd_prefix_clear_host_bits(unsigned char address[16], int prefix_len)
{
	int full_bytes, remaining_bits, i;

	if (!address || prefix_len < 0 || prefix_len > 128) {
		errno = EINVAL;
		return -1;
	}

	full_bytes = prefix_len / 8;
	remaining_bits = prefix_len % 8;
	if (remaining_bits) {
		address[full_bytes] &= (unsigned char)(0xffU << (8 - remaining_bits));
		full_bytes++;
	}
	for (i = full_bytes; i < 16; ++i)
		address[i] = 0;

	return 0;
}

/* Insert the SLA ID in network bit order without touching neighboring bits. */
static inline int
dhcp6c_pd_sla_insert(unsigned char address[16], int prefix_len,
		     int sla_len, uint32_t sla_id)
{
	int bit;

	if (!address || prefix_len < 0 || prefix_len > 128 ||
	    sla_len < 0 || sla_len > 32 ||
	    prefix_len > 128 - sla_len ||
	    dhcp6c_pd_sla_id_valid(sla_len, sla_id) != 0) {
		errno = EINVAL;
		return -1;
	}

	for (bit = 0; bit < sla_len; ++bit) {
		int destination_bit = prefix_len + bit;
		unsigned char mask = (unsigned char)(0x80U >> (destination_bit % 8));

		address[destination_bit / 8] &= (unsigned char)~mask;
		if ((sla_id >> (sla_len - bit - 1)) & 1U)
			address[destination_bit / 8] |= mask;
	}

	return 0;
}

#endif /* DHCP6C_PD_SLA_POLICY_H */
