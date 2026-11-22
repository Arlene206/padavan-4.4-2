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
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>

#include "rc.h"
#include "nvram_linux.h"
#include "ipv4_netmask_policy.h"
#include "storage_operation_lock.h"
#include "upnp_restart_policy.h"
#include "dnsmasq_file.h"
#include "dnsmasq_dhcp_range_policy.h"
#include "dnsmasq_static_host_policy.h"
#include "dnsmasq_static_host_nvram_policy.h"
#include "dnsmasq_domain_policy.h"
#include "dnsmasq_resolv_file_policy.h"
#include "inadyn_config_file.h"
#include "wan_nvram_status_policy.h"
#include "inadyn_lifecycle_lock.h"
#include "dnsmasq_lifecycle_lock.h"
#include "dnsmasq_restart_policy.h"
#include "dnsmasq_nvram_option_policy.h"
#include "dnsmasq_reload_policy.h"
#include "process_inventory.h"
#include "inadyn_daemon_policy.h"
#include "ddns_enabled_policy.h"
#include "miniupnpd_config_file.h"
#include "miniupnpd_daemon_policy.h"
#include "miniupnpd_presentation_url.h"
#include "miniupnpd_lifecycle_lock.h"
#include "wan_ifname_policy.h"
#include "service_wait_policy.h"
#include "samba_config_value_policy.h"
#if defined(APP_SMBD) || defined(APP_NMBD)
#include "samba_config_file.h"
#include "samba_daemon_policy.h"
#include "samba_process_policy.h"
#include "samba_config_transaction.h"
#include "samba_config_mode_policy.h"
#include "wan_nvram_status_policy.h"
#endif

#define DHCPD_STATIC_MAX	64
#define DHCPD_MULTIMAC_MAX	8
#define DHCPD_RANGE_DEF_TAG	"lan"
#define DHCPD_HOSTS_DIR		"/etc/dnsmasq/dhcp"
#define DHCPD_LEASE_FILE	"/tmp/dnsmasq.leases"
#define UPNPD_LEASE_FILE	"/tmp/miniupnpd.leases"
#define INADYN_USER_DIR		"/etc/storage/inadyn"

static int
service_start_wait_one_second(const char *service_name)
{
	int status;

	if (service_wait_seconds(1) == 0)
		return 0;
	status = errno ? errno : EIO;
	logmessage(LOGNAME, "%s startup wait failed (errno=%d)", service_name, status);
	errno = status;
	return status;
}


static int
simple_dhcp_range(const char *ip, char *dip1, char *dip2, const char *mask)
{
	struct in_addr ina;
	uint32_t new_start, new_end;

	if (!is_valid_ipv4(ip) || !is_valid_ipv4(mask) ||
	    !dnsmasq_dhcp_range_default_pool(ntohl(inet_addr_safe(ip)),
					     ntohl(inet_addr_safe(mask)),
					     &new_start, &new_end))
		return 0;

	ina.s_addr = htonl(new_start);
	strcpy(dip1, inet_ntoa(ina));
	ina.s_addr = htonl(new_end);
	strcpy(dip2, inet_ntoa(ina));
	return 1;
}

static int
chk_valid_subnet_pool(const char *ip, char *dip1, char *dip2, const char *mask)
{
	if (is_valid_ipv4(ip) && is_valid_ipv4(mask) &&
	    is_valid_ipv4(dip1) && is_valid_ipv4(dip2) &&
	    dnsmasq_dhcp_range_pool_is_valid(ntohl(inet_addr_safe(ip)),
					     ntohl(inet_addr_safe(mask)),
					     ntohl(inet_addr_safe(dip1)),
					     ntohl(inet_addr_safe(dip2))))
		return 0;

	return simple_dhcp_range(ip, dip1, dip2, mask) ? 1 : -1;
}

static int
arpbind_clear(void)
{
	FILE *fp;
	char buffer[256], arp_ip[16], arp_if[32];
	char *line_end;
	unsigned int arp_flags;
	int result = 0;

	fp = fopen("/proc/net/arp", "r");
	if (!fp)
		return errno ? errno : EIO;

	/* A missing, truncated or malformed header is not a complete view. */
	if (!fgets(buffer, sizeof(buffer), fp)) {
		result = ferror(fp) ? (errno ? errno : EIO) : EIO;
		goto done;
	}
	line_end = strchr(buffer, '\n');
	if (!line_end) {
		result = EIO;
		logmessage(LOGNAME,
			   "truncated /proc/net/arp header; static ARP cleanup is incomplete");
		goto done;
	}
	*line_end = '\0';
	if (!dnsmasq_static_arp_header_is_valid(buffer)) {
		result = EIO;
		logmessage(LOGNAME,
			   "malformed /proc/net/arp header; static ARP cleanup is incomplete");
		goto done;
	}

	while (fgets(buffer, sizeof(buffer), fp)) {
		int command_status, command_errno, arp_errno, parse_status;

		line_end = strchr(buffer, '\n');
		if (!line_end) {
			parse_status = EIO;
		} else {
			*line_end = '\0';
			parse_status = dnsmasq_static_arp_record_parse(
				buffer, arp_ip, sizeof(arp_ip), &arp_flags,
				arp_if, sizeof(arp_if));
		}
		if (parse_status != 0) {
			if (!result)
				result = parse_status;
			logmessage(LOGNAME,
				   "malformed or truncated /proc/net/arp row; static ARP cleanup is incomplete (status=%d)",
				   parse_status);
			break;
		}
		if (!(arp_flags & 0x04) || strcmp(arp_if, IFNAME_BR) != 0)
			continue;

		errno = 0;
		command_status = doSystem("arp -i %s -d %s", IFNAME_BR, arp_ip);
		command_errno = errno;
		arp_errno = dnsmasq_static_arp_command_errno(command_status,
							       command_errno);
		if (arp_errno) {
			if (!result)
				result = arp_errno;
			logmessage(LOGNAME,
				   "cannot remove static ARP binding %s (status=%d errno=%d)",
				   arp_ip, command_status, command_errno);
		}
	}
	if (ferror(fp) && !result)
		result = errno ? errno : EIO;

done:
	if (fclose(fp) != 0 && !result)
		result = errno ? errno : EIO;
	return result;
}

/* BusyBox arp reports success only after SIOCSARP succeeds. A failed add is
 * not included in rollback: deleting that target could destroy an older entry. */
static int
dnsmasq_static_arp_command(const char *ip, const char *mac, int add, void *context)
{
	int command_status, command_errno, arp_errno;

	(void)context;
	errno = 0;
	if (add)
		command_status = doSystem("arp -i %s -s %s %s", IFNAME_BR, ip, mac);
	else
		command_status = doSystem("arp -i %s -d %s", IFNAME_BR, ip);
	command_errno = errno;
	arp_errno = dnsmasq_static_arp_command_errno(command_status, command_errno);
	if (arp_errno)
		logmessage(LOGNAME, "cannot %s static ARP binding %s (status=%d errno=%d)",
			   add ? "install" : "remove", ip, command_status, command_errno);
	return arp_errno;
}

struct ip4_items_t {
	in_addr_t ip4;
	int count;
	char *hname;
	char *mac[DHCPD_MULTIMAC_MAX];
};

static void
free_static_ethers_items(struct ip4_items_t *ip4_list)
{
	int i, j;

	for (i = 0; i < DHCPD_STATIC_MAX; i++) {
		struct ip4_items_t *item = &ip4_list[i];

		for (j = 0; j < DHCPD_MULTIMAC_MAX; j++) {
			if (item->mac[j])
				free(item->mac[j]);
		}
		if (item->hname)
			free(item->hname);
	}
}

/*
 * Prepare every generated file beside its destination and publish each only
 * after the complete in-memory configuration has been written.  A failure
 * prevents DNSMasq from starting/restarting; existing running instances keep
 * their last loaded configuration.  The three renames are individually
 * atomic, not a cross-filesystem multi-file transaction.
 */
static int
dnsmasq_static_host_nvram_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}


static int
dnsmasq_static_host_nvram_get(void *context, const char *param_name,
			      char **value)
{
	return nvram_transaction_get((struct nvram_transaction *)context,
				     param_name, value);
}


static int
dnsmasq_static_host_nvram_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}


static int
fill_static_ethers(const char *lan_ip, const char *lan_mask)
{
	FILE *fp[3] = { NULL, NULL, NULL };
	in_addr_t ip4, ip4_lan;
	struct ip4_items_t ip4_list[DHCPD_STATIC_MAX];
	struct dnsmasq_static_host_nvram_snapshot nvram_snapshot;
	struct nvram_transaction nvram_transaction;
	int i, j, i_max_items, i_arp_bind;
	int path_length, result = 0;
	const char *arp_ips[DHCPD_STATIC_MAX];
	const char *arp_macs[DHCPD_STATIC_MAX];
	char arp_ip_text[DHCPD_STATIC_MAX][16];
	size_t arp_count = 0;
	char dh_fn[64] = {0}, dhcp_hosts_stage[96] = "";
	char dhcp_hosts_backup[128] = "";
	char ethers_stage[] = "/etc/ethers.XXXXXX";
	char ethers_backup[] = "/etc/ethers.rollback.XXXXXX";
	char hosts_static_stage[] = "/tmp/hosts.static.XXXXXX";
	char hosts_static_backup[] = "/tmp/hosts.static.rollback.XXXXXX";
	char *smac, *sip4, *shname, *copy;
	struct in_addr in;
	const char *destinations[3];
	char *staged_paths[3];
	char *backup_paths[3];

	memset(ip4_list, 0, sizeof(ip4_list));
	i_arp_bind = 0;

	/* Snapshot every configured value before opening staging files. A backend
	 * error must not replace last-known-good DHCP host/ethers configuration. */
	result = dnsmasq_static_host_nvram_snapshot_read(
		&nvram_snapshot, dnsmasq_static_host_nvram_begin,
		dnsmasq_static_host_nvram_get, dnsmasq_static_host_nvram_end,
		&nvram_transaction);
	if (result != 0) {
		logmessage(LOGNAME,
			   "cannot read static DHCP host configuration; refusing DNSMasq publication (errno=%d)",
			   result);
		errno = result;
		return result;
	}

	staged_paths[0] = dhcp_hosts_stage;
	staged_paths[1] = ethers_stage;
	staged_paths[2] = hosts_static_stage;
	backup_paths[0] = dhcp_hosts_backup;
	backup_paths[1] = ethers_backup;
	backup_paths[2] = hosts_static_backup;
	destinations[1] = "/etc/ethers";
	destinations[2] = "/tmp/hosts.static";

	mkdir_if_none(DHCPD_HOSTS_DIR, "755");
	path_length = snprintf(dh_fn, sizeof(dh_fn), "%s/dhcp-hosts.rc",
			       DHCPD_HOSTS_DIR);
	if (path_length < 0 || path_length >= (int)sizeof(dh_fn)) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	destinations[0] = dh_fn;
	path_length = snprintf(dhcp_hosts_backup, sizeof(dhcp_hosts_backup),
			       "%s.rollback.XXXXXX", dh_fn);
	if (path_length < 0 || path_length >= (int)sizeof(dhcp_hosts_backup)) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	path_length = snprintf(dhcp_hosts_stage, sizeof(dhcp_hosts_stage),
			       "%s.XXXXXX", dh_fn);
	if (path_length < 0 || path_length >= (int)sizeof(dhcp_hosts_stage)) {
		errno = ENAMETOOLONG;
		goto failed;
	}

	fp[0] = dnsmasq_file_open(dhcp_hosts_stage);
	if (!fp[0])
		goto failed;
	fp[1] = dnsmasq_file_open(ethers_stage);
	if (!fp[1])
		goto failed;
	fp[2] = dnsmasq_file_open(hosts_static_stage);
	if (!fp[2])
		goto failed;

	if (nvram_snapshot.enabled == 1) {
		ip4_lan = inet_addr_safe(lan_ip);
		i_arp_bind = nvram_snapshot.arp_enabled;
		i_max_items = (int)nvram_snapshot.item_count;

		/* First pass: allocate a complete normalized snapshot. */
		for (j = 0; j < i_max_items; j++) {
			smac = nvram_snapshot.entries[j].mac;
			if (!dnsmasq_static_mac_is_valid(smac)) {
				logmessage(LOGNAME,
					   "ignoring invalid static DHCP MAC at index %d", j);
				continue;
			}
			sip4 = nvram_snapshot.entries[j].ip;
			ip4 = inet_addr_safe(sip4);
			if (ip4 == INADDR_ANY || ip4 == INADDR_NONE || ip4 == ip4_lan)
				continue;
			if (!is_same_subnet(sip4, lan_ip, lan_mask))
				continue;
			shname = nvram_snapshot.entries[j].hostname;

			for (i = 0; i < DHCPD_STATIC_MAX; i++) {
				struct ip4_items_t *ipl = &ip4_list[i];

				if (!ipl->ip4) {
					copy = strdup(smac);
					if (!copy) {
						errno = ENOMEM;
						goto failed;
					}
					ipl->ip4 = ip4;
					ipl->count = 1;
					ipl->mac[0] = copy;
					if (is_valid_hostname(shname)) {
						copy = strdup(shname);
						if (!copy) {
							errno = ENOMEM;
							goto failed;
						}
						ipl->hname = copy;
					}
					break;
				}
				if (ipl->ip4 == ip4) {
					if (ipl->count < DHCPD_MULTIMAC_MAX) {
						copy = strdup(smac);
						if (!copy) {
							errno = ENOMEM;
							goto failed;
						}
						ipl->mac[ipl->count] = copy;
						ipl->count++;
					}
					if (!ipl->hname && is_valid_hostname(shname)) {
						copy = strdup(shname);
						if (!copy) {
							errno = ENOMEM;
							goto failed;
						}
						ipl->hname = copy;
					}
					break;
				}
			}
		}

		/* Second pass: write only to private staging files. */
		for (i = 0; i < DHCPD_STATIC_MAX; i++) {
			struct ip4_items_t *ipl = &ip4_list[i];

			if (!ipl->ip4)
				break;

			in.s_addr = ipl->ip4;
			sip4 = inet_ntoa(in);
			if (ipl->count > 0) {
				for (j = 0; j < ipl->count; j++) {
					smac = ipl->mac[j];
					if (!smac)
						continue;
					fprintf(fp[0], "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c,",
						smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
						smac[6], smac[7], smac[8], smac[9], smac[10], smac[11]);
				}
				fprintf(fp[0], "set:%s,%s\n", DHCPD_RANGE_DEF_TAG, sip4);
			}

			/* Keep /etc/ethers coherent with the same staged lease snapshot. */
			if (i_arp_bind && ipl->count == 1 && ipl->mac[0]) {
				smac = ipl->mac[0];
				fprintf(fp[1], "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c %s\n",
					smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
					smac[6], smac[7], smac[8], smac[9], smac[10], smac[11], sip4);
			}

			if (ipl->hname)
				fprintf(fp[2], "%s %s\n", sip4, sanity_hostname(ipl->hname));
		}
	}

	/* Keep DHCP hosts, ethers and named-host snapshots in one rollback set. */
	if (dnsmasq_file_publish_set(fp, staged_paths, destinations, backup_paths,
				     ARRAY_SIZE(fp), 0644) != 0) {
		result = errno ? errno : EIO;
		goto failed;
	}

	/* The set helper tries once; retry snapshots while the path buffers are
	 * live, but never undo already-published outputs for a cleanup warning. */
	for (i = 0; i < ARRAY_SIZE(backup_paths); i++) {
		if (dnsmasq_file_backup_discard(backup_paths[i]) != 0)
			logmessage(LOGNAME,
				   "cannot remove DHCP rollback snapshot '%s' after publish (errno=%d)",
				   backup_paths[i], errno ? errno : EIO);
	}

	/* Publish all generated files before kernel side effects. Keep stable
	 * IPv4 strings because inet_ntoa() reuses static storage. */
	if (i_arp_bind) {
		for (i = 0; i < DHCPD_STATIC_MAX; i++) {
			struct ip4_items_t *ipl = &ip4_list[i];

			if (!ipl->ip4)
				break;
			if (ipl->count == 1 && ipl->mac[0]) {
				in.s_addr = ipl->ip4;
				path_length = snprintf(arp_ip_text[arp_count],
						       sizeof(arp_ip_text[arp_count]),
						       "%s", inet_ntoa(in));
				if (path_length < 0 ||
				    path_length >= (int)sizeof(arp_ip_text[arp_count])) {
					result = EOVERFLOW;
					goto cleanup;
				}
				arp_ips[arp_count] = arp_ip_text[arp_count];
				arp_macs[arp_count] = ipl->mac[0];
				arp_count++;
			}
		}
		result = dnsmasq_static_arp_apply_transaction(arp_ips, arp_macs,
							      arp_count,
							      dnsmasq_static_arp_command,
							      NULL);
		if (result != 0) {
			logmessage(LOGNAME,
				   "static ARP installation failed; prior additions were rolled back where possible (errno=%d)",
				   result);
			goto cleanup;
		}
	}
	goto cleanup;

failed:
	if (!result)
		result = errno ? errno : EIO;
	for (i = 0; i < ARRAY_SIZE(fp); i++) {
		if (fp[i]) {
			(void)fclose(fp[i]);
			fp[i] = NULL;
		}
		if (dnsmasq_file_discard(staged_paths[i]) != 0)
			logmessage(LOGNAME, "cannot remove staged static DHCP file '%s' (errno=%d)",
				   staged_paths[i], errno ? errno : EIO);
		if (backup_paths[i] && backup_paths[i][0])
			logmessage(LOGNAME, "retaining DHCP rollback snapshot '%s' for recovery",
				   backup_paths[i]);
	}

cleanup:
	free_static_ethers_items(ip4_list);
	if (result != 0) {
		logmessage(LOGNAME,
			   "cannot complete static DHCP host/ethers preparation; refusing DNSMasq update (errno=%d)",
			   result);
		errno = result;
		return result;
	}
	return 0;
}

int
fill_dnsmasq_servers(void)
{
	FILE *fp;
	int lock, saved_errno;
	char word[256], *next, *wan_dns, *wan_dom;
	char servers_path[] = "/tmp/dnsmasq.servers.XXXXXX";
	const char *storage_dir = "/etc/storage/dnsmasq";

	lock = file_lock("dservers");
	if (lock < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNS server snapshot lock; refusing publication (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	fp = dnsmasq_file_open(servers_path);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(servers_path) != 0)
			logmessage(LOGNAME, "cannot remove DNS server snapshot stage '%s' after open failure (errno=%d)",
				   servers_path, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot stage DNS server snapshot (errno=%d)", saved_errno);
		file_unlock(lock);
		errno = saved_errno;
		return -1;
	}

	/* add DNS servers (via specific domain) for static VPN client */
	if (nvram_get_int("vpnc_pdns") > 0) {
		wan_dom = nvram_safe_get("vpnc_dom_t");
		wan_dns = nvram_safe_get("vpnc_dns_t");
		if (dnsmasq_valid_server_domain(wan_dom) && strlen(wan_dns) > 6) {
			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word))
					fprintf(fp, "server=/%s/%s\n", wan_dom, word);
			}
		} else if (wan_dom[0]) {
			logmessage(LOGNAME, "ignoring invalid domain-specific DNS routing domain");
		}
	}

	/* add DNS servers (via specific domain) for MAN subnet */
	wan_dom = nvram_safe_get("wanx_domain");
	wan_dns = nvram_safe_get("wanx_dns");
	if (dnsmasq_valid_server_domain(wan_dom) && strlen(wan_dns) > 6) {
		foreach(word, wan_dns, next) {
			if (is_valid_ipv4(word))
				fprintf(fp, "server=/%s/%s\n", wan_dom, word);
		}
	} else if (wan_dom[0]) {
		logmessage(LOGNAME, "ignoring invalid domain-specific DNS routing domain");
	}

	/* fill from user dnsmasq.servers */
	load_user_config(fp, storage_dir, "dnsmasq.servers", NULL);

	if (dnsmasq_file_publish(fp, servers_path, DNS_SERVERS_FILE, 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(servers_path) != 0)
			logmessage(LOGNAME, "cannot remove failed DNS server snapshot stage '%s' after publish failure (errno=%d)",
				   servers_path, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot publish DNS server snapshot (errno=%d)", saved_errno);
		file_unlock(lock);
		errno = saved_errno;
		return -1;
	}

	file_unlock(lock);

	return 0;
}

///////////////////////////////////////////////////////////////////////
// dnsmasq
///////////////////////////////////////////////////////////////////////

int
is_dns_dhcpd_run(void)
{
	return pids("dnsmasq");
}

#define DNSMASQ_START_WAIT_ATTEMPTS	4

static enum process_presence
dnsmasq_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;

	presence = process_inventory_scan("dnsmasq", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return presence;
}

/* Public status contract for callers outside services_ex.c:
 * 1 = RUNNING, 0 = ABSENT, -1 = UNKNOWN (errno and scan_error preserved). */
int
dnsmasq_process_status(int *scan_error)
{
	enum process_presence presence;
	int status;

	presence = dnsmasq_process_presence(scan_error);
	status = dnsmasq_reload_notification_decision(presence);
	if (status < 0)
		errno = (scan_error && *scan_error > 0) ? *scan_error : EIO;
	return status;
}

static int
dnsmasq_require_absent(const char *stage)
{
	int scan_error = 0;
	int status;
	enum process_presence presence;

	presence = dnsmasq_process_presence(&scan_error);
	status = dnsmasq_start_policy_preflight(presence);
	if (presence == PROCESS_PRESENCE_UNKNOWN) {
		int saved_error = scan_error ? scan_error : EIO;
		logmessage(LOGNAME,
			   "cannot verify DNSMasq process state %s; refusing start (scan_error=%d)",
			   stage, saved_error);
		return saved_error;
	}
	if (presence == PROCESS_PRESENCE_RUNNING) {
		logmessage(LOGNAME,
			   "refusing DNSMasq start %s while a previous instance is active",
			   stage);
	}
	return status;
}

static int
dnsmasq_confirm_started(int launch_status)
{
	int attempt;
	int scan_error = 0;
	enum process_presence presence = PROCESS_PRESENCE_ABSENT;

	if (launch_status != 0)
		return dnsmasq_start_policy_status(launch_status,
					   PROCESS_PRESENCE_ABSENT);

	/* dnsmasq daemonizes: eval() waits for its launcher, not the daemon child.
	 * Only a complete RUNNING inventory proves that the child is available. */
	for (attempt = 0; attempt < DNSMASQ_START_WAIT_ATTEMPTS; attempt++) {
		presence = dnsmasq_process_presence(&scan_error);
		if (presence == PROCESS_PRESENCE_RUNNING)
			return dnsmasq_start_policy_status(launch_status, presence);
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			logmessage(LOGNAME,
				   "cannot verify DNSMasq startup postcondition (scan_error=%d)",
				   scan_error ? scan_error : EIO);
			break;
		}
		if (attempt + 1 < DNSMASQ_START_WAIT_ATTEMPTS) {
			int wait_status = service_start_wait_one_second("DNSMasq");
			if (wait_status != 0)
				return wait_status;
		}
	}

	return dnsmasq_start_policy_status(launch_status, presence);
}

static int
dnsmasq_read_boolean_option(const char *param_name, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !enabled) {
		errno = EINVAL;
		return EINVAL;
	}

	/* Compare the checked-get cache value before making another NVRAM call. */
	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = dnsmasq_nvram_option_status(api_result, saved_errno,
					     value, 0, enabled);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read DNSMasq option '%s'; preserving last-known-good configuration (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

static int
dnsmasq_read_integer_range_option(const char *param_name, int missing_value,
				  int minimum, int maximum, int *value)
{
	char *text = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !value || minimum > maximum) {
		errno = EINVAL;
		return EINVAL;
	}
	errno = 0;
	api_result = nvram_get_checked(param_name, &text);
	saved_errno = errno;
	status = dnsmasq_nvram_integer_range_status(
		api_result, saved_errno, text, missing_value,
		minimum, maximum, value);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read or validate DNSMasq integer option '%s'; preserving last-known-good configuration (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

#if defined(APP_SMBD) || defined(APP_NMBD)
static int
dnsmasq_read_integer_boolean_option(const char *param_name, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !enabled) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = dnsmasq_nvram_integer_boolean_status(api_result, saved_errno,
						       value, 0, enabled);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read integer DNSMasq option '%s'; preserving last-known-good configuration (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}
#endif

static int
dnsmasq_read_string_option(const char *param_name, const char *missing_value,
			   char *destination, size_t destination_size)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !destination || destination_size == 0) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = dnsmasq_nvram_string_status(api_result, saved_errno, value,
					     missing_value, destination,
					     destination_size);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read DNSMasq string option '%s'; preserving last-known-good configuration (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

static int
dnsmasq_read_invmatch_option(const char *param_name, const char *match_value,
			     int missing_matches, int *matches)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !match_value || !matches ||
	    (missing_matches != 0 && missing_matches != 1)) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = dnsmasq_nvram_invmatch_status(api_result, saved_errno,
					       value, match_value,
					       missing_matches, matches);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read DNSMasq inverse-match option '%s'; preserving last-known-good configuration (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

int
is_dhcpd_enabled_status(int is_ap_mode, int *enabled)
{
	int lan_dhcp_enabled = 0;
	int lan_proto_not_dhcp = 0;
	int status;

	if (!enabled) {
		errno = EINVAL;
		return EINVAL;
	}
	*enabled = 0;

	if (!is_ap_mode)
		return dnsmasq_read_boolean_option("dhcp_enable_x", enabled);

	status = dnsmasq_read_boolean_option("lan_dhcpd_x", &lan_dhcp_enabled);
	if (status != 0)
		return status;
	if (!lan_dhcp_enabled)
		return 0;

	/*
	 * Preserve nvram_invmatch() semantics: a missing lan_proto_x key is
	 * false, just like nvram_get() returning NULL in the original helper.
	 */
	status = dnsmasq_read_invmatch_option("lan_proto_x", "1", 0,
					      &lan_proto_not_dhcp);
	if (status != 0)
		return status;

	*enabled = lan_proto_not_dhcp;
	return 0;
}

/* Keep the original exported boolean entry point for ABI compatibility. */
int
is_dhcpd_enabled(int is_ap_mode)
{
	int enabled = 0;
	int status;

	status = is_dhcpd_enabled_status(is_ap_mode, &enabled);
	if (status != 0) {
		errno = status;
		return 0;
	}
	return enabled;
}

static int
prepare_dns_dhcpd(int is_ap_mode, int *service_enabled)
{
	FILE *fp;
	int i_verbose, i_dhcp_enable, is_dhcp_used, is_dns_used;
	int dhcp_lease = 0;
	int dhcp_filter_aaaa_enabled = 0, dhcp_all_servers_enabled = 0;
	int dhcp_strict_order_enabled = 0, dhcp_stop_dns_rebind_enabled = 0;
	int dhcp_proxy_dnssec_enabled = 0, option_status;
#if defined(APP_SMBD) || defined(APP_NMBD)
	int wins_enabled = 0;
#endif
	int saved_errno, validation_errno, pool_status, pool_rollback_failed;
	int resolv_create_status, resolv_status;
	char config_path[] = "/etc/dnsmasq.conf.XXXXXX";
	char dhcp_start[32], dhcp_end[32], old_dhcp_start[32], old_dhcp_end[32];
	char dns_all[64], dnsv6[40];
	char *ipaddr, *netmask, *gw, *dns1, *dns2, *dns3, *wins, *domain, *dns6, *dns61;
	struct dnsmasq_dhcp_config_nvram_snapshot dhcp_nvram_snapshot;
	struct nvram_transaction dhcp_nvram_transaction;
#if defined(USE_IPV6)
	int radv_enabled = 0, dhcp6_mode = 0, dhcp6_irt = 600;
	int dhcp6_status;
	struct dnsmasq_dhcp6_nvram_snapshot dhcp6_snapshot;
#endif
#if defined(APP_VLMCSD)
	int vlmcsd_mode = 0;
#endif
	const char *storage_dir = "/etc/storage/dnsmasq";

	if (!service_enabled) {
		errno = EINVAL;
		return EINVAL;
	}
	*service_enabled = 0;

	/*
	 * Read every optional resolver policy before touching generated hosts,
	 * resolver or dnsmasq configuration files. A failed NVRAM read must not
	 * silently publish a configuration with security options omitted.
	 */
	if (!is_ap_mode) {
		option_status = dnsmasq_read_boolean_option(
			"dhcp_filter_aaaa", &dhcp_filter_aaaa_enabled);
		if (option_status != 0)
			return option_status;
		option_status = dnsmasq_read_boolean_option(
			"dhcp_all_servers", &dhcp_all_servers_enabled);
		if (option_status != 0)
			return option_status;
		option_status = dnsmasq_read_boolean_option(
			"dhcp_strict_order", &dhcp_strict_order_enabled);
		if (option_status != 0)
			return option_status;
		option_status = dnsmasq_read_boolean_option(
			"dhcp_stop_dns_rebind", &dhcp_stop_dns_rebind_enabled);
		if (option_status != 0)
			return option_status;
		option_status = dnsmasq_read_boolean_option(
			"dhcp_proxy_dnssec", &dhcp_proxy_dnssec_enabled);
		if (option_status != 0)
			return option_status;
	}

	option_status = is_dhcpd_enabled_status(is_ap_mode, &i_dhcp_enable);
	if (option_status != 0)
		return option_status;
	if (i_dhcp_enable) {
		/* The DHCP UI permits lease durations from 120 to 604800 seconds.
		 * Validate the setting before touching any generated files. */
		option_status = dnsmasq_read_integer_range_option(
			"dhcp_lease", 0, 120, 604800, &dhcp_lease);
		if (option_status != 0)
			return option_status;

	}
	option_status = dnsmasq_read_integer_range_option(
		"dhcp_verbose", 0, 0, 3, &i_verbose);
	if (option_status != 0)
		return option_status;

#if defined(USE_IPV6)
	/* Check RA/DHCPv6 state and snapshot every DHCPv6 option before staging. */
	if (!is_ap_mode) {
		dhcp6_status = get_lan_radv_enabled_status(&radv_enabled);
		if (dhcp6_status != 0) {
			logmessage(LOGNAME,
				   "cannot determine LAN router-advertisement state; preserving last-known-good DNSMasq config (status=%d)",
				   dhcp6_status);
			errno = dhcp6_status;
			return dhcp6_status;
		}
		if (radv_enabled) {
			dhcp6_status = get_lan_dhcp6s_mode_status(&dhcp6_mode);
			if (dhcp6_status == 0 && dhcp6_mode > 0)
				dhcp6_status = get_lan_dhcp6s_irt_status(&dhcp6_irt);
			if (dhcp6_status == 0)
				dhcp6_status = dnsmasq_dhcp6_nvram_snapshot_read(
					&dhcp6_snapshot, dhcp6_mode,
					dnsmasq_static_host_nvram_begin,
					dnsmasq_static_host_nvram_get,
					dnsmasq_static_host_nvram_end,
					&dhcp_nvram_transaction);
			if (dhcp6_status != 0) {
				logmessage(LOGNAME,
					   "cannot read/validate DHCPv6 configuration snapshot; preserving last-known-good DNSMasq config (status=%d)",
					   dhcp6_status);
				errno = dhcp6_status;
				return dhcp6_status;
			}
		}
	}
#endif

	/* Snapshot every LAN/IPv4-DHCP string before generated files are touched. */
	option_status = dnsmasq_dhcp_config_nvram_snapshot_read(
		&dhcp_nvram_snapshot, i_dhcp_enable,
		dnsmasq_static_host_nvram_begin, dnsmasq_static_host_nvram_get,
		dnsmasq_static_host_nvram_end, &dhcp_nvram_transaction);
	if (option_status != 0) {
		logmessage(LOGNAME,
			   "cannot read DNSMasq LAN/DHCP configuration snapshot; preserving last-known-good files (errno=%d)",
			   option_status);
		errno = option_status;
		return option_status;
	}

	ipaddr = dhcp_nvram_snapshot.lan_ipaddr;
	netmask = dhcp_nvram_snapshot.lan_netmask;
	domain = dhcp_nvram_snapshot.lan_domain;
	wins = dhcp_nvram_snapshot.dhcp_wins;
#if defined(APP_SMBD) || defined(APP_NMBD)
	if (i_dhcp_enable && !is_valid_ipv4(wins)) {
		option_status = dnsmasq_read_integer_boolean_option("wins_enable",
								    &wins_enabled);
		if (option_status != 0)
			return option_status;
	}
#endif
#if defined(APP_VLMCSD)
	if (i_dhcp_enable) {
		option_status = dnsmasq_read_integer_range_option(
			"vlmcsd_enable", 0, INT_MIN, INT_MAX, &vlmcsd_mode);
		if (option_status != 0)
			return option_status;
	}
#endif

	/* This value is emitted as a raw dnsmasq directive in both router and
	 * AP mode. Reject control characters and option separators before any
	 * generated host/ARP file is changed. */
	if (!domain || (*domain && !dnsmasq_valid_server_domain(domain))) {
		logmessage(LOGNAME, "refusing invalid DNSMasq LAN domain");
		errno = EINVAL;
		return EINVAL;
	}

	/* Validate the DHCP pool before changing generated host/ARP files. */
	if (i_dhcp_enable) {
		snprintf(dhcp_start, sizeof(dhcp_start), "%s", dhcp_nvram_snapshot.dhcp_start);
		snprintf(dhcp_end, sizeof(dhcp_end), "%s", dhcp_nvram_snapshot.dhcp_end);
		snprintf(old_dhcp_start, sizeof(old_dhcp_start), "%s", dhcp_start);
		snprintf(old_dhcp_end, sizeof(old_dhcp_end), "%s", dhcp_end);
		pool_status = chk_valid_subnet_pool(ipaddr, dhcp_start, dhcp_end, netmask);
		if (pool_status < 0) {
			logmessage(LOGNAME, "refusing DNSMasq DHCP configuration with invalid LAN IPv4 subnet");
			errno = EINVAL;
			return EINVAL;
		}
		if (pool_status > 0) {
			pool_rollback_failed = 0;
			if (dnsmasq_dhcp_range_store_nvram_pair(old_dhcp_start, old_dhcp_end,
							 dhcp_start, dhcp_end, nvram_set,
							 &pool_rollback_failed) != 0) {
				saved_errno = errno ? errno : EIO;
					if (pool_rollback_failed)
						logmessage(LOGNAME, "DHCP pool NVRAM update failed and its rollback was incomplete (errno=%d)", saved_errno);
					else
						logmessage(LOGNAME, "cannot persist corrected DHCP pool; refusing DNSMasq preparation (errno=%d)", saved_errno);
					errno = saved_errno;
					return saved_errno;
			}
			logmessage(LOGNAME, "adjusted DHCP pool to exclude the router LAN address");
		}
	}

	/* Stage DHCP hosts and ARP inputs before allowing DNSMasq to start.
	 * A failed output keeps the previous daemon/configuration in service. */
	if (fill_static_ethers(ipaddr, netmask) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot publish static DHCP host inputs; refusing DNSMasq preparation (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	if (is_ap_mode) {
		/* start_lan() populates the temporary AP lease fields before DNSMasq
		 * startup. Re-publish here as a fail-closed preparation gate so an
		 * earlier best-effort hostname write cannot leave stale host data. */
		if (update_hosts_ap() != 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "cannot publish AP hostname files; refusing DNSMasq preparation (errno=%d)",
				   saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
	} else {
		/* create /etc/hosts (run after fill_static_ethers!) */
		if (update_hosts_router(ipaddr) != 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "cannot publish router hostname files; refusing DNSMasq preparation (errno=%d)",
				   saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		
		/*
		 * DNSMasq is configured with resolv-file=DNS_RESOLV_CONF. Do not
		 * publish or launch a configuration whose resolver input could not
		 * be created; otherwise startup may appear successful while every
		 * forwarded lookup has no usable upstream snapshot.
		 */
		errno = 0;
		resolv_create_status = create_file(DNS_RESOLV_CONF);
		resolv_status = dnsmasq_resolv_file_error(resolv_create_status, errno);
		if (resolv_status != 0) {
			logmessage(LOGNAME,
				   "cannot prepare DNSMasq resolver file '%s'; refusing startup (errno=%d)",
				   DNS_RESOLV_CONF, resolv_status);
			errno = resolv_status;
			return resolv_status;
		}
	}

	/* Stage a complete configuration beside the live file.  A failed write,
	 * flush or close must not destroy the last-known-good dnsmasq.conf. */
	fp = dnsmasq_file_open(config_path);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(config_path) != 0)
			logmessage(LOGNAME, "cannot remove DNSMasq config stage '%s' after open failure (errno=%d)",
				   config_path, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot stage DNSMasq configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	fprintf(fp, "user=%s\n"
		    "resolv-file=%s\n"
		    "no-poll\n"
		    "bogus-priv\n"
		    "no-negcache\n"
		    "clear-on-reload\n"
		    "bind-dynamic\n"
		    "interface=%s\n",
		    SYS_USER_NOBODY,
		    DNS_RESOLV_CONF,
		    IFNAME_BR);

	if (!is_ap_mode) {
		/* listen DNS queries from clients of VPN server */
		fprintf(fp, "listen-address=%s\n", ipaddr);
	}
	if (!is_ap_mode && dhcp_filter_aaaa_enabled) {
		/* Don't include IPv6 addresses in DNS answers */
		fprintf(fp, "filter-AAAA\n");
	}
	if (!is_ap_mode && dhcp_all_servers_enabled) {
		/* DNS queries for all servers */
		fprintf(fp, "all-servers\n");
	}
	if (!is_ap_mode && dhcp_strict_order_enabled) {
		/* Name servers strictly in the order listed */
		fprintf(fp, "strict-order\n");
	}
	if (!is_ap_mode && dhcp_stop_dns_rebind_enabled) {
		/* Stop DNS rebinding and Allows upstream 127.0.0.0/8 responses */
		fprintf(fp, "rebind-localhost-ok\n" "stop-dns-rebind\n");
	}
	if (!is_ap_mode && dhcp_proxy_dnssec_enabled) {
		/* Proxy DNSSEC validation results from upstream nameservers */
		fprintf(fp, "proxy-dnssec\n");
	}
	if (!is_ap_mode) {
		is_dns_used = 1;
		fprintf(fp, "min-port=%d\n", 4096);
		fprintf(fp, "cache-size=%d\n", DNS_RELAY_CACHE_MAX);
		//remove it for overwrite // simonchen - 2023-2-16
		//fprintf(fp, "dns-forward-max=%d\n", DNS_RELAY_QUERIES_MAX);
		fprintf(fp, "addn-hosts=%s/hosts\n", storage_dir);
		fprintf(fp, "servers-file=%s\n", DNS_SERVERS_FILE);
		fprintf(fp, "dhcp-hostsfile=%s/dhcp.conf\n", storage_dir);
	} else {
		is_dns_used = 0;
		fprintf(fp, "cache-size=%d\n", 0);
		fprintf(fp, "port=%d\n", 0);
	}

	if (strlen(domain) > 0) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", domain);
	}

	is_dhcp_used = 0;
	if (i_dhcp_enable) {
		fprintf(fp, "dhcp-range=set:%s,%s,%s,%s,%d\n",
			DHCPD_RANGE_DEF_TAG, dhcp_start, dhcp_end, netmask, dhcp_lease);
		
		/* GATEWAY */
		gw = dhcp_nvram_snapshot.dhcp_gateway;
		if (!is_valid_ipv4(gw))
			gw = (!is_ap_mode) ? ipaddr : NULL;
		if (gw)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 3, gw);
		
		/* DNS server */
		memset(dns_all, 0, sizeof(dns_all));
		dns1 = dhcp_nvram_snapshot.dhcp_dns1;
		dns2 = dhcp_nvram_snapshot.dhcp_dns2;
		dns3 = dhcp_nvram_snapshot.dhcp_dns3;
		
		if (is_valid_ipv4(dns1))
			strcat(dns_all, dns1);
		if (is_valid_ipv4(dns2) && (strcmp(dns2, dns1))) {
			if (strlen(dns_all) > 0)
				strcat(dns_all, ",");
			strcat(dns_all, dns2);
		}
		if (is_valid_ipv4(dns3) && (strcmp(dns3, dns1) && strcmp(dns3, dns2))) {
			if (strlen(dns_all) > 0)
				strcat(dns_all, ",");
			strcat(dns_all, dns3);
		}
		if (strlen(dns_all) == 0 && !is_ap_mode)
			strcat(dns_all, ipaddr);
		if (strlen(dns_all) > 0)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 6, dns_all);
		
		/* DOMAIN search */
		if (strlen(domain) > 0)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 15, domain);
		
		/* WINS */
		if (is_valid_ipv4(wins))
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 44, wins);
#if defined(APP_SMBD) || defined(APP_NMBD)
		else if (wins_enabled)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 44, ipaddr);
#endif
#if defined(APP_VLMCSD)
		if (vlmcsd_mode == 1)	
			fprintf(fp, "srv-host=%s.%s,%s,%d\n", "_VLMCS", "_tcp", ipaddr, 1688);
#endif
		if (i_verbose == 0 || i_verbose == 2)
			fprintf(fp, "quiet-dhcp\n");
		
		is_dhcp_used |= DNSMASQ_DHCP_FEATURE_LEASES;
	}

#if defined (USE_IPV6)
	if (radv_enabled) {
		
		fprintf(fp, "enable-ra\n");
		if (i_verbose == 0 || i_verbose == 1)
			fprintf(fp, "quiet-ra\n");
		fprintf(fp, "ra-param=%s,%d,%d\n", IFNAME_BR, 30, 1800);
		
		is_dhcp_used |= DNSMASQ_DHCP_FEATURE_RA;
		
		if (dhcp6_mode == 0) {
			int i_pref_lifetime = 600;
			
			if (is_lan_addr6_static() == 1)
				i_pref_lifetime = 1800;
			
			/* Router Advertisement only, disable Stateful, disable SLAAC */
			fprintf(fp, "dhcp-range=set:%s,::,constructor:%s%s,%d,%d\n",
				DHCPD_RANGE_DEF_TAG, IFNAME_BR, ",ra-only,ra-names", 64, i_pref_lifetime);
		} else {
			/* DHCPv6 information-refresh time was checked before config staging. */
			
			if (dhcp6_mode == 1) {
				fprintf(fp, "dhcp-range=set:%s,::,constructor:%s%s,%d,%d\n",
					DHCPD_RANGE_DEF_TAG, IFNAME_BR, ",ra-stateless,ra-names", 64, dhcp6_irt);
			} else {
				const char *range_mode = "";
				int i_sflt = dhcp6_snapshot.valid_lifetime;
				int i_sfps = dhcp6_snapshot.start_port;
				int i_sfpe = dhcp6_snapshot.end_port;
				
				if (i_sfpe < i_sfps)
					i_sfpe = i_sfps;
				
				if (dhcp6_mode > 2)
					range_mode = ",slaac,ra-names";
				
				/* Enable Stateful, Enable/Disable SLAAC */
				fprintf(fp, "dhcp-range=set:%s,::%x,::%x,constructor:%s%s,%d,%d\n",
					DHCPD_RANGE_DEF_TAG, i_sfps, i_sfpe, IFNAME_BR, range_mode, 64, i_sflt);
			}
			
			/* DNS server */
			memset(dnsv6, 0, sizeof(dnsv6));
			dns6 = dhcp6_snapshot.dns_server;
			if (is_valid_ipv6(dns6)) {
				strcpy(dnsv6, dns6);
				fprintf(fp, "dhcp-option=tag:%s,option6:%d,%s\n", DHCPD_RANGE_DEF_TAG, 23, dnsv6);
			} else {
				// strcpy(dnsv6, "[::]");
				fprintf(fp, "dhcp-option=tag:%s,option6:%d\n", DHCPD_RANGE_DEF_TAG, 23);
			}
			
			/* DOMAIN search */
			if (strlen(domain) > 0)
				fprintf(fp, "dhcp-option=tag:%s,option6:%d,%s\n", DHCPD_RANGE_DEF_TAG, 24, domain);
			
			/* Information Refresh Time */
			fprintf(fp, "dhcp-option=tag:%s,option6:%d,%d\n", DHCPD_RANGE_DEF_TAG, 32, dhcp6_irt);
			
			if (i_verbose == 0 || i_verbose == 1)
				fprintf(fp, "quiet-dhcp6\n");
			
			is_dhcp_used |= DNSMASQ_DHCP_FEATURE_LEASES;
		}
	}
#endif

	if (dnsmasq_lease_file_required(is_dhcp_used)) {
		/* Only validate/create the lease path when this configuration will
		 * actually make dnsmasq open it. DNS-only and RA-only modes do not. */
		if (dnsmasq_lease_file_ensure(DHCPD_LEASE_FILE) != 0) {
			saved_errno = errno ? errno : EIO;
			if (fclose(fp) != 0)
				logmessage(LOGNAME, "error closing DNSMasq staging file after lease-path rejection (errno=%d)", errno ? errno : EIO);
			if (dnsmasq_file_discard(config_path) != 0)
				logmessage(LOGNAME, "cannot remove rejected DNSMasq config stage '%s' (errno=%d)",
					   config_path, errno ? errno : EIO);
			logmessage(LOGNAME, "refusing DNSMasq lease configuration with unsafe lease file (errno=%d)", saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		fprintf(fp, "dhcp-hostsfile=%s\n", DHCPD_HOSTS_DIR);
		fprintf(fp, "dhcp-leasefile=%s\n", DHCPD_LEASE_FILE);
		fprintf(fp, "dhcp-authoritative\n");
	}

#if (BOARD_NUM_USB_PORTS > 0)
	fprintf(fp, "tftp-no-fail\n");
#endif

	fprintf(fp, "conf-file=%s/dnsmasq.conf\n", storage_dir);

	/* Validate the DNS upstream snapshot before publishing its matching main
	 * configuration.  Fatal preparation failures leave the active daemon and
	 * its last-known-good main configuration untouched. */
	if (is_dns_used && fill_dnsmasq_servers() != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_is_safe_regular(DNS_SERVERS_FILE) != 0) {
			validation_errno = errno ? errno : EPERM;
			if (fclose(fp) != 0)
				logmessage(LOGNAME, "error closing rejected DNSMasq staging file (errno=%d)", errno ? errno : EIO);
			if (dnsmasq_file_discard(config_path) != 0)
				logmessage(LOGNAME, "cannot remove rejected DNSMasq config stage '%s' (errno=%d)",
					   config_path, errno ? errno : EIO);
			logmessage(LOGNAME, "refusing DNSMasq start with unsafe DNS server snapshot (errno=%d)", validation_errno);
			return validation_errno;
		}
		logmessage(LOGNAME, "DNS server snapshot refresh failed; retaining last-known-good file (errno=%d)", saved_errno);
	}

	if (dnsmasq_file_publish(fp, config_path, "/etc/dnsmasq.conf", 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(config_path) != 0)
			logmessage(LOGNAME, "cannot remove failed DNSMasq config stage '%s' after publish failure (errno=%d)",
				   config_path, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot publish DNSMasq configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	*service_enabled = (is_dns_used || is_dhcp_used);
	return 0;
}

/*
 * The *_locked entry points are for callers already holding the DNSMasq
 * lifecycle lock (currently full_restart_lan). Public RC APIs acquire the
 * cross-process lock before process checks, config publication, stop, launch
 * or cache reload.
 */
static int
dnsmasq_lifecycle_lock_finish(int lock_fd, int operation_status, const char *operation)
{
	int lock_errno;
	if (dnsmasq_lifecycle_lock_release(lock_fd) == 0)
		return operation_status;
	lock_errno = errno ? errno : EIO;
	logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock after %s (errno=%d)",
		   operation, lock_errno);
	return operation_status != 0 ? operation_status : lock_errno;
}

static int dnsmasq_rollback_failed_start(int start_status, const char *operation);

int
start_dns_dhcpd_locked(int is_ap_mode)
{
	int service_enabled = 0;
	int status;
	int launch_status;
	status = dnsmasq_require_absent("before configuration preparation");
	if (status != 0)
		return status;
	status = prepare_dns_dhcpd(is_ap_mode, &service_enabled);
	if (status != 0 || !service_enabled)
		return status;
	/* Retain a second complete scan against launchers outside this lock. */
	status = dnsmasq_require_absent("after configuration preparation");
	if (status != 0)
		return status;
	errno = 0;
	launch_status = eval("/usr/sbin/dnsmasq");
	status = dnsmasq_confirm_started(launch_status);
	if (status != 0) {
		if (launch_status != 0)
			logmessage(LOGNAME, "cannot launch DNSMasq after configuration publication (status=%d)", launch_status);
		else
			logmessage(LOGNAME, "DNSMasq launch returned success but no running process appeared within %d seconds",
				   DNSMASQ_START_WAIT_ATTEMPTS - 1);
		status = dnsmasq_rollback_failed_start(status, "start");
	}
	return status;
}

int
start_dns_dhcpd(int is_ap_mode)
{
	int lock_fd, status, lock_errno;
	lock_fd = dnsmasq_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNSMasq lifecycle lock; refusing start (errno=%d)", lock_errno);
		return lock_errno;
	}
	status = start_dns_dhcpd_locked(is_ap_mode);
	return dnsmasq_lifecycle_lock_finish(lock_fd, status, "start");
}

int
stop_dns_dhcpd_locked(void)
{
	char* svcs[] = { "dnsmasq", NULL };
	int status, arp_status;

	status = kill_services(svcs, 3, 1);
	if (dnsmasq_stop_policy_should_clear_arp(status)) {
		arp_status = arpbind_clear();
		if (arp_status != 0)
			logmessage(LOGNAME,
				   "DNSMasq stopped but static ARP cleanup was incomplete (errno=%d)",
				   arp_status);
		status = dnsmasq_stop_policy_cleanup_status(status, arp_status);
	} else {
		logmessage(LOGNAME, "DNSMasq did not stop cleanly; retaining static ARP bindings");
	}
	return status;
}

static int
dnsmasq_rollback_failed_start(int start_status, const char *operation)
{
	int start_errno;
	int rollback_status;
	int scan_error = 0;
	enum process_presence presence;

	start_errno = start_status > 0 ? start_status :
		(errno > 0 ? errno : EIO);
	rollback_status = stop_dns_dhcpd_locked();
	presence = dnsmasq_process_presence(&scan_error);
	if (dnsmasq_failed_start_may_leave_daemon(presence)) {
		logmessage(LOGNAME,
			   "DNSMasq may remain active after failed %s (start_status=%d, rollback_status=%d, state=%d, scan_error=%d)",
			   operation, start_status, rollback_status, presence,
			   scan_error);
	} else if (rollback_status != 0) {
		logmessage(LOGNAME,
			   "DNSMasq is absent after failed %s rollback but stop reported status=%d",
			   operation, rollback_status);
	}
	/* Keep stop/ARP cleanup diagnostics from replacing the startup failure. */
	errno = start_errno;
	return start_status;
}

int
stop_dns_dhcpd(void)
{
	int lock_fd, status, lock_errno;
	lock_fd = dnsmasq_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNSMasq lifecycle lock; refusing stop (errno=%d)", lock_errno);
		return lock_errno;
	}
	status = stop_dns_dhcpd_locked();
	return dnsmasq_lifecycle_lock_finish(lock_fd, status, "stop");
}

static int
restart_dhcpd_locked(void)
{
	int was_running;
	int service_enabled = 0;
	int is_ap_mode, ap_mode_status;
	int prepare_status;
	int stop_status;
	int stop_postcondition;
	int scan_error = 0;
	int launch_status;
	int start_status;
	enum process_presence initial_presence;
	enum process_presence final_presence;
	enum dnsmasq_restart_action action;

	/* Do not publish files or normalize NVRAM from an unknown process state. */
	initial_presence = dnsmasq_process_presence(&scan_error);
	if (initial_presence == PROCESS_PRESENCE_UNKNOWN) {
		int saved_error = scan_error ? scan_error : EIO;
		logmessage(LOGNAME,
			   "cannot verify DNSMasq state before restart preparation; refusing mutation (scan_error=%d)",
			   saved_error);
		return saved_error;
	}
	was_running = (initial_presence == PROCESS_PRESENCE_RUNNING);

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "DNSMasq restart aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	prepare_status = prepare_dns_dhcpd(is_ap_mode, &service_enabled);
	action = dnsmasq_restart_policy_action(was_running, prepare_status, service_enabled);
	if (action == DNSMASQ_RESTART_KEEP_RUNNING) {
		logmessage(LOGNAME, "DNSMasq refresh preparation failed (status=%d); keeping the running daemon", prepare_status);
		return prepare_status;
	}
	if (action == DNSMASQ_RESTART_ABORT)
		return prepare_status;

	stop_status = stop_dns_dhcpd_locked();
	final_presence = dnsmasq_process_presence(&scan_error);
	stop_postcondition = dnsmasq_restart_policy_stop_status(stop_status, final_presence);
	if (stop_postcondition != 0) {
		if (final_presence == PROCESS_PRESENCE_UNKNOWN && scan_error != 0)
			stop_postcondition = scan_error;
		logmessage(LOGNAME,
			   "DNSMasq stop incomplete (stop_status=%d, presence=%d, scan_error=%d); refusing duplicate launch",
			   stop_status, final_presence, scan_error);
		return stop_postcondition;
	}
	if (action == DNSMASQ_RESTART_STOP_ONLY)
		return 0;

	errno = 0;
	launch_status = eval("/usr/sbin/dnsmasq");
	start_status = dnsmasq_confirm_started(launch_status);
	if (start_status != 0) {
		if (launch_status != 0)
			logmessage(LOGNAME, "DNSMasq restart launch failed (status=%d); no unvalidated fallback attempted", launch_status);
		else
			logmessage(LOGNAME, "DNSMasq restart returned success but no running process appeared within %d seconds",
				   DNSMASQ_START_WAIT_ATTEMPTS - 1);
		start_status = dnsmasq_rollback_failed_start(start_status, "restart");
	}
	return start_status;
}

int
restart_dhcpd(void)
{
	int lock_fd, status, lock_errno;
	lock_fd = dnsmasq_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNSMasq lifecycle lock; refusing restart (errno=%d)", lock_errno);
		return lock_errno;
	}
	status = restart_dhcpd_locked();
	return dnsmasq_lifecycle_lock_finish(lock_fd, status, "restart");
}

static enum process_presence
dnsmasq_reload_validate_identity(void *context,
				const struct process_identity *identity)
{
	(void)context;
	return process_identity_validate("dnsmasq", identity);
}

static int
dnsmasq_reload_send_signal(void *context, pid_t pid, int signal_number)
{
	(void)context;
	return kill(pid, signal_number);
}

int
restart_dns(void)
{
	int lock_fd, status, lock_errno, scan_error, reload_decision;
	int post_scan_error;
	enum process_presence presence, post_presence;
	struct process_snapshot snapshot, post_snapshot;

	lock_fd = dnsmasq_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNSMasq lifecycle lock; refusing cache reload (errno=%d)", lock_errno);
		return lock_errno;
	}

	presence = process_inventory_scan("dnsmasq", &snapshot);
	scan_error = snapshot.error;
	reload_decision = dnsmasq_reload_notification_decision(presence);
	if (reload_decision < 0) {
		status = scan_error ? scan_error : EIO;
		logmessage(LOGNAME,
			   "cannot verify DNSMasq identities; refusing cache reload (scan_error=%d)",
			   status);
	} else if (reload_decision == 0) {
		/* A confirmed absent daemon has no cache to reload. */
		status = 0;
	} else {
		status = dnsmasq_reload_signal_snapshot(&snapshot,
				dnsmasq_reload_validate_identity,
				dnsmasq_reload_send_signal, NULL);
		if (status == ESRCH) {
			/* The last snapshot PID may exit after identity validation but
			 * before kill(SIGHUP). Treat that as a no-op only if a fresh,
			 * complete inventory confirms that no replacement is running. */
			post_presence = process_inventory_scan("dnsmasq", &post_snapshot);
			post_scan_error = post_snapshot.error;
			status = dnsmasq_reload_post_signal_status(status,
							    post_presence, post_scan_error);
			process_snapshot_release(&post_snapshot);
		}
		if (status != 0)
			logmessage(LOGNAME,
				   "DNSMasq cache reload was not confirmed (status=%d)",
				   status);
	}
	process_snapshot_release(&snapshot);
	return dnsmasq_lifecycle_lock_finish(lock_fd, status, "cache reload");
}

///////////////////////////////////////////////////////////////////////
// WINS (nmbd)
///////////////////////////////////////////////////////////////////////

#if defined(APP_SMBD) || defined(APP_NMBD)
static enum process_presence
samba_ex_daemon_is_running(void *context, const char *daemon)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int scan_error;

	(void)context;
	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan(daemon, &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = scan_error ? scan_error : EIO;
	return state;
}

static enum process_presence
samba_ex_validate_identity(void *context,
			   const struct process_identity *identity)
{
	const char *daemon = (const char *)context;

	if (!daemon || !identity) {
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	return process_identity_validate(daemon, identity);
}

static int
samba_ex_send_signal(void *context, pid_t pid, int signal_number)
{
	(void)context;
	return kill(pid, signal_number);
}

static int
samba_ex_require_absent(const char *daemon, const char *stage)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int scan_error;

	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan(daemon, &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (state == PROCESS_PRESENCE_ABSENT)
		return 0;

	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = scan_error ? scan_error : EIO;
	else if (state == PROCESS_PRESENCE_RUNNING)
		errno = EBUSY;
	else
		errno = EIO;
	logmessage("Samba Server",
		   "%s state is not confirmed ABSENT %s (presence=%d, errno=%d)",
		   daemon, stage ? stage : "", state, errno);
	return -1;
}

static int
samba_ex_daemon_sleep(void *context, unsigned int microseconds)
{
	(void)context;
	return service_wait_microseconds(microseconds);
}

static int
samba_ex_wait_for_daemon(const char *daemon)
{
	static const struct samba_daemon_wait_ops ops = {
		samba_ex_daemon_is_running,
		samba_ex_daemon_sleep
	};

	return samba_daemon_wait_for_stable_presence(daemon, 30, &ops, NULL);
}

static int
samba_ex_read_boolean_status(const char *name, int missing_value,
			     int *enabled);

static int
samba_ex_read_local_master_status(int *local_master)
{
	char *value = NULL;
	int api_result, saved_errno, read_status, parsed = 0;
	int status;

	if (!local_master) {
		errno = EINVAL;
		return -1;
	}
	*local_master = 0;
	errno = 0;
	api_result = nvram_get_checked("st_samba_lmb", &value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_int_status(
		api_result, saved_errno, value, 0, &parsed);
	status = samba_local_master_value_status(
		read_status, parsed, local_master);
	if (status != 0)
		errno = status;
	return status == 0 ? 0 : -1;
}

int
write_smb_conf_header(FILE *fp)
{
	int i_lmb, i_wins_enable;
	int dhcp_enabled = 0, dhcp_status;
	int is_ap_mode, ap_mode_status;
	char *p_computer_name, *p_workgroup, *p_res_order;

	if (!fp)
		return -1;

	/* A backend failure must not silently disable WINS in the published file. */
	if (samba_ex_read_boolean_status("wins_enable", 0,
					 &i_wins_enable) != 0) {
		int read_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot read WINS enable state; refusing smb.conf header publication (errno %d)",
			   read_error);
		errno = read_error;
		return -1;
	}

	if (!i_wins_enable) {
		ap_mode_status = get_ap_mode_status(&is_ap_mode);
		if (ap_mode_status != 0) {
			logmessage("Samba Server",
				   "cannot determine AP/router mode; refusing smb.conf header publication (errno %d)",
				   ap_mode_status);
			errno = ap_mode_status;
			return -1;
		}
		dhcp_status = is_dhcpd_enabled_status(is_ap_mode, &dhcp_enabled);
		if (dhcp_status != 0) {
			logmessage("Samba Server",
				   "cannot read DHCP enable state; refusing smb.conf header publication (errno %d)",
				   dhcp_status);
			errno = dhcp_status;
			return -1;
		}
	}

	if (samba_ex_read_local_master_status(&i_lmb) != 0) {
		int read_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot read or validate local-master mode; refusing smb.conf header publication (errno %d)",
			   read_error);
		errno = read_error;
		return -1;
	}
	p_workgroup = nvram_safe_get("st_samba_workgroup");
	p_computer_name = get_our_hostname();

	/* NVRAM/hostname data must not inject additional smb.conf directives. */
	if (!samba_config_value_is_safe(p_workgroup))
		p_workgroup = "";
	if (!samba_config_value_is_safe(p_computer_name) || !*p_computer_name)
		p_computer_name = "Padavan";

	p_res_order = "lmhosts hosts bcast";

	fprintf(fp, "[global]\n");
	if (strlen(p_workgroup) > 0)
		fprintf(fp, "workgroup = %s\n", p_workgroup);
	fprintf(fp, "netbios name = %s\n", p_computer_name);
	fprintf(fp, "server string = %s\n", p_computer_name);

	if (i_lmb == 0) {
		fprintf(fp, "local master = %s\n", "no");
	} else if (i_lmb == 1) {
		fprintf(fp, "local master = %s\n", "yes");
		fprintf(fp, "os level = %d\n", 128);
	} else {
		fprintf(fp, "local master = %s\n", "yes");
		fprintf(fp, "domain master = %s\n", "yes");
		fprintf(fp, "preferred master = %s\n", "yes");
		fprintf(fp, "os level = %d\n", 128);
	}

	if (i_wins_enable) {
		fprintf(fp, "wins support = %s\n", "yes");
	} else if (dhcp_enabled) {
		char *wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins)) {
			fprintf(fp, "wins server = %s\n", wins);
			p_res_order = "lmhosts wins hosts bcast";
		}
	}

	fprintf(fp, "name resolve order = %s\n", p_res_order);
	fprintf(fp, "log file = %s\n", "/var/log/samba.log");
	fprintf(fp, "log level = 0\n");
	fprintf(fp, "max log size = 5\n");
	fprintf(fp, "socket options = TCP_NODELAY SO_KEEPALIVE\n");
	fprintf(fp, "unix charset = UTF8\n");
	fprintf(fp, "display charset = UTF8\n");
	fprintf(fp, "bind interfaces only = %s\n", "yes");
	fprintf(fp, "interfaces = %s\n", IFNAME_BR);
	fprintf(fp, "unix extensions = no\n");
	fprintf(fp, "encrypt passwords = yes\n");
	fprintf(fp, "pam password change = no\n");
	fprintf(fp, "obey pam restrictions = no\n");
	fprintf(fp, "host msdfs = no\n");
	fprintf(fp, "disable spoolss = yes\n");
#if defined (APP_SMBD36)
	fprintf(fp, "max protocol = SMB2\n");
	fprintf(fp, "passdb backend = smbpasswd\n");
	fprintf(fp, "security = USER\n");
	fprintf(fp, "username level = 8\n");
#endif

	return ferror(fp) ? -1 : 0;
}

int
stop_nmbd(void)
{
	char* svcs[] = { "nmbd", NULL };
	int status;

	status = kill_services(svcs, 3, 1);
	if (status != 0)
		logmessage("Samba Server", "nmbd did not stop cleanly");
	return status;
}

static void
samba_rollback_failed_nmbd_start(int start_errno, const char *operation)
{
	int stop_status;
	int scan_error = 0;
	struct process_snapshot snapshot;
	enum process_presence presence;

	memset(&snapshot, 0, sizeof(snapshot));
	stop_status = stop_nmbd();
	presence = process_inventory_scan("nmbd", &snapshot);
	scan_error = snapshot.error;
	if (samba_failed_start_may_leave_daemon(presence)) {
		logmessage("Samba Server",
			   "nmbd may remain active after failed %s (rollback_status=%d, state=%d, scan_error=%d)",
			   operation, stop_status, presence, scan_error);
	} else if (stop_status != 0) {
		logmessage("Samba Server",
			   "nmbd is absent after failed %s rollback but stop reported status=%d",
			   operation, stop_status);
	}
	process_snapshot_release(&snapshot);
	errno = start_errno > 0 ? start_errno : EIO;
}

static int
samba_ex_read_boolean_status(const char *name, int missing_value, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, read_status, status;
	int parsed = 0;

	if (!name || !enabled)
		return EINVAL;
	*enabled = 0;
	errno = 0;
	api_result = nvram_get_checked(name, &value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_int_status(api_result, saved_errno,
						   value, missing_value,
						   &parsed);
	status = samba_boolean_value_status(read_status, parsed, enabled);
	if (status != 0)
		errno = status;
	return status == 0 ? 0 : -1;
}


int
start_wins(void)
{
	FILE *fp = NULL;
	char staged_path[64];
	int command_status, lock_fd, result = -1;
	int scan_error, wins_enabled;
	int launched_new_daemon = 0;
	int failure_errno;
	enum process_presence nmbd_presence;
	struct process_snapshot nmbd_snapshot;

	if (samba_ex_read_boolean_status("wins_enable", 1,
					 &wins_enabled) != 0) {
		int read_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot read WINS enable state; refusing startup (errno %d)",
			   read_error);
		errno = read_error;
		return -1;
	}
	if (!wins_enabled)
		return 0;

	memset(&nmbd_snapshot, 0, sizeof(nmbd_snapshot));
	lock_fd = samba_config_transaction_lock_acquire(
		SAMBA_CONFIG_TRANSACTION_LOCK_FILE);
	if (lock_fd < 0) {
		logmessage("Samba Server", "cannot lock WINS configuration publication (errno %d)",
			   errno ? errno : EIO);
		return -1;
	}
	/* UNKNOWN must block transaction recovery and all subsequent publication. */
	nmbd_presence = process_inventory_scan("nmbd", &nmbd_snapshot);
	if (nmbd_presence == PROCESS_PRESENCE_UNKNOWN) {
		scan_error = nmbd_snapshot.error ? nmbd_snapshot.error : EIO;
		errno = scan_error;
		logmessage("Samba Server", "nmbd inventory UNKNOWN; refusing WINS config publication (errno %d)",
			   scan_error);
		goto out;
	}
	if (nmbd_presence != PROCESS_PRESENCE_ABSENT &&
	    nmbd_presence != PROCESS_PRESENCE_RUNNING) {
		errno = EIO;
		goto out;
	}

	if (samba_config_transaction_prepare_parent_directory(
		SAMBA_CONFIG_BASE_DIRECTORY) != 0) {
		scan_error = errno ? errno : EIO;
		errno = scan_error;
		logmessage("Samba Server",
			   "unsafe or unavailable Samba state directory; refusing WINS startup (errno %d)",
			   scan_error);
		goto out;
	}
	if (samba_config_transaction_recover(
		SAMBA_CONFIG_TRANSACTION_DIRECTORY,
		SAMBA_CONFIG_TRANSACTION_PASSDB,
		SAMBA_CONFIG_TRANSACTION_CONFIG) != 0) {
		logmessage("Samba Server", "cannot recover interrupted Samba passdb/config publication (errno %d)",
			   errno ? errno : EIO);
		goto out;
	}

	staged_path[0] = '\0';
	if (samba_config_file_open(SAMBA_CONF, staged_path,
				   sizeof(staged_path), &fp) != 0 ||
	    write_smb_conf_header(fp) != 0 ||
	    samba_config_file_publish(&fp, staged_path, SAMBA_CONF) != 0) {
		samba_config_file_discard(&fp, staged_path);
		logmessage("Samba Server", "cannot publish WINS configuration; nmbd transition skipped");
		goto out;
	}

	if (nmbd_presence == PROCESS_PRESENCE_RUNNING) {
		command_status = samba_daemon_signal_snapshot(
			&nmbd_snapshot, SIGHUP, samba_ex_validate_identity,
			samba_ex_send_signal, (void *)"nmbd");
	} else {
		if (samba_ex_require_absent("nmbd", "before WINS launch") != 0)
			goto out;
		errno = 0;
		launched_new_daemon = 1;
		command_status = eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
	}

	if (command_status != 0) {
		logmessage("Samba Server", "nmbd %s command failed (status %d, errno %d)",
			   nmbd_presence == PROCESS_PRESENCE_RUNNING ? "reload" : "start",
			   command_status, errno ? errno : EIO);
		if (launched_new_daemon) {
			failure_errno = errno ? errno : EIO;
			samba_rollback_failed_nmbd_start(failure_errno, "WINS start");
		}
		goto out;
	}
	if (samba_ex_wait_for_daemon("nmbd") != 0) {
		failure_errno = errno ? errno : ETIMEDOUT;
		logmessage("Samba Server", "nmbd did not remain running after WINS start/reload (errno %d)",
			   failure_errno);
		if (launched_new_daemon)
			samba_rollback_failed_nmbd_start(failure_errno, "WINS start");
		goto out;
	}

	result = 0;
out:
	process_snapshot_release(&nmbd_snapshot);
	if (samba_config_transaction_lock_release(lock_fd) != 0) {
		logmessage("Samba Server", "cannot release WINS configuration lock (errno %d)",
			   errno ? errno : EIO);
		if (result == 0)
			result = -1;
	}
	return result;
}

int
reload_nmbd(void)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int result = 0, scan_error;

	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan("nmbd", &snapshot);
	scan_error = snapshot.error;
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		errno = scan_error ? scan_error : EIO;
		logmessage("Samba Server", "nmbd inventory UNKNOWN; refusing reload (errno %d)",
			   errno);
		result = -1;
	} else if (state == PROCESS_PRESENCE_RUNNING) {
		result = samba_daemon_signal_snapshot(
			&snapshot, SIGHUP, samba_ex_validate_identity,
			samba_ex_send_signal, (void *)"nmbd");
		if (result != 0) {
			logmessage("Samba Server", "nmbd identity-bound reload failed (errno %d)",
				   errno ? errno : EIO);
			result = -1;
		} else if (samba_ex_wait_for_daemon("nmbd") != 0) {
			result = -1;
		}
	} else if (state != PROCESS_PRESENCE_ABSENT) {
		result = -1;
		errno = EIO;
	}

	process_snapshot_release(&snapshot);
	return result;
}

int
restart_nmbd(void)
{
	int result = -1, scan_error, command_status;
	int storage_lock;
#if defined(APP_SMBD)
	int launched_new_daemon = 0;
	int failure_errno;
#endif
	struct process_snapshot nmbd_snapshot;
	enum process_presence nmbd_presence;
#if defined(APP_SMBD)
	struct process_snapshot smbd_snapshot;
	enum process_presence smbd_presence;
#endif

	/*
	 * Network notifications can republish Samba configuration and restart
	 * daemons while USB hotplug or safe removal is changing share mounts.
	 * Hold the common eject barrier through the entire process transition.
	 */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Samba Server", "cannot acquire storage operation lock; refusing nmbd restart");
		return -1;
	}

	memset(&nmbd_snapshot, 0, sizeof(nmbd_snapshot));
#if defined(APP_SMBD)
	memset(&smbd_snapshot, 0, sizeof(smbd_snapshot));
#endif
	nmbd_presence = process_inventory_scan("nmbd", &nmbd_snapshot);
	if (nmbd_presence == PROCESS_PRESENCE_UNKNOWN) {
		scan_error = nmbd_snapshot.error ? nmbd_snapshot.error : EIO;
		errno = scan_error;
		logmessage("Samba Server", "nmbd inventory UNKNOWN; refusing restart (errno %d)", scan_error);
		goto out;
	}
	if (nmbd_presence != PROCESS_PRESENCE_ABSENT &&
	    nmbd_presence != PROCESS_PRESENCE_RUNNING) {
		errno = EIO;
		goto out;
	}

#if defined(APP_SMBD)
	smbd_presence = process_inventory_scan("smbd", &smbd_snapshot);
	if (smbd_presence == PROCESS_PRESENCE_UNKNOWN) {
		scan_error = smbd_snapshot.error ? smbd_snapshot.error : EIO;
		errno = scan_error;
		logmessage("Samba Server", "smbd inventory UNKNOWN; refusing config publication/restart (errno %d)",
			   scan_error);
		goto out;
	}
	if (smbd_presence != PROCESS_PRESENCE_ABSENT &&
	    smbd_presence != PROCESS_PRESENCE_RUNNING) {
		errno = EIO;
		goto out;
	}
	if (smbd_presence == PROCESS_PRESENCE_RUNNING) {
		if (write_smb_conf() != 0) {
			logmessage("Samba Server", "configuration publication failed; nmbd/smbd reload skipped");
			goto out;
		}
		if (stop_nmbd() != 0)
			goto out;
		if (samba_ex_require_absent("nmbd", "after stop and before restart") != 0)
			goto out;
		errno = 0;
		launched_new_daemon = 1;
		command_status = eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
		if (command_status != 0) {
			failure_errno = errno ? errno : EIO;
			logmessage("Samba Server", "nmbd start command failed after configuration publication (status %d)",
				   command_status);
			samba_rollback_failed_nmbd_start(failure_errno, "restart");
			goto out;
		}
		if (samba_ex_wait_for_daemon("nmbd") != 0) {
			failure_errno = errno ? errno : ETIMEDOUT;
			logmessage("Samba Server", "nmbd did not remain running after restart (errno %d)",
				   failure_errno);
			samba_rollback_failed_nmbd_start(failure_errno, "restart");
			goto out;
		}
		command_status = samba_daemon_signal_snapshot(
			&smbd_snapshot, SIGHUP, samba_ex_validate_identity,
			samba_ex_send_signal, (void *)"smbd");
		if (command_status != 0) {
			logmessage("Samba Server", "smbd identity-bound reload failed after nmbd restart (errno %d)",
				   errno ? errno : EIO);
			goto out;
		}
		if (samba_ex_wait_for_daemon("smbd") != 0) {
			logmessage("Samba Server", "smbd did not remain running after reload (errno %d)",
				   errno ? errno : ETIMEDOUT);
			goto out;
		}
		result = 0;
		goto out;
	}
#endif

	if (stop_nmbd() != 0)
		goto out;
	if (samba_ex_require_absent("nmbd", "after stop and before WINS start") != 0)
		goto out;
	result = start_wins();

out:
	process_snapshot_release(&nmbd_snapshot);
#if defined(APP_SMBD)
	process_snapshot_release(&smbd_snapshot);
#endif
	file_unlock(storage_lock);
	return result;
}
#endif

///////////////////////////////////////////////////////////////////////
// miniupnpd
///////////////////////////////////////////////////////////////////////

static int
miniupnpd_lifecycle_lock_finish(int lock_fd, int operation_status,
                                const char *operation)
{
	int lock_errno;

	if (miniupnpd_lifecycle_lock_release(lock_fd) == 0)
		return operation_status;
	lock_errno = errno ? errno : EIO;
	logmessage(LOGNAME, "cannot release miniupnpd lifecycle lock after %s (errno=%d)",
		   operation, lock_errno);
	return operation_status != 0 ? operation_status : lock_errno;
}

static enum process_presence
miniupnpd_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence state;

	state = process_inventory_scan("miniupnpd", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return state;
}

static enum process_presence
miniupnpd_validate_identity(void *context,
			    const struct process_identity *identity)
{
	(void)context;
	return process_identity_validate("miniupnpd", identity);
}

static int
miniupnpd_send_signal(void *context, pid_t pid, int signal_number)
{
	(void)context;
	return kill(pid, signal_number);
}

int
is_upnp_run(void)
{
	return miniupnpd_process_presence(NULL) == PROCESS_PRESENCE_RUNNING;
}

static int
miniupnpd_confirm_started(int launch_status)
{
	int attempt;
	int daemon_running = 0;
	int scan_error;
	int wait_status;
	enum process_presence state;

	if (launch_status != 0)
		return miniupnpd_start_policy_status(launch_status, 0);

	/* Let the detached child finish socket initialization before probing. */
	wait_status = service_start_wait_one_second("miniupnpd");
	if (wait_status != 0)
		return wait_status;
	for (attempt = 0; attempt < MINIUPNPD_START_WAIT_ATTEMPTS; attempt++) {
		state = miniupnpd_process_presence(&scan_error);
		if (state == PROCESS_PRESENCE_RUNNING) {
			daemon_running = 1;
			break;
		}
		if (state == PROCESS_PRESENCE_UNKNOWN ||
		    state != PROCESS_PRESENCE_ABSENT) {
			errno = miniupnpd_presence_error(state, scan_error);
			logmessage(LOGNAME,
				   "miniupnpd startup inventory became unknown (errno %d)",
				   errno);
			return errno;
		}
		if (attempt + 1 < MINIUPNPD_START_WAIT_ATTEMPTS) {
			wait_status = service_start_wait_one_second("miniupnpd");
			if (wait_status != 0)
				return wait_status;
		}
	}

	return miniupnpd_start_policy_status(launch_status, daemon_running);
}

struct miniupnpd_config_data {
	const char *wan_ifname;
	const char *lan_url;
	const char *proto_upnp;
	const char *proto_npmp;
	const char *secured;
	const char *lan_class;
	int clean_min;
	int clean_int;
	int iport_min;
	int iport_max;
	int eport_min;
	int eport_max;
	uint8_t lan_mac[6];
};

static int
miniupnpd_write_config(FILE *fp, void *opaque)
{
	struct miniupnpd_config_data *config = (struct miniupnpd_config_data *)opaque;

	if (fprintf(fp, "# automatically generated\n") < 0)
		return errno ? errno : EIO;
	if (fprintf(fp,
		"ext_ifname=%s\n"
		"listening_ip=%s\n"
		"port=%d\n"
		"enable_upnp=%s\n"
		"enable_natpmp=%s\n"
		"upnp_forward_chain=%s\n"
		"upnp_nat_chain=%s\n"
		"upnp_nat_postrouting_chain=%s\n"
		"secure_mode=%s\n"
		"lease_file=%s\n"
		"presentation_url=http://%s/\n"
		"system_uptime=yes\n"
		"notify_interval=%d\n"
		"clean_ruleset_interval=%d\n"
		"clean_ruleset_threshold=%d\n"
		"uuid=75802409-bccb-40e7-8e6c-%02x%02x%02x%02x%02x%02x\n"
		"friendly_name=%s\n"
		"manufacturer_name=%s\n"
		"manufacturer_url=%s\n"
		"model_name=%s\n"
		"model_description=%s\n"
		"model_url=%s\n"
		"model_number=%s\n"
		"serial=%s\n"
		"bitrate_up=%d\n"
		"bitrate_down=%d\n"
		"allow %d-%d %s %d-%d\n"
		"deny 0-65535 0.0.0.0/0 0-65535\n",
		config->wan_ifname,
		IFNAME_BR, /*lan_addr, lan_mask,*/
		0,
		config->proto_upnp,
		config->proto_npmp,
		MINIUPNPD_CHAIN_IP4_FORWARD,
		MINIUPNPD_CHAIN_IP4_NAT,
		MINIUPNPD_CHAIN_IP4_NAT_POST,
		config->secured,
		UPNPD_LEASE_FILE,
		config->lan_url,
		60,
		config->clean_int,
		config->clean_min,
		config->lan_mac[0], config->lan_mac[1], config->lan_mac[2],
		config->lan_mac[3], config->lan_mac[4], config->lan_mac[5],
		BOARD_DESC,
		BOARD_VENDOR_NAME,
		BOARD_VENDOR_URL,
		"Wireless Router",
		BOARD_DESC,
		BOARD_MODEL_URL,
		BOARD_NAME,
		"1.0",
		100000000,
		100000000,
		config->eport_min, config->eport_max, config->lan_class,
		config->iport_min, config->iport_max) < 0)
		return errno ? errno : EIO;

	return 0;
}

static int
miniupnpd_read_transaction_int(struct nvram_transaction *transaction,
				const char *name, int *number)
{
	char *value = NULL;
	int api_result, saved_errno;

	if (!transaction || !name || !number)
		return EINVAL;
	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	return miniupnpd_nvram_int_status(api_result, saved_errno,
					  value, 0, number);
}

static int
miniupnpd_read_transaction_bridge(struct nvram_transaction *transaction,
				  int *bridged)
{
	char *value = NULL;
	int api_result, saved_errno;

	if (!transaction || !bridged)
		return EINVAL;
	errno = 0;
	api_result = nvram_transaction_get(transaction, "wan_route_x", &value);
	saved_errno = errno;
	return miniupnpd_nvram_bridge_status(api_result, saved_errno,
					     value, bridged);
}

/*
 * Read the service enablement tuple under one NVRAM lock. An unreadable
 * setting must not be treated as disabled and must never stop a live daemon.
 */
static int
miniupnpd_read_stop_policy(int *should_stop)
{
	struct nvram_transaction transaction;
	int upnp_enabled = 0, nat_enabled = 0, bridged = 0;
	int local_should_stop = 0;
	int status, end_result, end_errno;

	if (!should_stop)
		return EINVAL;
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0) {
		status = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot begin UPnP service-policy NVRAM transaction (errno=%d)",
			   status);
		errno = status;
		return status;
	}

	status = miniupnpd_read_transaction_int(&transaction,
						"upnp_enable_x", &upnp_enabled);
	if (status == 0)
		status = miniupnpd_read_transaction_int(&transaction,
							"wan_nat_x", &nat_enabled);
	if (status == 0)
		status = miniupnpd_read_transaction_bridge(&transaction, &bridged);
	if (status == 0)
		status = miniupnpd_update_status(0, upnp_enabled, 0, nat_enabled,
						 0, bridged, &local_should_stop);

	errno = 0;
	end_result = nvram_transaction_end(&transaction);
	end_errno = errno;
	if (status == 0 && end_result != 0)
		status = end_errno > 0 ? end_errno : EIO;
	if (status != 0) {
		logmessage(LOGNAME,
			   "UPnP service-policy NVRAM read failed; preserving daemon state (errno=%d)",
			   status);
		errno = status;
		return status;
	}

	*should_stop = local_should_stop;
	return 0;
}

static int stop_upnp_locked(void);

static int
miniupnpd_read_nvram_integer(const char *name, int default_value,
                             int minimum, int maximum, int *result)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	errno = 0;
	api_result = nvram_get_checked(name, &value);
	saved_errno = errno;
	status = miniupnpd_config_file_read_integer_status(
		api_result, saved_errno, value, default_value, minimum, maximum,
		result);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read or validate MiniUPnP option %s (status=%d)",
			   name, status);
		errno = status;
	}
	return status;
}

static int
start_upnp_locked(void)
{
	int ret, launch_status, start_status, start_errno, read_status;
	int rollback_status, rollback_scan_error = 0;
	int i_proto_use, i_upnp_secure, i_http_lanport;
	int i_clean_min, i_clean_int, i_iports[2], i_eports[2];
	enum process_presence rollback_presence;
	int scan_error, presence_error;
	int url_error;
	int service_policy_status, should_stop;
	enum process_presence state;
	char *lan_addr, *lan_mask, *lan_url, *proto_upnp, *proto_npmp, *secured;
	char var[100];
	char wan_ifname[16];
	char lan_class[24];
	uint8_t lan_mac[16];
	struct miniupnpd_config_data config;

	service_policy_status = miniupnpd_read_stop_policy(&should_stop);
	if (service_policy_status != 0) {
		errno = service_policy_status;
		logmessage(LOGNAME,
			   "miniupnpd start aborted: service configuration is unreadable (errno=%d)",
			   service_policy_status);
		return service_policy_status;
	}
	if (should_stop)
		return 0;

	state = miniupnpd_process_presence(&scan_error);
	if (state == PROCESS_PRESENCE_RUNNING) {
		logmessage(LOGNAME, "miniupnpd is already running; duplicate start skipped");
		return 0;
	}
	if (!miniupnpd_start_allowed(state)) {
		presence_error = miniupnpd_presence_error(state, scan_error);
		errno = presence_error;
		logmessage(LOGNAME,
			   "miniupnpd start aborted: process inventory is not ABSENT (errno %d)",
			   presence_error);
		return presence_error;
	}

	/*
	 * Protocol, security, presentation-port and mapping-range settings define
	 * the exposure boundary. Cleanup timers also participate in the published
	 * daemon configuration. Never substitute a permissive default on a backend
	 * error, malformed value or out-of-range policy.
	 */
	if ((read_status = miniupnpd_read_nvram_integer("upnp_proto", 0, 0, 2,
						       &i_proto_use)) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_secure", 0, 0, 1,
						       &i_upnp_secure)) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("http_lanport", 0, 0, 65535,
						       &i_http_lanport)) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_clean_min", 10, 1, 999,
						       &i_clean_min)) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_clean_int", 600, 0, 86400,
						       &i_clean_int)) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_iport_min", 21, 1, 65535,
						       &i_iports[0])) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_iport_max", 65535, 1, 65535,
						       &i_iports[1])) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_eport_min", 21, 1, 65535,
						       &i_eports[0])) != 0 ||
	    (read_status = miniupnpd_read_nvram_integer("upnp_eport_max", 65535, 1, 65535,
						       &i_eports[1])) != 0) {
		errno = read_status;
		logmessage(LOGNAME,
			   "miniupnpd start aborted: protocol/security/HTTP/port/cleanup policy is unreadable (status=%d)",
			   read_status);
		return read_status;
	}

	if (i_iports[0] > i_iports[1])
		i_iports[0] = i_iports[1];
	if (i_eports[0] > i_eports[1])
		i_eports[0] = i_eports[1];

	ret = get_wan_ifname(wan_ifname);
	if (ret != 0) {
		errno = ret;
		logmessage(LOGNAME, "miniupnpd start aborted: invalid WAN interface name (status=%d)",
			   ret);
		return ret;
	}

	lan_addr = nvram_safe_get("lan_ipaddr");
	lan_mask = nvram_safe_get("lan_netmask");
	if (!is_valid_ipv4(lan_addr) || !is_valid_ipv4(lan_mask) ||
	    !ipv4_netmask_is_contiguous(lan_mask)) {
		logmessage(LOGNAME, "miniupnpd start aborted: invalid LAN IPv4 address or non-contiguous netmask");
		errno = EINVAL;
		return EINVAL;
	}
	ip2class(lan_addr, lan_mask, lan_class, sizeof(lan_class));
	memset(lan_mac, 0, sizeof(lan_mac));
	ether_atoe(nvram_safe_get("lan_hwaddr"), lan_mac);

	if (miniupnpd_format_lan_url(lan_addr, i_http_lanport, var, sizeof(var)) != 0) {
		url_error = errno > 0 ? errno : EIO;
		logmessage(LOGNAME, "miniupnpd start aborted: cannot format LAN presentation URL (errno %d)",
			   url_error);
		return url_error;
	}
	lan_url = var;

	if (i_proto_use == 2) {
		proto_upnp = "yes";
		proto_npmp = "yes";
	} else if (i_proto_use == 1) {
		proto_upnp = "no";
		proto_npmp = "yes";
	} else {
		proto_upnp = "yes";
		proto_npmp = "no";
	}

	secured = i_upnp_secure ? "yes" : "no";

	memset(&config, 0, sizeof(config));
	config.wan_ifname = wan_ifname;
	config.lan_url = lan_url;
	config.proto_upnp = proto_upnp;
	config.proto_npmp = proto_npmp;
	config.secured = secured;
	config.lan_class = lan_class;
	config.clean_min = i_clean_min;
	config.clean_int = i_clean_int;
	config.iport_min = i_iports[0];
	config.iport_max = i_iports[1];
	config.eport_min = i_eports[0];
	config.eport_max = i_eports[1];
	memcpy(config.lan_mac, lan_mac, sizeof(config.lan_mac));

	ret = miniupnpd_config_publish("/etc/miniupnpd.conf",
				       miniupnpd_write_config, &config);
	if (ret != 0) {
		logmessage(LOGNAME, "miniupnpd configuration publication failed (status %d)", ret);
		return ret;
	}

	state = miniupnpd_process_presence(&scan_error);
	if (state != PROCESS_PRESENCE_ABSENT) {
		presence_error = miniupnpd_presence_error(state, scan_error);
		if (presence_error == 0)
			presence_error = EBUSY;
		errno = presence_error;
		logmessage(LOGNAME,
			   "miniupnpd launch aborted: daemon appeared during configuration (state=%d, errno=%d)",
			   state, presence_error);
		return presence_error;
	}

	errno = 0;
	launch_status = eval("/usr/bin/miniupnpd");
	start_status = miniupnpd_confirm_started(launch_status);
	if (start_status != 0) {
		/*
		 * eval() only reports the launcher result. The detached daemon may
		 * already be active even when the startup wait/inventory fails.
		 * Roll it back under the already-held MiniUPnP lifecycle lock.
		 */
		start_errno = errno > 0 ? errno :
			(start_status > 0 ? start_status : EIO);
		rollback_status = stop_upnp_locked();
		rollback_presence = miniupnpd_process_presence(&rollback_scan_error);
		if (miniupnpd_failed_start_may_leave_daemon(rollback_presence)) {
			logmessage(LOGNAME,
				   "miniupnpd may remain active after failed start (start_status=%d, rollback_status=%d, state=%d, scan_error=%d)",
				   start_status, rollback_status, rollback_presence,
				   rollback_scan_error);
		} else if (rollback_status != 0) {
			logmessage(LOGNAME,
				   "miniupnpd is absent after failed-start rollback but stop reported status=%d",
				   rollback_status);
		}
		/* Rollback diagnostics must never replace the original start error. */
		errno = start_errno;
		ret = start_status;
	} else {
		ret = 0;
	}
	return ret;
}

int
start_upnp(void)
{
	int lock_fd, status, lock_errno;

	lock_fd = miniupnpd_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire miniupnpd lifecycle lock; refusing start (errno=%d)",
			   lock_errno);
		return lock_errno;
	}
	status = start_upnp_locked();
	return miniupnpd_lifecycle_lock_finish(lock_fd, status, "start");
}

static int
stop_upnp_locked(void)
{
	char* svcs[] = { "miniupnpd", NULL };
	int ret;
	int scan_error;
	enum process_presence state;

	ret = kill_services(svcs, 3, 1);
	state = miniupnpd_process_presence(&scan_error);
	if (ret == 0) {
		ret = miniupnpd_presence_error(state, scan_error);
		if (ret == 0)
			ret = miniupnpd_stop_policy_status(0,
						   state == PROCESS_PRESENCE_RUNNING);
	}
	if (ret != 0)
		logmessage(LOGNAME, "miniupnpd did not stop cleanly (status %d)", ret);

	return ret;
}

int
stop_upnp(void)
{
	int lock_fd, status, lock_errno;

	lock_fd = miniupnpd_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire miniupnpd lifecycle lock; refusing stop (errno=%d)",
			   lock_errno);
		return lock_errno;
	}
	status = stop_upnp_locked();
	return miniupnpd_lifecycle_lock_finish(lock_fd, status, "stop");
}

static int
check_upnp_wanif_changed_locked(char *wan_ifname)
{
	int scan_error;
	int presence_error;
	enum process_presence state;

	state = miniupnpd_process_presence(&scan_error);
	presence_error = miniupnpd_presence_error(state, scan_error);
	if (presence_error != 0) {
		errno = presence_error;
		logmessage(LOGNAME, "UPnP WAN transition aborted: process inventory unknown (errno %d)",
			   presence_error);
		return presence_error;
	}
	if (state == PROCESS_PRESENCE_ABSENT)
		return 0;

	/* Restart miniupnpd only if the active WAN interface changes. */
	if (strcmp(wan_ifname, get_wan_unit_value(0, "ifname_t")) != 0)
		return stop_upnp_locked();

	return 0;
}

int
check_upnp_wanif_changed(char *wan_ifname)
{
	int lock_fd, status, lock_errno;

	lock_fd = miniupnpd_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire miniupnpd lifecycle lock; refusing WAN check (errno=%d)",
			   lock_errno);
		return lock_errno;
	}
	status = check_upnp_wanif_changed_locked(wan_ifname);
	return miniupnpd_lifecycle_lock_finish(lock_fd, status, "WAN check");
}

static int
update_upnp_locked(void)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	enum miniupnpd_daemon_action action;
	int scan_error;
	int result;
	int saved_errno;
	int service_policy_status, should_stop;

	state = process_inventory_scan("miniupnpd", &snapshot);
	action = miniupnpd_action_for_presence(state);
	if (action == MINIUPNPD_DAEMON_ABORT) {
		scan_error = snapshot.error > 0 ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = miniupnpd_presence_error(state, scan_error);
		logmessage(LOGNAME,
			   "UPnP update aborted: process inventory unknown (errno %d)",
			   errno);
		return errno;
	}
	if (action == MINIUPNPD_DAEMON_START) {
		process_snapshot_release(&snapshot);
		return start_upnp_locked();
	}

	service_policy_status = miniupnpd_read_stop_policy(&should_stop);
	if (service_policy_status != 0) {
		process_snapshot_release(&snapshot);
		errno = service_policy_status;
		logmessage(LOGNAME,
			   "UPnP update aborted: service configuration is unreadable; daemon remains active (errno=%d)",
			   service_policy_status);
		return service_policy_status;
	}
	if (should_stop) {
		process_snapshot_release(&snapshot);
		return stop_upnp_locked();
	}

	/* Update UPnP forwards from the lease file without changing mapping policy. */
	if (!check_if_file_exist(UPNPD_LEASE_FILE)) {
		process_snapshot_release(&snapshot);
		return 0;
	}

	result = miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					   miniupnpd_validate_identity,
					   miniupnpd_send_signal, NULL);
	saved_errno = errno;
	process_snapshot_release(&snapshot);
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		logmessage(LOGNAME, "miniupnpd mapping reload signal failed (errno %d)", errno);
		return -1;
	}

	return 0;
}

int
update_upnp(void)
{
	int lock_fd, status, lock_errno;

	lock_fd = miniupnpd_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire miniupnpd lifecycle lock; refusing update (errno=%d)",
			   lock_errno);
		return lock_errno;
	}
	status = update_upnp_locked();
	return miniupnpd_lifecycle_lock_finish(lock_fd, status, "update");
}

int
restart_upnp(void)
{
	int lock_fd, ret, lock_errno;

	lock_fd = miniupnpd_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire miniupnpd lifecycle lock; refusing restart (errno=%d)",
			   lock_errno);
		return lock_errno;
	}
	ret = rc_upnp_restart_run(stop_upnp_locked, restart_firewall);
	if (ret != 0)
		logmessage(LOGNAME, "skipping firewall restart because miniupnpd is still active (status %d)", ret);
	return miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart");
}

///////////////////////////////////////////////////////////////////////
// inadyn
///////////////////////////////////////////////////////////////////////

int
ddns_updated_main(int argc, char *argv[])
{
	int restart_status;
	int scan_error = 0;
	enum process_presence dnsmasq_presence;
	enum dnsmasq_ddns_cache_purge_action cache_action;

	/* The cache purge is best effort; do not fail the DDNS update callback. */
	if (getenv("INADYN_IP")) {
		dnsmasq_presence = dnsmasq_process_presence(&scan_error);
		cache_action = dnsmasq_ddns_cache_purge_action_for_presence(dnsmasq_presence);
		if (cache_action == DNSMASQ_DDNS_CACHE_PURGE_RESTART) {
			restart_status = restart_dns();
			if (restart_status != 0)
				logmessage(LOGNAME, "DDNS update completed but DNSMasq cache purge failed (status=%d)",
					   restart_status);
		} else if (cache_action == DNSMASQ_DDNS_CACHE_PURGE_UNKNOWN) {
			logmessage(LOGNAME,
				   "skipping DNSMasq cache purge after DDNS update because process state is unknown (scan_error=%d)",
				   scan_error ? scan_error : EIO);
		}
	}

	return 0;
}

static int
stop_ddns_locked(void)
{
	char* svcs[] = { "inadyn", NULL };

	return kill_services(svcs, 3, 1);
}

static int
inadyn_lifecycle_lock_acquire_status(const char *operation, int *lock_fd)
{
	*lock_fd = inadyn_lifecycle_lock_acquire();
	if (*lock_fd >= 0)
		return 0;
	{
		int lock_error = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire Inadyn %s lifecycle lock (status=%d)",
			   operation, lock_error);
		return lock_error;
	}
}

static int
inadyn_lifecycle_lock_finish(int lock_fd, int operation_status,
			     const char *operation)
{
	if (inadyn_lifecycle_lock_release(lock_fd) != 0) {
		int release_error = errno ? errno : EIO;
		logmessage(LOGNAME, "Inadyn %s lifecycle lock release failed (status=%d)",
			   operation, release_error);
		if (operation_status == 0)
			operation_status = release_error;
	}
	return operation_status;
}

int
stop_ddns(void)
{
	int lock_fd, lock_status, status;
	lock_status = inadyn_lifecycle_lock_acquire_status("stop", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = stop_ddns_locked();
	return inadyn_lifecycle_lock_finish(lock_fd, status, "stop");
}

static const struct inadyn_system_t {
	const char *alias;
	const char *system;
} inadyn_systems[] = {
	{ "WWW.DYNDNS.ORG",       "default@dyndns.org"         },
	{ "WWW.ZONEEDIT.COM",     "default@zoneedit.com"       },
	{ "WWW.EASYDNS.COM",      "default@easydns.com"        },
	{ "WWW.NO-IP.COM",        "default@no-ip.com"          },
	{ "WWW.DNSOMATIC.COM",    "default@dnsomatic.com"      },
	{ "WWW.DNSEXIT.COM",      "default@dnsexit.com"        },
	{ "WWW.CHANGEIP.COM",     "default@changeip.com"       },
	{ "WWW.SITELUTIONS.COM",  "default@sitelutions.com"    },
	{ "WWW.DHIS.ORG",         "default@dhis.org"           },
	{ "WWW.DUCKDNS.ORG",      "default@duckdns.org"        },
	{ "WWW.OVH.COM",          "default@ovh.com"            },
	{ "WWW.LOOPIA.COM",       "default@loopia.com"         },
	{ "WWW.DUIADNS.NET",      "default@duiadns.net"        },
	{ "WWW.TUNNELBROKER.NET", "default@tunnelbroker.net"   },
	{ "DNS.HE.NET",           "dyndns@he.net"              },
	{ "DDNSS.DE",             "default@ddnss.de"           },
	{ "HOMESERVER.GIRA.DE",   "default@gira.de"            },
	{ "DOMAINS.GOOGLE.COM",   "default@domains.google.com" },
	{ "IPV4.DYNV6.COM",       "default@ipv4.dynv6.com"     },
	{ "DYNV6.COM",            "default@dynv6.com"          },
	{ "TB.NETASSIST.UA",      "ipv6tb@netassist.ua"        },
	{ "IPV4.NSUPDATE.INFO",   "ipv4@nsupdate.info"         },
	{ "FREEDNS.AFRAID.ORG",   "default@freedns.afraid.org" },
	{ "FREEMYIP.COM",         "default@freemyip.com"       },
	{ "SPDYN.DE",             "default@spdyn.de"           },
	{ "WWW.STRATO.DE",        "default@strato.com"         },
	{ "CLOUDXNS.NET",         "default@cloudxns.net"       },
	{ "3322.ORG",             "dyndns@3322.org"            },
	{ "DNSPOD.CN",            "default@dnspod.cn"          },
	{ "DYNU.COM",             "default@dynu.com"           },
	{ "SELFHOST.DE",          "default@selfhost.de"        },
	{ "PDD.YANDEX.RU",        "default@pdd.yandex.ru"      },
	{ "CLOUDFLARE.COM",       "default@cloudflare.com"     },
	{ "ORAY.COM",             "default@oray.com"           },
	{ "CUSTOM",               "custom"                     },
	{ NULL, NULL }
};

static const struct _inadyn_checkip_url {
	const char* server;
	const char* path;
	bool  ssl;
} checkip_urls[] = {
	{ "",                         ""      , false },
	{ "checkip.dyndns.org",       "/"     , false },	/* v4 only */
	{ "checkip.dyndns.org:8245",  "/"     , false },	/* v4 only */
	{ "ip.dnsexit.com",           "/"     , false },	/* v4 only */
	{ "ip.changeip.com",          "/"     , true  },	/* v4 only */
	{ "myip.dnsomatic.com",       "/"     , true  },	/* v4 only */
	{ "ip1.dynupdate.no-ip.com",  "/"     , false },	/* v4 only */
	{ "checkip.dns.he.net",       "/"     , true  },	/* dual stack */
	{ "checkip.two-dns.de",       "/"     , false },	/* v4 only */
	{ "ip.3322.net",              "/"     , false },	/* v4 only */
	{ "myip.ipip.net",            "/s"    , true  },	/* v4 only */
	{ "api.myip.la",              "/"     , true  },	/* dual stack */
	{ "ipv4.wtfismyip.com",       "/text" , true  },
	{ "ipv6.wtfismyip.com",       "/text" , true  },
	{ "ipv4.nsupdate.info",       "/myip" , true  },
	{ "ipv6.nsupdate.info",       "/myip" , true  },
	{ "api-ipv4.ip.sb",           "/ip"   , true  },
	{ "api-ipv6.ip.sb",           "/ip"   , true  },
	{ "v4.ident.me",              "/"     , true  },
	{ "v6.ident.me",              "/"     , true  },
	{ "api4.my-ip.io",            "/ip"   , true  },
	{ "api6.my-ip.io",            "/ip"   , true  },
	{ "api.ipify.org",            "/"     , true  },
	{ "api6.ipify.org",           "/"     , true  },
	{ "ipv4.duiadns.net",         "/"     , true  },
	{ "ipv6.duiadns.net",         "/"     , true  },
	{ "checkip4.spdyn.de",        "/"     , false },
	{ "checkip6.spdyn.de",        "/"     , false },
	{ "v4.ipv6-test.com",         "/api/myip.php", true },
	{ "v6.ipv6-test.com",         "/api/myip.php", true },
};

static const char *
get_inadyn_system(const char *alias)
{
	const struct inadyn_system_t *inadyn;
	const char *system = NULL;

	for (inadyn = &inadyn_systems[0]; inadyn->alias; inadyn++) {
		if (strcmp(alias, inadyn->alias) == 0) {
			system = inadyn->system;
			break;
		}
	}

	return system;
}

enum inadyn_config_nvram_field {
	INADYN_CFG_DDNS_PERIOD = 0,
	INADYN_CFG_DDNS_FORCED,
	INADYN_CFG_DDNS_SOURCE,
	INADYN_CFG_DDNS_IPV6,
	INADYN_CFG_DDNS_SSL,
	INADYN_CFG_DDNS_WILDCARD,
	INADYN_CFG_DDNS2_SSL,
	INADYN_CFG_DDNS2_WILDCARD,
	INADYN_CFG_DDNS_CHECKIP,
	INADYN_CFG_DDNS2_CHECKIP,
	INADYN_CFG_DDNS_SERVER,
	INADYN_CFG_DDNS_HOSTNAME,
	INADYN_CFG_DDNS_HOSTNAME2,
	INADYN_CFG_DDNS_HOSTNAME3,
	INADYN_CFG_DDNS_USERNAME,
	INADYN_CFG_DDNS_PASSWORD,
	INADYN_CFG_DDNS_CUSTOM_SERVER,
	INADYN_CFG_DDNS_CUSTOM_URL,
	INADYN_CFG_DDNS2_SERVER,
	INADYN_CFG_DDNS2_HOSTNAME,
	INADYN_CFG_DDNS2_USERNAME,
	INADYN_CFG_DDNS2_PASSWORD,
	INADYN_CFG_FIELD_COUNT
};

#define INADYN_CONFIG_VALUE_SIZE (NVRAM_MAX_VALUE_LEN + 1)

static const char *const inadyn_config_nvram_names[INADYN_CFG_FIELD_COUNT] = {
	"ddns_period", "ddns_forced", "ddns_source", "ddns_ipv6",
	"ddns_ssl", "ddns_wildcard_x", "ddns2_ssl", "ddns2_wildcard_x",
	"ddns_checkip", "ddns2_checkip", "ddns_server_x", "ddns_hostname_x",
	"ddns_hostname2_x", "ddns_hostname3_x", "ddns_username_x",
	"ddns_passwd_x", "ddns_cst_svr", "ddns_cst_url", "ddns2_server",
	"ddns2_hname", "ddns2_user", "ddns2_pass"
};

static int
inadyn_nvram_snapshot_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
inadyn_nvram_snapshot_get(void *context, const char *name, char **value)
{
	return nvram_transaction_get((struct nvram_transaction *)context,
				     name, value);
}

static int
inadyn_nvram_snapshot_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

static int
inadyn_parse_config_integer(const char *name, const char *value,
			    int default_value, int minimum, int maximum,
			    int *result)
{
	int status;

	status = inadyn_config_file_read_integer_status(
		1, 0, value, default_value, minimum, maximum, result);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot validate Inadyn option %s (status=%d)",
			   name, status);
		errno = status;
	}
	return status;
}

static int
write_inadyn_conf(const char *conf_file)
{
	FILE *fp = NULL;
	char staged_path[PATH_MAX] = { 0 };
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[INADYN_CFG_FIELD_COUNT];
	char *config_storage;
	char *config_value[INADYN_CFG_FIELD_COUNT];
	size_t field_index;
	int saved_errno = 0, interface_status, read_status;
	int i_max, i_ddns_source, i_ddns_period, i_ddns_forced, i_ddns_ipv6;
	int i_ddns1_ssl, i_ddns1_checkip, i_ddns1_wildcard;
	int i_ddns2_ssl, i_ddns2_checkip, i_ddns2_wildcard;
	char *ddns1_hname[3], *ddns1_user, *ddns1_pass, *ddns1_svr, *ddns1_url;
	char *ddns2_hname,    *ddns2_user, *ddns2_pass;
	const char *ddns1_svc, *ddns2_svc;
	const char *management_ifname;
	char wan_ifname[16];

	config_storage = (char *)calloc(INADYN_CFG_FIELD_COUNT,
					INADYN_CONFIG_VALUE_SIZE);
	if (!config_storage) {
		errno = ENOMEM;
		logmessage(LOGNAME, "cannot allocate Inadyn NVRAM snapshot");
		return 0;
	}
	for (field_index = 0; field_index < INADYN_CFG_FIELD_COUNT; field_index++) {
		config_value[field_index] = config_storage +
			field_index * INADYN_CONFIG_VALUE_SIZE;
		fields[field_index].param_name = inadyn_config_nvram_names[field_index];
		fields[field_index].destination = config_value[field_index];
		fields[field_index].destination_size = INADYN_CONFIG_VALUE_SIZE;
	}
	memset(&transaction, 0, sizeof(transaction));
	transaction.lock_fd = -1;
	read_status = rc_wan_nvram_copy_snapshot(
		fields, INADYN_CFG_FIELD_COUNT, inadyn_nvram_snapshot_begin,
		inadyn_nvram_snapshot_get, inadyn_nvram_snapshot_end,
		&transaction);
	if (read_status != 0) {
		saved_errno = read_status;
		logmessage(LOGNAME,
			   "cannot snapshot Inadyn configuration NVRAM (status=%d)",
			   saved_errno);
		goto failed;
	}

	/* Parse every numeric option from one copied NVRAM generation. */
	if ((read_status = inadyn_parse_config_integer("ddns_period",
			config_value[INADYN_CFG_DDNS_PERIOD], 0, INT_MIN, INT_MAX,
			&i_ddns_period)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns_forced",
			config_value[INADYN_CFG_DDNS_FORCED], 0, INT_MIN, INT_MAX,
			&i_ddns_forced)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns_source",
			config_value[INADYN_CFG_DDNS_SOURCE], 0, 0, 2,
			&i_ddns_source)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns_ipv6",
			config_value[INADYN_CFG_DDNS_IPV6], 0, 0, 1,
			&i_ddns_ipv6)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns_ssl",
			config_value[INADYN_CFG_DDNS_SSL], 0, 0, 1,
			&i_ddns1_ssl)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns_wildcard_x",
			config_value[INADYN_CFG_DDNS_WILDCARD], 0, 0, 1,
			&i_ddns1_wildcard)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns2_ssl",
			config_value[INADYN_CFG_DDNS2_SSL], 0, 0, 1,
			&i_ddns2_ssl)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns2_wildcard_x",
			config_value[INADYN_CFG_DDNS2_WILDCARD], 0, 0, 1,
			&i_ddns2_wildcard)) != 0) {
		saved_errno = read_status;
		goto failed;
	}

	if (i_ddns_period < 0 || i_ddns_period > 72)
		i_ddns_period = 24;
	i_ddns_period *= 3600;
	if (i_ddns_period < 600)
		i_ddns_period = 600; // 10 min
	if (i_ddns_forced < 3 || i_ddns_forced > 50)
		i_ddns_forced = 10;
	i_ddns_forced *= (24 * 3600);

	wan_ifname[0] = 0;
	if (i_ddns_source == 1) {
		interface_status = get_wan_ifname(wan_ifname);
		if (interface_status != 0) {
			saved_errno = interface_status;
			logmessage(LOGNAME, "Inadyn config rejected: unsafe WAN interface name (status=%d)",
				   interface_status);
			goto failed;
		}
	} else if (i_ddns_source == 2) {
		management_ifname = get_man_ifname(0);
		if (management_ifname != NULL && management_ifname[0] != '\0') {
			interface_status = wan_ifname_copy(wan_ifname, sizeof(wan_ifname),
							 management_ifname);
			if (interface_status != 0) {
				saved_errno = interface_status;
				logmessage(LOGNAME, "Inadyn config rejected: unsafe management interface name (status=%d)",
					   interface_status);
				goto failed;
			}
		}
	}

	i_max = ARRAY_SIZE(checkip_urls) - 1;
	if ((read_status = inadyn_parse_config_integer("ddns_checkip",
			config_value[INADYN_CFG_DDNS_CHECKIP], 0, 0, i_max,
			&i_ddns1_checkip)) != 0 ||
	    (read_status = inadyn_parse_config_integer("ddns2_checkip",
			config_value[INADYN_CFG_DDNS2_CHECKIP], 0, 0, i_max,
			&i_ddns2_checkip)) != 0) {
		saved_errno = read_status;
		goto failed;
	}

	ddns1_svc = get_inadyn_system(config_value[INADYN_CFG_DDNS_SERVER]);
	if (!ddns1_svc) {
		ddns1_svc = inadyn_systems[0].system;
		if (nvram_set("ddns_server_x", inadyn_systems[0].alias) != 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot persist fallback Inadyn provider (errno=%d)", saved_errno);
			errno = saved_errno;
			goto failed;
		}
	}

	ddns1_hname[0] = config_value[INADYN_CFG_DDNS_HOSTNAME];
	ddns1_hname[1] = config_value[INADYN_CFG_DDNS_HOSTNAME2];
	ddns1_hname[2] = config_value[INADYN_CFG_DDNS_HOSTNAME3];
	ddns1_user     = config_value[INADYN_CFG_DDNS_USERNAME];
	ddns1_pass     = config_value[INADYN_CFG_DDNS_PASSWORD];
	ddns1_svr      = config_value[INADYN_CFG_DDNS_CUSTOM_SERVER];
	ddns1_url      = config_value[INADYN_CFG_DDNS_CUSTOM_URL];

	ddns2_svc = get_inadyn_system(config_value[INADYN_CFG_DDNS2_SERVER]);
	ddns2_hname = config_value[INADYN_CFG_DDNS2_HOSTNAME];
	ddns2_user  = config_value[INADYN_CFG_DDNS2_USERNAME];
	ddns2_pass  = config_value[INADYN_CFG_DDNS2_PASSWORD];

	fp = inadyn_config_file_open(conf_file, staged_path, sizeof(staged_path));
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot stage Inadyn configuration (errno=%d)", saved_errno);
		goto failed;
	}
	if (strlen(wan_ifname) > 0)
		fprintf(fp, "iface = %s\n", wan_ifname);
	fprintf(fp, "period = %d\n", i_ddns_period);
	fprintf(fp, "forced-update = %d\n", i_ddns_forced);
	fprintf(fp, "allow-ipv6 = %s\n", i_ddns_ipv6 ? "true" : "false");
	fprintf(fp, "secure-ssl = false\n");
	fprintf(fp, "broken-rtc = true\n");

	/* DDNS 1*/
	if (strcmp(ddns1_svc, "custom") == 0) {
		fprintf(fp, "custom %s:1 {\n", "http_srv_basic_auth");
	} else {
		fprintf(fp, "provider %s:1 {\n", ddns1_svc);
	}
	fprintf(fp, "  ssl = %s\n", i_ddns1_ssl ? "true" : "false");
	fprintf(fp, "  wildcard = %s\n", i_ddns1_wildcard ? "true" : "false");
	if (strlen(ddns1_user) > 0 &&
	    inadyn_config_file_write_quoted_setting(fp, "  username = \"", ddns1_user,
					    "\"\n") != 0)
		goto write_failed;
	if (strlen(ddns1_pass) > 0 &&
	    inadyn_config_file_write_quoted_setting(fp, "  password = \"", ddns1_pass,
					    "\"\n") != 0)
		goto write_failed;
	if (ddns1_svr && *ddns1_svr && ddns1_url && *ddns1_url) {
		/* custom ddns server */
		if (inadyn_config_file_write_quoted_setting(fp, "  ddns-server = \"", ddns1_svr,
						    "\"\n") != 0 ||
		    inadyn_config_file_write_quoted_setting(fp, "  ddns-path = \"/",
						    ddns1_url, "\"\n") != 0)
			goto write_failed;
	}
	if (i_ddns_source == 0 && strlen(checkip_urls[i_ddns1_checkip].server) > 0) {
		fprintf(fp, "  checkip-server = \"%s\"\n", checkip_urls[i_ddns1_checkip].server);
		fprintf(fp, "  checkip-path = \"%s\"\n", checkip_urls[i_ddns1_checkip].path);
		fprintf(fp, "  checkip-ssl = \"%s\"\n", checkip_urls[i_ddns1_checkip].ssl ? "true" : "false");
	}
	if (inadyn_config_file_write_quoted_setting(fp, "  hostname = { \"", ddns1_hname[0],
					    "\" }\n") != 0)
		goto write_failed;
	if (strlen(ddns1_hname[1]) > 2 &&
	    inadyn_config_file_write_quoted_setting(fp, "  hostname += { \"", ddns1_hname[1],
					    "\" }\n") != 0)
		goto write_failed;
	if (strlen(ddns1_hname[2]) > 2 &&
	    inadyn_config_file_write_quoted_setting(fp, "  hostname += { \"", ddns1_hname[2],
					    "\" }\n") != 0)
		goto write_failed;
	fprintf(fp, "}\n");

	/* DDNS 2*/
	if (ddns2_svc) {
		fprintf(fp, "provider %s:2 {\n", ddns2_svc);
		fprintf(fp, "  ssl = %s\n", i_ddns2_ssl ? "true" : "false");
		fprintf(fp, "  wildcard = %s\n", i_ddns2_wildcard ? "true" : "false");
		if (i_ddns_source == 0 && strlen(checkip_urls[i_ddns2_checkip].server) > 0) {
			fprintf(fp, "  checkip-server = \"%s\"\n", checkip_urls[i_ddns2_checkip].server);
			fprintf(fp, "  checkip-path = \"%s\"\n", checkip_urls[i_ddns2_checkip].path);
			fprintf(fp, "  checkip-ssl = \"%s\"\n", checkip_urls[i_ddns2_checkip].ssl ? "true" : "false");
		}
		if (strlen(ddns2_user) > 0 &&
		    inadyn_config_file_write_quoted_setting(fp, "  username = \"", ddns2_user,
						    "\"\n") != 0)
			goto write_failed;
		if (strlen(ddns2_pass) > 0 &&
		    inadyn_config_file_write_quoted_setting(fp, "  password = \"", ddns2_pass,
						    "\"\n") != 0)
			goto write_failed;
		if (inadyn_config_file_write_quoted_setting(fp, "  hostname = \"", ddns2_hname,
						    "\"\n") != 0)
			goto write_failed;
		fprintf(fp, "}\n");
	}

	load_user_config(fp, INADYN_USER_DIR, "inadyn.conf", NULL);

	if (inadyn_config_file_publish(fp, staged_path, conf_file) != 0) {
		saved_errno = errno ? errno : EIO;
		fp = NULL;
		logmessage(LOGNAME, "cannot publish Inadyn configuration (errno=%d)", saved_errno);
		goto failed;
	}
	fp = NULL;
	inadyn_config_file_clear_sensitive(
		config_storage, INADYN_CFG_FIELD_COUNT * INADYN_CONFIG_VALUE_SIZE);
	free(config_storage);
	return 1;

write_failed:
	saved_errno = errno ? errno : EIO;
	logmessage(LOGNAME, "refusing unsafe or unwriteable Inadyn configuration value (errno=%d)",
		   saved_errno);

failed:
	if (!saved_errno)
		saved_errno = errno ? errno : EIO;
	if (fp) {
		if (inadyn_config_file_discard(fp, staged_path) != 0)
			logmessage(LOGNAME, "cannot discard incomplete Inadyn configuration (errno=%d)",
				   errno ? errno : EIO);
		fp = NULL;
	} else if (staged_path[0] &&
		   inadyn_config_file_discard(NULL, staged_path) != 0) {
		logmessage(LOGNAME, "cannot remove incomplete Inadyn configuration (errno=%d)",
			   errno ? errno : EIO);
	}
	inadyn_config_file_clear_sensitive(
		config_storage, INADYN_CFG_FIELD_COUNT * INADYN_CONFIG_VALUE_SIZE);
	free(config_storage);
	errno = saved_errno;
	return 0;
}

static enum process_presence
inadyn_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence state;

	state = process_inventory_scan("inadyn", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = (scan_error && *scan_error > 0) ? *scan_error : EIO;
	return state;
}

static int
inadyn_confirm_started(int launch_status)
{
	int attempt;
	int scan_error = 0;
	int status;
	enum process_presence state = PROCESS_PRESENCE_ABSENT;

	if (launch_status != 0)
		return inadyn_start_status(launch_status, state, 0);

	/* Inadyn daemonizes by default. The launcher exit status does not prove
	 * that the detached process survived startup; --no-pidfile means procfs
	 * inventory is the postcondition source.
	 */
	for (attempt = 0; attempt < INADYN_START_WAIT_ATTEMPTS; attempt++) {
		state = inadyn_process_presence(&scan_error);
		status = inadyn_start_status(launch_status, state, scan_error);
		if (state == PROCESS_PRESENCE_RUNNING)
			return 0;
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			logmessage(LOGNAME,
				   "cannot verify Inadyn startup process inventory (errno=%d)",
				   status);
			return status;
		}
		if (state != PROCESS_PRESENCE_ABSENT)
			return status;
		if (attempt + 1 < INADYN_START_WAIT_ATTEMPTS) {
			status = service_start_wait_one_second("Inadyn");
			if (status != 0)
				return status;
		}
	}

	status = inadyn_start_status(launch_status, state, scan_error);
	logmessage(LOGNAME,
		   "Inadyn launcher returned success but no daemon appeared within %d seconds",
		   INADYN_START_WAIT_ATTEMPTS - 1);
	return status;
}

static int
inadyn_cache_clear_cb(void *context, const char *path)
{
	int command_status;

	(void)context;
	command_status = doSystem("rm -rf %s", path);
	return inadyn_cache_command_status(command_status, errno);
}

static int
inadyn_cache_mkdir_cb(void *context, const char *path, mode_t mode)
{
	(void)context;
	return mkdir(path, mode);
}

static int
inadyn_cache_lstat_cb(void *context, const char *path, struct stat *st)
{
	(void)context;
	return lstat(path, st);
}

static int
inadyn_cache_chmod_cb(void *context, const char *path, mode_t mode)
{
	(void)context;
	return chmod(path, mode);
}

/*
 * Cache/config preparation can fail after the old Inadyn daemon has been
 * stopped. The config writer is atomic, so these pre-launch failures leave
 * the last-known-good file intact; restore its daemon before returning.
 */
static int
inadyn_restore_previous_instance(int previous_running)
{
	struct stat config_stat, cache_stat;
	enum process_presence state;
	enum inadyn_restore_action action;
	int scan_error = 0, status, verbose_idx, launch_status;
	char *ddns_verbose_str[] = { "err", "notice", "debug" };
	char *startup_delay;

	if (!previous_running)
		return 0;

	state = inadyn_process_presence(&scan_error);
	action = inadyn_restore_action_for_state(previous_running, state);
	if (action == INADYN_RESTORE_NOOP)
		return 0;
	if (action == INADYN_RESTORE_ABORT) {
		status = inadyn_presence_error(state, scan_error);
		errno = status;
		logmessage(LOGNAME,
			   "cannot safely restore previous Inadyn daemon (presence=%d, status=%d)",
			   state, status);
		return status;
	}

	if (lstat(DDNS_CONF_FILE, &config_stat) != 0) {
		status = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot restore previous Inadyn daemon: last-known-good config is unavailable (errno=%d)",
			   status);
		errno = status;
		return status;
	}
	if (!S_ISREG(config_stat.st_mode) || config_stat.st_uid != geteuid() ||
	    config_stat.st_nlink != 1 || (config_stat.st_mode & 077) != 0) {
		errno = EPERM;
		logmessage(LOGNAME,
			   "cannot restore previous Inadyn daemon: config is not a private regular file");
		return EPERM;
	}


	if (lstat(DDNS_CACHE_DIR, &cache_stat) != 0) {
		status = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot restore previous Inadyn daemon: cache directory is unavailable (errno=%d)",
			   status);
		errno = status;
		return status;
	}
	if (!S_ISDIR(cache_stat.st_mode) || cache_stat.st_uid != geteuid() ||
	    (cache_stat.st_mode & 0777) != 0700) {
		errno = !S_ISDIR(cache_stat.st_mode) ? ENOTDIR : EPERM;
		logmessage(LOGNAME,
			   "cannot restore previous Inadyn daemon: cache path is not a private owned directory");
		return errno;
	}
	verbose_idx = nvram_safe_get_int("ddns_verbose", 0, 0, 2);
	startup_delay = (!is_ntpc_updated()) ? "30" : "3";
	launch_status = eval("/bin/inadyn",
		"--no-pidfile",
		"-l", ddns_verbose_str[verbose_idx],
		"-t", startup_delay,
		"-f", DDNS_CONF_FILE,
		"--cache-dir", DDNS_CACHE_DIR,
		"-e", DDNS_DONE_SCRIPT,
		"--exec-mode", "compat");
	status = inadyn_confirm_started(launch_status);
	if (status != 0)
		logmessage(LOGNAME,
			   "previous Inadyn daemon restoration failed (status=%d)", status);
	else
		logmessage(LOGNAME,
			   "restored previous Inadyn daemon after an unsuccessful configuration refresh");
	return status;
}

/*
 * The previous daemon was confirmed absent before the fresh launch. If the
 * detached launcher or its readiness wait fails, remove only this attempt,
 * then verify absence. UNKNOWN remains fail-closed and the original error is
 * preserved for rc's caller.
 */
static int
inadyn_rollback_failed_start(int start_status, int start_errno,
			     const char *operation)
{
	int stop_status;
	int scan_error = 0;
	enum process_presence presence;

	stop_status = stop_ddns_locked();
	presence = inadyn_process_presence(&scan_error);
	if (inadyn_failed_start_may_leave_daemon(presence)) {
		logmessage(LOGNAME,
			   "Inadyn may remain active after failed %s (start_status=%d, rollback_status=%d, state=%d, scan_error=%d)",
			   operation, start_status, stop_status, presence,
			   scan_error);
	} else if (stop_status != 0) {
		logmessage(LOGNAME,
			   "Inadyn is absent after failed %s rollback but stop reported status=%d",
			   operation, stop_status);
	}
	errno = start_errno > 0 ? start_errno : EIO;
	return start_status;
}

static int
start_ddns_locked(int clear_cache)
{
	int verbose_idx = nvram_safe_get_int("ddns_verbose", 0, 0, 2);
	char* ddns_verbose_str[] = { "err", "notice", "debug" };
	char* startup_delay;
	int stop_status;
	int cache_status;
	int launch_status;
	int launch_errno;
	int start_status;
	int start_errno;
	int previous_running = 0;
	int scan_error = 0;
	int restore_status;
	int saved_status;
	int is_ap_mode, ap_mode_status;
	int enabled_api_result, enabled_status, ddns_enabled;
	int saved_errno;
	char *enabled_value = NULL;
	enum process_presence previous_presence;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		errno = ap_mode_status;
		logmessage(LOGNAME,
			   "Inadyn start aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return -1;

	/*
	 * Read the enable flag before stopping the existing daemon. An NVRAM
	 * backend error must not masquerade as "disabled" and take DDNS offline.
	 * A confirmed disabled value still follows the legacy stop-then-return
	 * behavior below.
	 */
	errno = 0;
	enabled_api_result = nvram_get_checked("ddns_enable_x", &enabled_value);
	saved_errno = errno;
	enabled_status = inadyn_ddns_enabled_status(
		enabled_api_result, saved_errno, enabled_value, &ddns_enabled);
	if (enabled_status != 0) {
		errno = enabled_status;
		logmessage(LOGNAME,
			   "Inadyn start aborted because enable state cannot be read (status=%d)",
			   enabled_status);
		return enabled_status;
	}

	/* Do not destroy the current DDNS service when its state is unknown. */
	previous_presence = inadyn_process_presence(&scan_error);
	if (previous_presence == PROCESS_PRESENCE_UNKNOWN) {
		saved_status = inadyn_presence_error(previous_presence, scan_error);
		errno = saved_status;
		logmessage(LOGNAME,
			   "Inadyn start aborted: cannot verify previous daemon state (status=%d)",
			   saved_status);
		return saved_status;
	}
	if (previous_presence != PROCESS_PRESENCE_ABSENT &&
	    previous_presence != PROCESS_PRESENCE_RUNNING) {
		errno = EIO;
		return EIO;
	}
	previous_running = (previous_presence == PROCESS_PRESENCE_RUNNING);

	stop_status = stop_ddns_locked();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "inadyn start aborted because previous instance did not stop (status %d)",
			   stop_status);
		return stop_status;
	}

	if (!ddns_enabled)
		return -1;

	cache_status = inadyn_prepare_cache_directory(
		clear_cache, DDNS_CACHE_DIR, inadyn_cache_clear_cb,
		inadyn_cache_mkdir_cb, inadyn_cache_lstat_cb,
		inadyn_cache_chmod_cb, NULL);
	if (cache_status != 0) {
		saved_status = cache_status > 0 ? cache_status : (errno ? errno : EIO);
		logmessage(LOGNAME,
			   "Inadyn cache preparation failed (status=%d, errno=%d)",
			   cache_status, saved_status);
		if (previous_running) {
			restore_status = inadyn_restore_previous_instance(previous_running);
			if (restore_status != 0)
				logmessage(LOGNAME,
					   "Inadyn remains stopped after cache failure (restore status=%d)",
					   restore_status);
		}
		errno = saved_status;
		return cache_status;
	}

	if (!write_inadyn_conf(DDNS_CONF_FILE)) {
		saved_status = errno ? errno : EIO;
		if (previous_running) {
			restore_status = inadyn_restore_previous_instance(previous_running);
			if (restore_status != 0)
				logmessage(LOGNAME,
					   "Inadyn remains stopped after config failure (restore status=%d)",
					   restore_status);
		}
		errno = saved_status;
		return -1;
	}

	startup_delay = (!is_ntpc_updated()) ? "30" : "3";

	errno = 0;
	launch_status = eval("/bin/inadyn",
		"--no-pidfile",
		"-l", ddns_verbose_str[verbose_idx],
		"-t", startup_delay,
		"-f", DDNS_CONF_FILE,
		"--cache-dir", DDNS_CACHE_DIR,
		"-e", DDNS_DONE_SCRIPT,
		"--exec-mode", "compat");
	launch_errno = errno;
	if (launch_status != 0) {
		start_errno = launch_errno > 0 ? launch_errno :
			(launch_status > 0 ? launch_status : EIO);
		logmessage(LOGNAME, "Inadyn launcher failed (status=%d)", launch_status);
		return inadyn_rollback_failed_start(
			launch_status, start_errno, "launcher");
	}

	start_status = inadyn_confirm_started(launch_status);
	if (start_status != 0) {
		start_errno = errno > 0 ? errno :
			(start_status > 0 ? start_status : EIO);
		return inadyn_rollback_failed_start(
			start_status, start_errno, "startup confirmation");
	}
	return 0;
}

int
start_ddns(int clear_cache)
{
	int lock_fd, lock_status, status;
	lock_status = inadyn_lifecycle_lock_acquire_status("start", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = start_ddns_locked(clear_cache);
	return inadyn_lifecycle_lock_finish(lock_fd, status, "start");
}

static enum process_presence
inadyn_validate_identity(void *context,
			 const struct process_identity *identity)
{
	(void)context;
	return process_identity_validate("inadyn", identity);
}

static int
inadyn_send_signal(void *context, pid_t pid, int signal_number)
{
	(void)context;
	return kill(pid, signal_number);
}

static int
notify_ddns_update_locked(void)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	enum inadyn_daemon_action action;
	int scan_error;
	int result;
	int saved_errno;
	int is_ap_mode, ap_mode_status;
	int enabled_api_result, enabled_status, ddns_enabled;
	char *enabled_value = NULL;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		errno = ap_mode_status;
		logmessage(LOGNAME,
			   "DDNS update aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	errno = 0;
	enabled_api_result = nvram_get_checked("ddns_enable_x", &enabled_value);
	saved_errno = errno;
	enabled_status = inadyn_ddns_enabled_status(
		enabled_api_result, saved_errno, enabled_value, &ddns_enabled);
	if (enabled_status != 0) {
		errno = enabled_status;
		logmessage(LOGNAME,
			   "DDNS update aborted because enable state cannot be read (status=%d)",
			   enabled_status);
		return enabled_status;
	}
	if (!inadyn_update_should_run(ddns_enabled, is_ap_mode))
		return 0;

	state = process_inventory_scan("inadyn", &snapshot);
	action = inadyn_action_for_presence(state);
	if (action == INADYN_DAEMON_ABORT) {
		scan_error = snapshot.error > 0 ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = inadyn_presence_error(state, scan_error);
		logmessage(LOGNAME,
			   "DDNS update aborted: inadyn process inventory unknown (errno %d)",
			   errno);
		return -1;
	}
	if (action == INADYN_DAEMON_START) {
		process_snapshot_release(&snapshot);
		return start_ddns_locked(0);
	}

	if (inadyn_validate_snapshot(&snapshot, inadyn_validate_identity, NULL) != 0) {
		saved_errno = errno ? errno : EIO;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		logmessage(LOGNAME, "DDNS update aborted: inadyn identity preflight failed (errno %d)",
			   errno);
		return -1;
	}
	if (!write_inadyn_conf(DDNS_CONF_FILE)) {
		saved_errno = errno ? errno : EIO;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		return -1;
	}

	result = inadyn_signal_snapshot(&snapshot, SIGHUP, inadyn_validate_identity,
					inadyn_send_signal, NULL);
	saved_errno = errno;
	process_snapshot_release(&snapshot);
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		logmessage(LOGNAME, "inadyn configuration reload signal failed (errno %d)",
			   errno);
		return -1;
	}

	return 0;
}

int
notify_ddns_update(void)
{
	int lock_fd, lock_status, status;
	lock_status = inadyn_lifecycle_lock_acquire_status("update", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = notify_ddns_update_locked();
	return inadyn_lifecycle_lock_finish(lock_fd, status, "update");
}

/*
 * Manual UI refresh sends SIGUSR1 only to identities captured by the process
 * inventory. ABSENT remains a no-op; UNKNOWN must fail closed.
 */
static int
notify_ddns_manual_update_locked(void)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int result;
	int saved_errno;

	state = process_inventory_scan("inadyn", &snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = snapshot.error > 0 ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		logmessage(LOGNAME,
			   "manual DDNS refresh aborted: inadyn inventory unknown (errno %d)",
			   errno);
		return -1;
	}
	if (state == PROCESS_PRESENCE_ABSENT) {
		process_snapshot_release(&snapshot);
		return 0;
	}
	if (state != PROCESS_PRESENCE_RUNNING) {
		process_snapshot_release(&snapshot);
		errno = EINVAL;
		return -1;
	}
	if (inadyn_validate_snapshot(&snapshot, inadyn_validate_identity, NULL) != 0) {
		saved_errno = errno ? errno : EIO;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		logmessage(LOGNAME,
			   "manual DDNS refresh aborted: inadyn identity preflight failed (errno %d)",
			   errno);
		return -1;
	}
	result = inadyn_signal_snapshot(&snapshot, SIGUSR1, inadyn_validate_identity,
					inadyn_send_signal, NULL);
	saved_errno = errno;
	process_snapshot_release(&snapshot);
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		logmessage(LOGNAME, "manual DDNS refresh signal failed (errno %d)", errno);
		return -1;
	}
	return 0;
}

int
notify_ddns_manual_update(void)
{
	int lock_fd, lock_status, status;
	lock_status = inadyn_lifecycle_lock_acquire_status("manual update", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = notify_ddns_manual_update_locked();
	return inadyn_lifecycle_lock_finish(lock_fd, status, "manual update");
}

char *
get_ddns_fqdn(void)
{
	char *ddns_fqdn = NULL;
	const char *ddns_svc;

	if (!nvram_match("ddns_enable_x", "1"))
		return NULL;

	ddns_svc = get_inadyn_system(nvram_safe_get("ddns_server_x"));
	if (ddns_svc && strncmp(ddns_svc, "ipv6tb@", 7) != 0 && !strstr(ddns_svc, "tunnelbroker.net")) {
		ddns_fqdn = nvram_safe_get("ddns_hostname_x");
		if (strcmp(ddns_svc, "update@asus.com") == 0) {
			if (!strstr(ddns_fqdn, ".asuscomm.com"))
				ddns_fqdn = NULL;
		} else {
			if (!strchr(ddns_fqdn, '.')) {
				ddns_fqdn = nvram_safe_get("ddns_hostname2_x");
				if (!strchr(ddns_fqdn, '.')) {
					ddns_fqdn = nvram_safe_get("ddns_hostname3_x");
					if (!strchr(ddns_fqdn, '.'))
						ddns_fqdn = NULL;
				}
			}
		}
	}

	if (!ddns_fqdn) {
		ddns_svc = get_inadyn_system(nvram_safe_get("ddns2_server"));
		if (ddns_svc && strncmp(ddns_svc, "ipv6tb@", 7) != 0 && !strstr(ddns_svc, "tunnelbroker.net")) {
			ddns_fqdn = nvram_safe_get("ddns2_hname");
			if (!strchr(ddns_fqdn, '.'))
				ddns_fqdn = NULL;
		}
	}

	return ddns_fqdn;
}

void
manual_ddns_hostname_check(void)
{
	nvram_set_temp("ddns_return_code", "inadyn_unsupport");
}

