/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_WAN6_DNS_POLICY_H
#define PADAVAN_WAN6_DNS_POLICY_H

#include <errno.h>
#include <string.h>
#include <stddef.h>

#include "wan_nvram_status_policy.h"

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



#define WAN6_DNS_CONFIG_VALUE_COUNT 3

/*
 * Read the static DNS tuple under one checked NVRAM transaction. Each value
 * is copied before the next cache-backed read; any read or close failure
 * clears the entire tuple so callers cannot publish a mixed/empty fallback.
 */
static inline int
wan6_dns_copy_config_snapshot(
	const char *const *param_names, char *const *destinations,
	const size_t *destination_sizes,
	rc_wan_nvram_transaction_fn begin_transaction,
	rc_wan_nvram_get_fn get_value,
	rc_wan_nvram_transaction_fn end_transaction, void *context)
{
	struct rc_wan_nvram_snapshot_field fields[WAN6_DNS_CONFIG_VALUE_COUNT];
	unsigned int i;

	if (!param_names || !destinations || !destination_sizes ||
	    !begin_transaction || !get_value || !end_transaction) {
		if (destinations && destination_sizes) {
			for (i = 0; i < WAN6_DNS_CONFIG_VALUE_COUNT; ++i) {
				if (destinations[i] && destination_sizes[i])
					destinations[i][0] = '\0';
			}
		}
		return EINVAL;
	}

	for (i = 0; i < WAN6_DNS_CONFIG_VALUE_COUNT; ++i) {
		if (!param_names[i] || !param_names[i][0] ||
		    !destinations[i] || destination_sizes[i] == 0) {
			unsigned int j;

			for (j = 0; j < WAN6_DNS_CONFIG_VALUE_COUNT; ++j) {
				if (destinations[j] && destination_sizes[j])
					destinations[j][0] = '\0';
			}
			return EINVAL;
		}
		fields[i].param_name = param_names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = destination_sizes[i];
	}

	return rc_wan_nvram_copy_snapshot(
		fields, WAN6_DNS_CONFIG_VALUE_COUNT, begin_transaction,
		get_value, end_transaction, context);
}

#endif /* PADAVAN_WAN6_DNS_POLICY_H */
