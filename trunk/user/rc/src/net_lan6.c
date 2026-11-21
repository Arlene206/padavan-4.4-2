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

#if defined (USE_IPV6)

int reset_lan6_vars(void)
{
	int api_result, saved_errno, status;
	struct in6_addr addr6;
	char addr6_text[INET6_ADDRSTRLEN];
	char addr6s[INET6_ADDRSTRLEN] = {0};
	char *lan_addr6;
	int lan_size6;

	if (is_lan_addr6_static() == 1)
	{
		lan_addr6 = nvram_safe_get("ip6_lan_addr");
		lan_size6 = nvram_get_int("ip6_lan_size");
		if (*lan_addr6) {
			memset(&addr6, 0, sizeof(addr6));
			if (ipv6_from_string(lan_addr6, &addr6) >= 0 &&
			    inet_ntop(AF_INET6, &addr6, addr6_text, sizeof(addr6_text))) {
				if (lan_size6 < 48 || lan_size6 > 64)
					lan_size6 = 64;
				if (rc_lan_addr6_format_prefix(addr6_text, lan_size6,
							       addr6s, sizeof(addr6s)) != 0)
					logmessage("LAN IPv6",
						   "cannot format configured LAN IPv6 prefix (errno=%d)",
						   errno ? errno : EIO);
			}
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
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_invmatch("ip6_lan_radv", "0"))
		return 1;

	return 0;
}

int is_lan_addr6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD)
		return 0;

	if (nvram_match("ip6_lan_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_6IN4)
		return 1;
	
	return 0;
}

int get_lan_dhcp6s_mode(void)
{
	if (get_ipv6_type() == IPV6_DISABLED)
		return -1;

	return nvram_get_int("ip6_lan_dhcp");
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
