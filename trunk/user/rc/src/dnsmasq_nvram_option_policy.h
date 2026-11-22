#ifndef PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H
#define PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H

#include <errno.h>
#include <string.h>

#include "wan_nvram_status_policy.h"

/*
 * nvram_get_checked() returns 1 for a present value, 0 for a missing key,
 * and -1 for a backend/read/close failure. Preserve nvram_match(..., "1")
 * semantics for present values while keeping missing-key defaults explicit.
 */
static inline int
dnsmasq_nvram_option_status(int api_result, int saved_errno,
			    const char *value, int missing_enabled,
			    int *enabled)
{
	if (!enabled)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*enabled = missing_enabled != 0;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

static inline int
dnsmasq_nvram_invmatch_status(int api_result, int saved_errno,
			      const char *value, const char *match_value,
			      int missing_matches, int *matches)
{
	if (!matches || !match_value ||
	    (missing_matches != 0 && missing_matches != 1))
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*matches = missing_matches;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*matches = strcmp(value, match_value) != 0;
	return 0;
}

static inline int
dnsmasq_nvram_integer_boolean_status(int api_result, int saved_errno,
				     const char *value, int missing_enabled,
				     int *enabled)
{
	int parsed, status;

	if (!enabled || (missing_enabled != 0 && missing_enabled != 1))
		return EINVAL;

	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					      value, missing_enabled, &parsed);
	if (status != 0)
		return status;

	*enabled = parsed != 0;
	return 0;
}

/*
 * Parse an integer option completely and enforce its documented live range.
 * Missing keys retain the caller's historical default, even if it is outside
 * the range for a present value; malformed present values never fall back.
 */
static inline int
dnsmasq_nvram_integer_range_status(int api_result, int saved_errno,
				   const char *text, int missing_value,
				   int minimum, int maximum, int *result)
{
	int parsed, status;

	if (!result || minimum > maximum)
		return EINVAL;
	if (api_result == 0) {
		if (text)
			return EIO;
		*result = missing_value;
		return 0;
	}
	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					     text, missing_value, &parsed);
	if (status != 0)
		return status;
	if (parsed < minimum || parsed > maximum)
		return ERANGE;
	*result = parsed;
	return 0;
}

static inline int
dnsmasq_nvram_string_status(int api_result, int saved_errno,
			    const char *value, const char *missing_value,
			    char *destination, size_t destination_size)
{
	size_t value_length;

	if (!destination || destination_size == 0)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		value = missing_value ? missing_value : "";
	} else if (api_result != 1 || !value) {
		return EIO;
	}

	value_length = strlen(value);
	if (value_length >= destination_size)
		return ENAMETOOLONG;

	memcpy(destination, value, value_length + 1);
	return 0;
}


#define DNSMASQ_DHCP_LAN_VALUE_SIZE 32
#define DNSMASQ_DHCP_DOMAIN_VALUE_SIZE 256
#define DNSMASQ_DHCP_WINS_VALUE_SIZE 128

struct dnsmasq_dhcp_config_nvram_snapshot {
	char lan_ipaddr[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char lan_netmask[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char lan_domain[DNSMASQ_DHCP_DOMAIN_VALUE_SIZE];
	char dhcp_start[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_end[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_gateway[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_dns1[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_dns2[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_dns3[DNSMASQ_DHCP_LAN_VALUE_SIZE];
	char dhcp_wins[DNSMASQ_DHCP_WINS_VALUE_SIZE];
};

/* Copy a coherent bounded LAN/DHCP tuple while one NVRAM transaction is held. */
static inline int
dnsmasq_dhcp_config_nvram_snapshot_read(
	struct dnsmasq_dhcp_config_nvram_snapshot *snapshot,
	int dhcp_enabled,
	rc_wan_nvram_transaction_fn begin_transaction,
	rc_wan_nvram_get_fn get_value,
	rc_wan_nvram_transaction_fn end_transaction,
	void *context)
{
	struct rc_wan_nvram_snapshot_field fields[10];
	unsigned int count = 0;

	if (!snapshot)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));
	if ((dhcp_enabled != 0 && dhcp_enabled != 1) ||
	    !begin_transaction || !get_value || !end_transaction)
		return EINVAL;

	fields[count].param_name = "lan_ipaddr";
	fields[count].destination = snapshot->lan_ipaddr;
	fields[count++].destination_size = sizeof(snapshot->lan_ipaddr);
	fields[count].param_name = "lan_netmask";
	fields[count].destination = snapshot->lan_netmask;
	fields[count++].destination_size = sizeof(snapshot->lan_netmask);
	fields[count].param_name = "lan_domain";
	fields[count].destination = snapshot->lan_domain;
	fields[count++].destination_size = sizeof(snapshot->lan_domain);

	if (dhcp_enabled) {
		fields[count].param_name = "dhcp_start";
		fields[count].destination = snapshot->dhcp_start;
		fields[count++].destination_size = sizeof(snapshot->dhcp_start);
		fields[count].param_name = "dhcp_end";
		fields[count].destination = snapshot->dhcp_end;
		fields[count++].destination_size = sizeof(snapshot->dhcp_end);
		fields[count].param_name = "dhcp_gateway_x";
		fields[count].destination = snapshot->dhcp_gateway;
		fields[count++].destination_size = sizeof(snapshot->dhcp_gateway);
		fields[count].param_name = "dhcp_dns1_x";
		fields[count].destination = snapshot->dhcp_dns1;
		fields[count++].destination_size = sizeof(snapshot->dhcp_dns1);
		fields[count].param_name = "dhcp_dns2_x";
		fields[count].destination = snapshot->dhcp_dns2;
		fields[count++].destination_size = sizeof(snapshot->dhcp_dns2);
		fields[count].param_name = "dhcp_dns3_x";
		fields[count].destination = snapshot->dhcp_dns3;
		fields[count++].destination_size = sizeof(snapshot->dhcp_dns3);
		fields[count].param_name = "dhcp_wins_x";
		fields[count].destination = snapshot->dhcp_wins;
		fields[count++].destination_size = sizeof(snapshot->dhcp_wins);
	}

	return rc_wan_nvram_copy_snapshot(fields, count, begin_transaction,
					  get_value, end_transaction, context);
}

#endif /* PADAVAN_DNSMASQ_NVRAM_OPTION_POLICY_H */
