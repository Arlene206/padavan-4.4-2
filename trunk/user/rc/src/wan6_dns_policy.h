/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_WAN6_DNS_POLICY_H
#define PADAVAN_WAN6_DNS_POLICY_H

#include <string.h>

/* Missing DNS options are represented as an empty automatic server list. */
static inline int
wan6_dns_update_required(const char *current, const char *incoming)
{
	const char *old_value = current ? current : "";
	const char *new_value = incoming ? incoming : "";

	return strcmp(old_value, new_value) != 0;
}

#endif /* PADAVAN_WAN6_DNS_POLICY_H */
