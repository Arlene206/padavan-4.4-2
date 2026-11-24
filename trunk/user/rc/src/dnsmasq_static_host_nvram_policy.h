#ifndef PADAVAN_DNSMASQ_STATIC_HOST_NVRAM_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_NVRAM_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

#define DNSMASQ_STATIC_HOST_MAX 64
#define DNSMASQ_STATIC_HOST_MAC_SIZE 32
#define DNSMASQ_STATIC_HOST_IPV4_SIZE 16
#define DNSMASQ_STATIC_HOST_NAME_SIZE 256

struct dnsmasq_static_host_nvram_entry {
	char mac[DNSMASQ_STATIC_HOST_MAC_SIZE];
	char ip[DNSMASQ_STATIC_HOST_IPV4_SIZE];
	char hostname[DNSMASQ_STATIC_HOST_NAME_SIZE];
};

struct dnsmasq_static_host_nvram_snapshot {
	int enabled;
	int arp_enabled;
	unsigned int item_count;
	struct dnsmasq_static_host_nvram_entry entries[DNSMASQ_STATIC_HOST_MAX];
};

/* Missing keys retain old zero/empty defaults. Backend, parse, truncation or
 * transaction-close failures invalidate the complete static-host snapshot. */
static inline int
dnsmasq_static_host_nvram_snapshot_read(
	struct dnsmasq_static_host_nvram_snapshot *snapshot,
	rc_wan_nvram_transaction_fn begin_transaction,
	rc_wan_nvram_get_fn get_value,
	rc_wan_nvram_transaction_fn end_transaction,
	void *context)
{
	char text[32];
	char name[40];
	int api_result, saved_errno, status, end_status;
	int item_count = 0;
	unsigned int i;
	int length;

	if (!snapshot || !begin_transaction || !get_value || !end_transaction)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));

	errno = 0;
	api_result = begin_transaction(context);
	saved_errno = errno;
	status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status != 0)
		return status;

	status = rc_wan_nvram_copy_value(
		get_value, context, "dhcp_static_x", text, sizeof(text));
	if (status == 0)
		status = rc_wan_nvram_parse_int_value(text, &snapshot->enabled);
	if (status == 0 && snapshot->enabled != 0 && snapshot->enabled != 1)
		status = EINVAL;

	if (status == 0 && snapshot->enabled == 1) {
		status = rc_wan_nvram_copy_value(
			get_value, context, "dhcp_static_arp", text, sizeof(text));
		if (status == 0) {
			status = rc_wan_nvram_parse_int_value(text, &snapshot->arp_enabled);
			if (status == 0 && snapshot->arp_enabled != 0 &&
			    snapshot->arp_enabled != 1)
				status = EINVAL;
		}

		if (status == 0)
			status = rc_wan_nvram_copy_value(
				get_value, context, "dhcp_staticnum_x", text, sizeof(text));
		if (status == 0)
			status = rc_wan_nvram_parse_int_value(text, &item_count);

		if (status == 0) {
			if (item_count < 0)
				item_count = 0;
			if (item_count > DNSMASQ_STATIC_HOST_MAX)
				item_count = DNSMASQ_STATIC_HOST_MAX;
			snapshot->item_count = (unsigned int)item_count;
		}
	}

	for (i = 0; status == 0 && snapshot->enabled == 1 &&
		    i < snapshot->item_count; ++i) {
		length = snprintf(name, sizeof(name), "dhcp_staticmac_x%u", i);
		if (length < 0 || length >= (int)sizeof(name)) {
			status = EOVERFLOW;
			break;
		}
		status = rc_wan_nvram_copy_value(
			get_value, context, name, snapshot->entries[i].mac,
			sizeof(snapshot->entries[i].mac));
		if (status != 0)
			break;

		length = snprintf(name, sizeof(name), "dhcp_staticip_x%u", i);
		if (length < 0 || length >= (int)sizeof(name)) {
			status = EOVERFLOW;
			break;
		}
		status = rc_wan_nvram_copy_value(
			get_value, context, name, snapshot->entries[i].ip,
			sizeof(snapshot->entries[i].ip));
		if (status != 0)
			break;

		length = snprintf(name, sizeof(name), "dhcp_staticname_x%u", i);
		if (length < 0 || length >= (int)sizeof(name)) {
			status = EOVERFLOW;
			break;
		}
		status = rc_wan_nvram_copy_value(
			get_value, context, name, snapshot->entries[i].hostname,
			sizeof(snapshot->entries[i].hostname));
	}

	errno = 0;
	api_result = end_transaction(context);
	saved_errno = errno;
	end_status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status == 0)
		status = end_status;

	if (status != 0) {
		memset(snapshot, 0, sizeof(*snapshot));
		errno = status;
	}
	return status;
}

#endif /* PADAVAN_DNSMASQ_STATIC_HOST_NVRAM_POLICY_H */
