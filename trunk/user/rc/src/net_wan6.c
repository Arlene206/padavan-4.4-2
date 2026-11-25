/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/ethernet.h>
#include <net/if.h>

#include "rc.h"
#include "route_status_policy.h"
#include "wan6_teardown_policy.h"
#include "wan_ipv6_service_policy.h"
#include "wan6_identity_policy.h"
#include "wan_nvram_status_policy.h"
#include "dhcp_lease_policy.h"
#include <dhcp6c_duid_file_policy.h>
#include "dhcp6c_config_file.h"
#include "wan6_dns_policy.h"
#include "wan6_sixrd_prefix_policy.h"
#include "wan6_sixrd_option_policy.h"
#include "dhcp6c_callback_generation_policy.h"
#include "service_wait_policy.h"

#if defined (USE_IPV6)

#define DHCP6C_CALLBACK_STATE_FILE "/tmp/dhcp6c-callback.state"
#define DHCP6C_CALLBACK_GENERATION_ENV "DHCP6C_CALLBACK_GENERATION"
#define DHCP6C_CALLBACK_IFNAME_ENV "DHCP6C_CALLBACK_IFNAME"

#define WAN6_DHCP_INTEGER_SNAPSHOT_MAX 3
#define WAN6_DHCP_INTEGER_VALUE_SIZE 32

static int
wan6_snapshot_transaction_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
wan6_snapshot_transaction_get(void *context, const char *param_name,
			      char **value)
{
	if (!context || !param_name || !value) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_get((struct nvram_transaction *)context,
				     param_name, value);
}

static int
wan6_snapshot_transaction_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

/*
 * DHCP 6rd publication updates size and relay together. SIT activation must
 * read the same pair under one lock, otherwise it can combine generations.
 */
static int
wan6_read_sit_6rd_parameters(int unit, int *size4, char *relay4,
			     size_t relay4_size)
{
	char size_name[64], relay_name[64];
	struct nvram_transaction transaction;
	int length;

	if (unit < 0 || !size4 || !relay4 || relay4_size == 0)
		return EINVAL;
	length = snprintf(size_name, sizeof(size_name), "wan%d_6rd_size", unit);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(size_name))
		return ENAMETOOLONG;
	length = snprintf(relay_name, sizeof(relay_name), "wan%d_6rd_relay", unit);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(relay_name))
		return ENAMETOOLONG;

	return rc_wan_nvram_copy_int_text_snapshot(
		size_name, relay_name, size4, relay4, relay4_size,
		wan6_snapshot_transaction_begin, wan6_snapshot_transaction_get,
		wan6_snapshot_transaction_end, &transaction);
}

/*
 * DHCPv6 startup and lease callbacks must not interpret an NVRAM ioctl/read/
 * close error as a disabled flag. Copy the complete set under one lock,
 * preserve the legacy zero value for absent keys, then parse all members
 * before making any configuration or lease-state decision.
 */
static int
wan6_read_dhcp_integer_snapshot(const char *const *param_names,
				unsigned int count, int *values)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[WAN6_DHCP_INTEGER_SNAPSHOT_MAX];
	char text_values[WAN6_DHCP_INTEGER_SNAPSHOT_MAX][WAN6_DHCP_INTEGER_VALUE_SIZE];
	const char *text_pointers[WAN6_DHCP_INTEGER_SNAPSHOT_MAX];
	unsigned int i;
	int status;

	if (!values || !param_names || count == 0 ||
	    count > WAN6_DHCP_INTEGER_SNAPSHOT_MAX)
		return EINVAL;
	for (i = 0; i < count; ++i)
		values[i] = 0;
	for (i = 0; i < count; ++i) {
		if (!param_names[i] || !param_names[i][0])
			return EINVAL;
		fields[i].param_name = param_names[i];
		fields[i].destination = text_values[i];
		fields[i].destination_size = sizeof(text_values[i]);
		text_pointers[i] = text_values[i];
	}

	status = rc_wan_nvram_copy_snapshot(
		fields, count, wan6_snapshot_transaction_begin,
		wan6_snapshot_transaction_get, wan6_snapshot_transaction_end,
		&transaction);
	if (status != 0)
		return status;

	return rc_wan_nvram_parse_int_snapshot(text_pointers, count, values);
}

static int
dhcp6c_callback_session_advance(int fd, unsigned long *generation)
{
	unsigned long next;

	if (!generation ||
	    dhcp6c_callback_generation_next(*generation, &next) != 0)
		return -1;
	if (dhcp6c_callback_session_write(fd, next) != 0)
		return -1;
	*generation = next;
	return 0;
}

/*
 * Copy the unit address/gateway fields used by IPv6 activation as one
 * transaction. Do this before stopping the old client or changing routes,
 * so a failed read cannot activate from a mixed or incomplete NVRAM tuple.
 */
static int
wan6_read_unit_address_snapshot(int unit, int ipv6_type,
				char *wan_addr4, size_t wan_addr4_size,
				char *wan_gate4, size_t wan_gate4_size,
				char *wan_addr6, size_t wan_addr6_size,
				char *wan_gate6, size_t wan_gate6_size)
{
	static const char *const sit_sixin_sixrd_names[3] = {
		"ipaddr", "gateway", "addr6"
	};
	static const char *const sit_six_to_four_names[2] = {
		"ipaddr", "gateway"
	};
	static const char *const native_static_names[2] = {
		"addr6", "gate6"
	};
	const char *const *param_suffixes;
	char param_names[3][64];
	char *destinations[3];
	size_t destination_sizes[3];
	struct rc_wan_nvram_snapshot_field fields[3];
	struct nvram_transaction transaction;
	unsigned int count, i;
	int length;

	if (unit < 0)
		return EINVAL;
	if (wan_addr4 && wan_addr4_size)
		wan_addr4[0] = '\0';
	if (wan_gate4 && wan_gate4_size)
		wan_gate4[0] = '\0';
	if (wan_addr6 && wan_addr6_size)
		wan_addr6[0] = '\0';
	if (wan_gate6 && wan_gate6_size)
		wan_gate6[0] = '\0';

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6RD) {
		if (!wan_addr4 || !wan_addr4_size || !wan_gate4 || !wan_gate4_size ||
		    !wan_addr6 || !wan_addr6_size)
			return EINVAL;
		param_suffixes = sit_sixin_sixrd_names;
		destinations[0] = wan_addr4;
		destinations[1] = wan_gate4;
		destinations[2] = wan_addr6;
		destination_sizes[0] = wan_addr4_size;
		destination_sizes[1] = wan_gate4_size;
		destination_sizes[2] = wan_addr6_size;
		count = 3;
	} else if (ipv6_type == IPV6_6TO4) {
		if (!wan_addr4 || !wan_addr4_size || !wan_gate4 || !wan_gate4_size)
			return EINVAL;
		param_suffixes = sit_six_to_four_names;
		destinations[0] = wan_addr4;
		destinations[1] = wan_gate4;
		destination_sizes[0] = wan_addr4_size;
		destination_sizes[1] = wan_gate4_size;
		count = 2;
	} else if (ipv6_type == IPV6_NATIVE_STATIC) {
		if (!wan_addr6 || !wan_addr6_size || !wan_gate6 || !wan_gate6_size)
			return EINVAL;
		param_suffixes = native_static_names;
		destinations[0] = wan_addr6;
		destinations[1] = wan_gate6;
		destination_sizes[0] = wan_addr6_size;
		destination_sizes[1] = wan_gate6_size;
		count = 2;
	} else {
		return 0;
	}

	for (i = 0; i < count; ++i) {
		length = snprintf(param_names[i], sizeof(param_names[i]),
				  "wan%d_%s", unit, param_suffixes[i]);
		if (length < 0)
			return errno ? errno : EIO;
		if ((size_t)length >= sizeof(param_names[i]))
			return ENAMETOOLONG;
		fields[i].param_name = param_names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = destination_sizes[i];
	}

	return rc_wan_nvram_copy_snapshot(
		fields, count, wan6_snapshot_transaction_begin,
		wan6_snapshot_transaction_get, wan6_snapshot_transaction_end,
		&transaction);
}

static int
format_ipv4_literal(const char *input, char *output, size_t output_size)
{
	struct in_addr addr;

	if (!input || !*input || !output || output_size < INET_ADDRSTRLEN ||
	    inet_pton(AF_INET, input, &addr) != 1 || addr.s_addr == INADDR_ANY)
		return -1;

	return inet_ntop(AF_INET, &addr, output, output_size) ? 0 : -1;
}

static int
format_ipv6_literal(const char *input, char *output, size_t output_size,
		    int allow_prefix)
{
	struct in6_addr addr;
	char addr_text[INET6_ADDRSTRLEN];
	const char *slash;
	int prefix_len, written;

	if (!input || !*input || !output || output_size == 0)
		return -1;

	slash = strchr(input, '/');
	if (slash && !allow_prefix)
		return -1;

	prefix_len = ipv6_from_string(input, &addr);
	if (prefix_len < 0 ||
	    !inet_ntop(AF_INET6, &addr, addr_text, sizeof(addr_text)))
		return -1;

	if (slash)
		written = snprintf(output, output_size, "%s/%d", addr_text, prefix_len);
	else
		written = snprintf(output, output_size, "%s", addr_text);

	return (written < 0 || (size_t)written >= output_size) ? -1 : 0;
}

static int
build_dns6_value(char *dns6s, size_t dns6_size)
{
	static const char *const dns_names[WAN6_DNS_CONFIG_VALUE_COUNT] = {
		"ip6_dns1", "ip6_dns2", "ip6_dns3"
	};
	char dnsnv[WAN6_DNS_CONFIG_VALUE_COUNT][INET6_ADDRSTRLEN] = {{0}};
	char *dns_destinations[WAN6_DNS_CONFIG_VALUE_COUNT];
	size_t dns_sizes[WAN6_DNS_CONFIG_VALUE_COUNT];
	struct nvram_transaction transaction;
	int status, written, dns_static;
	unsigned int i;

	if (!dns6s || dns6_size == 0)
		return EINVAL;
	dns6s[0] = '\0';
	status = is_wan_dns6_static_status(&dns_static);
	if (status != 0)
		return status;
	if (dns_static != 1)
		return 0;

	for (i = 0; i < WAN6_DNS_CONFIG_VALUE_COUNT; ++i) {
		dns_destinations[i] = dnsnv[i];
		dns_sizes[i] = sizeof(dnsnv[i]);
	}
	status = wan6_dns_copy_config_snapshot(
		dns_names, dns_destinations, dns_sizes,
		wan6_snapshot_transaction_begin, wan6_snapshot_transaction_get,
		wan6_snapshot_transaction_end, &transaction);
	if (status != 0)
		return status;

	for (i = 0; i < WAN6_DNS_CONFIG_VALUE_COUNT; ++i) {
		status = wan6_dns_validate_static_server(dnsnv[i]);
		if (status != 0)
			return status;
	}

	written = snprintf(dns6s, dns6_size, "%s %s %s",
			   dnsnv[0], dnsnv[1], dnsnv[2]);
	if (written < 0 || (size_t)written >= dns6_size) {
		dns6s[0] = '\0';
		return ENAMETOOLONG;
	}
	trim_r(dns6s);
	return 0;
}

static int
publish_dns6_var_status(void)
{
	char dns6s[INET6_ADDRSTRLEN * 3 + 4];
	int status;

	status = build_dns6_value(dns6s, sizeof(dns6s));
	if (status != 0)
		return status;
	return set_wan_unit_value_status(0, "dns6", dns6s);
}

static int
wan6_set_runtime_value_status(void *context, const char *param_name,
			      const char *value)
{
	(void)context;
	return set_wan_unit_value_status(0, param_name, value);
}

static int
wan6_sit_route4_delete(void *context, const char *ifname,
		       const char *endpoint, const char *gateway)
{
	(void)context;
	return route_del((char *)ifname, 0, (char *)endpoint,
			 (char *)wan6_sit_route4_gateway_argument(gateway),
			 "255.255.255.255");
}

static int
wan6_sit_route4_clear_record(void *context, int unit)
{
	(void)context;
	return set_wan_unit_value_status(unit, "sit_route4", "");
}

static int
wan6_sit_route4_cleanup(int unit)
{
	char record[WAN6_SIT_ROUTE4_RECORD_SIZE] = {0};
	int status;

	status = get_wan_unit_value_copy_status(
		unit, "sit_route4", record, sizeof(record));
	if (status != 0)
		return status;
	return wan6_sit_route4_cleanup_record(
		unit, record, wan6_sit_route4_delete,
		wan6_sit_route4_clear_record, NULL);
}

static int
wan6_sit_route4_publish(int unit, const char *ifname,
			const char *endpoint, const char *gateway)
{
	char record[WAN6_SIT_ROUTE4_RECORD_SIZE];
	int status;

	status = wan6_sit_route4_record_format(
		ifname, endpoint, gateway, record, sizeof(record));
	if (status != 0)
		return status;
	return set_wan_unit_value_status(unit, "sit_route4", record);
}

int reset_wan6_vars_status(void)
{
	struct in6_addr addr6;
	char addr6s[INET6_ADDRSTRLEN + 5] = {0};
	char wan_addr6[INET6_ADDRSTRLEN] = {0};
	char wan_gate6[INET6_ADDRSTRLEN] = {0};
	char relay6[INET6_ADDRSTRLEN] = {0};
	char size6[INET6_ADDRSTRLEN] = {0};
	char wan_size_text[16] = {0};
	char dns6s[INET6_ADDRSTRLEN * 3 + 4] = {0};
	static const char *const reset_names[5] = {
		"ip6_wan_addr", "ip6_wan_size", "ip6_wan_gate",
		"ip6_6rd_relay", "ip6_6rd_size"
	};
	char *reset_destinations[5] = {
		wan_addr6, wan_size_text, wan_gate6, relay6, size6
	};
	size_t reset_sizes[5] = {
		sizeof(wan_addr6), sizeof(wan_size_text), sizeof(wan_gate6),
		sizeof(relay6), sizeof(size6)
	};
	struct rc_wan_nvram_snapshot_field reset_fields[5];
	struct nvram_transaction transaction;
	const char *wan_size_texts[1];
	int parsed_wan_size[1];
	struct rc_wan_nvram_value reset_values[7];
	unsigned int first_field, field_count, i;
	int wan_size6 = 0;
	int status, wan_addr6_static;

	status = wan6_sit_route4_cleanup(0);
	if (status != 0)
		return status;
	status = is_wan_addr6_static_status(&wan_addr6_static);
	if (status != 0)
		return status;
	/*
	 * Read every setting consumed by the active mode in one transaction.
	 * Static WAN uses address/prefix/gateway and the 6rd pair; dynamic
	 * modes only consume the 6rd pair.
	 */
	first_field = wan_addr6_static == 1 ? 0U : 3U;
	field_count = wan_addr6_static == 1 ? 5U : 2U;
	for (i = 0; i < field_count; ++i) {
		unsigned int index = first_field + i;

		reset_fields[i].param_name = reset_names[index];
		reset_fields[i].destination = reset_destinations[index];
		reset_fields[i].destination_size = reset_sizes[index];
	}
	status = rc_wan_nvram_copy_snapshot(
		reset_fields, field_count, wan6_snapshot_transaction_begin,
		wan6_snapshot_transaction_get, wan6_snapshot_transaction_end,
		&transaction);
	if (status != 0)
		return status;

	if (wan_addr6_static == 1) {
		wan_size_texts[0] = wan_size_text;
		status = rc_wan_nvram_parse_int_snapshot(
			wan_size_texts, 1, parsed_wan_size);
		if (status != 0)
			return status;
		wan_size6 = parsed_wan_size[0];
		if (*wan_addr6 && wan_size6 >= 0 && wan_size6 <= 128) {
			memset(&addr6, 0, sizeof(addr6));
			if (ipv6_from_string(wan_addr6, &addr6) >= 0 &&
			    inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN) &&
			    wan_size6 > 0 && wan_size6 < 128) {
				snprintf(addr6s + strlen(addr6s),
					 sizeof(addr6s) - strlen(addr6s), "/%d", wan_size6);
			}
		}
	}
	status = build_dns6_value(dns6s, sizeof(dns6s));
	if (status != 0)
		return status;

	/*
	 * Clear the old interface identity last. If any required configuration
	 * field fails to publish, the prior ifname6 remains available to teardown.
	 */
	reset_values[0].param_name = "addr6";
	reset_values[0].value = addr6s;
	reset_values[1].param_name = "gate6";
	reset_values[1].value = wan_gate6;
	reset_values[2].param_name = "6rd_relay";
	reset_values[2].value = relay6;
	reset_values[3].param_name = "6rd_size";
	reset_values[3].value = size6;
	reset_values[4].param_name = "dns6";
	reset_values[4].value = dns6s;
	reset_values[5].param_name = "ifname6";
	reset_values[5].value = "";
	reset_values[6].param_name = "sit_route4";
	reset_values[6].value = "";

	return rc_wan_nvram_set_required_values(
		reset_values, 7, wan6_set_runtime_value_status, NULL);
}

int
reset_wan6_vars(void)
{
	int status = reset_wan6_vars_status();

	if (status != 0)
		logmessage("WAN IPv6",
			   "failed to reset IPv6 runtime configuration (status=%d)",
			   status);
	return status;
}

static int
wan6_read_nvram_match_status(const char *param_name, const char *expected,
			     int *matches)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !expected || !matches)
		return EINVAL;
	*matches = 0;

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0)
		value = "";

	return rc_wan_nvram_match_status(0, value, expected, matches);
}

int
is_wan_addr6_static_status(int *is_static)
{
	int ipv6_type, status, sixrd_dhcp_status = 0, sixrd_is_manual = 0;

	if (!is_static)
		return EINVAL;
	*is_static = -1;
	status = get_ipv6_type_status(&ipv6_type);
	if (status == 0 && ipv6_type == IPV6_6RD)
		sixrd_dhcp_status = wan6_read_nvram_match_status(
			"ip6_6rd_dhcp", "0", &sixrd_is_manual);

	return wan_ipv6_service_addr_static_status(
		status, ipv6_type, IPV6_DISABLED, IPV6_NATIVE_STATIC,
		IPV6_NATIVE_DHCP6, IPV6_6IN4, IPV6_6TO4, IPV6_6RD,
		sixrd_dhcp_status, sixrd_is_manual, is_static);
}

int
is_wan_dns6_static_status(int *is_static)
{
	int ipv6_type, status, dns_auto_status = 0, dns_auto_is_manual = 0;

	if (!is_static)
		return EINVAL;
	*is_static = -1;
	status = get_ipv6_type_status(&ipv6_type);
	if (status == 0 && ipv6_type == IPV6_NATIVE_DHCP6)
		dns_auto_status = wan6_read_nvram_match_status(
			"ip6_dns_auto", "0", &dns_auto_is_manual);

	return wan_ipv6_service_dns_static_status(
		status, ipv6_type, IPV6_DISABLED, IPV6_NATIVE_STATIC,
		IPV6_NATIVE_DHCP6, IPV6_6IN4, IPV6_6TO4, IPV6_6RD,
		dns_auto_status, dns_auto_is_manual, is_static);
}

int
is_wan_ipv6_type_sit_status(int *is_sit)
{
	int ipv6_type, status;

	if (!is_sit)
		return EINVAL;
	*is_sit = -1;
	status = get_ipv6_type_status(&ipv6_type);
	return wan_ipv6_service_is_sit_status(
		status, ipv6_type, IPV6_DISABLED, IPV6_6IN4,
		IPV6_6TO4, IPV6_6RD, is_sit);
}

/* Compatibility wrappers: state-changing callers use the status APIs above. */
int
is_wan_addr6_static(void)
{
	int is_static = -1;
	int status = is_wan_addr6_static_status(&is_static);

	if (status != 0) {
		errno = status;
		return -1;
	}
	return is_static;
}

int
is_wan_dns6_static(void)
{
	int is_static = -1;
	int status = is_wan_dns6_static_status(&is_static);

	if (status != 0) {
		errno = status;
		return -1;
	}
	return is_static;
}

int
is_wan_ipv6_type_sit(void)
{
	int is_sit = -1;
	int status = is_wan_ipv6_type_sit_status(&is_sit);

	if (status != 0) {
		errno = status;
		return -1;
	}
	return is_sit;
}

int
is_wan_ipv6_if_ppp_status(int *is_ppp)
{
	char *value = NULL;
	int api_result, saved_errno;

	if (!is_ppp)
		return EINVAL;
	*is_ppp = -1;
	errno = 0;
	api_result = nvram_get_checked("ip6_wan_if", &value);
	saved_errno = errno;
	return rc_wan_nvram_ipv6_if_ppp_status(
		api_result, saved_errno, value, is_ppp);
}

/* Compatibility wrapper: unknown interface identity is treated as PPP. */
int is_wan_ipv6_if_ppp(void)
{
	int is_ppp = 1;
	int status = is_wan_ipv6_if_ppp_status(&is_ppp);

	if (status != 0) {
		errno = status;
		return 1;
	}
	return is_ppp;
}

#define WAN6_6RD_SNAPSHOT_COUNT 3
#define WAN6_6RD_SNAPSHOT_VALUE_MAX 128

static int
wan6_dhcp_nvram_transaction_set(void *context, const char *name,
				const char *value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)context;
	int result, saved_errno;

	if (!transaction || !name || !value)
		return EINVAL;
	errno = 0;
	result = nvram_transaction_set(transaction, name, (char *)value, 1);
	saved_errno = errno;
	return rc_wan_nvram_write_status(result, saved_errno);
}

static int
wan6_restore_6rd_values(const struct dhcp_lease_nvram_value *values,
			int *rollback_status)
{
	struct nvram_transaction transaction;
	int status, end_status, restore_status = 0;

	if (rollback_status)
		*rollback_status = 0;
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0) {
		status = errno ? errno : EIO;
		if (rollback_status)
			*rollback_status = status;
		return status;
	}

	status = dhcp_lease_nvram_restore(
		values, WAN6_6RD_SNAPSHOT_COUNT,
		wan6_dhcp_nvram_transaction_set, &transaction, &restore_status);
	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		if (restore_status == 0)
			restore_status = end_status;
	}
	if (rollback_status)
		*rollback_status = restore_status;
	return status;
}

/*
 * Snapshot and publish all DHCP-provided 6rd fields under one NVRAM lock.
 * If any setter or the transaction close fails, restore the complete old
 * tuple under a fresh lock before returning the original publication error.
 */
static int
wan6_publish_6rd_snapshot(const char *prefix, const char *addr6,
			  const char *size4, const char *relay4,
			  int *rollback_status)
{
	static const char *const suffixes[WAN6_6RD_SNAPSHOT_COUNT] = {
		"addr6", "6rd_size", "6rd_relay"
	};
	const char *new_values[WAN6_6RD_SNAPSHOT_COUNT];
	char names[WAN6_6RD_SNAPSHOT_COUNT][64];
	char previous_values[WAN6_6RD_SNAPSHOT_COUNT]
		[WAN6_6RD_SNAPSHOT_VALUE_MAX];
	struct dhcp_lease_nvram_value values[WAN6_6RD_SNAPSHOT_COUNT];
	struct nvram_transaction transaction;
	char *previous_value;
	size_t i;
	int length, read_result, saved_errno;
	int status = 0, end_status, publish_attempted = 0;
	int transaction_rollback_status = 0;
	int recovery_status, recovery_detail = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!prefix || !prefix[0] || !addr6 || !size4 || !relay4)
		return EINVAL;

	new_values[0] = addr6;
	new_values[1] = size4;
	new_values[2] = relay4;
	for (i = 0; i < WAN6_6RD_SNAPSHOT_COUNT; ++i) {
		length = snprintf(names[i], sizeof(names[i]), "%s%s",
				  prefix, suffixes[i]);
		if (length < 0)
			return errno ? errno : EIO;
		if ((size_t)length >= sizeof(names[i]))
			return ENAMETOOLONG;
		values[i].name = names[i];
		values[i].value = new_values[i];
		values[i].previous_value = previous_values[i];
	}

	/* Avoid restoring values read from a different concurrent DHCP writer. */
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;

	for (i = 0; i < WAN6_6RD_SNAPSHOT_COUNT; ++i) {
		previous_value = NULL;
		errno = 0;
		read_result = nvram_transaction_get(
			&transaction, names[i], &previous_value);
		saved_errno = errno;
		status = rc_wan_nvram_read_status(read_result, saved_errno);
		if (status != 0)
			break;
		if (read_result == 0 || !previous_value)
			previous_value = "";
		status = rc_wan_nvram_copy_text(
			previous_value, previous_values[i],
			sizeof(previous_values[i]));
		if (status != 0)
			break;
	}

	if (status == 0) {
		publish_attempted = 1;
		status = dhcp_lease_nvram_publish(
			values, WAN6_6RD_SNAPSHOT_COUNT,
			wan6_dhcp_nvram_transaction_set, &transaction,
			&transaction_rollback_status);
	}

	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		else if (transaction_rollback_status == 0)
			transaction_rollback_status = end_status;
	}

	if (publish_attempted && status != 0) {
		recovery_status = wan6_restore_6rd_values(values, &recovery_detail);
		transaction_rollback_status = recovery_status != 0 ?
			(recovery_detail != 0 ? recovery_detail : recovery_status) : 0;
	}
	if (rollback_status)
		*rollback_status = transaction_rollback_status;
	return status;
}

int
store_ip6rd_from_dhcp_status(const char *env_value, const char *prefix)
{
	int i, written;
	long ipv4_mask_len, ipv6_prefix_len;
	char ip6rd[WAN6_SIXRD_OPTION_VALUE_MAX];
	char addr6[INET6_ADDRSTRLEN + 5];
	char first_relay[WAN6_SIXRD_IPV4_TEXT_SIZE];
	char *value, *values[3], *end;
	struct in6_addr prefix6;
	int sixrd_dhcp_disabled = 0;
	int status, rollback_status = 0;

	/* Malformed or absent optional options remain a no-op. */
	if (!env_value || !prefix || !*prefix)
		return 0;
	if (strlen(env_value) >= sizeof(ip6rd))
		return 0;

	snprintf(ip6rd, sizeof(ip6rd), "%s", env_value);
	value = ip6rd;
	for (i = 0; i < 3; i++) {
		values[i] = strtok_r(i == 0 ? value : NULL, " \t\r\n", &value);
		if (!values[i])
			return 0;
	}
	/*
	 * RFC 5969 permits multiple border relays. This platform stores one
	 * relay in NVRAM, so validate the full DHCP list and use its first entry.
	 */
	if (!wan6_sixrd_relay_list_first_valid(
		    value, first_relay, sizeof(first_relay), is_valid_ipv4))
		return 0;

	for (end = values[0]; *end; end++)
		if (*end < '0' || *end > '9')
			return 0;
	ipv4_mask_len = strtol(values[0], &end, 10);
	if (*end || ipv4_mask_len < 0 || ipv4_mask_len > 32)
		return 0;

	for (end = values[1]; *end; end++)
		if (*end < '0' || *end > '9')
			return 0;
	ipv6_prefix_len = strtol(values[1], &end, 10);
	if (*end || ipv6_prefix_len < 0 || ipv6_prefix_len > 128)
		return 0;

	/*
	 * Reject an impossible RFC 5969 mapping before publishing the DHCP
	 * option tuple. Otherwise an invalid prefix poisons NVRAM and makes a
	 * later WAN restart fail even though the previous 6rd tuple was usable.
	 */
	if (!wan6_sixrd_prefix_mapping_is_valid((int)ipv4_mask_len,
					       (int)ipv6_prefix_len))
		return 0;

	if (inet_pton(AF_INET6, values[2], &prefix6) != 1)
		return 0;
	if (!wan6_sixrd_prefix_clear_host_bits(prefix6.s6_addr,
					       (int)ipv6_prefix_len))
		return 0;

	status = wan6_read_nvram_match_status(
		"ip6_6rd_dhcp", "0", &sixrd_dhcp_disabled);
	if (status != 0) {
		errno = status;
		logmessage("WAN IPv6",
			   "cannot read DHCPv6 6rd selector; refusing automatic update (status=%d)",
			   status);
		return status;
	}
	if (sixrd_dhcp_disabled)
		return 0;

	if (!inet_ntop(AF_INET6, &prefix6, addr6, sizeof(addr6)))
		return errno ? errno : EINVAL;
	{
		size_t used = strlen(addr6);
		size_t available = sizeof(addr6) - used;

		written = snprintf(addr6 + used, available, "/%ld",
				   ipv6_prefix_len);
		if (written < 0 || (size_t)written >= available)
			return ENAMETOOLONG;
	}

	status = wan6_publish_6rd_snapshot(
		prefix, addr6, values[0], first_relay, &rollback_status);
	if (status != 0)
		logmessage("WAN IPv6",
			   "failed to publish DHCP 6rd configuration (status=%d, rollback=%d)",
			   status, rollback_status);
	return status;
}

void
store_ip6rd_from_dhcp(const char *env_value, const char *prefix)
{
	(void)store_ip6rd_from_dhcp_status(env_value, prefix);
}

static int
wan6_dns_set_status(void *context, const char *dns_value)
{
	(void)context;
	return set_wan_unit_value_status(0, "dns6", dns_value);
}

int store_wan_dns6(char *dns6_new)
{
	char dns6s[INET6_ADDRSTRLEN*3+8] = {0};
	char dns6_old[INET6_ADDRSTRLEN*3+8] = {0};
	size_t dns6_len = dns6_new ? strlen(dns6_new) : 0;
	int read_status, status;

	/* A missing option means the server withdrew its automatic DNS list. */
	if (dns6_len >= sizeof(dns6s)) {
		errno = EOVERFLOW;
		return -1;
	}
	if (dns6_len)
		memcpy(dns6s, dns6_new, dns6_len);
	trim_r(dns6s);

	read_status = get_wan_unit_value_copy_status(
		0, "dns6", dns6_old, sizeof(dns6_old));
	status = wan6_dns_store_after_checked_read(
		read_status, dns6_old, dns6s, wan6_dns_set_status, NULL);
	if (status < 0) {
		errno = -status;
		logmessage("WAN IPv6",
			   "failed to publish DHCPv6 DNS state (status=%d)",
			   status);
		return -1;
	}

	return status;
}


static int
wan6_lookup_ifaddr6_status(void *context, const char *ifname,
			   char *address, int *found)
{
	char *result;

	(void)context;
	if (!ifname || !address || !found)
		return EINVAL;

	*found = 0;
	errno = 0;
	result = get_ifaddr6(ifname, 0, address);
	if (!result)
		return errno ? errno : 0;

	*found = 1;
	return 0;
}

int
get_wan_addr6_host_status(char *p_addr6s, const char **p_host)
{
	char wan6_ifname[IFNAMSIZ] = {0};
	int status;

	if (!p_addr6s || !p_host)
		return EINVAL;
	*p_host = NULL;
	status = get_wan_unit_value_copy_status(
		0, "ifname6", wan6_ifname, sizeof(wan6_ifname));
	return wan6_identity_resolve_addr6_host(
		status, wan6_ifname, p_addr6s, p_host,
		wan6_lookup_ifaddr6_status, NULL);
}

char *get_wan_addr6_host(char *p_addr6s)
{
	const char *host = NULL;
	int status;

	status = get_wan_addr6_host_status(p_addr6s, &host);
	if (status != 0) {
		errno = status;
		return NULL;
	}
	return (char *)host;
}

static int
wan6_checked_nvram_get_callback(void *context, const char *param_name,
				char **value)
{
	(void)context;
	return nvram_get_checked(param_name, value);
}

static int
wan6_read_nvram_copy_status(const char *param_name, char *destination,
			    size_t destination_size)
{
	return rc_wan_nvram_copy_value(wan6_checked_nvram_get_callback, NULL,
				       param_name, destination,
				       destination_size);
}


static int
wan6_sit_clear_lan_addr6(void *context)
{
	(void)context;
	return clear_if_addr6(IFNAME_BR);
}

static int
wan6_sit_add_lan_addr6(void *context, const char *address)
{
	char *address_arg = (char *)address;

	(void)context;
	if (!address || !address[0]) {
		errno = EINVAL;
		return -1;
	}
	return eval("ip", "-6", "addr", "add", address_arg, "dev", IFNAME_BR);
}

static int
wan6_read_nvram_int_status(const char *param_name, int *value)
{
	char *stored_value = NULL;
	int api_result, saved_errno;

	if (!param_name || !param_name[0] || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_get_checked(param_name, &stored_value);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(api_result, saved_errno,
					    stored_value, 0, value);
}

#define WAN6_SIT_READ_CHECK(operation, description) do { \
	status = (operation); \
	if (status != 0) { \
		errno = status; \
		failure = (description); \
		goto failed; \
	} \
} while (0)

#define WAN6_SIT_CHECK(operation, description) do { \
	status = (operation); \
	if (status != 0) { \
		failure = (description); \
		goto failed; \
	} \
} while (0)

int start_sit_tunnel(int ipv6_type, char *wan_ifname, char *wan_addr4,
		     char *wan_gate4, char *wan_addr6, int unit)
{
	int sit_ttl, sit_mtu, size4, size6, status, cleanup_status;
	int tunnel_created, wan_route_added, route_recorded, record_status, written;
	int lan_address_touched, lan_rollback_status;
	char *sit_remote, *sit_relay, *sit_ep, *wan_gate6;
	char wan_addr4_text[INET_ADDRSTRLEN];
	char sit_remote_value[INET_ADDRSTRLEN];
	char sit_relay_value[INET_ADDRSTRLEN];
	char sit_gate6_value[INET6_ADDRSTRLEN + 5];
	char sit_remote_text[INET_ADDRSTRLEN];
	char sit_relay_text[INET_ADDRSTRLEN];
	char wan_gate6_text[INET6_ADDRSTRLEN];
	char generated_gate6_text[INET6_ADDRSTRLEN];
	char addr6s[INET6_ADDRSTRLEN + 5];
	char previous_lan_addr6[INET6_ADDRSTRLEN + 5];
	char lan_addr6s[INET6_ADDRSTRLEN + 5];
	char sit_6rd_prefix[INET6_ADDRSTRLEN + 5];
	char sit_6rd_relay_prefix[32];
	char sit_mtu_text[12], sit_ttl_text[4];
	struct in_addr addr4, net4;
	struct in6_addr addr6, net6, lan_addr6;
	const char *failure = "invalid parameters";

	status = -1;
	tunnel_created = 0;
	wan_route_added = 0;
	route_recorded = 0;
	lan_address_touched = 0;
	lan_rollback_status = 0;
	previous_lan_addr6[0] = '\0';
	sit_ep = NULL;
	sit_relay = "";
	sit_remote = "any";
	wan_gate6 = "";
	generated_gate6_text[0] = '\0';
	lan_addr6s[0] = '\0';
	sit_6rd_prefix[0] = '\0';
	sit_6rd_relay_prefix[0] = '\0';

	if (unit < 0 || !wan_ifname || !*wan_ifname ||
	    format_ipv4_literal(wan_addr4, wan_addr4_text,
				 sizeof(wan_addr4_text)) < 0 ||
	    inet_pton(AF_INET, wan_addr4_text, &addr4) != 1) {
		errno = EINVAL;
		logmessage("WAN IPv6", "invalid IPv4 endpoint for SIT tunnel");
		return -1;
	}

	memset(&addr6, 0, sizeof(addr6));
	size6 = ipv6_from_string(wan_addr6, &addr6);
	size4 = 0;

	if (ipv6_type == IPV6_6IN4) {
		WAN6_SIT_READ_CHECK(wan6_read_nvram_copy_status(
			"ip6_6in4_remote", sit_remote_value,
			sizeof(sit_remote_value)), "read 6in4 remote endpoint");
		if (!wan_addr6 || !*wan_addr6 || size6 < 0 ||
		    format_ipv4_literal(sit_remote_value, sit_remote_text,
				       sizeof(sit_remote_text)) < 0) {
			errno = EINVAL;
			return -1;
		}
		sit_remote = sit_remote_text;
		sit_ep = sit_remote_text;
		WAN6_SIT_READ_CHECK(get_wan_unit_value_copy_status(
			0, "gate6", sit_gate6_value, sizeof(sit_gate6_value)),
			"read 6in4 IPv6 gateway");
		wan_gate6 = sit_gate6_value;
		if (*wan_gate6) {
			if (format_ipv6_literal(wan_gate6, wan_gate6_text,
						sizeof(wan_gate6_text), 0) < 0) {
				errno = EINVAL;
				return -1;
			}
			wan_gate6 = wan_gate6_text;
		}
	} else if (ipv6_type == IPV6_6TO4) {
		WAN6_SIT_READ_CHECK(wan6_read_nvram_copy_status(
			"ip6_6to4_relay", sit_relay_value,
			sizeof(sit_relay_value)), "read 6to4 relay endpoint");
		if (format_ipv4_literal(sit_relay_value, sit_relay_text,
				       sizeof(sit_relay_text)) < 0) {
			errno = EINVAL;
			return -1;
		}
		sit_relay = sit_relay_text;
		sit_ep = sit_relay_text;
		size6 = 16;
		memset(&addr6, 0, sizeof(addr6));
		addr6.s6_addr16[0] = htons(0x2002);
		if (ipv6_to_ipv4_map(&addr6, size6, &addr4, 0) <= 0) {
			errno = EINVAL;
			return -1;
		}
		addr6.s6_addr16[7] = htons(0x0001);
		written = snprintf(generated_gate6_text, sizeof(generated_gate6_text),
				   "::%s", sit_relay);
		if (written < 0 || (size_t)written >= sizeof(generated_gate6_text)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		wan_gate6 = generated_gate6_text;

		memset(&lan_addr6, 0, sizeof(lan_addr6));
		lan_addr6.s6_addr16[0] = htons(0x2002);
		if (ipv6_to_ipv4_map(&lan_addr6, 16, &addr4, 0) <= 0) {
			errno = EINVAL;
			return -1;
		}
		lan_addr6.s6_addr16[3] = htons(0x0001);
		lan_addr6.s6_addr16[7] = htons(0x0001);
		if (!inet_ntop(AF_INET6, &lan_addr6, lan_addr6s, INET6_ADDRSTRLEN))
			return -1;
		{
			size_t used = strlen(lan_addr6s);
			size_t available = sizeof(lan_addr6s) - used;
			written = snprintf(lan_addr6s + used, available, "/64");
			if (written < 0 || (size_t)written >= available) {
				errno = ENAMETOOLONG;
				return -1;
			}
		}
	} else if (ipv6_type == IPV6_6RD) {
		if (!wan_addr6 || !*wan_addr6 || size6 < 0) {
			errno = EINVAL;
			return -1;
		}
		WAN6_SIT_READ_CHECK(wan6_read_sit_6rd_parameters(
			0, &size4, sit_relay_value, sizeof(sit_relay_value)),
			"snapshot 6rd IPv4 prefix length and relay endpoint");
		if (size4 < 0 || size4 > 32 ||
		    size6 + 32 - size4 > 128 ||
		    format_ipv4_literal(sit_relay_value, sit_relay_text,
			       sizeof(sit_relay_text)) < 0) {
			errno = EINVAL;
			return -1;
		}
		/* Ignore any non-zero suffix bits in legacy or static NVRAM values. */
		if (!wan6_sixrd_prefix_clear_host_bits(addr6.s6_addr, size6)) {
			errno = EINVAL;
			return -1;
		}
		sit_relay = sit_relay_text;
		sit_ep = sit_relay_text;

		memcpy(&net6, &addr6, sizeof(net6));
		ipv6_to_net(&net6, size6);
		if (!inet_ntop(AF_INET6, &net6, sit_6rd_prefix, INET6_ADDRSTRLEN))
			return -1;
		{
			size_t used = strlen(sit_6rd_prefix);
			size_t available = sizeof(sit_6rd_prefix) - used;
			written = snprintf(sit_6rd_prefix + used, available, "/%d", size6);
			if (written < 0 || (size_t)written >= available) {
				errno = ENAMETOOLONG;
				return -1;
			}
		}
		strcpy(sit_6rd_relay_prefix, "0.0.0.0/0");
		if (size4 > 0) {
			net4.s_addr = addr4.s_addr & htonl(0xffffffffUL << (32 - size4));
			snprintf(sit_6rd_relay_prefix, sizeof(sit_6rd_relay_prefix),
				 "%s/%d", inet_ntoa(net4), size4);
		}

		memcpy(&lan_addr6, &addr6, sizeof(lan_addr6));
		if (ipv6_to_ipv4_map(&lan_addr6, size6, &addr4, size4) <= 0) {
			errno = EINVAL;
			return -1;
		}
		lan_addr6.s6_addr16[7] = htons(0x0001);
		if (!inet_ntop(AF_INET6, &lan_addr6, lan_addr6s, INET6_ADDRSTRLEN))
			return -1;
		{
			size_t used = strlen(lan_addr6s);
			size_t available = sizeof(lan_addr6s) - used;
			written = snprintf(lan_addr6s + used, available, "/64");
			if (written < 0 || (size_t)written >= available) {
				errno = ENAMETOOLONG;
				return -1;
			}
		}
		written = snprintf(generated_gate6_text, sizeof(generated_gate6_text),
				   "::%s", sit_relay);
		if (written < 0 || (size_t)written >= sizeof(generated_gate6_text)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		wan_gate6 = generated_gate6_text;
	} else {
		errno = EINVAL;
		return -1;
	}

	WAN6_SIT_READ_CHECK(wan6_read_nvram_int_status(
		"ip6_sit_mtu", &sit_mtu), "read SIT MTU");
	WAN6_SIT_READ_CHECK(wan6_read_nvram_int_status(
		"ip6_sit_ttl", &sit_ttl), "read SIT TTL");
	if (sit_mtu < 1280)
		sit_mtu = 1280;
	if (sit_ttl < 1)
		sit_ttl = 1;
	if (sit_ttl > 255)
		sit_ttl = 255;

	written = snprintf(sit_mtu_text, sizeof(sit_mtu_text), "%d", sit_mtu);
	if (written < 0 || (size_t)written >= sizeof(sit_mtu_text)) {
		errno = EOVERFLOW;
		return -1;
	}
	written = snprintf(sit_ttl_text, sizeof(sit_ttl_text), "%d", sit_ttl);
	if (written < 0 || (size_t)written >= sizeof(sit_ttl_text)) {
		errno = EOVERFLOW;
		return -1;
	}

	/* Reconcile the previous owned endpoint route before replacing the tunnel. */
	WAN6_SIT_READ_CHECK(wan6_sit_route4_cleanup(unit),
			    "remove previous owned SIT endpoint route");

	if (is_interface_exist(IFNAME_SIT))
		WAN6_SIT_CHECK(eval("ip", "tunnel", "del", IFNAME_SIT),
			       "remove previous SIT tunnel");

	WAN6_SIT_CHECK(eval("ip", "tunnel", "add", IFNAME_SIT, "mode", "sit", "remote", sit_remote, "local", wan_addr4_text, "ttl", sit_ttl_text),
		       "create SIT tunnel");
	tunnel_created = 1;

	if (ipv6_type == IPV6_6RD)
		WAN6_SIT_CHECK(eval("ip", "tunnel", "6rd", "dev", IFNAME_SIT, "6rd-prefix", sit_6rd_prefix, "6rd-relay_prefix", sit_6rd_relay_prefix),
			       "configure 6rd tunnel");

	/* Route the IPv4 tunnel endpoint through the physical WAN first. */
	status = wan6_sit_endpoint_route_add_status(
		route_add(wan_ifname, 0, sit_ep,
			(char *)wan6_sit_route4_gateway_argument(wan_gate4),
			"255.255.255.255"),
		&wan_route_added);
	if (status != 0) {
		failure = "route SIT IPv4 endpoint";
		goto failed;
	}
	if (wan_route_added) {
		status = wan6_sit_route4_publish(
			unit, wan_ifname, sit_ep, wan_gate4);
		if (status != 0) {
			failure = "publish owned SIT endpoint route identity";
			goto failed;
		}
		route_recorded = 1;
	}

	if (!inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN)) {
		status = -1;
		failure = "format SIT WAN IPv6 address";
		goto failed;
	}
	if (size6 > 0) {
		size_t used = strlen(addr6s);
		size_t available = sizeof(addr6s) - used;
		written = snprintf(addr6s + used, available, "/%d", size6);
		if (written < 0 || (size_t)written >= available) {
			status = -1;
			errno = ENAMETOOLONG;
			failure = "format SIT WAN IPv6 prefix";
			goto failed;
		}
	}

	WAN6_SIT_CHECK(control_if_ipv6_radv(IFNAME_SIT, 0),
		       "disable SIT router advertisements");
	WAN6_SIT_CHECK(eval("ip", "link", "set", "mtu", sit_mtu_text, "dev", IFNAME_SIT, "up"),
		       "bring SIT interface up");
	WAN6_SIT_CHECK(control_if_ipv6(IFNAME_SIT, 1),
		       "enable IPv6 on SIT interface");
	WAN6_SIT_CHECK(clear_if_addr6(IFNAME_SIT),
		       "clear stale SIT IPv6 addresses");
	WAN6_SIT_CHECK(eval("ip", "-6", "addr", "add", addr6s, "dev", IFNAME_SIT),
		       "add SIT WAN IPv6 address");

	if (ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
		WAN6_SIT_CHECK(eval("ip", "-6", "route", "add", "default", "dev", IFNAME_SIT, "metric", "2048"), "add SIT fallback default route");

	if (*wan_gate6)
		WAN6_SIT_CHECK(eval("ip", "-6", "route", "add", "default", "via", wan_gate6, "dev", IFNAME_SIT, "metric", "1"), "add SIT IPv6 default gateway");

	/* Publish a derived LAN address only after tunnel and routes are ready. */
	if (*lan_addr6s) {
		WAN6_SIT_READ_CHECK(wan6_read_nvram_copy_status(
			"lan_addr6", previous_lan_addr6, sizeof(previous_lan_addr6)),
			"snapshot previous LAN IPv6 address before replacement");
		lan_address_touched = 1;
		WAN6_SIT_CHECK(clear_if_addr6(IFNAME_BR),
			       "clear previous LAN IPv6 address");
		WAN6_SIT_CHECK(eval("ip", "-6", "addr", "add", lan_addr6s, "dev", IFNAME_BR),
			       "add derived LAN IPv6 address");
		if (store_lan_addr6(lan_addr6s) < 0) {
			status = errno ? errno : EIO;
			failure = "publish derived LAN IPv6 address";
			goto failed;
		}
	}

	set_vpn_balancing(IFNAME_SIT, 0);
	return 0;

failed:
	if (lan_address_touched) {
		cleanup_status = wan6_sit_restore_lan_addr6(
			lan_address_touched, previous_lan_addr6,
			wan6_sit_clear_lan_addr6, wan6_sit_add_lan_addr6,
			NULL, &lan_rollback_status);
		if (cleanup_status != 0)
			logmessage("WAN IPv6",
				   "failed to restore previous LAN IPv6 address after SIT startup failure (status=%d)",
				   lan_rollback_status);
	}
	if (wan_route_added) {
		if (route_recorded) {
			cleanup_status = wan6_sit_route4_cleanup(unit);
		} else {
			cleanup_status = route_del(
			wan_ifname, 0, sit_ep,
			(char *)wan6_sit_route4_gateway_argument(wan_gate4),
			"255.255.255.255");
			if (cleanup_status == ESRCH || cleanup_status == ENODEV)
				cleanup_status = 0;
			if (cleanup_status == 0) {
				record_status = wan6_sit_route4_clear_record(NULL, unit);
				cleanup_status = record_status;
			} else {
				record_status = wan6_sit_route4_publish(
					unit, wan_ifname, sit_ep, wan_gate4);
				if (record_status != 0)
					logmessage("WAN IPv6",
						   "failed to preserve SIT endpoint route recovery identity (status=%d)",
						   record_status);
			}
		}
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to roll back SIT endpoint route (status=%d)",
				   cleanup_status);
	}
	if (tunnel_created) {
		cleanup_status = eval("ip", "link", "set", "dev", IFNAME_SIT, "down");
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to lower incomplete SIT tunnel (status=%d)",
				   cleanup_status);
		cleanup_status = clear_if_route6(IFNAME_SIT);
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to clear incomplete SIT routes (status=%d)",
				   cleanup_status);
		cleanup_status = clear_if_addr6(IFNAME_SIT);
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to clear incomplete SIT addresses (status=%d)",
				   cleanup_status);
		cleanup_status = stop_sit_tunnel();
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to remove incomplete SIT tunnel (status=%d)",
				   cleanup_status);
	}
	logmessage("WAN IPv6", "SIT startup failed at %s (status=%d)",
		   failure, status);
	return status ? status : -1;
}
#undef WAN6_SIT_CHECK
#undef WAN6_SIT_READ_CHECK
int stop_sit_tunnel(void)
{
	if (is_interface_exist(IFNAME_SIT))
		return eval("ip", "tunnel", "del", IFNAME_SIT);
	return 0;
}

static int
wan6_identity_set_value(void *context, int unit,
			const char *param_name, const char *value)
{
	(void)context;
	return set_wan_unit_value_status(unit, param_name, value);
}

static int
wan6_identity_stop_sit(void *context)
{
	(void)context;
	return stop_sit_tunnel();
}

#define WAN6_NATIVE_CHECK(operation, description) do { \
	status = (operation); \
	if (status != 0) { \
		failure = (description); \
		goto native_failed; \
	} \
} while (0)

int wan6_up(char *wan_ifname, int unit)
{
	int ipv6_type, allow_ra, status, operation_status;
	int wan_dhcp_enabled = 0, wan_priv_enabled = 0, privacy_enabled = 0;
	int routes_touched, addresses_touched, radv_touched;
	int autoconf_touched, privacy_touched, dhcp6_start_attempted;
	int identity_publish_failed, rollback_status;
	char wan_addr4_value[INET_ADDRSTRLEN] = {0};
	char wan_gate4_value[INET_ADDRSTRLEN] = {0};
	char wan_addr6_value[INET6_ADDRSTRLEN + 5] = {0};
	char wan_gate6_value[INET6_ADDRSTRLEN + 5] = {0};
	char wan_addr4_text[INET_ADDRSTRLEN];
	struct in6_addr preflight_addr6;
	char *wan_addr6 = wan_addr6_value, *wan_gate6 = wan_gate6_value;
	char *wan_addr4 = wan_addr4_value, *wan_gate4 = wan_gate4_value;
	char addr6_text[INET6_ADDRSTRLEN + 5];
	char gate6_text[INET6_ADDRSTRLEN];
	const char *failure = "invalid parameters";

	if (!wan_ifname || !*wan_ifname || unit < 0) {
		errno = EINVAL;
		return -1;
	}

	status = get_ipv6_type_status(&ipv6_type);
	if (status != 0) {
		errno = status;
		logmessage("WAN IPv6",
			   "refusing IPv6 startup because service configuration cannot be read (status=%d)",
			   status);
		return status;
	}

	/* Read native DHCPv6 RA/privacy controls before stopping or reconfiguring. */
	if (ipv6_type == IPV6_NATIVE_DHCP6) {
		status = wan6_read_nvram_match_status(
			"ip6_wan_dhcp", "1", &wan_dhcp_enabled);
		if (status != 0) {
			errno = status;
			logmessage("WAN IPv6",
				   "cannot read DHCPv6 RA selector; refusing native startup (status=%d)",
				   status);
			return status;
		}
		allow_ra = !wan_dhcp_enabled;
		if (allow_ra) {
			status = wan6_read_nvram_match_status(
				"ip6_wan_priv", "1", &wan_priv_enabled);
			if (status != 0) {
				errno = status;
				logmessage("WAN IPv6",
					   "cannot read DHCPv6 privacy selector; refusing native startup (status=%d)",
					   status);
				return status;
			}
		}
		privacy_enabled = allow_ra && wan_priv_enabled;
	}

	/* Snapshot every per-unit address field needed by this mode before stop. */
	status = wan6_read_unit_address_snapshot(
		unit, ipv6_type,
		wan_addr4_value, sizeof(wan_addr4_value),
		wan_gate4_value, sizeof(wan_gate4_value),
		wan_addr6_value, sizeof(wan_addr6_value),
		wan_gate6_value, sizeof(wan_gate6_value));
	if (status != 0) {
		errno = status;
		logmessage("WAN IPv6",
			   "cannot read WAN IPv6 address configuration for unit %d (status=%d)",
			   unit, status);
		return status;
	}
	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD) {
		if (format_ipv4_literal(wan_addr4, wan_addr4_text,
					 sizeof(wan_addr4_text)) < 0) {
			errno = EINVAL;
			logmessage("WAN IPv6", "invalid WAN IPv4 address for unit %d", unit);
			return EINVAL;
		}
		if (!is_valid_ipv4(wan_gate4))
			wan_gate4 = NULL;
		if ((ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6RD) &&
		    (!wan_addr6[0] || ipv6_from_string(wan_addr6, &preflight_addr6) < 0)) {
			errno = EINVAL;
			logmessage("WAN IPv6", "invalid WAN IPv6 prefix address for unit %d", unit);
			return EINVAL;
		}
	} else if (ipv6_type == IPV6_NATIVE_STATIC) {
		if (!wan_addr6[0] ||
		    format_ipv6_literal(wan_addr6, addr6_text,
					 sizeof(addr6_text), 1) < 0) {
			errno = EINVAL;
			logmessage("WAN IPv6", "invalid native static IPv6 address for unit %d", unit);
			return EINVAL;
		}
		if (wan_gate6[0] &&
		    format_ipv6_literal(wan_gate6, gate6_text,
					 sizeof(gate6_text), 0) < 0) {
			errno = EINVAL;
			logmessage("WAN IPv6", "invalid native static IPv6 gateway for unit %d", unit);
			return EINVAL;
		}
	}

	status = stop_dhcp6c();
	if (status != 0) {
		logmessage("WAN IPv6", "failed to stop previous DHCPv6 client on %s (status=%d)",
			   wan_ifname, status);
		return status;
	}
	if (ipv6_type == IPV6_DISABLED)
		return 0;

	status = publish_dns6_var_status();
	if (status != 0) {
		logmessage("WAN IPv6",
			   "failed to publish configured DNS state before startup (status=%d)",
			   status);
		return status;
	}

	status = control_if_ipv6_dad(IFNAME_BR, 1);
	if (status != 0) {
		logmessage("WAN IPv6", "failed to configure bridge DAD on %s (status=%d)",
			   IFNAME_BR, status);
		return status;
	}

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD) {
		status = start_sit_tunnel(ipv6_type, wan_ifname, wan_addr4,
					  wan_gate4, wan_addr6, unit);
		if (status != 0) {
			logmessage("WAN IPv6", "SIT setup incomplete on %s (status=%d)",
				   wan_ifname, status);
			return status;
		}
		status = wan6_publish_sit_identity(
			wan6_identity_set_value, wan6_identity_stop_sit, NULL,
			unit, IFNAME_SIT, &operation_status);
		if (status != 0) {
			if (operation_status != 0)
				logmessage("WAN IPv6",
					   "SIT interface identity rollback was incomplete (status=%d)",
					   operation_status);
			logmessage("WAN IPv6",
				   "SIT startup aborted because the active IPv6 interface identity could not be published (status=%d)",
				   status);
			return status;
		}
		return 0;
	}

	status = control_if_ipv6_dad(wan_ifname, 1);
	if (status != 0) {
		logmessage("WAN IPv6", "failed to configure DAD on %s (status=%d)",
			   wan_ifname, status);
		return status;
	}

	routes_touched = 0;
	addresses_touched = 0;
	radv_touched = 0;
	autoconf_touched = 0;
	privacy_touched = 0;
	dhcp6_start_attempted = 0;
	identity_publish_failed = 0;
	rollback_status = 0;

	if (ipv6_type == IPV6_NATIVE_STATIC) {
		privacy_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_privacy(wan_ifname, 0),
				  "disable native static privacy addresses");
		radv_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_radv(wan_ifname, 0),
				  "disable native static router advertisements");

		addresses_touched = 1;
		WAN6_NATIVE_CHECK(clear_if_addr6(wan_ifname),
				  "clear previous native static IPv6 addresses");
		WAN6_NATIVE_CHECK(eval("ip", "-6", "addr", "add", addr6_text, "dev", wan_ifname),
				  "add native static IPv6 address");

		if (*wan_gate6) {
			routes_touched = 1;
			WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", gate6_text, "dev", wan_ifname),
					  "add native static on-link gateway route");
			WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", "default", "via", gate6_text, "metric", "1"),
					  "add native static default route");
		}
	} else if (ipv6_type == IPV6_NATIVE_DHCP6) {
		routes_touched = 1;
		WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", "default", "dev", wan_ifname, "metric", "2048"),
				  "add native DHCPv6 fallback default route");
		privacy_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_privacy(wan_ifname, privacy_enabled),
				  "configure native DHCPv6 privacy addresses");
		autoconf_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_autoconf(wan_ifname, allow_ra),
				  "configure native DHCPv6 autoconfiguration");
		radv_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_radv(wan_ifname, 1),
				  "enable native DHCPv6 router advertisements");

		/* Complete the full readiness interval even when a signal interrupts it. */
		if (service_wait_seconds(2) != 0) {
			status = errno > 0 ? errno : EIO;
			errno = status;
			failure = "wait for native DHCPv6 interface readiness";
			goto native_failed;
		}
		dhcp6_start_attempted = 1;
		WAN6_NATIVE_CHECK(start_dhcp6c(wan_ifname),
				  "start native DHCPv6 client");
	} else {
		errno = EINVAL;
		status = -1;
		failure = "unsupported IPv6 WAN type";
		goto native_failed;
	}

	status = set_wan_unit_value_status(unit, "ifname6", wan_ifname);
	if (status != 0) {
		identity_publish_failed = 1;
		failure = "publish native IPv6 WAN interface identity";
		goto native_failed;
	}
	return 0;

native_failed:
	if (dhcp6_start_attempted) {
		operation_status = stop_dhcp6c();
		if (operation_status != 0) {
			logmessage("WAN IPv6", "failed to stop partially started DHCPv6 client (status=%d)",
				   operation_status);
			return operation_status;
		}
	}
	if (radv_touched) {
		operation_status = control_if_ipv6_radv(wan_ifname, 0);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back router advertisement setting (status=%d)",
				   operation_status);
	}
	if (autoconf_touched) {
		operation_status = control_if_ipv6_autoconf(wan_ifname, 0);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back autoconfiguration setting (status=%d)",
				   operation_status);
	}
	if (privacy_touched) {
		operation_status = control_if_ipv6_privacy(wan_ifname, 0);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back privacy setting (status=%d)",
				   operation_status);
	}
	if (routes_touched) {
		operation_status = clear_if_route6(wan_ifname);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to clear partially configured routes (status=%d)",
				   operation_status);
	}
	if (routes_touched || addresses_touched) {
		operation_status = clear_if_neigh6(wan_ifname);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to clear partially configured neighbours (status=%d)",
				   operation_status);
	}
	if (addresses_touched) {
		operation_status = clear_if_addr6(wan_ifname);
		rollback_status = route_status_accumulate(rollback_status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to clear partially configured addresses (status=%d)",
				   operation_status);
	}
	if (identity_publish_failed) {
		operation_status = wan6_clear_native_identity_after_rollback(
			wan6_identity_set_value, NULL, unit, rollback_status);
		if (rollback_status != 0)
			logmessage("WAN IPv6",
				   "retaining native IPv6 interface identity because kernel rollback was incomplete (status=%d)",
				   rollback_status);
		else if (operation_status != 0)
			logmessage("WAN IPv6",
				   "failed to clear native IPv6 interface identity after startup rollback (status=%d)",
				   operation_status);
	}
	logmessage("WAN IPv6", "native startup failed at %s on %s (status=%d)",
		   failure, wan_ifname, status);
	return status ? status : -1;
}
#undef WAN6_NATIVE_CHECK

int wan6_down(char *wan_ifname, int unit)
{
	int ipv6_type, stop_status, status, operation_status;
	int clear_status, previous_if_exists, sit_if_exists, ipv6_sit;
	char previous_ifname_buffer[IFNAMSIZ] = {0};
	char *previous_ifname, *wan6_ifname;

	stop_status = stop_dhcp6c();
	if (stop_status != 0) {
		logmessage("WAN IPv6", "refusing IPv6 teardown while DHCPv6 client/callback remains active on %s (status=%d)",
			   wan_ifname ? wan_ifname : "(unknown)", stop_status);
		return stop_status;
	}
	status = get_ipv6_type_status(&ipv6_type);
	if (status != 0) {
		errno = status;
		logmessage("WAN IPv6",
			   "refusing IPv6 teardown because service configuration cannot be read (status=%d)",
			   status);
		return status;
	}

	status = get_wan_unit_value_copy_status(
		unit, "ifname6", previous_ifname_buffer,
		sizeof(previous_ifname_buffer));
	if (status != 0) {
		errno = status;
		logmessage("WAN IPv6",
			   "refusing IPv6 teardown because previous interface identity cannot be read for unit %d (status=%d)",
			   unit, status);
		return status;
	}
	previous_ifname = previous_ifname_buffer;
	previous_if_exists = previous_ifname[0] &&
		is_interface_exist(previous_ifname);
	sit_if_exists = is_interface_exist(IFNAME_SIT);
	ipv6_sit = ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 ||
		ipv6_type == IPV6_6RD;
	wan6_ifname = wan6_teardown_select_ifname(
		previous_ifname, previous_if_exists,
		ipv6_type == IPV6_DISABLED, ipv6_sit, sit_if_exists,
		wan_ifname, IFNAME_SIT);
	if (!wan6_ifname) {
		/* A vanished PPP/device interface has already lost IPv6 state. Reconcile
		 * any owned IPv4 SIT endpoint route before clearing interface identity. */
		status = wan6_sit_route4_cleanup(unit);
		clear_status = wan6_clear_runtime_metadata(
			wan6_identity_set_value, NULL, unit, 1);
		status = route_status_accumulate(status, clear_status);
		if (status != 0)
			logmessage("WAN IPv6",
				   "failed to clear metadata after IPv6 interface disappearance (status=%d)",
				   status);
		return status;
	}

	status = 0;

	operation_status = control_if_ipv6_radv(wan_ifname, 0);

	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to disable router advertisements on %s (status=%d)",
			   wan_ifname, operation_status);

	operation_status = control_if_ipv6_autoconf(wan_ifname, 0);
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to disable address autoconfiguration on %s (status=%d)",
			   wan_ifname, operation_status);

	if (strcmp(wan6_ifname, wan_ifname) != 0 &&
	    strcmp(wan6_ifname, IFNAME_SIT) != 0) {
		operation_status = control_if_ipv6_radv(wan6_ifname, 0);
		status = route_status_accumulate(status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to disable router advertisements on previous interface %s (status=%d)",
				   wan6_ifname, operation_status);

		operation_status = control_if_ipv6_autoconf(wan6_ifname, 0);
		status = route_status_accumulate(status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to disable address autoconfiguration on previous interface %s (status=%d)",
				   wan6_ifname, operation_status);
	}

	if (strcmp(wan6_ifname, IFNAME_SIT) == 0) {
		if (is_interface_exist(IFNAME_SIT)) {
			operation_status = eval("ip", "link", "set", "dev", IFNAME_SIT, "down");
			status = route_status_accumulate(status, operation_status);
			if (operation_status != 0)
				logmessage("WAN IPv6", "failed to bring SIT interface down (status=%d)",
					   operation_status);
		}
	}

	operation_status = clear_if_route6(wan6_ifname);
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to clear routes on %s (status=%d)",
			   wan6_ifname, operation_status);

	operation_status = clear_if_neigh6(wan6_ifname);
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to clear neighbour entries on %s (status=%d)",
			   wan6_ifname, operation_status);

	operation_status = clear_if_addr6(wan6_ifname);
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to clear addresses on %s (status=%d)",
			   wan6_ifname, operation_status);

	operation_status = stop_sit_tunnel();
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to remove SIT tunnel (status=%d)",
			   operation_status);
	else {
		operation_status = wan6_sit_route4_cleanup(unit);
		status = route_status_accumulate(status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6",
				   "failed to remove owned SIT endpoint route (status=%d)",
				   operation_status);
	}

	if (status == 0) {
		operation_status = wan6_clear_runtime_metadata(
			wan6_identity_set_value, NULL, unit, 1);
		status = route_status_accumulate(status, operation_status);
		if (operation_status != 0)
			logmessage("WAN IPv6",
				   "failed to clear IPv6 metadata after teardown (status=%d)",
				   operation_status);
	}
	if (status != 0)
		logmessage("WAN IPv6", "retaining IPv6 metadata after incomplete teardown (status=%d)",
			   status);

	return status;
}


static int
wan6_lookup_lan_addr6_status(void *context, char *address,
			     size_t address_size, int *found)
{
	char *result;

	(void)context;
	if (!address || !found || address_size < INET6_ADDRSTRLEN + 5)
		return EINVAL;
	address[0] = '\0';
	*found = 0;
	errno = 0;
	result = get_ifaddr6(IFNAME_BR, 0, address);
	if (!result)
		return errno ? errno : 0;
	*found = 1;
	return 0;
}

static int
wan6_publish_lan_addr6_status(void *context, const char *address)
{
	int status;

	(void)context;
	status = store_lan_addr6((char *)address);
	if (status < 0)
		return errno ? errno : EIO;
	return 0;
}

int dhcp6c_main(int argc, char **argv)
{
	static const char *const setting_names[] = {
		"ip6_lan_auto", "ip6_dns_auto"
	};
	int ipv6_type, dns6_auto, lan6_auto, setting_values[2];
	int callback_fd, unlock_status, callback_status = 0, saved_errno = 0;
	int dns_status, dns_errno, settings_status, lan_addr_status;
	unsigned long generation;
	char *dns6_new;
	char addr6s[INET6_ADDRSTRLEN + 5];
	const char *callback_generation, *expected_ifname, *callback_ifname;

	(void)argc;
	(void)argv;
	callback_generation = getenv(DHCP6C_CALLBACK_GENERATION_ENV);
	expected_ifname = getenv(DHCP6C_CALLBACK_IFNAME_ENV);
	callback_ifname = getenv("interface");

	callback_fd = dhcp6c_callback_session_lock(DHCP6C_CALLBACK_STATE_FILE,
						    &generation, 0);
	if (callback_fd < 0) {
		logmessage("DHCPv6 WAN Client",
			   "refusing callback: session lock unavailable (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	if (!dhcp6c_callback_generation_matches(callback_generation, generation,
						expected_ifname, callback_ifname)) {
		logmessage("DHCPv6 WAN Client", "ignoring stale callback on %s",
			   callback_ifname ? callback_ifname : "(unknown)");
		unlock_status = dhcp6c_callback_session_unlock(callback_fd);
		return unlock_status == 0 ? 0 : -1;
	}

	settings_status = get_ipv6_type_status(&ipv6_type);
	if (settings_status != 0) {
		saved_errno = settings_status;
		logmessage("DHCPv6 WAN Client",
			   "refusing callback because IPv6 service configuration cannot be read (status=%d)",
			   settings_status);
		unlock_status = dhcp6c_callback_session_unlock(callback_fd);
		if (unlock_status != 0)
			return -1;
		errno = saved_errno;
		return -1;
	}
	if (ipv6_type != IPV6_NATIVE_DHCP6) {
		unlock_status = dhcp6c_callback_session_unlock(callback_fd);
		return unlock_status == 0 ? 0 : -1;
	}

	settings_status = wan6_read_dhcp_integer_snapshot(
		setting_names, 2, setting_values);
	if (settings_status != 0) {
		saved_errno = settings_status;
		callback_status = -1;
		logmessage("DHCPv6 WAN Client",
			   "refusing lease-state publication because automatic settings could not be read (status=%d)",
			   settings_status);
	} else {
		lan6_auto = setting_values[0];
		dns6_auto = setting_values[1];
		if (lan6_auto) {
			lan_addr_status = wan6_identity_publish_lan_addr6(
				wan6_lookup_lan_addr6_status,
				wan6_publish_lan_addr6_status,
				NULL, addr6s, sizeof(addr6s));
			if (lan_addr_status != 0) {
				saved_errno = lan_addr_status;
				callback_status = -1;
				logmessage("DHCPv6 WAN Client",
					   "cannot read or publish automatic LAN address state (status=%d)",
					   lan_addr_status);
			}
		}

		if (dns6_auto) {
			dns6_new = getenv("new_domain_name_servers");
			dns_status = store_wan_dns6(dns6_new);
			if (dns_status > 0)
				refresh_resolvconf_nonfatal("DHCPv6 DNS update", 0);
			else if (dns_status < 0) {
				dns_errno = errno ? errno : EIO;
				if (callback_status == 0) {
					callback_status = dns_status;
					saved_errno = dns_errno;
				}
				logmessage("DHCPv6 WAN Client",
					   "rejecting invalid DHCPv6 DNS server list (errno=%d)",
					   dns_errno);
			}
		}
	}

	unlock_status = dhcp6c_callback_session_unlock(callback_fd);
	if (unlock_status != 0)
		return -1;
	if (callback_status < 0) {
		errno = saved_errno;
		return callback_status;
	}
	return 0;
}

int start_dhcp6c(char *wan_ifname)
{
	char wan6_hwaddr_snapshot[32] = {0};
	struct nvram_transaction wan6_hwaddr_transaction;
	struct rc_wan_nvram_snapshot_field wan6_hwaddr_field;
	int wan6_hwaddr_status;

	char *svcs[] = { "dhcp6c", NULL };
	static const char *const setting_names[] = {
		"ip6_wan_dhcp", "ip6_dns_auto", "ip6_lan_auto"
	};
	FILE *fp;
	int duid_fd, session_fd, start_status, unlock_status;
	int client_stop_required, client_stop_status;
	int wan6_dhcp, dns6_auto, lan6_auto, setting_values[3], settings_status;
	int sla_id, sla_len;
	int duid_error, saved_errno, start_errno, cleanup_errno;
	int generation_active, env_cleanup_status;
	unsigned int ia_id;
	unsigned long generation;
	unsigned char ea[ETHER_ADDR_LEN];
	const char *conf_file = "/etc/dhcp6c.conf";
	const char *duid_file = "/tmp/dhcp6c_duid";
	char staged_path[] = "/etc/.dhcp6c.conf.XXXXXX";
	char generation_text[sizeof(unsigned long) * CHAR_BIT + 2];

	if (!wan_ifname || !wan_ifname[0] || strlen(wan_ifname) >= IFNAMSIZ) {
		errno = EINVAL;
		return -1;
	}

	settings_status = wan6_read_dhcp_integer_snapshot(
		setting_names, 3, setting_values);
	if (settings_status != 0) {
		errno = settings_status;
		logmessage("DHCPv6 WAN Client",
			   "cannot read DHCPv6 client configuration flags; refusing startup (status=%d)",
			   settings_status);
		return -1;
	}
	wan6_dhcp = setting_values[0];
	dns6_auto = setting_values[1];
	lan6_auto = setting_values[2];

	if (!wan6_dhcp && !dns6_auto && !lan6_auto)
		return 0; /* No DHCPv6 client work is configured. */

	wan6_hwaddr_field.param_name = "wan_hwaddr";
	wan6_hwaddr_field.destination = wan6_hwaddr_snapshot;
	wan6_hwaddr_field.destination_size = sizeof(wan6_hwaddr_snapshot);
	wan6_hwaddr_status = rc_wan_nvram_copy_snapshot(
		&wan6_hwaddr_field, 1, wan6_snapshot_transaction_begin,
		wan6_snapshot_transaction_get, wan6_snapshot_transaction_end,
		&wan6_hwaddr_transaction);
	if (wan6_hwaddr_status != 0)
		return wan6_hwaddr_status;

	
	session_fd = dhcp6c_callback_session_lock(DHCP6C_CALLBACK_STATE_FILE,
						  &generation, 1);
	if (session_fd < 0) {
		logmessage("DHCPv6 WAN Client",
			   "cannot lock callback session before startup (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	generation_active = 0;
	if (dhcp6c_callback_session_advance(session_fd, &generation) != 0)
		goto session_failed;
	generation_active = 1;
	if (dhcp6c_callback_generation_format(generation, generation_text,
					      sizeof(generation_text)) != 0)
		goto session_failed;

	ia_id = 0;
	sla_id = 1;
	sla_len = 0; /* auto prefix always /64 */

	if (ether_atoe(wan6_hwaddr_snapshot, ea)) {
		uint16_t duid_len;
		struct {
			uint16_t type;
			uint16_t hwtype;
		} __attribute__ ((__packed__)) duid3;
		size_t duid_done;

		/* generate IAID from the last 20 bits of WAN MAC */
		ia_id = ((unsigned int)(ea[3] & 0x0f) << 16) |
			((unsigned int)(ea[4]) << 8) |
			((unsigned int)(ea[5]));

		/* create DUID from WAN MAC */
		duid_len = sizeof(duid3) + ETHER_ADDR_LEN;
		duid3.type = htons(3);	/* DUID-LL */
		duid3.hwtype = htons(1);	/* Ethernet */

		/* /tmp is shared-writable: never reopen this pathname with
		 * fopen("w"), which follows a symlink raced into place. */
		if (unlink(duid_file) != 0 && errno != ENOENT) {
			duid_error = errno ? errno : EIO;
			logmessage("DHCPv6 WAN Client",
				   "cannot remove stale DUID file (errno=%d)", duid_error);
			errno = duid_error;
			goto session_failed;
		}

		duid_fd = dhcp6c_duid_open_create(duid_file);
		if (duid_fd < 0) {
			duid_error = errno ? errno : EIO;
			logmessage("DHCPv6 WAN Client",
				   "cannot safely create DUID file (errno=%d)", duid_error);
			errno = duid_error;
			goto session_failed;
		}

		fp = fdopen(duid_fd, "wb");
		if (!fp) {
			duid_error = errno ? errno : EIO;
			close(duid_fd);
			unlink(duid_file);
			logmessage("DHCPv6 WAN Client",
				   "cannot open DUID stream (errno=%d)", duid_error);
			errno = duid_error;
			goto session_failed;
		}

		duid_done = 0;
		duid_done += fwrite(&duid_len, sizeof(duid_len), 1, fp);
		duid_done += fwrite(&duid3, sizeof(duid3), 1, fp);
		duid_done += fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		duid_error = (duid_done == 3) ? 0 : EIO;
		if (fclose(fp) != 0 && duid_error == 0)
			duid_error = errno ? errno : EIO;
		if (duid_error != 0) {
			unlink(duid_file);
			logmessage("DHCPv6 WAN Client",
				   "incomplete DUID file write (errno=%d)", duid_error);
			errno = duid_error;
			goto session_failed;
		}
	}

	if (dhcp6c_config_file_open_staged(staged_path, &fp) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("DHCPv6 WAN Client",
			   "cannot safely stage client configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		goto session_failed;
	}

	fprintf(fp, "interface %s {\n", wan_ifname);
	if (wan6_dhcp)
		fprintf(fp, " send ia-na %d;\n", ia_id);
	if (lan6_auto)
		fprintf(fp, " send ia-pd %d;\n", ia_id);
	if (wan6_dhcp || lan6_auto)
		fprintf(fp, " send rapid-commit;\n");
	else
		fprintf(fp, " information-only ;\n"); /* space needed for dhcp6c port in busybox */
	if (dns6_auto)
		fprintf(fp, " request domain-name-servers;\n");
	fprintf(fp,
		" script \"%s\";\n"
		"};\n",
		SCRIPT_DHCP6C_WAN
	);
	if (wan6_dhcp)
		fprintf(fp, "id-assoc na %d { };\n", ia_id);
	if (lan6_auto)
		fprintf(fp,
			"id-assoc pd %d {\n"
			" prefix-interface %s {\n"
			"  sla-id %d;\n"
			"  sla-len %d;\n"
			" };\n"
			"};\n",
			ia_id,
			IFNAME_BR,
			sla_id,
			sla_len
		);

	if (dhcp6c_config_file_publish(fp, staged_path, conf_file) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("DHCPv6 WAN Client",
			   "cannot publish complete client configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		goto session_failed;
	}

	if (setenv(DHCP6C_CALLBACK_GENERATION_ENV, generation_text, 1) != 0)
		goto session_failed;
	if (setenv(DHCP6C_CALLBACK_IFNAME_ENV, wan_ifname, 1) != 0)
		goto session_failed;

	logmessage("DHCPv6 WAN Client", "starting on wan (%s) ...", wan_ifname);
	start_status = eval("/sbin/dhcp6c", "-D", "LL", wan_ifname);
	start_errno = errno;
	env_cleanup_status = dhcp6c_callback_environment_clear(
		DHCP6C_CALLBACK_IFNAME_ENV, DHCP6C_CALLBACK_GENERATION_ENV, unsetenv);
	cleanup_errno = env_cleanup_status != 0 ? (errno ? errno : EIO) : 0;
	saved_errno = dhcp6c_callback_start_errno(start_status, start_errno,
							 env_cleanup_status, cleanup_errno);
	client_stop_required = dhcp6c_callback_start_requires_stop(
		start_status, env_cleanup_status);
	if (env_cleanup_status != 0) {
		if (client_stop_required)
			start_status = -1;
		logmessage("DHCPv6 WAN Client",
			   "cannot clear temporary callback environment after client start (errno=%d)",
			   cleanup_errno);
	}
	if (start_status != 0) {
		if (dhcp6c_callback_session_advance(session_fd, &generation) != 0)
			logmessage("DHCPv6 WAN Client",
				   "cannot invalidate failed client start (errno=%d)", errno ? errno : EIO);
		if (client_stop_required) {
			client_stop_status = kill_services(svcs, 3, 1);
			if (client_stop_status != 0)
				logmessage("DHCPv6 WAN Client",
					   "failed to stop client after callback environment cleanup failure (status=%d)",
					   client_stop_status);
		}
		unlock_status = dhcp6c_callback_session_unlock(session_fd);
		if (unlock_status != 0)
			logmessage("DHCPv6 WAN Client",
				   "cannot release callback session lock after start failure (errno=%d)",
				   errno ? errno : EIO);
		if (saved_errno)
			errno = saved_errno;
		return start_status;
	}

	unlock_status = dhcp6c_callback_session_unlock(session_fd);
	if (unlock_status != 0) {
		logmessage("DHCPv6 WAN Client",
			   "cannot release callback session lock after client start (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	return 0;

session_failed:
	saved_errno = errno ? errno : EIO;
	if (dhcp6c_callback_environment_clear(
		DHCP6C_CALLBACK_IFNAME_ENV, DHCP6C_CALLBACK_GENERATION_ENV, unsetenv) != 0)
		logmessage("DHCPv6 WAN Client",
			   "cannot clear temporary callback environment during startup rollback (errno=%d)",
			   errno ? errno : EIO);
	if (generation_active &&
	    dhcp6c_callback_session_advance(session_fd, &generation) != 0)
		logmessage("DHCPv6 WAN Client",
			   "cannot invalidate failed startup generation (errno=%d)", errno ? errno : EIO);
	unlock_status = dhcp6c_callback_session_unlock(session_fd);
	if (unlock_status != 0)
		logmessage("DHCPv6 WAN Client",
			   "cannot release callback session lock after startup failure (errno=%d)",
			   errno ? errno : EIO);
	errno = saved_errno;
	return -1;
}

int stop_dhcp6c(void)
{
	char *svcs[] = { "dhcp6c", NULL };
	unsigned long generation;
	int session_fd, stop_status, unlock_status, saved_errno, unlock_errno;

	session_fd = dhcp6c_callback_session_lock(DHCP6C_CALLBACK_STATE_FILE,
						  &generation, 1);
	if (session_fd < 0) {
		logmessage("DHCPv6 WAN Client",
			   "cannot lock callback session before stop (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	if (dhcp6c_callback_session_advance(session_fd, &generation) != 0) {
		saved_errno = errno ? errno : EIO;
		unlock_errno = 0;
		unlock_status = dhcp6c_callback_session_unlock(session_fd);
		if (unlock_status != 0) {
			unlock_errno = errno ? errno : EIO;
			logmessage("DHCPv6 WAN Client",
				   "cannot release callback session lock after generation invalidation failure (errno=%d)",
				   unlock_errno);
		}
		logmessage("DHCPv6 WAN Client",
			   "cannot invalidate callback session before stop (errno=%d)", saved_errno);
		errno = dhcp6c_callback_primary_errno(saved_errno, unlock_errno);
		return -1;
	}

	/* Waiting callback children now see the new generation after this lock
	 * is released; killall only targets dhcp6c, not its synchronous rc child. */
	stop_status = kill_services(svcs, 3, 1);
	unlock_status = dhcp6c_callback_session_unlock(session_fd);
	if (stop_status == 0 && unlock_status != 0)
		stop_status = unlock_status;
	if (stop_status != 0)
		logmessage("DHCPv6 WAN Client",
			   "client stop or callback-lock release incomplete (status=%d)",
			   stop_status);
	return stop_status;
}

#endif

