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
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <signal.h>

#include "rc.h"
#include <nvram_linux.h>
#include "network_sysctl.h"
#include "network_sysctl_policy.h"
#include "service_start_status.h"
#include "wan_mtu_setup_policy.h"
#include "wan_route_flush_policy.h"
#include "apcli_wait_policy.h"
#include "service_wait_policy.h"
#include <process_inventory.h>
#include <process_pidfile.h>
#include "process_presence_start_policy.h"
#include "udhcpc_start_readiness.h"
#include "ppp_options_file.h"
#include "ppp_callback_link_policy.h"
#include "vpn_server_lifecycle_lock.h"
#include "route_status_policy.h"
#include "wan_nvram_status_policy.h"
#include "wan_vlan_vid_policy.h"
#include "modem_wan_start_policy.h"
#include "detect_internet_notification_policy.h"
#include "dnsmasq_file.h"
#include "dnsmasq_lifecycle_lock.h"
#include "dnsmasq_domain_policy.h"
#include "wan_resolver_refresh_policy.h"
#include "wan_teardown_policy.h"
#include "wan_deferred_reconnect_policy.h"
#include "dhcp_lease_policy.h"
#include "dhcp_wan_interface_policy.h"
#include "wan_ifname_policy.h"
#include "lan_kernel_name_policy.h"
#include "dhcp_wan_route_transition_policy.h"
#include "wan_viptv_static_policy.h"
#include "bridge_membership_policy.h"
#include "switch.h"
#include "switch_status_policy.h"
#include "wan_src_phy_policy.h"
#include <gpioutils.h>

static int
wan_sethostname_status(const char *name, size_t length)
{
	if (sethostname(name, length) == 0)
		return 0;
	return errno ? errno : EIO;
}

static int
wan_setdomainname_status(const char *name, size_t length)
{
	if (setdomainname(name, length) == 0)
		return 0;
	return errno ? errno : EIO;
}

static const lan_kernel_name_ops_t wan_kernel_name_ops = {
	wan_sethostname_status,
	wan_setdomainname_status
};

char*
get_wan_unit_value(int unit, const char* param_name)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	return nvram_safe_get(wanN_param);
}


static int
wan_nvram_checked_get_callback(void *context, const char *param_name,
				char **value)
{
	(void)context;
	return nvram_get_checked(param_name, value);
}


int
get_wan_unit_value_copy_status(int unit, const char *param_name,
			       char *destination, size_t destination_size)
{
	char wanN_param[64];
	int length;

	if (unit < 0 || !param_name || !param_name[0] ||
	    !destination || destination_size == 0)
		return EINVAL;
	length = snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s",
			  unit, param_name);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(wanN_param))
		return ENAMETOOLONG;
	return rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
				       wanN_param, destination,
				       destination_size);
}


int
get_wan_unit_value_int_status(int unit, const char *param_name, int *value)
{
	char wanN_param[64];
	char *stored_value = NULL;
	int length, api_result, saved_errno;

	if (unit < 0 || !param_name || !param_name[0] || !value)
		return EINVAL;
	length = snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit,
			  param_name);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(wanN_param))
		return ENAMETOOLONG;

	errno = 0;
	api_result = nvram_get_checked(wanN_param, &stored_value);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(api_result, saved_errno,
					    stored_value, 0, value);
}

static int
get_nvram_int_checked_status(const char *param_name, int *value)
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


static int
get_nvram_match_checked_status(const char *param_name,
			       const char *expected, int *matches)
{
	char *stored_value = NULL;
	int api_result, saved_errno, read_status;

	if (!param_name || !param_name[0] || !expected || !matches)
		return EINVAL;
	*matches = 0;

	errno = 0;
	api_result = nvram_get_checked(param_name, &stored_value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (read_status != 0)
		return rc_wan_nvram_match_status(read_status, NULL,
						expected, matches);
	if (api_result == 0)
		stored_value = "";
	return rc_wan_nvram_match_status(0, stored_value, expected, matches);
}


int
get_wan_unit_value_int(int unit, const char* param_name)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	return nvram_get_int(wanN_param);
}

int
set_wan_unit_value_status(int unit, const char *param_name, const char *value)
{
	char wanN_param[64];
	int path_len, api_result, saved_errno;

	if (unit < 0 || !param_name || !param_name[0] || !value)
		return EINVAL;

	path_len = snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit,
			    param_name);
	if (path_len < 0)
		return errno ? errno : EIO;
	if ((size_t)path_len >= sizeof(wanN_param))
		return ENAMETOOLONG;

	errno = 0;
	api_result = nvram_set_temp(wanN_param, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}


static int
get_wan_modem_is_ndis_status(int *is_ndis)
{
	int modem_type = 0;
	int status;

	if (!is_ndis)
		return EINVAL;
	status = get_nvram_int_checked_status("modem_type", &modem_type);
	return rc_wan_nvram_modem_is_ndis_status(status, modem_type, is_ndis);
}


static int
wan_nvram_int_temp_status(const char *param_name, int value)
{
	int api_result, saved_errno;

	if (!param_name || !param_name[0])
		return EINVAL;

	errno = 0;
	api_result = nvram_set_int_temp(param_name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
wan_nvram_int_status(const char *param_name, int value)
{
	int api_result, saved_errno;

	if (!param_name || !param_name[0])
		return EINVAL;

	errno = 0;
	api_result = nvram_set_int(param_name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}




int
set_wan_unit_value_int_status(int unit, const char *param_name, int value)
{
	char value_text[16];
	int length;

	if (!param_name || !param_name[0])
		return EINVAL;

	length = snprintf(value_text, sizeof(value_text), "%d", value);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(value_text))
		return EOVERFLOW;

	return set_wan_unit_value_status(unit, param_name, value_text);
}


static int
wan_nvram_transaction_get_callback(void *context, const char *param_name,
				   char **value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)context;

	if (!transaction) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_get(transaction, param_name, value);
}


static int
wan_nvram_transaction_begin_callback(void *context)
{
	if (!context) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_begin((struct nvram_transaction *)context);
}


static int
wan_nvram_transaction_end_callback(void *context)
{
	if (!context) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_end((struct nvram_transaction *)context);
}


static int
wan_dhcp_nvram_transaction_get_value(void *context, const char *param_name,
                                     const char **value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)context;
	char *stored_value = NULL;
	int api_result, saved_errno;

	if (!transaction || !param_name || !param_name[0] || !value)
		return EINVAL;
	*value = NULL;
	errno = 0;
	api_result = nvram_transaction_get(transaction, param_name, &stored_value);
	saved_errno = errno;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		*value = "";
		return 0;
	}
	if (api_result != 1 || !stored_value)
		return EIO;
	*value = stored_value;
	return 0;
}


#define WAN_DHCP_RENEW_ROUTE_TEXT_SIZE 2049

struct wan_dhcp_renew_nvram_state {
	char lease_error[16];
	char route_error[16];
	char mtu_error[16];
	char netmask[INET_ADDRSTRLEN];
	char gateway[INET_ADDRSTRLEN];
	char ipaddr[INET_ADDRSTRLEN];
	char routes[WAN_DHCP_RENEW_ROUTE_TEXT_SIZE];
	char routes_ms[WAN_DHCP_RENEW_ROUTE_TEXT_SIZE];
	char routes_rfc[WAN_DHCP_RENEW_ROUTE_TEXT_SIZE];
	char mtu[16];
	char dns[256];
	char domain[256];
	char wins[256];
	char lease[16];
};

/*
 * A DHCP renewal can skip the bind path only when the complete previous
 * lease/retry tuple was read successfully. Hold one NVRAM transaction so
 * backend errors cannot masquerade as empty values or mix concurrent states.
 */
static int
wan_dhcp_renew_state_snapshot(const char *prefix, int include_lease_options,
			      struct wan_dhcp_renew_nvram_state *state)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[14];
	char keys[14][32];
	unsigned int count = 0;
	int length;

	if (!prefix || !prefix[0] || !state ||
	    (include_lease_options != 0 && include_lease_options != 1))
		return EINVAL;
	memset(state, 0, sizeof(*state));

#define WAN_DHCP_RENEW_ADD_FIELD(suffix, member) do { \
	length = snprintf(keys[count], sizeof(keys[count]), "%s%s", \
			  prefix, suffix); \
	if (length < 0) \
		return errno ? errno : EIO; \
	if ((size_t)length >= sizeof(keys[count])) \
		return ENAMETOOLONG; \
	fields[count].param_name = keys[count]; \
	fields[count].destination = state->member; \
	fields[count].destination_size = sizeof(state->member); \
	++count; \
} while (0)

	WAN_DHCP_RENEW_ADD_FIELD("err", lease_error);
	WAN_DHCP_RENEW_ADD_FIELD("route_err", route_error);
	WAN_DHCP_RENEW_ADD_FIELD("mtu_err", mtu_error);
	WAN_DHCP_RENEW_ADD_FIELD("netmask", netmask);
	WAN_DHCP_RENEW_ADD_FIELD("gateway", gateway);
	WAN_DHCP_RENEW_ADD_FIELD("ipaddr", ipaddr);
	WAN_DHCP_RENEW_ADD_FIELD("routes", routes);
	WAN_DHCP_RENEW_ADD_FIELD("routes_ms", routes_ms);
	WAN_DHCP_RENEW_ADD_FIELD("routes_rfc", routes_rfc);
	WAN_DHCP_RENEW_ADD_FIELD("mtu", mtu);
	if (include_lease_options) {
		WAN_DHCP_RENEW_ADD_FIELD("dns", dns);
		WAN_DHCP_RENEW_ADD_FIELD("domain", domain);
		WAN_DHCP_RENEW_ADD_FIELD("wins", wins);
		WAN_DHCP_RENEW_ADD_FIELD("lease", lease);
	}

#undef WAN_DHCP_RENEW_ADD_FIELD

	return rc_wan_nvram_copy_snapshot(
		fields, count, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
}


static int
wan_static_ipv4_snapshot(int unit, const char *wan_addr_suffix,
			 const char *wan_mask_suffix,
			 char *wan_addr, size_t wan_addr_size,
			 char *wan_mask, size_t wan_mask_size,
			 char *lan_addr, size_t lan_addr_size,
			 char *lan_mask, size_t lan_mask_size)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[4];
	char wan_addr_key[64], wan_mask_key[64];
	int length;

	if (!wan_addr_suffix || !wan_addr_suffix[0] ||
	    !wan_mask_suffix || !wan_mask_suffix[0] ||
	    !wan_addr || !wan_addr_size || !wan_mask || !wan_mask_size ||
	    !lan_addr || !lan_addr_size || !lan_mask || !lan_mask_size)
		return EINVAL;

	length = snprintf(wan_addr_key, sizeof(wan_addr_key), "wan%d_%s",
			 unit, wan_addr_suffix);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(wan_addr_key))
		return ENAMETOOLONG;
	length = snprintf(wan_mask_key, sizeof(wan_mask_key), "wan%d_%s",
			 unit, wan_mask_suffix);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(wan_mask_key))
		return ENAMETOOLONG;

	fields[0].param_name = wan_addr_key;
	fields[0].destination = wan_addr;
	fields[0].destination_size = wan_addr_size;
	fields[1].param_name = wan_mask_key;
	fields[1].destination = wan_mask;
	fields[1].destination_size = wan_mask_size;
	fields[2].param_name = "lan_ipaddr";
	fields[2].destination = lan_addr;
	fields[2].destination_size = lan_addr_size;
	fields[3].param_name = "lan_netmask";
	fields[3].destination = lan_mask;
	fields[3].destination_size = lan_mask_size;

	return rc_wan_nvram_copy_snapshot(
		fields, 4, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
}


/*
 * WAN gateway, address and netmask must describe one NVRAM generation before
 * teardown can decide whether WAN-up owns an off-subnet gateway /32 route.
 */
static int
wan_unit_route_snapshot(int unit,
			char *gateway, size_t gateway_size,
			char *wan_addr, size_t wan_addr_size,
			char *wan_mask, size_t wan_mask_size)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[3];
	char gateway_key[64], address_key[64], netmask_key[64];
	int length;

	if (unit < 0 || !gateway || !gateway_size ||
	    !wan_addr || !wan_addr_size || !wan_mask || !wan_mask_size)
		return EINVAL;

	length = snprintf(gateway_key, sizeof(gateway_key),
			  "wan%d_gateway", unit);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(gateway_key))
		return ENAMETOOLONG;
	length = snprintf(address_key, sizeof(address_key),
			  "wan%d_ipaddr", unit);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(address_key))
		return ENAMETOOLONG;
	length = snprintf(netmask_key, sizeof(netmask_key),
			  "wan%d_netmask", unit);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(netmask_key))
		return ENAMETOOLONG;

	fields[0].param_name = gateway_key;
	fields[0].destination = gateway;
	fields[0].destination_size = gateway_size;
	fields[1].param_name = address_key;
	fields[1].destination = wan_addr;
	fields[1].destination_size = wan_addr_size;
	fields[2].param_name = netmask_key;
	fields[2].destination = wan_mask;
	fields[2].destination_size = wan_mask_size;

	return rc_wan_nvram_copy_snapshot(
		fields, 3, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
}


#define WAN_MAN_DNS_SNAPSHOT_MAX 512

static int
wan_man_gateway_snapshot(char *gateway, size_t gateway_size,
			 char *wan_addr, size_t wan_addr_size,
			 char *wan_mask, size_t wan_mask_size,
			 char *dns_servers, size_t dns_servers_size)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[4];

	if (!gateway || !gateway_size || !wan_addr || !wan_addr_size ||
	    !wan_mask || !wan_mask_size || !dns_servers || !dns_servers_size)
		return EINVAL;

	gateway[0] = '\0';
	wan_addr[0] = '\0';
	wan_mask[0] = '\0';
	dns_servers[0] = '\0';

	/*
	 * A failed read or transaction close invalidates the complete route
	 * snapshot; callers must never act on a partially refreshed tuple.
	 */
	fields[0].param_name = "wanx_gateway";
	fields[0].destination = gateway;
	fields[0].destination_size = gateway_size;
	fields[1].param_name = "wanx_ipaddr";
	fields[1].destination = wan_addr;
	fields[1].destination_size = wan_addr_size;
	fields[2].param_name = "wanx_netmask";
	fields[2].destination = wan_mask;
	fields[2].destination_size = wan_mask_size;
	fields[3].param_name = "wanx_dns";
	fields[3].destination = dns_servers;
	fields[3].destination_size = dns_servers_size;

	return rc_wan_nvram_copy_snapshot(
		fields, 4, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
}


int
set_wan_unit_error_status(int unit, int error)
{
	int status;

	status = set_wan_unit_value_int_status(unit, "err", error);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot publish WAN error state for unit %d (value=%d, status=%d)",
			   unit, error, status);
	return status;
}


static int
set_wan_unit_param_status(int unit, const char *param_name)
{
	struct nvram_transaction transaction;
	char wan_param[64], wanN_param[64], *source_value = NULL;
	int path_len, read_result, api_result, saved_errno;
	int status = 0, end_status;

	if (unit < 0 || !param_name || !param_name[0])
		return EINVAL;

	path_len = snprintf(wan_param, sizeof(wan_param), "wan_%s", param_name);
	if (path_len < 0)
		return errno ? errno : EIO;
	if ((size_t)path_len >= sizeof(wan_param))
		return ENAMETOOLONG;

	path_len = snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s",
			    unit, param_name);
	if (path_len < 0)
		return errno ? errno : EIO;
	if ((size_t)path_len >= sizeof(wanN_param))
		return ENAMETOOLONG;

	/* Read the shared setting and publish the WAN-unit copy under one lock. */
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;

	errno = 0;
	read_result = nvram_transaction_get(&transaction, wan_param, &source_value);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(read_result, saved_errno);
	if (status == 0 && read_result == 1 && !source_value)
		status = EIO;

	if (status == 0) {
		errno = 0;
		api_result = nvram_transaction_set(
			&transaction, wanN_param,
			(read_result == 1) ? source_value : "", 1);
		saved_errno = errno;
		status = rc_wan_nvram_write_status(api_result, saved_errno);
	}

	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
	}

	return status;
}

static int
set_wan_unit_param_status_callback(void *context, const char *param_name)
{
	int *unit = (int *)context;

	if (!unit)
		return EINVAL;
	return set_wan_unit_param_status(*unit, param_name);
}

void
set_wan_unit_value(int unit, const char* param_name, const char* value)
{
	int status;

	status = set_wan_unit_value_status(unit, param_name, value);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot publish WAN unit value %s for unit %d (status=%d)",
			   param_name ? param_name : "(null)", unit, status);
}

void
set_wan_unit_value_int(int unit, const char* param_name, int value)
{
	int status;

	status = set_wan_unit_value_int_status(unit, param_name, value);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot publish WAN unit integer %s for unit %d (status=%d)",
			   param_name ? param_name : "(null)", unit, status);
}

void
set_wan_unit_param(int unit, const char* param_name)
{
	int status;

	status = set_wan_unit_param_status(unit, param_name);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot copy WAN unit parameter %s for unit %d (status=%d)",
			   param_name ? param_name : "(null)", unit, status);
}


static int
set_wan_unit_value_status_callback(void *context, const char *param_name,
				   const char *value)
{
	int *unit = (int *)context;

	if (!unit)
		return EINVAL;
	return set_wan_unit_value_status(*unit, param_name, value);
}

static void
control_wan_led_isp_state(int is_wan_up, int is_modem_unit)
{
	int front_led_wan = nvram_get_int("front_led_wan");

	if (front_led_wan == 2) {
		int has_link = 0;
		int is_wisp_active = 0, wisp_has_link = 0, wisp_status = 0;

		if (is_wan_up) {
			has_link = 1;
			wisp_status = get_wan_wisp_active_status(
				&is_wisp_active, &wisp_has_link);
			if (wisp_status != 0)
				has_link = 0;
			else if (is_wisp_active)
				has_link = wisp_has_link;
			else if (!is_modem_unit)
				has_link = get_wan_ether_link_cached();
		}
		LED_CONTROL(LED_WAN, (is_wan_up && has_link) ? LED_ON : LED_OFF);
	} else if (front_led_wan == 3) {
		if (!is_wan_up)
			LED_CONTROL(LED_WAN, LED_OFF);
	}
}

static int
set_nvram_value_status(const char *name, const char *value)
{
	int api_result, saved_errno;

	if (!name || !name[0] || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_set_temp(name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
wan_deferred_reconnect_publish_nvram(void *context, const char *name,
                                     long value)
{
	char value_text[16];
	int length;

	(void)context;
	if (!name || !name[0] || value < 0)
		return EINVAL;
	if (value > INT_MAX)
		return EOVERFLOW;

	length = snprintf(value_text, sizeof(value_text), "%ld", value);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(value_text))
		return EOVERFLOW;

	return set_nvram_value_status(name, value_text);
}


static int
clear_deferred_wan_reconnect_status(void)
{
	return wan_deferred_reconnect_clear(
		wan_deferred_reconnect_publish_nvram, NULL);
}


static int
schedule_deferred_wan_reconnect_status(long now, long delay)
{
	return wan_deferred_reconnect_schedule(
		now, delay, wan_deferred_reconnect_publish_nvram, NULL);
}


static int
set_nvram_persistent_value_status(const char *name, const char *value)
{
	int api_result, saved_errno;

	if (!name || !name[0] || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_set(name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
set_nvram_named_value_status(void *context, const char *name)
{
	const char *value = (const char *)context;

	if (!value)
		return EINVAL;
	return set_nvram_value_status(name, value);
}


static int
set_nvram_snapshot_value_status(void *context, const char *param_name,
				const char *value)
{
	int api_result, saved_errno;

	(void)context;
	errno = 0;
	api_result = nvram_set_temp(param_name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}


static int
clear_wan_state_status(void)
{
	static const struct rc_wan_nvram_value stats_clear_values[] = {
		{ "err", "0" }, { "uptime", "0" }, { "dltime", "0" },
		{ "bytes_rx", "0" }, { "bytes_tx", "0" }
	};
	static const struct rc_wan_nvram_value vpn_runtime_clear_values[] = {
		{ "l2tp_wan_t", "0" }, { "vpnc_dns_t", "" },
		{ "vpnc_dom_t", "" }
	};
	int unit = 0, stats_status, runtime_status;

	/* Statistics are best-effort; do not let a counter block link teardown. */
	stats_status = rc_wan_nvram_set_all_values(
		stats_clear_values,
		(unsigned int)(sizeof(stats_clear_values) / sizeof(stats_clear_values[0])),
		set_wan_unit_value_status_callback, &unit);
	if (stats_status != 0)
		logmessage(LOGNAME,
			   "WAN statistics snapshot clear incomplete (status=%d)",
			   stats_status);

	/* These control DNS and the shared L2TP client/server lifecycle. */
	runtime_status = rc_wan_nvram_set_all_values(
		vpn_runtime_clear_values,
		(unsigned int)(sizeof(vpn_runtime_clear_values) / sizeof(vpn_runtime_clear_values[0])),
		set_nvram_snapshot_value_status, NULL);
	if (runtime_status != 0)
		logmessage(LOGNAME,
			   "WAN/VPN runtime state clear incomplete (status=%d)",
			   runtime_status);
	return runtime_status;
}

static int
reset_man_vars_status(void)
{
	static const char *const empty_params[] = {
		"wanx_ipaddr", "wanx_netmask", "wanx_gateway", "wanx_dns",
		"wanx_lease", "wanx_domain", "wanx_mtu", "wanx_routes",
		"wanx_routes_ms", "wanx_routes_rfc",
		"manv_ipaddr", "manv_netmask", "manv_gateway", "manv_mtu",
		"manv_routes", "manv_routes_ms", "manv_routes_rfc"
	};
	static const char *const zero_params[] = { "wanx_err", "manv_err" };
	int status;

	status = rc_wan_nvram_set_required_params(
		empty_params, (unsigned int)(sizeof(empty_params) / sizeof(empty_params[0])),
		set_nvram_named_value_status, "");
	if (status != 0)
		return status;

	return rc_wan_nvram_set_required_params(
		zero_params, (unsigned int)(sizeof(zero_params) / sizeof(zero_params[0])),
		set_nvram_named_value_status, "0");
}

void
reset_man_vars(void)
{
	int status = reset_man_vars_status();

	if (status != 0)
		logmessage(LOGNAME,
			   "MAN/virtual IPTV route snapshot clear incomplete (status=%d)",
			   status);
}

/*
 * Read the manual WAN endpoint as one checked NVRAM generation. These values
 * control the address assigned to the PPTP/L2TP/PPPoE underlay; a backend or
 * transaction-close error must not silently turn a configured endpoint into
 * four empty defaults.
 */
int
get_wan_unit_proto_checked_status(int unit, int *wan_proto)
{
	char proto_text[32];
	int status;

	if (!wan_proto)
		return EINVAL;
	status = get_wan_unit_value_copy_status(
		unit, "proto", proto_text, sizeof(proto_text));
	if (status != 0)
		return status;

	if (strcmp(proto_text, "pppoe") == 0)
		*wan_proto = IPV4_WAN_PROTO_PPPOE;
	else if (strcmp(proto_text, "pptp") == 0)
		*wan_proto = IPV4_WAN_PROTO_PPTP;
	else if (strcmp(proto_text, "l2tp") == 0)
		*wan_proto = IPV4_WAN_PROTO_L2TP;
	else if (strcmp(proto_text, "dhcp") == 0)
		*wan_proto = IPV4_WAN_PROTO_IPOE_DHCP;
	else if (strcmp(proto_text, "static") == 0)
		*wan_proto = IPV4_WAN_PROTO_IPOE_STATIC;
	else
		return EINVAL;

	return 0;
}

/*
 * is_ifunit_man() has a boolean ABI. On protocol read errors, classify the
 * unit conservatively as manual PPP so DHCP cleanup cannot tear down an
 * underlay whose protocol is temporarily unreadable.
 */
static int
get_wan_unit_proto_for_manual_check(int unit)
{
	int wan_proto, status;

	status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot classify WAN unit %d protocol (status=%d); preserving manual underlay",
			   unit, status);
		return -1;
	}
	return wan_proto;
}



static int
wan_hardware_address_copy_status(char *hardware_address,
				 size_t hardware_address_size)
{
	char *custom_mac = NULL;
	char formatted_mac[18];
	char factory_mac[64];
	int api_result, saved_errno, status;

	if (!hardware_address || hardware_address_size == 0)
		return EINVAL;
	hardware_address[0] = '\0';

	errno = 0;
	api_result = wan_nvram_checked_get_callback(
		NULL, "wan_hwaddr_x", &custom_mac);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 1) {
		if (!custom_mac)
			return EIO;
		status = rc_wan_nvram_format_mac_override(
			custom_mac, formatted_mac, sizeof(formatted_mac));
		if (status != 0)
			return status;
		if (formatted_mac[0] != '\0')
			return rc_wan_nvram_copy_text(
				formatted_mac, hardware_address,
				hardware_address_size);
	}

	/* Missing/invalid/zero/broadcast override retains il1macaddr fallback. */
	status = rc_wan_nvram_copy_value(
		wan_nvram_checked_get_callback, NULL, "il1macaddr",
		factory_mac, sizeof(factory_mac));
	if (status != 0)
		return status;
	return rc_wan_nvram_copy_text(
		factory_mac, hardware_address, hardware_address_size);
}


static int
wan_manual_endpoint_snapshot(char *wan_addr, size_t wan_addr_size,
			     char *wan_mask, size_t wan_mask_size,
			     char *wan_gateway, size_t wan_gateway_size,
			     char *wan_mtu, size_t wan_mtu_size)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[4];

	if (!wan_addr || !wan_addr_size || !wan_mask || !wan_mask_size ||
	    !wan_gateway || !wan_gateway_size || !wan_mtu || !wan_mtu_size)
		return EINVAL;

	fields[0].param_name = "wan_ipaddr";
	fields[0].destination = wan_addr;
	fields[0].destination_size = wan_addr_size;
	fields[1].param_name = "wan_netmask";
	fields[1].destination = wan_mask;
	fields[1].destination_size = wan_mask_size;
	fields[2].param_name = "wan_gateway";
	fields[2].destination = wan_gateway;
	fields[2].destination_size = wan_gateway_size;
	fields[3].param_name = "wan_mtu";
	fields[3].destination = wan_mtu;
	fields[3].destination_size = wan_mtu_size;

	return rc_wan_nvram_copy_snapshot(
		fields, 4, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
}


static int wan_vars_reset_status;

static int
reset_wan_vars_status(void)
{
	int unit, wan_proto, proto_status;
	int manual_endpoint_enabled = 0, manual_pppoe_enabled = 0;
	char mac_buf[24];
	char man_addr[INET_ADDRSTRLEN] = "0.0.0.0";
	char man_mask[INET_ADDRSTRLEN] = "0.0.0.0";
	char man_gate[INET_ADDRSTRLEN] = "0.0.0.0";
	char man_mtu[16] = "";
	const char *wan_hwaddr;
	const char *reset_failure_context = NULL;
	static const char *const dhcp_ppp_params[] = {
		"dnsenable_x", "vci", "hostname"
	};
	static const char *const auth_params[] = {
		"auth_mode", "auth_user", "auth_pass"
	};
	static const char *const static_ipoe_params[] = {
		"ipaddr", "netmask", "gateway", "mtu"
	};
	static const char *const ppp_credential_params[] = {
		"pppoe_username", "pppoe_passwd"
	};
	static const char *const ppp_option_params[] = {
		"pppoe_txonly_x", "pppoe_service", "pppoe_ac", "pppoe_man",
		"pppoe_mtu", "pppoe_mru", "pptp_mtu", "pptp_mru",
		"l2tp_mtu", "l2tp_mru", "ppp_peer", "ppp_auth", "ppp_mppe",
		"ppp_lcp", "ppp_alcp", "ppp_pppd"
	};
	static const struct rc_wan_nvram_value runtime_clear_values[] = {
		{ "dns", "" }, { "wins", "" }, { "lease", "" },
		{ "domain", "" }, { "routes", "" }, { "routes_ms", "" },
		{ "routes_rfc", "" }, { "err", "0" }
	};
	static const struct rc_wan_nvram_value nonstatic_address_clear_values[] = {
		{ "ipaddr", "0.0.0.0" }, { "netmask", "0.0.0.0" },
		{ "gateway", "0.0.0.0" }, { "mtu", "" }
	};


	wan_vars_reset_status = 0;
	unit = 0;

	proto_status = clear_wan_state_status();
	if (proto_status != 0) {
		reset_failure_context = "WAN/VPN runtime state";
		goto reset_failed;
	}
	proto_status = reset_man_vars_status();
	if (proto_status != 0) {
		reset_failure_context = "MAN/virtual IPTV route snapshot";
		goto reset_failed;
	}

	/*
	 * get_wan_proto() reads wanN_proto, so a failed copy from wan_proto
	 * must not leave a previous protocol selected for this WAN restart.
	 */
	proto_status = set_wan_unit_param_status(unit, "proto");
	if (proto_status != 0) {
		wan_vars_reset_status = proto_status;
		errno = proto_status;
		logmessage(LOGNAME,
			   "cannot stage configured WAN protocol before reset (status=%d)",
			   proto_status);
		return proto_status;
	}
	proto_status = rc_wan_nvram_set_required_params(
		dhcp_ppp_params,
		(unsigned int)(sizeof(dhcp_ppp_params) / sizeof(dhcp_ppp_params[0])),
		set_wan_unit_param_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN DHCP/DNS client options";
		goto reset_failed;
	}

	proto_status = rc_wan_nvram_set_required_params(
		auth_params, (unsigned int)(sizeof(auth_params) / sizeof(auth_params[0])),
		set_wan_unit_param_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN authentication mode and credentials";
		goto reset_failed;
	}


	proto_status = rc_wan_nvram_set_required_values(
		runtime_clear_values,
		(unsigned int)(sizeof(runtime_clear_values) / sizeof(runtime_clear_values[0])),
		set_wan_unit_value_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN runtime DNS and route snapshot";
		goto reset_failed;
	}


	proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (proto_status != 0) {
		reset_failure_context = "WAN protocol snapshot";
		goto reset_failed;
	}

	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
		proto_status = rc_wan_nvram_set_required_params(
			static_ipoe_params,
			(unsigned int)(sizeof(static_ipoe_params) /
				       sizeof(static_ipoe_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "static IPoE address settings";
			goto reset_failed;
		}
	} else {

		proto_status = rc_wan_nvram_set_required_values(
			nonstatic_address_clear_values,
			(unsigned int)(sizeof(nonstatic_address_clear_values) /
				       sizeof(nonstatic_address_clear_values[0])),
			set_wan_unit_value_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "non-static WAN address snapshot";
			goto reset_failed;
		}

	}



	proto_status = get_nvram_match_checked_status(
		"x_DHCPClient", "0", &manual_endpoint_enabled);
	if (proto_status != 0) {
		reset_failure_context = "manual WAN address-mode selector";
		goto reset_failed;
	}
	if (manual_endpoint_enabled) {
		if (wan_proto == IPV4_WAN_PROTO_PPPOE) {
			proto_status = get_nvram_match_checked_status(
				"wan_pppoe_man", "1", &manual_pppoe_enabled);
			if (proto_status != 0) {
				reset_failure_context = "manual PPPoE selector";
				goto reset_failed;
			}
			manual_endpoint_enabled = manual_pppoe_enabled;
		} else if (wan_proto != IPV4_WAN_PROTO_PPTP &&
			   wan_proto != IPV4_WAN_PROTO_L2TP) {
			manual_endpoint_enabled = 0;
		}
	}
	if (manual_endpoint_enabled) {
		proto_status = wan_manual_endpoint_snapshot(
			man_addr, sizeof(man_addr), man_mask, sizeof(man_mask),
			man_gate, sizeof(man_gate), man_mtu, sizeof(man_mtu));
		if (proto_status != 0) {
			reset_failure_context = "manual WAN endpoint snapshot";
			goto reset_failed;
		}
	}

	proto_status = set_wan_unit_value_status(unit, "man_ipaddr", man_addr);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_netmask", man_mask);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_gateway", man_gate);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_mtu", man_mtu);
	if (proto_status != 0) {
		reset_failure_context = "manual WAN endpoint settings";
		goto reset_failed;
	}

	if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
	    wan_proto == IPV4_WAN_PROTO_PPTP ||
	    wan_proto == IPV4_WAN_PROTO_L2TP) {
		proto_status = set_nvram_value_status("wanx_ipaddr", man_addr);
		if (proto_status == 0 && is_valid_ipv4(man_addr) &&
		    is_valid_ipv4(man_mask))
			proto_status = set_nvram_value_status("wanx_netmask", man_mask);
		if (proto_status == 0 && is_valid_ipv4(man_gate))
			proto_status = set_nvram_value_status("wanx_gateway", man_gate);
		if (proto_status != 0) {
			reset_failure_context = "global MAN route endpoint snapshot";
			goto reset_failed;
		}
		
		proto_status = rc_wan_nvram_set_required_params(
			ppp_credential_params,
			(unsigned int)(sizeof(ppp_credential_params) /
				       sizeof(ppp_credential_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "PPP authentication credentials";
			goto reset_failed;
		}

		if (wan_proto == IPV4_WAN_PROTO_PPPOE)
			proto_status = set_wan_unit_param_status(unit, "pppoe_idletime");
		else
			proto_status = set_wan_unit_value_status(
				unit, "pppoe_idletime", "0");
		if (proto_status != 0) {
			reset_failure_context = "PPP idle-time setting";
			goto reset_failed;
		}

		proto_status = rc_wan_nvram_set_required_params(
			ppp_option_params,
			(unsigned int)(sizeof(ppp_option_params) /
				       sizeof(ppp_option_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "PPP peer and negotiation options";
			goto reset_failed;
		}
	}

	proto_status = wan_hardware_address_copy_status(
		mac_buf, sizeof(mac_buf));
	if (proto_status != 0) {
		reset_failure_context = "WAN hardware address configuration";
		goto reset_failed;
	}
	wan_hwaddr = mac_buf;
	proto_status = set_nvram_persistent_value_status("wan_hwaddr", wan_hwaddr);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "hwaddr", wan_hwaddr);
	if (proto_status != 0) {
		reset_failure_context = "WAN interface hardware address";
		goto reset_failed;
	}

#if defined (USE_IPV6)
	proto_status = reset_wan6_vars_status();
	if (proto_status != 0) {
		reset_failure_context = "IPv6 WAN runtime snapshot";
		goto reset_failed;
	}
#endif
	return 0;

reset_failed:
	wan_vars_reset_status = proto_status;
	errno = proto_status;
	logmessage(LOGNAME,
		   "cannot stage %s before WAN activation (status=%d)",
		   reset_failure_context ? reset_failure_context : "required WAN settings",
		   proto_status);
	return proto_status;
}

void
reset_wan_vars(void)
{
	(void)reset_wan_vars_status();
}

/*
 * PID 1 must observe the result of the legacy void reset entry point without
 * exporting the internal reset implementation or changing its public ABI.
 */
int
get_wan_vars_reset_status(void)
{
	return wan_vars_reset_status;
}

int
get_vlan_vid_wan(void)
{
	int vlan_vid;
	int is_vlan_filter = 0;
	int status;

	status = get_nvram_match_checked_status(
		"vlan_filter", "1", &is_vlan_filter);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN VLAN filter selector for Hardware NAT (status=%d)",
			   status);
		errno = status;
		return wan_vlan_vid_policy_read_failure_result(status);
	}

	if (is_vlan_filter) {
		status = get_nvram_int_checked_status("vlan_vid_cpu", &vlan_vid);
		if (status != 0) {
			logmessage(LOGNAME,
				   "cannot read configured WAN VLAN ID for Hardware NAT (status=%d)",
				   status);
			errno = status;
			return wan_vlan_vid_policy_read_failure_result(status);
		}
	} else {
		vlan_vid = 0;
	}

	if (!is_vlan_vid_valid(vlan_vid)) {
		vlan_vid = phy_vlan_pvid_wan_get();
		if (vlan_vid < 0)
			return vlan_vid;
		if (!is_vlan_vid_valid(vlan_vid))
			return -EINVAL;
	}

	return vlan_vid;
}

static int
get_wan_bridge_mode_status(int *bridge_mode)
{
	int configured_mode = 0;
	int status;

	if (!bridge_mode)
		return EINVAL;
	status = get_nvram_int_checked_status("wan_stb_x", &configured_mode);
	return rc_wan_nvram_int_range_status(status, configured_mode, 0, 7,
					     SWAPI_WAN_BRIDGE_DISABLE,
					     bridge_mode);
}

int
get_wan_bridge_mode(void)
{
	int bridge_mode = SWAPI_WAN_BRIDGE_DISABLE;
	int status;

	status = get_wan_bridge_mode_status(&bridge_mode);
	if (status != 0) {
		errno = status;
		return SWAPI_WAN_BRIDGE_DISABLE;
	}
	return bridge_mode;
}

static int
get_wan_bridge_iso_mode_status(int *bwan_isolation)
{
	int configured_isolation = 0;
	int status;

	if (!bwan_isolation)
		return EINVAL;
	status = get_nvram_int_checked_status("wan_stb_iso",
					     &configured_isolation);
	return rc_wan_nvram_int_range_status(status, configured_isolation,
					     0, 2,
					     SWAPI_WAN_BWAN_ISOLATION_NONE,
					     bwan_isolation);
}

int
get_wan_bridge_iso_mode(int bridge_mode)
{
	int bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;
	int status;

	if (bridge_mode == SWAPI_WAN_BRIDGE_DISABLE)
		return SWAPI_WAN_BWAN_ISOLATION_NONE;
	status = get_wan_bridge_iso_mode_status(&bwan_isolation);
	if (status != 0) {
		errno = status;
		return SWAPI_WAN_BWAN_ISOLATION_NONE;
	}
	return bwan_isolation;
}

struct soft_bridge_wan_ownership {
	char bridge_name[16];
	char wan_ifname[24];
	char bwan_ifname[24];
	int bridge_created;
	int bridge_existed;
	int bridge_was_up;
	int bwan_vlan_created;
	int bwan_vlan_existed;
	int bwan_vlan_was_up;
	int wan_port_attach_attempted;
	int wan_port_added;
	int wan_port_detached_safe;
	int bwan_port_attach_attempted;
	int bwan_port_added;
	int bwan_port_detached_safe;
};

struct soft_bridge_other_entry {
	char port_ifname[24];
	int port_vlan_created;
	int port_vlan_existed;
	int port_vlan_was_up;
	struct soft_bridge_wan_ownership bridge;
};

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)

#define BRPREFIX_WAN	"brwan"

static int
config_soft_bridge_lan(void)
{
	char lan_ifname[24];
	char lan_hwaddr[18] = {0};
	const char *wired_ifname;
	int igmp_snoop, wired_m2u;
	int ret, saved_errno, cleanup_ret;
	int bridge_mode, bridge_enabled, vlan_flags, vlan_exists, vlan_was_up;
	int vlan_created = 0;
	int physical_detached = 0, vlan_attached = 0;
	int vlan_detached = 0, physical_attached = 0;
	int vlan_attach_attempted = 0;
	int vlan_attach_ambiguous = 0, physical_attach_ambiguous = 0;
	int vlan_state_safe = 1, physical_state_safe = 1;
	int vlan_detached_safe = 0;

	ret = get_nvram_int_checked_status("ether_igmp", &igmp_snoop);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read LAN bridge IGMP snooping setting before topology changes (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}
	ret = get_nvram_int_checked_status("ether_m2u", &wired_m2u);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read LAN bridge multicast-to-unicast setting before topology changes (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}

	snprintf(lan_ifname, sizeof(lan_ifname), "%s.%d", IFNAME_MAC, 1);
	ret = get_wan_bridge_mode_status(&bridge_mode);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN bridge mode before LAN topology changes (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}
	bridge_enabled = bridge_mode != SWAPI_WAN_BRIDGE_DISABLE;

	if (bridge_enabled) {
		ret = rc_wan_nvram_copy_value(
			wan_nvram_checked_get_callback, NULL, "lan_hwaddr",
			lan_hwaddr, sizeof(lan_hwaddr));
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read LAN MAC override before bridge topology changes (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
	}

	/*
	 * Snapshot the VLAN before changing it.  The tracked creator publishes
	 * ownership only after its full setup succeeds; a reused VLAN must retain
	 * its original administrative state when the bridge transaction rolls
	 * back.
	 */
	vlan_flags = get_interface_flags(lan_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO)
		return -1;
	vlan_exists = vlan_flags >= 0;
	vlan_was_up = vlan_exists && ((vlan_flags & IFF_UP) != 0);

	if (bridge_enabled) {
		ret = create_vlan_iface_tracked(IFNAME_MAC, 1, -1, -1,
						lan_hwaddr, 1, &vlan_created);
		if (ret != 0)
			goto rollback;
#if defined (USE_IPV6)
		control_if_ipv6(lan_ifname, 0);
#endif
		ret = br_add_del_if_tracked(IFNAME_BR, IFNAME_MAC, 0,
					    &physical_detached);
		if (ret != 0)
			goto rollback;
		vlan_attach_attempted = 1;
		ret = br_add_del_if_tracked(IFNAME_BR, lan_ifname, 1,
					    &vlan_attached);
		if (ret != 0) {
			vlan_attach_ambiguous = 1;
			goto rollback;
		}
		wired_ifname = lan_ifname;
	} else {
		if (vlan_exists) {
			ret = br_add_del_if_tracked(IFNAME_BR, lan_ifname, 0,
						    &vlan_detached);
			if (ret != 0)
				goto rollback;
		}
		ret = br_add_del_if_tracked(IFNAME_BR, IFNAME_MAC, 1,
					    &physical_attached);
		if (ret != 0) {
			physical_attach_ambiguous = 1;
			goto rollback;
		}
		wired_ifname = IFNAME_MAC;
	}

	ret = brport_set_m2u(wired_ifname,
			     (igmp_snoop && wired_m2u == 1) ? 1 : 0);
	if (ret != 0)
		goto rollback;

	/* Delete the old VLAN only after the replacement port is fully ready. */
	if (!bridge_enabled && vlan_exists) {
		ret = remove_vlan_iface(lan_ifname);
		if (ret != 0)
			goto rollback;
	}

	return 0;

rollback:
	saved_errno = errno;
	if (bridge_enabled) {
		/*
		 * Undo only a VLAN membership created by this call.  A successful
		 * idempotent add (changed_out == 0) belongs to the pre-existing
		 * topology and must not be detached or deleted here.
		 */
		if (vlan_attached) {
			cleanup_ret = br_add_del_if(IFNAME_BR, lan_ifname, 0);
			if (cleanup_ret == 0)
				vlan_detached_safe = 1;
			else {
				vlan_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not detach %s (status=%d)",
					   lan_ifname, cleanup_ret);
			}
		} else if (!vlan_attach_attempted) {
			vlan_detached_safe = 1;
		}

		if (vlan_attach_ambiguous)
			vlan_state_safe = 0;

		/* Restore the physical port only after the new VLAN port is safe. */
		if (physical_detached && vlan_state_safe) {
			cleanup_ret = br_add_del_if(IFNAME_BR, IFNAME_MAC, 1);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore %s (status=%d)",
					   IFNAME_MAC, cleanup_ret);
		}

		if (bridge_rollback_can_remove_created_vlan(vlan_created,
							    vlan_detached_safe)) {
			cleanup_ret = remove_vlan_iface(lan_ifname);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not remove new VLAN %s (status=%d)",
					   lan_ifname, cleanup_ret);
		} else if (bridge_rollback_can_restore_interface(vlan_exists,
								 vlan_state_safe)) {
			cleanup_ret = vlan_was_up ?
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(lan_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore VLAN %s state (status=%d)",
					   lan_ifname, cleanup_ret);
		}
	} else {
		/*
		 * On the reverse transition, remove only the physical membership
		 * introduced here before restoring the detached VLAN membership.
		 */
		if (physical_attached) {
			cleanup_ret = br_add_del_if(IFNAME_BR, IFNAME_MAC, 0);
			if (cleanup_ret != 0) {
				physical_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not detach %s (status=%d)",
					   IFNAME_MAC, cleanup_ret);
			}
		}
		if (physical_attach_ambiguous)
			physical_state_safe = 0;

		if (vlan_detached && physical_state_safe) {
			cleanup_ret = br_add_del_if(IFNAME_BR, lan_ifname, 1);
			if (cleanup_ret != 0) {
				vlan_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not restore %s membership (status=%d)",
					   lan_ifname, cleanup_ret);
			}
		}
		if (vlan_exists && physical_state_safe && vlan_state_safe) {
			cleanup_ret = vlan_was_up ?
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(lan_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore VLAN %s state (status=%d)",
					   lan_ifname, cleanup_ret);
		}
	}

	logmessage(LOGNAME, "LAN bridge transition failed (status=%d); rollback attempted",
		   ret);
	errno = saved_errno;
	return ret;
}

static int
disassembly_bridges_wan(void)
{
	int i, ret, status = 0;
	char br_ifname[16];

	/* disassembly all wan bridges brwan5..brwan0 */
	for (i = 5; i >= 0; i--) {
		snprintf(br_ifname, sizeof(br_ifname), "%s%d", BRPREFIX_WAN, i);
		if (!is_interface_exist(br_ifname))
			continue;

		ret = ifconfig(br_ifname, 0, NULL, NULL);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0)
			logmessage(LOGNAME, "WAN bridge %s shutdown failed (status=%d)",
				   br_ifname, ret);

		ret = br_del_all_ifs(br_ifname, 1, 1);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s port teardown incomplete (status=%d)",
				   br_ifname, ret);
			continue;
		}

		ret = br_add_del_bridge(br_ifname, 0);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0)
			logmessage(LOGNAME, "WAN bridge %s removal failed (status=%d)",
				   br_ifname, ret);
	}

	return status;
}



static int
config_soft_bridge_wan_internal(int br_id, int vid, int prio, const char *wan_ifname,
				const char *wan_hwaddr,
				struct soft_bridge_wan_ownership *ownership_out)
{
	struct soft_bridge_wan_ownership local_ownership;
	struct soft_bridge_wan_ownership *ownership =
		ownership_out ? ownership_out : &local_ownership;
	char br_ifname[16], bwan_ifname[24];
	int ret, saved_errno, cleanup_ret, bridge_empty;
	int bridge_flags, bridge_created = 0, bridge_was_up = 0;
	int bridge_remove_safe = 1;
	int vlan_flags = -1, vlan_exists = 0, vlan_was_up = 0;
	int vlan_created = 0, vlan_remove_safe = 1, vlan_restore_safe = 1;
	int wan_attach_attempted = 0, wan_port_added = 0;
	int bwan_attach_attempted = 0, bwan_port_added = 0;
	int bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;

	if (br_id < 2) {
		ret = get_wan_bridge_iso_mode_status(&bwan_isolation);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN bridge isolation mode before topology changes (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
	}

	memset(ownership, 0, sizeof(*ownership));
	ownership->wan_port_detached_safe = 1;
	ownership->bwan_port_detached_safe = 1;
	bwan_ifname[0] = '\0';
	snprintf(br_ifname, sizeof(br_ifname), "%s%d", BRPREFIX_WAN, br_id);
	snprintf(ownership->bridge_name, sizeof(ownership->bridge_name), "%s", br_ifname);
	snprintf(ownership->wan_ifname, sizeof(ownership->wan_ifname), "%s", wan_ifname);
	bridge_flags = get_interface_flags(br_ifname);
	if (bridge_flags < 0) {
		if (errno != ENODEV && errno != ENXIO)
			return -1;
		ret = br_add_del_bridge(br_ifname, 1);
		if (ret != 0)
			return ret;
		bridge_created = 1;
		ownership->bridge_created = 1;
	} else {
		ownership->bridge_existed = 1;
		bridge_was_up = (bridge_flags & IFF_UP) != 0;
		ownership->bridge_was_up = bridge_was_up;
		ret = ifconfig(br_ifname, 0, NULL, NULL);
		if (ret != 0)
			goto rollback;
	}

	if (bridge_created) {
		ret = br_set_stp(br_ifname, 0);
		if (ret != 0)
			goto rollback;
		ret = br_set_fd(br_ifname, 2);
		if (ret != 0)
			goto rollback;
		ret = br_set_param_int(br_ifname, "multicast_snooping", 0);
		if (ret != 0)
			goto rollback;
	}
	ret = set_interface_hwaddr(br_ifname, wan_hwaddr);
	if (ret != 0)
		goto rollback;
	ret = set_interface_mtu(br_ifname, 1500);
	if (ret != 0)
		goto rollback;

	snprintf(bwan_ifname, sizeof(bwan_ifname), "%s.%d", IFNAME_MAC, vid);
	vlan_flags = get_interface_flags(bwan_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO) {
		ret = -1;
		goto rollback;
	}
	vlan_exists = vlan_flags >= 0;
	vlan_was_up = vlan_exists && ((vlan_flags & IFF_UP) != 0);
	ownership->bwan_vlan_existed = vlan_exists;
	ownership->bwan_vlan_was_up = vlan_was_up;
	snprintf(ownership->bwan_ifname, sizeof(ownership->bwan_ifname), "%s", bwan_ifname);
	ret = create_vlan_iface_tracked(IFNAME_MAC, vid, prio, -1,
					wan_hwaddr, 1, &vlan_created);
	ownership->bwan_vlan_created = vlan_created;
	if (ret != 0)
		goto rollback;

#if defined (USE_IPV6)
	control_if_ipv6(wan_ifname, 0);
	control_if_ipv6(bwan_ifname, 0);
#endif
	wan_attach_attempted = 1;
	ownership->wan_port_attach_attempted = 1;
	ownership->wan_port_detached_safe = 0;
	ret = br_add_del_if_tracked(br_ifname, wan_ifname, 1, &wan_port_added);
	ownership->wan_port_added = wan_port_added;
	if (ret != 0)
		goto rollback;
	bwan_attach_attempted = 1;
	ownership->bwan_port_attach_attempted = 1;
	ownership->bwan_port_detached_safe = 0;
	ret = br_add_del_if_tracked(br_ifname, bwan_ifname, 1, &bwan_port_added);
	ownership->bwan_port_added = bwan_port_added;
	if (ret != 0)
		goto rollback;
	if (br_id < 2) {
		if (bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
			ret = brport_set_param_int(wan_ifname, "isolate_mode", 1);
			if (ret != 0)
				goto rollback;
			ret = brport_set_param_int(bwan_ifname, "isolate_mode", 1);
			if (ret != 0)
				goto rollback;
		}
	} else {
#if defined (USE_IPV6)
		control_if_ipv6(br_ifname, 0);
#endif
	}
	ret = ifconfig(br_ifname, IFUP, "0.0.0.0", NULL);
	if (ret != 0)
		goto rollback;
	return 0;

rollback:
	saved_errno = errno;
	/* Detach only memberships actually added by this invocation. */
	if (bwan_port_added) {
		cleanup_ret = br_add_del_if(br_ifname, bwan_ifname, 0);
		if (cleanup_ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s rollback could not detach %s (status=%d)",
				   br_ifname, bwan_ifname, cleanup_ret);
			vlan_remove_safe = 0;
			vlan_restore_safe = 0;
			bridge_remove_safe = 0;
		} else {
			ownership->bwan_port_detached_safe = 1;
		}
	} else if (bwan_attach_attempted) {
		ownership->bwan_port_detached_safe = 0;
	}
	if (wan_port_added) {
		cleanup_ret = br_add_del_if(br_ifname, wan_ifname, 0);
		if (cleanup_ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s rollback could not detach %s (status=%d)",
				   br_ifname, wan_ifname, cleanup_ret);
			bridge_remove_safe = 0;
		} else {
			ownership->wan_port_detached_safe = 1;
		}
	} else if (wan_attach_attempted) {
		ownership->wan_port_detached_safe = 0;
	}
	/*
	 * A successful idempotent result or a raced attach is not ours to undo.
	 * Preserve the VLAN/bridge when ownership of an attempted attach is unclear.
	 */
	if (bwan_attach_attempted && !bwan_port_added) {
		vlan_remove_safe = 0;
		if (bridge_created)
			bridge_remove_safe = 0;
	}
	if (wan_attach_attempted && !wan_port_added && bridge_created)
		bridge_remove_safe = 0;
	bridge_remove_safe = bridge_remove_safe &&
		ownership->wan_port_detached_safe &&
		ownership->bwan_port_detached_safe;

	if (vlan_created) {
		if (vlan_remove_safe) {
			cleanup_ret = remove_vlan_iface(bwan_ifname);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not remove VLAN %s (status=%d)",
					   br_ifname, bwan_ifname, cleanup_ret);
		} else {
			logmessage(LOGNAME, "WAN bridge %s rollback preserved VLAN %s because attach ownership is uncertain",
				   br_ifname, bwan_ifname);
		}
	} else if (vlan_exists && vlan_restore_safe) {
		cleanup_ret = vlan_was_up ?
			ifconfig(bwan_ifname, IFUP, "0.0.0.0", NULL) :
			ifconfig(bwan_ifname, 0, NULL, NULL);
		if (cleanup_ret != 0)
			logmessage(LOGNAME, "WAN bridge %s rollback could not restore VLAN %s state (status=%d)",
				   br_ifname, bwan_ifname, cleanup_ret);
	}

	if (bridge_created) {
		bridge_empty = br_bridge_is_empty(br_ifname);

		if (bridge_remove_safe &&
		    bridge_batch_can_remove_created_bridge(bridge_created, bridge_empty)) {
			cleanup_ret = ifconfig(br_ifname, 0, NULL, NULL);
			if (cleanup_ret == 0)
				cleanup_ret = br_add_del_bridge(br_ifname, 0);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not remove new bridge (status=%d)",
					   br_ifname, cleanup_ret);
		} else {
			cleanup_ret = ifconfig(br_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not keep incomplete bridge down (status=%d)",
					   br_ifname, cleanup_ret);
			logmessage(LOGNAME, "WAN bridge %s retained because rollback could not prove ownership of all ports",
				   br_ifname);
		}
	} else if (bridge_was_up) {
		cleanup_ret = ifconfig(br_ifname, IFUP, "0.0.0.0", NULL);
		if (cleanup_ret != 0)
			logmessage(LOGNAME, "WAN bridge %s rollback could not restore its previous UP state (status=%d)",
				   br_ifname, cleanup_ret);
	}

	logmessage(LOGNAME, "WAN bridge %s setup failed (status=%d); rollback attempted",
		   br_ifname, ret);
	errno = saved_errno;
	return ret;
}

static int
config_soft_bridge_wan(int br_id, int vid, int prio, const char *wan_ifname,
		       const char *wan_hwaddr)
{
	return config_soft_bridge_wan_internal(br_id, vid, prio, wan_ifname,
					       wan_hwaddr, NULL);
}
static int
rollback_soft_bridge_other_entry(struct soft_bridge_other_entry *entry)
{
	struct soft_bridge_wan_ownership *bridge = &entry->bridge;
	int ret, bridge_flags, bridge_empty, bridge_empty_errno, status = 0;
	int wan_detached_safe, bwan_detached_safe;

	wan_detached_safe = bridge_batch_membership_detached_safe(
		bridge->wan_port_attach_attempted, bridge->wan_port_added,
		bridge->wan_port_detached_safe);
	bwan_detached_safe = bridge_batch_membership_detached_safe(
		bridge->bwan_port_attach_attempted, bridge->bwan_port_added,
		bridge->bwan_port_detached_safe);

	/* Reverse attachment order: WAN-side VLAN port was attached last. */
	if (bridge->bwan_port_added && !bwan_detached_safe) {
		ret = br_add_del_if(bridge->bridge_name, bridge->bwan_ifname, 0);
		bwan_detached_safe = (ret == 0);
		if (bwan_detached_safe)
			bridge->bwan_port_detached_safe = 1;
		if (ret != 0) {
			status = route_status_accumulate(status, ret);
			logmessage(LOGNAME, "LAN bridge batch rollback could not detach %s (status=%d)",
				   bridge->bwan_ifname, ret);
		}
	}
	if (bridge->wan_port_added && !wan_detached_safe) {
		ret = br_add_del_if(bridge->bridge_name, bridge->wan_ifname, 0);
		wan_detached_safe = (ret == 0);
		if (wan_detached_safe)
			bridge->wan_port_detached_safe = 1;
		if (ret != 0) {
			status = route_status_accumulate(status, ret);
			logmessage(LOGNAME, "LAN bridge batch rollback could not detach %s (status=%d)",
				   bridge->wan_ifname, ret);
		}
	}

	/* Restore/remove VLANs only after their bridge membership is known safe. */
	if (bridge->bwan_vlan_created) {
		if (bwan_detached_safe) {
			ret = remove_vlan_iface(bridge->bwan_ifname);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not remove WAN-side VLAN %s (status=%d)",
					   bridge->bwan_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s because membership ownership is uncertain",
				   bridge->bwan_ifname);
		}
	} else if (bridge->bwan_vlan_existed) {
		if (bwan_detached_safe) {
			ret = bridge->bwan_vlan_was_up ?
				ifconfig(bridge->bwan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(bridge->bwan_ifname, 0, NULL, NULL);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not restore VLAN %s state (status=%d)",
					   bridge->bwan_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s state because membership ownership is uncertain",
				   bridge->bwan_ifname);
		}
	}

	if (entry->port_vlan_created) {
		if (wan_detached_safe) {
			ret = remove_vlan_iface(entry->port_ifname);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not remove VLAN %s (status=%d)",
					   entry->port_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s because membership ownership is uncertain",
				   entry->port_ifname);
		}
	} else if (entry->port_vlan_existed) {
		if (wan_detached_safe) {
			ret = entry->port_vlan_was_up ?
				ifconfig(entry->port_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(entry->port_ifname, 0, NULL, NULL);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not restore VLAN %s state (status=%d)",
					   entry->port_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s state because membership ownership is uncertain",
				   entry->port_ifname);
		}
	}

	if (bridge->bridge_name[0]) {
		bridge_flags = get_interface_flags(bridge->bridge_name);
		if (bridge->bridge_created) {
			if (bridge_flags >= 0) {
				bridge_empty = br_bridge_is_empty(bridge->bridge_name);
				bridge_empty_errno = bridge_empty < 0 ? (errno ? errno : EIO) : 0;
				if (bridge_batch_can_remove_created_bridge(1, bridge_empty) &&
				    wan_detached_safe && bwan_detached_safe) {
					ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
					if (ret == 0)
						ret = br_add_del_bridge(bridge->bridge_name, 0);
					if (ret != 0) {
						status = route_status_accumulate(status, ret);
						logmessage(LOGNAME, "LAN bridge batch rollback could not remove bridge %s (status=%d)",
							   bridge->bridge_name, ret);
					}
				} else {
					ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
					if (ret != 0)
						status = route_status_accumulate(status, ret);
					if (bridge_empty < 0)
						status = route_status_accumulate(status, bridge_empty_errno);
					else
						status = route_status_accumulate(status, EBUSY);
					logmessage(LOGNAME, "LAN bridge batch rollback preserved bridge %s because it is non-empty or its port inventory is unknown",
						   bridge->bridge_name);
				}
			} else if (errno != ENODEV && errno != ENXIO) {
				status = route_status_accumulate(status, errno ? errno : EIO);
				logmessage(LOGNAME, "LAN bridge batch rollback could not inspect bridge %s",
					   bridge->bridge_name);
			}
		} else if (bridge->bridge_existed) {
			if (wan_detached_safe && bwan_detached_safe) {
				ret = bridge->bridge_was_up ?
					ifconfig(bridge->bridge_name, IFUP, "0.0.0.0", NULL) :
					ifconfig(bridge->bridge_name, 0, NULL, NULL);
				if (ret != 0) {
					status = route_status_accumulate(status, ret);
					logmessage(LOGNAME, "LAN bridge batch rollback could not restore bridge %s state (status=%d)",
						   bridge->bridge_name, ret);
				}
			} else {
				ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
				status = route_status_accumulate(status, EIO);
				if (ret != 0)
					status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback kept bridge %s down because membership ownership is uncertain",
					   bridge->bridge_name);
			}
		}
	}
	return status;
}

static int
read_soft_bridge_port_vlan_config(int port, int *vid, int *prio)
{
	char param_name[32];
	int configured_vid = 0, configured_prio = 0;
	int status;

	if (port < 1 || port > 4 || !vid || !prio)
		return EINVAL;
	snprintf(param_name, sizeof(param_name), "vlan_vid_lan%d", port);
	status = get_nvram_int_checked_status(param_name, &configured_vid);
	if (status != 0)
		return status;
	snprintf(param_name, sizeof(param_name), "vlan_pri_lan%d", port);
	status = get_nvram_int_checked_status(param_name, &configured_prio);
	if (status != 0)
		return status;

	*vid = configured_vid;
	*prio = configured_prio;
	return 0;
}


static int
config_soft_bridges_other(int vinet_vid, int viptv_vid, int pvid_wan, const char *wan_hwaddr, int bridge_mode)
{
	struct soft_bridge_other_entry entries[4];
	struct soft_bridge_other_entry *entry;
	int i, br_id, port_vid[4], port_pri[4] = {0};
	int ret, saved_errno, port_flags, entry_count = 0;
	char port_ifname[24];

	for (i = 0; i < 4; i++)
		port_vid[i] = -1;
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN1:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		ret = read_soft_bridge_port_vlan_config(1,
						 &port_vid[0], &port_pri[0]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN bridge LAN1 VLAN settings (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		port_pri[0] &= 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		ret = read_soft_bridge_port_vlan_config(2,
						 &port_vid[1], &port_pri[1]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN bridge LAN2 VLAN settings (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		port_pri[1] &= 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN3:
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		ret = read_soft_bridge_port_vlan_config(3,
						 &port_vid[2], &port_pri[2]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN bridge LAN3 VLAN settings (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		port_pri[2] &= 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN4:
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		ret = read_soft_bridge_port_vlan_config(4,
						 &port_vid[3], &port_pri[3]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN bridge LAN4 VLAN settings (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		port_pri[3] &= 0x07;
		break;
	}
	for (i = 0; i < 4; i++) {
		if (port_vid[i] < 0)
			continue;
		if (!is_vlan_vid_valid(port_vid[i]))
			port_vid[i] = pvid_wan;
		if (port_vid[i] == vinet_vid || port_vid[i] == viptv_vid)
			port_vid[i] = 0;
	}
	br_id = 2;
	for (i = 0; i < 4; i++) {
		if (port_vid[i] < 2)
			continue;
		entry = &entries[entry_count];
		memset(entry, 0, sizeof(*entry));
		snprintf(port_ifname, sizeof(port_ifname), "%s.%d", IFNAME_MAC2, port_vid[i]);
		snprintf(entry->port_ifname, sizeof(entry->port_ifname), "%s", port_ifname);
		port_flags = get_interface_flags(port_ifname);
		if (port_flags < 0 && errno != ENODEV && errno != ENXIO) {
			ret = -1;
			goto rollback;
		}
		entry->port_vlan_existed = port_flags >= 0;
		entry->port_vlan_was_up = entry->port_vlan_existed && (port_flags & IFF_UP);
		entry_count++;

		ret = create_vlan_iface_tracked(IFNAME_MAC2, port_vid[i], port_pri[i],
						1500, wan_hwaddr, 1,
						&entry->port_vlan_created);
		if (ret != 0) {
			logmessage(LOGNAME, "LAN bridge VLAN creation failed for %s (status=%d)",
				   port_ifname, ret);
			goto rollback;
		}
		ret = config_soft_bridge_wan_internal(br_id, port_vid[i], port_pri[i],
						      port_ifname, wan_hwaddr,
						      &entry->bridge);
		if (ret != 0) {
			logmessage(LOGNAME, "LAN bridge setup failed for %s (status=%d)",
				   port_ifname, ret);
			goto rollback;
		}
		br_id++;
	}
	return 0;

rollback:
	saved_errno = errno;
	while (entry_count > 0) {
		int rollback_status = rollback_soft_bridge_other_entry(&entries[--entry_count]);
		if (rollback_status != 0)
			logmessage(LOGNAME, "additional WAN bridge batch rollback incomplete (status=%d)",
				   rollback_status);
	}
	errno = saved_errno;
	return ret;
}
#endif

static int
wan_viptv_ifname_publish_status(const char *ifname)
{
	int api_result, saved_errno;

	if (!ifname)
		return EINVAL;

	errno = 0;
	api_result = nvram_set_temp("viptv_ifname", ifname);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}


static int
rollback_viptv_vlan_setup(struct soft_bridge_other_entry *entry)
{
	int status = 0, ret = 0;
	const char *recovery_ifname = NULL;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	int bridge_flags;

	status = rollback_soft_bridge_other_entry(entry);
	if (entry->bridge.bridge_name[0]) {
		bridge_flags = get_interface_flags(entry->bridge.bridge_name);
		if (entry->bridge.bridge_existed || bridge_flags >= 0 ||
		    (bridge_flags < 0 && errno != ENODEV && errno != ENXIO))
			recovery_ifname = entry->bridge.bridge_name;
		else if (status != 0 || entry->port_vlan_existed)
			recovery_ifname = entry->port_ifname;
	} else if (status != 0 || entry->port_vlan_existed) {
		recovery_ifname = entry->port_ifname;
	}
#else
	/* Boards without GMAC2 bridge support still need VLAN-only rollback. */
	if (entry->port_vlan_created) {
		ret = remove_vlan_iface(entry->port_ifname);
	} else if (entry->port_vlan_existed) {
		ret = entry->port_vlan_was_up ?
			ifconfig(entry->port_ifname, IFUP, "0.0.0.0", NULL) :
			ifconfig(entry->port_ifname, 0, NULL, NULL);
	}
	if (ret != 0) {
		status = route_status_accumulate(status, ret);
		logmessage(LOGNAME, "IPTV VLAN-only rollback incomplete for %s (status=%d)",
			   entry->port_ifname, ret);
	}
	if (status != 0 || entry->port_vlan_existed)
		recovery_ifname = entry->port_ifname;
#endif

	ret = wan_viptv_ifname_publish_status(
		recovery_ifname ? recovery_ifname : "");
	if (ret != 0) {
		logmessage(LOGNAME,
			   "failed to publish IPTV VLAN recovery interface identity (status=%d)",
			   ret);
		status = route_status_accumulate(status, ret);
	}
	return status;
}

struct wan_viptv_ifname_publish_context {
	const char *ifname;
	struct soft_bridge_other_entry *rollback_entry;
};

static int
wan_viptv_ifname_publish_callback(void *opaque)
{
	struct wan_viptv_ifname_publish_context *context =
		(struct wan_viptv_ifname_publish_context *)opaque;

	if (!context || !context->ifname)
		return EINVAL;
	return wan_viptv_ifname_publish_status(context->ifname);
}

static int
wan_viptv_ifname_rollback_callback(void *opaque)
{
	struct wan_viptv_ifname_publish_context *context =
		(struct wan_viptv_ifname_publish_context *)opaque;

	if (!context || !context->rollback_entry)
		return EINVAL;
	return rollback_viptv_vlan_setup(context->rollback_entry);
}

static const struct wan_viptv_ifname_ops wan_viptv_ifname_publish_ops = {
	wan_viptv_ifname_publish_callback,
	wan_viptv_ifname_rollback_callback
};


struct wan_viptv_old_vlan_context {
	const char *ifname;
};

static int
wan_viptv_old_vlan_clear_identity(void *opaque)
{
	int status;

	(void)opaque;
	status = wan_viptv_ifname_publish_status("");
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot clear old IPTV VLAN recovery identity (status=%d); old VLAN retained",
			   status);
	return status;
}

static int
wan_viptv_old_vlan_remove(void *opaque)
{
	struct wan_viptv_old_vlan_context *context =
		(struct wan_viptv_old_vlan_context *)opaque;
	int status;

	if (!context || !context->ifname || !context->ifname[0])
		return EINVAL;
	status = remove_vlan_iface(context->ifname);
	if (status != 0)
		logmessage(LOGNAME,
			   "old IPTV VLAN removal failed for %s (status=%d); recovery identity will be restored",
			   context->ifname, status);
	return status;
}

static int
wan_viptv_old_vlan_restore_identity(void *opaque)
{
	struct wan_viptv_old_vlan_context *context =
		(struct wan_viptv_old_vlan_context *)opaque;
	int status;

	if (!context || !context->ifname || !context->ifname[0])
		return EINVAL;
	status = wan_viptv_ifname_publish_status(context->ifname);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot restore old IPTV VLAN recovery identity for %s (status=%d)",
			   context->ifname, status);
	return status;
}

static const struct wan_viptv_old_vlan_ops wan_viptv_old_vlan_ops = {
	wan_viptv_old_vlan_clear_identity,
	wan_viptv_old_vlan_remove,
	wan_viptv_old_vlan_restore_identity
};


static int
config_vinet_wan(void)
{
	int vlan_vid[2], vlan_pri, pvid_wan;
	int is_vlan_filter, is_vlan_ifname;
	int need_pvid, pvid_result = 0, ret;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status;
#endif
	char vinet_ifname[24], vlan_iflast[24];
	char previous_vinet_ifname[IFNAMSIZ], vinet_iflast[IFNAMSIZ];
	char wan_hwaddr[18] = {0};
#if defined (USE_SINGLE_MAC)
	const char *ifname_wan_cpu = IFNAME_MAC;
#else
	const char *ifname_wan_cpu = IFNAME_MAC2;
#endif
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	int bridge_mode;
#endif

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	ret = get_wan_bridge_mode_status(&bridge_mode);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN bridge mode before WAN VLAN setup (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}
#endif

	ret = get_nvram_match_checked_status("vlan_filter", "1",
					     &is_vlan_filter);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN VLAN filter flag before interface setup (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}
	if (is_vlan_filter) {
		ret = get_nvram_int_checked_status("vlan_vid_cpu", &vlan_vid[0]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN CPU VLAN ID before interface setup (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		ret = get_nvram_int_checked_status("vlan_vid_iptv", &vlan_vid[1]);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN IPTV VLAN ID before interface setup (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		ret = get_nvram_int_checked_status("vlan_pri_cpu", &vlan_pri);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read WAN CPU VLAN priority before interface setup (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
		vlan_pri &= 0x07;
	} else {
		vlan_vid[0] = 0;
		vlan_vid[1] = 0;
		vlan_pri = 0;
	}

	need_pvid = (!is_vlan_vid_valid(vlan_vid[0]) ||
		     !is_vlan_vid_valid(vlan_vid[1]));
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE && is_vlan_filter)
		need_pvid = 1;
#endif
	if (need_pvid) {
		pvid_result = phy_vlan_pvid_wan_get();
		if (pvid_result < 0 || !is_vlan_vid_valid(pvid_result)) {
			ret = (pvid_result < 0) ? pvid_result : -EINVAL;
			logmessage(LOGNAME, "cannot configure WAN VLANs: switch PVID read failed (%d)", pvid_result);
			return ret;
		}
	}
	pvid_wan = pvid_result;
	{
		int configured_vid = vlan_vid[0];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_result, is_vlan_vid_valid(pvid_result),
						     &vlan_vid[0]);
		if (ret != 0)
			return ret;
		if (!is_vlan_vid_valid(configured_vid))
			vlan_pri = 0;
	}
	{
		int configured_vid = vlan_vid[1];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_result, is_vlan_vid_valid(pvid_result),
						     &vlan_vid[1]);
		if (ret != 0)
			return ret;
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "cannot configure WAN VLAN interfaces without verified IPv6 service state (status=%d)",
			   ipv6_type_status);
		return ipv6_type_status;
	}
#endif

	/*
	 * A failed cleanup-identity read must not look like an empty value:
	 * otherwise replacing the VLAN could orphan the previous interface.
	 * Validate the persisted name before any switch/interface state changes.
	 */
	ret = get_wan_unit_value_copy_status(
		0, "viflast", previous_vinet_ifname, sizeof(previous_vinet_ifname));
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read previous WAN VLAN cleanup identity (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}
	vinet_iflast[0] = '\0';
	if (previous_vinet_ifname[0]) {
		ret = wan_ifname_copy(vinet_iflast, sizeof(vinet_iflast),
				      previous_vinet_ifname);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "previous WAN VLAN cleanup identity is invalid (status=%d)",
				   ret);
			errno = ret;
			return ret;
		}
	}

	ret = rc_wan_nvram_copy_value(
		wan_nvram_checked_get_callback, NULL, "wan_hwaddr",
		wan_hwaddr, sizeof(wan_hwaddr));
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN MAC override before VLAN hardware changes (status=%d)",
			   ret);
		errno = ret;
		return ret;
	}

	/* update VLAN VID for raeth (HW_VLAN_TX) */
	hw_vlan_tx_map(6, vlan_vid[0]);
	hw_vlan_tx_map(7, vlan_vid[1]);

	is_vlan_ifname = 1;
	snprintf(vinet_ifname, sizeof(vinet_ifname), "%s.%d", ifname_wan_cpu, vlan_vid[0]);
#if !defined (USE_SINGLE_MAC)
#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	/* case1: CPU Internet tagged: n, CPU IPTV tagged: n */
	/* case2: CPU Internet tagged: y, CPU IPTV tagged: y (common VID) */
	if (vlan_vid[0] == vlan_vid[1])
#else
	/* case: no VLAN filter */
	if (!is_vlan_filter)
#endif
	{
		is_vlan_ifname = 0;
		snprintf(vinet_ifname, sizeof(vinet_ifname), "%s", IFNAME_MAC2);
	}

	/* always prepare eth3 interface */
	ret = ifconfig(IFNAME_MAC2, 0, NULL, NULL);
	if (ret != 0)
		return ret;
	ret = set_interface_hwaddr(IFNAME_MAC2, wan_hwaddr);
	if (ret != 0)
		return ret;
	ret = set_interface_mtu(IFNAME_MAC2, 1500);
	if (ret != 0)
		return ret;
	ret = ifconfig(IFNAME_MAC2, IFUP, "0.0.0.0", NULL);
	if (ret != 0)
		return ret;
#endif

	/* remove previous VLAN interface */
	if (vinet_iflast[0] && strcmp(vinet_iflast, vinet_ifname)) {
		ret = remove_vlan_iface(vinet_iflast);
		if (ret != 0) {
			logmessage(LOGNAME, "previous WAN VLAN removal failed for %s (status=%d)",
				   vinet_iflast, ret);
			return ret;
		}
	}

	/*
	 * Publish the cleanup identity before creating/replacing the VLAN. If a
	 * later bridge or IPv6 step fails, the next restart can still find and
	 * remove this interface. A failed NVRAM write must abort before creation.
	 */
	vlan_iflast[0] = '\0';
	if (is_vlan_ifname)
		snprintf(vlan_iflast, sizeof(vlan_iflast), "%s.%d",
			 ifname_wan_cpu, vlan_vid[0]);
	ret = set_wan_unit_value_status(0, "viflast", vlan_iflast);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot publish WAN VLAN cleanup identity %s (status=%d)",
			   vlan_iflast, ret);
		return ret;
	}

	/* create VLAN interface for INET */
	if (is_vlan_ifname) {
#if defined (USE_IPV6)
		if (ipv6_type != IPV6_DISABLED) {
			ret = control_if_ipv6(ifname_wan_cpu, 0);
			if (ret != 0) {
				logmessage(LOGNAME, "failed to disable IPv6 on WAN VLAN parent %s (status=%d)",
					   ifname_wan_cpu, ret);
				return ret;
			}
		}
#endif
		ret = create_vlan_iface(ifname_wan_cpu, vlan_vid[0], vlan_pri, 1500, wan_hwaddr, 1);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN VLAN creation failed for %s (status=%d)",
				   vinet_ifname, ret);
			return ret;
		}
	}

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE && is_vlan_filter) {
		ret = config_soft_bridges_other(vlan_vid[0], vlan_vid[1], pvid_wan, wan_hwaddr, bridge_mode);
		if (ret != 0)
			return ret;
	}
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE) {
		ret = config_soft_bridge_wan(0, vlan_vid[0], vlan_pri, vinet_ifname, wan_hwaddr);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN bridge brwan0 setup failed (status=%d)", ret);
			return ret;
		}
		snprintf(vinet_ifname, sizeof(vinet_ifname), "%s%d", BRPREFIX_WAN, 0);
	}
#endif


#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		ret = control_if_ipv6(vinet_ifname, 1);
		if (ret != 0) {
			logmessage(LOGNAME, "failed to enable IPv6 on WAN interface %s (status=%d)",
				   vinet_ifname, ret);
			return ret;
		}
	}
#endif
	ret = set_wan_unit_value_status(0, "ifname", vinet_ifname);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot publish active WAN interface %s (status=%d)",
			   vinet_ifname, ret);
		return ret;
	}
	return 0;
}

static int
config_apcli_wisp(void)
{
	int unit = 0, publication_status, ifname_status, wisp_status;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status;
#endif
	char man_ifname[16], man_ifname_nvram[16];
	const char *wisp_ifname = NULL;

	wisp_status = get_apcli_wisp_ifname_status(&wisp_ifname);
	if (wisp_status != 0) {
		errno = wisp_status;
		logmessage(LOGNAME,
			   "cannot migrate AP-client WAN without verified WISP configuration (status=%d)",
			   wisp_status);
		return wisp_status;
	}
	if (!wisp_ifname)
		return 0;

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "cannot migrate AP-client WAN interface without verified IPv6 service state (status=%d)",
			   ipv6_type_status);
		return ipv6_type_status;
	}
#endif

	/*
	 * Resolve the previous WAN interface before changing it or publishing
	 * the AP-client interface. A safe_get-backed read can hide NVRAM errors
	 * as an empty interface and skip the required old-interface cleanup.
	 */
	ifname_status = get_wan_unit_value_copy_status(
		unit, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
	if (ifname_status != 0) {
		errno = ifname_status;
		logmessage(LOGNAME,
			   "cannot read previous WAN interface before AP-client migration (status=%d)",
			   ifname_status);
		return ifname_status;
	}
	if (man_ifname_nvram[0]) {
		ifname_status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (ifname_status != 0) {
			errno = ifname_status;
			logmessage(LOGNAME,
				   "invalid previous WAN interface before AP-client migration (status=%d)",
				   ifname_status);
			return ifname_status;
		}
	} else {
		man_ifname[0] = '\0';
	}
	if (man_ifname[0] && !is_man_wisp(man_ifname)) {
		int address_status, down_status;

		address_status = ifconfig(man_ifname, IFUP, "0.0.0.0", NULL);
		if (address_status != 0)
			logmessage(LOGNAME, "failed to clear previous WAN address on %s (status=%d)",
				   man_ifname, address_status);
#if defined (USE_SINGLE_MAC)
		down_status = ifconfig(man_ifname, 0, "0.0.0.0", NULL);
		if (down_status != 0) {
			logmessage(LOGNAME, "failed to shut down previous WAN interface %s (status=%d)",
				   man_ifname, down_status);
			down_status = ifconfig(man_ifname, 0, NULL, NULL);
			if (down_status != 0) {
				logmessage(LOGNAME, "unable to keep previous WAN interface %s down; refusing WISP migration (status=%d)",
					   man_ifname, down_status);
				return down_status;
			}
		}
#else
		if (address_status != 0) {
			down_status = ifconfig(man_ifname, 0, NULL, NULL);
			if (down_status != 0) {
				logmessage(LOGNAME, "unable to clear or shut down previous WAN interface %s; refusing WISP migration (status=%d)",
					   man_ifname, down_status);
				return down_status;
			}
		}
#endif
	}

#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		int ipv6_status = control_if_ipv6(wisp_ifname, 1);
		if (ipv6_status != 0) {
			logmessage(LOGNAME, "failed to enable IPv6 on AP-client WAN interface %s (status=%d)",
				   wisp_ifname, ipv6_status);
			return ipv6_status;
		}
	}
#endif
	publication_status = set_wan_unit_value_status(0, "ifname", wisp_ifname);
	if (publication_status != 0) {
		logmessage(LOGNAME,
			   "cannot publish AP-client WAN interface %s (status=%d)",
			   wisp_ifname, publication_status);
		return publication_status;
	}
	return 0;
}

static int
wait_apcli_connected(const char *ifname, int wait_sec)
{
	int i;

	for (i = 0; i <= (wait_sec * 2); i++) {
		if (service_wait_microseconds(500000U) != 0)
			return -1;
		if (get_apcli_connected(ifname))
			return 1;
	}

	return 0;
}


struct wan_viptv_static_start_context {
	char *ifname;
	char *ipaddr;
	char *netmask;
	char *gateway;
};

static int wan_dhcp_nvram_publish_values(const char *const *names,
					 const char *const *new_values,
					 size_t count);

static int
wan_viptv_static_configure_address(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	if (!context || !context->ifname || !context->ipaddr || !context->netmask)
		return EINVAL;
	return ifconfig(context->ifname, IFUP, context->ipaddr, context->netmask);
}

static int
wan_viptv_static_publish_snapshot(void *opaque)
{
	static const char *const names[] = {
		"manv_ipaddr", "manv_netmask", "manv_gateway"
	};
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;
	const char *values[3];

	if (!context || !context->ipaddr || !context->netmask)
		return EINVAL;
	values[0] = context->ipaddr;
	values[1] = context->netmask;
	values[2] = (context->gateway && is_valid_ipv4(context->gateway)) ?
		context->gateway : "";

	return wan_dhcp_nvram_publish_values(names, values, 3);
}

static int
wan_viptv_static_clear_address(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;
	int status;

	if (!context || !context->ifname)
		return EINVAL;
	status = ifconfig(context->ifname, IFUP, "0.0.0.0", NULL);
	if (status != 0)
		logmessage(LOGNAME,
			   "failed to clear static IPTV address on %s after NVRAM publication failure (status=%d)",
			   context->ifname, status);
	return status;
}

static int
wan_viptv_static_add_default_route(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	return route_add(context->ifname, 10, "0.0.0.0",
			 context->gateway, "0.0.0.0");
}

static int
wan_viptv_static_start_multicast(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	return start_igmpproxy(context->ifname);
}

static const struct wan_viptv_static_ops wan_viptv_static_start_ops = {
	wan_viptv_static_configure_address,
	wan_viptv_static_publish_snapshot,
	wan_viptv_static_clear_address,
	wan_viptv_static_add_default_route,
	wan_viptv_static_start_multicast
};

struct wan_viptv_dynamic_start_context {
	const char *ifname;
	int zeroconf;
	struct soft_bridge_other_entry *rollback_entry;
};

static int
wan_viptv_dynamic_start_callback(void *opaque)
{
	struct wan_viptv_dynamic_start_context *context =
		(struct wan_viptv_dynamic_start_context *)opaque;

	if (!context || !context->ifname || !context->ifname[0])
		return EINVAL;
	if (context->zeroconf)
		return start_zcip_viptv((char *)context->ifname);
	return start_udhcpc_viptv((char *)context->ifname);
}

static int
wan_viptv_dynamic_stop_callback(void *opaque)
{
	struct wan_viptv_dynamic_start_context *context =
		(struct wan_viptv_dynamic_start_context *)opaque;

	if (!context)
		return EINVAL;
	if (context->zeroconf) {
		char *services[] = { "zcip", NULL };
		return kill_services(services, 3, 1);
	}
	return stop_udhcpc_viptv();
}

static int
wan_viptv_dynamic_rollback_callback(void *opaque)
{
	struct wan_viptv_dynamic_start_context *context =
		(struct wan_viptv_dynamic_start_context *)opaque;

	if (!context || !context->rollback_entry)
		return EINVAL;
	return rollback_viptv_vlan_setup(context->rollback_entry);
}

static const struct wan_viptv_dynamic_start_ops wan_viptv_dynamic_start_ops = {
	wan_viptv_dynamic_start_callback, wan_viptv_dynamic_stop_callback,
	wan_viptv_dynamic_rollback_callback
};

static int
launch_viptv_wan(void)
{
	int vlan_vid[2], vlan_pri, pvid_wan = 0;
	int need_pvid, ret;
	int is_vlan_filter, viptv_mode;
	int vlan_flags, saved_errno, rollback_status;
	char viptv_ifname[24], viptv_addr[16], viptv_mask[16], viptv_gate[16];
	char viptv_iflast[24], vinet_iflast[24], wan_hwaddr[24];
	char viptv_addr_setting[16], viptv_mask_setting[16], viptv_gate_setting[16];
	char vlan_filter[8];
	struct soft_bridge_other_entry rollback_entry;
	struct wan_viptv_ifname_publish_context ifname_context;
	struct wan_viptv_old_vlan_context old_vlan_context;
	char old_vlan_ifname[24];
#if defined (USE_SINGLE_MAC)
	const char *ifname_wan_cpu = IFNAME_MAC;
#else
	const char *ifname_wan_cpu = IFNAME_MAC2;
#endif

	ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
				      "vlan_filter", vlan_filter,
				      sizeof(vlan_filter));
	if (ret != 0)
		return ret;
	is_vlan_filter = (strcmp(vlan_filter, "1") == 0);
	if (is_vlan_filter) {
		ret = get_nvram_int_checked_status("vlan_vid_cpu", &vlan_vid[0]);
		if (ret == 0)
			ret = get_nvram_int_checked_status("vlan_vid_iptv", &vlan_vid[1]);
		if (ret == 0)
			ret = get_nvram_int_checked_status("vlan_pri_iptv", &vlan_pri);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read IPTV VLAN configuration (status=%d)", ret);
			return ret;
		}
		vlan_pri &= 0x07;
	} else {
		vlan_vid[0] = 0;
		vlan_vid[1] = 0;
		vlan_pri = 0;
	}
	need_pvid = (!is_vlan_vid_valid(vlan_vid[0]) ||
		     !is_vlan_vid_valid(vlan_vid[1]));
	if (need_pvid) {
		pvid_wan = phy_vlan_pvid_wan_get();
		if (pvid_wan < 0 || !is_vlan_vid_valid(pvid_wan)) {
			ret = (pvid_wan < 0) ? pvid_wan : -EINVAL;
			logmessage(LOGNAME, "cannot configure IPTV VLANs: switch PVID read failed (%d)", pvid_wan);
			return ret;
		}
	}
	{
		int configured_vid = vlan_vid[0];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_wan, is_vlan_vid_valid(pvid_wan),
						     &vlan_vid[0]);
		if (ret != 0)
			return ret;
	}
	{
		int configured_vid = vlan_vid[1];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_wan, is_vlan_vid_valid(pvid_wan),
						     &vlan_vid[1]);
		if (ret != 0)
			return ret;
		if (!is_vlan_vid_valid(configured_vid))
			vlan_pri = 0;
	}
	ret = get_wan_unit_value_copy_status(0, "viflast",
					   vinet_iflast, sizeof(vinet_iflast));
	if (ret != 0)
		return ret;
	ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
				      "viptv_ifname", viptv_iflast,
				      sizeof(viptv_iflast));
	if (ret != 0)
		return ret;

	if (vlan_vid[1] == vlan_vid[0]) {
		/* case1: CPU Internet tagged: n, CPU IPTV tagged: n */
		/* case2: CPU Internet tagged: y, CPU IPTV tagged: y (common VID) */
		if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast)) {
			ret = remove_vlan_iface(viptv_iflast);
			if (ret != 0)
				return ret;
		}
		viptv_ifname[0] = 0;
		ret = wan_viptv_ifname_publish_status(viptv_ifname);
		if (ret != 0)
			logmessage(LOGNAME,
				   "failed to clear IPTV VLAN interface identity (status=%d)",
				   ret);
		return ret;
	}

	/* case3: CPU Internet tagged: y, CPU IPTV tagged: y (different VID) */
	/* case4: CPU Internet tagged: n, CPU IPTV tagged: y (different VID) */
	/* case5: CPU Internet tagged: y, CPU IPTV tagged: n */
	ret = get_nvram_int_checked_status("viptv_mode", &viptv_mode);
	if (ret != 0) {
		logmessage(LOGNAME, "cannot read IPTV address mode (status=%d)", ret);
		return ret;
	}
	viptv_gate[0] = 0;
	if (viptv_mode == 2) {
		/* Copy each cache-backed value before the next NVRAM lookup. */
		ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
					      "viptv_ipaddr", viptv_addr_setting,
					      sizeof(viptv_addr_setting));
		if (ret == 0)
			ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback,
						      NULL, "viptv_netmask",
						      viptv_mask_setting,
						      sizeof(viptv_mask_setting));
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read static IPTV address/netmask (status=%d)", ret);
			return ret;
		}
		if (!is_valid_ipv4(viptv_addr_setting) ||
		    !is_valid_ipv4(viptv_mask_setting)) {
			logmessage(LOGNAME, "rejecting invalid static IPTV address or netmask");
			return EINVAL;
		}
		strcpy(viptv_addr, viptv_addr_setting);
		strcpy(viptv_mask, viptv_mask_setting);

		ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
					      "viptv_gateway", viptv_gate_setting,
					      sizeof(viptv_gate_setting));
		if (ret != 0) {
			logmessage(LOGNAME,
				   "cannot read static IPTV gateway (status=%d)", ret);
			return ret;
		}
		if (is_valid_ipv4(viptv_gate_setting))
			strcpy(viptv_gate, viptv_gate_setting);
	}

	/* VLAN interface for IPTV */
	snprintf(viptv_ifname, sizeof(viptv_ifname), "%s.%d", ifname_wan_cpu, vlan_vid[1]);

	ret = rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL,
				      "wan_hwaddr", wan_hwaddr,
				      sizeof(wan_hwaddr));
	if (ret != 0) {
		logmessage(LOGNAME, "cannot read WAN MAC for IPTV VLAN (status=%d)", ret);
		return ret;
	}
	memset(&rollback_entry, 0, sizeof(rollback_entry));
	snprintf(rollback_entry.port_ifname, sizeof(rollback_entry.port_ifname),
		 "%s", viptv_ifname);
	vlan_flags = get_interface_flags(viptv_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO)
		return -1;
	rollback_entry.port_vlan_existed = vlan_flags >= 0;
	rollback_entry.port_vlan_was_up =
		rollback_entry.port_vlan_existed && ((vlan_flags & IFF_UP) != 0);


	/* Remove a no-longer-used VLAN without losing its recovery handle. */
	if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast) &&
	    strcmp(viptv_iflast, viptv_ifname)) {
		if (strchr(viptv_iflast, '.') != NULL) {
			if (strlen(viptv_iflast) >= sizeof(old_vlan_ifname))
				return ENAMETOOLONG;
			snprintf(old_vlan_ifname, sizeof(old_vlan_ifname), "%s",
				 viptv_iflast);
			old_vlan_context.ifname = old_vlan_ifname;
			ret = wan_viptv_ifname_remove_old_vlan(
				&wan_viptv_old_vlan_ops, &old_vlan_context);
			if (ret != 0)
				return ret;
		} else {
			ret = remove_vlan_iface(viptv_iflast);
			if (ret != 0)
				return ret;
		}
	}

	ret = create_vlan_iface_tracked(ifname_wan_cpu, vlan_vid[1], vlan_pri,
					1500, wan_hwaddr, 1,
					&rollback_entry.port_vlan_created);
	if (ret != 0) {
		saved_errno = errno;
		rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
		if (rollback_status != 0)
			logmessage(LOGNAME, "IPTV VLAN creation rollback incomplete (status=%d)",
				   rollback_status);
		errno = saved_errno;
		return ret;
	}
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE) {
		ret = config_soft_bridge_wan_internal(1, vlan_vid[1], vlan_pri,
						      viptv_ifname, wan_hwaddr,
						      &rollback_entry.bridge);
		if (ret != 0) {
			saved_errno = errno;
			rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
			if (rollback_status != 0)
				logmessage(LOGNAME, "IPTV VLAN/bridge rollback incomplete (status=%d)",
					   rollback_status);
			errno = saved_errno;
			return ret;
		}
		snprintf(viptv_ifname, sizeof(viptv_ifname), "%s%d", BRPREFIX_WAN, 1);
	}
#endif
	ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);
	if (ret != 0) {
		saved_errno = errno;
		rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
		if (rollback_status != 0)
			logmessage(LOGNAME, "IPTV interface setup rollback incomplete (status=%d)",
				   rollback_status);
		errno = saved_errno;
		return ret;
	}
	ifname_context.ifname = viptv_ifname;
	ifname_context.rollback_entry = &rollback_entry;
	ret = wan_viptv_ifname_publish_or_rollback(
		&wan_viptv_ifname_publish_ops, &ifname_context);
	if (ret != 0) {
		logmessage(LOGNAME,
			   "cannot publish IPTV VLAN interface identity on %s; VLAN rollback attempted (status=%d)",
			   viptv_ifname, ret);
		return ret;
	}

	if (viptv_mode == 2) {
		struct wan_viptv_static_start_context context;
		context.ifname = viptv_ifname;
		context.ipaddr = viptv_addr;
		context.netmask = viptv_mask;
		context.gateway = viptv_gate;
		ret = wan_viptv_static_start(is_valid_ipv4(viptv_gate),
					     &wan_viptv_static_start_ops, &context);
		if (ret != 0)
			logmessage(LOGNAME, "static IPTV startup incomplete on %s (error=%d)",
				   viptv_ifname, ret);
		else
			logmessage(LOGNAME, "%s %s (%s)", "IPTV MAN", "up", viptv_ifname);
		return ret;
	} else {
		struct wan_viptv_dynamic_start_context dynamic_context;
		struct wan_viptv_dynamic_start_result dynamic_result;

		dynamic_context.ifname = viptv_ifname;
		dynamic_context.zeroconf = (viptv_mode == 1);
		dynamic_context.rollback_entry = &rollback_entry;
		ret = wan_viptv_dynamic_start_with_rollback(
			&wan_viptv_dynamic_start_ops, &dynamic_context,
			&dynamic_result);
		if (ret != 0) {
			logmessage(LOGNAME,
				   "%s IPTV client startup failed on %s (status=%d)",
				   dynamic_context.zeroconf ? "ZeroConf" : "DHCP",
				   viptv_ifname, ret);
			if (dynamic_result.stop_status != 0) {
				logmessage(LOGNAME,
					   "IPTV client stop is unconfirmed (status=%d errno=%d); retaining VLAN recovery identity",
					   dynamic_result.stop_status,
					   dynamic_result.stop_errno);
			} else if (dynamic_result.rollback_status != 0) {
				logmessage(LOGNAME,
					   "IPTV VLAN rollback after client startup failure is incomplete (status=%d errno=%d)",
					   dynamic_result.rollback_status,
					   dynamic_result.rollback_errno);
			}
		}
	}
	return ret;
}

static int man_up_internal(char *, int, int, int *);

static int
launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)
{
	int launch_status = 0;
	int man_mtu = 0;
	int snapshot_status, nvram_status;
#if defined (USE_IPV6)
	int ipv6_type_status, ipv6_is_sit;
	int ipv6_if_status, ipv6_is_ppp;
#endif
	char man_addr_storage[16], man_mask_storage[16], lan_addr[16], lan_mask[16];
	char *man_addr = man_addr_storage, *man_mask = man_mask_storage;

	nvram_status = get_wan_unit_value_int_status(
		unit, "man_mtu", &man_mtu);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "cannot read MAN MTU for %s (status=%d)",
			   man_ifname, nvram_status);
		return nvram_status;
	}

	snapshot_status = wan_static_ipv4_snapshot(
		unit, "man_ipaddr", "man_netmask",
		man_addr_storage, sizeof(man_addr_storage),
		man_mask_storage, sizeof(man_mask_storage),
		lan_addr, sizeof(lan_addr), lan_mask, sizeof(lan_mask));
	if (snapshot_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot MAN/LAN IPv4 configuration on %s (status=%d)",
			   man_ifname, snapshot_status);
		return snapshot_status;
	}

	if (!is_valid_ipv4(man_addr))
		man_addr = "0.0.0.0";

	if (!is_valid_ipv4(man_addr) || !is_valid_ipv4(man_mask))
		man_mask = NULL;

	/* Do not launch a physical WAN client until the AP-client link is ready. */
	if (is_man_wisp(man_ifname)) {
		launch_status = rc_wan_apcli_wait_status(
			wait_apcli_connected(man_ifname, 3));
		if (launch_status != 0) {
			logmessage(LOGNAME,
				   "AP-client WAN interface %s did not associate before physical WAN setup (status=%d)",
				   man_ifname, launch_status);
			return launch_status;
		}
	}

	if (use_zcip || !is_valid_ipv4(man_addr)) {
		/* PPPoE connection not needed WAN physical address first, skip wait DHCP lease */
		/* PPTP and L2TP needed WAN physical first for create VPN tunnel, wait DHCP lease */
		if (!use_zcip) {
			launch_status = start_udhcpc_wan(man_ifname, unit, wait_dhcpc);
			
			/* add delay 2s after eth3 up: gethostbyname delay issue (L2TP/PPTP) */
			if (wait_dhcpc && launch_status == 0 &&
			    service_wait_seconds(2) != 0) {
				launch_status = errno > 0 ? errno : EIO;
				logmessage(LOGNAME, "WAN DHCP settling wait failed on %s (status=%d)",
					   man_ifname, launch_status);
			}
		} else
			launch_status = start_zcip_wan(man_ifname);
	} else {
		int man_err = 0, gateway_route_status = 0, route_status, address_status, mtu_status;
		int publish_status;
		
		if (!dhcp_lease_ipv4_is_valid(man_addr) ||
		    !dhcp_lease_netmask_is_valid(man_mask)) {
			man_err = 1;
			launch_status = EINVAL;
			logmessage(LOGNAME, "invalid static WAN IPv4 address or netmask on %s",
				   man_ifname);
		} else if (is_same_subnet2(man_addr, lan_addr, man_mask, lan_mask)) {
			man_err = 1;
			launch_status = EINVAL;
			
			logmessage(LOGNAME, "%s (%s) ERROR! Static IP [%s/%s] in LAN subnet [%s/%s]!",
				"MAN", man_ifname, man_addr, man_mask, lan_addr, lan_mask);
		} else {
			if (man_mtu >= 1300 && man_mtu < 1500) {
				mtu_status = set_interface_mtu(man_ifname, man_mtu);
				if (mtu_status != 0) {
					launch_status = route_status_accumulate(
						launch_status, mtu_status);
					logmessage(LOGNAME, "MAN interface %s MTU setup incomplete (status=%d)",
						   man_ifname, mtu_status);
				}
			}
			address_status = ifconfig(man_ifname, IFUP, man_addr, man_mask);
			if (address_status != 0) {
				man_err = 1;
				launch_status = route_status_accumulate(launch_status, address_status);
				logmessage(LOGNAME, "failed to configure static WAN address on %s (status=%d)",
					   man_ifname, address_status);
			} else {
				route_status = man_up_internal(man_ifname, unit, 1, &gateway_route_status);
				if (route_status != 0) {
					man_err = 1;
					logmessage(LOGNAME, "MAN route setup incomplete on %s (error=%d)",
						   man_ifname, route_status);
				}
				launch_status = route_status_accumulate(launch_status, route_status);
				if (gateway_route_status != 0) {
					man_err = 1;
					launch_status = route_status_accumulate(launch_status, gateway_route_status);
				}
			}
		}
		
		publish_status = wan_nvram_int_temp_status("wanx_err", man_err);
		launch_status = route_status_accumulate(launch_status, publish_status);
		if (publish_status != 0)
			logmessage(LOGNAME,
				   "cannot publish static WAN error state (status=%d)",
				   publish_status);
	}

#if defined (USE_IPV6)
	ipv6_type_status = is_wan_ipv6_type_sit_status(&ipv6_is_sit);
	if (ipv6_type_status != 0) {
		launch_status = route_status_accumulate(launch_status, ipv6_type_status);
		logmessage(LOGNAME,
			   "cannot determine WAN IPv6 transport for %s (status=%d)",
			   man_ifname, ipv6_type_status);
	} else if (ipv6_is_sit == 0) {
		ipv6_if_status = is_wan_ipv6_if_ppp_status(&ipv6_is_ppp);
		if (ipv6_if_status != 0) {
			launch_status = route_status_accumulate(launch_status, ipv6_if_status);
			logmessage(LOGNAME,
				   "cannot determine WAN IPv6 PPP interface selection for %s (status=%d)",
				   man_ifname, ipv6_if_status);
		} else if (!ipv6_is_ppp) {
			int ipv6_status = wan6_up(man_ifname, unit);
			if (ipv6_status != 0)
				logmessage(LOGNAME, "WAN physical IPv6 setup incomplete on %s (status=%d)",
					   man_ifname, ipv6_status);
			launch_status = route_status_accumulate(launch_status, ipv6_status);
		}
	}
#endif

	return launch_status;
}

static int
wait_ppp_up(char *ppp_ifname, int unit)
{
	int timeout, sockfd, route_status;
	int interface_status, failure_status, socket_status;
	struct ifreq ifr;
	struct rc_wan_nvram_value ppp_snapshot_values[3];
	const char *ppp_mask;
	char ppp_addr[16], ppp_gate[16];

	/* Wait for pppX to be created */
	timeout = 5;
	while ((interface_status = ifconfig(ppp_ifname, IFUP, NULL, NULL)) != 0) {
		/* Six failed probes (five one-second waits) mean the interface never appeared. */
		if (timeout-- <= 0) {
			failure_status = (interface_status < 0 && errno != 0) ?
				errno : interface_status;
			logmessage(LOGNAME, "PPP interface %s did not appear before startup timeout (status=%d)",
				   ppp_ifname, failure_status);
			return failure_status;
		}
		if (service_wait_seconds(1) != 0) {
			failure_status = errno > 0 ? errno : EIO;
			logmessage(LOGNAME, "PPP interface %s startup wait failed (status=%d)",
				   ppp_ifname, failure_status);
			return failure_status;
		}
	}

	/* Retrieve IP info */
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		socket_status = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot inspect demand PPP interface %s (errno=%d)",
			   ppp_ifname, socket_status);
		return socket_status;
	}

	/* Refresh the full PPP runtime tuple; never leave a previous session's fields. */
	ppp_addr[0] = '\0';
	ppp_gate[0] = '\0';
	ppp_mask = "0.0.0.0";
	strncpy(ifr.ifr_name, ppp_ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
		snprintf(ppp_addr, sizeof(ppp_addr), "%s",
			 inet_ntoa(sin_addr(&ifr.ifr_addr)));
		ppp_mask = "255.255.255.255";

		/* Store the temporary point-to-point peer when one is available. */
		if (ioctl(sockfd, SIOCGIFDSTADDR, &ifr) == 0)
			snprintf(ppp_gate, sizeof(ppp_gate), "%s",
				 inet_ntoa(sin_addr(&ifr.ifr_dstaddr)));
	} else {
		/* Demand PPP can exist before IPCP; clear any prior local/peer tuple. */
		snprintf(ppp_addr, sizeof(ppp_addr), "%s", "0.0.0.0");
	}

	ppp_snapshot_values[0].param_name = "ipaddr";
	ppp_snapshot_values[0].value = ppp_addr;
	ppp_snapshot_values[1].param_name = "netmask";
	ppp_snapshot_values[1].value = ppp_mask;
	ppp_snapshot_values[2].param_name = "gateway";
	ppp_snapshot_values[2].value = ppp_gate;
	route_status = rc_wan_nvram_set_required_values(
		ppp_snapshot_values, 3, set_wan_unit_value_status_callback, &unit);
	close(sockfd);
	if (route_status != 0) {
		logmessage(LOGNAME,
			   "failed to publish complete PPP runtime address snapshot on %s (status=%d)",
			   ppp_ifname, route_status);
		return route_status;
	}

	/* Preset routes so that traffic can be sent to proper pppx even before the link is brought up. */
	route_status = preset_wan_ppp_routes(ppp_ifname, unit);
	if (!route_status_demand_ready(route_status)) {
		logmessage(LOGNAME, "PPP demand route preinstallation incomplete on %s (status=%d)",
			   ppp_ifname, route_status);
		/*
		 * A failed batch can still have installed some routes. Do not blindly
		 * delete the configured set: EEXIST-normalized entries may predate this
		 * attempt. Demand pppd remains persistent; a later ip-up callback retries
		 * wan_up() route setup after start_wan() restores physical WAN publication.
		 */
		return route_status ? route_status : EIO;
	}

	/* Stimulate link up */
	if (is_valid_ipv4(ppp_gate))
		eval("ping", "-c1", ppp_gate);

	return 0;
}

static int
restore_wan_after_ppp_start_failure(int unit, char *wan_ifname)
{
	int upnp_status, firewall_status, nvram_status;

	/* Restore the physical path while demand PPP is unavailable. A later
	 * successful ip-up callback may switch the active WAN back to ppp0. */
	upnp_status = check_upnp_wanif_changed(wan_ifname);
	if (upnp_status != 0) {
		logmessage(LOGNAME, "cannot restore WAN after PPP start failure: miniupnpd did not stop (status=%d)",
			   upnp_status);
		return upnp_status;
	}
	nvram_status = set_wan_unit_value_status(unit, "ifname_t", wan_ifname);
	if (nvram_status != 0) {
		errno = nvram_status;
		logmessage(LOGNAME, "cannot publish physical WAN interface %s after PPP start failure (status=%d)",
			   wan_ifname, nvram_status);
		return nvram_status;
	}
	firewall_status = start_firewall_ex();
	if (firewall_status != 0)
		logmessage(LOGNAME, "cannot restore WAN firewall after PPP start failure (status=%d)",
			   firewall_status);
	return firewall_status;
}


static int
restore_wan_after_firewall_failure(int unit, char *wan_ifname)
{
	int restore_status, down_status;

	restore_status = restore_wan_after_ppp_start_failure(unit, wan_ifname);
	if (restore_status == 0)
		return 0;

	/*
	 * A stale or partial firewall is not sufficient to expose the WAN. If
	 * rebuilding the physical-WAN policy also fails, take that interface down.
	 */
	down_status = ifconfig(wan_ifname, 0, NULL, NULL);
	if (down_status != 0)
		logmessage(LOGNAME, "failed to keep physical WAN interface %s down after firewall recovery failure (status=%d)",
			   wan_ifname, down_status);
	return rc_first_service_start_status(restore_status, down_status);
}

static const char *const wan_callback_link_paths[] = {
 "/tmp/ppp/ip-up", "/tmp/ppp/ip-down",
 SCRIPT_UDHCPC_WAN, SCRIPT_UDHCPC_VIPTV, SCRIPT_ZCIP_WAN,
 SCRIPT_ZCIP_VIPTV, SCRIPT_WPACLI_WAN,
#if defined (USE_IPV6)
 "/tmp/ppp/ipv6-up", "/tmp/ppp/ipv6-down", SCRIPT_DHCP6C_WAN,
#endif
};
#define WAN_CALLBACK_LINK_COUNT (sizeof(wan_callback_link_paths)/sizeof(wan_callback_link_paths[0]))
static int
create_cb_links(void)
{
 unsigned char created[WAN_CALLBACK_LINK_COUNT];size_t i;int saved_errno;
 memset(created,0,sizeof(created));
 if(ppp_callback_link_directory_ready("/tmp")!=0||
    ppp_options_directory_ready("/tmp/ppp")!=0||
    ppp_options_directory_ready(PPP_PEERS_DIR)!=0){
  saved_errno=errno?errno:EPERM;
  logmessage(LOGNAME,"refusing WAN startup: unsafe shared tmp/PPP callback/peer directory (errno=%d)",saved_errno);
  errno=saved_errno;return -1;
 }
 for(i=0;i<WAN_CALLBACK_LINK_COUNT;++i){
  if(ppp_callback_link_ensure("/sbin/rc",wan_callback_link_paths[i],&created[i])!=0){
   saved_errno=errno?errno:EIO;
   logmessage(LOGNAME,"refusing WAN startup: cannot publish trusted callback link %s (errno=%d)",wan_callback_link_paths[i],saved_errno);
   while(i>0){
    int rollback_errno;--i;
    if(created[i]&&ppp_callback_link_remove("/sbin/rc",wan_callback_link_paths[i])!=0){
     rollback_errno=errno?errno:EIO;
     logmessage(LOGNAME,"cannot roll back callback link %s after startup abort (errno=%d)",wan_callback_link_paths[i],rollback_errno);
    }
   }
   errno=saved_errno;return -1;
  }
 }
 return 0;
}
static int
remove_cb_links(void)
{
 size_t i;int first_error=0;
 for(i=0;i<WAN_CALLBACK_LINK_COUNT;++i){
  if(ppp_callback_link_remove("/sbin/rc",wan_callback_link_paths[i])!=0){
   int remove_errno=errno?errno:EIO;
   if(first_error==0)first_error=remove_errno;
   logmessage(LOGNAME,"cannot safely remove WAN callback link %s (errno=%d)",wan_callback_link_paths[i],remove_errno);
  }
 }
 if(first_error){errno=first_error;return first_error;}
 return 0;
}

int
start_wan(void)
{
	int unit, wan_proto, wan_mtu;
	int vlan_result, mtu_status, interface_status, interface_cleanup_status;
	int cleanup_status;
	int callback_status;
	int static_address_clear_status;
	int wan_start_status = 0, wan_up_status, sysctl_status;
	int tcp_syncookies_enabled = 0;
	int tcp_syncookies_status = 0;
	int nvram_status, modem_type, ifname_status;
#if defined (USE_IPV6)
	int ipv6_type_status, ipv6_is_sit;
#endif
	char wan_ifname[16], wan_ifname_nvram[16];

	if (wan_vars_reset_status != 0) {
		wan_start_status = wan_vars_reset_status;
		logmessage(LOGNAME,
			   "aborting WAN startup because configured WAN protocol staging failed (status=%d)",
			   wan_start_status);
		return wan_start_status;
	}

	/* Read the security policy before any WAN setup side effects. */
	nvram_status = get_nvram_int_checked_status(
		"fw_syn_cook", &tcp_syncookies_enabled);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN startup: cannot read TCP SYN-cookie setting (status=%d)",
			   nvram_status);
		return nvram_status;
	}
	tcp_syncookies_enabled = tcp_syncookies_enabled ? 1 : 0;
	sysctl_status = set_tcp_syncookies(tcp_syncookies_enabled, &tcp_syncookies_status);
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN TCP/ICMP security sysctl configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);
	if (!rc_network_sysctl_tcp_syncookies_allows_wan(
		tcp_syncookies_enabled, tcp_syncookies_status)) {
		logmessage(LOGNAME,
			   "aborting WAN startup: configured TCP SYN cookies could not be applied (status=%d)",
			   tcp_syncookies_status);
		return rc_first_service_start_status(wan_start_status, tcp_syncookies_status);
	}

	/* Secure PPP callback/peer storage before publishing callback links. */
	if (create_cb_links() != 0) {
		callback_status = errno ? errno : EIO;
		wan_start_status = rc_first_service_start_status(
			wan_start_status, callback_status);
		logmessage(LOGNAME, "aborting WAN startup: PPP callback/peer directory setup failed (errno=%d)",
			   callback_status);
		return wan_start_status;
	}

	vlan_result = config_vinet_wan();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "aborting WAN startup: WAN VLAN configuration failed (%d)", vlan_result);
		cleanup_status = remove_cb_links();
		if (cleanup_status != 0)
			logmessage(LOGNAME, "WAN startup callback cleanup incomplete (status=%d)", cleanup_status);
		return wan_start_status;
	}

	/*
	 * Resolve the old physical WAN interface before starting an IPTV
	 * DHCP/ZeroConf client. On migration failure, leave no client running
	 * against an interface whose address/administrative state is unknown.
	 */
	vlan_result = config_apcli_wisp();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "aborting WAN startup: AP-client WISP migration failed (status=%d)",
			   vlan_result);
		cleanup_status = remove_cb_links();
		if (cleanup_status != 0)
			logmessage(LOGNAME, "WAN startup callback cleanup incomplete (status=%d)", cleanup_status);
		return wan_start_status;
	}

	vlan_result = launch_viptv_wan();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "IPTV VLAN or service setup incomplete (error=%d)", vlan_result);
	}

	vlan_result = reload_nat_modules();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "NAT helper or acceleration setup incomplete (status=%d); continuing WAN startup",
			   vlan_result);
	}

	refresh_resolvconf_nonfatal("WAN startup", 1);

	sysctl_status = set_nf_conntrack();
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN conntrack limit configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);


	sysctl_status = set_igmp_mld_version();
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN IGMP/MLD version sysctl configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);

	/* DI wakeup after 60 secs; retain launch/inventory/signal errors. */
	sysctl_status = notify_run_detect_internet(60);
	if (sysctl_status != 0)
		logmessage(LOGNAME, "Internet monitor startup incomplete (status=%d)", sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);

	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < 1; unit++)
	{
		int static_address_cleanup_failed = 0;
#if defined (USE_USB_SUPPORT)
		int is_usb_modem = 0;
#endif

		nvram_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (nvram_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, nvram_status);
			logmessage(LOGNAME,
				   "aborting WAN setup for unit %d: cannot read configured protocol (status=%d)",
				   unit, nvram_status);
			continue;
		}
		
		/*
		 * Do not let nvram_safe_get() turn a temporary NVRAM read/close error
		 * into an empty interface name and a successful no-op WAN startup.
		 * Preserve the legacy empty/unconfigured interface behavior, but make
		 * backend errors and invalid interface tokens visible to PID 1.
		 */
		ifname_status = get_wan_unit_value_copy_status(
			unit, "ifname", wan_ifname_nvram, sizeof(wan_ifname_nvram));
		if (ifname_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, ifname_status);
			logmessage(LOGNAME,
				   "aborting WAN setup for unit %d: cannot read configured interface name (status=%d)",
				   unit, ifname_status);
			continue;
		}
		if (!wan_ifname_nvram[0])
			continue;
		ifname_status = wan_ifname_copy(
			wan_ifname, sizeof(wan_ifname), wan_ifname_nvram);
		if (ifname_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, ifname_status);
			logmessage(LOGNAME,
				   "aborting WAN setup for unit %d: invalid configured interface name (status=%d)",
				   unit, ifname_status);
			continue;
		}

#if defined (USE_USB_SUPPORT)
		/* Do not configure ordinary PPP/Ethernet when modem identity is unknown. */
		nvram_status = get_usb_modem_wan_status(unit, &is_usb_modem);
		if (nvram_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, nvram_status);
			logmessage(LOGNAME,
				   "aborting WAN setup for unit %d: cannot read USB modem identity (status=%d)",
				   unit, nvram_status);
			continue;
		}
#endif
		
		/* Read the configured MTU before applying the physical default. */
		nvram_status = get_wan_unit_value_int_status(
			unit, "mtu", &wan_mtu);
		if (nvram_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, nvram_status);
			logmessage(LOGNAME,
				   "aborting WAN setup for unit %d: cannot read configured MTU (status=%d)",
				   unit, nvram_status);
			continue;
		}

		/*
		 * Static IPoE applies its requested MTU later, immediately before
		 * publishing the address. Do not let a redundant 1500-byte request
		 * mask a successful configured MTU or report a false startup error.
		 */
		if (wan_default_mtu_should_be_set(
			wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, wan_mtu)) {
			mtu_status = set_interface_mtu(wan_ifname, 1500);
			if (mtu_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, mtu_status);
				logmessage(LOGNAME, "WAN interface %s default MTU setup incomplete (status=%d)",
					   wan_ifname, mtu_status);
			}
		}
		interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		if (interface_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, interface_status);
			logmessage(LOGNAME, "aborting WAN startup for unit %d: failed to bring up physical interface %s (status=%d)",
				   unit, wan_ifname, interface_status);
			interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
			if (interface_cleanup_status != 0)
				logmessage(LOGNAME, "failed to keep physical WAN interface %s down after startup failure (status=%d)",
					   wan_ifname, interface_cleanup_status);
			continue;
		}
		
		/* Perform AP-client reconnect and retain driver command failures. */
		vlan_result = reconnect_apcli(wan_ifname, 0);
		if (vlan_result != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, vlan_result);
			logmessage(LOGNAME, "WAN AP-client reconnect incomplete on %s (status=%d)",
				   wan_ifname, vlan_result);
		}
		
#if defined (USE_USB_SUPPORT)
		if (is_usb_modem)
		{
			nvram_status = get_nvram_int_checked_status(
				"modem_type", &modem_type);
			if (nvram_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, nvram_status);
				logmessage(LOGNAME,
					   "aborting USB modem WAN startup: cannot determine modem transport type (status=%d)",
					   nvram_status);
				vlan_result = restore_wan_after_firewall_failure(
					unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			if (modem_type == 3)
			{
				vlan_result = launch_wan_usbnet(unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0)
					logmessage(LOGNAME, "USB modem WAN startup incomplete (status=%d)",
						   vlan_result);
			}
			else
			{
				char *ppp_ifname = IFNAME_RAS;
				
				vlan_result = check_upnp_wanif_changed(ppp_ifname);
				if (vlan_result != 0) {
					logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
						   unit, vlan_result);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				vlan_result = rc_wan_nvram_publish_runtime_identity(
					set_wan_unit_value_status_callback, &unit,
					"RAS Modem", ppp_ifname);
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME, "aborting USB modem RAS startup for unit %d: cannot publish active WAN interface %s (status=%d)",
						   unit, ppp_ifname, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				
				if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
				    wan_proto == IPV4_WAN_PROTO_PPTP ||
				    wan_proto == IPV4_WAN_PROTO_L2TP)
				{
					int i_pppoe, i_pppoe_man, wanx_status;
					
					i_pppoe = (wan_proto == IPV4_WAN_PROTO_PPPOE) ? 1 : 0;
					nvram_status = get_wan_unit_value_int_status(
						unit, "pppoe_man", &i_pppoe_man);
					if (nvram_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, nvram_status);
						logmessage(LOGNAME,
							   "aborting USB modem RAS startup: cannot read PPPoE manual mode (status=%d)",
							   nvram_status);
						vlan_result = restore_wan_after_firewall_failure(
							unit, wan_ifname);
						wan_start_status = rc_first_service_start_status(
							wan_start_status, vlan_result);
						continue;
					}
					wanx_status = 0;
					if (!i_pppoe || i_pppoe_man == 1)
						wanx_status = launch_wanx(wan_ifname, unit, 0, 0);
					else if (i_pppoe && i_pppoe_man == 2)
						wanx_status = launch_wanx(wan_ifname, unit, 0, 1);
										if (wanx_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, wanx_status);
						logmessage(LOGNAME, "WAN physical interface setup incomplete on %s (status=%d)",
							wan_ifname, wanx_status);
					}
				}
				
				/* Rebuild required firewall policy before exposing the RAS client. */
				vlan_result = start_firewall_ex();
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0)
					logmessage(LOGNAME, "USB modem RAS firewall startup incomplete (status=%d)",
						   vlan_result);
				if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
					logmessage(LOGNAME, "aborting USB modem RAS startup for unit %d because the firewall rebuild failed (status=%d)",
						   unit, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				
				vlan_result = launch_wan_modem_ras(unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0) {
					logmessage(LOGNAME, "USB modem RAS startup incomplete (status=%d)",
						   vlan_result);
					/*
					 * RAS launch failure leaves no usable PPP client. Restore the
					 * physical WAN binding and firewall before returning to callers.
					 * If recovery itself fails, the helper keeps the interface down.
					 */
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					if (vlan_result != 0)
						logmessage(LOGNAME,
							   "WAN recovery after USB modem RAS launch failure incomplete (status=%d)",
							   vlan_result);
				}
			}
		}
		else
#endif
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP)
		{
			char *proto_desc, *ppp_ifname = IFNAME_PPP;
			int i_pppoe, i_pppoe_man, i_demand, wanx_status;
			
			if (wan_proto == IPV4_WAN_PROTO_PPPOE)
				proto_desc = "PPPoE";
			else if (wan_proto == IPV4_WAN_PROTO_PPTP)
				proto_desc = "PPTP";
			else
				proto_desc = "L2TP";
			
			vlan_result = check_upnp_wanif_changed(ppp_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
					   unit, vlan_result);
				continue;
			}
			vlan_result = rc_wan_nvram_publish_runtime_identity(
				set_wan_unit_value_status_callback, &unit,
				proto_desc, ppp_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d: cannot publish active WAN interface %s (status=%d)",
					   unit, ppp_ifname, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			i_pppoe = (wan_proto == IPV4_WAN_PROTO_PPPOE) ? 1 : 0;
			nvram_status = get_wan_unit_value_int_status(
				unit, "pppoe_man", &i_pppoe_man);
			if (nvram_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, nvram_status);
				logmessage(LOGNAME,
					   "aborting WAN PPP startup: cannot read PPPoE manual mode (status=%d)",
					   nvram_status);
				vlan_result = restore_wan_after_firewall_failure(
					unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			wanx_status = 0;
			if (!i_pppoe || i_pppoe_man == 1)
				wanx_status = launch_wanx(wan_ifname, unit, !i_pppoe, 0);
			else if (i_pppoe && i_pppoe_man == 2)
				wanx_status = launch_wanx(wan_ifname, unit, 0, 1);
			if (wanx_status != 0) {
				/* PPPoE can start without a physical IPv4 lease. Keep pppd best-effort,
				 * but report failure of the explicitly configured physical WAN path. */
				wan_start_status = rc_first_service_start_status(
					wan_start_status, wanx_status);
				logmessage(LOGNAME, "WAN physical interface setup incomplete on %s (status=%d)",
					   wan_ifname, wanx_status);
			}
			if (!i_pppoe && wanx_status != 0) {
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d: physical WAN setup failed (status=%d)",
					   unit, wanx_status);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
				continue;
			}
			
			/* Rebuild required firewall policy before launching the PPP client. */
			vlan_result = start_firewall_ex();
			wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
			if (vlan_result != 0)
				logmessage(LOGNAME, "WAN PPP firewall startup incomplete (status=%d)", vlan_result);
			if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d because the firewall rebuild failed (status=%d)",
					   unit, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			/* update demand option only from a successful NVRAM read. */
			nvram_status = get_wan_unit_value_int_status(
				unit, "pppoe_idletime", &i_demand);
			if (nvram_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, nvram_status);
				logmessage(LOGNAME,
					   "aborting WAN PPP startup: cannot read PPP idle-time setting (status=%d)",
					   nvram_status);
				vlan_result = restore_wan_after_firewall_failure(
					unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			if (!i_pppoe || i_demand < 0)
				i_demand = 0;
			
			if (i_demand)
				i_demand = 1;
			
			/*
			 * pppd's options generator reads this volatile value. Do not
			 * launch with a stale demand setting if its publication fails.
			 */
			vlan_result = set_wan_unit_value_int_status(
				unit, "pppoe_demand", i_demand);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME,
					   "aborting WAN PPP startup for unit %d: cannot publish PPPoE demand option (status=%d)",
					   unit, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(
					unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			/* Launch the PPP client only after the options file is finalized. */
			vlan_result = launch_wan_pppd(unit, wan_proto);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d after launch failure (status=%d)",
					   unit, vlan_result);
				/* Do not leave a phantom ppp0 interface in firewall/UPnP state. */
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
				continue;
			}

			/* Pretend that the WAN interface is up */
			if (i_demand) {
				vlan_result = wait_ppp_up(ppp_ifname, unit);
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME, "aborting WAN PPP startup for unit %d because the demand interface or route setup is incomplete (status=%d)",
						   unit, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
					continue;
				}
			}
		}
		else if (wan_proto == IPV4_WAN_PROTO_IPOE_DHCP ||
			 wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)
		{
			int wan_auth_mode, wan_err = 0, wan_upnp_status, auth_status, dhcp_status;
			int firewall_cleanup_status;
			
			/* Configure DHCP connection. */
			vlan_result = check_upnp_wanif_changed(wan_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
					   unit, vlan_result);
				continue;
			}
			vlan_result = rc_wan_nvram_publish_runtime_identity(
				set_wan_unit_value_status_callback, &unit,
				"IPoE", wan_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting IPoE WAN startup for unit %d: cannot publish active WAN interface %s (status=%d)",
					   unit, wan_ifname, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			/* Do not publish static/DHCP WAN state until WISP association is ready. */
			if (is_man_wisp(wan_ifname)) {
				vlan_result = rc_wan_apcli_wait_status(
					wait_apcli_connected(wan_ifname, 3));
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME,
						   "aborting IPoE WAN activation because AP-client interface %s did not associate (status=%d)",
						   wan_ifname, vlan_result);
					vlan_result = set_wan_unit_error_status(unit, 1);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
			}
			
			/* Assign static IP address to i/f */
			if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
				int mtu_status, address_status, clear_status, snapshot_status;
				char wan_addr[16], wan_mask[16], lan_addr[16], lan_mask[16];

				snapshot_status = wan_static_ipv4_snapshot(
					unit, "ipaddr", "netmask",
					wan_addr, sizeof(wan_addr), wan_mask, sizeof(wan_mask),
					lan_addr, sizeof(lan_addr), lan_mask, sizeof(lan_mask));
				if (snapshot_status != 0) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, snapshot_status);
					logmessage(LOGNAME,
						   "cannot snapshot static WAN/LAN IPv4 configuration on %s (status=%d)",
						   wan_ifname, snapshot_status);
				} else if (!dhcp_lease_ipv4_is_valid(wan_addr) ||
					   !dhcp_lease_netmask_is_valid(wan_mask)) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, EINVAL);
					logmessage(LOGNAME, "invalid static WAN IPv4 address or netmask on %s",
						   wan_ifname);
				} else if (is_same_subnet2(wan_addr, lan_addr, wan_mask, lan_mask)) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, EADDRINUSE);
					logmessage(LOGNAME, "%s (%s) ERROR! Static IP [%s/%s] in LAN subnet [%s/%s]!",
						"WAN", wan_ifname, wan_addr, wan_mask, lan_addr, lan_mask);
				} else {
					if (wan_mtu >= 1300 && wan_mtu < 1500) {
						mtu_status = set_interface_mtu(wan_ifname, wan_mtu);
						if (mtu_status != 0) {
							wan_err = 1;
							wan_start_status = rc_first_service_start_status(
								wan_start_status, mtu_status);
							logmessage(LOGNAME, "static WAN interface %s MTU setup failed (status=%d); skipping address and route setup",
								   wan_ifname, mtu_status);
						}
					}
					
					/* Do not publish an address or route using a requested MTU we failed to apply. */
					if (!wan_err) {
						address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);
						if (address_status != 0) {
							wan_err = 1;
							wan_start_status = rc_first_service_start_status(
								wan_start_status, address_status);
							logmessage(LOGNAME, "failed to configure static WAN address on %s (status=%d)",
								   wan_ifname, address_status);
							clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
							if (rc_wan_static_ipoe_clear_failure_requires_ifdown(
								wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, clear_status)) {
								static_address_cleanup_failed = 1;
								wan_start_status = rc_first_service_start_status(
									wan_start_status, clear_status);
								logmessage(LOGNAME, "failed to clear partially configured static WAN address on %s (status=%d); forcing interface down",
									   wan_ifname, clear_status);
								interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
								if (interface_cleanup_status != 0) {
									wan_start_status = rc_first_service_start_status(
										wan_start_status, interface_cleanup_status);
									logmessage(LOGNAME, "failed to take static WAN interface %s down after partial address cleanup failure (status=%d)",
										   wan_ifname, interface_cleanup_status);
								}
							}
						}
					}
				}
				
				vlan_result = set_wan_unit_error_status(unit, wan_err);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
			}
			
			/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */
			vlan_result = start_firewall_ex();
			wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
			if (vlan_result != 0)
				logmessage(LOGNAME, "WAN IPoE firewall startup incomplete (status=%d)", vlan_result);
			if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
				wan_err = 1;
				logmessage(LOGNAME, "aborting WAN IPoE protocol startup on %s because the firewall rebuild failed (status=%d)",
					   wan_ifname, vlan_result);
				if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
					firewall_cleanup_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
					if (firewall_cleanup_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, firewall_cleanup_status);
						logmessage(LOGNAME, "failed to clear static WAN address after firewall failure on %s (status=%d); forcing interface down",
							   wan_ifname, firewall_cleanup_status);
						interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
						if (interface_cleanup_status != 0) {
							wan_start_status = rc_first_service_start_status(
								wan_start_status, interface_cleanup_status);
							logmessage(LOGNAME, "failed to take static WAN interface %s down after firewall failure (status=%d)",
								   wan_ifname, interface_cleanup_status);
						}
					}
				}
				vlan_result = set_wan_unit_error_status(unit, 1);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			if (static_address_cleanup_failed) {
				logmessage(LOGNAME, "skipping IPoE protocol activation after static address cleanup failure on %s",
					   wan_ifname);
				continue;
			}

			/* Start eapol authenticator */
			nvram_status = get_wan_unit_value_int_status(
				unit, "auth_mode", &wan_auth_mode);
			auth_status = nvram_status;
			if (nvram_status == 0 && wan_auth_mode > 1)
				auth_status = start_auth_eapol(
					wan_ifname, unit, wan_auth_mode - 2);
			if (auth_status != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, auth_status);
					logmessage(LOGNAME, "WAN EAPOL authentication startup failed on %s (status=%d); skipping address/lease setup",
						   wan_ifname, auth_status);
					vlan_result = set_wan_unit_error_status(unit, 1);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					static_address_clear_status = 0;
					if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)
						static_address_clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
					if (rc_wan_static_ipoe_clear_failure_requires_ifdown(
						wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, static_address_clear_status)) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, static_address_clear_status);
						logmessage(LOGNAME, "failed to clear static WAN address after EAPOL startup failure on %s (status=%d); forcing interface down",
							   wan_ifname, static_address_clear_status);
						interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
						if (interface_cleanup_status != 0) {
							wan_start_status = rc_first_service_start_status(
								wan_start_status, interface_cleanup_status);
							logmessage(LOGNAME, "failed to take static WAN interface %s down after EAPOL address cleanup failure (status=%d)",
								   wan_ifname, interface_cleanup_status);
						}
					}
					continue;
				}

			/* We are done configuration */
			if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
				if (!wan_err) {
					wan_up_status = wan_up(wan_ifname, unit, 1);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, wan_up_status);
					if (wan_up_status != 0)
						logmessage(LOGNAME, "static WAN route/service setup incomplete on %s (status=%d)",
							   wan_ifname, wan_up_status);
					/* Update UPnP forwards after WAN setup; do not hide reload failure. */
					wan_upnp_status = update_upnp();
					if (wan_upnp_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, wan_upnp_status);
						logmessage(LOGNAME, "UPnP update incomplete after WAN startup (status=%d)", wan_upnp_status);
					}
				}
			} else {
				dhcp_status = start_udhcpc_wan(wan_ifname, unit, 0);
				if (dhcp_status != 0) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, dhcp_status);
					logmessage(LOGNAME, "WAN DHCP client launch failed on %s (status=%d)",
						   wan_ifname, dhcp_status);
				}
				vlan_result = set_wan_unit_error_status(unit, wan_err);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
			}
#if defined (USE_IPV6)
			ipv6_type_status = is_wan_ipv6_type_sit_status(&ipv6_is_sit);
			if (ipv6_type_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, ipv6_type_status);
				logmessage(LOGNAME,
					   "cannot determine WAN IPv6 transport for %s (status=%d)",
					   wan_ifname, ipv6_type_status);
			} else if (ipv6_is_sit == 0) {
				int ipv6_status = wan6_up(wan_ifname, unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, ipv6_status);
				if (ipv6_status != 0)
					logmessage(LOGNAME, "WAN IPv6 setup incomplete on %s (status=%d)",
						   wan_ifname, ipv6_status);
			}
#endif
		}
	}

	{
		int passthrough_status = set_passthrough_pppoe(1);
		if (passthrough_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, passthrough_status);
			logmessage(LOGNAME, "PPPoE passthrough startup incomplete (status=%d)",
				   passthrough_status);
		}
	}

	return wan_start_status;
}

static int
stop_wan_ppp_locked(void)
{
	char* svcs_ppp[] = {
		"ip-up", "ip-down",
#if defined (USE_IPV6)
		"ipv6-up", "ipv6-down",
#endif
		"l2tpd", "xl2tpd", "pppd", NULL
	};
	int pause_status, stop_status = 0, resume_status;

	/* A running monitor must be paused before fencing or stopping PPP. */
	pause_status = notify_pause_detect_internet();
	if (pause_status != 0) {
		logmessage(LOGNAME,
			   "refusing to stop WAN PPP because Internet monitor pause failed (status=%d)",
			   pause_status);
		return pause_status;
	}
	stop_status = invalidate_wan_ppp_callback_session();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing to stop WAN PPP because callback-session fencing failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = clear_deferred_wan_reconnect_status();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because deferred reconnect cancellation failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = stop_vpn_client_under_lifecycle_lock();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN PPP teardown because VPN client stop was incomplete (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = kill_services(svcs_ppp, 6, 1);
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing to clear WAN PPP state while PPP services remain active (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = clear_wan_state_status();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN PPP teardown because VPN runtime state clear failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	control_wan_led_isp_state(0, 0);
	return 0;

abort_resume_detect:
	resume_status = notify_run_detect_internet(2);
	if (resume_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor resume failed after aborted WAN PPP teardown (status=%d)",
			   resume_status);
	return wan_teardown_abort_status(stop_status, resume_status);
}

static int
wan_teardown_ifdown_interface(void *context, const char *ifname)
{
	int ret;

	(void)context;
	ret = ifconfig(ifname, 0, "0.0.0.0", NULL);
	if (ret == ENODEV)
		return 0; /* The interface disappeared and is already unavailable. */
	if (ret != 0)
		logmessage(LOGNAME, "failed to bring WAN interface %s down (status=%d)", ifname, ret);

	return ret;
}

static const struct wan_teardown_interface_ops wan_teardown_ifdown_ops = {
	wan_teardown_ifdown_interface
};

static int
stop_wan_locked(void)
{
	int unit, wan_proto = 0, proto_status, ifdown_status, stop_status = -1;
	int callback_cleanup_status, modem_status, ifname_status;
	int is_usb_modem, is_ndis_modem = 0;
	int manual_ifname_is_current_wan = 0;
	int pause_status, eapol_stop_status, kabinet_stop_status, resume_status;
#if defined (USE_IPV6)
	int ipv6_status = 0, ipv6_type_status, ipv6_is_sit;
#endif
	char man_ifname[16], man_ifname_nvram[16];
	char wan_ifname_nvram[16], wan_ifname_temp[16] = "";
	const char *usbnet_ifname;
	char* svcs_ppp[] = {
		"ip-up", "ip-down",
#if defined (USE_IPV6)
		"ipv6-up", "ipv6-down",
#endif
		"l2tpd", "xl2tpd", "pppd", NULL
	};
	char* svcs_wan[] = {
		"ntpd", "inadyn", "igmpproxy", "xupnpd", "udpxy", "msd_lite",
		"zcip", "pppoe-relay", "detect_wan", NULL
	};
	const char *udhcpc_pidfiles[] = {
		"/var/run/udhcpc0.pid", "/var/run/udhcpc_viptv.pid", NULL
	};

	unit = 0;
	modem_status = get_usb_modem_wan_status(unit, &is_usb_modem);
	if (modem_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because USB modem identity cannot be read (status=%d)",
			   modem_status);
		errno = modem_status;
		return modem_status;
	}
	if (is_usb_modem) {
		modem_status = get_wan_modem_is_ndis_status(&is_ndis_modem);
		if (modem_status != 0) {
			logmessage(LOGNAME,
				   "refusing WAN teardown because USB modem transport type cannot be read (status=%d)",
				   modem_status);
			errno = modem_status;
			return modem_status;
		}
	}

	/*
	 * Resolve and validate the manual interface before pausing monitors or
	 * cancelling callbacks. A lossy nvram_safe_get() here could turn an I/O
	 * failure into an empty name and let a partial teardown proceed.
	 * Preserve the legacy empty/unconfigured interface as an empty string.
	 */
	ifname_status = get_wan_unit_value_copy_status(
		unit, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
	if (ifname_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because manual interface name cannot be read (status=%d)",
			   ifname_status);
		errno = ifname_status;
		return ifname_status;
	}
	if (man_ifname_nvram[0]) {
		ifname_status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (ifname_status != 0) {
			logmessage(LOGNAME,
				   "refusing WAN teardown because manual interface name is invalid (status=%d)",
				   ifname_status);
			errno = ifname_status;
			return ifname_status;
		}
	} else {
		man_ifname[0] = '\0';
	}
	/*
	 * Snapshot the effective transport interface before pausing monitors or
	 * cancelling callbacks. A lossy NVRAM read here can skip static-route
	 * teardown or leave an NDIS interface outside the ifdown inventory.
	 */
	ifname_status = get_wan_unit_value_copy_status(
		unit, "ifname_t", wan_ifname_nvram, sizeof(wan_ifname_nvram));
	if (ifname_status == 0 && wan_ifname_nvram[0])
		ifname_status = wan_ifname_copy(
			wan_ifname_temp, sizeof(wan_ifname_temp), wan_ifname_nvram);
	ifname_status = wan_teardown_select_interfaces(
		ifname_status, man_ifname, wan_ifname_temp, is_usb_modem,
		is_ndis_modem, &manual_ifname_is_current_wan, &usbnet_ifname);
	if (ifname_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because transport interface cannot be read or validated (status=%d)",
			   ifname_status);
		errno = ifname_status;
		return ifname_status;
	}
	pause_status = notify_pause_detect_internet();
	if (pause_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because Internet monitor pause failed (status=%d)",
			   pause_status);
		return pause_status;
	}
	stop_status = invalidate_wan_ppp_callback_session();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because PPP callback-session fencing failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = clear_deferred_wan_reconnect_status();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because deferred reconnect cancellation failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (proto_status != 0) {
		logmessage(LOGNAME,
			   "cannot determine WAN protocol during stop; continuing conservative teardown (status=%d)",
			   proto_status);
		}
	stop_status = stop_vpn_client_under_lifecycle_lock();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN PPP teardown because VPN client stop was incomplete (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}

	stop_status = kill_services(svcs_ppp, 6, 1);
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN interface teardown while PPP services remain active (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
#if defined (USE_IPV6)
	ipv6_type_status = is_wan_ipv6_type_sit_status(&ipv6_is_sit);
	if (ipv6_type_status != 0) {
		/* wan6_down() still fences DHCPv6 before its checked read refuses
		 * an unsafe interface-selection guess. */
		ipv6_status = wan6_down(man_ifname, unit);
		stop_status = route_status_accumulate(ipv6_type_status, ipv6_status);
		logmessage(LOGNAME,
			   "refusing WAN interface teardown because IPv6 service type cannot be read (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	if (ipv6_is_sit != 1) {
		ipv6_status = wan6_down(man_ifname, unit);
		if (ipv6_status != 0) {
			logmessage(LOGNAME, "WAN IPv6 teardown incomplete on %s (status=%d)",
				   man_ifname, ipv6_status);
			logmessage(LOGNAME, "refusing WAN interface teardown while DHCPv6 callback fencing is incomplete");
			stop_status = ipv6_status;
			goto abort_resume_detect;
		}
	}
#endif
	/* Release only WAN/VIPTV DHCP clients; never signal the LAN DHCP client. */
	if (process_pidfile_signal(udhcpc_pidfiles[0], "udhcpc", SIGUSR2) != 0 &&
	    errno != ENOENT && errno != ESRCH) {
		logmessage(LOGNAME, "refusing WAN teardown: WAN DHCP PID identity is unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (process_pidfile_signal(udhcpc_pidfiles[1], "udhcpc", SIGUSR2) != 0 &&
	    errno != ENOENT && errno != ESRCH) {
		logmessage(LOGNAME, "refusing WAN teardown: VIPTV DHCP PID identity is unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (service_wait_microseconds(300000U) != 0) {
		stop_status = errno > 0 ? errno : EIO;
		logmessage(LOGNAME, "refusing WAN teardown: DHCP deconfig grace wait failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	/*
	 * udhcpc owns retries for a failed deconfig callback (default -A is 20s).
	 * Never SIGKILL it: wait once for its callback retry and abort teardown
	 * on timeout. Stop unrelated WAN daemons separately so they retain the
	 * normal bounded TERM/KILL policy.
	 */
	if (kill_services_with_pidfiles(NULL, udhcpc_pidfiles, "udhcpc", 0, 30, 0) != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown while DHCP deconfig is active or unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (kill_services(svcs_wan, 3, 1) != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown while WAN services remain active");
		stop_status = -1;
		goto abort_resume_detect;
	}

	eapol_stop_status = stop_auth_eapol();
	if (eapol_stop_status != 0)
		logmessage(LOGNAME, "WAN teardown could not stop EAPOL authentication (status=%d)", eapol_stop_status);
	kabinet_stop_status = stop_auth_kabinet();
	if (kabinet_stop_status != 0)
		logmessage(LOGNAME, "WAN teardown could not stop kabinet authentication (status=%d)", kabinet_stop_status);
	if (eapol_stop_status != 0 || kabinet_stop_status != 0) {
		stop_status = eapol_stop_status != 0 ? eapol_stop_status : kabinet_stop_status;
		goto abort_resume_detect;
	}
	stop_status = set_passthrough_pppoe(0);
	if (stop_status != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown because PPPoE relay did not stop (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	if ((proto_status != 0 || wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) &&
	    manual_ifname_is_current_wan) {
		stop_status = wan_down(man_ifname, unit, 1);
		if (stop_status != 0)
			logmessage(LOGNAME, "static WAN route teardown incomplete on %s (status=%d)",
				   man_ifname, stop_status);
	}
#if defined (USE_USB_SUPPORT)
	{
		int modem_stop_status = stop_wan_usbnet();
		stop_status = rc_first_service_start_status(stop_status, modem_stop_status);
	}
#endif
	ifdown_status = wan_teardown_ifdown_interfaces(man_ifname, IFNAME_WAN,
						       usbnet_ifname,
						       &wan_teardown_ifdown_ops, NULL);
	callback_cleanup_status = remove_cb_links();
	if (callback_cleanup_status != 0)
		logmessage(LOGNAME, "WAN callback link cleanup incomplete (status=%d)", callback_cleanup_status);
	stop_status = wan_teardown_first_error_status(
		stop_status, proto_status, ifdown_status, callback_cleanup_status);
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "WAN interface shutdown incomplete (stop=%d protocol=%d interface=%d callbacks=%d); skipping final WAN state clear",
			   stop_status, proto_status, ifdown_status, callback_cleanup_status);
		goto abort_resume_detect;
	}
#if defined (USE_IPV6)
	if (ipv6_status != 0) {
		logmessage(LOGNAME, "WAN IPv6 teardown incomplete; skipping final WAN state clear");
		stop_status = ipv6_status;
		goto abort_resume_detect;
	}
#endif
	stop_status = clear_wan_state_status();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN teardown because VPN runtime state clear failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	control_wan_led_isp_state(0, 0);
	return 0;

abort_resume_detect:
	/* Every failure exits before a confirmed complete teardown; restore the
	 * monitor so it can observe or recover the still-live/partial WAN. */
	resume_status = notify_run_detect_internet(2);
	if (resume_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor resume failed after aborted WAN teardown (status=%d)",
			   resume_status);
	return wan_teardown_abort_status(stop_status, resume_status);
}

static int
run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(LOGNAME, "refusing %s: cannot lock VPN server lifecycle (errno=%d)",
			   name, errno);
		return -1;
	}
	result = operation();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release VPN server lifecycle lock after %s (errno=%d)",
			   name, release_errno);
		if (result == 0) {
			result = -1;
			saved_errno = release_errno;
		}
	}
	if (result != 0 && saved_errno)
		errno = saved_errno;
	return result;
}

static int
stop_wan_ppp(void)
{
	return run_wan_stop_under_vpn_lifecycle_lock(stop_wan_ppp_locked, "WAN PPP stop");
}

int
stop_wan(void)
{
	return run_wan_stop_under_vpn_lifecycle_lock(stop_wan_locked, "WAN stop");
}

static int
get_wan_unit(char *wan_ifname)
{
	int unit;

	for (unit = 0; unit < 1; unit++) {
		if (strcmp(wan_ifname, get_wan_unit_value(unit, "ifname_t")) == 0)
			return unit;
		if (strcmp(wan_ifname, get_man_ifname(unit)) == 0)
			return unit;
	}

	/* fallback to unit 0 */
	return 0;
}

static int
is_ifunit_man(char *wan_ifname, int unit)
{
	int wan_proto = get_wan_unit_proto_for_manual_check(unit);
		if (wan_proto < 0)
			return 1;

	if (!is_usbnet_interface(wan_ifname) &&
	    (wan_proto == IPV4_WAN_PROTO_PPPOE ||
	     wan_proto == IPV4_WAN_PROTO_PPTP ||
	     wan_proto == IPV4_WAN_PROTO_L2TP))
		return 1;

	return 0;
}

static int
is_ifunit_modem_status(char *wan_ifname, int unit, int *modem_unit_id)
{
	char ifname_temp[IFNAMSIZ];
	int is_modem, status;

	if (!wan_ifname || !modem_unit_id)
		return EINVAL;
	*modem_unit_id = 0;

	status = get_usb_modem_wan_status(unit, &is_modem);
	if (status != 0) {
		errno = status;
		return status;
	}
	if (!is_modem)
		return 0;

	if (ppp_ifindex(wan_ifname) >= RAS_PPP_UNIT) {
		*modem_unit_id = 1;
		return 0;
	}

	status = get_wan_unit_value_copy_status(
		unit, "ifname_t", ifname_temp, sizeof(ifname_temp));
	if (status != 0) {
		errno = status;
		return status;
	}
	if (is_usbnet_interface(ifname_temp) &&
	    strcmp(wan_ifname, ifname_temp) == 0)
		*modem_unit_id = 2;

	return 0;
}

static int
add_man_gateway_routes(char *man_ifname, int unit, int metric, int *gateway_route_status)
{
	int is_man_dns_auto, is_modem_active, wan_proto = 0, dns_enabled;
	int dns_mode_status, proto_status, modem_status;
	int status = 0, route_status, snapshot_status;
	int man_address_valid, man_netmask_valid, gateway_off_subnet;
	char man_gate_storage[16], man_addr_storage[16], man_mask_storage[16];
	char man_dns_storage[WAN_MAN_DNS_SNAPSHOT_MAX];
	char *next, *man_gate = man_gate_storage, *man_addr = man_addr_storage;
	char *man_mask = man_mask_storage, *man_dns = man_dns_storage;
	char word[100];

	if (gateway_route_status)
		*gateway_route_status = 0;

	snapshot_status = wan_man_gateway_snapshot(
		man_gate_storage, sizeof(man_gate_storage),
		man_addr_storage, sizeof(man_addr_storage),
		man_mask_storage, sizeof(man_mask_storage),
		man_dns_storage, sizeof(man_dns_storage));
	if (snapshot_status != 0) {
		if (gateway_route_status)
			*gateway_route_status = snapshot_status;
		logmessage(LOGNAME,
			   "cannot snapshot MAN gateway/address/DNS state on %s (status=%d)",
			   man_ifname, snapshot_status);
		return snapshot_status;
	}
	if (!is_valid_ipv4(man_gate))
		return 0;

	man_address_valid = dhcp_lease_ipv4_is_valid(man_addr);
	man_netmask_valid = dhcp_lease_netmask_is_valid(man_mask);
	if (!man_address_valid || !man_netmask_valid) {
		status = route_status_accumulate(status, EINVAL);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(
				*gateway_route_status, EINVAL);
		logmessage(LOGNAME,
			   "invalid MAN address/netmask snapshot on %s; skipping host and DNS gateway routes",
			   man_ifname);
	}

	proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (proto_status != 0) {
		status = route_status_accumulate(status, proto_status);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(
				*gateway_route_status, proto_status);
		logmessage(LOGNAME,
			   "cannot read WAN protocol for MAN DNS routes on %s; skipping optional DNS routes (status=%d)",
			   man_ifname, proto_status);
	}
	modem_status = get_usb_modem_wan_status(unit, &is_modem_active);
	if (modem_status != 0) {
		status = route_status_accumulate(status, modem_status);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(
				*gateway_route_status, modem_status);
		logmessage(LOGNAME,
			   "cannot read modem identity for MAN DNS routes on %s; skipping optional DNS route additions (status=%d)",
			   man_ifname, modem_status);
		is_modem_active = 1;
	}
	dns_mode_status = get_wan_unit_value_int_status(
		unit, "dnsenable_x", &dns_enabled);
	if (dns_mode_status != 0) {
		status = route_status_accumulate(status, dns_mode_status);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(
				*gateway_route_status, dns_mode_status);
		logmessage(LOGNAME,
			   "cannot read MAN DNS mode on %s; skipping optional DNS routes (status=%d)",
			   man_ifname, dns_mode_status);
		is_man_dns_auto = 0;
	} else {
		is_man_dns_auto = dns_enabled > 0;
	}

	/*
	 * Do not blindly roll back successful additions after a later failure:
	 * the kernel route operation cannot distinguish a route created here
	 * from an identical route already owned by another path. Continue all
	 * attempts, report the first error, and let matching teardown remove the
	 * scoped route set.
	 */
	/* Do not interpret inet_addr() failures as an off-subnet gateway. */
	gateway_off_subnet = man_address_valid && man_netmask_valid &&
		!is_same_subnet(man_gate, man_addr, man_mask);
	if (route_status_gateway_host_route_required(
		    1, man_address_valid, man_netmask_valid,
		    gateway_off_subnet)) {
		route_status = route_add(man_ifname, metric, man_gate, NULL, "255.255.255.255");
		status = route_status_accumulate(status, route_status);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(*gateway_route_status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add MAN gateway host route %s on %s (error=%d)",
				   man_gate, man_ifname, route_status);
	}

	/* default route via default gateway */
	route_status = route_add(man_ifname, metric, "0.0.0.0", man_gate, "0.0.0.0");
	status = route_status_accumulate(status, route_status);
	if (gateway_route_status)
		*gateway_route_status = route_status_accumulate(*gateway_route_status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add MAN default route via %s on %s (error=%d)",
			   man_gate, man_ifname, route_status);

	/* ... and to dns servers as well for demand ppp to work */
	if (wan_man_dns_route_add_allowed(proto_status,
					     wan_proto == IPV4_WAN_PROTO_PPPOE,
					     is_man_dns_auto, is_modem_active)) {
		foreach(word, man_dns, next) {
			if (is_valid_ipv4(word) && inet_addr(word) != inet_addr(man_gate) &&
			    man_address_valid && man_netmask_valid &&
			    !is_same_subnet(word, man_addr, man_mask)) {
				route_status = route_add(man_ifname, metric, word, man_gate, "255.255.255.255");
				status = route_status_accumulate(status, route_status);
				if (route_status != 0)
					logmessage(LOGNAME, "failed to add MAN DNS route %s via %s on %s (error=%d)",
						   word, man_gate, man_ifname, route_status);
			}
		}
	}

	return status;
}

static int
queue_wan_firewall_restart(const char *context)
{
	int status = notify_rc_status("restart_firewall_wan");

	if (status != 0)
		logmessage(LOGNAME,
			   "could not queue WAN firewall refresh after %s (errno=%d)",
			   context, errno ? errno : EIO);
	return status;
}

static int
man_up_internal(char *man_ifname, int unit, int is_static, int *gateway_route_status)
{
	int status = 0, route_status;

	if (gateway_route_status)
		*gateway_route_status = 0;

	logmessage(LOGNAME, "%s %s (%s)", "MAN", "up", man_ifname);

	/* setup static wan routes via physical device */
	route_status = add_static_man_routes(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add static MAN routes on %s (error=%d)",
			   man_ifname, route_status);

	if (!is_static) {
		/* and routes supplied via DHCP */
		route_status = add_dhcp_routes_by_prefix("wanx_", man_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add DHCP MAN routes on %s (error=%d)",
				   man_ifname, route_status);
	}

	/* and default route (metric 2) */
	route_status = add_man_gateway_routes(man_ifname, unit, 2, gateway_route_status);
	status = route_status_accumulate(status, route_status);

	if (!is_static) {
		/* A resolver refresh error must not tear down this live link. */
		refresh_resolvconf_nonfatal("MAN up", 0);

		/* re-start firewall */
		route_status = queue_wan_firewall_restart("MAN up");
		status = route_status_accumulate(status, route_status);
	}

	/* Start multicast services and preserve their status for DHCP callers. */
	route_status = start_igmpproxy(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "MAN multicast startup incomplete on %s (error=%d)",
			   man_ifname, route_status);

	return status;
}

void
man_up(char *man_ifname, int unit, int is_static)
{
	int status = man_up_internal(man_ifname, unit, is_static, NULL);

	if (status != 0)
		logmessage(LOGNAME, "MAN route setup incomplete on %s (error=%d)",
			   man_ifname, status);
}

static int
man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)
{
	int wan_proto = 0, proto_status, is_modem_active, is_man_dns_auto, dns_enabled;
	int dns_mode_status, modem_status;
	int status = 0, route_status, snapshot_status;
	char man_gate_storage[16], man_addr_storage[16], man_mask_storage[16];
	char man_dns_storage[WAN_MAN_DNS_SNAPSHOT_MAX];
	char *next, *man_gate = man_gate_storage, *man_addr = man_addr_storage;
	char *man_mask = man_mask_storage, *man_dns = man_dns_storage;
	char word[100];

	static const struct rc_wan_nvram_value clear_address_values[] = {
		{ "wanx_ipaddr", "0.0.0.0" }
	};
	static const struct rc_wan_nvram_value clear_runtime_values[] = {
		{ "wanx_gateway", "" }, { "wanx_dns", "" },
		{ "wanx_domain", "" }, { "wanx_routes", "" },
		{ "wanx_routes_ms", "" }, { "wanx_routes_rfc", "" }
	};


	logmessage(LOGNAME, "%s %s (%s)", "MAN", "down", man_ifname);
	/*
	 * DHCP loss is already committed, so a monitor-pause failure must not
	 * prevent route/interface cleanup. Retain it as the callback's first
	 * error while continuing the idempotent teardown sequence.
	 */
	route_status = notify_pause_detect_internet();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor pause incomplete during MAN teardown (status=%d)",
			   route_status);
	snapshot_status = wan_man_gateway_snapshot(
		man_gate_storage, sizeof(man_gate_storage),
		man_addr_storage, sizeof(man_addr_storage),
		man_mask_storage, sizeof(man_mask_storage),
		man_dns_storage, sizeof(man_dns_storage));
	if (snapshot_status != 0) {
		status = route_status_accumulate(status, snapshot_status);
		logmessage(LOGNAME,
			   "cannot snapshot MAN gateway/address/DNS state during teardown on %s (status=%d)",
			   man_ifname, snapshot_status);
	}
	proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (proto_status != 0) {
		status = route_status_accumulate(status, proto_status);
		logmessage(LOGNAME,
			   "cannot read WAN protocol during MAN teardown on %s (status=%d); attempting conservative route cleanup",
			   man_ifname, proto_status);
	}
	modem_status = get_usb_modem_wan_status(unit, &is_modem_active);
	if (modem_status != 0) {
		status = route_status_accumulate(status, modem_status);
		logmessage(LOGNAME,
			   "cannot read modem identity during MAN teardown on %s (status=%d); attempting scoped DNS route cleanup",
			   man_ifname, modem_status);
		/* Unknown is not proof that this path owns no routes; cleanup is idempotent. */
		is_modem_active = 0;
	}
	dns_mode_status = get_wan_unit_value_int_status(
		unit, "dnsenable_x", &dns_enabled);
	if (dns_mode_status != 0) {
		status = route_status_accumulate(status, dns_mode_status);
		logmessage(LOGNAME,
			   "cannot read MAN DNS mode during teardown on %s (status=%d); attempting DNS route removal",
			   man_ifname, dns_mode_status);
		/* Cleanup is idempotent: attempt removal even when the mode is unknown. */
		is_man_dns_auto = 1;
	} else {
		is_man_dns_auto = dns_enabled > 0;
	}

	if (dhcp_lease_ipv4_is_valid(man_gate)) {
		if (dhcp_lease_ipv4_is_valid(man_addr) && dhcp_lease_netmask_is_valid(man_mask) &&
		    !is_same_subnet(man_gate, man_addr, man_mask)) {
			route_status = route_del(man_ifname, 2, man_gate, NULL, "255.255.255.255");
			if (!dhcp_lease_route_delete_is_ok(route_status)) {
				status = route_status_accumulate(status, route_status);
				logmessage(LOGNAME, "failed to delete MAN gateway host route %s on %s (error=%d)", man_gate, man_ifname, route_status);
			}
		}
		route_status = route_del(man_ifname, 2, "0.0.0.0", man_gate, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_status)) {
			status = route_status_accumulate(status, route_status);
			logmessage(LOGNAME, "failed to delete MAN default route via %s on %s (error=%d)", man_gate, man_ifname, route_status);
		}
		if (wan_man_dns_route_remove_allowed(proto_status,
						   wan_proto == IPV4_WAN_PROTO_PPPOE,
						   is_man_dns_auto, is_modem_active)) {
			foreach(word, man_dns, next) {
				if (is_valid_ipv4(word) && inet_addr(word) != inet_addr(man_gate) &&
				    dhcp_lease_ipv4_is_valid(man_addr) && dhcp_lease_netmask_is_valid(man_mask) &&
				    !is_same_subnet(word, man_addr, man_mask)) {
					route_status = route_del(man_ifname, 2, word, man_gate, "255.255.255.255");
					if (!dhcp_lease_route_delete_is_ok(route_status)) {
						status = route_status_accumulate(status, route_status);
						logmessage(LOGNAME, "failed to delete MAN DNS route %s via %s on %s (error=%d)", word, man_gate, man_ifname, route_status);
					}
				}
			}
		}
	}

	route_status = del_dhcp_routes_by_prefix("wanx_", man_ifname, 0);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete DHCP MAN routes on %s (error=%d)", man_ifname, route_status);
	route_status = del_static_man_routes(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete static MAN routes on %s (error=%d)", man_ifname, route_status);

	if (status == 0) {
		if (clear_dhcp_address) {
			route_status = rc_wan_nvram_set_all_values(
				clear_address_values,
				(unsigned int)(sizeof(clear_address_values) /
					       sizeof(clear_address_values[0])),
				set_nvram_snapshot_value_status, NULL);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME,
					   "failed to clear MAN DHCP address snapshot (status=%d)",
					   route_status);
		}
		/* Attempt every independent clear even if the address write failed. */
		route_status = rc_wan_nvram_set_all_values(
			clear_runtime_values,
			(unsigned int)(sizeof(clear_runtime_values) /
				       sizeof(clear_runtime_values[0])),
			set_nvram_snapshot_value_status, NULL);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME,
				   "MAN runtime DNS/route snapshot clear incomplete (status=%d)",
				   route_status);
	} else {
		logmessage(LOGNAME, "MAN route teardown incomplete on %s (error=%d); preserving lease and route inventory", man_ifname, status);
	}

	refresh_resolvconf_nonfatal("MAN down", 0);
	route_status = queue_wan_firewall_restart("MAN down");
	status = route_status_accumulate(status, route_status);
	route_status = stop_igmpproxy(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "MAN multicast teardown incomplete on %s (error=%d)",
			   man_ifname, route_status);
	return status;
}

void
man_down(char *man_ifname, int unit)
{
	int status = man_down_internal(man_ifname, unit, 0);

	if (status != 0)
		logmessage(LOGNAME, "MAN route teardown incomplete on %s (error=%d)",
			   man_ifname, status);
}

static int
wan_up_internal(char *wan_ifname, int unit, int is_static)
{
	char wan_cnt[32], wan_addr[16], wan_mask[16], wan_gate[16];
	const char *script_postw = SCRIPT_POST_WAN;
	int wan_proto = 0, modem_unit_id, modem_status, proto_status;
	int ppp_index, gateway_valid, gateway_off_subnet;
	int wan_address_valid, wan_netmask_valid;
	int status = 0, route_status, auth_status, auth_mode;
	int pppoemwan_status, pppoemwan_enabled;
	int gateway_arp_status, gateway_arp_enabled;
	int route_snapshot_status, wan_addr_status, wan_mask_status, wan_gate_status;
	int ifstats_status;
	uint64_t ifstats_rx_bytes, ifstats_tx_bytes;
#if defined (USE_IPV6)
	int ipv6_status, ipv6_type_status, ipv6_is_sit;
#endif

	logmessage(LOGNAME, "%s %s (%s)", "WAN", "up", wan_ifname);

	modem_status = is_ifunit_modem_status(wan_ifname, unit, &modem_unit_id);
	if (modem_status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN-up on %s because USB modem identity cannot be read (status=%d)",
			   wan_ifname, modem_status);
		errno = modem_status;
		return modem_status;
	}
	if (!modem_unit_id) {
		proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (proto_status != 0) {
			logmessage(LOGNAME,
				   "aborting WAN-up on %s because configured protocol cannot be read (status=%d)",
				   wan_ifname, proto_status);
			errno = proto_status;
			return proto_status;
		}
	}

	snprintf(wan_cnt, sizeof(wan_cnt), "%ld", uptime());
	set_wan_unit_value(unit, "uptime", wan_cnt);

	if (modem_unit_id || wan_proto != IPV4_WAN_PROTO_IPOE_STATIC) {
		ifstats_status = get_ifstats_bytes_status(
			wan_ifname, &ifstats_rx_bytes, &ifstats_tx_bytes);
		if (ifstats_status == 0) {
			snprintf(wan_cnt, sizeof(wan_cnt), "%llu", ifstats_rx_bytes);
			set_wan_unit_value(unit, "bytes_rx", wan_cnt);
			snprintf(wan_cnt, sizeof(wan_cnt), "%llu", ifstats_tx_bytes);
			set_wan_unit_value(unit, "bytes_tx", wan_cnt);
		} else {
			logmessage(LOGNAME,
				   "cannot establish WAN byte-counter baseline on %s (status=%d); preserving prior baseline",
				   wan_ifname, ifstats_status);
		}
	}

	/*
	 * Read gateway, address and netmask from one checked NVRAM generation.
	 * Independent nvram_safe_get()-backed reads can hide backend failures and
	 * mix DHCP renew generations while deciding whether a gateway /32 is needed.
	 */
	wan_addr[0] = '\0';
	wan_mask[0] = '\0';
	wan_gate[0] = '\0';
	route_snapshot_status = wan_unit_route_snapshot(
		unit, wan_gate, sizeof(wan_gate),
		wan_addr, sizeof(wan_addr),
		wan_mask, sizeof(wan_mask));
	if (route_snapshot_status != 0) {
		status = route_status_accumulate(status, route_snapshot_status);
		logmessage(LOGNAME,
			   "cannot read coherent WAN route snapshot during startup on %s (status=%d)",
			   wan_ifname, route_snapshot_status);
		wan_addr_status = route_snapshot_status;
		wan_mask_status = route_snapshot_status;

		/*
		 * Preserve the legacy default-route attempt when the tuple transaction
		 * fails, but do not infer gateway subnet ownership from mixed reads.
		 */
		wan_gate_status = get_wan_unit_value_copy_status(
			unit, "gateway", wan_gate, sizeof(wan_gate));
		if (wan_gate_status != 0) {
			status = route_status_accumulate(status, wan_gate_status);
			logmessage(LOGNAME,
				   "cannot read WAN gateway fallback during startup on %s (status=%d)",
				   wan_ifname, wan_gate_status);
		}
	} else {
		wan_addr_status = 0;
		wan_mask_status = 0;
		wan_gate_status = 0;
	}

	wan_address_valid = wan_addr_status == 0 &&
		dhcp_lease_ipv4_is_valid(wan_addr);
	wan_netmask_valid = wan_mask_status == 0 &&
		dhcp_lease_netmask_is_valid(wan_mask);
	if (wan_addr_status == 0 && !wan_address_valid) {
		wan_addr_status = EINVAL;
		status = route_status_accumulate(status, EINVAL);
		logmessage(LOGNAME,
			   "invalid WAN IPv4 address snapshot on %s; skipping gateway host route",
			   wan_ifname);
	}
	if (wan_mask_status == 0 && !wan_netmask_valid) {
		wan_mask_status = EINVAL;
		status = route_status_accumulate(status, EINVAL);
		logmessage(LOGNAME,
			   "invalid WAN netmask snapshot on %s; skipping gateway host route",
			   wan_ifname);
	}

	gateway_valid = wan_gate_status == 0 && is_valid_ipv4(wan_gate);
	ppp_index = ppp_ifindex(wan_ifname);
	gateway_off_subnet = gateway_valid && wan_address_valid &&
		wan_netmask_valid && ppp_index < 0 &&
		!is_same_subnet(wan_gate, wan_addr, wan_mask);

	/* Set the same off-subnet gateway host route that WAN teardown removes. */
	if (route_status_wan_gateway_host_route_required(
		    gateway_valid, wan_address_valid, wan_netmask_valid,
		    ppp_index, gateway_off_subnet)) {
		route_status = route_add(wan_ifname, 1, wan_gate, NULL, "255.255.255.255");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add WAN gateway host route %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
	}

	/* default route via default gateway */
	if (gateway_valid) {
		route_status = route_add(wan_ifname, 1, "0.0.0.0", wan_gate, "0.0.0.0");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add WAN default route via %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
	}

#if 0
	/* hack: avoid routing cycles, when both peer and server has the same IP (for PPTP or L2TP) */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_PPTP || wan_proto == IPV4_WAN_PROTO_L2TP)) {
		/* delete gateway route as it's no longer needed */
		if (gateway_valid)
			route_del(wan_ifname, 0, wan_gate, "0.0.0.0", "255.255.255.255");
	}
#endif

	/* Install interface dependent static routes. Keep attempting later setup
	 * steps after an individual route failure and preserve the first status. */
	route_status = add_static_wan_routes(wan_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add static WAN routes on %s (error=%d)",
			   wan_ifname, route_status);

	/* Publish the global MAN gateway before static routes use it as a fallback. */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		route_status = set_nvram_snapshot_value_status(
			NULL, "wanx_gateway", gateway_valid ? wan_gate : "0.0.0.0");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0) {
			logmessage(LOGNAME,
				   "failed to publish MAN gateway snapshot; skipping dependent static MAN routes (status=%d)",
				   route_status);
		} else {
			route_status = add_static_man_routes(wan_ifname);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "failed to add static MAN routes on %s (error=%d)",
					   wan_ifname, route_status);
		}
	}

	/* Add dynamic routes supplied via DHCP (for NDIS or IPoE Auto) */
	if ((modem_unit_id == 2) || (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		char prefix[16];
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		route_status = add_dhcp_routes_by_prefix(prefix, wan_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add DHCP WAN routes on %s (error=%d)",
				   wan_ifname, route_status);
	}

#if defined (USE_IPV6)
	ipv6_type_status = is_wan_ipv6_type_sit_status(&ipv6_is_sit);
	if (ipv6_type_status != 0) {
		status = route_status_accumulate(status, ipv6_type_status);
		logmessage(LOGNAME,
			   "cannot determine WAN IPv6 transport while bringing %s up (status=%d)",
			   wan_ifname, ipv6_type_status);
	} else if (ipv6_is_sit == 1) {
		ipv6_status = wan6_up(wan_ifname, unit);
		status = route_status_accumulate(status, ipv6_status);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN IPv6 tunnel setup incomplete on %s (status=%d)",
				   wan_ifname, ipv6_status);
	}
#endif

	/* Resolver refresh failure is non-fatal to the established WAN link. */
	refresh_resolvconf_nonfatal("WAN up", 0);

	if (!is_static) {
		/* re-start firewall */
		route_status = queue_wan_firewall_restart("WAN up");
		status = route_status_accumulate(status, route_status);
	}

	/* Start Kabinet only when the checked auth-mode read succeeds. */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		auth_status = get_wan_unit_value_int_status(unit, "auth_mode", &auth_mode);
		if (auth_status != 0) {
			status = route_status_accumulate(status, auth_status);
			logmessage(LOGNAME,
				   "cannot read WAN Kabinet authentication mode (status=%d)", auth_status);
		} else if (auth_mode == 1) {
			route_status = start_auth_kabinet();
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "WAN kabinet authentication startup incomplete (status=%d)", route_status);
		}
	}

	/* Start multicast router (for NDIS or IPoE) and retain its status. */
	if (ppp_ifindex(wan_ifname) < 0) {
		route_status = start_igmpproxy(wan_ifname);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN multicast startup incomplete on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Notify the DDNS client only when configured, and preserve any failure. */
	route_status = notify_ddns_update();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN DDNS client update incomplete on %s (status=%d)",
			   wan_ifname, route_status);

	/* Notify watchdog for NTP update and retain signalling/recovery errors. */
	route_status = notify_watchdog_time();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN watchdog time notification incomplete (status=%d)", route_status);

	/* Queue the deferred VPN-client startup and preserve notification errors. */
	route_status = notify_rc_status("start_vpn_client");
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN VPN client start notification incomplete (status=%d)",
			   route_status);

	/*
	 * A failed gateway-ARP enable read is not equivalent to a disabled setting:
	 * preserve the error in the WAN-up result rather than silently skipping
	 * the configured checker.
	 */
	if (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP) {
		gateway_arp_status = get_nvram_match_checked_status(
			"gw_arp_ping", "1", &gateway_arp_enabled);
		if (gateway_arp_status != 0) {
			status = route_status_accumulate(status, gateway_arp_status);
			logmessage(LOGNAME,
				   "cannot read gateway ARP checker enable setting (status=%d)",
				   gateway_arp_status);
		} else if (gateway_arp_enabled) {
			struct process_snapshot snapshot;
			enum process_presence presence;
			enum process_presence_start_action action;
			int scan_error;

			presence = process_inventory_scan("detect_wan", &snapshot);
			scan_error = snapshot.error;
			process_snapshot_release(&snapshot);
			action = process_presence_start_action_for_state(presence);
			route_status = 0;
			if (action == PROCESS_PRESENCE_START_NEW)
				route_status = eval("detect_wan");
			route_status = process_presence_start_status_for_action(
				action, scan_error, route_status);
			if (route_status != 0) {
				status = route_status_accumulate(status, route_status);
				if (action == PROCESS_PRESENCE_START_ABORT)
					logmessage(LOGNAME,
						   "skipping gateway ARP checker start because process inventory is unknown (status=%d)",
						   route_status);
				else
					logmessage(LOGNAME, "WAN gateway ARP checker startup incomplete (status=%d)",
						   route_status);
			}
		}
	}

	if (gateway_valid)
		control_wan_led_isp_state(1, modem_unit_id);

	/* DI wakeup after 2 secs; retain launch/inventory/signal errors. */
	route_status = notify_run_detect_internet(2);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN Internet monitor notification incomplete (status=%d)", route_status);

	/* Only invoke the optional hook with a complete, stable IPv4 snapshot. */
	if (check_if_file_exist(script_postw)) {
		if (wan_addr_status != 0) {
			logmessage(LOGNAME,
				   "skipping WAN post-up script because the IPv4 address snapshot is invalid (status=%d)",
				   wan_addr_status);
		} else {
			route_status = eval(script_postw, "up", wan_ifname, wan_addr);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "WAN post-up script incomplete on %s (status=%d)",
					   wan_ifname, route_status);
		}
	}
#if defined(APP_ZEROTIER)
	/*
	 * start_zerotier() owns the checked exact-string enable read and preserves
	 * the disabled no-op. Do not pre-gate it with an unchecked NVRAM matcher:
	 * a backend error must reach the WAN-up status accumulator.
	 */
	route_status = start_zerotier();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN ZeroTier startup incomplete (status=%d)",
			   route_status);
#endif
	pppoemwan_status = get_nvram_match_checked_status(
		"pppoemwan_enable", "1", &pppoemwan_enabled);
	if (pppoemwan_status != 0) {
		status = route_status_accumulate(status, pppoemwan_status);
		logmessage(LOGNAME,
			   "cannot read PPPoE-MWAN enable setting during WAN-up (status=%d)",
			   pppoemwan_status);
	} else if (pppoemwan_enabled) {
		route_status = eval("/usr/bin/detect.sh");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN PPPoE-MWAN detection incomplete (status=%d)",
				   route_status);
	}
	return status;
}

int
wan_up(char *wan_ifname, int unit, int is_static)
{
	int status = wan_up_internal(wan_ifname, unit, is_static);

	if (status != 0)
		logmessage(LOGNAME, "WAN route setup incomplete on %s (error=%d)",
			   wan_ifname, status);
	return status;
}

static int
wan_down_internal(char *wan_ifname, int unit, int is_static)
{
	static const struct rc_wan_nvram_value statistics_clear_values[] = {
		{ "uptime", "0" }, { "dltime", "0" },
		{ "bytes_rx", "0" }, { "bytes_tx", "0" }
	};
	char wan_addr[16], wan_gate[16], wan_mask[16];
	const char *script_postw = SCRIPT_POST_WAN;
	int wan_proto = 0, proto_status, modem_unit_id, modem_status;
	int ppp_index, gateway_valid, address_valid, netmask_valid, gateway_off_subnet;
	int status = 0, route_status, interface_cleanup_status;
	int auth_status, auth_mode;
	int pppoemwan_status, pppoemwan_enabled;
	int statistics_status;
	int wan_addr_status, wan_gate_status, wan_mask_status;
	int route_snapshot_status;
	int force_clear_wan_dns, wan_dns_read_status, clear_wan_dns;
	char wanx_dns[256];
#if defined (USE_IPV6)
	int ipv6_status, ipv6_type_status, ipv6_is_sit;
#endif

	logmessage(LOGNAME, "%s %s (%s)", "WAN", "down", wan_ifname);
	/*
	 * The DHCP/PPP down callback is already in flight. Complete teardown even
	 * when pause fails, but do not discard the notification error.
	 */
	route_status = notify_pause_detect_internet();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor pause incomplete during WAN teardown (status=%d)",
			   route_status);

	/* Clear the temporary VPN resolver marker before the resolver rebuild. */
	route_status = set_nvram_value_status("vpnc_dns_t", "");
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME,
			   "failed to clear VPN resolver snapshot during WAN teardown (status=%d)",
			   route_status);
	if (!is_static) {
		route_status = notify_rc_status("stop_vpn_client");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME,
				   "WAN VPN client stop notification incomplete (status=%d)",
				   route_status);
	}

#if defined (USE_IPV6)
	ipv6_type_status = is_wan_ipv6_type_sit_status(&ipv6_is_sit);
	if (ipv6_type_status != 0) {
		status = route_status_accumulate(status, ipv6_type_status);
		/* Stop DHCPv6 callback work before declining an interface guess. */
		ipv6_status = wan6_down(wan_ifname, unit);
		status = route_status_accumulate(status, ipv6_status);
		logmessage(LOGNAME,
			   "cannot determine WAN IPv6 transport during teardown on %s (status=%d)",
			   wan_ifname, ipv6_type_status);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN IPv6 best-effort teardown incomplete on %s (status=%d)",
				   wan_ifname, ipv6_status);
	} else if (ipv6_is_sit == 1) {
		ipv6_status = wan6_down(wan_ifname, unit);
		status = route_status_accumulate(status, ipv6_status);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN IPv6 teardown incomplete on %s (status=%d)",
				   wan_ifname, ipv6_status);
	}
#endif

	proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (proto_status != 0) {
		status = route_status_accumulate(status, proto_status);
		logmessage(LOGNAME,
			   "cannot determine WAN protocol during teardown on %s (status=%d); preserving protocol-dependent state",
			   wan_ifname, proto_status);
	}
	modem_status = is_ifunit_modem_status(wan_ifname, unit, &modem_unit_id);
	if (modem_status != 0) {
		status = route_status_accumulate(status, modem_status);
		logmessage(LOGNAME,
			   "cannot read USB modem identity during WAN teardown on %s (status=%d); cleaning DHCP lease routes conservatively",
			   wan_ifname, modem_status);
	}

	/* Remove only the lease-derived routes that this WAN path installed. */
	if (modem_status != 0 || modem_unit_id == 2 ||
	    (!modem_unit_id && (proto_status != 0 ||
			       wan_proto == IPV4_WAN_PROTO_IPOE_DHCP))) {
		char prefix[16];
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		route_status = del_dhcp_routes_by_prefix(prefix, wan_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to delete WAN DHCP routes on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Stop multicast router (for NDIS or IPoE) and retain its status. */
	if (ppp_ifindex(wan_ifname) < 0) {
		route_status = stop_igmpproxy(wan_ifname);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN multicast teardown incomplete on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Stop explicitly enabled auth; on read failure, stop defensively too. */
	if (modem_status != 0 || (!modem_unit_id && (proto_status != 0 ||
			wan_proto == IPV4_WAN_PROTO_IPOE_STATIC ||
			wan_proto == IPV4_WAN_PROTO_IPOE_DHCP))) {
		auth_status = get_wan_unit_value_int_status(unit, "auth_mode", &auth_mode);
		if (auth_status != 0) {
			status = route_status_accumulate(status, auth_status);
			logmessage(LOGNAME,
				   "cannot read WAN Kabinet auth mode during teardown (status=%d); stopping defensively",
				   auth_status);
		}
		if (auth_status != 0 || auth_mode == 1) {
			route_status = stop_auth_kabinet();
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "WAN kabinet authentication teardown incomplete (status=%d)", route_status);
		}
	}

	/*
	 * The default route can be removed with a checked gateway-only read, but
	 * the optional gateway /32 is owned by this WAN only when gateway,
	 * address and netmask came from one NVRAM transaction. Never combine
	 * independently read generations to make that ownership decision.
	 */
	ppp_index = ppp_ifindex(wan_ifname);
	wan_addr_status = 0;
	wan_gate_status = 0;
	wan_mask_status = 0;
	wan_addr[0] = '\0';
	wan_gate[0] = '\0';
	wan_mask[0] = '\0';
	address_valid = 0;
	netmask_valid = 0;
	gateway_off_subnet = 0;
	if (ppp_index < 0) {
		route_snapshot_status = wan_unit_route_snapshot(
			unit, wan_gate, sizeof(wan_gate),
			wan_addr, sizeof(wan_addr),
			wan_mask, sizeof(wan_mask));
		if (route_snapshot_status != 0) {
			status = route_status_accumulate(status, route_snapshot_status);
			logmessage(LOGNAME,
				   "cannot read coherent WAN route snapshot during teardown on %s (status=%d)",
				   wan_ifname, route_snapshot_status);
			/* Preserve best-effort default-route cleanup without authorizing /32 deletion. */
			wan_addr_status = route_snapshot_status;
			wan_mask_status = route_snapshot_status;
			wan_gate_status = get_wan_unit_value_copy_status(
				unit, "gateway", wan_gate, sizeof(wan_gate));
			if (wan_gate_status != 0) {
				status = route_status_accumulate(status, wan_gate_status);
				logmessage(LOGNAME,
					   "cannot read WAN gateway fallback during teardown on %s (status=%d)",
					   wan_ifname, wan_gate_status);
			}
		}
	} else {
		/*
		 * PPP links do not use the gateway /32 ownership decision, but retain
		 * the checked address needed by the later conntrack flush.
		 */
		wan_addr_status = get_wan_unit_value_copy_status(
			unit, "ipaddr", wan_addr, sizeof(wan_addr));
		if (wan_addr_status != 0) {
			status = route_status_accumulate(status, wan_addr_status);
			logmessage(LOGNAME,
				   "cannot read WAN IPv4 address snapshot during teardown on %s (status=%d)",
				   wan_ifname, wan_addr_status);
		}
		wan_gate_status = get_wan_unit_value_copy_status(
			unit, "gateway", wan_gate, sizeof(wan_gate));
		if (wan_gate_status != 0) {
			status = route_status_accumulate(status, wan_gate_status);
			logmessage(LOGNAME,
				   "cannot read WAN gateway snapshot during teardown on %s (status=%d)",
				   wan_ifname, wan_gate_status);
		}
	}

	gateway_valid = wan_gate_status == 0 && is_valid_ipv4(wan_gate);
	if (gateway_valid) {
		route_status = route_del(wan_ifname, 1, "0.0.0.0", wan_gate, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_status)) {
			status = route_status_accumulate(status, route_status);
			logmessage(LOGNAME, "failed to delete WAN default route via %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
		}

		if (ppp_index < 0 && wan_addr_status == 0 && wan_mask_status == 0) {
			address_valid = dhcp_lease_ipv4_is_valid(wan_addr);
			if (!address_valid) {
				status = route_status_accumulate(status, EINVAL);
				logmessage(LOGNAME,
					   "invalid WAN IPv4 address snapshot during teardown on %s",
					   wan_ifname);
			} else {
				netmask_valid = dhcp_lease_netmask_is_valid(wan_mask);
				if (!netmask_valid) {
					wan_mask_status = EINVAL;
					status = route_status_accumulate(status, EINVAL);
					logmessage(LOGNAME,
						   "invalid WAN netmask snapshot during teardown on %s",
						   wan_ifname);
				} else {
					gateway_off_subnet =
						!is_same_subnet(wan_gate, wan_addr, wan_mask);
				}
			}
		}
		if (wan_teardown_gateway_host_route_required(
			    wan_gate_status, wan_addr_status, wan_mask_status,
			    gateway_valid, address_valid, netmask_valid,
			    ppp_index, gateway_off_subnet)) {
			route_status = route_del(wan_ifname, 1, wan_gate, NULL,
						 "255.255.255.255");
			route_status = route_status_idempotent_delete(route_status);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME,
					   "failed to delete WAN gateway host route %s on %s (error=%d)",
					   wan_gate, wan_ifname, route_status);
		}
	}

	/* Remove interface dependent static routes */
	route_status = del_static_wan_routes(wan_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete static WAN routes on %s (error=%d)",
			   wan_ifname, route_status);

	/*
	 * Preserve the demand-PPP DNS snapshot unless MAN DNS is known to remain.
	 * A failed global NVRAM read is not an empty setting: retain the indexed
	 * snapshot and report the read failure while continuing independent cleanup.
	 */
	force_clear_wan_dns =
		modem_status == 0 && proto_status == 0 &&
		(wan_proto == IPV4_WAN_PROTO_IPOE_STATIC ||
		 wan_proto == IPV4_WAN_PROTO_IPOE_DHCP);
	wan_dns_read_status = route_status_accumulate(proto_status, modem_status);
	if (wan_dns_read_status == 0 && !force_clear_wan_dns)
		wan_dns_read_status = rc_wan_nvram_copy_value(
			wan_nvram_checked_get_callback, NULL, "wanx_dns",
			wanx_dns, sizeof(wanx_dns));
	route_status = wan_teardown_should_clear_dns_snapshot(
		force_clear_wan_dns, wan_dns_read_status,
		force_clear_wan_dns ? NULL : wanx_dns, &clear_wan_dns);
	if (route_status != 0) {
		status = route_status_accumulate(status, route_status);
		logmessage(LOGNAME,
			   "cannot determine whether MAN DNS remains during WAN teardown on %s (status=%d); retaining indexed WAN DNS",
			   wan_ifname, route_status);
	} else if (clear_wan_dns) {
		route_status = set_wan_unit_value_status(unit, "dns", "");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME,
				   "failed to clear indexed WAN DNS snapshot on %s (status=%d)",
				   wan_ifname, route_status);
	}

	/* Resolver refresh failure must not block WAN teardown/cleanup. */
	refresh_resolvconf_nonfatal("WAN down", 0);

	if (modem_status == 0 && !modem_unit_id && proto_status == 0 &&
	    wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
		route_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		status = route_status_accumulate(status, route_status);
		if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, route_status)) {
			logmessage(LOGNAME, "failed to clear static WAN address on %s (error=%d); forcing interface down",
				   wan_ifname, route_status);
			interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
			status = route_status_accumulate(status, interface_cleanup_status);
			if (interface_cleanup_status != 0)
				logmessage(LOGNAME, "failed to take static WAN interface %s down after address cleanup failure (status=%d)",
					   wan_ifname, interface_cleanup_status);
		}
	}

	/* Flush conntrack only when the checked previous WAN address is valid. */
	if (wan_addr_status == 0 && is_valid_ipv4(wan_addr)) {
		route_status = flush_conntrack_table(wan_addr);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN conntrack cleanup failed on %s (status=%d)",
				   wan_ifname, route_status);
	} else
		snprintf(wan_addr, sizeof(wan_addr), "0.0.0.0");

	control_wan_led_isp_state(0, 0);

	/*
	 * Statistics are diagnostic state, not a prerequisite for route or
	 * interface teardown. Attempt every clear with errno reporting, but do
	 * not let a stale counter prevent WAN-down completion.
	 */
	statistics_status = rc_wan_nvram_set_all_values(
		statistics_clear_values,
		(unsigned int)(sizeof(statistics_clear_values) /
			       sizeof(statistics_clear_values[0])),
		set_wan_unit_value_status_callback, &unit);
	if (statistics_status != 0)
		logmessage(LOGNAME,
			   "WAN statistics snapshot clear incomplete for unit %d (status=%d); teardown continues",
			   unit, statistics_status);

	/* Call the optional post-WAN hook and preserve its exit status. */
	if (check_if_file_exist(script_postw)) {
		route_status = eval(script_postw, "down", wan_ifname, wan_addr);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN post-down script incomplete on %s (status=%d)",
				   wan_ifname, route_status);
	}
	pppoemwan_status = get_nvram_match_checked_status(
		"pppoemwan_enable", "1", &pppoemwan_enabled);
	if (pppoemwan_status != 0) {
		status = route_status_accumulate(status, pppoemwan_status);
		logmessage(LOGNAME,
			   "cannot read PPPoE-MWAN enable setting during WAN-down (status=%d)",
			   pppoemwan_status);
	} else if (pppoemwan_enabled) {
		route_status = eval("/usr/bin/detect.sh");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN PPPoE-MWAN reconfiguration incomplete (status=%d)",
				   route_status);
	}

	return status;
}

int
wan_down(char *wan_ifname, int unit, int is_static)
{
	int status = wan_down_internal(wan_ifname, unit, is_static);

	if (status != 0)
		logmessage(LOGNAME, "WAN route teardown incomplete on %s (error=%d)",
			   wan_ifname, status);
	return status;
}

static int
merge_l2tp_server_start_status(int wan_status)
{
	int l2tp_enabled = 0;
	int l2tp_config_status;
	int xl2tp_status = 0;

	l2tp_config_status = get_nvram_match_checked_status(
		"l2tp_srv_t", "1", &l2tp_enabled);
	if (l2tp_config_status != 0)
		logmessage(LOGNAME,
			   "cannot read L2TP server enable setting after WAN start (status=%d)",
			   l2tp_config_status);

	/* Do not treat an unreadable enable selector as a disabled server. */
	if (route_status_l2tp_server_start_requested(
		    l2tp_config_status, l2tp_enabled)) {
		/* safe_start_xl2tpd() returns 1 when the shared daemon is already running. */
		xl2tp_status = safe_start_xl2tpd();
		if (wan_status == 0 && xl2tp_status < 0)
			logmessage(LOGNAME,
				   "L2TP server start failed after WAN start (status=%d)",
				   xl2tp_status);
	}

	return route_status_l2tp_server_start_result(
		wan_status, l2tp_config_status, xl2tp_status);
}

int
full_restart_wan(void)
{
	int status;
	int reset_status;
	int static_lan_route_status;
	int switch_vlan_status;
	int route_flush_status;
	int route_ifname_status;
	int is_ap_mode, ap_mode_status, ifname_status;
	unsigned int route_ifname_count;
	char man_ifname[16], man_ifname_nvram[16];
	char *route_ifnames[2];

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "aborting full WAN restart because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return 0;

	/*
	 * Read the route-flush interface before stopping WAN or changing router
	 * state. get_man_ifname() is backed by nvram_safe_get(), which cannot
	 * distinguish an empty setting from a transient NVRAM read failure.
	 */
	ifname_status = get_wan_unit_value_copy_status(
		0, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
	if (ifname_status != 0) {
		errno = ifname_status;
		logmessage(LOGNAME,
			   "aborting full WAN restart because manual interface cannot be read (status=%d)",
			   ifname_status);
		return ifname_status;
	}
	if (man_ifname_nvram[0]) {
		ifname_status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (ifname_status != 0) {
			errno = ifname_status;
			logmessage(LOGNAME,
				   "aborting full WAN restart because manual interface is invalid (status=%d)",
				   ifname_status);
			return ifname_status;
		}
	} else {
		man_ifname[0] = '\0';
	}

	/*
	 * No manual interface is a valid empty configuration: still flush the
	 * bridge and global route cache, but never pass an empty device to ip.
	 * Select and validate the complete route set before stopping WAN services.
	 */
	route_ifname_status = rc_wan_route_flush_select_interfaces(
		man_ifname, IFNAME_BR, route_ifnames,
		sizeof(route_ifnames) / sizeof(route_ifnames[0]),
		&route_ifname_count);
	if (route_ifname_status != 0) {
		errno = route_ifname_status;
		logmessage(LOGNAME,
			   "aborting full WAN restart because route-flush interface selection failed (status=%d)",
			   route_ifname_status);
		return route_ifname_status;
	}

	status = update_router_mode();
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting full WAN restart because router-mode NVRAM refresh failed (status=%d)",
			   status);
		return status;
	}
	status = stop_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because services did not stop (status=%d)",
			   status);
		return status;
	}
	route_flush_status = rc_wan_route_flush_and_cache_apply(
		route_ifnames, route_ifname_count, clear_if_route4, flush_route_caches);
	if (route_flush_status != 0) {
		status = rc_wan_restore_lan_routes_after_failed_step(
			route_flush_status, IFNAME_BR, add_static_lan_routes,
			&static_lan_route_status);
		if (static_lan_route_status != 0)
			logmessage(LOGNAME,
				   "static LAN route restoration also failed after interface route flush error (status=%d)",
				   static_lan_route_status);
		logmessage(LOGNAME, "aborting full WAN restart because interface routes or the global route cache could not be flushed (status=%d)",
			   status);
		return status;
	}
	status = phy_clear_mac_table();
	if (status != 0) {
		status = rc_wan_restore_lan_routes_after_failed_step(
			status, IFNAME_BR, add_static_lan_routes,
			&static_lan_route_status);
		if (static_lan_route_status != 0)
			logmessage(LOGNAME,
				   "static LAN route restoration also failed after switch MAC table clear error (status=%d)",
				   static_lan_route_status);
		logmessage(LOGNAME, "aborting full WAN restart because switch MAC table could not be cleared (status=%d)",
			   status);
		return status;
	}
	reset_status = reset_wan_vars_status();
	if (reset_status != 0) {
		status = rc_wan_restore_lan_routes_after_failed_reset(
			reset_status, IFNAME_BR, add_static_lan_routes,
			&static_lan_route_status);
		if (static_lan_route_status != 0)
			logmessage(LOGNAME,
				   "static LAN route restoration also failed after WAN protocol staging error (status=%d)",
				   static_lan_route_status);
		logmessage(LOGNAME,
			   "aborting full WAN restart because configured WAN protocol staging failed (status=%d)",
			   reset_status);
		return status;
	}
	status = add_static_lan_routes(IFNAME_BR);
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because static LAN routes could not be restored (status=%d)",
			   status);
		return status;
	}
	status = select_usb_modem_to_wan();
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting full WAN restart because USB modem selection could not be published (status=%d)",
			   status);
		return status;
	}
	notify_reset_detect_link();
	switch_vlan_status = switch_config_vlan(0);
	if (switch_vlan_status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because switch VLAN reconfiguration failed (status=%d)",
			   switch_vlan_status);
		return switch_vlan_status;
	}
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	status = disassembly_bridges_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because WAN bridge teardown failed (status=%d)",
			   status);
		return status;
	}
	status = config_soft_bridge_lan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because LAN bridge VLAN setup failed (status=%d)",
			   status);
		return status;
	}
#endif
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
try_wan_reconnect(int try_use_modem, long pause_in_seconds)
{
	int status;
	int is_ap_mode, ap_mode_status;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN reconnect because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return 0;
	status = stop_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting WAN reconnect because services did not stop (status=%d)", status);
		return status;
	}
	status = reset_wan_vars_status();
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN reconnect because configured WAN protocol staging failed (status=%d)",
			   status);
		return status;
	}
	if (pause_in_seconds > 0) {
		status = schedule_deferred_wan_reconnect_status(
			uptime(), pause_in_seconds);
		if (status != 0) {
			logmessage(LOGNAME,
				   "cannot publish deferred WAN reconnect deadline (status=%d)",
				   status);
			return status;
		}
		status = start_watchdog();
		if (status != 0) {
			/*
			 * Keep the deferred intent published so a later watchdog recovery
			 * can still perform the requested WAN reconnect.
			 */
			logmessage(LOGNAME,
				   "cannot schedule deferred WAN reconnect because watchdog startup failed (status=%d)",
				   status);
			return status;
		}
		logmessage(LOGNAME, "WAN up delay: %lds.", pause_in_seconds);
		return 0;
	}
	if (try_use_modem) {
		status = select_usb_modem_to_wan();
		if (status != 0) {
			logmessage(LOGNAME,
				   "aborting WAN reconnect because USB modem selection could not be published (status=%d)",
				   status);
			return status;
		}
	}
	notify_reset_detect_link();
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
deferred_wan_connect(void)
{
	int status;
	if (check_if_file_exist(SCRIPT_UDHCPC_WAN))
		return 0;
	status = clear_deferred_wan_reconnect_status();
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot clear deferred WAN reconnect deadline before startup (status=%d)",
			   status);
		return status;
	}
	status = select_usb_modem_to_wan();
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting deferred WAN startup because USB modem selection could not be published (status=%d)",
			   status);
		return status;
	}
	notify_reset_detect_link();
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
manual_wan_disconnect(void)
{
	int wan_proto, status, is_ndis_modem = 0;
	int is_ap_mode, ap_mode_status, is_usb_modem;

	logmessage(LOGNAME, "Perform WAN %s %s", "manual", "disconnect");
	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "refusing manual WAN disconnect because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return 0;

	status = get_usb_modem_wan_status(0, &is_usb_modem);
	if (status != 0) {
		logmessage(LOGNAME,
			   "refusing manual WAN disconnect because USB modem identity cannot be read (status=%d)",
			   status);
		errno = status;
		return status;
	}
	if (!is_usb_modem) {
		status = get_wan_unit_proto_checked_status(0, &wan_proto);
		if (status != 0) {
			logmessage(LOGNAME,
				   "refusing manual WAN disconnect because configured protocol cannot be read (status=%d)",
				   status);
			errno = status;
			return status;
		}
	}

	if (is_usb_modem) {
		status = get_wan_modem_is_ndis_status(&is_ndis_modem);
		if (status != 0) {
			logmessage(LOGNAME,
				   "refusing manual WAN disconnect because USB modem transport type cannot be read (status=%d)",
				   status);
			errno = status;
			return status;
		}
		if (is_ndis_modem)
			status = stop_wan();
		else
			status = stop_wan_ppp();
	} else if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		   wan_proto == IPV4_WAN_PROTO_PPTP ||
		   wan_proto == IPV4_WAN_PROTO_L2TP) {
		status = stop_wan_ppp();
	} else {
		status = stop_wan();
	}

	if (status != 0)
		logmessage(LOGNAME, "manual WAN disconnect failed (status=%d)", status);
	return status;
}

int
manual_wan_reconnect(void)
{
	logmessage(LOGNAME, "Perform WAN %s %s", "manual", "reconnect");
	return try_wan_reconnect(1, 0);
}

int
manual_wisp_reassoc(void)
{
	const char *wisp_ifname = NULL;
	int reconnect_status, wisp_status;

	wisp_status = get_apcli_wisp_ifname_status(&wisp_ifname);
	if (wisp_status != 0) {
		errno = wisp_status;
		logmessage(LOGNAME,
			   "cannot reassociate AP-client WAN without verified WISP configuration (status=%d)",
			   wisp_status);
		return wisp_status;
	}
	if (!wisp_ifname)
		return 0;

	logmessage(LOGNAME, "Perform WISP %s %s", "manual", "reassoc");

	reconnect_status = reconnect_apcli(wisp_ifname, 1);
	if (reconnect_status != 0)
		logmessage(LOGNAME, "manual WISP reassociation failed on %s (status=%d)",
			   wisp_ifname, reconnect_status);
	return reconnect_status;
}

int
auto_wan_reconnect(void)
{
	logmessage(LOGNAME, "Perform WAN %s %s", "auto", "reconnect");
	return try_wan_reconnect(1, 0);
}

int
auto_wan_reconnect_pause(void)
{
	int pause_seconds = 0;
	int status;

	status = get_nvram_int_checked_status("di_recon_pause", &pause_seconds);
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting automatic WAN reconnect because delay configuration is unavailable (status=%d)",
			   status);
		return status;
	}

	logmessage(LOGNAME, "Perform WAN %s %s", "auto", "reconnect");
	return try_wan_reconnect(1, pause_seconds);
}

int
notify_on_wan_ether_link_restored(void)
{
	int unit = 0, wan_proto;
	int is_wisp_active = 0, manual_dhcp_disabled = 0;
	int status, saved_errno;
	char wan_ifname_temp[IFNAMSIZ];

	status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot check WAN protocol before link-restored DHCP renew (status=%d)",
			   status);
		return status;
	}

	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)
		return 0;

	status = get_nvram_match_checked_status(
		"x_DHCPClient", "0", &manual_dhcp_disabled);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot read manual WAN DHCP-client selector before link-restored renew (status=%d)",
			   status);
		return status;
	}
	if (manual_dhcp_disabled)
		return 0;

	status = get_wan_unit_value_copy_status(
		unit, "ifname_t", wan_ifname_temp, sizeof(wan_ifname_temp));
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot read WAN transport interface before link-restored DHCP renew (status=%d)",
			   status);
		return status;
	}
	if (is_usbnet_interface(wan_ifname_temp))
		return 0;

	status = get_wan_wisp_active_status(&is_wisp_active, NULL);
	if (status != 0)
		return status;
	if (is_wisp_active)
		return 0;

	errno = 0;
	status = renew_udhcpc_wan(unit);
	if (status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot signal WAN DHCP client to renew after Ethernet link restoration (status=%d, errno=%d)",
			   status, saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	logmessage(LOGNAME, "force WAN DHCP client renew...");
	return 0;
}

static void
queue_internet_lost_action(const char *event_name, const char *action_name)
{
	int status, saved_errno;

	errno = 0;
	status = notify_rc_status(event_name);
	saved_errno = detect_internet_notification_error(status, errno);
	if (saved_errno == 0)
		return;

	logmessage(LOGNAME,
		   "cannot enqueue %s after Internet loss (status=%d, errno=%d)",
		   action_name, status, saved_errno);
	errno = saved_errno;
}


void
notify_on_internet_state_changed(int has_internet, long elapsed)
{
	const char *script_inet = SCRIPT_INTERNET_STATE;
	int hook_status;

	if (!has_internet) {
		int is_ap_mode, ap_mode_status;

		ap_mode_status = get_ap_mode_status(&is_ap_mode);
		if (ap_mode_status != 0) {
			logmessage(LOGNAME,
				   "router Internet-loss actions suppressed because AP/router mode cannot be verified (status=%d)",
				   ap_mode_status);
		} else if (!is_ap_mode) {
			int fail_action_value = 0, fail_action = 0;
			int fail_action_status;

			fail_action_status = get_nvram_int_checked_status(
				"di_lost_action", &fail_action_value);
			fail_action_status = rc_wan_internet_loss_action_status(
				fail_action_status, fail_action_value, &fail_action);
			if (fail_action_status != 0) {
				errno = fail_action_status;
				logmessage(LOGNAME,
					   "router Internet-loss automatic action suppressed because di_lost_action cannot be read (status=%d)",
					   fail_action_status);
			} else {
				switch (fail_action)
				{
				case 1:
					logmessage(LOGNAME, "Perform router auto-reboot on %s event", "Internet lost");
					queue_internet_lost_action(RCN_RESTART_REBOOT, "router auto-reboot");
					return;
				case 2:
					queue_internet_lost_action("auto_wan_reconnect_pause",
							   "automatic WAN reconnect pause");
					break;
#if defined (USE_USB_SUPPORT)
				case 3:
					if (get_modem_devnum()) {
						int priority_status, modem_identity_status;
						int is_modem_wan = 0;

						modem_identity_status = get_usb_modem_wan_status(
							0, &is_modem_wan);
						if (modem_identity_status != 0) {
							logmessage(LOGNAME,
								   "cannot read modem WAN identity before automatic reconnect (status=%d)",
								   modem_identity_status);
							break;
						}
						priority_status = wan_nvram_int_status(
							"modem_prio", is_modem_wan ? 0 : 1);
						if (priority_status != 0) {
							logmessage(LOGNAME,
								   "cannot publish modem priority before automatic reconnect (status=%d)",
								   priority_status);
							break;
						}
						queue_internet_lost_action("auto_wan_reconnect",
								   "automatic WAN reconnect");
					}
					break;
#endif
				}
			}
		}
	}

	if (check_if_file_exist(script_inet)) {
		hook_status = doSystem("%s %d %ld", script_inet, has_internet, elapsed);
		if (hook_status != 0)
			logmessage(LOGNAME,
				   "internet state hook failed (state=%d, elapsed=%ld, wait status=%d)",
				   has_internet, elapsed, hook_status);
	}
}

static int get_wan_dns_static_status(int *is_static);

int
update_resolvconf(int is_first_run, int do_not_notify)
{
	FILE *fp;
	char word[512], *next, *wan_dns;
	char vpnc_dns_snapshot[512] = {0};
	char static_wan_dns[3][INET_ADDRSTRLEN] = {{0}};
	char unit_wan_dns[512] = {0}, man_wan_dns[512] = {0};
#if defined (USE_IPV6)
	char wan_dns6_snapshot[512] = {0};
#endif
	struct nvram_transaction vpnc_dns_transaction, wan_dns_transaction;
	struct rc_wan_nvram_snapshot_field static_dns_fields[3];
	const char *google_dns = "8.8.8.8";
	char resolv_temp[] = "/etc/resolv.conf.XXXXXX";
	int i, i_pdns, i_total_dns = 0;
	int vpnc_snapshot_status;
	int lock, dns_static, dns_static_status, resolv_changed = 0;
	int dns_restart_status;
	int dns_servers_ready = 1, dns_servers_error = 0, saved_errno, validation_errno;
	int wan_dns_config_status = 0;
	int modem_identity_status, is_modem_wan = 0;

	if (!is_first_run && fill_dnsmasq_servers() != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_is_safe_regular(DNS_SERVERS_FILE) != 0) {
			validation_errno = errno ? errno : EPERM;
			dns_servers_ready = 0;
			dns_servers_error = validation_errno;
			logmessage(LOGNAME, "DNSMasq reload blocked: unsafe DNS server snapshot (errno=%d)", validation_errno);
		} else {
			logmessage(LOGNAME, "DNS server snapshot refresh failed; keeping last-known-good file (errno=%d)", saved_errno);
		}
	}

	lock = file_lock("resolv");
	if (lock < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire resolver publication lock; refusing update (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	vpnc_snapshot_status = rc_wan_nvram_copy_int_text_snapshot(
		"vpnc_pdns", "vpnc_dns_t", &i_pdns,
		vpnc_dns_snapshot, sizeof(vpnc_dns_snapshot),
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &vpnc_dns_transaction);
	if (vpnc_snapshot_status != 0) {
		errno = vpnc_snapshot_status;
		logmessage(LOGNAME,
			   "cannot snapshot VPN DNS policy and server list; keeping the existing resolver file (status=%d)",
			   vpnc_snapshot_status);
		file_unlock(lock);
		errno = vpnc_snapshot_status;
		return vpnc_snapshot_status;
	}

	dns_static_status = get_wan_dns_static_status(&dns_static);
	if (dns_static_status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN DNS mode; keeping the existing resolver file (status=%d)",
			   dns_static_status);
		file_unlock(lock);
		errno = dns_static_status;
		return dns_static_status;
	}

	/*
	 * Resolve every WAN DNS input with checked reads before staging a new
	 * resolver file. A backend error must preserve the last-known-good file,
	 * not silently publish the public fallback as if the custom DNS were empty.
	 */
	if (dns_static) {
		static_dns_fields[0].param_name = "wan_dns1_x";
		static_dns_fields[0].destination = static_wan_dns[0];
		static_dns_fields[0].destination_size = sizeof(static_wan_dns[0]);
		static_dns_fields[1].param_name = "wan_dns2_x";
		static_dns_fields[1].destination = static_wan_dns[1];
		static_dns_fields[1].destination_size = sizeof(static_wan_dns[1]);
		static_dns_fields[2].param_name = "wan_dns3_x";
		static_dns_fields[2].destination = static_wan_dns[2];
		static_dns_fields[2].destination_size = sizeof(static_wan_dns[2]);
		wan_dns_config_status = rc_wan_nvram_copy_snapshot(
			static_dns_fields, 3, wan_nvram_transaction_begin_callback,
			wan_nvram_transaction_get_callback,
			wan_nvram_transaction_end_callback, &wan_dns_transaction);
	} else if (!is_first_run) {
		wan_dns_config_status = get_wan_unit_value_copy_status(
			0, "dns", unit_wan_dns, sizeof(unit_wan_dns));
		if (wan_dns_config_status == 0 && strlen(unit_wan_dns) < 7)
			wan_dns_config_status = rc_wan_nvram_copy_value(
				wan_nvram_checked_get_callback, NULL, "wanx_dns",
				man_wan_dns, sizeof(man_wan_dns));
		if (wan_dns_config_status == 0 && strlen(unit_wan_dns) >= 7)
			man_wan_dns[0] = '\0';
	}
#if defined (USE_IPV6)
	if (wan_dns_config_status == 0)
		wan_dns_config_status = get_wan_unit_value_copy_status(
			0, "dns6", wan_dns6_snapshot, sizeof(wan_dns6_snapshot));
#endif
	if (wan_dns_config_status != 0) {
		errno = wan_dns_config_status;
		logmessage(LOGNAME,
			   "cannot read configured WAN DNS servers; preserving existing resolver file (status=%d)",
			   wan_dns_config_status);
		file_unlock(lock);
		errno = wan_dns_config_status;
		return wan_dns_config_status;
	}

	/* Never truncate /etc/resolv.conf or use a predictable temporary path. */
	fp = dnsmasq_file_open(resolv_temp);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove resolver staging file '%s' after open failure (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot stage resolver configuration (errno=%d)", saved_errno);
		file_unlock(lock);
		errno = saved_errno;
		return saved_errno;
	}

	/* dnsmasq will resolve localhost DNS queries */
	fprintf(fp, "nameserver %s\n", "127.0.0.1");

	/* DNS servers for static VPN client */
	if (!is_first_run && i_pdns > 0) {
		wan_dns = vpnc_dns_snapshot;
		if (strlen(wan_dns) > 6) {
			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word)) {
					fprintf(fp, "nameserver %s\n", word);
					i_total_dns++;
				}
			}
		}
	}

	/* DNS servers for WAN/MAN */
	if (i_pdns != 2 || i_total_dns < 1) {
		if (dns_static) {
			if (is_first_run)
				resolv_changed = 1;

			for (i = 0; i < 3; i++) {
				wan_dns = static_wan_dns[i];
				if (is_valid_ipv4(wan_dns)) {
					fprintf(fp, "nameserver %s\n", wan_dns);
					i_total_dns++;
				}
			}

		} else if (!is_first_run) {
			wan_dns = (strlen(unit_wan_dns) >= 7) ?
				unit_wan_dns : man_wan_dns;

			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word)) {
					fprintf(fp, "nameserver %s\n", word);
					i_total_dns++;
				}
			}
		}
	}

	if (i_total_dns < 1)
		fprintf(fp, "nameserver %s\n", google_dns);

#if defined (USE_IPV6)
	/* DNSv6 servers */
	wan_dns = wan_dns6_snapshot;
	foreach(word, wan_dns, next) {
		if (strlen(word) > 0) {
			char dns6s[INET6_ADDRSTRLEN] = {0};
			if (ipv6_compact(word, dns6s, 0) == 0) {
				fprintf(fp, "nameserver %s\n", dns6s);
				if (is_first_run)
					resolv_changed = 1;
			}
		}
	}
#endif

	/* Catch buffered generation failures before comparing or publishing. */
	if (fflush(fp) != 0 || ferror(fp)) {
		saved_errno = errno ? errno : EIO;
		if (fclose(fp) != 0)
			logmessage(LOGNAME, "error closing failed resolver staging file (errno=%d)", errno ? errno : EIO);
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove failed resolver staging file '%s' (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		file_unlock(lock);
		logmessage(LOGNAME, "cannot write resolver configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* Unsafe/missing destinations are treated as changed and atomically replaced.
	 * Equal files are retained to avoid needless dnsmasq reloads. */
	if (dnsmasq_file_contents_equal(DNS_RESOLV_CONF, resolv_temp)) {
		if (fclose(fp) != 0) {
			saved_errno = errno ? errno : EIO;
			if (dnsmasq_file_discard(resolv_temp) != 0)
				logmessage(LOGNAME, "cannot remove unchanged resolver staging file '%s' (errno=%d)",
					   resolv_temp, errno ? errno : EIO);
			file_unlock(lock);
			logmessage(LOGNAME, "cannot close unchanged resolver staging file (errno=%d)", saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		if (dnsmasq_file_discard(resolv_temp) != 0) {
			saved_errno = errno ? errno : EIO;
			file_unlock(lock);
			logmessage(LOGNAME, "cannot remove unchanged resolver staging file '%s' (errno=%d)",
				   resolv_temp, saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
	} else {
		if (dnsmasq_file_publish(fp, resolv_temp, DNS_RESOLV_CONF, 0644) != 0) {
			saved_errno = errno ? errno : EIO;
			if (dnsmasq_file_discard(resolv_temp) != 0)
				logmessage(LOGNAME, "cannot remove failed resolver staging file '%s' (errno=%d)",
					   resolv_temp, errno ? errno : EIO);
			file_unlock(lock);
			logmessage(LOGNAME, "cannot publish resolver configuration (errno=%d)", saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		resolv_changed = 1;
	}

	file_unlock(lock);

/* Do not report success when an unsafe upstream snapshot prevented the
 * DNSMasq cache refresh. The resolver file is published, but the daemon
 * must not be signalled until its server input is safe. */
	saved_errno = wan_resolver_refresh_policy_blocked_status(dns_servers_ready,
								 dns_servers_error);
	if (saved_errno != 0) {
		logmessage(LOGNAME,
			   "resolver file published but DNSMasq server snapshot is unsafe; cache reload was blocked (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* Force a cache reload for modems and when modem identity is unknown. */
	modem_identity_status = get_usb_modem_wan_status(0, &is_modem_wan);
	if (modem_identity_status != 0) {
		logmessage(LOGNAME,
			   "cannot read modem WAN identity while publishing resolver; forcing DNSMasq reload (status=%d)",
			   modem_identity_status);
		resolv_changed = 1;
	} else if (!resolv_changed && is_modem_wan) {
		resolv_changed = 1;
	}

	/* Notify DNSMasq only on a complete process snapshot. A failed legacy
	 * pids() scan is indistinguishable from ABSENT and can leave stale caches. */
	if (resolv_changed && !do_not_notify && dns_servers_ready) {
		int dnsmasq_scan_error = 0;
		int dnsmasq_status = dnsmasq_process_status(&dnsmasq_scan_error);

		if (dnsmasq_status < 0) {
			saved_errno = dnsmasq_scan_error ? dnsmasq_scan_error : EIO;
			logmessage(LOGNAME,
				   "resolver file published but DNSMasq process state is unknown; cache reload not attempted (scan_error=%d)",
				   saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		if (dnsmasq_status > 0) {
			dns_restart_status = restart_dns();
			if (dns_restart_status != 0) {
				logmessage(LOGNAME,
					   "resolver file published but DNSMasq cache reload failed (status=%d)",
					   dns_restart_status);
				return dns_restart_status;
			}
		}
	}

	return 0;
}

static void
refresh_resolvconf_nonfatal_internal(const char *reason, int is_first_run,
				     int do_not_notify)
{
	int refresh_status;

	refresh_status = update_resolvconf(is_first_run, do_not_notify);
	if (wan_resolver_refresh_policy_action(refresh_status) ==
	    WAN_RESOLVER_REFRESH_KEEP_WAN) {
		logmessage(LOGNAME,
			   "resolver/DNS refresh did not complete after %s (status=%d); keeping WAN state",
			   (reason && *reason) ? reason : "WAN event", refresh_status);
	}
}

void
refresh_resolvconf_nonfatal(const char *reason, int is_first_run)
{
	refresh_resolvconf_nonfatal_internal(reason, is_first_run, 0);
}

void
refresh_resolvconf_nonfatal_no_notify(const char *reason, int is_first_run)
{
	refresh_resolvconf_nonfatal_internal(reason, is_first_run, 1);
}

int
update_hosts_router(const char *lan_ipaddr)
{
	char hostname_backup[] = "/etc/hostname.rollback.XXXXXX";
	char hosts_backup[] = "/etc/hosts.rollback.XXXXXX";
	FILE *hostname_fp = NULL, *hosts_fp = NULL;
	char hostname_temp[] = "/etc/hostname.XXXXXX";
	char hosts_temp[] = "/etc/hosts.XXXXXX";
	char *lan_hname;
	char lan_dname[256];
	int lan_domain_status;
	int saved_errno = 0;
	int hosts_lock = -1;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status, lan_addr6_status;
	char lan_addr6_host[INET6_ADDRSTRLEN] = {0};
#endif

	lan_hname = get_our_hostname();
	if (!lan_hname) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		logmessage(LOGNAME,
			   "cannot read router hostname before hostname publication (status=%d)",
			   saved_errno);
		return saved_errno;
	}
	lan_domain_status = rc_wan_nvram_copy_value(
		wan_nvram_checked_get_callback, NULL, "lan_domain",
		lan_dname, sizeof(lan_dname));
	if (lan_domain_status != 0) {
		errno = lan_domain_status;
		logmessage(LOGNAME,
			   "cannot snapshot router hostname domain (status=%d)",
			   lan_domain_status);
		return lan_domain_status;
	}
	if (!dhcp_lease_ipv4_is_valid(lan_ipaddr) ||
	    (*lan_dname && !dnsmasq_valid_server_domain(lan_dname))) {
		errno = EINVAL;
		logmessage(LOGNAME, "refusing unsafe router hostname input");
		return EINVAL;
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "refusing to publish router hostname files without verified IPv6 service state (status=%d)",
			   ipv6_type_status);
		return ipv6_type_status;
	}
	if (ipv6_type != IPV6_DISABLED) {
		lan_addr6_status = get_lan_addr6_host_status(
			lan_addr6_host, sizeof(lan_addr6_host));
		if (lan_addr6_status != 0) {
			errno = lan_addr6_status;
			logmessage(LOGNAME,
				   "refusing to publish hostname files without a verified LAN IPv6 address snapshot (status=%d)",
				   lan_addr6_status);
			return lan_addr6_status;
		}
	}
#endif

	/* Stage both resolver-visible files before replacing either destination. */
	hostname_fp = dnsmasq_file_open(hostname_temp);
	if (!hostname_fp)
		goto failed;
	hosts_fp = dnsmasq_file_open(hosts_temp);
	if (!hosts_fp)
		goto failed;

	fprintf(hostname_fp, "%s\n", lan_hname);
	fprintf(hosts_fp, "%s %s %s\n", "127.0.0.1", "localhost.localdomain", "localhost");
	if (strlen(lan_dname) > 0)
		fprintf(hosts_fp, "%s %s.%s %s\n", lan_ipaddr, lan_hname, lan_dname, lan_hname);
	else
		fprintf(hosts_fp, "%s %s\n", lan_ipaddr, lan_hname);
	fprintf(hosts_fp, "%s %s\n", lan_ipaddr, "my.router");

	/* Never ingest a stale or attacker-planted shared-/tmp link as hosts data. */
	if (dnsmasq_file_is_safe_regular("/tmp/hosts.static") == 0)
		saved_errno = load_user_config(hosts_fp, "/tmp", "hosts.static", NULL);
		if (saved_errno != 0)
			goto failed;
	else if (errno != ENOENT)
		logmessage(LOGNAME, "skipping unsafe static DHCP host snapshot (errno=%d)", errno);

#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		fprintf(hosts_fp, "%s %s %s\n", "::1", "localhost.localdomain", "localhost");
		if (lan_addr6_host[0]) {
			if (strlen(lan_dname) > 0)
				fprintf(hosts_fp, "%s %s.%s %s\n", lan_addr6_host, lan_hname, lan_dname, lan_hname);
			else
				fprintf(hosts_fp, "%s %s\n", lan_addr6_host, lan_hname);
			fprintf(hosts_fp, "%s %s\n", lan_addr6_host, "my.router");
		}
	}
#endif

	hosts_lock = dnsmasq_lifecycle_lock_acquire_path(
		DNSMASQ_HOSTNAME_LOCK_FILE, DNSMASQ_LIFECYCLE_LOCK_WAIT_SECONDS);
	if (hosts_lock < 0) {
		saved_errno = errno ? errno : EIO;
		goto failed;
	}
	{
		FILE *staged_hostname_fp = hostname_fp;
		FILE *staged_hosts_fp = hosts_fp;
		hostname_fp = NULL;
		hosts_fp = NULL;
		if (dnsmasq_file_publish_pair(staged_hostname_fp, hostname_temp,
					      "/etc/hostname", hostname_backup,
					      staged_hosts_fp, hosts_temp,
					      "/etc/hosts", hosts_backup, 0644) != 0) {
			saved_errno = errno ? errno : EIO;
			goto failed;
		}
	}

	/* The pair helper already tried once; retry snapshots while their buffers
	 * remain in scope and warn without undoing the completed publication. */
	if (dnsmasq_file_backup_discard(hostname_backup) != 0)
		logmessage(LOGNAME, "cannot remove router hostname rollback snapshot '%s' (errno=%d)",
			   hostname_backup, errno ? errno : EIO);
	if (dnsmasq_file_backup_discard(hosts_backup) != 0)
		logmessage(LOGNAME, "cannot remove router hosts rollback snapshot '%s' (errno=%d)",
			   hosts_backup, errno ? errno : EIO);

	/*
	 * The durable files are already published. Keep kernel-name failures
	 * visible so DNSMasq preparation can abort and a later retry can apply
	 * the same runtime identity; do not roll back a valid file pair.
	 */
	saved_errno = lan_kernel_names_apply(lan_hname, lan_dname,
					     &wan_kernel_name_ops);
	if (saved_errno != 0)
		logmessage(LOGNAME,
			   "router kernel hostname/domain update failed (status=%d)",
			   saved_errno);

	if (dnsmasq_lifecycle_lock_release(hosts_lock) != 0) {
		int release_status = errno ? errno : EIO;

		if (saved_errno == 0)
			saved_errno = release_status;
		else
			logmessage(LOGNAME,
				   "hostname file lock release also failed (status=%d; first_error=%d)",
				   release_status, saved_errno);
	}
	hosts_lock = -1;
	if (saved_errno != 0) {
		errno = saved_errno;
		return saved_errno;
	}
	return 0;

failed:
	if (hosts_lock >= 0) {
		if (dnsmasq_lifecycle_lock_release(hosts_lock) != 0)
			logmessage(LOGNAME, "cannot release hostname file lock (errno=%d)", errno ? errno : EIO);
		hosts_lock = -1;
	}
	if (!saved_errno)
		saved_errno = errno ? errno : EIO;
	if (hostname_fp)
		fclose(hostname_fp);
	if (hosts_fp)
		fclose(hosts_fp);
	if (dnsmasq_file_discard(hostname_temp) != 0)
		logmessage(LOGNAME, "cannot remove router hostname stage '%s' (errno=%d)",
			   hostname_temp, errno ? errno : EIO);
	if (dnsmasq_file_discard(hosts_temp) != 0)
		logmessage(LOGNAME, "cannot remove router hosts stage '%s' (errno=%d)",
			   hosts_temp, errno ? errno : EIO);
	if (hostname_backup[0])
		logmessage(LOGNAME, "retaining router hostname rollback snapshot '%s' for recovery", hostname_backup);
	if (hosts_backup[0])
		logmessage(LOGNAME, "retaining router hosts rollback snapshot '%s' for recovery", hosts_backup);
	logmessage(LOGNAME, "cannot stage/publish router hostname files (errno=%d)", saved_errno);
	errno = saved_errno;
	return saved_errno;
}

static int
control_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms,
                    char *ifname, int metric, int is_add)
{
	int dr_enable_value = 0;
	int dr_enable_status;

	char *routes, *tmp;
	const char *classless_routes;
	struct in_addr mask;
	char *ipaddr, *gateway, *bits_str, *bits_end;
	long parsed_bits;
	int bits;
	int status = 0, route_status;
	char netmask[] = "255.255.255.255";

	/* Keep cleanup working even after the dynamic-route option is disabled. */
	if (is_add) {
		dr_enable_status = get_nvram_match_checked_status(
			"dr_enable_x", "1", &dr_enable_value);
		if (dr_enable_status != 0) {
			logmessage(LOGNAME,
				   "cannot read dynamic-route enable flag before adding DHCP routes (status=%d)",
				   dr_enable_status);
			errno = dr_enable_status;
			return dr_enable_status;
		}
		if (!dr_enable_value)
			return 0;
	}
	if (!ifname || !*ifname)
		return EINVAL;
	if (!rt)
		rt = "";
	if (!rt_rfc)
		rt_rfc = "";
	if (!rt_ms)
		rt_ms = "";

	/* Legacy destination/gateway pairs. */
	routes = NULL;
	if (*rt) {
		routes = strdup(rt);
		if (!routes) {
			status = route_status_accumulate(status, ENOMEM);
			logmessage(LOGNAME, "failed to copy legacy DHCP routes on %s (errno=%d)",
				   ifname, ENOMEM);
		} else {
			for (tmp = routes; tmp && *tmp; ) {
				ipaddr = strsep(&tmp, "/");
				gateway = strsep(&tmp, " ");
				if (gateway && *gateway &&
				    dhcp_lease_ipv4_is_valid(ipaddr) &&
				    dhcp_lease_ipv4_is_valid(gateway)) {
					route_status = is_add ?
						route_add(ifname, metric, ipaddr, gateway, netmask) :
						route_del(ifname, metric, ipaddr, gateway, netmask);
					if ((is_add && !dhcp_lease_route_add_is_ok(route_status)) ||
					    (!is_add && !dhcp_lease_route_delete_is_ok(route_status))) {
						status = route_status_accumulate(status, route_status);
						logmessage(LOGNAME, "failed to %s DHCP route %s via %s on %s (error=%d)",
							   is_add ? "add" : "delete", ipaddr, gateway,
							   ifname, route_status);
					}
				}
			}
			free(routes);
		}
	}
	/* RFC3442 or Microsoft classless static routes. */
	classless_routes = *rt_rfc ? rt_rfc : rt_ms;
	if (*classless_routes) {
		routes = strdup(classless_routes);
		if (!routes) {
			status = route_status_accumulate(status, ENOMEM);
			logmessage(LOGNAME, "failed to copy classless DHCP routes on %s (errno=%d)",
				   ifname, ENOMEM);
		} else {
			for (tmp = routes; tmp && *tmp; ) {
				ipaddr = strsep(&tmp, "/");
				if (!ipaddr || !tmp || !*tmp)
					break;
				bits_str = strsep(&tmp, " ");
				if (!bits_str || !*bits_str || !tmp || !*tmp)
					break;
				errno = 0;
				parsed_bits = strtol(bits_str, &bits_end, 10);
				gateway = strsep(&tmp, " ");
				if (errno == ERANGE || bits_end == bits_str || *bits_end != '\0' ||
				    parsed_bits < 0 || parsed_bits > 32 ||
				    !gateway || !*gateway ||
				    !dhcp_lease_route_destination_is_valid(ipaddr, (unsigned int)parsed_bits) ||
				    !dhcp_lease_ipv4_is_valid(gateway))
					continue;
				bits = (int)parsed_bits;
				mask.s_addr = (bits == 0) ? 0 : htonl(0xffffffffU << (32 - bits));
				strcpy(netmask, inet_ntoa(mask));
				route_status = is_add ?
					route_add(ifname, metric, ipaddr, gateway, netmask) :
					route_del(ifname, metric, ipaddr, gateway, netmask);
				if ((is_add && !dhcp_lease_route_add_is_ok(route_status)) ||
				    (!is_add && !dhcp_lease_route_delete_is_ok(route_status))) {
					status = route_status_accumulate(status, route_status);
					logmessage(LOGNAME, "failed to %s DHCP classless route %s/%d via %s on %s (error=%d)",
						   is_add ? "add" : "delete", ipaddr, bits, gateway,
						   ifname, route_status);
				}
			}
			free(routes);
		}
	}
	return status;
}

int
add_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms, char *ifname, int metric)
{
	return control_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric, 1);
}

static int
snapshot_dhcp_route_options(const char *prefix,
			    char *rt, size_t rt_size,
			    char *rt_rfc, size_t rt_rfc_size,
			    char *rt_ms, size_t rt_ms_size)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[3];
	char key_routes[NVRAM_MAX_PARAM_LEN];
	char key_routes_rfc[NVRAM_MAX_PARAM_LEN];
	char key_routes_ms[NVRAM_MAX_PARAM_LEN];
	int length;

	if (!prefix || !prefix[0] ||
	    !rt || rt_size == 0 ||
	    !rt_rfc || rt_rfc_size == 0 ||
	    !rt_ms || rt_ms_size == 0)
		return EINVAL;

	length = snprintf(key_routes, sizeof(key_routes), "%sroutes", prefix);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(key_routes))
		return ENAMETOOLONG;
	length = snprintf(key_routes_rfc, sizeof(key_routes_rfc),
			  "%sroutes_rfc", prefix);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(key_routes_rfc))
		return ENAMETOOLONG;
	length = snprintf(key_routes_ms, sizeof(key_routes_ms),
			  "%sroutes_ms", prefix);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(key_routes_ms))
		return ENAMETOOLONG;

	fields[0].param_name = key_routes;
	fields[0].destination = rt;
	fields[0].destination_size = rt_size;
	fields[1].param_name = key_routes_rfc;
	fields[1].destination = rt_rfc;
	fields[1].destination_size = rt_rfc_size;
	fields[2].param_name = key_routes_ms;
	fields[2].destination = rt_ms;
	fields[2].destination_size = rt_ms_size;

	/*
	 * These NVRAM values live in a reusable cache ring. Hold one transaction
	 * and copy each field before the next read can recycle its storage.
	 */
	return rc_wan_nvram_copy_snapshot(
		fields, 3,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback,
		&transaction);
}


int
add_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric)
{
	char rt[NVRAM_MAX_VALUE_LEN + 1];
	char rt_rfc[NVRAM_MAX_VALUE_LEN + 1];
	char rt_ms[NVRAM_MAX_VALUE_LEN + 1];
	int read_status;

	read_status = snapshot_dhcp_route_options(
		prefix, rt, sizeof(rt), rt_rfc, sizeof(rt_rfc),
		rt_ms, sizeof(rt_ms));
	if (read_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot DHCP route settings for %s (error=%d)",
			   prefix ? prefix : "(null)", read_status);
		errno = read_status;
		return read_status;
	}
	return add_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric);
}

int
del_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric)
{
	char rt[NVRAM_MAX_VALUE_LEN + 1];
	char rt_rfc[NVRAM_MAX_VALUE_LEN + 1];
	char rt_ms[NVRAM_MAX_VALUE_LEN + 1];
	int read_status;

	read_status = snapshot_dhcp_route_options(
		prefix, rt, sizeof(rt), rt_rfc, sizeof(rt_rfc),
		rt_ms, sizeof(rt_ms));
	if (read_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot DHCP route settings for %s (error=%d)",
			   prefix ? prefix : "(null)", read_status);
		errno = read_status;
		return read_status;
	}
	return control_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric, 0);
}

int
add_static_wan_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 1);
}

int
del_static_wan_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 0);
}

int
add_static_man_routes(char *man_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, man_ifname, 1);
}

int
del_static_man_routes(char *man_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, man_ifname, 0);
}

int
select_usb_modem_to_wan(void)
{
	int modem_devnum = 0;
	int selected_devnum = 0;
	int modem_priority = 0;
	int priority_status = 0, link_probe_status = 0;
	int is_apcli_wisp = 0, has_link = 0;
	int status;
#if defined (USE_USB_SUPPORT)
	int wisp_status;
	const char *wisp_ifname = NULL;
#endif

#if defined (USE_USB_SUPPORT)
	/* check modem enabled and ready */
	modem_devnum = get_modem_devnum();
	if (modem_devnum) {
		priority_status = get_nvram_int_checked_status("modem_prio",
							 &modem_priority);
		if (priority_status == 0 && modem_priority == 2) {
			wisp_status = get_apcli_wisp_ifname_status(&wisp_ifname);
			if (wisp_status != 0) {
				logmessage(LOGNAME,
					   "aborting USB modem WAN selection because AP-client WISP configuration is unavailable (status=%d)",
					   wisp_status);
				errno = wisp_status;
				return wisp_status;
			}
			is_apcli_wisp = (wisp_ifname != NULL);
			if (!is_apcli_wisp) {
				errno = 0;
				has_link = get_wan_ether_link_direct(0);
				if (has_link < 0)
					link_probe_status = errno > 0 ? errno : EIO;
			}
		}

		status = modem_wan_select_device_status(
			modem_devnum, priority_status, modem_priority,
			is_apcli_wisp, link_probe_status, has_link,
			&selected_devnum);
		if (status != 0) {
			logmessage(LOGNAME,
				   "aborting USB modem WAN selection because priority or Ethernet link state is unavailable (status=%d)",
				   status);
			errno = status;
			return status;
		}
		modem_devnum = selected_devnum;
	}
#endif
	status = set_usb_modem_dev_wan(0, modem_devnum);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot publish selected USB modem device for WAN0 (devnum=%d, status=%d)",
			   modem_devnum, status);
		return status;
	}
	return 0;
}
int
get_wan_ether_link_direct(int is_ap_mode)
{
	int ret = 0, wan_src_phy = SWAPI_PORT_WAN;
	int status;
	unsigned int phy_link = 0;

	if (!is_ap_mode) {
		status = get_nvram_int_checked_status("wan_src_phy", &wan_src_phy);
		if (status != 0) {
			errno = status;
			return -1;
		}
	}

	status = wan_src_phy_resolve(wan_src_phy, &wan_src_phy);
	if (status != 0) {
		errno = status;
		return -1;
	}

	errno = 0;
	ret = phy_status_port_link(wan_src_phy, &phy_link);
	if (ret != 0) {
		if (errno <= 0)
			errno = EIO;
		return -1;
	}

	return (phy_link) ? 1 : 0;
}

static int
get_wan_dns_static_status(int *is_static)
{
	int unit = 0, api_result, saved_errno, status, dns_enabled, wan_proto;
	int is_usb_modem = 0;
	char *stored_value = NULL;

	if (!is_static)
		return EINVAL;
	status = get_usb_modem_wan_status(unit, &is_usb_modem);
	if (status != 0) {
		errno = status;
		return status;
	}
	if (is_usb_modem) {
		/* The legacy nvram_match() default for an absent key is false. */
		*is_static = 0;
		errno = 0;
		api_result = nvram_get_checked("modem_dnsa", &stored_value);
		saved_errno = errno;
		status = rc_wan_nvram_read_status(api_result, saved_errno);
		if (status != 0)
			return status;
		if (api_result == 1 && !stored_value)
			return EIO;
		*is_static = (api_result == 1 && strcmp(stored_value, "0") == 0);
		return 0;
	}

	status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (status != 0)
		return status;
	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
		*is_static = 1;
		return 0;
	}

	/* Preserve the legacy fallback: failed/missing integer reads select static DNS. */
	*is_static = 1;
	status = get_wan_unit_value_int_status(unit, "dnsenable_x", &dns_enabled);
	if (status != 0)
		return status;
	*is_static = (dns_enabled < 1);
	return 0;
}

int
get_wan_dns_static(void)
{
	int is_static = 0;
	int status = get_wan_dns_static_status(&is_static);

	if (status != 0)
		errno = status;
	return is_static;
}

int
get_wan_wisp_active_status(int *p_is_active, int *p_has_link)
{
	int unit = 0;
	int read_status, validation_status = 0;
	int is_wisp = 0, status;
	char man_ifname[16], man_ifname_nvram[16];

	if (!p_is_active)
		return EINVAL;
	*p_is_active = 0;
	if (p_has_link)
		*p_has_link = 0;
	man_ifname[0] = '\0';
	man_ifname_nvram[0] = '\0';

	read_status = get_wan_unit_value_copy_status(
		unit, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
	if (read_status == 0 && man_ifname_nvram[0]) {
		validation_status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (validation_status == 0)
			is_wisp = is_man_wisp(man_ifname);
	}

	status = rc_wan_nvram_wisp_active_status(
		read_status, validation_status, is_wisp, p_is_active);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot determine manual WISP interface status (read=%d, validation=%d)",
			   read_status, validation_status);
		return status;
	}

	if (*p_is_active && p_has_link)
		*p_has_link = get_apcli_connected(man_ifname);
	return 0;
}

/*
 * Preserve the legacy boolean ABI conservatively: unknown MAN identity must
 * not be interpreted as "not WISP" by a caller that cannot handle errno.
 */
int
get_wan_wisp_active(int *p_has_link)
{
	int is_active = 0;
	int status;

	status = get_wan_wisp_active_status(&is_active, p_has_link);
	if (status != 0) {
		if (p_has_link)
			*p_has_link = 0;
		errno = status;
		return 1;
	}
	return is_active;
}

int
get_wan_ifname(char wan_ifname[16])
{
	int unit = 0;
	int status, is_usb_modem, is_ndis_modem = 0, wan_proto;
	char man_ifname[16], man_ifname_nvram[16], ifname_temp[16];
	const char *ifname = man_ifname;

	status = get_wan_unit_value_copy_status(unit, "ifname_t",
					       ifname_temp, sizeof(ifname_temp));
	if (status != 0) {
		errno = status;
		return status;
	}

	status = get_usb_modem_wan_status(unit, &is_usb_modem);
	if (status != 0) {
		errno = status;
		return status;
	}
	if (is_usb_modem) {
		status = get_wan_modem_is_ndis_status(&is_ndis_modem);
		if (status != 0) {
			errno = status;
			return status;
		}
		if (is_ndis_modem) {
			if (is_usbnet_interface(ifname_temp))
				ifname = ifname_temp;
		} else {
			if (ppp_ifindex(ifname_temp) >= RAS_PPP_UNIT)
				ifname = ifname_temp;
			else
				ifname = IFNAME_RAS;
		}
	} else {
		status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (status != 0) {
			errno = status;
			return status;
		}
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP) {
			if (ppp_ifindex(ifname_temp) >= WAN_PPP_UNIT)
				ifname = ifname_temp;
			else
				ifname = IFNAME_PPP;
		}
	}

	/*
	 * Only the manual interface is needed when no PPP/modem interface
	 * overrides it. Read it through the checked API so transient NVRAM
	 * errors cannot be converted into a misleading empty/invalid name.
	 */
	if (ifname == man_ifname) {
		status = get_wan_unit_value_copy_status(
			unit, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
		if (status != 0) {
			errno = status;
			return status;
		}
		status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (status != 0) {
			errno = status;
			return status;
		}
	}

	status = wan_ifname_copy(wan_ifname, 16, ifname);
	if (status != 0) {
		errno = status;
		return status;
	}
	return 0;
}

in_addr_t
get_wan_ip4(int only_broadband_wan)
{
	int unit = 0;
	int status, is_ap_mode, is_usb_modem = 0, wan_proto;
	char man_ifname[16], man_ifname_nvram[16], ifname_temp[16];
	const char *ifname = NULL;

	status = get_ap_mode_status(&is_ap_mode);
	if (status != 0) {
		errno = status;
		return INADDR_ANY;
	}
	if (is_ap_mode)
		return INADDR_ANY;

	status = get_wan_unit_value_copy_status(unit, "ifname_t",
					       ifname_temp, sizeof(ifname_temp));
	if (status != 0) {
		errno = status;
		return INADDR_ANY;
	}

	if (!only_broadband_wan) {
		status = get_usb_modem_wan_status(unit, &is_usb_modem);
		if (status != 0) {
			errno = status;
			return INADDR_ANY;
		}
	}
	if (!only_broadband_wan && is_usb_modem) {
		ifname = ifname_temp;
	} else {
		status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (status != 0) {
			errno = status;
			return INADDR_ANY;
		}
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP) {
			if (ppp_ifindex(ifname_temp) >= 0)
				ifname = ifname_temp;
			else
				ifname = IFNAME_PPP;
		}
	}

	/*
	 * A modem or PPP interface overrides the configured manual interface.
	 * Only read wan0_ifname when that fallback is actually selected; a
	 * nvram_safe_get()-backed getter would hide backend errors as an empty
	 * interface and can make callers mistake a read failure for no WAN IP.
	 */
	if (ifname == NULL) {
		status = get_wan_unit_value_copy_status(
			unit, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
		if (status != 0) {
			errno = status;
			return INADDR_ANY;
		}
		status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (status != 0) {
			errno = status;
			return INADDR_ANY;
		}
		ifname = man_ifname;
	}

	return get_interface_addr4(ifname);
}

int
has_wan_ip4(int only_broadband_wan)
{
	if (get_wan_ip4(only_broadband_wan) != INADDR_ANY)
		return 1;

	return 0;
}

int
has_wan_gw4(void)
{
	int unit = 0;

	if (is_valid_ipv4(get_wan_unit_value(unit, "gateway")))
		return 1;

	return 0;
}

static int
udhcpc_deconfig(char *wan_ifname, int is_zcip)
{
	int unit, wan_proto = 0, is_man, proto_status = 0;
	int interface_status = 0, reset_man_address = 0;
	int conntrack_status = 0;
	int cleanup_status;
	char log_prefix[32], *udhcpc_state;

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", (is_zcip) ? "ZeroConf" : "DHCP", (is_man) ? "MAN" : "WAN");
	udhcpc_state = "deconfig";

	if (is_man) {
		proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (proto_status != 0) {
			logmessage(log_prefix,
				   "cannot verify MAN protocol on %s (status=%d); preserving underlay address",
				   wan_ifname, proto_status);
			errno = proto_status;
		}
	}

	if (is_man && (proto_status != 0 ||
	               wan_proto == IPV4_WAN_PROTO_PPTP ||
	               wan_proto == IPV4_WAN_PROTO_L2TP))
	{
		/* fix hang-up issue */
//		logmessage(log_prefix, "skipping resetting IP address to 0.0.0.0");
		;
	}
	else
	{
		interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		if (interface_status != 0)
			logmessage(log_prefix, "failed to clear address on %s (error=%d); continuing lease cleanup",
				   wan_ifname, interface_status);

		if (is_man) {
			char man_addr[INET_ADDRSTRLEN];
			int man_addr_status;

			/*
			 * Do not silently skip stale MAN conntrack cleanup when the NVRAM
			 * backend is unavailable. A missing key remains an empty address.
			 */
			man_addr_status = rc_wan_nvram_copy_value(
				wan_nvram_checked_get_callback, NULL, "wanx_ipaddr",
				man_addr, sizeof(man_addr));
			if (man_addr_status != 0) {
				conntrack_status = man_addr_status;
				logmessage(log_prefix,
					   "cannot read MAN old address for conntrack cleanup (status=%d)",
					   man_addr_status);
			} else if (is_valid_ipv4(man_addr)) {
				conntrack_status = flush_conntrack_table(man_addr);
				if (conntrack_status != 0)
					logmessage(log_prefix, "MAN conntrack cleanup failed (status=%d)",
						   conntrack_status);
			}

			/* man_down_internal() needs the old address to remove its host route. */
			reset_man_address = (interface_status == 0);
		}
	}

	logmessage(log_prefix, "%s (%s): lease is lost",
		udhcpc_state, wan_ifname);

	if (is_man)
		cleanup_status = man_down_internal(wan_ifname, unit, reset_man_address);
	else
		cleanup_status = wan_down_internal(wan_ifname, unit, 0);

	if (cleanup_status != 0)
		logmessage(log_prefix, "route teardown incomplete on %s (error=%d)",
			   wan_ifname, cleanup_status);

	return route_status_accumulate(
		route_status_accumulate(
			route_status_accumulate(proto_status, interface_status),
			conntrack_status),
		cleanup_status);
}

struct wan_dhcp_interface_context {
	char *ifname;
};

static int
wan_dhcp_interface_clear(void *opaque)
{
	struct wan_dhcp_interface_context *context =
		(struct wan_dhcp_interface_context *)opaque;

	return ifconfig(context->ifname, IFUP, "0.0.0.0", NULL);
}

static int
wan_dhcp_interface_set(void *opaque, const char *ipaddr,
		       const char *netmask)
{
	struct wan_dhcp_interface_context *context =
		(struct wan_dhcp_interface_context *)opaque;

	return ifconfig(context->ifname, IFUP, (char *)ipaddr, (char *)netmask);
}

static const struct dhcp_wan_interface_ops wan_dhcp_interface_ops = {
	wan_dhcp_interface_clear,
	wan_dhcp_interface_set
};


struct wan_dhcp_route_transition_context {
	struct wan_dhcp_interface_context *interface_context;
	const char *prefix;
	const char *old_gateway;
	const char *old_ip;
	const char *old_mask;
	int route_metric;
	int restore_host_route;
	int remove_dhcp_routes;
};

static int
wan_dhcp_delete_old_host_route(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return route_del(context->interface_context->ifname,
			 context->route_metric, (char *)context->old_gateway,
			 NULL, "255.255.255.255");
}

static int
wan_dhcp_delete_old_default_route(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return route_del(context->interface_context->ifname,
			 context->route_metric, "0.0.0.0",
			 (char *)context->old_gateway, "0.0.0.0");
}

static int
wan_dhcp_delete_old_dhcp_routes(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return del_dhcp_routes_by_prefix((char *)context->prefix,
					 context->interface_context->ifname, 0);
}

static int
wan_dhcp_restore_old_address(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	if (dhcp_lease_ipv4_is_valid(context->old_ip) &&
	    dhcp_lease_netmask_is_valid(context->old_mask))
		return wan_dhcp_interface_set(context->interface_context,
					      context->old_ip, context->old_mask);
	return wan_dhcp_interface_clear(context->interface_context);
}

static int
wan_dhcp_restore_old_routes(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;
	char *ifname = context->interface_context->ifname;
	int status = 0, route_status;

	if (dhcp_lease_ipv4_is_valid(context->old_gateway)) {
		if (context->restore_host_route &&
		    dhcp_lease_ipv4_is_valid(context->old_ip) &&
		    dhcp_lease_netmask_is_valid(context->old_mask) &&
		    !is_same_subnet((char *)context->old_gateway,
				    (char *)context->old_ip,
				    (char *)context->old_mask)) {
			route_status = route_add(ifname, context->route_metric,
						  (char *)context->old_gateway,
						  NULL, "255.255.255.255");
			if (!dhcp_lease_route_add_is_ok(route_status))
				status = route_status_accumulate(status, route_status);
		}
		route_status = route_add(ifname, context->route_metric,
					 "0.0.0.0", (char *)context->old_gateway,
					 "0.0.0.0");
		if (!dhcp_lease_route_add_is_ok(route_status))
			status = route_status_accumulate(status, route_status);
	}
	if (context->remove_dhcp_routes) {
		route_status = add_dhcp_routes_by_prefix((char *)context->prefix,
							 ifname, 0);
		status = route_status_accumulate(status, route_status);
	}
	return status;
}

static const struct dhcp_wan_route_transition_ops wan_dhcp_route_transition_ops = {
	wan_dhcp_delete_old_host_route,
	wan_dhcp_delete_old_default_route,
	wan_dhcp_delete_old_dhcp_routes,
	wan_dhcp_restore_old_address,
	wan_dhcp_restore_old_routes
};

enum {
	WAN_DHCP_BOUND_IPADDR = 0,
	WAN_DHCP_BOUND_NETMASK,
	WAN_DHCP_BOUND_DNS,
	WAN_DHCP_BOUND_WINS,
	WAN_DHCP_BOUND_MTU,
	WAN_DHCP_BOUND_ROUTES,
	WAN_DHCP_BOUND_ROUTES_MS,
	WAN_DHCP_BOUND_ROUTES_RFC,
	WAN_DHCP_BOUND_DOMAIN,
	WAN_DHCP_BOUND_LEASE,
	WAN_DHCP_BOUND_GATEWAY,
	WAN_DHCP_BOUND_DLTIME,
	WAN_DHCP_BOUND_SNAPSHOT_COUNT
};

#define WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX 2049
#define WAN_DHCP_BOUND_SNAPSHOT_KEY_MAX 32

static const char *const wan_dhcp_bound_snapshot_params[] = {
	"ipaddr", "netmask", "dns", "wins", "mtu", "routes",
	"routes_ms", "routes_rfc", "domain", "lease", "gateway", "dltime"
};

enum {
	WAN_DHCP_VIPTV_SNAPSHOT_COUNT = 7
};

static const char *const wan_dhcp_viptv_snapshot_params[] = {
	"manv_ipaddr", "manv_netmask", "manv_gateway", "manv_routes",
	"manv_routes_ms", "manv_routes_rfc", "manv_mtu"
};

enum {
	WAN_ZCIP_SNAPSHOT_NETMASK = 0,
	WAN_ZCIP_SNAPSHOT_GATEWAY,
	WAN_ZCIP_SNAPSHOT_DNS,
	WAN_ZCIP_SNAPSHOT_IPADDR,
	WAN_ZCIP_SNAPSHOT_COUNT
};

static const char *const wan_zcip_snapshot_params[] = {
	"wanx_netmask", "wanx_gateway", "wanx_dns", "wanx_ipaddr"
};

enum {
	WAN_ZCIP_VIPTV_SNAPSHOT_NETMASK = 0,
	WAN_ZCIP_VIPTV_SNAPSHOT_GATEWAY,
	WAN_ZCIP_VIPTV_SNAPSHOT_IPADDR,
	WAN_ZCIP_VIPTV_SNAPSHOT_COUNT
};

static const char *const wan_zcip_viptv_snapshot_params[] = {
	"manv_netmask", "manv_gateway", "manv_ipaddr"
};

static int
wan_dhcp_nvram_state_publish(
	const struct dhcp_lease_nvram_value *values, size_t count,
	int *rollback_status);



static int
wan_dhcp_bound_snapshot_add(struct dhcp_lease_nvram_value *values,
			    size_t *count, const char *name,
			    const char *value, const char *previous_value);

static int
udhcpc_bound(char *wan_ifname, int is_renew_mode)
{
	char *value, *lan_ip, *lan_nm, *wan_ip, *wan_nm, *wan_gw;
	char tmp[100], prefix[16], route[32], log_prefix[32], *udhcpc_state;
	char gateway_buf[INET_ADDRSTRLEN], old_gateway[INET_ADDRSTRLEN];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char lan_ipaddr_snapshot[INET_ADDRSTRLEN];
	char lan_netmask_snapshot[INET_ADDRSTRLEN];
	char *lease_ip, *lease_netmask, *lease_dns, *lease_wins, *lease_domain;
	char lease_netmask_storage[INET_ADDRSTRLEN];
	char *lease_routes, *lease_routes_ms, *lease_routes_rfc, *route_value;
	char bound_names[WAN_DHCP_BOUND_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_KEY_MAX];
	char previous_text[WAN_DHCP_BOUND_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char routes_rfc_parse[WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char mtu_text[16], lease_text[16], dltime_text[16];
	unsigned int parsed_value;
	size_t i, snapshot_count, previous_snapshot_count;
	int unit, is_man, is_changed, i_err, has_gateway, lease_dur, dhcp_mtu;
	int mtu_supported;
	int wan_proto = 0, modem_unit_id, modem_status, proto_status, route_result, route_metric;
	int interface_status, rollback_status, route_setup_status, mtu_status;
	int lan_config_status;
	int remove_old_host_route, remove_old_dhcp_routes;
	int route_cleanup_status, route_rollback_status;
	int key_length, snapshot_status, snapshot_rollback_status;
	int netmask_read_status;
	int snapshot_abort_status, callback_status, publish_status;
	int dltime_length, snapshot_published = 0;
	int snapshot_restore_status = 0, snapshot_restore_rollback_status = 0;
	struct dhcp_lease_nvram_value snapshot_values[WAN_DHCP_BOUND_SNAPSHOT_COUNT];
	struct dhcp_lease_nvram_value rollback_values[WAN_DHCP_BOUND_SNAPSHOT_COUNT];
	struct nvram_transaction previous_transaction;
	struct nvram_transaction lan_transaction;
	struct rc_wan_nvram_snapshot_field previous_snapshot_fields[WAN_DHCP_BOUND_SNAPSHOT_COUNT];
	struct rc_wan_nvram_snapshot_field lan_snapshot_fields[2];
	struct wan_dhcp_interface_context interface_context;
	struct wan_dhcp_route_transition_context route_context;

	unit = get_wan_unit(wan_ifname);
	modem_status = is_ifunit_modem_status(wan_ifname, unit, &modem_unit_id);
	if (modem_status != 0) {
		logmessage(LOGNAME,
			   "refusing DHCP bound callback on %s because USB modem identity cannot be read (status=%d)",
			   wan_ifname, modem_status);
		errno = modem_status;
		return modem_status;
	}
	proto_status = 0;
	if (modem_unit_id) {
		is_man = 0;
	} else {
		proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
		if (proto_status != 0) {
			logmessage(LOGNAME,
				   "refusing DHCP bound callback on %s because WAN protocol cannot be read (status=%d)",
				   wan_ifname, proto_status);
			errno = proto_status;
			return proto_status;
		}
		is_man = !is_usbnet_interface(wan_ifname) &&
			(wan_proto == IPV4_WAN_PROTO_PPPOE ||
			 wan_proto == IPV4_WAN_PROTO_PPTP ||
			 wan_proto == IPV4_WAN_PROTO_L2TP);
	}
	route_setup_status = 0;
	mtu_status = 0;

	if (is_man) {
		strcpy(prefix, "wanx_");
	} else {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		/* Capture callback time now; publish only with an accepted lease. */
		dltime_length = snprintf(dltime_text, sizeof(dltime_text),
					 "%ld", uptime());
		if (dltime_length < 0)
			return errno ? errno : EIO;
		if ((size_t)dltime_length >= sizeof(dltime_text))
			return EOVERFLOW;
		snprintf(tmp, sizeof(tmp), "%ld", uptime());
	}

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	udhcpc_state = (is_renew_mode) ? "renew" : "bound";

	/* Reject a malformed lease before it can reach NVRAM or ifconfig. */
	value = getenv("ip");
	lease_ip = value ? trim_r(value) : "";
	value = getenv("subnet");
	if (value) {
		lease_netmask = trim_r(value);
	} else {
		netmask_read_status = rc_wan_nvram_copy_value(
			wan_nvram_checked_get_callback, NULL,
			strcat_r(prefix, "netmask", tmp),
			lease_netmask_storage, sizeof(lease_netmask_storage));
		if (netmask_read_status != 0) {
			errno = netmask_read_status;
			logmessage(log_prefix,
				   "cannot read fallback DHCP netmask on %s (status=%d)",
				   wan_ifname, netmask_read_status);
			return netmask_read_status;
		}
		lease_netmask = lease_netmask_storage;
	}
	if (!dhcp_lease_ipv4_is_valid(lease_ip) ||
	    !dhcp_lease_netmask_is_valid(lease_netmask)) {
		logmessage(log_prefix, "%s (%s) rejected invalid DHCP address/netmask",
			udhcpc_state, wan_ifname);
		return EINVAL;
	}

	/* Reject a malformed Router option before interface or old-route changes.
	 * A bad non-empty option must not be mistaken for a gateway withdrawal. */
	value = getenv("router");
	route_result = dhcp_lease_router_option_parse(
		value ? trim_r(value) : NULL,
		gateway_buf, sizeof(gateway_buf), &has_gateway);
	if (route_result != 0) {
		logmessage(log_prefix,
			   "%s (%s) rejected invalid DHCP router list (status=%d)",
			   udhcpc_state, wan_ifname, route_result);
		return route_result;
	}

	/*
	 * Capture the complete previous lease before changing the interface or
	 * deleting routes. Route options can be 2048 bytes; refuse an oversized
	 * old value rather than risk a truncated rollback snapshot.
	 */
	previous_snapshot_count = 0;
	for (i = 0; i < WAN_DHCP_BOUND_SNAPSHOT_COUNT; ++i) {
		/* MAN uses a separate metadata namespace and has no WAN dltime. */
		if (is_man && i == WAN_DHCP_BOUND_DLTIME) {
			previous_text[i][0] = '\0';
			continue;
		}
		key_length = snprintf(bound_names[i], sizeof(bound_names[i]),
				      "%s%s", prefix,
				      wan_dhcp_bound_snapshot_params[i]);
		if (key_length < 0 ||
		    (size_t)key_length >= sizeof(bound_names[i]))
			return ENAMETOOLONG;
		previous_snapshot_fields[previous_snapshot_count].param_name =
			bound_names[i];
		previous_snapshot_fields[previous_snapshot_count].destination =
			previous_text[i];
		previous_snapshot_fields[previous_snapshot_count].destination_size =
			sizeof(previous_text[i]);
		++previous_snapshot_count;
	}
	snapshot_status = rc_wan_nvram_copy_snapshot(
		previous_snapshot_fields, (unsigned int)previous_snapshot_count,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &previous_transaction);
	if (snapshot_status != 0) {
		logmessage(log_prefix,
			   "%s (%s) cannot snapshot previous DHCP NVRAM tuple (status=%d)",
			   udhcpc_state, wan_ifname, snapshot_status);
		return snapshot_status;
	}
	if (!dhcp_lease_text_fits_buffer(previous_text[WAN_DHCP_BOUND_GATEWAY], sizeof(old_gateway)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[WAN_DHCP_BOUND_IPADDR], sizeof(old_ip)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[WAN_DHCP_BOUND_NETMASK], sizeof(old_mask))) {
		logmessage(log_prefix, "%s (%s) rejected oversized previous DHCP address snapshot", udhcpc_state, wan_ifname);
		return ENAMETOOLONG;
	}
	snprintf(old_gateway, sizeof(old_gateway), "%s",
	 previous_text[WAN_DHCP_BOUND_GATEWAY]);
	snprintf(old_ip, sizeof(old_ip), "%s",
	 previous_text[WAN_DHCP_BOUND_IPADDR]);
	snprintf(old_mask, sizeof(old_mask), "%s",
	 previous_text[WAN_DHCP_BOUND_NETMASK]);
	route_metric = is_man ? 2 : 1;

	/*
	 * Configure the candidate address before removing the previous lease's
	 * routes or publishing new NVRAM state. A failed ioctl must leave the
	 * last-known-good lease and its routes available for the next callback.
	 */
	is_changed = strcmp(old_ip, lease_ip) != 0;
	i_err = 0;
	wan_ip = lease_ip;
	wan_nm = lease_netmask;
	/*
	 * The WAN/LAN overlap decision must use one published LAN subnet
	 * generation. Independent reads can pair an old address with a newer
	 * broader netmask while the LAN configuration is being committed.
	 */
	lan_snapshot_fields[0].param_name = "lan_ipaddr";
	lan_snapshot_fields[0].destination = lan_ipaddr_snapshot;
	lan_snapshot_fields[0].destination_size = sizeof(lan_ipaddr_snapshot);
	lan_snapshot_fields[1].param_name = "lan_netmask";
	lan_snapshot_fields[1].destination = lan_netmask_snapshot;
	lan_snapshot_fields[1].destination_size = sizeof(lan_netmask_snapshot);
	lan_config_status = rc_wan_nvram_copy_snapshot(
		lan_snapshot_fields, 2,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &lan_transaction);
	if (lan_config_status != 0) {
		errno = lan_config_status;
		logmessage(log_prefix,
			   "cannot snapshot LAN subnet before WAN DHCP overlap check on %s (status=%d)",
			   wan_ifname, lan_config_status);
		return lan_config_status;
	}
	lan_ip = lan_ipaddr_snapshot;
	lan_nm = lan_netmask_snapshot;
	interface_context.ifname = wan_ifname;

	if (is_same_subnet2(wan_ip, lan_ip, wan_nm, lan_nm)) {
		if (!is_renew_mode) {
			interface_status = wan_dhcp_interface_clear(&interface_context);
			if (interface_status != 0) {
				logmessage(log_prefix,
					   "failed to clear conflicting DHCP address on %s (status=%d)",
					   wan_ifname, interface_status);
				return interface_status;
			}
		}
		i_err = 1;
		logmessage(log_prefix,
			   "%s (%s). ERROR! Obtained IP [%s/%s] in LAN subnet [%s/%s]. Need change LAN subnet!",
			   udhcpc_state, wan_ifname, wan_ip, wan_nm, lan_ip, lan_nm);
	} else {
		interface_status = dhcp_wan_interface_apply(
			old_ip, old_mask, lease_ip, lease_netmask, is_changed,
			&wan_dhcp_interface_ops, &interface_context, &rollback_status);
		if (interface_status != 0) {
			logmessage(log_prefix,
				   "failed to apply DHCP address on %s (status=%d); preserving previous lease and routes",
				   wan_ifname, interface_status);
			if (rollback_status != 0)
				logmessage(log_prefix,
					   "failed to restore previous DHCP address on %s (status=%d)",
					   wan_ifname, rollback_status);
			return interface_status;
		}
	}

	/* The old route inventory is still in NVRAM here. Do not publish a
	 * replacement lease if deleting that inventory fails: restore the old
	 * interface address and every route, then let udhcpc retry the callback. */
	remove_old_host_route = dhcp_lease_ipv4_is_valid(old_gateway) &&
		dhcp_lease_ipv4_is_valid(old_ip) &&
		dhcp_lease_netmask_is_valid(old_mask) &&
		!is_same_subnet(old_gateway, old_ip, old_mask);
	remove_old_dhcp_routes = is_man || modem_unit_id == 2 ||
		(!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP);
	route_context.interface_context = &interface_context;
	route_context.prefix = prefix;
	route_context.old_gateway = old_gateway;
	route_context.old_ip = old_ip;
	route_context.old_mask = old_mask;
	route_context.route_metric = route_metric;
	route_context.restore_host_route = remove_old_host_route;
	route_context.remove_dhcp_routes = remove_old_dhcp_routes;
	route_cleanup_status = dhcp_wan_route_transition_apply(
		remove_old_host_route, dhcp_lease_ipv4_is_valid(old_gateway),
		remove_old_dhcp_routes, &wan_dhcp_route_transition_ops,
		&route_context, &route_rollback_status);
	if (route_cleanup_status != 0) {
		logmessage(log_prefix,
			   "failed to remove previous DHCP route inventory on %s (status=%d); preserving old lease",
			   wan_ifname, route_cleanup_status);
		if (route_rollback_status != 0)
			logmessage(log_prefix,
				   "failed to restore previous DHCP route inventory on %s (status=%d)",
				   wan_ifname, route_rollback_status);
		return route_cleanup_status;
	}

	lease_dur = 0;
	dhcp_mtu = 0;
	snapshot_count = 0;
	snapshot_rollback_status = 0;

	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_IPADDR], lease_ip,
		previous_text[WAN_DHCP_BOUND_IPADDR]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_NETMASK], lease_netmask,
		previous_text[WAN_DHCP_BOUND_NETMASK]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	value = getenv("dns");
	lease_dns = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_dns, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP DNS list on %s", wan_ifname);
		lease_dns = "";
	}
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, bound_names[WAN_DHCP_BOUND_DNS],
		lease_dns, previous_text[WAN_DHCP_BOUND_DNS]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	value = getenv("wins");
	lease_wins = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_wins, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP WINS list on %s", wan_ifname);
		lease_wins = "";
	}
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, bound_names[WAN_DHCP_BOUND_WINS],
		lease_wins, previous_text[WAN_DHCP_BOUND_WINS]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	value = getenv("mtu");
	if (value)
		value = trim_r(value);
	snapshot_status = dhcp_lease_mtu_option_value(
		value, &dhcp_mtu, &mtu_supported);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	if (value && !mtu_supported)
		logmessage(log_prefix, "ignoring invalid DHCP MTU on %s", wan_ifname);
	snprintf(mtu_text, sizeof(mtu_text), "%d", dhcp_mtu);
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_MTU], mtu_text,
		previous_text[WAN_DHCP_BOUND_MTU]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	route_value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP route option on %s", wan_ifname);
		route_value = "";
	}
	lease_routes = route_value;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_ROUTES], lease_routes,
		previous_text[WAN_DHCP_BOUND_ROUTES]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	route_value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP Microsoft route option on %s", wan_ifname);
		route_value = "";
	}
	lease_routes_ms = route_value;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_ROUTES_MS], lease_routes_ms,
		previous_text[WAN_DHCP_BOUND_ROUTES_MS]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	route_value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP classless route option on %s", wan_ifname);
		route_value = "";
	}
	lease_routes_rfc = route_value;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_ROUTES_RFC], lease_routes_rfc,
		previous_text[WAN_DHCP_BOUND_ROUTES_RFC]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	value = getenv("domain");
	lease_domain = value ? trim_r(value) : "";
	if (*lease_domain && !dnsmasq_valid_server_domain(lease_domain)) {
		logmessage(log_prefix, "ignoring invalid DHCP domain on %s", wan_ifname);
		lease_domain = "";
	}
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, bound_names[WAN_DHCP_BOUND_DOMAIN],
		lease_domain, previous_text[WAN_DHCP_BOUND_DOMAIN]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value)) {
			lease_dur = 0;
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", wan_ifname);
		} else {
			lease_dur = (int)parsed_value;
		}
		snprintf(lease_text, sizeof(lease_text), "%d", lease_dur);
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count,
			bound_names[WAN_DHCP_BOUND_LEASE], lease_text,
			previous_text[WAN_DHCP_BOUND_LEASE]);
		if (snapshot_status != 0)
			goto snapshot_publication_failed;
	}

	if (!has_gateway) {
		snprintf(routes_rfc_parse, sizeof(routes_rfc_parse), "%s",
			 lease_routes_rfc);
		foreach(route, routes_rfc_parse, value) {
			if (has_gateway) {
				snprintf(gateway_buf, sizeof(gateway_buf), "%s", route);
				break;
			} else
				has_gateway = !strcmp(route, "0.0.0.0/0");
		}
	}
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count,
		bound_names[WAN_DHCP_BOUND_GATEWAY], gateway_buf,
		previous_text[WAN_DHCP_BOUND_GATEWAY]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	if (!is_man) {
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count,
			bound_names[WAN_DHCP_BOUND_DLTIME], dltime_text,
			previous_text[WAN_DHCP_BOUND_DLTIME]);
		if (snapshot_status != 0)
			goto snapshot_publication_failed;
	}

	/*
	 * Do not expose a mixed lease tuple after the old route inventory has
	 * been removed. On any failed setter/close, restore the complete old
	 * NVRAM tuple under a fresh transaction, then restore the old address
	 * and routes before returning the original publication error.
	 */
	snapshot_status = wan_dhcp_nvram_state_publish(
		snapshot_values, snapshot_count, &snapshot_rollback_status);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_published = 1;

#if defined (USE_IPV6)
	value = getenv("ip6rd");
	if (value) {
		snapshot_status = store_ip6rd_from_dhcp_status(value, prefix);
		if (snapshot_status != 0)
			goto snapshot_publication_failed;
	}
#endif

	if (!i_err) {
		if (wan_dhcp_mtu_is_supported(dhcp_mtu)) {
			mtu_status = set_interface_mtu(wan_ifname, dhcp_mtu);
			if (mtu_status != 0)
				logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)",
					   udhcpc_state, wan_ifname, mtu_status);
		}

		wan_gw = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
		logmessage(log_prefix,
			"%s (%s), IP: %s, GW: %s, lease time: %d",
			udhcpc_state, wan_ifname, wan_ip, wan_gw, lease_dur);

		if (is_man)
			route_setup_status = man_up_internal(wan_ifname, unit, 0, NULL);
		else
			route_setup_status = wan_up_internal(wan_ifname, unit, 0);
		if (route_setup_status != 0)
			logmessage(log_prefix,
				   "%s (%s) accepted but route setup is incomplete (error=%d)",
				   udhcpc_state, wan_ifname, route_setup_status);
	}

	callback_status = route_status_accumulate(route_setup_status, mtu_status);
	publish_status = wan_nvram_int_temp_status(
		strcat_r(prefix, "mtu_err", tmp), mtu_status);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish DHCP MTU retry state on %s (status=%d)",
			   wan_ifname, publish_status);

	publish_status = wan_nvram_int_temp_status(
		strcat_r(prefix, "route_err", tmp), route_setup_status);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish DHCP route retry state on %s (status=%d)",
			   wan_ifname, publish_status);

	publish_status = wan_nvram_int_temp_status(
		strcat_r(prefix, "err", tmp), i_err);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish DHCP lease error state on %s (status=%d)",
			   wan_ifname, publish_status);

	return callback_status;

snapshot_publication_failed:
	if (snapshot_published) {
		/*
		 * The later optional 6rd write failed after the IPv4 tuple was
		 * committed. Compare-and-restore only while every current field
		 * still matches this callback's committed lease; never undo a
		 * newer DHCP callback.
		 */
		snapshot_restore_status = dhcp_lease_nvram_reverse_snapshot(
			snapshot_values, snapshot_count, rollback_values);
		if (snapshot_restore_status != 0) {
			logmessage(log_prefix,
				   "cannot prepare reverse DHCP lease snapshot on %s (status=%d); preserving current interface/routes",
				   wan_ifname, snapshot_restore_status);
			return snapshot_status;
		}
		snapshot_restore_status = wan_dhcp_nvram_state_publish(
			rollback_values, snapshot_count,
			&snapshot_restore_rollback_status);
		if (snapshot_restore_status != 0) {
			logmessage(log_prefix,
				   "cannot restore committed DHCP lease snapshot after 6rd failure on %s (status=%d, rollback=%d); preserving current interface/routes",
				   wan_ifname, snapshot_restore_status,
				   snapshot_restore_rollback_status);
			return snapshot_status;
		}
		snapshot_published = 0;
	}
	logmessage(log_prefix,
		   "%s (%s) cannot publish complete DHCP lease snapshot (status=%d)",
		   udhcpc_state, wan_ifname, snapshot_status);
	if (snapshot_rollback_status != 0) {
		logmessage(log_prefix,
			   "DHCP lease snapshot restoration incomplete on %s (status=%d); not replaying the possibly mixed DHCP route inventory",
			   wan_ifname, snapshot_rollback_status);
		/* The old address/default route are still independently recoverable. */
		route_context.remove_dhcp_routes = 0;
	}
	route_rollback_status = 0;
	snapshot_abort_status = dhcp_wan_route_transition_abort_snapshot(
		snapshot_status, &wan_dhcp_route_transition_ops, &route_context,
		&route_rollback_status);
	if (route_rollback_status != 0)
		logmessage(log_prefix,
			   "failed to restore previous DHCP address/routes on %s after snapshot error (status=%d)",
			   wan_ifname, route_rollback_status);
	return snapshot_abort_status;
}

static int
udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)
{
	char log_prefix[32];
	const char *udhcpc_state;
	char *value, *lan_ip, *lan_nm, *ip, *nm, *gw, *rt, *rt_ms, *rt_rfc;
	char gateway_buf[INET_ADDRSTRLEN], old_gateway[INET_ADDRSTRLEN];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char lan_ipaddr_snapshot[INET_ADDRSTRLEN];
	char lan_netmask_snapshot[INET_ADDRSTRLEN];
	char previous_text[WAN_DHCP_VIPTV_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char mtu_text[16];
	unsigned int parsed_value;
	size_t i, snapshot_count, previous_snapshot_count;
	int lease_dur, is_changed, i_err, dhcp_mtu, mtu_seen, route_result;
	int mtu_supported;
	int interface_status, rollback_status, route_setup_status, mtu_status;
	int lan_config_status;
	int route_cleanup_status, route_rollback_status, multicast_status = 0;
	int snapshot_status, snapshot_rollback_status, snapshot_abort_status;
	int callback_status, publish_status;
	struct dhcp_lease_nvram_value snapshot_values[WAN_DHCP_VIPTV_SNAPSHOT_COUNT];
	struct nvram_transaction previous_transaction;
	struct nvram_transaction lan_transaction;
	struct rc_wan_nvram_snapshot_field previous_snapshot_fields[WAN_DHCP_VIPTV_SNAPSHOT_COUNT];
	struct rc_wan_nvram_snapshot_field lan_snapshot_fields[2];
	struct wan_dhcp_interface_context interface_context;
	struct wan_dhcp_route_transition_context route_context;

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	udhcpc_state = (is_renew_mode) ? "renew" : "bound";

	value = getenv("ip");
	ip = value ? trim_r(value) : "";
	value = getenv("subnet");
	nm = value ? trim_r(value) : "255.255.0.0";
	if (!dhcp_lease_ipv4_is_valid(ip) || !dhcp_lease_netmask_is_valid(nm)) {
		logmessage(log_prefix, "%s (%s) rejected invalid DHCP address/netmask",
			udhcpc_state, man_ifname);
		return EINVAL;
	}

	/*
	 * Preserve every published IPTV field before touching the interface or
	 * route table. Route text is bounded to the full DHCP option size so an
	 * unsuccessful publication can restore the exact old lease tuple.
	 */
	previous_snapshot_count = 0;
	for (i = 0; i < WAN_DHCP_VIPTV_SNAPSHOT_COUNT; ++i) {
		previous_snapshot_fields[previous_snapshot_count].param_name =
			wan_dhcp_viptv_snapshot_params[i];
		previous_snapshot_fields[previous_snapshot_count].destination =
			previous_text[i];
		previous_snapshot_fields[previous_snapshot_count].destination_size =
			sizeof(previous_text[i]);
		++previous_snapshot_count;
	}
	snapshot_status = rc_wan_nvram_copy_snapshot(
		previous_snapshot_fields, (unsigned int)previous_snapshot_count,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &previous_transaction);
	if (snapshot_status != 0) {
		logmessage(log_prefix,
			   "%s (%s) cannot snapshot previous IPTV DHCP NVRAM tuple (status=%d)",
			   udhcpc_state, man_ifname, snapshot_status);
		return snapshot_status;
	}
	if (!dhcp_lease_text_fits_buffer(previous_text[2], sizeof(old_gateway)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[0], sizeof(old_ip)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[1], sizeof(old_mask))) {
		logmessage(log_prefix, "%s (%s) rejected oversized previous IPTV address snapshot", udhcpc_state, man_ifname);
		return ENAMETOOLONG;
	}
	snprintf(old_gateway, sizeof(old_gateway), "%s", previous_text[2]);
	snprintf(old_ip, sizeof(old_ip), "%s", previous_text[0]);
	snprintf(old_mask, sizeof(old_mask), "%s", previous_text[1]);
	interface_context.ifname = man_ifname;
	is_changed = strcmp(old_ip, ip) != 0;

	gateway_buf[0] = '\0';
	value = getenv("router");
	if (value) {
		route_result = dhcp_lease_ipv4_list_first(trim_r(value),
			gateway_buf, sizeof(gateway_buf));
		if (route_result < 0)
			logmessage(log_prefix, "ignoring invalid DHCP router list on %s", man_ifname);
	}
	gw = gateway_buf;

	value = getenv("routes");
	rt = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV route option on %s", man_ifname);
		rt = "";
	}
	value = getenv("msstaticroutes");
	rt_ms = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt_ms)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV Microsoft route option on %s", man_ifname);
		rt_ms = "";
	}
	value = getenv("staticroutes");
	rt_rfc = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt_rfc)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV classless route option on %s", man_ifname);
		rt_rfc = "";
	}

	lease_dur = 0;
	dhcp_mtu = 0;
	mtu_seen = 0;
	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value))
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", man_ifname);
		else
			lease_dur = (int)parsed_value;
	}
	value = getenv("mtu");
	mtu_seen = 1;
	if (value)
		value = trim_r(value);
	snapshot_status = dhcp_lease_mtu_option_value(
		value, &dhcp_mtu, &mtu_supported);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	if (value && !mtu_supported)
		logmessage(log_prefix, "ignoring invalid DHCP MTU on %s", man_ifname);

	/*
	 * The IPTV/LAN overlap decision requires one coherent LAN subnet.
	 * Separate reads can combine an old address with a newly broadened
	 * netmask while the administrator is changing the LAN configuration.
	 */
	lan_snapshot_fields[0].param_name = "lan_ipaddr";
	lan_snapshot_fields[0].destination = lan_ipaddr_snapshot;
	lan_snapshot_fields[0].destination_size = sizeof(lan_ipaddr_snapshot);
	lan_snapshot_fields[1].param_name = "lan_netmask";
	lan_snapshot_fields[1].destination = lan_netmask_snapshot;
	lan_snapshot_fields[1].destination_size = sizeof(lan_netmask_snapshot);
	lan_config_status = rc_wan_nvram_copy_snapshot(
		lan_snapshot_fields, 2,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &lan_transaction);
	if (lan_config_status != 0) {
		errno = lan_config_status;
		logmessage(log_prefix,
			   "cannot snapshot LAN subnet before IPTV DHCP overlap check on %s (status=%d)",
			   man_ifname, lan_config_status);
		return lan_config_status;
	}
	lan_ip = lan_ipaddr_snapshot;
	lan_nm = lan_netmask_snapshot;
	i_err = 0;
	if (is_same_subnet2(ip, lan_ip, nm, lan_nm)) {
		if (!is_renew_mode) {
			interface_status = wan_dhcp_interface_clear(&interface_context);
			if (interface_status != 0) {
				logmessage(log_prefix,
					"failed to clear conflicting IPTV DHCP address on %s (error=%d)",
					man_ifname, interface_status);
				return interface_status;
			}
		}

		i_err = 1;
		logmessage(log_prefix,
			"%s (%s). ERROR! Obtained IP [%s/%s] in LAN subnet [%s/%s]. Need change LAN subnet!",
			udhcpc_state, man_ifname, ip, nm, lan_ip, lan_nm);
	} else {
		/*
		 * Do not publish a lease or discard old routes until the new
		 * interface address is confirmed. Restore the last-known-good
		 * address if either address operation fails.
		 */
		interface_status = dhcp_wan_interface_apply(
			old_ip, old_mask, ip, nm, is_changed,
			&wan_dhcp_interface_ops, &interface_context, &rollback_status);
		if (interface_status != 0) {
			logmessage(log_prefix,
				"failed to apply IPTV DHCP address on %s (error=%d); preserving previous lease and routes",
				man_ifname, interface_status);
			if (rollback_status != 0)
				logmessage(log_prefix,
					"failed to restore previous IPTV DHCP address on %s (error=%d)",
					man_ifname, rollback_status);
			return interface_status;
		}
	}

	/*
	 * Keep the old IPTV lease as a recoverable unit. If any old route cannot
	 * be removed, restore both its address and route inventory before retry.
	 */
	route_context.interface_context = &interface_context;
	route_context.prefix = "manv_";
	route_context.old_gateway = old_gateway;
	route_context.old_ip = old_ip;
	route_context.old_mask = old_mask;
	route_context.route_metric = 10;
	route_context.restore_host_route = 0;
	route_context.remove_dhcp_routes = 1;
	route_cleanup_status = dhcp_wan_route_transition_apply(
		0, dhcp_lease_ipv4_is_valid(old_gateway), 1,
		&wan_dhcp_route_transition_ops, &route_context,
		&route_rollback_status);
	if (route_cleanup_status != 0) {
		logmessage(log_prefix,
			"previous IPTV route teardown incomplete on %s (error=%d); preserving previous lease and route inventory",
			man_ifname, route_cleanup_status);
		if (route_rollback_status != 0)
			logmessage(log_prefix,
				"failed to restore previous IPTV route inventory on %s (error=%d)",
				man_ifname, route_rollback_status);
		return route_cleanup_status;
	}

	/*
	 * Publish the new IPTV lease as one checked NVRAM tuple. If a setter or
	 * transaction close fails, restore the old tuple and then restore its
	 * address/routes before allowing udhcpc to retry.
	 */
	snapshot_count = 0;
	snapshot_rollback_status = 0;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[0],
		ip, previous_text[0]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[1],
		nm, previous_text[1]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[2],
		gw, previous_text[2]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[3],
		rt, previous_text[3]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[4],
		rt_ms, previous_text[4]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_dhcp_viptv_snapshot_params[5],
		rt_rfc, previous_text[5]);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;
	if (mtu_seen) {
		snprintf(mtu_text, sizeof(mtu_text), "%d", dhcp_mtu);
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count,
			wan_dhcp_viptv_snapshot_params[6], mtu_text,
			previous_text[6]);
		if (snapshot_status != 0)
			goto snapshot_publication_failed;
	}
	snapshot_status = wan_dhcp_nvram_state_publish(
		snapshot_values, snapshot_count, &snapshot_rollback_status);
	if (snapshot_status != 0)
		goto snapshot_publication_failed;

	route_setup_status = 0;
	mtu_status = 0;
	if (!i_err) {
		if (wan_dhcp_mtu_is_supported(dhcp_mtu)) {
			mtu_status = set_interface_mtu(man_ifname, dhcp_mtu);
			if (mtu_status != 0)
				logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)",
					   udhcpc_state, man_ifname, mtu_status);
		}

		logmessage(log_prefix,
			"%s (%s), IP: %s, GW: %s, lease time: %d",
			udhcpc_state, man_ifname, ip, gw, lease_dur);

		if (*rt || *rt_rfc || *rt_ms) {
			route_result = add_dhcp_routes(rt, rt_rfc, rt_ms, man_ifname, 0);
			route_setup_status = route_status_accumulate(route_setup_status, route_result);
		}

		/* Default route via IPTV gateway (metric 10). */
		if (dhcp_lease_ipv4_is_valid(gw)) {
			route_result = route_add(man_ifname, 10, "0.0.0.0", gw, "0.0.0.0");
			route_setup_status = route_status_accumulate(route_setup_status, route_result);
			if (route_result != 0)
				logmessage(log_prefix, "failed to add IPTV default route via %s (error=%d)",
					gw, route_result);
		}

		if (route_setup_status != 0)
			logmessage(log_prefix,
				"%s (%s) accepted but route setup is incomplete (error=%d)",
				udhcpc_state, man_ifname, route_setup_status);
		multicast_status = start_igmpproxy(man_ifname);
		if (multicast_status != 0)
			logmessage(log_prefix,
				"%s (%s) multicast startup is incomplete (error=%d)",
				udhcpc_state, man_ifname, multicast_status);
	}

	/* Keep route retry state independent from the IPTV proxy lifecycle. */
	callback_status = route_status_accumulate(route_setup_status, multicast_status);
	callback_status = route_status_accumulate(callback_status, mtu_status);
	publish_status = wan_nvram_int_temp_status("manv_mtu_err", mtu_status);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish IPTV MTU retry state (status=%d)",
			   publish_status);

	publish_status = wan_nvram_int_temp_status("manv_route_err", route_setup_status);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish IPTV route retry state (status=%d)",
			   publish_status);

	publish_status = wan_nvram_int_temp_status("manv_err", i_err);
	callback_status = route_status_accumulate(callback_status, publish_status);
	if (publish_status != 0)
		logmessage(log_prefix,
			   "cannot publish IPTV lease error state (status=%d)",
			   publish_status);

	return callback_status;

snapshot_publication_failed:
	logmessage(log_prefix,
		   "%s (%s) cannot publish complete IPTV DHCP snapshot (status=%d)",
		   udhcpc_state, man_ifname, snapshot_status);
	if (snapshot_rollback_status != 0) {
		logmessage(log_prefix,
			   "IPTV DHCP snapshot restoration incomplete on %s (status=%d); not replaying possibly mixed route inventory",
			   man_ifname, snapshot_rollback_status);
		route_context.remove_dhcp_routes = 0;
	}
	route_rollback_status = 0;
	snapshot_abort_status = dhcp_wan_route_transition_abort_snapshot(
		snapshot_status, &wan_dhcp_route_transition_ops, &route_context,
		&route_rollback_status);
	if (route_rollback_status != 0)
		logmessage(log_prefix,
			   "failed to restore previous IPTV address/routes on %s after snapshot error (status=%d)",
			   man_ifname, route_rollback_status);
	return snapshot_abort_status;
}

static int
zcip_bound(char *man_ifname)
{
	char *value, *ip;
	char log_prefix[32];
	const char *udhcpc_state;
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char previous_text[WAN_ZCIP_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char *zeroconf_mask = "255.255.0.0";
	size_t i, snapshot_count, previous_snapshot_count;
	int unit, interface_status, rollback_status, route_setup_status;
	int snapshot_status, snapshot_rollback_status, interface_rollback_status;
	struct dhcp_lease_nvram_value snapshot_values[WAN_ZCIP_SNAPSHOT_COUNT];
	struct nvram_transaction previous_transaction;
	struct rc_wan_nvram_snapshot_field previous_snapshot_fields[WAN_ZCIP_SNAPSHOT_COUNT];
	struct wan_dhcp_interface_context interface_context;

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "ZeroConf", "WAN");
	udhcpc_state = "bound";

	value = getenv("ip");
	if (!value) {
		logmessage(LOGNAME, "ZeroConf received no address on %s", man_ifname);
		return EINVAL;
	}
	ip = trim_r(value);
	if (!dhcp_lease_ipv4_is_valid(ip)) {
		logmessage(LOGNAME, "ZeroConf rejected invalid address on %s", man_ifname);
		return EINVAL;
	}

	/* Capture every field before the new link address can become visible. */
	previous_snapshot_count = 0;
	for (i = 0; i < WAN_ZCIP_SNAPSHOT_COUNT; ++i) {
		previous_snapshot_fields[previous_snapshot_count].param_name =
			wan_zcip_snapshot_params[i];
		previous_snapshot_fields[previous_snapshot_count].destination =
			previous_text[i];
		previous_snapshot_fields[previous_snapshot_count].destination_size =
			sizeof(previous_text[i]);
		++previous_snapshot_count;
	}
	snapshot_status = rc_wan_nvram_copy_snapshot(
		previous_snapshot_fields, (unsigned int)previous_snapshot_count,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &previous_transaction);
	if (snapshot_status != 0) {
		logmessage(log_prefix,
			   "%s (%s) cannot snapshot previous ZeroConf WAN NVRAM tuple (status=%d)",
			   udhcpc_state, man_ifname, snapshot_status);
		return snapshot_status;
	}
	if (!dhcp_lease_text_fits_buffer(previous_text[WAN_ZCIP_SNAPSHOT_IPADDR], sizeof(old_ip)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[WAN_ZCIP_SNAPSHOT_NETMASK], sizeof(old_mask))) {
		logmessage(LOGNAME, "ZeroConf rejected oversized previous WAN address snapshot on %s", man_ifname);
		return ENAMETOOLONG;
	}
	snprintf(old_ip, sizeof(old_ip), "%s",
		 previous_text[WAN_ZCIP_SNAPSHOT_IPADDR]);
	snprintf(old_mask, sizeof(old_mask), "%s",
		 previous_text[WAN_ZCIP_SNAPSHOT_NETMASK]);

	interface_context.ifname = man_ifname;
	interface_status = dhcp_wan_interface_apply(old_ip, old_mask, ip, zeroconf_mask, 1,
		&wan_dhcp_interface_ops, &interface_context, &rollback_status);
	if (interface_status != 0) {
		logmessage(LOGNAME, "failed to apply ZeroConf address on %s (error=%d)", man_ifname, interface_status);
		if (rollback_status != 0)
			logmessage(LOGNAME, "failed to restore previous ZeroConf WAN address on %s (error=%d)", man_ifname, rollback_status);
		return interface_status;
	}

	/*
	 * Commit the address, empty gateway/DNS and mask together. If the NVRAM
	 * transaction fails, restore the previous interface address before the
	 * MAN route setup can observe a mixed lease.
	 */
	snapshot_count = 0;
	snapshot_rollback_status = 0;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_zcip_snapshot_params[WAN_ZCIP_SNAPSHOT_NETMASK],
		zeroconf_mask, previous_text[WAN_ZCIP_SNAPSHOT_NETMASK]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, wan_zcip_snapshot_params[WAN_ZCIP_SNAPSHOT_GATEWAY],
			"", previous_text[WAN_ZCIP_SNAPSHOT_GATEWAY]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, wan_zcip_snapshot_params[WAN_ZCIP_SNAPSHOT_DNS],
			"", previous_text[WAN_ZCIP_SNAPSHOT_DNS]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, wan_zcip_snapshot_params[WAN_ZCIP_SNAPSHOT_IPADDR],
			ip, previous_text[WAN_ZCIP_SNAPSHOT_IPADDR]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_nvram_state_publish(
			snapshot_values, snapshot_count, &snapshot_rollback_status);
	if (snapshot_status != 0) {
		logmessage(LOGNAME,
			   "cannot publish complete ZeroConf WAN snapshot on %s (status=%d)",
			   man_ifname, snapshot_status);
		interface_rollback_status = dhcp_wan_interface_restore_previous(
			old_ip, old_mask, &wan_dhcp_interface_ops, &interface_context);
		if (interface_rollback_status != 0)
			logmessage(LOGNAME,
				   "failed to restore previous ZeroConf WAN address on %s (error=%d)",
				   man_ifname, interface_rollback_status);
		if (snapshot_rollback_status != 0)
			logmessage(LOGNAME,
				   "ZeroConf WAN NVRAM snapshot restoration incomplete on %s (status=%d)",
				   man_ifname, snapshot_rollback_status);
		return snapshot_status;
	}

	unit = get_wan_unit(man_ifname);
	route_setup_status = man_up_internal(man_ifname, unit, 0, NULL);
	if (route_setup_status != 0)
		logmessage(LOGNAME, "ZeroConf address on %s is active but MAN route setup is incomplete (error=%d)", man_ifname, route_setup_status);
	return route_setup_status;
}

static int
zcip_viptv_bound(char *man_ifname)
{
	char *value, *ip;
	char log_prefix[32];
	const char *udhcpc_state;
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char *zeroconf_mask = "255.255.0.0";
	size_t i, snapshot_count, previous_snapshot_count;
	int interface_status, rollback_status;
	int snapshot_status, snapshot_rollback_status, interface_rollback_status;
	struct dhcp_lease_nvram_value snapshot_values[WAN_ZCIP_VIPTV_SNAPSHOT_COUNT];
	struct nvram_transaction previous_transaction;
	struct rc_wan_nvram_snapshot_field previous_snapshot_fields[WAN_ZCIP_VIPTV_SNAPSHOT_COUNT];
	struct wan_dhcp_interface_context interface_context;

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "ZeroConf", "IPTV");
	udhcpc_state = "bound";

	value = getenv("ip");
	if (!value) {
		logmessage(LOGNAME, "ZeroConf IPTV received no address on %s", man_ifname);
		return EINVAL;
	}
	ip = trim_r(value);
	if (!dhcp_lease_ipv4_is_valid(ip)) {
		logmessage(LOGNAME, "ZeroConf IPTV rejected invalid address on %s", man_ifname);
		return EINVAL;
	}

	previous_snapshot_count = 0;
	for (i = 0; i < WAN_ZCIP_VIPTV_SNAPSHOT_COUNT; ++i) {
		previous_snapshot_fields[previous_snapshot_count].param_name =
			wan_zcip_viptv_snapshot_params[i];
		previous_snapshot_fields[previous_snapshot_count].destination =
			previous_text[i];
		previous_snapshot_fields[previous_snapshot_count].destination_size =
			sizeof(previous_text[i]);
		++previous_snapshot_count;
	}
	snapshot_status = rc_wan_nvram_copy_snapshot(
		previous_snapshot_fields, (unsigned int)previous_snapshot_count,
		wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &previous_transaction);
	if (snapshot_status != 0) {
		logmessage(log_prefix,
			   "%s (%s) cannot snapshot previous ZeroConf IPTV NVRAM tuple (status=%d)",
			   udhcpc_state, man_ifname, snapshot_status);
		return snapshot_status;
	}
	if (!dhcp_lease_text_fits_buffer(previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_IPADDR], sizeof(old_ip)) ||
	    !dhcp_lease_text_fits_buffer(previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_NETMASK], sizeof(old_mask))) {
		logmessage(LOGNAME, "ZeroConf rejected oversized previous IPTV address snapshot on %s", man_ifname);
		return ENAMETOOLONG;
	}
	snprintf(old_ip, sizeof(old_ip), "%s",
		 previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_IPADDR]);
	snprintf(old_mask, sizeof(old_mask), "%s",
		 previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_NETMASK]);

	interface_context.ifname = man_ifname;
	interface_status = dhcp_wan_interface_apply(old_ip, old_mask, ip, zeroconf_mask, 1,
		&wan_dhcp_interface_ops, &interface_context, &rollback_status);
	if (interface_status != 0) {
		logmessage(LOGNAME, "failed to apply ZeroConf IPTV address on %s (error=%d)", man_ifname, interface_status);
		if (rollback_status != 0)
			logmessage(LOGNAME, "failed to restore previous ZeroConf IPTV address on %s (error=%d)", man_ifname, rollback_status);
		return interface_status;
	}

	snapshot_count = 0;
	snapshot_rollback_status = 0;
	snapshot_status = wan_dhcp_bound_snapshot_add(
		snapshot_values, &snapshot_count, wan_zcip_viptv_snapshot_params[WAN_ZCIP_VIPTV_SNAPSHOT_NETMASK],
		zeroconf_mask, previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_NETMASK]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, wan_zcip_viptv_snapshot_params[WAN_ZCIP_VIPTV_SNAPSHOT_GATEWAY],
			"", previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_GATEWAY]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, wan_zcip_viptv_snapshot_params[WAN_ZCIP_VIPTV_SNAPSHOT_IPADDR],
			ip, previous_text[WAN_ZCIP_VIPTV_SNAPSHOT_IPADDR]);
	if (snapshot_status == 0)
		snapshot_status = wan_dhcp_nvram_state_publish(
			snapshot_values, snapshot_count, &snapshot_rollback_status);
	if (snapshot_status != 0) {
		logmessage(LOGNAME,
			   "cannot publish complete ZeroConf IPTV snapshot on %s (status=%d)",
			   man_ifname, snapshot_status);
		interface_rollback_status = dhcp_wan_interface_restore_previous(
			old_ip, old_mask, &wan_dhcp_interface_ops, &interface_context);
		if (interface_rollback_status != 0)
			logmessage(LOGNAME,
				   "failed to restore previous ZeroConf IPTV address on %s (error=%d)",
				   man_ifname, interface_rollback_status);
		if (snapshot_rollback_status != 0)
			logmessage(LOGNAME,
				   "ZeroConf IPTV NVRAM snapshot restoration incomplete on %s (status=%d)",
				   man_ifname, snapshot_rollback_status);
		return snapshot_status;
	}
	return start_igmpproxy(man_ifname);
}

static int
wan_dhcp_nvram_transaction_set(void *opaque, const char *name,
			       const char *value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)opaque;
	int result;

	if (!transaction || !name || !value)
		return EINVAL;
	errno = 0;
	result = nvram_transaction_set(transaction, name, (char *)value, 1);
	if (result < 0)
		return errno ? errno : EIO;
	return result;
}

static int
wan_dhcp_nvram_restore_values(
	const struct dhcp_lease_nvram_value *values, size_t count,
	int *rollback_status)
{
	struct nvram_transaction transaction;
	int status, end_status, restore_attempted = 0, restore_detail = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!values || count == 0 || count > WAN_DHCP_BOUND_SNAPSHOT_COUNT)
		return EINVAL;
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0) {
		status = errno ? errno : EIO;
		if (rollback_status)
			*rollback_status = status;
		return status;
	}
	status = dhcp_lease_nvram_restore_if_current(
		values, count, wan_dhcp_nvram_transaction_get_value,
		wan_dhcp_nvram_transaction_set, &transaction,
		&restore_attempted, &restore_detail);
	if (status != 0 && restore_detail == 0)
		restore_detail = status;
	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		if (restore_detail == 0)
			restore_detail = end_status;
	}
	if (rollback_status)
		*rollback_status = restore_detail;
	return status;
}



static int
wan_dhcp_nvram_state_publish(
	const struct dhcp_lease_nvram_value *values, size_t count,
	int *rollback_status)
{
	struct nvram_transaction transaction;
	int status, end_status, recovery_status, recovery_detail = 0;
	int publish_attempted = 0, publish_rollback_status = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!values || count == 0 || count > WAN_DHCP_BOUND_SNAPSHOT_COUNT)
		return EINVAL;
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;
	status = dhcp_lease_nvram_publish_if_current(
		values, count, wan_dhcp_nvram_transaction_get_value,
		wan_dhcp_nvram_transaction_set, &transaction,
		&publish_attempted, &publish_rollback_status);
	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		if (publish_attempted && publish_rollback_status == 0)
			publish_rollback_status = end_status;
	}
	if (publish_attempted && publish_rollback_status != 0) {
		/* Retry only failed compensation under a guarded fresh transaction. */
		recovery_status = wan_dhcp_nvram_restore_values(
			values, count, &recovery_detail);
		if (rollback_status)
			*rollback_status = recovery_status != 0 ?
				(recovery_detail != 0 ? recovery_detail :
				 recovery_status) : 0;
	} else if (rollback_status) {
		*rollback_status = publish_rollback_status;
	}
	return status;
}



static int
wan_dhcp_bound_snapshot_add(struct dhcp_lease_nvram_value *values,
			    size_t *count, const char *name,
			    const char *value, const char *previous_value)
{
	if (!values || !count || *count >= WAN_DHCP_BOUND_SNAPSHOT_COUNT ||
	    !name || !name[0] || !value || !previous_value)
		return EINVAL;

	values[*count].name = name;
	values[*count].value = value;
	values[*count].previous_value = previous_value;
	++*count;
	return 0;
}


static int
wan_dhcp_nvram_publish_values(const char *const *names,
			      const char *const *new_values, size_t count)
{
	struct nvram_transaction transaction;
	struct dhcp_lease_nvram_value snapshot_values[WAN_DHCP_BOUND_SNAPSHOT_COUNT];
	char previous_values[WAN_DHCP_BOUND_SNAPSHOT_COUNT][WAN_DHCP_BOUND_SNAPSHOT_VALUE_MAX];
	char *previous_value;
	size_t i, snapshot_count = 0;
	int status = 0, read_status, end_status;
	int rollback_status = 0, recovery_status, recovery_detail = 0;
	int publish_attempted = 0;

	if (!names || !new_values || count == 0 ||
	    count > WAN_DHCP_BOUND_SNAPSHOT_COUNT)
		return EINVAL;
	for (i = 0; i < count; ++i) {
		if (!names[i] || !names[i][0] || !new_values[i])
			return EINVAL;
	}

	/*
	 * Read the prior tuple and publish/compensate it under the same exclusive
	 * lease. Separate nvram_safe_get() calls could snapshot different writers'
	 * states, then restore stale values over a newer committed lease.
	 */
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;

	for (i = 0; i < count; ++i) {
		previous_value = NULL;
		errno = 0;
		read_status = nvram_transaction_get(
			&transaction, names[i], &previous_value);
		if (read_status < 0) {
			status = errno ? errno : EIO;
			break;
		}
		if (read_status == 0 || !previous_value)
			previous_value = "";
		if (!dhcp_lease_text_fits_buffer(
			    previous_value, sizeof(previous_values[i]))) {
			logmessage(LOGNAME,
				   "IPTV deconfig rejected oversized prior NVRAM field %s",
				   names[i]);
			status = ENAMETOOLONG;
			break;
		}
		snprintf(previous_values[i], sizeof(previous_values[i]), "%s",
			 previous_value);
		status = wan_dhcp_bound_snapshot_add(
			snapshot_values, &snapshot_count, names[i], new_values[i],
			previous_values[i]);
		if (status != 0)
			break;
	}

	if (status == 0) {
		publish_attempted = 1;
		status = dhcp_lease_nvram_publish(
			snapshot_values, snapshot_count,
			wan_dhcp_nvram_transaction_set, &transaction,
			&rollback_status);
	}

	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		else if (rollback_status == 0)
			rollback_status = end_status;
	}

	if (publish_attempted && status != 0) {
		/* Retry the entire prior tuple under a fresh exclusive lease. */
		recovery_status = wan_dhcp_nvram_restore_values(
			snapshot_values, snapshot_count, &recovery_detail);
		rollback_status = recovery_status != 0 ?
			(recovery_detail != 0 ? recovery_detail : recovery_status) : 0;
	}
	if (status != 0)
		logmessage(LOGNAME,
			   "IPTV NVRAM snapshot publication failed (status=%d, rollback=%d)",
			   status, rollback_status);
	return status;
}



static int
udhcpc_renew(char *wan_ifname)
{
	char *value, *lease_dns, *lease_wins, *lease_domain;
	char prefix[16], log_prefix[32];
	char key_dns[32], key_domain[32], key_wins[32], key_lease[32];
	char lease_text[16], gateway_buf[INET_ADDRSTRLEN];
	char dltime_name[32], dltime_text[16];
	const char *dltime_names[1], *dltime_values[1];
	unsigned int parsed_value;
	size_t renew_count = 0;
	int unit, is_man, resolver_changed = 0;
	int nvram_status, rollback_status = 0;
	int lease_error = 0, route_error = 0, mtu_error = 0, retry_required = 0;
	int lease_error_read_status, route_error_read_status, mtu_error_read_status;
	int stored_mtu = DHCP_LEASE_DEFAULT_MTU, dhcp_mtu, mtu_supported, mtu_changed;
	int dltime_length;
	struct dhcp_lease_nvram_value renew_values[4];
	struct wan_dhcp_renew_nvram_state previous_state;

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);
	if (is_man) {
		strcpy(prefix, "wanx_");
	} else {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		/* Capture time but defer publication until renewal validation succeeds. */
		dltime_length = snprintf(dltime_text, sizeof(dltime_text),
					 "%ld", uptime());
		if (dltime_length < 0)
			return errno ? errno : EIO;
		if ((size_t)dltime_length >= sizeof(dltime_text))
			return EOVERFLOW;
	}
	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");

	nvram_status = wan_dhcp_renew_state_snapshot(prefix, 1, &previous_state);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot previous DHCP lease state on %s (status=%d)",
			   wan_ifname, nvram_status);
		return nvram_status;
	}
	lease_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.lease_error, &lease_error);
	route_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.route_error, &route_error);
	mtu_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.mtu_error, &mtu_error);
	nvram_status = dhcp_wan_bound_retry_decide(
		lease_error_read_status, route_error_read_status,
		mtu_error_read_status, lease_error, route_error, mtu_error,
		&retry_required);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "cannot verify previous DHCP retry state on %s (status=%d)",
			   wan_ifname, nvram_status);
		return nvram_status;
	}
	if (retry_required) {
		logmessage(log_prefix,
			   "retrying DHCP lease/address/route/MTU application on %s (lease_error=%d, route_error=%d, mtu_error=%d)",
			   wan_ifname, lease_error, route_error, mtu_error);
		return udhcpc_bound(wan_ifname, 1);
	}

	value = getenv("subnet");
	if (dhcp_lease_required_option_is_missing_or_changed(
		previous_state.netmask, value ? trim_r(value) : NULL))
		return udhcpc_bound(wan_ifname, 1);
	value = getenv("router");
	if (dhcp_lease_router_is_changed(
		previous_state.gateway, value ? trim_r(value) : NULL,
		gateway_buf, sizeof(gateway_buf)))
		return udhcpc_bound(wan_ifname, 1);
	value = getenv("ip");
	if (dhcp_lease_required_option_is_missing_or_changed(
		previous_state.ipaddr, value ? trim_r(value) : NULL))
		return udhcpc_bound(wan_ifname, 1);

	/* Changed route options must re-run WAN setup after removing old routes. */
	value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes, value) != 0)
		return udhcpc_bound(wan_ifname, 1);
	value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes_ms, value) != 0)
		return udhcpc_bound(wan_ifname, 1);
	value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes_rfc, value) != 0)
		return udhcpc_bound(wan_ifname, 1);
	value = getenv("mtu");
	if (value)
		value = trim_r(value);
	nvram_status = dhcp_lease_mtu_option_value(
		value, &dhcp_mtu, &mtu_supported);
	if (nvram_status != 0) {
		logmessage(log_prefix,
			   "cannot determine current DHCP MTU on %s (status=%d)",
			   wan_ifname, nvram_status);
		return nvram_status;
	}
	if (value && !mtu_supported)
		logmessage(log_prefix, "ignoring invalid DHCP MTU on %s", wan_ifname);
	stored_mtu = DHCP_LEASE_DEFAULT_MTU;
	if (previous_state.mtu[0]) {
		nvram_status = rc_wan_nvram_parse_int_value(
			previous_state.mtu, &stored_mtu);
		if (nvram_status != 0) {
			logmessage(log_prefix,
				   "cannot parse stored DHCP MTU on %s (status=%d)",
				   wan_ifname, nvram_status);
			return nvram_status;
		}
	}
	nvram_status = dhcp_lease_mtu_should_reapply(
		stored_mtu, previous_state.mtu[0] != '\0',
		dhcp_mtu, &mtu_changed);
	if (nvram_status != 0)
		return nvram_status;
	if (mtu_changed)
		return udhcpc_bound(wan_ifname, 1);

	snprintf(key_dns, sizeof(key_dns), "%sdns", prefix);
	snprintf(key_domain, sizeof(key_domain), "%sdomain", prefix);
	snprintf(key_wins, sizeof(key_wins), "%swins", prefix);
	snprintf(key_lease, sizeof(key_lease), "%slease", prefix);

	value = getenv("dns");
	lease_dns = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_dns, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP DNS list on %s", wan_ifname);
		lease_dns = "";
	}
	if (strcmp(previous_state.dns, lease_dns) != 0) {
		renew_values[renew_count].name = key_dns;
		renew_values[renew_count].value = lease_dns;
		renew_values[renew_count].previous_value = previous_state.dns;
		++renew_count;
		resolver_changed = 1;
	}

	value = getenv("domain");
	lease_domain = value ? trim_r(value) : "";
	if (*lease_domain && !dnsmasq_valid_server_domain(lease_domain)) {
		logmessage(log_prefix, "ignoring invalid DHCP domain on %s", wan_ifname);
		lease_domain = "";
	}
	if (strcmp(previous_state.domain, lease_domain) != 0) {
		renew_values[renew_count].name = key_domain;
		renew_values[renew_count].value = lease_domain;
		renew_values[renew_count].previous_value = previous_state.domain;
		++renew_count;
		resolver_changed = 1;
	}

	value = getenv("wins");
	lease_wins = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_wins, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP WINS list on %s", wan_ifname);
		lease_wins = "";
	}
	if (strcmp(previous_state.wins, lease_wins) != 0) {
		renew_values[renew_count].name = key_wins;
		renew_values[renew_count].value = lease_wins;
		renew_values[renew_count].previous_value = previous_state.wins;
		++renew_count;
	}

	snprintf(lease_text, sizeof(lease_text), "%s", previous_state.lease);
	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value)) {
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", wan_ifname);
		} else {
			snprintf(lease_text, sizeof(lease_text), "%u", parsed_value);
		}
	}
	if (strcmp(previous_state.lease, lease_text) != 0) {
		renew_values[renew_count].name = key_lease;
		renew_values[renew_count].value = lease_text;
		renew_values[renew_count].previous_value = previous_state.lease;
		++renew_count;
	}

	if (renew_count != 0) {
		nvram_status = wan_dhcp_nvram_state_publish(
			renew_values, renew_count, &rollback_status);
		if (nvram_status != 0) {
			logmessage(log_prefix,
				   "cannot publish complete DHCP renewal option snapshot on %s (status=%d)",
				   wan_ifname, nvram_status);
			if (rollback_status != 0)
				logmessage(log_prefix,
					   "DHCP renewal option restoration incomplete on %s (status=%d)",
					   wan_ifname, rollback_status);
			return nvram_status;
		}
	}
	if (resolver_changed) {
		logmessage(log_prefix, "renew (%s), DNS/domain configuration changed", wan_ifname);
		refresh_resolvconf_nonfatal("WAN DHCP renewal", 0);
	}

	if (!is_man) {
		dltime_length = snprintf(dltime_name, sizeof(dltime_name),
					 "%sdltime", prefix);
		if (dltime_length < 0)
			return errno ? errno : EIO;
		if ((size_t)dltime_length >= sizeof(dltime_name))
			return ENAMETOOLONG;

		dltime_names[0] = dltime_name;
		dltime_values[0] = dltime_text;
		nvram_status = wan_dhcp_nvram_publish_values(
			dltime_names, dltime_values, 1);
		if (nvram_status != 0) {
			logmessage(log_prefix,
				   "cannot publish successful DHCP renewal timestamp on %s (status=%d)",
				   wan_ifname, nvram_status);
			return nvram_status;
		}
	}
	return 0;
}


static int
udhcpc_viptv_renew(char *man_ifname)
{
	char *value;
	char prefix[16], gateway_buf[INET_ADDRSTRLEN];
	unsigned int parsed_value;
	int nvram_status, stored_mtu = DHCP_LEASE_DEFAULT_MTU;
	int mtu_supported, mtu_changed, dhcp_mtu = DHCP_LEASE_DEFAULT_MTU;
	int lease_error = 0, route_error = 0, mtu_error = 0, retry_required = 0;
	int lease_error_read_status, route_error_read_status, mtu_error_read_status;
	struct wan_dhcp_renew_nvram_state previous_state;

	strcpy(prefix, "manv_");
	nvram_status = wan_dhcp_renew_state_snapshot(prefix, 0, &previous_state);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot previous IPTV DHCP lease state on %s (status=%d)",
			   man_ifname, nvram_status);
		return nvram_status;
	}

	lease_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.lease_error, &lease_error);
	route_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.route_error, &route_error);
	mtu_error_read_status = rc_wan_nvram_parse_int_value(
		previous_state.mtu_error, &mtu_error);
	nvram_status = dhcp_wan_bound_retry_decide(
		lease_error_read_status, route_error_read_status,
		mtu_error_read_status, lease_error, route_error, mtu_error,
		&retry_required);
	if (nvram_status != 0) {
		logmessage(LOGNAME,
			   "cannot verify previous DHCP retry state on %s (status=%d)",
			   man_ifname, nvram_status);
		return nvram_status;
	}
	if (retry_required) {
		logmessage(LOGNAME,
			   "retrying IPTV lease/address/route/MTU application on %s (lease_error=%d, route_error=%d, mtu_error=%d)",
			   man_ifname, lease_error, route_error, mtu_error);
		return udhcpc_viptv_bound(man_ifname, 1);
	}

	value = getenv("subnet");
	if (dhcp_lease_required_option_is_missing_or_changed(
		previous_state.netmask, value ? trim_r(value) : NULL))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("router");
	if (dhcp_lease_router_is_changed(previous_state.gateway,
					 value ? trim_r(value) : NULL,
					 gateway_buf, sizeof(gateway_buf)))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("ip");
	if (dhcp_lease_required_option_is_missing_or_changed(
		previous_state.ipaddr, value ? trim_r(value) : NULL))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes, value) != 0)
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes_ms, value) != 0)
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (strcmp(previous_state.routes_rfc, value) != 0)
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("mtu");
	if (value)
		value = trim_r(value);
	nvram_status = dhcp_lease_mtu_option_value(
		value, &dhcp_mtu, &mtu_supported);
	if (nvram_status != 0)
		return nvram_status;
	if (value && !mtu_supported)
		logmessage(LOGNAME, "ignoring invalid IPTV DHCP MTU on %s", man_ifname);
	stored_mtu = DHCP_LEASE_DEFAULT_MTU;
	if (previous_state.mtu[0]) {
		nvram_status = rc_wan_nvram_parse_int_value(
			previous_state.mtu, &stored_mtu);
		if (nvram_status != 0) {
			logmessage(LOGNAME,
				   "cannot parse stored IPTV MTU on %s (status=%d)",
				   man_ifname, nvram_status);
			return nvram_status;
		}
	}
	nvram_status = dhcp_lease_mtu_should_reapply(
		stored_mtu, previous_state.mtu[0] != '\0',
		dhcp_mtu, &mtu_changed);
	if (nvram_status != 0)
		return nvram_status;
	if (mtu_changed)
		return udhcpc_viptv_bound(man_ifname, 1);

	return 0;
}


static int
udhcpc_leasefail(char *wan_ifname)
{
	/* DHCP failed for INET */
	return 0;
}

static int
udhcpc_viptv_leasefail(char *man_ifname)
{
	/* DHCP failed for IPTV */
	return 0;
}

static int
udhcpc_noack(char *wan_ifname)
{
	int unit, is_man;
	char log_prefix[32];

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	logmessage(log_prefix, "Received NAK for %s", wan_ifname);
	return 0;
}

static int
udhcpc_viptv_noack(char *man_ifname)
{
	char log_prefix[32];

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	logmessage(log_prefix, "Received NAK for %s", man_ifname);
	return 0;
}

int
udhcpc_main(int argc, char **argv)
{
	int ret = 0;
	char *wan_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(wan_ifname))
		return EINVAL;

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(wan_ifname, 0);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_bound(wan_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_renew(wan_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_leasefail(wan_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_noack(wan_ifname);

	return ret;
}

#define WAN_MAN_DHCP_ROUTE_SNAPSHOT_SIZE 2049

static int
manv_route_deconfig(char *man_ifname)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[4];
	char old_gateway[INET_ADDRSTRLEN];
	char route_storage[3][WAN_MAN_DHCP_ROUTE_SNAPSHOT_SIZE];
	int route_status, route_result;

	if (!man_ifname || !man_ifname[0])
		return EINVAL;

	/*
	 * Snapshot the gateway and all three DHCP route inventories under one
	 * NVRAM lease. Copy cache-backed values before the next read so renewal
	 * cannot make route deletion operate on a mixed old/new tuple.
	 */
	fields[0].param_name = "manv_gateway";
	fields[0].destination = old_gateway;
	fields[0].destination_size = sizeof(old_gateway);
	fields[1].param_name = "manv_routes";
	fields[1].destination = route_storage[0];
	fields[1].destination_size = sizeof(route_storage[0]);
	fields[2].param_name = "manv_routes_rfc";
	fields[2].destination = route_storage[1];
	fields[2].destination_size = sizeof(route_storage[1]);
	fields[3].param_name = "manv_routes_ms";
	fields[3].destination = route_storage[2];
	fields[3].destination_size = sizeof(route_storage[2]);

	route_status = rc_wan_nvram_copy_snapshot(
		fields, 4, wan_nvram_transaction_begin_callback,
		wan_nvram_transaction_get_callback,
		wan_nvram_transaction_end_callback, &transaction);
	if (route_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot IPTV DHCP route inventory on %s (status=%d); preserving lease metadata",
			   man_ifname, route_status);
		return route_status;
	}

	route_result = control_dhcp_routes(
		route_storage[0], route_storage[1], route_storage[2],
		man_ifname, 0, 0);
	route_status = route_status_accumulate(0, route_result);
	if (route_result != 0)
		logmessage(LOGNAME,
			   "failed to delete IPTV DHCP routes on %s (error=%d)",
			   man_ifname, route_result);

	if (dhcp_lease_ipv4_is_valid(old_gateway)) {
		route_result = route_del(
			man_ifname, 10, "0.0.0.0", old_gateway, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_result)) {
			route_status = route_status_accumulate(route_status, route_result);
			logmessage(LOGNAME,
				   "failed to delete IPTV default route via %s (error=%d)",
				   old_gateway, route_result);
		}
	}

	if (route_status != 0)
		logmessage(LOGNAME,
			   "IPTV DHCP route teardown incomplete on %s (error=%d); preserving lease metadata for retry",
			   man_ifname, route_status);
	return route_status;
}


struct udhcpc_viptv_deconfig_context {
	char *ifname;
};

static int
udhcpc_viptv_deconfig_clear_address(void *opaque)
{
	struct udhcpc_viptv_deconfig_context *context =
		(struct udhcpc_viptv_deconfig_context *)opaque;

	return ifconfig(context->ifname, IFUP, "0.0.0.0", NULL);
}

static int
udhcpc_viptv_deconfig_delete_routes(void *opaque)
{
	struct udhcpc_viptv_deconfig_context *context =
		(struct udhcpc_viptv_deconfig_context *)opaque;

	return manv_route_deconfig(context->ifname);
}

static int
manv_viptv_deconfig_publish_empty_metadata(void *opaque)
{
	static const char *const names[] = {
		"manv_ipaddr", "manv_gateway", "manv_routes",
		"manv_routes_ms", "manv_routes_rfc"
	};
	static const char *const values[] = { "0.0.0.0", "", "", "", "" };

	(void)opaque;
	return wan_dhcp_nvram_publish_values(names, values, 5);
}

static int
udhcpc_viptv_deconfig(char *man_ifname)
{
	struct udhcpc_viptv_deconfig_context context;
	static const struct dhcp_lease_viptv_deconfig_ops ops = {
		udhcpc_viptv_deconfig_clear_address,
		udhcpc_viptv_deconfig_delete_routes,
		manv_viptv_deconfig_publish_empty_metadata
	};
	int status, multicast_status;

	context.ifname = man_ifname;
	status = dhcp_lease_viptv_deconfig_apply(&ops, &context);
	if (status != 0)
		logmessage(LOGNAME,
			   "IPTV DHCP deconfig incomplete on %s (status=%d); retaining lease metadata for retry",
			   man_ifname, status);

	/* Always stop multicast service, even when address or route cleanup failed. */
	multicast_status = stop_igmpproxy(man_ifname);
	if (multicast_status != 0)
		logmessage(LOGNAME,
			   "IPTV DHCP multicast teardown incomplete on %s (status=%d)",
			   man_ifname, multicast_status);
	return route_status_accumulate(status, multicast_status);
}

int
udhcpc_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;
	umask(0000);
	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_viptv_deconfig(man_ifname);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_viptv_bound(man_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_viptv_renew(man_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_viptv_leasefail(man_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_viptv_noack(man_ifname);
	return ret;
}

int
zcip_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(man_ifname, 1);
	else if (!strcmp(argv[1], "config"))
		ret = zcip_bound(man_ifname);

	return ret;
}

static int
zcip_viptv_deconfig(char *man_ifname)
{
	int interface_status, route_status, metadata_status = 0, multicast_status;

	interface_status = ifconfig(man_ifname, IFUP, "0.0.0.0", NULL);
	if (interface_status != 0)
		logmessage(LOGNAME, "failed to clear ZeroConf IPTV address on %s (error=%d)", man_ifname, interface_status);
	route_status = manv_route_deconfig(man_ifname);
	if (interface_status == 0 && route_status == 0) {
		metadata_status = manv_viptv_deconfig_publish_empty_metadata(NULL);
		if (metadata_status != 0)
			logmessage(LOGNAME,
				   "ZeroConf IPTV lease metadata clear failed on %s (status=%d)",
				   man_ifname, metadata_status);
	} else {
		logmessage(LOGNAME, "ZeroConf IPTV deconfig incomplete on %s (interface=%d route=%d); preserving lease identity", man_ifname, interface_status, route_status);
	}
	multicast_status = stop_igmpproxy(man_ifname);
	if (multicast_status != 0)
		logmessage(LOGNAME, "ZeroConf IPTV multicast teardown incomplete on %s (error=%d)",
			   man_ifname, multicast_status);
	return route_status_accumulate(
		route_status_accumulate(
			route_status_accumulate(interface_status, route_status),
			metadata_status),
		multicast_status);
}

int
zcip_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;
	umask(0000);
	if (!strcmp(argv[1], "deconfig"))
		ret = zcip_viptv_deconfig(man_ifname);
	else if (!strcmp(argv[1], "config"))
		ret = zcip_viptv_bound(man_ifname);
	return ret;
}

int start_udhcpc_wan(char *wan_ifname, int unit, int wait_lease)
{
	int dr_enable_value = 0;
	int dr_enable_status;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status;
#endif

	int index, is_man, option_status;
	char log_prefix[32], pidfile[32] = {0};
	char wan_hostname[256], wan_vci[256];
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", wan_ifname,
		"-s", SCRIPT_UDHCPC_WAN,
		"-p", pidfile,
		"-t4",
		"-T4",
		NULL,
		NULL, NULL,	/* -x hostname:hostname */
		NULL, NULL,	/* -V vendorclass	*/
		NULL,		/* -O mtu		*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
#if defined (USE_IPV6)
		NULL,		/* -O 6rd		*/
		NULL,		/* -O comcast6rd	*/
#endif
		NULL
	};

	index = 9;		/* first NULL index	*/

	snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);

	if (is_man_wisp(wan_ifname) || is_usbnet_interface(wan_ifname))
		dhcp_argv[7] = "-t6";

	if (wait_lease)
		dhcp_argv[index++] = "-b";	/* Background if lease is not obtained (timeout 4*4 sec) */
	else
		dhcp_argv[index++] = "-d";	/* Background after run (new patch for udhcpc) */

	/*
	 * get_wan_unit_value() returns a pointer into the shared NVRAM cache.
	 * Reading the vendor class can overwrite the hostname before udhcpc
	 * consumes argv. Keep both option strings in checked owned buffers.
	 */
	option_status = get_wan_unit_value_copy_status(
		unit, "hostname", wan_hostname, sizeof(wan_hostname));
	if (option_status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN DHCP hostname for unit %d (status=%d)",
			   unit, option_status);
		errno = option_status;
		return option_status;
	}
	option_status = get_wan_unit_value_copy_status(
		unit, "vci", wan_vci, sizeof(wan_vci));
	if (option_status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN DHCP vendor class for unit %d (status=%d)",
			   unit, option_status);
		errno = option_status;
		return option_status;
	}

	if (wan_hostname[0]) {
		dhcp_argv[index++] = "-H";
		dhcp_argv[index++] = sanity_hostname(wan_hostname);
	}

	if (wan_vci[0]) {
		dhcp_argv[index++] = "-V";
		dhcp_argv[index++] = wan_vci;
	}

	dhcp_argv[index++] = "-O26";	/* "mtu" */

	dr_enable_status = get_nvram_match_checked_status(
		"dr_enable_x", "1", &dr_enable_value);
	if (dr_enable_status != 0) {
		logmessage(LOGNAME,
			   "cannot read dynamic-route enable flag before DHCP option setup (status=%d)",
			   dr_enable_status);
		errno = dr_enable_status;
		return dr_enable_status;
	}
	if (dr_enable_value) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "cannot determine 6rd DHCP option policy; refusing WAN DHCP start (status=%d)",
			   ipv6_type_status);
		return ipv6_type_status;
	}
	if (ipv6_type == IPV6_6RD) {
		dhcp_argv[index++] = "-O212";	/* "6rd" */
		dhcp_argv[index++] = "-O150";	/* "comcast6rd" */
	}
#endif

	is_man = is_ifunit_man(wan_ifname, unit);
	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	return udhcpc_start_and_verify(dhcp_argv, pidfile, wan_ifname, log_prefix);
}

int start_udhcpc_viptv(char *man_ifname)
{
	int dr_enable_value = 0;
	int dr_enable_status;

	int index;
	char log_prefix[32], pidfile[32] = {0};
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", man_ifname,
		"-s", SCRIPT_UDHCPC_VIPTV,
		"-p", pidfile,
		"-t4",
		"-T4",
		"-d",		/* Background after run (new patch for udhcpc) */
		NULL,		/* -O mtu		*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
		NULL
	};

	index = 10;		/* first NULL index	*/

	snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc_viptv.pid");

	dhcp_argv[index++] = "-O26";	/* "mtu" */

	dr_enable_status = get_nvram_match_checked_status(
		"dr_enable_x", "1", &dr_enable_value);
	if (dr_enable_status != 0) {
		logmessage(LOGNAME,
			   "cannot read dynamic-route enable flag before DHCP option setup (status=%d)",
			   dr_enable_status);
		errno = dr_enable_status;
		return dr_enable_status;
	}
	if (dr_enable_value) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
	}

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	return udhcpc_start_and_verify(dhcp_argv,
				       "/var/run/udhcpc_viptv.pid",
				       man_ifname, log_prefix);
}

int start_zcip_wan(char *wan_ifname)
{
	return eval("/sbin/zcip", wan_ifname, SCRIPT_ZCIP_WAN);
}

int start_zcip_viptv(char *man_ifname)
{
	char log_prefix[32];

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "ZeroConf", "IPTV");
	logmessage(log_prefix, "starting on %s ...", man_ifname);

	return eval("/sbin/zcip", "-q", man_ifname, SCRIPT_ZCIP_VIPTV);
}

static int
stop_udhcpc_pidfile(const char *path)
{
	const char *pidfiles[2];

	pidfiles[0] = path;
	pidfiles[1] = NULL;
	/* Let BusyBox complete its pending-deconfig retry; never SIGKILL it. */
	return kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0);
}

int renew_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return process_pidfile_signal(pidfile, "udhcpc", SIGUSR1);
}

int release_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return process_pidfile_signal(pidfile, "udhcpc", SIGUSR2);
}

int stop_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return stop_udhcpc_pidfile(pidfile);
}

int stop_udhcpc_viptv(void)
{
	return stop_udhcpc_pidfile("/var/run/udhcpc_viptv.pid");
}
