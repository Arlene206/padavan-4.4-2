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

#endif /* WAN6_SIXRD_PREFIX_POLICY_H */
