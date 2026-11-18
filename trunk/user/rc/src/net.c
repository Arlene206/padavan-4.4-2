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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <dirent.h>

#include "rc.h"
#include "network_sysctl.h"
#include "process_inventory.h"
#include "pppoe_relay_policy.h"
#include "route_status_policy.h"
#include "conntrack_flush_policy.h"
#include "igmpproxy_config_file.h"
#include "service_start_status.h"
#include "service_wait_policy.h"
#include "ipv4_forward_policy.h"
#include "wireless_module_load_policy.h"
#include "hwnat_config_policy.h"
#include "sfe_config_policy.h"
#include "network_sysctl_policy.h"
#include "xupnpd_config_file.h"
#include "xupnpd_daemon_policy.h"
#include "detached_daemon_start_policy.h"
#if defined (APP_MSD_LITE)
#include "msd_lite_config_file.h"
#endif

#define SR_BUF_LEN (8192)

static int
route_manip(int cmd, char *ifname, int metric, char *dst, char *gateway, char *genmask)
{
	int s, saved_errno;
	struct rtentry rt;

	/* Open a raw socket to the kernel. */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return errno;

	/* Fill in rtentry. */
	memset(&rt, 0, sizeof(rt));
	if (dst && inet_aton(dst, &sin_addr(&rt.rt_dst)) == 0) {
		saved_errno = EINVAL;
		goto err;
	}
	if (gateway && inet_aton(gateway, &sin_addr(&rt.rt_gateway)) == 0) {
		saved_errno = EINVAL;
		goto err;
	}
	if (genmask && inet_aton(genmask, &sin_addr(&rt.rt_genmask)) == 0) {
		saved_errno = EINVAL;
		goto err;
	}
	rt.rt_metric = metric;
	rt.rt_flags = RTF_UP;
	if (sin_addr(&rt.rt_gateway).s_addr)
		rt.rt_flags |= RTF_GATEWAY;
	if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST)
		rt.rt_flags |= RTF_HOST;
	rt.rt_dev = ifname;

	/* Force address family to AF_INET. */
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	rt.rt_genmask.sa_family = AF_INET;

	if (ioctl(s, cmd, &rt) < 0) {
		saved_errno = errno;
		goto err;
	}

	close(s);
	return 0;

err:
	close(s);
	return saved_errno;
}
int
route_add(char *ifname, int metric, char *dst, char *gateway, char *genmask)
{
	if (metric < 0 || metric == INT_MAX)
		return EINVAL;
	return route_manip(SIOCADDRT, ifname, metric + 1, dst, gateway, genmask);
}

int
route_del(char *ifname, int metric, char *dst, char *gateway, char *genmask)
{
	if (metric < 0 || metric == INT_MAX)
		return EINVAL;
	return route_manip(SIOCDELRT, ifname, metric + 1, dst, gateway, genmask);
}

int
control_static_routes(char *ift, char *ifname, int is_add)
{
	char word[128], *next;
	char *route_buf;
	char *ipaddr, *netmask, *gateway, *metric, *metric_end;
	long metric_value;
	int status = 0, route_status;

	if (is_add && nvram_invmatch("sr_enable_x", "1"))
		return 0;
	if (!ifname || !*ifname)
		return EINVAL;
	route_buf = (char *)malloc(SR_BUF_LEN*sizeof(char));
	if (!route_buf) {
		logmessage(LOGNAME, "failed to allocate static route list (errno=%d)", ENOMEM);
		return ENOMEM;
	}
	fill_static_routes(route_buf, SR_BUF_LEN, ift);
	foreach(word, route_buf, next) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		if (!is_valid_ipv4(gateway))
			gateway = nvram_safe_get("wanx_gateway");	// oleg patch
		errno = 0;
		metric_value = strtol(metric, &metric_end, 10);
		if (errno == ERANGE || metric_end == metric || *metric_end ||
		    metric_value < 0 || metric_value >= INT_MAX)
			continue;
		route_status = is_add ?
			route_add(ifname, (int)metric_value, ipaddr, gateway, netmask) :
			route_del(ifname, (int)metric_value, ipaddr, gateway, netmask);
		if (is_add)
			route_status = route_status_idempotent_add(route_status);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to %s static route %s via %s on %s (error=%d)",
				   is_add ? "add" : "delete", ipaddr, gateway, ifname,
				   route_status);
	}
	free(route_buf);
	return status;
}

char*
sanity_hostname(char *hname)
{
	char *c;

	for (c = hname; *c; c++) {
		if (*c == 0x20)
			*c = '_';
	}

	return hname;
}

char*
get_our_hostname(void)
{
	char* host_name = nvram_safe_get("computer_name");
	if (!(*host_name) || !is_valid_hostname(host_name))
		host_name = nvram_safe_get("productid");
	return sanity_hostname(host_name);
}

int
is_same_subnet(const char *ip1, const char *ip2, const char *msk)
{
	unsigned int addr1, addr2, mask;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask  = ntohl(inet_addr(msk));

	return (addr1 & mask) == (addr2 & mask);
}

int
is_same_subnet2(const char *ip1, const char *ip2, const char *msk1, const char *msk2)
{
	unsigned int addr1, addr2, mask, mask1, mask2;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask1 = ntohl(inet_addr(msk1));
	mask2 = ntohl(inet_addr(msk2));

	mask = (mask1 < mask2) ? mask1 : mask2;

	return (addr1 & mask) == (addr2 & mask);
}

static int
create_vlan_iface_internal(const char *dev_ifname, int vid, int prio, int mtu,
			   const char *hwaddr, int do_up, int *created_out)
{
	char vlan_ifname[24];
	int ret, saved_errno, created = 0, path_len;

	if (created_out)
		*created_out = 0;
	if (!dev_ifname || !*dev_ifname || vid < 0 || vid > 4095) {
		errno = EINVAL;
		return -1;
	}
	path_len = snprintf(vlan_ifname, sizeof(vlan_ifname), "%s.%d", dev_ifname, vid);
	if (path_len < 0 || path_len >= (int)sizeof(vlan_ifname)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (!is_interface_exist(vlan_ifname)) {
		ret = vconfig_add_if(dev_ifname, (unsigned int)vid);
		if (ret != 0) {
			/* A concurrent network transition may have created this exact VLAN. */
			if (!is_interface_exist(vlan_ifname))
				return ret;
		} else {
			created = 1;
		}
	} else {
		ret = ifconfig(vlan_ifname, 0, NULL, NULL);
		if (ret != 0)
			return ret;
	}

	if (prio >= 0 && prio <= 7) {
		ret = vconfig_egress_map(vlan_ifname, 0, prio);
		if (ret != 0)
			goto fail;
	}
	if (mtu >= 1280) {
		ret = set_interface_mtu(vlan_ifname, mtu);
		if (ret != 0)
			goto fail;
	}
	if (hwaddr) {
		ret = set_interface_hwaddr(vlan_ifname, hwaddr);
		if (ret != 0)
			goto fail;
	}
	if (do_up) {
		ret = ifconfig(vlan_ifname, IFUP, "0.0.0.0", NULL);
		if (ret != 0)
			goto fail;
	}
	if (created_out)
		*created_out = created;
	return 0;

fail:
	saved_errno = errno;
	if (created) {
		/* Roll back only the interface created by this invocation. */
		ifconfig(vlan_ifname, 0, NULL, NULL);
		vconfig_del_if(vlan_ifname);
	}
	errno = saved_errno;
	return ret;
}

int
create_vlan_iface(const char *dev_ifname, int vid, int prio, int mtu,
		  const char *hwaddr, int do_up)
{
	return create_vlan_iface_internal(dev_ifname, vid, prio, mtu, hwaddr,
				  do_up, NULL);
}

int
create_vlan_iface_tracked(const char *dev_ifname, int vid, int prio, int mtu,
			  const char *hwaddr, int do_up, int *created_out)
{
	if (!created_out) {
		errno = EINVAL;
		return -1;
	}
	return create_vlan_iface_internal(dev_ifname, vid, prio, mtu, hwaddr,
				  do_up, created_out);
}

int
remove_vlan_iface(const char *vlan_ifname)
{
	int ret;

	if (!vlan_ifname || !strchr(vlan_ifname, '.'))
		return 0;
	if (!is_interface_exist(vlan_ifname))
		return 0;

	ret = ifconfig(vlan_ifname, 0, NULL, NULL);
	if (ret != 0)
		return ret;
	return vconfig_del_if(vlan_ifname);
}

static enum process_presence
iptv_proxy_process_presence(void *context, int *scan_error)
{
	const char *process_name = (const char *)context;
	struct process_snapshot snapshot;
	enum process_presence state;

	if (!process_name || !*process_name) {
		if (scan_error)
			*scan_error = EINVAL;
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan(process_name, &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = snapshot.error ? snapshot.error : EIO;
	process_snapshot_release(&snapshot);
	return state;
}

static int
iptv_proxy_process_wait(void *context, unsigned int seconds)
{
	unsigned int remaining;

	(void)context;
	remaining = sleep(seconds);
	if (remaining != 0) {
		errno = EINTR;
		return -1;
	}
	return 0;
}

static int
iptv_proxy_confirm_started(const char *process_name, int launch_status)
{
	return detached_daemon_start_postcondition_with_ops(
		launch_status, iptv_proxy_process_presence, iptv_proxy_process_wait,
		(void *)process_name, PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS,
		PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS,
		PROCESS_PRESENCE_START_PROBE_ATTEMPTS);
}

void
stop_udpxy(void)
{
	char* svcs[] = { "udpxy", "msd_lite", NULL };

	kill_services(svcs, 3, 1);
}

int
start_udpxy(char *wan_ifname)
{
	int port = nvram_get_int("udpxy_enable_x");
	int launch_status, scan_error = 0, saved_errno;
	enum process_presence process_state;
	const char *process_name;

	if (port < 1024)
		return 0;

#if defined (APP_MSD_LITE)
	process_name = "msd_lite";
#else
	process_name = "udpxy";
#endif

	/* Do not replace a working relay or publish config on an incomplete scan. */
	process_state = iptv_proxy_process_presence((void *)process_name, &scan_error);
	if (process_state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = scan_error > 0 ? scan_error : EIO;
		errno = saved_errno;
		logmessage(LOGNAME, "%s process inventory is UNKNOWN; refusing startup (errno=%d)",
			   process_name, saved_errno);
		return saved_errno;
	}
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		saved_errno = process_state == PROCESS_PRESENCE_RUNNING ? EBUSY : EIO;
		errno = saved_errno;
		logmessage(LOGNAME, "%s is already present or its state is invalid; refusing duplicate startup",
			   process_name);
		return saved_errno;
	}

#if defined (APP_MSD_LITE)
	{
		const char *src_conf = "/etc_ro/msd_lite.conf";
		const char *dst_conf = "/etc/storage/msd_lite.conf";
		int config_status;

		config_status = msd_lite_config_publish(src_conf, dst_conf, wan_ifname, port);
		if (config_status != 0) {
			logmessage(LOGNAME, "MSD Lite configuration publication failed (status=%d)",
				   config_status);
			return config_status;
		}

		nvram_set("msd_lite_enable", "1");
		launch_status = eval("/usr/bin/msd_lite", "-d", "-c", dst_conf);
	}
#else
	launch_status = eval("/usr/sbin/udpxy",
		"-m", wan_ifname,
		"-p", nvram_safe_get("udpxy_enable_x"),
		"-B", "65536",
		"-c", nvram_safe_get("udpxy_clients"),
		"-M", nvram_safe_get("udpxy_renew_period")
		);
#endif

	launch_status = iptv_proxy_confirm_started(process_name, launch_status);
	if (launch_status != 0)
		logmessage(LOGNAME, "%s launch/readiness failed (status=%d errno=%d)",
			   process_name, launch_status, errno ? errno : EIO);
	return launch_status;
}


#if defined(APP_XUPNPD)
int
is_xupnpd_support(void)
{
	return check_if_file_exist("/usr/bin/xupnpd");
}

void
stop_xupnpd(void)
{
	char* svcs[] = { "xupnpd", NULL };

	if (!is_xupnpd_support())
		return;

	kill_services(svcs, 3, 1);
}

static enum process_presence
xupnpd_process_presence(void *context, int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence state;

	(void)context;
	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan("xupnpd", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = snapshot.error ? snapshot.error : EIO;
	process_snapshot_release(&snapshot);
	return state;
}

static int
xupnpd_process_wait(void *context, unsigned int seconds)
{
	unsigned int remaining;

	(void)context;
	remaining = sleep(seconds);
	if (remaining != 0) {
		errno = EINTR;
		return -1;
	}
	return 0;
}

static int
xupnpd_confirm_started(int launch_status)
{
	return xupnpd_daemon_start_postcondition_with_ops(
		launch_status, xupnpd_process_presence, xupnpd_process_wait,
		NULL, PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS,
		PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS,
		PROCESS_PRESENCE_START_PROBE_ATTEMPTS);
}

int
start_xupnpd(char *wan_ifname)
{
	int i, xport, has_daemon, launch_status;
	int config_status, saved_errno, sync_status, config_fd, scan_error;
	enum process_presence process_state;
	int copy_storage, directory_status, copy_status;
	FILE *fp1, *fp2;
	char tmp1[128], tmp2[128], line[256];
	char *dir_src = "/etc_ro/xupnpd";
	char *dir_dst = "/etc/storage/xupnpd";
	char *xdir1[] = { "config", "playlists", NULL };
	char *xdir2[] = { "plugins", "profiles", NULL };
	char *xlua[] = { "_http", "_m3u", "_main", "_mime", "_soap", "_ssdp", "_webapp", NULL };

	if (!is_xupnpd_support())
		return 0;
	if (!xupnpd_config_valid_interface(wan_ifname) ||
	    !xupnpd_config_valid_ipv4(nvram_safe_get("lan_ipaddr_t")))
		return EINVAL;


	xport = nvram_get_int("xupnpd_enable_x");
	if (xport < 1024)
		return 0;
	if (xport > 65535)
		return EINVAL;

	/* A complete procfs inventory is required before touching the live PID
	 * file or publishing configuration. UNKNOWN is never an empty process set. */
	process_state = xupnpd_process_presence(NULL, &scan_error);
	if (process_state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = scan_error ? scan_error : EIO;
		errno = saved_errno;
		logmessage(LOGNAME, "xupnpd inventory UNKNOWN; refusing startup (errno=%d)",
			   saved_errno);
		return saved_errno;
	}
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		errno = EBUSY;
		logmessage(LOGNAME, "xupnpd is already present; refusing duplicate startup");
		return EBUSY;
	}

	{
		int status = xupnpd_config_directory_validate("/etc/storage");
		if (status != 0) {
			logmessage(LOGNAME, "xupnpd storage root is unsafe (status=%d)", status);
			return status;
		}
	}

	directory_status = xupnpd_config_ensure_directory(dir_dst, 0755);
	if (directory_status != 0) {
		logmessage(LOGNAME, "xupnpd storage directory creation/validation failed (status=%d)",
			   directory_status);
		return directory_status;
	}

	for (i=0; xdir1[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir1[i]);
		directory_status = xupnpd_config_ensure_directory(tmp2, 0755);
		if (directory_status != 0) {
			logmessage(LOGNAME, "xupnpd storage subdirectory %s is unsafe (status=%d)",
				   tmp2, directory_status);
			return directory_status;
		}
	}
	snprintf(tmp2, sizeof(tmp2), "%s/config", dir_dst);
	{
		int status = xupnpd_config_directory_validate(tmp2);
		if (status != 0) {
			logmessage(LOGNAME, "xupnpd config directory is unsafe (status=%d)", status);
			return status;
		}
	}

	if (unlink("/var/run/xupnpd.pid") != 0 && errno != ENOENT) {
		directory_status = errno ? errno : EIO;
		logmessage(LOGNAME, "xupnpd stale pid cleanup failed (status=%d)",
			   directory_status);
		return directory_status;
	}

	copy_storage = get_mtd_size("Storage") > 0x10000;
	for (i=0; xdir2[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir2[i]);
		snprintf(tmp1, sizeof(tmp1), "%s/%s", dir_src, xdir2[i]);
		directory_status = xupnpd_config_directory_validate(tmp1);
		if (directory_status != 0) {
			logmessage(LOGNAME, "xupnpd read-only source directory %s is unsafe (status=%d)",
				   tmp1, directory_status);
			return directory_status;
		}
		directory_status = xupnpd_config_subdirectory_validate(
			tmp2, tmp1, !copy_storage);
		if (directory_status == ENOENT) {
			if (copy_storage) {
				copy_status = doSystem("cp -rf %s %s", tmp1, tmp2);
				if (copy_status != 0) {
					logmessage(LOGNAME,
						   "xupnpd storage copy %s failed (status=%d)",
						   tmp1, copy_status);
					return copy_status;
				}
			} else if (symlink(tmp1, tmp2) != 0 && errno != EEXIST) {
				directory_status = errno ? errno : EIO;
				logmessage(LOGNAME,
					   "xupnpd storage link %s -> %s failed (status=%d)",
					   tmp2, tmp1, directory_status);
				return directory_status;
			}
			directory_status = xupnpd_config_subdirectory_validate(
				tmp2, tmp1, !copy_storage);
		}
		if (directory_status != 0) {
			logmessage(LOGNAME,
				   "xupnpd storage subdirectory %s validation failed (status=%d)",
				   tmp2, directory_status);
			return directory_status;
		}
	}

	for (i=0; xlua[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/xupnpd%s.lua", dir_dst, xlua[i]);
		if (unlink(tmp2) != 0 && errno != ENOENT) {
			directory_status = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "xupnpd stale Lua file cleanup %s failed (status=%d)",
				   tmp2, directory_status);
			return directory_status;
		}
	}

	snprintf(tmp1, sizeof(tmp1), "%s/xupnpd%s.lua", dir_src, "");
	snprintf(tmp2, sizeof(tmp2), "%s/xupnpd%s.lua", dir_dst, "");
	{
		struct stat lua_stat;
		if (lstat(tmp2, &lua_stat) == 0) {
			if (S_ISLNK(lua_stat.st_mode)) {
				logmessage(LOGNAME, "xupnpd runtime Lua path is a symlink");
				return ELOOP;
			}
			if (!S_ISREG(lua_stat.st_mode)) {
				logmessage(LOGNAME, "xupnpd runtime Lua path is not a regular file");
				return EINVAL;
			}
		} else {
			if (errno != ENOENT) {
				directory_status = errno ? errno : EIO;
				logmessage(LOGNAME,
					   "xupnpd runtime Lua path inspection failed (status=%d)",
					   directory_status);
				return directory_status;
			}
			copy_status = doSystem("cp -f %s %s", tmp1, tmp2);
			if (copy_status != 0) {
				int cleanup_status = unlink(tmp2);
				if (cleanup_status != 0 && errno != ENOENT)
					logmessage(LOGNAME,
						   "xupnpd partial Lua cleanup %s failed (errno=%d)",
						   tmp2, errno);
				logmessage(LOGNAME,
					   "xupnpd runtime Lua copy %s failed (status=%d)",
					   tmp1, copy_status);
				return copy_status;
			}
		}
	}

	/*
	 * Do not replace a working config with an empty/truncated file when the
	 * old config cannot be read or a write/close/rename step fails.
	 */
	snprintf(tmp2, sizeof(tmp2), "%s/config/common.lua", dir_dst);
	config_fd = xupnpd_config_tempfile_open(tmp2, tmp1, sizeof(tmp1));
	if (config_fd < 0)
		return errno ? errno : EIO;
	fp1 = fdopen(config_fd, "w");
	if (fp1 == NULL) {
		saved_errno = errno ? errno : EIO;
		close(config_fd);
		unlink(tmp1);
		return saved_errno;
	}
	fp2 = xupnpd_config_source_open(tmp2);
	if (fp2 == NULL) {
		saved_errno = errno ? errno : EIO;
		fclose(fp1);
		unlink(tmp1);
		return saved_errno;
	}
	config_status = 0;
	while (fgets(line, sizeof(line), fp2)) {
		if (strstr(line, "mcast_interface") && !strstr(line, "--"))
			snprintf(line, sizeof(line), "cfg[\"mcast_interface\"]=\"%s\"\n", wan_ifname);
		else if (strstr(line, "http_port") && !strstr(line, "--"))
			snprintf(line, sizeof(line), "cfg[\"http_port\"]=%d\n", xport);
		if (fputs(line, fp1) == EOF) {
			config_status = errno ? errno : EIO;
			break;
		}
	}
	if (config_status == 0 && ferror(fp2))
		config_status = errno ? errno : EIO;
	if (fclose(fp2) != 0 && config_status == 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0 && fflush(fp1) != 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0) {
		do {
			errno = 0;
			sync_status = fsync(fileno(fp1));
		} while (sync_status != 0 && errno == EINTR);
		if (sync_status != 0)
			config_status = errno ? errno : EIO;
	}
	if (fclose(fp1) != 0 && config_status == 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0 && rename(tmp1, tmp2) != 0)
		config_status = errno ? errno : EIO;
	if (config_status != 0) {
		unlink(tmp1);
		return config_status;
	}

	has_daemon = 0;
	snprintf(tmp2, sizeof(tmp2), "%s/xupnpd.lua", dir_dst);
	config_fd = xupnpd_config_tempfile_open(tmp2, tmp1, sizeof(tmp1));
	if (config_fd < 0)
		return errno ? errno : EIO;
	fp1 = fdopen(config_fd, "w");
	if (fp1 == NULL) {
		saved_errno = errno ? errno : EIO;
		close(config_fd);
		unlink(tmp1);
		return saved_errno;
	}
	fp2 = xupnpd_config_source_open(tmp2);
	if (fp2 == NULL) {
		saved_errno = errno ? errno : EIO;
		fclose(fp1);
		unlink(tmp1);
		return saved_errno;
	}
	config_status = 0;
	while (fgets(line, sizeof(line), fp2)) {
		if (strstr(line, "cfg.mcast_interface=") && !strstr(line, "--"))
			snprintf(line, sizeof(line), "cfg.mcast_interface='%s'\n", wan_ifname);
		else if (strstr(line, "cfg.http_port=") && !strstr(line, "--"))
			snprintf(line, sizeof(line), "cfg.http_port=%d\n", xport);
		else if (strstr(line, "cfg.udpxy_url=")) {
			char *lua_rem = "--";
			char *lan_addr = nvram_safe_get("lan_ipaddr_t");
			int udpxy_port = nvram_get_int("udpxy_enable_x");
			if (udpxy_port < 0 || udpxy_port > 65535) {
				config_status = EINVAL;
				break;
			}
			if (udpxy_port > 1023 && nvram_match("xupnpd_udpxy", "1"))
				lua_rem = "";
			if (udpxy_port == 0)
				udpxy_port = 4022;
			snprintf(line, sizeof(line), "%scfg.udpxy_url='http://%s:%d'\n", lua_rem, lan_addr, udpxy_port);
		}
		else if (strstr(line, "cfg.daemon") && !strstr(line, "--")) {
			snprintf(line, sizeof(line), "cfg.daemon=true\n");
			has_daemon = 1;
		}
		if (fputs(line, fp1) == EOF) {
			config_status = errno ? errno : EIO;
			break;
		}
	}
	if (config_status == 0 && ferror(fp2))
		config_status = errno ? errno : EIO;
	if (fclose(fp2) != 0 && config_status == 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0 && fflush(fp1) != 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0) {
		do {
			errno = 0;
			sync_status = fsync(fileno(fp1));
		} while (sync_status != 0 && errno == EINTR);
		if (sync_status != 0)
			config_status = errno ? errno : EIO;
	}
	if (fclose(fp1) != 0 && config_status == 0)
		config_status = errno ? errno : EIO;
	if (config_status == 0 && rename(tmp1, tmp2) != 0)
		config_status = errno ? errno : EIO;
	if (config_status != 0) {
		unlink(tmp1);
		return config_status;
	}

	/* Config preparation can take several seconds; recheck immediately before
	 * launch to avoid racing another RC/UI start request. */
	process_state = xupnpd_process_presence(NULL, &scan_error);
	if (process_state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = scan_error ? scan_error : EIO;
		errno = saved_errno;
		logmessage(LOGNAME, "xupnpd inventory UNKNOWN before launch; refusing (errno=%d)",
			   saved_errno);
		return saved_errno;
	}
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		errno = EBUSY;
		logmessage(LOGNAME, "xupnpd appeared before launch; refusing duplicate startup");
		return EBUSY;
	}

	if (has_daemon)
		launch_status = eval("/usr/bin/xupnpd");
	else
		launch_status = system("/usr/bin/xupnpd &");
	launch_status = xupnpd_confirm_started(launch_status);
	if (launch_status != 0)
		logmessage(LOGNAME,
			   "xupnpd launch/readiness failed (status=%d errno=%d)",
			   launch_status, errno ? errno : EIO);
	return launch_status;
}

#endif

int
stop_igmpproxy(char *wan_ifname)
{
	char *svcs[] = { "xupnpd", "udpxy", "msd_lite", "igmpproxy", NULL };

	/* check used IPTV via VLAN interface */
	if (wan_ifname) {
		char *viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast) && strcmp(wan_ifname, viptv_iflast))
			return 0;
	}

	return kill_services(svcs, 3, 1);
}

int
start_igmpproxy(char *wan_ifname)
{
	struct igmpproxy_config_values config_values;
	char *altnet, *viptv_iflast;
	const char *altnet_mask;
	char *igmpproxy_conf = "/etc/igmpproxy.conf";
	int config_status, launch_status, scan_error = 0, saved_errno;
	int start_status = 0;
	enum process_presence process_state;

	if (wan_ifname == NULL || *wan_ifname == '\0')
		return EINVAL;

	/* check used IPTV via VLAN interface */
	viptv_iflast = nvram_safe_get("viptv_ifname");
	if (*viptv_iflast && is_interface_exist(viptv_iflast) && strcmp(wan_ifname, viptv_iflast))
		return 0;

	/* Do not start replacements until every old IPTV process is confirmed stopped. */
	{
		int stop_status = stop_igmpproxy(wan_ifname);
		if (stop_status != 0) {
			logmessage("IPTV", "proxy restart aborted because an existing proxy did not stop (status=%d)",
				   stop_status);
			return stop_status;
		}
	}

	start_status = rc_first_service_start_status(start_status, start_udpxy(wan_ifname));
#if defined(APP_XUPNPD)
	start_status = rc_first_service_start_status(start_status, start_xupnpd(wan_ifname));
#endif

	if (nvram_invmatch("mr_enable_x", "1"))
		return start_status;

	/* IGMPproxy forks by default; never trust the short-lived parent status. */
	process_state = iptv_proxy_process_presence((void *)"igmpproxy", &scan_error);
	if (process_state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = scan_error > 0 ? scan_error : EIO;
		errno = saved_errno;
		logmessage("IPTV", "igmpproxy process inventory is UNKNOWN; refusing startup (errno=%d)",
			   saved_errno);
		return rc_first_service_start_status(start_status, saved_errno);
	}
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		saved_errno = process_state == PROCESS_PRESENCE_RUNNING ? EBUSY : EIO;
		errno = saved_errno;
		logmessage("IPTV", "igmpproxy is already present or its state is invalid; refusing duplicate startup");
		return rc_first_service_start_status(start_status, saved_errno);
	}

	altnet = nvram_get("mr_altnet_x");
	if (altnet == NULL || strlen(altnet) <= 6)
		altnet_mask = "0.0.0.0/0";
	else
		altnet_mask = altnet;

	config_values.upstream_interface = wan_ifname;
	config_values.altnet = altnet_mask;
	config_values.downstream_interface = IFNAME_BR;
	config_values.quickleave = nvram_invmatch("mr_qleave_x", "0");
	config_status = igmpproxy_config_publish(igmpproxy_conf, &config_values);
	if (config_status != 0) {
		logmessage("IPTV", "igmpproxy config publication failed (status=%d)",
			   config_status);
		return rc_first_service_start_status(start_status, config_status);
	}

	launch_status = eval("/bin/igmpproxy", igmpproxy_conf);
	launch_status = iptv_proxy_confirm_started("igmpproxy", launch_status);
	if (launch_status != 0)
		logmessage("IPTV", "igmpproxy launch/readiness failed (status=%d errno=%d)",
			   launch_status, errno ? errno : EIO);
	return rc_first_service_start_status(start_status, launch_status);
}

int
restart_iptv(int is_ap_mode)
{
    int operation_status;
    int stop_status;
    int start_status = 0;

    operation_status = config_bridge(is_ap_mode);
    start_status = rc_first_service_start_status(start_status, operation_status);

    if (!is_ap_mode) {
        char *wan_ifname, *viptv_iflast;

        /* check used IPTV via VLAN interface */
        viptv_iflast = nvram_safe_get("viptv_ifname");
        if (*viptv_iflast && is_interface_exist(viptv_iflast))
            wan_ifname = viptv_iflast;
        else
            wan_ifname = get_man_ifname(0);
        operation_status = start_igmpproxy(wan_ifname);
        return rc_first_service_start_status(start_status, operation_status);
    }

    stop_status = stop_igmpproxy(NULL);
    if (stop_status != 0) {
        logmessage("IPTV", "AP-mode proxy restart aborted because an existing proxy did not stop (status=%d)",
                   stop_status);
        return rc_first_service_start_status(start_status, stop_status);
    }
    start_status = rc_first_service_start_status(start_status, start_udpxy(IFNAME_BR));
#if defined(APP_XUPNPD)
    start_status = rc_first_service_start_status(start_status, start_xupnpd(IFNAME_BR));
#endif
    return start_status;
}

int
flush_conntrack_table(char *ip)
{
	return rc_conntrack_flush_apply(ip, fput_string);
}

int
flush_route_caches(void)
{
	return eval("ip", "route", "flush", "cache");
}

int
clear_if_route4(char *ifname)
{
	if (!ifname || !ifname[0])
		return EINVAL;

	/* Keep the interface name as one argv element; never interpolate it into a shell. */
	return eval("ip", "route", "flush", "dev", ifname, "scope", "global");
}

int
is_ftp_conntrack_loaded(int ftp_port0, int ftp_port1)
{
	char ports_use[64] = {0};
	char ports_val[64];

	if (module_param_get("nf_conntrack_ftp", "ports", ports_use, sizeof(ports_use)))
		return 0;

	if (ftp_port0 || ftp_port1)
	{
		if (ftp_port0 && ftp_port1)
			sprintf(ports_val, "%d,%d", ftp_port0, ftp_port1);
		else if (ftp_port0)
			sprintf(ports_val, "%d", ftp_port0);
		else
			sprintf(ports_val, "%d", ftp_port1);
		
		if (strcmp(ports_val, ports_use) == 0)
			return 2;
	}

	return 1;
}

#if defined (USE_HW_NAT)
static int
is_hwnat_loaded_status(void)
{
	return is_module_loaded_status("hw_nat");
}

static int
is_hwnat_allow(void)
{
	int sw_mode = nvram_get_int("sw_mode");
	int hw_nat_mode = nvram_get_int("hw_nat_mode");

	if (sw_mode != 1 && sw_mode != 4)
		return 0;

	if (hw_nat_mode == 0)
		return 0;

	return 1;
}

static int
hwnat_set_wan_vid(int wan_vid)
{
	int status;

	errno = 0;
	status = module_param_set_int("hw_nat", "wan_vid", wan_vid);
	if (status == 0)
		return 0;

	return errno ? errno : EIO;
}

static int
hwnat_report_command_status(const char *operation, int command_status,
			    int saved_errno)
{
	int status = rc_hwnat_command_status_policy(command_status, saved_errno);

	if (status != 0)
		logmessage(LOGNAME, "%s failed (status=%d)", operation, status);
	return status;
}

static int
hwnat_set_radio_registration(const char *ifname, int enabled)
{
	int command_status, saved_errno;

	errno = 0;
	command_status = doSystem("iwpriv %s set hw_nat_register=%d", ifname, enabled);
	saved_errno = errno;
	return hwnat_report_command_status("Hardware NAT radio registration",
					   command_status, saved_errno);
}

static int
hwnat_set_ipv6_nat_debug(int enabled)
{
	int command_status, saved_errno;

	errno = 0;
	command_status = doSystem("echo 7 %d > /sys/kernel/debug/hnat/hnat_setting",
				   enabled);
	saved_errno = errno;
	return hwnat_report_command_status("Hardware NAT IPv6 debug setting",
					   command_status, saved_errno);
}

static int
hwnat_configure(int wan_vid, int hwnat_allow)
{
	const char *hwnat_status = "Disabled";
	int module_loaded;
	int status;

	if (!hwnat_allow) {
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
		return 0;
	}

	module_loaded = is_hwnat_loaded_status();
	if (module_loaded < 0) {
		status = -module_loaded;
		logmessage(LOGNAME, "Hardware NAT/Routing: cannot verify module state (status=%d)",
			   status);
		return status;
	}

	status = rc_hwnat_configure_policy(hwnat_allow, module_loaded == 1, wan_vid,
					   hwnat_set_wan_vid);
	if (status != 0) {
		if (wan_vid < 0)
			logmessage(LOGNAME, "Hardware NAT/Routing: retaining current WAN VID; lookup failed (status=%d)",
				   status);
		else
			logmessage(LOGNAME, "Hardware NAT/Routing: failed to set WAN VID %d (status=%d)",
				   wan_vid, status);
		return status;
	}
	if (!module_loaded) {
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
		return 0;
	}

	hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/WLAN]";
	logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
	return 0;
}
#endif /* USE_HW_NAT */

#if defined (USE_SFE)
static int
sfe_set_skip_to_bridge_ingress(int skip)
{
	return fput_int("/sys/fast_classifier/skip_to_bridge_ingress", skip);
}
#endif

void
hw_vlan_tx_map(int idx, int vid)
{
	char vlan_tx_data[16];

	/* use slots 6..10 for custom VID
	   NOTE: slots 11..15 used by hw_nat WiFi/USB offload */
	if (vid < 6 || idx < 6 || idx > 10)
		return;

	/* map VLAN VID to raeth (for support RT3883/MT7620 HW_VLAN_TX with VID > 15) */
	snprintf(vlan_tx_data, sizeof(vlan_tx_data), "%d: %d", idx, vid);

#if defined (CONFIG_RALINK_RT3883)
	fput_string("/proc/rt3883/vlan_tx", vlan_tx_data);
#elif defined (CONFIG_RALINK_MT7620)
	fput_string("/proc/mt7620/vlan_tx", vlan_tx_data);
#endif
}

int
reload_nat_modules(void)
{
	int loaded_ftp, ftp_module_loaded, ftp_reconfigure_allowed;
	int needed_ftp0 = 0;
	int needed_ftp1 = 0;
	int needed_sip = 0;
	int needed_h323 = 0;
	int needed_pptp = 0;
	int wan_nat_x = nvram_get_int("wan_nat_x");
	int reload_status = 0;
	int module_status;
	static const rc_module_load_ops_t module_ops = {
		module_smart_load, is_module_loaded_status
	};
	static const rc_module_unload_ops_t module_unload_ops = {
		module_smart_unload, is_module_loaded_status
	};
	rc_module_load_requirement_t pptp_modules[2] = {
		{ "nf_conntrack_pptp", NULL, 1 },
		{ "nf_nat_pptp", NULL, 0 }
	};
	rc_module_load_requirement_t h323_modules[2] = {
		{ "nf_conntrack_h323", NULL, 1 },
		{ "nf_nat_h323", NULL, 0 }
	};
	rc_module_load_requirement_t sip_modules[2] = {
		{ "nf_conntrack_sip", NULL, 1 },
		{ "nf_nat_sip", NULL, 0 }
	};
	rc_module_load_requirement_t ftp_modules[2] = {
		{ "nf_conntrack_ftp", NULL, 0 },
		{ "nf_nat_ftp", NULL, 0 }
	};
#if defined (USE_HW_NAT)
	char hnat_param[80];
	int hwnat_wan_vid = -1;
	int hwnat_allow = is_hwnat_allow();
	int hwnat_loaded = 0;
	int ipv6_nat = nvram_get_int("ip6_lan_auto");
#endif
#if defined (USE_SFE)
	int sfe_enable;
	int sfe_loaded;
#endif

	if (!get_ap_mode())
	{
		needed_ftp0 = nvram_get_int("nf_alg_ftp0");
		needed_ftp1 = nvram_get_int("nf_alg_ftp1");
		if (needed_ftp0 <   21 || needed_ftp0 > 65535) needed_ftp0 = 0;
		if (needed_ftp1 < 1024 || needed_ftp1 > 65535) needed_ftp1 = 0;

		if (nvram_match("nf_alg_pptp", "1"))
			needed_pptp = 1;

		if (nvram_match("nf_alg_h323", "1"))
			needed_h323 = 1;

		if (nvram_match("nf_alg_sip", "1"))
			needed_sip = 1;
	}

	pptp_modules[1].required = (wan_nat_x != 0);
	h323_modules[1].required = (wan_nat_x != 0);
	sip_modules[1].required = (wan_nat_x != 0);

#if defined (USE_HW_NAT)
	if (!hwnat_allow) {
		/* The strict unload helper handles absent, loaded and unknown states. */
		module_status = rc_module_unload_verified("hw_nat", 0,
							 &module_unload_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "Hardware NAT disable requested but hw_nat state could not be cleared (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	} else {
		hwnat_loaded = is_hwnat_loaded_status();
		if (hwnat_loaded < 0) {
			module_status = -hwnat_loaded;
			logmessage(LOGNAME, "Hardware NAT requested but module state cannot be verified (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}
#endif

	/* Configured ALGs are required; keep trying independent modules after one fails. */
	if (needed_pptp) {
		module_status = rc_module_load_requirements_apply(pptp_modules, 2, &module_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "PPTP conntrack/NAT helper load failed (status=%d)", module_status);
			reload_status = rc_first_service_start_status(reload_status, module_status);
		}
	}
	if (!needed_pptp || wan_nat_x == 0) {
		module_status = rc_module_unload_verified("nf_nat_pptp", 1,
							 &module_unload_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "PPTP NAT helper remains loaded while disabled (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}

	if (needed_h323) {
		module_status = rc_module_load_requirements_apply(h323_modules, 2, &module_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "H.323 conntrack/NAT helper load failed (status=%d)", module_status);
			reload_status = rc_first_service_start_status(reload_status, module_status);
		}
	}
	if (!needed_h323 || wan_nat_x == 0) {
		module_status = rc_module_unload_verified("nf_nat_h323", 1,
							 &module_unload_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "H.323 NAT helper remains loaded while disabled (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}

	if (needed_sip) {
		module_status = rc_module_load_requirements_apply(sip_modules, 2, &module_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "SIP conntrack/NAT helper load failed (status=%d)", module_status);
			reload_status = rc_first_service_start_status(reload_status, module_status);
		}
	}
	if (!needed_sip || wan_nat_x == 0) {
		module_status = rc_module_unload_verified("nf_nat_sip", 1,
							 &module_unload_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "SIP NAT helper remains loaded while disabled (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}

	ftp_module_loaded = is_module_loaded_status("nf_conntrack_ftp");
	ftp_reconfigure_allowed = 1;
	if (ftp_module_loaded < 0) {
		module_status = -ftp_module_loaded;
		logmessage(LOGNAME, "Cannot verify nf_conntrack_ftp module state; preserving current helpers (status=%d)",
			   module_status);
		reload_status = rc_first_service_start_status(reload_status, module_status);
		ftp_reconfigure_allowed = 0;
		loaded_ftp = 0;
	} else {
		loaded_ftp = ftp_module_loaded == 1 ?
			is_ftp_conntrack_loaded(needed_ftp0, needed_ftp1) : 0;
	}
	if (ftp_reconfigure_allowed && ftp_module_loaded == 1 && loaded_ftp == 0) {
		/* A failed parameter read is not the same as an absent module. */
		logmessage(LOGNAME, "Cannot read active nf_conntrack_ftp ports; preserving the current helper");
		reload_status = rc_first_service_start_status(reload_status, EIO);
		ftp_reconfigure_allowed = 0;
	} else if (ftp_reconfigure_allowed && loaded_ftp == 1) {
		/* Remove the NAT consumer before changing conntrack module parameters. */
		module_status = rc_module_unload_verified("nf_nat_ftp", 0,
							 &module_unload_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "Cannot reconfigure FTP helper while nf_nat_ftp is still referenced (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
			ftp_reconfigure_allowed = 0;
		}
		if (ftp_reconfigure_allowed) {
			module_status = rc_module_unload_verified("nf_conntrack_ftp", 0,
								 &module_unload_ops);
			if (module_status != 0) {
				logmessage(LOGNAME, "Cannot reconfigure FTP helper while nf_conntrack_ftp is still referenced (status=%d)",
					   module_status);
				reload_status = rc_first_service_start_status(reload_status,
									 module_status);
				ftp_reconfigure_allowed = 0;
			}
		}
		if (ftp_reconfigure_allowed)
			loaded_ftp = 0;
	}

	if (ftp_reconfigure_allowed && (needed_ftp0 || needed_ftp1)) {
		char ports_val[32];

		if (needed_ftp0 && needed_ftp1)
			snprintf(ports_val, sizeof(ports_val), "ports=%d,%d", needed_ftp0, needed_ftp1);
		else if (needed_ftp0)
			snprintf(ports_val, sizeof(ports_val), "ports=%d", needed_ftp0);
		else
			snprintf(ports_val, sizeof(ports_val), "ports=%d", needed_ftp1);

		ftp_modules[0].module_param = ports_val;
		ftp_modules[0].required = (loaded_ftp != 2);
		ftp_modules[1].required = (wan_nat_x != 0);
		module_status = rc_module_load_requirements_apply(ftp_modules, 2, &module_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "FTP conntrack/NAT helper load failed (status=%d)", module_status);
			reload_status = rc_first_service_start_status(reload_status, module_status);
		}
		if (ftp_modules[0].required &&
		    is_ftp_conntrack_loaded(needed_ftp0, needed_ftp1) != 2) {
			logmessage(LOGNAME, "nf_conntrack_ftp did not retain the requested ports");
			reload_status = rc_first_service_start_status(reload_status, EIO);
		}
	} else if (ftp_reconfigure_allowed && loaded_ftp == 2 && wan_nat_x != 0) {
		ftp_modules[1].required = 1;
		module_status = rc_module_load_requirements_apply(ftp_modules, 2, &module_ops);
		if (module_status != 0) {
			logmessage(LOGNAME, "FTP NAT helper load failed (status=%d)", module_status);
			reload_status = rc_first_service_start_status(reload_status, module_status);
		}
	}

#if defined (USE_HW_NAT)
	if (hwnat_allow)
	{
		hwnat_wan_vid = get_vlan_vid_wan();
		if (hwnat_wan_vid < 0) {
			logmessage(LOGNAME, "Hardware NAT/Routing: skipping WAN VID load/update after switch PVID failure (%d)",
				   hwnat_wan_vid);
		} else if (hwnat_loaded == 0) {
			snprintf(hnat_param, sizeof(hnat_param), "wan_vid=%d", hwnat_wan_vid);
			module_status = rc_module_load_verified("hw_nat", hnat_param,
								&module_ops);
			if (module_status == 0) {
				hwnat_loaded = 1;
			} else {
				logmessage(LOGNAME, "Hardware NAT requested but hw_nat failed to load (status=%d); continuing without hardware offload",
					   module_status);
				reload_status = rc_first_service_start_status(reload_status,
									 module_status);
				hwnat_loaded = 0;
			}
		}
#if defined (USE_MT7615_AP) || defined (USE_MT7915_AP) || defined (USE_MT76X2_AP)
		if (hwnat_loaded == 1) {
			module_status = rc_hwnat_configure_radio_policy(
					hwnat_allow, 1, nvram_get_int("hw_nat_mode"),
						ipv6_nat, IFNAME_2G_MAIN, IFNAME_5G_MAIN,
						hwnat_set_radio_registration, hwnat_set_ipv6_nat_debug);
			if (module_status != 0) {
				logmessage(LOGNAME, "Hardware NAT radio configuration incomplete (status=%d)",
					   module_status);
				reload_status = rc_first_service_start_status(reload_status,
								 module_status);
			}
		}
#endif
	}

	module_status = hwnat_configure(hwnat_wan_vid, hwnat_allow);
	if (module_status != 0)
		reload_status = rc_first_service_start_status(reload_status,
							 module_status);
#endif
#if defined (USE_SFE)
	sfe_enable = nvram_get_int("sfe_enable");
	if (sfe_enable) {
		module_status = rc_module_load_verified("fast_classifier", NULL, &module_ops);
		if (module_status == 0) {
			sfe_loaded = 1;
		} else {
			sfe_loaded = 0;
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
			logmessage(LOGNAME, "Software flow offload requested but fast_classifier failed verification (status=%d); continuing without SFE",
				   module_status);
		}
	} else {
		module_status = rc_module_unload_verified("fast_classifier", 1,
						 &module_unload_ops);
		sfe_loaded = 0;
		if (module_status != 0) {
			logmessage(LOGNAME, "Software flow offload disable requested but fast_classifier remains loaded or its state is unknown (status=%d)",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}
	if (sfe_loaded) {
		module_status = rc_sfe_configure_policy(sfe_enable,
						       sfe_set_skip_to_bridge_ingress);
		if (module_status != 0) {
			logmessage(LOGNAME, "SFE is loaded but skip_to_bridge_ingress configuration failed (status=%d); continuing without configured SFE acceleration",
				   module_status);
			reload_status = rc_first_service_start_status(reload_status,
								 module_status);
		}
	}
#endif

	if (reload_status != 0)
		errno = reload_status;
	return reload_status;
}

int
restart_firewall(void)
{
	int firewall_status, upnp_status;
	int saved_errno = 0;

	if (get_ap_mode())
		return 0;

	/* Rebuild the core ruleset, but always attempt the independent UPnP refresh. */
	firewall_status = start_firewall_ex();
	if (firewall_status != 0)
		saved_errno = errno ? errno : firewall_status;

	upnp_status = update_upnp();
	if (upnp_status != 0)
		logmessage(LOGNAME, "UPnP forward refresh incomplete after firewall restart (status=%d)",
			   upnp_status);

	if (firewall_status != 0) {
		logmessage(LOGNAME, "Firewall rule refresh incomplete (status=%d)", firewall_status);
		errno = saved_errno;
	}
	return rc_first_service_start_status(firewall_status, upnp_status);
}

static int
write_ipv4_forward_sysctl(void *context, int enabled)
{
	(void)context;
	return fput_int("/proc/sys/net/ipv4/ip_forward", enabled);
}

int
set_ipv4_forward(int is_on)
{
	return rc_ipv4_forward_apply(is_on, write_ipv4_forward_sysctl, NULL);
}

int
set_nf_conntrack(void)
{
	int i_nf_val;

#if (BOARD_RAM_SIZE < 32)
	int i_nf_lim = 4096;
#elif (BOARD_RAM_SIZE < 64)
	int i_nf_lim = 16384;
#elif (BOARD_RAM_SIZE < 128)
	int i_nf_lim = 65536;
#elif (BOARD_RAM_SIZE < 256)
	int i_nf_lim = 262144;
#else
	int i_nf_lim = 327680;
#endif

	i_nf_val = nvram_safe_get_int("nf_max_conn", 16384, 4096, i_nf_lim);
	return fput_int("/proc/sys/net/nf_conntrack_max", i_nf_val);
}

struct rc_tcp_syncookies_context {
	int enabled;
	int syncookies_status;
};

static int
write_tcp_syncookies_operation(void *opaque, unsigned int operation)
{
	struct rc_tcp_syncookies_context *context =
		(struct rc_tcp_syncookies_context *)opaque;

	switch (operation) {
	case 0:
		context->syncookies_status =
			fput_int("/proc/sys/net/ipv4/tcp_syncookies", context->enabled);
		return context->syncookies_status;
	case 1:
		return fput_int("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", 1);
	case 2:
		return fput_int("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", 1);
	default:
		return EINVAL;
	}
}

int
set_tcp_syncookies(int enabled, int *syncookies_status)
{
	struct rc_tcp_syncookies_context context;
	int status;

	context.enabled = enabled ? 1 : 0;
	context.syncookies_status = 0;
	status = rc_network_sysctl_apply(3, write_tcp_syncookies_operation, &context);
	if (syncookies_status)
		*syncookies_status = context.syncookies_status;
	return status;
}

void
set_tcp_tweaks(void)
{
	char tmp[64];

	/* Tweak TCP IPv4 performance */
	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_fin_timeout");
	fput_int(tmp, 30);		// def: 60

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_intvl");
	fput_int(tmp, 30);		// def: 75

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_probes");
	fput_int(tmp, 5);		// def: 9

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_time");
	fput_int(tmp, 1800);		// def: 7200

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_retries2");
	fput_int(tmp, 5);		// def: 15

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_syn_retries");
	fput_int(tmp, 3);		// def: 5

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_synack_retries");
	fput_int(tmp, 3);		// def: 5

//	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_tw_recycle");
//	fput_int(tmp, 1);		//def: 1

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_tw_reuse");
	fput_int(tmp, 1);		//def: 1

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_rfc1337");
	fput_int(tmp, 1);		//def: 1
}

#define PPPOE_RELAY_START_WAIT_ATTEMPTS 4

static enum process_presence
pppoe_relay_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;

	presence = process_inventory_scan("pppoe-relay", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return presence;
}

static int
pppoe_relay_confirm_started(int launch_status)
{
	int attempt;
	int wait_status;
	int scan_error = 0;
	enum process_presence presence = PROCESS_PRESENCE_ABSENT;

	if (launch_status != 0)
		return pppoe_relay_start_status(launch_status, presence);

	/* eval() waits for the launcher; require the detached relay child. */
	for (attempt = 0; attempt < PPPOE_RELAY_START_WAIT_ATTEMPTS; attempt++) {
		presence = pppoe_relay_process_presence(&scan_error);
		if (presence == PROCESS_PRESENCE_RUNNING)
			return pppoe_relay_start_status(launch_status, presence);
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			break;
		}
		if (presence != PROCESS_PRESENCE_ABSENT) {
			errno = EIO;
			break;
		}
		if (attempt + 1 < PPPOE_RELAY_START_WAIT_ATTEMPTS) {
			wait_status = service_wait_seconds(1);
			if (wait_status != 0) {
				wait_status = errno > 0 ? errno : EIO;
				logmessage(LOGNAME, "PPPoE relay readiness wait failed (status=%d)",
					   wait_status);
				errno = wait_status;
				return wait_status;
			}
		}
	}

	if (presence == PROCESS_PRESENCE_ABSENT)
		errno = ESRCH;
	return pppoe_relay_start_status(launch_status, presence);
}

int
set_passthrough_pppoe(int is_on)
{
	char* svcs[] = { "pppoe-relay", NULL };
	if (is_on && nvram_match("fw_pt_pppoe", "1")) {
		enum process_presence presence;
		int scan_error = 0;
		int start_status;

		presence = pppoe_relay_process_presence(&scan_error);
		if (pppoe_relay_should_start(presence)) {
			start_status = eval("/usr/sbin/pppoe-relay", "-C", IFNAME_BR,
					    "-S", get_man_ifname(0));
			start_status = pppoe_relay_confirm_started(start_status);
			if (start_status != 0)
				logmessage(LOGNAME, "pppoe-relay launch/readiness failed (status=%d, errno=%d)",
					   start_status, errno);
			return start_status;
		}
		if (presence == PROCESS_PRESENCE_RUNNING)
			return 0;

		if (scan_error <= 0)
			scan_error = EIO;
		errno = scan_error;
		logmessage(LOGNAME,
			   "pppoe-relay process inventory failed (%s); refusing startup",
			   strerror(scan_error));
		return scan_error;
	}

	return kill_services(svcs, 3, 1);
}

struct rc_igmp_mld_context {
	const char *ifname;
	int igmp_value;
	int mld_value;
};

static int
write_igmp_mld_operation(void *opaque, unsigned int operation)
{
	struct rc_igmp_mld_context *context =
		(struct rc_igmp_mld_context *)opaque;

	switch (operation) {
	case 0:
		return set_interface_conf_int("ipv4", "all", "force_igmp_version",
					      context->igmp_value);
	case 1:
		return set_interface_conf_int("ipv4", context->ifname,
					      "force_igmp_version", context->igmp_value);
	case 2:
		return set_interface_conf_int("ipv4", IFNAME_BR, "force_igmp_version",
					      context->igmp_value);
#if defined (USE_IPV6)
	case 3:
		return set_interface_conf_int("ipv6", "all", "force_mld_version",
					      context->mld_value);
	case 4:
		return set_interface_conf_int("ipv6", context->ifname,
					      "force_mld_version", context->mld_value);
	case 5:
		return set_interface_conf_int("ipv6", IFNAME_BR, "force_mld_version",
					      context->mld_value);
#endif
	default:
		return EINVAL;
	}
}

int
set_igmp_mld_version(void)
{
	struct rc_igmp_mld_context context;

	context.ifname = get_man_ifname(0);
	context.igmp_value = nvram_safe_get_int("force_igmp", 2, 0, 2);
#if defined (USE_IPV6)
	context.mld_value = nvram_safe_get_int("force_mld", 0, 0, 1);
	return rc_network_sysctl_apply(6, write_igmp_mld_operation, &context);
#else
	context.mld_value = 0;
	return rc_network_sysctl_apply(3, write_igmp_mld_operation, &context);
#endif
}

void
set_libc_gai(int ipv4_first)
{
	if (ipv4_first) {
		FILE *fp = fopen("/etc/gai.conf", "w");
		if (fp) {
			fprintf(fp, "precedence %s\n", "ipv4");
			fclose(fp);
		}
	} else {
		unlink("/etc/gai.conf");
	}
}

