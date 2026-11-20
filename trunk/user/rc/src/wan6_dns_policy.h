/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_WAN6_DNS_POLICY_H
#define PADAVAN_WAN6_DNS_POLICY_H

#include <errno.h>
#include <string.h>

/* Missing DNS options are represented as an empty automatic server list. */
static inline int
wan6_dns_update_required(const char *current, const char *incoming)
{
	const char *old_value = current ? current : "";
	const char *new_value = incoming ? incoming : "";

	return strcmp(old_value, new_value) != 0;
}

typedef int (*wan6_dns_set_value_fn)(void *context, const char *dns_value);

/*
 * Return 0 for no change, 1 for a published change, and a negative errno-style
 * result when required NVRAM publication fails.
 */
static inline int
wan6_dns_store_if_changed(const char *old_dns, const char *new_dns,
			  wan6_dns_set_value_fn set_value, void *context)
{
	int status;

	if (!new_dns || !set_value)
		return -EINVAL;
	if (!wan6_dns_update_required(old_dns, new_dns))
		return 0;

	status = set_value(context, new_dns);
	if (status == 0)
		return 1;
	return (status > 0) ? -status : status;
}

#endif /* PADAVAN_WAN6_DNS_POLICY_H */
