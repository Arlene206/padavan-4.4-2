#ifndef PADAVAN_NAT_OFFLOAD_SETTINGS_POLICY_H
#define PADAVAN_NAT_OFFLOAD_SETTINGS_POLICY_H

#include <errno.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

struct rc_nat_offload_settings {
	int hwnat_allowed;
	int hw_nat_mode;
	int ipv6_nat;
	int sfe_enabled;
};

/*
 * Read the acceleration selectors as one coherent snapshot. A failed read
 * must never be interpreted as an instruction to unload an active offload
 * module. Missing keys retain the legacy zero defaults.
 */
static inline int
rc_nat_offload_settings_read(int read_hwnat, int read_sfe,
			     rc_wan_nvram_transaction_fn begin_transaction,
			     rc_wan_nvram_get_fn get_value,
			     rc_wan_nvram_transaction_fn end_transaction,
			     void *context,
			     struct rc_nat_offload_settings *settings)
{
	const char *names[4];
	char values[4][32] = {{0}};
	struct rc_wan_nvram_snapshot_field fields[4];
	const char *integer_texts[4];
	int parsed[4] = {0, 0, 0, 0};
	struct rc_nat_offload_settings result;
	int sw_mode_index = -1, hw_nat_mode_index = -1;
	int ipv6_nat_index = -1, sfe_enable_index = -1;
	unsigned int count = 0, i;
	int status;

	if (!settings || (read_hwnat != 0 && read_hwnat != 1) ||
	    (read_sfe != 0 && read_sfe != 1))
		return EINVAL;

	if (read_hwnat) {
		sw_mode_index = (int)count;
		names[count++] = "sw_mode";
		hw_nat_mode_index = (int)count;
		names[count++] = "hw_nat_mode";
		ipv6_nat_index = (int)count;
		names[count++] = "ip6_lan_auto";
	}
	if (read_sfe) {
		sfe_enable_index = (int)count;
		names[count++] = "sfe_enable";
	}

	if (count == 0) {
		memset(&result, 0, sizeof(result));
		*settings = result;
		return 0;
	}
	if (!begin_transaction || !get_value || !end_transaction)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		fields[i].param_name = names[i];
		fields[i].destination = values[i];
		fields[i].destination_size = sizeof(values[i]);
		integer_texts[i] = values[i];
	}

	status = rc_wan_nvram_copy_snapshot(fields, count, begin_transaction,
					    get_value, end_transaction, context);
	if (status != 0)
		return status;

	status = rc_wan_nvram_parse_int_snapshot(integer_texts, count, parsed);
	if (status != 0)
		return status;

	memset(&result, 0, sizeof(result));
	if (read_hwnat) {
		result.hw_nat_mode = parsed[hw_nat_mode_index];
		result.hwnat_allowed =
			(parsed[sw_mode_index] == 1 || parsed[sw_mode_index] == 4) &&
			(result.hw_nat_mode != 0);
		result.ipv6_nat = parsed[ipv6_nat_index];
	}
	if (read_sfe)
		result.sfe_enabled = parsed[sfe_enable_index];

	*settings = result;
	return 0;
}

#endif /* PADAVAN_NAT_OFFLOAD_SETTINGS_POLICY_H */
