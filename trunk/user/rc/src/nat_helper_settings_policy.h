#ifndef PADAVAN_NAT_HELPER_SETTINGS_POLICY_H
#define PADAVAN_NAT_HELPER_SETTINGS_POLICY_H

#include <errno.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

struct rc_nat_helper_settings {
	int wan_nat_x;
	int ftp0_port;
	int ftp1_port;
	int pptp_enabled;
	int h323_enabled;
	int sip_enabled;
};

/*
 * Read the complete NAT-helper selector tuple under one NVRAM transaction.
 * On any read/close/validation error, leave the published settings untouched
 * so a failed snapshot cannot unload modules still required by the prior
 * configuration. AP mode intentionally reads only wan_nat_x.
 */
static inline int
rc_nat_helper_settings_read(int is_ap_mode,
			    rc_wan_nvram_transaction_fn begin_transaction,
			    rc_wan_nvram_get_fn get_value,
			    rc_wan_nvram_transaction_fn end_transaction,
			    void *context,
			    struct rc_nat_helper_settings *settings)
{
	static const char *const names[] = {
		"wan_nat_x", "nf_alg_ftp0", "nf_alg_ftp1",
		"nf_alg_pptp", "nf_alg_h323", "nf_alg_sip"
	};
	char values[6][32] = {{0}};
	struct rc_wan_nvram_snapshot_field fields[6];
	const char *integer_texts[3];
	int parsed[3] = {0, 0, 0};
	struct rc_nat_helper_settings result;
	unsigned int field_count, integer_count, i;
	int status;

	if (!settings || (is_ap_mode != 0 && is_ap_mode != 1) ||
	    !begin_transaction || !get_value || !end_transaction)
		return EINVAL;

	field_count = is_ap_mode ? 1U : 6U;
	integer_count = is_ap_mode ? 1U : 3U;
	for (i = 0; i < field_count; ++i) {
		fields[i].param_name = names[i];
		fields[i].destination = values[i];
		fields[i].destination_size = sizeof(values[i]);
	}

	status = rc_wan_nvram_copy_snapshot(fields, field_count,
					    begin_transaction, get_value,
					    end_transaction, context);
	if (status != 0)
		return status;

	for (i = 0; i < integer_count; ++i)
		integer_texts[i] = values[i];
	status = rc_wan_nvram_parse_int_snapshot(integer_texts,
					 integer_count, parsed);
	if (status != 0)
		return status;

	memset(&result, 0, sizeof(result));
	result.wan_nat_x = parsed[0] != 0;
	if (!is_ap_mode) {
		result.ftp0_port = parsed[1];
		result.ftp1_port = parsed[2];
		if (result.ftp0_port < 21 || result.ftp0_port > 65535)
			result.ftp0_port = 0;
		if (result.ftp1_port < 1024 || result.ftp1_port > 65535)
			result.ftp1_port = 0;
		result.pptp_enabled = (strcmp(values[3], "1") == 0);
		result.h323_enabled = (strcmp(values[4], "1") == 0);
		result.sip_enabled = (strcmp(values[5], "1") == 0);
	}

	*settings = result;
	return 0;
}

#endif /* PADAVAN_NAT_HELPER_SETTINGS_POLICY_H */
