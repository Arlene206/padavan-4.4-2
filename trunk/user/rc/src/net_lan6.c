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
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include "rc.h"
#include "lan_addr6_reload_policy.h"
#include "wan_nvram_status_policy.h"

#if defined (USE_IPV6)

static int
get_lan6_service_type(int *ipv6_type)
{
	int status = get_ipv6_type_status(ipv6_type);

	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot read IPv6 service configuration (status=%d)",
			   status);
	}

	return status;
}

#define LAN6_RESET_ADDRESS_SIZE (INET6_ADDRSTRLEN + 5)
#define LAN6_RESET_INTEGER_SIZE 32
#define LAN6_RESET_AUTO_SIZE 16

struct lan6_reset_snapshot {
	int is_static;
	int prefix_size;
	char address[LAN6_RESET_ADDRESS_SIZE];
};

static int
lan6_snapshot_transaction_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
lan6_snapshot_transaction_get(void *context, const char *param_name,
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
lan6_snapshot_transaction_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

static int
lan6_read_config_value_status(const char *param_name, char *value,
			      size_t value_size)
{
	struct rc_wan_nvram_snapshot_field field;
	struct nvram_transaction transaction;
	int status;

	if (!param_name || !param_name[0] || !value || value_size == 0)
		return EINVAL;
	field.param_name = param_name;
	field.destination = value;
	field.destination_size = value_size;
	status = rc_wan_nvram_copy_snapshot(
		&field, 1, lan6_snapshot_transaction_begin,
		lan6_snapshot_transaction_get, lan6_snapshot_transaction_end,
		&transaction);
	return status;
}


/*
 * Resolve static/dynamic mode and its address/prefix size from one NVRAM
 * generation. A failed read or transaction close aborts before reset writes
 * lan_addr6, preventing a transient backend error from erasing valid state.
 */
static int
lan6_read_reset_snapshot(int ipv6_type,
			 struct lan6_reset_snapshot *snapshot)
{
	static const char *const static_names[2] = {
		"ip6_lan_addr", "ip6_lan_size"
	};
	static const char *const dynamic_names[3] = {
		"ip6_lan_auto", "ip6_lan_addr", "ip6_lan_size"
	};
	struct rc_wan_nvram_snapshot_field fields[3];
	struct nvram_transaction transaction;
	char auto_value[LAN6_RESET_AUTO_SIZE] = {0};
	char address[LAN6_RESET_ADDRESS_SIZE] = {0};
	char size_text[LAN6_RESET_INTEGER_SIZE] = {0};
	const char *const *names;
	char *destinations[3];
	size_t destination_sizes[3];
	unsigned int count, i;
	int disabled, excluded, forced_static, status;

	if (!snapshot)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));

	disabled = (ipv6_type == IPV6_DISABLED);
	excluded = (ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD);
	forced_static = (ipv6_type == IPV6_NATIVE_STATIC ||
			 ipv6_type == IPV6_6IN4);

	if (disabled || excluded) {
		return rc_lan_addr6_static_mode_status(
			disabled, excluded, forced_static, 0, "",
			&snapshot->is_static);
	}

	if (forced_static) {
		names = static_names;
		destinations[0] = address;
		destinations[1] = size_text;
		destination_sizes[0] = sizeof(address);
		destination_sizes[1] = sizeof(size_text);
		count = 2;
	} else {
		names = dynamic_names;
		destinations[0] = auto_value;
		destinations[1] = address;
		destinations[2] = size_text;
		destination_sizes[0] = sizeof(auto_value);
		destination_sizes[1] = sizeof(address);
		destination_sizes[2] = sizeof(size_text);
		count = 3;
	}

	for (i = 0; i < count; ++i) {
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = destination_sizes[i];
	}
	status = rc_wan_nvram_copy_snapshot(
		fields, count, lan6_snapshot_transaction_begin,
		lan6_snapshot_transaction_get, lan6_snapshot_transaction_end,
		&transaction);
	if (status != 0)
		return status;

	status = rc_lan_addr6_static_mode_status(
		disabled, excluded, forced_static, 0, auto_value,
		&snapshot->is_static);
	if (status != 0)
		return status;
	if (!snapshot->is_static)
		return 0;

	status = rc_lan_addr6_legacy_prefix_size(0, size_text,
					       &snapshot->prefix_size);
	if (status != 0)
		return status;
	return rc_wan_nvram_copy_text(address, snapshot->address,
				      sizeof(snapshot->address));
}


static int
lan_addr6_static_for_type(int ipv6_type)
{
	struct lan6_reset_snapshot snapshot;
	int status;

	status = lan6_read_reset_snapshot(ipv6_type, &snapshot);
	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot read LAN address mode snapshot (status=%d)",
			   status);
		return -1;
	}
	return snapshot.is_static;
}

int reset_lan6_vars(void)
{
	int api_result, saved_errno, status;
	int ipv6_type, service_status;
	struct in6_addr addr6;
	struct lan6_reset_snapshot snapshot;
	char addr6_text[INET6_ADDRSTRLEN];
	char addr6s[INET6_ADDRSTRLEN] = {0};

	service_status = get_lan6_service_type(&ipv6_type);
	if (service_status != 0)
		return service_status;

	status = lan6_read_reset_snapshot(ipv6_type, &snapshot);
	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot read complete LAN address configuration; preserving current state (status=%d)",
			   status);
		return status;
	}

	if (snapshot.is_static && snapshot.address[0]) {
		memset(&addr6, 0, sizeof(addr6));
		if (ipv6_from_string(snapshot.address, &addr6) >= 0 &&
		    inet_ntop(AF_INET6, &addr6, addr6_text, sizeof(addr6_text))) {
			if (rc_lan_addr6_format_prefix(
				addr6_text, snapshot.prefix_size,
				addr6s, sizeof(addr6s)) != 0)
				logmessage("LAN IPv6",
					   "cannot format configured LAN IPv6 prefix (errno=%d)",
					   errno ? errno : EIO);
		}
	}

	errno = 0;
	api_result = nvram_set_temp("lan_addr6", addr6s);
	saved_errno = errno;
	status = rc_lan_addr6_nvram_write_status(api_result, saved_errno);
	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot publish LAN address state to NVRAM (status=%d)",
			   status);
	}
	return status;
}

int is_lan_radv_on(void)
{
	char radv_value[16] = {0};
	int ipv6_type, status, enabled;

	status = get_lan6_service_type(&ipv6_type);
	if (status != 0 || ipv6_type == IPV6_DISABLED)
		return -1;

	status = lan6_read_config_value_status(
		"ip6_lan_radv", radv_value, sizeof(radv_value));
	if (status == 0)
		status = rc_lan_addr6_radv_enabled_status(
			0, radv_value, &enabled);
	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot read router advertisement configuration (status=%d)",
			   status);
		return -1;
	}
	return enabled;
}

int is_lan_addr6_static(void)
{
	int ipv6_type, status;

	status = get_lan6_service_type(&ipv6_type);
	if (status != 0)
		return -1;

	return lan_addr6_static_for_type(ipv6_type);
}

int get_lan_dhcp6s_mode(void)
{
	char mode_value[LAN6_RESET_INTEGER_SIZE] = {0};
	int ipv6_type, mode, status;

	status = get_lan6_service_type(&ipv6_type);
	if (status != 0 || ipv6_type == IPV6_DISABLED)
		return -1;

	status = lan6_read_config_value_status(
		"ip6_lan_dhcp", mode_value, sizeof(mode_value));
	if (status == 0)
		status = rc_lan_addr6_legacy_integer_status(
			0, mode_value, &mode);
	if (status != 0) {
		errno = status;
		logmessage("LAN IPv6",
			   "cannot read DHCPv6 server mode (status=%d)",
			   status);
		return -1;
	}
	return mode;
}

int get_lan_dhcp6s_irt(void)
{
	int irt = 600;			// 10 min (IRT_MINIMUM=600)

	if (is_lan_addr6_static() == 1)
		irt = 1800;		// 30 min

	return irt;
}

int get_lan_dhcp6s_prefix_size(void)
{
	int lan_size6 = 64;

#if 0
	/* dnsmasq support only prefix len 64 for construct case */
	if (is_lan_addr6_static() == 1)
		lan_size6 = nvram_safe_get_int("ip6_lan_size", 64, 48, 64);
#endif

	return lan_size6;
}

int store_lan_addr6(char *lan_addr6_new)
{
	char *lan_addr6_old;
	int api_result, saved_errno, status;

	if (!lan_addr6_new || !lan_addr6_new[0])
		return 0;

	lan_addr6_old = nvram_safe_get("lan_addr6");
	if (strcmp(lan_addr6_new, lan_addr6_old ? lan_addr6_old : "") != 0) {
		errno = 0;
		api_result = nvram_set_temp("lan_addr6", lan_addr6_new);
		saved_errno = errno;
		status = rc_lan_addr6_nvram_write_status(api_result, saved_errno);
		if (status != 0) {
			errno = status;
			logmessage("LAN IPv6",
				   "cannot publish updated LAN address to NVRAM (status=%d)",
				   status);
			return -1;
		}
		return 1;
	}

	return 0;
}

static int
clear_lan_addr6_for_reload(void)
{
	return clear_if_addr6(IFNAME_BR);
}

static int
add_lan_addr6_for_reload(void *context)
{
	char *lan_addr6 = (char *)context;

	if (!lan_addr6 || !lan_addr6[0]) {
		errno = EINVAL;
		return EINVAL;
	}

	/* Pass the NVRAM address as one argv element, never through a shell. */
	return eval("ip", "-6", "addr", "add", lan_addr6, "dev", IFNAME_BR);
}

int reload_lan_addr6(void)
{
	char *lan_addr6 = nvram_safe_get("lan_addr6");

	return rc_lan_addr6_reload_apply(lan_addr6 && lan_addr6[0],
					 clear_lan_addr6_for_reload,
					 add_lan_addr6_for_reload, lan_addr6);
}

int clear_lan_addr6(void)
{
	return clear_if_addr6(IFNAME_BR);
}

char *get_lan_addr6_host(char *p_addr6s)
{
	char *tmp = p_addr6s;
	char *lan_addr6 = nvram_safe_get("lan_addr6");

	if (*lan_addr6) {
		snprintf(p_addr6s, INET6_ADDRSTRLEN, "%s", lan_addr6);
		strsep(&tmp, "/");
		return p_addr6s;
	}

	return NULL;
}

char *get_lan_addr6_prefix(char *p_addr6s)
{
	/* force prefix len to 64 for SLAAC */
	struct in6_addr addr6;
	char addr6_text[INET6_ADDRSTRLEN];
	char *lan_addr6 = nvram_safe_get("lan_addr6");

	if (p_addr6s && *lan_addr6 &&
	    ipv6_from_string(lan_addr6, &addr6) >= 0) {
		ipv6_to_net(&addr6, 64);
		if (inet_ntop(AF_INET6, &addr6, addr6_text, sizeof(addr6_text)) != NULL &&
		    rc_lan_addr6_format_prefix(addr6_text, 64, p_addr6s,
					       INET6_ADDRSTRLEN) == 0)
			return p_addr6s;
	}

	return NULL;
}

#endif
