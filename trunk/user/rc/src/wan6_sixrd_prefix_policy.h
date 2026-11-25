/* SPDX-License-Identifier: GPL-2.0 */
#ifndef WAN6_SIXRD_PREFIX_POLICY_H
#define WAN6_SIXRD_PREFIX_POLICY_H

/*
 * RFC 5969 requires the 6rd delegated prefix plus the IPv4 suffix bits to fit
 * in an IPv6 address. IPv4MaskLen is the number of shared IPv4 prefix bits;
 * the remaining (32 - IPv4MaskLen) bits are appended to the 6rd prefix.
 */
static inline int
wan6_sixrd_prefix_mapping_is_valid(int ipv4_mask_len, int ipv6_prefix_len)
{
	if (ipv4_mask_len < 0 || ipv4_mask_len > 32 ||
	    ipv6_prefix_len < 0 || ipv6_prefix_len > 128)
		return 0;

	return ipv6_prefix_len <= 128 - (32 - ipv4_mask_len);
}

/*
 * RFC 5969 says address bits after 6rdPrefixLen are reserved and must be
 * ignored by a CE. Clear them before either persisting or using the prefix.
 */
static inline int
wan6_sixrd_prefix_clear_host_bits(unsigned char *prefix, int prefix_len)
{
	int byte_index, remaining_bits;

	if (!prefix || prefix_len < 0 || prefix_len > 128)
		return 0;

	byte_index = prefix_len >> 3;
	remaining_bits = prefix_len & 7;
	if (remaining_bits) {
		prefix[byte_index] &= (unsigned char)(0xffU << (8 - remaining_bits));
		++byte_index;
	}
	while (byte_index < 16)
		prefix[byte_index++] = 0;

	return 1;
}

#endif /* WAN6_SIXRD_PREFIX_POLICY_H */
