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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <dirent.h>

#include "rc.h"
#include "dnsmasq_lifecycle_lock.h"
#include "service_start_status.h"
#include "ipv6_cleanup_policy.h"
#include "ipv6_global_conf_policy.h"

#if defined (USE_IPV6)

int init_ipv6(void)
{
	int ipv6_type = get_ipv6_type();
	int conf_status = control_if_ipv6_all((ipv6_type == IPV6_DISABLED) ? 0 : 1);
	int gai_status;

	if (conf_status != 0)
		logmessage(LOGNAME, "IPv6 global sysctl initialization incomplete (status=%d)", conf_status);
	gai_status = set_libc_gai((ipv6_type == IPV6_DISABLED) ? 1 : 0);
	if (gai_status != 0)
		logmessage(LOGNAME, "IPv6 address-selection policy initialization failed (status=%d)", gai_status);
	conf_status = rc_first_service_start_status(conf_status, gai_status);
	reset_lan6_vars();
	return conf_status;
}

static int write_ipv6_conf_operation(void *context, const char *ifname, const char *param, int value)
{
	(void)context;
	/* Per-device sysctls do not exist until their interface is registered. */
	if (strcmp(ifname, "all") != 0 && strcmp(ifname, "default") != 0 && !is_interface_exist(ifname))
		return 0;
	return set_interface_conf_int("ipv6", ifname, param, value);
}

int control_if_ipv6_all(int enable)
{
	static const struct rc_ipv6_conf_operation disable_ops[] = {
		{ "default", "disable_ipv6", 1 }, { "all", "disable_ipv6", 1 },
		{ "default", "accept_ra", 0 }, { "default", "accept_ra_pinfo", 0 },
		{ "lo", "accept_ra", 0 }, { "lo", "accept_ra_pinfo", 0 },
		{ "sit0", "accept_ra", 0 }, { "sit0", "accept_ra_pinfo", 0 },
		{ IFNAME_MAC, "accept_ra", 0 }, { IFNAME_MAC, "accept_ra_pinfo", 0 },
		{ "all", "forwarding", 0 }
	};
	static const struct rc_ipv6_conf_operation enable_ops[] = {
		{ "all", "forwarding", 1 }, { "default", "disable_ipv6", 0 },
		{ "lo", "disable_ipv6", 0 }, { IFNAME_BR, "disable_ipv6", 0 }
	};
	char path[64];
	int path_len, status;
	if (enable) {
		status = rc_ipv6_conf_apply(enable_ops, sizeof(enable_ops)/sizeof(enable_ops[0]), write_ipv6_conf_operation, NULL);
		if (is_interface_exist(IFNAME_BR)) {
			path_len = snprintf(path, sizeof(path), "/proc/sys/net/ipv6/neigh/%s/gc_stale_time", IFNAME_BR);
			if (path_len < 0) { errno = EIO; status = rc_first_service_start_status(status, EIO); }
			else if (path_len >= (int)sizeof(path)) { errno = ENAMETOOLONG; status = rc_first_service_start_status(status, ENAMETOOLONG); }
			else { int write_status = fput_int(path, 900); status = rc_first_service_start_status(status, write_status); }
		}
	} else {
		status = rc_ipv6_conf_apply(disable_ops, sizeof(disable_ops)/sizeof(disable_ops[0]), write_ipv6_conf_operation, NULL);
	}
	return status;
}

int control_if_ipv6(const char *ifname, int enable)
{
	return set_interface_conf_int("ipv6", ifname, "disable_ipv6", (enable) ? 0 : 1);
}

int control_if_ipv6_autoconf(const char *ifname, int enable)
{
	return set_interface_conf_int("ipv6", ifname, "accept_ra_pinfo", (enable) ? 1 : 0);
}

int control_if_ipv6_radv(const char *ifname, int enable)
{
	return set_interface_conf_int("ipv6", ifname, "accept_ra", (enable) ? 2 : 0);
}

int control_if_ipv6_dad(const char *ifname, int enable)
{
	int status, operation_status;

	status = set_interface_conf_int("ipv6", ifname, "accept_dad", (enable) ? 1 : 0);
	if (enable) {
		operation_status = set_interface_conf_int("ipv6", ifname, "dad_transmits",
						  (strcmp(ifname, IFNAME_BR) == 0) ? 2 : 1);
		if (status == 0)
			status = operation_status;
	}
	return status;
}

int control_if_ipv6_privacy(const char *ifname, int enable)
{
	return set_interface_conf_int("ipv6", ifname, "use_tempaddr", (enable) ? 2 : 0);
}

int clear_if_addr6(char *ifname)
{
	if (!ifname || !ifname[0]) {
		errno = EINVAL;
		return -1;
	}
	if (!is_interface_exist(ifname))
		return 0;
	return eval("ip", "-6", "addr", "flush", "dev", ifname, "scope", "global");
}

int clear_if_route6(char *ifname)
{
	if (!ifname || !ifname[0]) {
		errno = EINVAL;
		return -1;
	}
	if (!is_interface_exist(ifname))
		return 0;
	return eval("ip", "-6", "route", "flush", "dev", ifname);
}

int clear_if_neigh6(char *ifname)
{
	if (!ifname || !ifname[0]) {
		errno = EINVAL;
		return -1;
	}
	if (!is_interface_exist(ifname))
		return 0;
	return eval("ip", "-6", "neigh", "flush", "dev", ifname);
}

int clear_all_addr6(void)
{
	return doSystem("ip -6 addr flush scope global");
}

int clear_all_route6(void)
{
	return doSystem("ip -6 route flush scope all");
}

int
full_restart_ipv6(int ipv6_type_old)
{
	int ipv6_type = get_ipv6_type();
	int ipv6_toggled = ((ipv6_type == IPV6_DISABLED || ipv6_type_old == IPV6_DISABLED) && (ipv6_type != ipv6_type_old)) ? 1 : 0;
	int dnsmasq_status, dnsmasq_start_status, dnsmasq_stop_status, scan_error = 0;
	int dnsmasq_lock, dnsmasq_lock_errno;
	int restart_status = 0, operation_status, lock_release_status;
	int httpd_stop_status = 0, lltd_stop_status = 0;
#if defined (APP_NFSD)
	int nfs_stop_status;
#endif

	/*
	 * Serialize the IPv6-only DNSMasq configuration changes with watchdog and
	 * other RC applets. Acquire before stopping any service so lock failure
	 * leaves the existing network state untouched.
	 */
	dnsmasq_lock = dnsmasq_lifecycle_lock_acquire();
	if (dnsmasq_lock < 0) {
		dnsmasq_lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "IPv6 restart refused: cannot acquire DNSMasq lifecycle lock (errno=%d)",
			   dnsmasq_lock_errno);
		return dnsmasq_lock_errno;
	}

	if (ipv6_toggled) {
		lltd_stop_status = stop_lltd();
		if (lltd_stop_status != 0)
			logmessage(LOGNAME, "IPv6 restart could not stop LLTD (status=%d)",
				   lltd_stop_status);
		restart_status = rc_first_service_start_status(restart_status,
								lltd_stop_status);

		httpd_stop_status = stop_httpd();
		if (httpd_stop_status != 0)
			logmessage(LOGNAME, "IPv6 restart could not stop HTTPD (status=%d)",
				   httpd_stop_status);
		restart_status = rc_first_service_start_status(restart_status,
								httpd_stop_status);
	}
#if defined (APP_NFSD)
	nfs_stop_status = stop_nfsd();
	if (nfs_stop_status != 0)
		logmessage(LOGNAME, "IPv6 restart could not stop NFS (status=%d)",
			   nfs_stop_status);
	restart_status = rc_first_service_start_status(restart_status,
							nfs_stop_status);
#endif
	operation_status = stop_upnp();
	if (operation_status != 0)
		logmessage(LOGNAME, "IPv6 restart could not stop UPnP (status=%d)",
			   operation_status);
	restart_status = rc_first_service_start_status(restart_status,
							operation_status);

	operation_status = stop_dhcp6c();
	if (operation_status != 0)
		logmessage(LOGNAME, "IPv6 restart could not stop DHCPv6 client (status=%d)",
			   operation_status);
	restart_status = rc_first_service_start_status(restart_status,
							operation_status);

	dnsmasq_stop_status = stop_dns_dhcpd_locked();
	if (dnsmasq_stop_status != 0)
		logmessage(LOGNAME, "IPv6 restart could not confirm DNSMasq stop (status=%d); start remains fail-closed",
			   dnsmasq_stop_status);
	restart_status = rc_first_service_start_status(restart_status,
							dnsmasq_stop_status);

	if (ipv6_type == IPV6_DISABLED) {
		operation_status = rc_ipv6_disable_cleanup_apply(
			clear_all_route6, clear_all_addr6, clear_if_neigh6, IFNAME_BR);
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable global route/address/neighbor cleanup incomplete (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);

		operation_status = stop_sit_tunnel();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable could not remove SIT tunnel (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);

		reset_lan6_vars();
		reset_wan6_vars();
		operation_status = control_if_ipv6_all(0);
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable could not apply global sysctls (status=%d)", operation_status);
		restart_status = rc_first_service_start_status(restart_status, operation_status);
		operation_status = set_libc_gai(1);
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable could not apply address-selection policy (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
		/* This no-notify refresh cannot recurse into restart_dns(). */
		refresh_resolvconf_nonfatal_no_notify("IPv6 disabled", 0);
		operation_status = reload_nat_modules();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable could not reload NAT modules (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
		operation_status = restart_firewall();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 disable could not refresh firewall/UPnP mappings (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status, operation_status);
		dnsmasq_start_status = start_dns_dhcpd_locked(0);
		if (dnsmasq_start_status != 0)
			logmessage(LOGNAME, "IPv6 disable completed but DNSMasq did not restart (status=%d)",
				   dnsmasq_start_status);
		restart_status = rc_first_service_start_status(restart_status,
								dnsmasq_start_status);
		lock_release_status = dnsmasq_lifecycle_lock_release(dnsmasq_lock);
		if (lock_release_status != 0) {
			dnsmasq_lock_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock after IPv6 disable (errno=%d)",
				   dnsmasq_lock_errno);
			restart_status = rc_first_service_start_status(restart_status,
									dnsmasq_lock_errno);
		}
	} else {
		operation_status = set_libc_gai(0);
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 enable could not remove address-selection override (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
		operation_status = control_if_ipv6_all(1);
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 enable could not apply global sysctls (status=%d)", operation_status);
		restart_status = rc_first_service_start_status(restart_status, operation_status);
		operation_status = clear_all_addr6();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 enable could not clear stale global addresses (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
		reset_lan6_vars();
		operation_status = reload_lan_addr6();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 enable could not reload LAN address (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status, operation_status);

		/*
		 * Release before full_restart_wan(): WAN teardown owns the VPN lock and
		 * resolver refresh can re-enter the DNSMasq lock. Do not enter WAN
		 * teardown when release failed: lock ownership is then uncertain.
		 */
		lock_release_status = dnsmasq_lifecycle_lock_release(dnsmasq_lock);
		if (lock_release_status != 0) {
			dnsmasq_lock_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock before WAN restart (errno=%d)",
				   dnsmasq_lock_errno);
			restart_status = rc_first_service_start_status(restart_status,
									dnsmasq_lock_errno);
		} else {
			operation_status = full_restart_wan();
			if (operation_status != 0)
				logmessage(LOGNAME, "IPv6 reconfiguration WAN restart failed (status=%d)",
					   operation_status);
			restart_status = rc_first_service_start_status(restart_status,
									operation_status);

			/* Re-enter the DNSMasq barrier only after all VPN/WAN work is complete. */
			dnsmasq_lock = dnsmasq_lifecycle_lock_acquire();
			if (dnsmasq_lock < 0) {
				dnsmasq_lock_errno = errno ? errno : EIO;
				logmessage(LOGNAME, "IPv6 WAN restart completed but DNSMasq lifecycle lock reacquisition failed (errno=%d)",
					   dnsmasq_lock_errno);
				restart_status = rc_first_service_start_status(restart_status,
										dnsmasq_lock_errno);
			} else {
				dnsmasq_status = dnsmasq_process_status(&scan_error);
				if (dnsmasq_status == 0) {
					dnsmasq_start_status = start_dns_dhcpd_locked(0);
					if (dnsmasq_start_status != 0)
						logmessage(LOGNAME, "IPv6 restart could not start DNSMasq (status=%d)",
							   dnsmasq_start_status);
					restart_status = rc_first_service_start_status(
						restart_status, dnsmasq_start_status);
				} else if (dnsmasq_status < 0) {
					dnsmasq_lock_errno = scan_error ? scan_error : EIO;
					logmessage(LOGNAME,
						   "IPv6 restart cannot verify DNSMasq state; refusing duplicate start (scan_error=%d)",
						   dnsmasq_lock_errno);
					restart_status = rc_first_service_start_status(
						restart_status, dnsmasq_lock_errno);
				} else if (dnsmasq_status != 1) {
					logmessage(LOGNAME,
						   "IPv6 restart received invalid DNSMasq state; refusing duplicate start (status=%d)",
						   dnsmasq_status);
					restart_status = rc_first_service_start_status(
						restart_status, dnsmasq_status);
				}
				lock_release_status = dnsmasq_lifecycle_lock_release(dnsmasq_lock);
				if (lock_release_status != 0) {
					dnsmasq_lock_errno = errno ? errno : EIO;
					logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock after IPv6 WAN restart (errno=%d)",
						   dnsmasq_lock_errno);
					restart_status = rc_first_service_start_status(
						restart_status, dnsmasq_lock_errno);
				}
			}
		}
		operation_status = reload_nat_modules();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 reconfiguration could not reload NAT modules (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
	}
#if defined (APP_NFSD)
	if (nfs_stop_status == 0) {
		operation_status = run_nfsd();
		if (operation_status != 0)
			logmessage(LOGNAME, "IPv6 restart could not restart NFS (status=%d)",
				   operation_status);
		restart_status = rc_first_service_start_status(restart_status,
								operation_status);
	}
#endif
	if (ipv6_toggled) {
		if (httpd_stop_status == 0) {
			operation_status = start_httpd(0);
			if (operation_status != 0)
				logmessage(LOGNAME, "IPv6 restart could not restart HTTPD (status=%d)",
					   operation_status);
			restart_status = rc_first_service_start_status(restart_status,
									operation_status);
		}
		if (lltd_stop_status == 0 && nvram_match("lltd_enable", "1")) {
			operation_status = start_lltd();
			if (operation_status != 0)
				logmessage(LOGNAME, "IPv6 restart could not restart LLTD (status=%d)",
					   operation_status);
			restart_status = rc_first_service_start_status(restart_status,
									operation_status);
		}
	}
	return restart_status;
}

int ipv6_from_string(const char *str, struct in6_addr *addr6)
{
	char addr6s[INET6_ADDRSTRLEN] = "::";
	char *prefix, *end;
	long parsed_prefix;
	int ret = (!str || !*str) ? 0 : 128;

	if (!addr6)
		return -1;

	if (str && *str) {
		if (strlen(str) >= sizeof(addr6s))
			return -1;
		snprintf(addr6s, sizeof(addr6s), "%s", str);
	}

	prefix = strchr(addr6s, '/');
	if (prefix) {
		*prefix++ = '\0';
		if (!*prefix)
			return -1;

		for (end = prefix; *end; end++) {
			if (*end < '0' || *end > '9')
				return -1;
		}

		parsed_prefix = strtol(prefix, &end, 10);
		if (*end || parsed_prefix < 0 || parsed_prefix > 128)
			return -1;
		ret = (int)parsed_prefix;
	}

	if (inet_pton(AF_INET6, addr6s, addr6) != 1)
		return -1;

	return ret;
}

int ipv6_to_net(struct in6_addr *addr6, int prefix)
{
	int i, m;

	if (!addr6 || prefix < 0 || prefix > 128)
		return 0;

	i = prefix >> 5;
	m = prefix & 0x1f;
	if (m)
		addr6->s6_addr32[i++] &= htonl(0xffffffffUL << (32 - m));
	while (i < 4)
		addr6->s6_addr32[i++] = 0;

	return prefix;
}

int ipv6_to_host(struct in6_addr *addr6, int prefix)
{
	int i, m;

	if (!addr6 || prefix < 0 || prefix > 128)
		return 0;

	i = prefix >> 5;
	m = prefix & 0x1f;
	if (m) {
		addr6->s6_addr32[i--] &= htonl(0xffffffffUL >> m);
	} else {
		i--;
	}
	while (i >= 0)
		addr6->s6_addr32[i--] = 0;

	return 128 - prefix;
}

int ipv6_to_ipv4_map(struct in6_addr *addr6, int size6, struct in_addr *addr4, int size4)
{
	int i, m, ret;
	u_int32_t addr = 0;
	u_int32_t mask;

	if (!addr6 || size6 < 0 || size6 > 128 || size4 < 0 || size4 > 32)
		return 0;

	ret = size6 + 32 - size4;
	if (ret > 128)
		return 0;
	if (size4 == 32)
		return size6;

	i = size6 >> 5;
	m = size6 & 0x1f;
	mask = 0xffffffffUL << size4;

	if (addr4)
		addr = ntohl(addr4->s_addr) << size4;

	addr6->s6_addr32[i] &= ~htonl(mask >> m);
	addr6->s6_addr32[i] |= htonl(addr >> m);
	if (m && i < 3) {
		i++;
		addr6->s6_addr32[i] &= ~htonl(mask << (32 - m));
		addr6->s6_addr32[i] |= htonl(addr << (32 - m));
	}

	return ret;
}

int ipv6_compact(const char *str6, char *p_comp6, int allow_prefix)
{
	int prefix_len;
	size_t addr_len;
	struct in6_addr addr6;
	char compact[INET6_ADDRSTRLEN + 5];

	if (!str6 || !p_comp6)
		return -1;

	memset(&addr6, 0, sizeof(addr6));
	prefix_len = ipv6_from_string(str6, &addr6);
	if (prefix_len < 0)
		return -1;

	if (!inet_ntop(AF_INET6, &addr6, compact, sizeof(compact)))
		return -1;

	if (allow_prefix && prefix_len > 0 && prefix_len < 128) {
		addr_len = strlen(compact);
		snprintf(compact + addr_len, sizeof(compact) - addr_len,
			 "/%d", prefix_len);
	}

	/* INET6_ADDRSTRLEN also accommodates the longest address plus /128. */
	memcpy(p_comp6, compact, strlen(compact) + 1);

	return 0;
}


#endif

