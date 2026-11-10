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
#include "../../shared/src/dhcp6c_duid_file_policy.h"
#include "dhcp6c_config_file.h"
#include "dhcp6c_callback_generation_policy.h"

#if defined (USE_IPV6)

#define DHCP6C_CALLBACK_STATE_FILE "/tmp/dhcp6c-callback.state"
#define DHCP6C_CALLBACK_GENERATION_ENV "DHCP6C_CALLBACK_GENERATION"
#define DHCP6C_CALLBACK_IFNAME_ENV "DHCP6C_CALLBACK_IFNAME"

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

void build_dns6_var(void)
{
	char dns6s[INET6_ADDRSTRLEN*3+4] = {0};
	char *dnsnv[3];

	if (is_wan_dns6_static() == 1) {
		dnsnv[0] = nvram_safe_get("ip6_dns1");
		dnsnv[1] = nvram_safe_get("ip6_dns2");
		dnsnv[2] = nvram_safe_get("ip6_dns3");
		if (*dnsnv[0] || *dnsnv[1] || *dnsnv[2]) {
			snprintf(dns6s, sizeof(dns6s), "%s %s %s", dnsnv[0], dnsnv[1], dnsnv[2]);
			trim_r(dns6s);
		}
	}

	set_wan_unit_value(0, "dns6", dns6s);
}

void reset_wan6_vars(void)
{
	struct in6_addr addr6;
	char addr6s[INET6_ADDRSTRLEN + 5] = {0};
	char *wan_addr6, *wan_gate6;
	int wan_size6;

	wan_gate6 = "";
	if (is_wan_addr6_static() == 1) {
		wan_addr6 = nvram_safe_get("ip6_wan_addr");
		wan_size6 = nvram_get_int("ip6_wan_size");
		wan_gate6 = nvram_safe_get("ip6_wan_gate");
		if (*wan_addr6 && wan_size6 >= 0 && wan_size6 <= 128) {
			memset(&addr6, 0, sizeof(addr6));
			if (ipv6_from_string(wan_addr6, &addr6) >= 0 &&
			    inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN) &&
			    wan_size6 > 0 && wan_size6 < 128) {
				snprintf(addr6s + strlen(addr6s), sizeof(addr6s) - strlen(addr6s), "/%d", wan_size6);
			}
		}
	}

	set_wan_unit_value(0, "ifname6", "");
	set_wan_unit_value(0, "addr6", addr6s);
	set_wan_unit_value(0, "gate6", wan_gate6);
	set_wan_unit_value(0, "6rd_relay", nvram_safe_get("ip6_6rd_relay"));
	set_wan_unit_value(0, "6rd_size", nvram_safe_get("ip6_6rd_size"));

	build_dns6_var();
}

int is_wan_addr6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (ipv6_type == IPV6_NATIVE_DHCP6 ||
	    ipv6_type == IPV6_6TO4)
		return 0;

	if (ipv6_type == IPV6_NATIVE_STATIC ||
	    ipv6_type == IPV6_6IN4 ||
	   (ipv6_type == IPV6_6RD && nvram_match("ip6_6rd_dhcp", "0")))
		return 1;

	return 0;
}

int is_wan_dns6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_match("ip6_dns_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_6IN4 ||
	    ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD)
		return 1;

	return 0;
}

int is_wan_ipv6_type_sit(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (ipv6_type == IPV6_6IN4 ||
	    ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD)
		return 1;

	return 0;
}

int is_wan_ipv6_if_ppp(void)
{
	if (nvram_get_int("ip6_wan_if") == 0)
		return 1;

	return 0;
}

void store_ip6rd_from_dhcp(const char *env_value, const char *prefix)
{
	int i;
	long ipv4_mask_len, ipv6_prefix_len;
	char tmp[100];
	char ip6rd[64];
	char addr6[INET6_ADDRSTRLEN];
	char *value, *values[4], *end;
	struct in6_addr prefix6;

	// "32 128 FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF 255.255.255.255 "

	if (nvram_match("ip6_6rd_dhcp", "0") || !env_value || !prefix)
		return;
	if (strlen(env_value) >= sizeof(ip6rd))
		return;

	snprintf(ip6rd, sizeof(ip6rd), "%s", env_value);
	value = ip6rd;
	for (i = 0; i < 4; i++) {
		values[i] = strtok_r(i == 0 ? value : NULL, " \t\r\n", &value);
		if (!values[i])
			return;
	}
	if (strtok_r(NULL, " \t\r\n", &value))
		return;

	for (end = values[0]; *end; end++)
		if (*end < '0' || *end > '9')
			return;
	ipv4_mask_len = strtol(values[0], &end, 10);
	if (*end || ipv4_mask_len < 0 || ipv4_mask_len > 32)
		return;

	for (end = values[1]; *end; end++)
		if (*end < '0' || *end > '9')
			return;
	ipv6_prefix_len = strtol(values[1], &end, 10);
	if (*end || ipv6_prefix_len < 0 || ipv6_prefix_len > 128)
		return;

	if (inet_pton(AF_INET6, values[2], &prefix6) != 1 ||
	    !is_valid_ipv4(values[3]))
		return;

	snprintf(addr6, sizeof(addr6), "%s/%ld", values[2], ipv6_prefix_len);
	nvram_set_temp(strcat_r(prefix, "addr6", tmp), addr6);
	nvram_set_temp(strcat_r(prefix, "6rd_size", tmp), values[0]);
	nvram_set_temp(strcat_r(prefix, "6rd_relay", tmp), values[3]);
}

int store_wan_dns6(char *dns6_new)
{
	char dns6s[INET6_ADDRSTRLEN*3+8] = {0};
	char *dns6_old;

	if (!dns6_new)
		return 0;

	snprintf(dns6s, sizeof(dns6s), "%s", dns6_new);
	trim_r(dns6s);

	if (!(*dns6s))
		return 0;

	dns6_old = get_wan_unit_value(0, "dns6");
	if (strcmp(dns6s, dns6_old) != 0) {
		set_wan_unit_value(0, "dns6", dns6s);
		return 1;
	}

	return 0;
}

char *get_wan_addr6_host(char *p_addr6s)
{
	char *tmp = p_addr6s;
	char *wan6_ifname = get_wan_unit_value(0, "ifname6");

	if (strlen(wan6_ifname) > 0) {
		if (get_ifaddr6(wan6_ifname, 0, p_addr6s)) {
			strsep(&tmp, "/");
			return p_addr6s;
		}
	}

	return NULL;
}

#define WAN6_SIT_CHECK(operation, description) do { \
	status = (operation); \
	if (status != 0) { \
		failure = (description); \
		goto failed; \
	} \
} while (0)

int start_sit_tunnel(int ipv6_type, char *wan_ifname, char *wan_addr4,
		     char *wan_gate4, char *wan_addr6)
{
	int sit_ttl, sit_mtu, size4, size6, status, cleanup_status;
	int tunnel_created, wan_route_added, written;
	char *sit_remote, *sit_relay, *sit_ep, *wan_gate6;
	char wan_addr4_text[INET_ADDRSTRLEN];
	char sit_remote_text[INET_ADDRSTRLEN];
	char sit_relay_text[INET_ADDRSTRLEN];
	char wan_gate6_text[INET6_ADDRSTRLEN];
	char generated_gate6_text[INET6_ADDRSTRLEN];
	char addr6s[INET6_ADDRSTRLEN + 5];
	char lan_addr6s[INET6_ADDRSTRLEN + 5];
	char sit_6rd_prefix[INET6_ADDRSTRLEN + 5];
	char sit_6rd_relay_prefix[32];
	struct in_addr addr4, net4;
	struct in6_addr addr6, net6, lan_addr6;
	const char *failure = "invalid parameters";

	status = -1;
	tunnel_created = 0;
	wan_route_added = 0;
	sit_ep = NULL;
	sit_relay = "";
	sit_remote = "any";
	wan_gate6 = "";
	generated_gate6_text[0] = '\0';
	lan_addr6s[0] = '\0';
	sit_6rd_prefix[0] = '\0';
	sit_6rd_relay_prefix[0] = '\0';

	if (!wan_ifname || !*wan_ifname ||
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
		if (!wan_addr6 || !*wan_addr6 || size6 < 0 ||
		    format_ipv4_literal(nvram_safe_get("ip6_6in4_remote"),
				       sit_remote_text, sizeof(sit_remote_text)) < 0) {
			errno = EINVAL;
			return -1;
		}
		sit_remote = sit_remote_text;
		sit_ep = sit_remote_text;
		wan_gate6 = get_wan_unit_value(0, "gate6");
		if (*wan_gate6) {
			if (format_ipv6_literal(wan_gate6, wan_gate6_text,
						sizeof(wan_gate6_text), 0) < 0) {
				errno = EINVAL;
				return -1;
			}
			wan_gate6 = wan_gate6_text;
		}
	} else if (ipv6_type == IPV6_6TO4) {
		if (format_ipv4_literal(nvram_safe_get("ip6_6to4_relay"),
				       sit_relay_text, sizeof(sit_relay_text)) < 0) {
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
		size4 = get_wan_unit_value_int(0, "6rd_size");
		if (size4 < 0 || size4 > 32 ||
		    size6 + 32 - size4 > 128 ||
		    format_ipv4_literal(get_wan_unit_value(0, "6rd_relay"),
				       sit_relay_text, sizeof(sit_relay_text)) < 0) {
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

	sit_mtu = nvram_get_int("ip6_sit_mtu");
	sit_ttl = nvram_get_int("ip6_sit_ttl");
	if (sit_mtu < 1280)
		sit_mtu = 1280;
	if (sit_ttl < 1)
		sit_ttl = 1;
	if (sit_ttl > 255)
		sit_ttl = 255;

	if (is_interface_exist(IFNAME_SIT))
		WAN6_SIT_CHECK(doSystem("ip tunnel del %s", IFNAME_SIT),
			       "remove previous SIT tunnel");

	WAN6_SIT_CHECK(doSystem("ip tunnel add %s mode sit remote %s local %s ttl %d",
				IFNAME_SIT, sit_remote, wan_addr4_text, sit_ttl),
		       "create SIT tunnel");
	tunnel_created = 1;

	if (ipv6_type == IPV6_6RD)
		WAN6_SIT_CHECK(doSystem("ip tunnel 6rd dev %s 6rd-prefix %s 6rd-relay_prefix %s",
					IFNAME_SIT, sit_6rd_prefix, sit_6rd_relay_prefix),
			       "configure 6rd tunnel");

	/* Route the IPv4 tunnel endpoint through the physical WAN first. */
	WAN6_SIT_CHECK(route_add(wan_ifname, 0, sit_ep, wan_gate4,
				 "255.255.255.255"), "route SIT IPv4 endpoint");
	wan_route_added = 1;

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
	WAN6_SIT_CHECK(doSystem("ip link set mtu %d dev %s up", sit_mtu, IFNAME_SIT),
		       "bring SIT interface up");
	WAN6_SIT_CHECK(control_if_ipv6(IFNAME_SIT, 1),
		       "enable IPv6 on SIT interface");
	WAN6_SIT_CHECK(clear_if_addr6(IFNAME_SIT),
		       "clear stale SIT IPv6 addresses");
	WAN6_SIT_CHECK(doSystem("ip -6 addr add %s dev %s", addr6s, IFNAME_SIT),
		       "add SIT WAN IPv6 address");

	if (ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
		WAN6_SIT_CHECK(doSystem("ip -6 route add default dev %s metric %d",
					IFNAME_SIT, 2048), "add SIT fallback default route");

	if (*wan_gate6)
		WAN6_SIT_CHECK(doSystem("ip -6 route add default via %s dev %s metric %d",
					wan_gate6, IFNAME_SIT, 1), "add SIT IPv6 default gateway");

	/* Publish a derived LAN address only after tunnel and routes are ready. */
	if (*lan_addr6s) {
		WAN6_SIT_CHECK(clear_if_addr6(IFNAME_BR),
			       "clear previous LAN IPv6 address");
		WAN6_SIT_CHECK(doSystem("ip -6 addr add %s dev %s",
					lan_addr6s, IFNAME_BR), "add derived LAN IPv6 address");
		store_lan_addr6(lan_addr6s);
	}

	set_vpn_balancing(IFNAME_SIT, 0);
	return 0;

failed:
	if (wan_route_added) {
		cleanup_status = route_del(wan_ifname, 0, sit_ep, wan_gate4,
					   "255.255.255.255");
		if (cleanup_status != 0)
			logmessage("WAN IPv6", "failed to roll back SIT endpoint route (status=%d)",
				   cleanup_status);
	}
	if (tunnel_created) {
		cleanup_status = doSystem("ip link set dev %s down", IFNAME_SIT);
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
int stop_sit_tunnel(void)
{
	if (is_interface_exist(IFNAME_SIT))
		return doSystem("ip tunnel del %s", IFNAME_SIT);
	return 0;
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
	int routes_touched, addresses_touched, radv_touched;
	int autoconf_touched, privacy_touched, dhcp6_start_attempted;
	char *wan_addr6, *wan_gate6, *wan_addr4, *wan_gate4;
	char addr6_text[INET6_ADDRSTRLEN + 5];
	char gate6_text[INET6_ADDRSTRLEN];
	const char *failure = "invalid parameters";

	if (!wan_ifname || !*wan_ifname || unit < 0) {
		errno = EINVAL;
		return -1;
	}

	ipv6_type = get_ipv6_type();
	status = stop_dhcp6c();
	if (status != 0) {
		logmessage("WAN IPv6", "failed to stop previous DHCPv6 client on %s (status=%d)",
			   wan_ifname, status);
		return status;
	}
	if (ipv6_type == IPV6_DISABLED)
		return 0;

	build_dns6_var();

	status = control_if_ipv6_dad(IFNAME_BR, 1);
	if (status != 0) {
		logmessage("WAN IPv6", "failed to configure bridge DAD on %s (status=%d)",
			   IFNAME_BR, status);
		return status;
	}

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD) {
		wan_addr4 = get_wan_unit_value(unit, "ipaddr");
		wan_gate4 = get_wan_unit_value(unit, "gateway");
		wan_addr6 = get_wan_unit_value(unit, "addr6");
		if (!is_valid_ipv4(wan_gate4))
			wan_gate4 = NULL;

		status = start_sit_tunnel(ipv6_type, wan_ifname, wan_addr4,
					  wan_gate4, wan_addr6);
		if (status != 0) {
			logmessage("WAN IPv6", "SIT setup incomplete on %s (status=%d)",
				   wan_ifname, status);
			return status;
		}
		set_wan_unit_value(unit, "ifname6", IFNAME_SIT);
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

	if (ipv6_type == IPV6_NATIVE_STATIC) {
		wan_addr6 = get_wan_unit_value(unit, "addr6");
		wan_gate6 = get_wan_unit_value(unit, "gate6");
		if (!wan_addr6 || !*wan_addr6 ||
		    format_ipv6_literal(wan_addr6, addr6_text,
					 sizeof(addr6_text), 1) < 0) {
			errno = EINVAL;
			status = -1;
			failure = "invalid native static IPv6 address";
			goto native_failed;
		}
		if (*wan_gate6 &&
		    format_ipv6_literal(wan_gate6, gate6_text,
					 sizeof(gate6_text), 0) < 0) {
			errno = EINVAL;
			status = -1;
			failure = "invalid native static IPv6 gateway";
			goto native_failed;
		}

		privacy_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_privacy(wan_ifname, 0),
				  "disable native static privacy addresses");
		radv_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_radv(wan_ifname, 0),
				  "disable native static router advertisements");

		addresses_touched = 1;
		WAN6_NATIVE_CHECK(clear_if_addr6(wan_ifname),
				  "clear previous native static IPv6 addresses");
		WAN6_NATIVE_CHECK(doSystem("ip -6 addr add %s dev %s",
					   addr6_text, wan_ifname),
				  "add native static IPv6 address");

		if (*wan_gate6) {
			routes_touched = 1;
			WAN6_NATIVE_CHECK(doSystem("ip -6 route add %s dev %s",
						   gate6_text, wan_ifname),
					  "add native static on-link gateway route");
			WAN6_NATIVE_CHECK(doSystem("ip -6 route add default via %s metric %d",
						   gate6_text, 1),
					  "add native static default route");
		}
	} else if (ipv6_type == IPV6_NATIVE_DHCP6) {
		routes_touched = 1;
		WAN6_NATIVE_CHECK(doSystem("ip -6 route add default dev %s metric %d",
					   wan_ifname, 2048),
				  "add native DHCPv6 fallback default route");
		allow_ra = nvram_invmatch("ip6_wan_dhcp", "1");
		privacy_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_privacy(wan_ifname,
							 allow_ra && nvram_match("ip6_wan_priv", "1")),
				  "configure native DHCPv6 privacy addresses");
		autoconf_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_autoconf(wan_ifname, allow_ra),
				  "configure native DHCPv6 autoconfiguration");
		radv_touched = 1;
		WAN6_NATIVE_CHECK(control_if_ipv6_radv(wan_ifname, 1),
				  "enable native DHCPv6 router advertisements");

		/* Wait for the interface to become ready before launching dhcp6c. */
		sleep(2);
		dhcp6_start_attempted = 1;
		WAN6_NATIVE_CHECK(start_dhcp6c(wan_ifname),
				  "start native DHCPv6 client");
	} else {
		errno = EINVAL;
		status = -1;
		failure = "unsupported IPv6 WAN type";
		goto native_failed;
	}

	set_wan_unit_value(unit, "ifname6", wan_ifname);
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
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back router advertisement setting (status=%d)",
				   operation_status);
	}
	if (autoconf_touched) {
		operation_status = control_if_ipv6_autoconf(wan_ifname, 0);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back autoconfiguration setting (status=%d)",
				   operation_status);
	}
	if (privacy_touched) {
		operation_status = control_if_ipv6_privacy(wan_ifname, 0);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to roll back privacy setting (status=%d)",
				   operation_status);
	}
	if (routes_touched) {
		operation_status = clear_if_route6(wan_ifname);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to clear partially configured routes (status=%d)",
				   operation_status);
	}
	if (addresses_touched) {
		operation_status = clear_if_addr6(wan_ifname);
		if (operation_status != 0)
			logmessage("WAN IPv6", "failed to clear partially configured addresses (status=%d)",
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
	char *wan6_ifname;

	stop_status = stop_dhcp6c();
	if (stop_status != 0) {
		logmessage("WAN IPv6", "refusing IPv6 teardown while DHCPv6 client/callback remains active on %s (status=%d)",
			   wan_ifname ? wan_ifname : "(unknown)", stop_status);
		return stop_status;
	}
	ipv6_type = get_ipv6_type();
	if (ipv6_type == IPV6_DISABLED)
		return 0;

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

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD) {
		wan6_ifname = IFNAME_SIT;
		if (is_interface_exist(IFNAME_SIT)) {
			operation_status = doSystem("ip link set dev %s down", IFNAME_SIT);
			status = route_status_accumulate(status, operation_status);
			if (operation_status != 0)
				logmessage("WAN IPv6", "failed to bring SIT interface down (status=%d)",
					   operation_status);
		}
	} else {
		wan6_ifname = wan_ifname;
	}

	operation_status = clear_if_route6(wan6_ifname);
	status = route_status_accumulate(status, operation_status);
	if (operation_status != 0)
		logmessage("WAN IPv6", "failed to clear routes on %s (status=%d)",
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

	if (status == 0) {
		set_wan_unit_value(unit, "dns6", "");
	} else {
		logmessage("WAN IPv6", "retaining IPv6 DNS state after incomplete teardown (status=%d)",
			   status);
	}

	return status;
}

int dhcp6c_main(int argc, char **argv)
{
	int ipv6_type, dns6_auto, lan6_auto;
	int callback_fd, unlock_status;
	unsigned long generation;
	char *dns6_new, *lan_addr6_new;
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

	ipv6_type = get_ipv6_type();
	if (ipv6_type != IPV6_NATIVE_DHCP6) {
		unlock_status = dhcp6c_callback_session_unlock(callback_fd);
		return unlock_status == 0 ? 0 : -1;
	}

	lan6_auto = nvram_get_int("ip6_lan_auto");
	if (lan6_auto) {
		lan_addr6_new = get_ifaddr6(IFNAME_BR, 0, addr6s);
		store_lan_addr6(lan_addr6_new);
	}

	dns6_auto = nvram_get_int("ip6_dns_auto");
	if (dns6_auto) {
		dns6_new = getenv("new_domain_name_servers");
		if (store_wan_dns6(dns6_new))
			refresh_resolvconf_nonfatal("DHCPv6 DNS update", 0);
	}

	unlock_status = dhcp6c_callback_session_unlock(callback_fd);
	return unlock_status == 0 ? 0 : -1;
}

int start_dhcp6c(char *wan_ifname)
{
	FILE *fp;
	int duid_fd, session_fd, start_status, unlock_status;
	int wan6_dhcp, dns6_auto, lan6_auto, sla_id, sla_len;
	int duid_error, saved_errno, generation_active;
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

	wan6_dhcp = nvram_get_int("ip6_wan_dhcp");
	dns6_auto = nvram_get_int("ip6_dns_auto");
	lan6_auto = nvram_get_int("ip6_lan_auto");

	if (!wan6_dhcp && !dns6_auto && !lan6_auto)
		return 0; /* No DHCPv6 client work is configured. */

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

	if (ether_atoe(nvram_safe_get("wan_hwaddr"), ea)) {
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
	saved_errno = errno;
	if (unsetenv(DHCP6C_CALLBACK_IFNAME_ENV) != 0 ||
	    unsetenv(DHCP6C_CALLBACK_GENERATION_ENV) != 0) {
		if (start_status == 0)
			start_status = -1;
		if (saved_errno == 0)
			saved_errno = errno ? errno : EIO;
	}
	if (start_status != 0) {
		if (dhcp6c_callback_session_advance(session_fd, &generation) != 0)
			logmessage("DHCPv6 WAN Client",
				   "cannot invalidate failed client start (errno=%d)", errno ? errno : EIO);
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
	(void)unsetenv(DHCP6C_CALLBACK_IFNAME_ENV);
	(void)unsetenv(DHCP6C_CALLBACK_GENERATION_ENV);
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
	int session_fd, stop_status, unlock_status, saved_errno;

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
		(void)dhcp6c_callback_session_unlock(session_fd);
		logmessage("DHCPv6 WAN Client",
			   "cannot invalidate callback session before stop (errno=%d)", saved_errno);
		errno = saved_errno;
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

