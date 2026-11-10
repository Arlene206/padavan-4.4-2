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
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "rc.h"
#include "firewall_ppp_interface_policy.h"
#include "vpn_server_lease_policy.h"
#include "vpn_server_lease_lock.h"
#include "vpn_server_lifecycle_lock.h"
#include "vpn_server_callback_policy.h"

#define VPNS_LOG_NAME		"VPN server"
#define VPNS_PPP_UP_SCRIPT	"/tmp/ppp/ip-up.vpns"
#define VPNS_PPP_DW_SCRIPT	"/tmp/ppp/ip-down.vpns"

static int xl2tpd_killed_vpns = 0;

/* Keep the VPN lease index root-private even though the caller uses umask(0000).
 * O_NOFOLLOW, descriptor checks and fchmod prevent shared-/tmp replacement and
 * world-writable lease entries from becoming firewall policy input. */
/* Truncate only the exact lease inode opened before a failed append. */
static int
rollback_vpns_lease_append(const struct stat *original, off_t expected_size)
{
	int fd, saved_errno;
	struct stat current;
	fd = open(VPN_SERVER_LEASE_FILE, O_WRONLY | O_NOFOLLOW);
	if (fd < 0)
		return -1;
	if (fstat(fd, &current) != 0)
		goto failed;
	if (!S_ISREG(current.st_mode) || current.st_uid != 0 ||
	    current.st_nlink != 1 || current.st_dev != original->st_dev ||
	    current.st_ino != original->st_ino || current.st_size != expected_size) {
		errno = ESTALE;
		goto failed;
	}
	if (ftruncate(fd, original->st_size) != 0 || fsync(fd) != 0)
		goto failed;
	if (close(fd) != 0)
		return -1;
	return 0;
failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static int
append_vpns_lease_record(const char *record, size_t record_length)
{
	int fd = -1, read_fd = -1, append_started = 0;
	int saved_errno, close_status;
	FILE *reader = NULL;
	struct stat original;
	off_t expected_size;
	char interfaces[MAX_CLIENTS_NUM][VPN_SERVER_LEASE_IFNAME_SIZE];
	size_t interface_count = 0, i;

	fd = open(VPN_SERVER_LEASE_FILE,
		  O_RDWR | O_APPEND | O_CREAT | O_NOFOLLOW,
		  S_IRUSR | S_IWUSR);
	if (fd < 0)
		return -1;
	if (fstat(fd, &original) != 0)
		goto failed;
	if (!S_ISREG(original.st_mode) || original.st_uid != 0 ||
	    original.st_nlink != 1 || original.st_size < 0) {
		errno = EPERM;
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;
	read_fd = dup(fd);
	if (read_fd < 0)
		goto failed;
	reader = fdopen(read_fd, "r");
	if (!reader)
		goto failed;
	read_fd = -1;
	if (vpn_server_lease_read_inventory(reader, interfaces,
					    MAX_CLIENTS_NUM, &interface_count) != 0)
		goto failed;
	if (fclose(reader) != 0) {
		reader = NULL;
		if (errno == 0)
			errno = EIO;
		goto failed;
	}
	reader = NULL;
	for (i = 0; i < interface_count; ++i) {
		if (!firewall_ppp_interface_is_safe(interfaces[i], VPN_SERVER_PPP_UNIT)) {
			errno = EINVAL;
			goto failed;
		}
	}
	if (interface_count >= MAX_CLIENTS_NUM) {
		errno = EOVERFLOW;
		goto failed;
	}
	expected_size = original.st_size + (off_t)record_length;
	if (expected_size < original.st_size) {
		errno = EOVERFLOW;
		goto failed;
	}
	append_started = 1;
	if (vpn_server_lease_append_record(fd, record, record_length) != 0)
		goto failed;
	close_status = close(fd);
	fd = -1;
	if (close_status != 0) {
		saved_errno = errno ? errno : EIO;
		if (rollback_vpns_lease_append(&original, expected_size) != 0)
			logmessage(VPNS_LOG_NAME,
				   "cannot roll back uncertain VPN lease append (errno=%d)",
				   errno ? errno : EIO);
		errno = saved_errno;
		return -1;
	}
	return 0;
failed:
	saved_errno = errno ? errno : EIO;
	if (reader)
		(void)fclose(reader);
	if (read_fd >= 0)
		(void)close(read_fd);
	if (fd >= 0) {
		if (append_started &&
		    (ftruncate(fd, original.st_size) != 0 || fsync(fd) != 0))
			logmessage(VPNS_LOG_NAME,
				   "cannot roll back failed VPN lease append (errno=%d)",
				   errno ? errno : EIO);
		(void)close(fd);
	}
	errno = saved_errno;
	return -1;
}

static int
clear_vpns_lease_file(void)
{
	int lock_fd, result = 0, saved_errno = 0;
	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0)
		return -1;
	if (unlink(VPN_SERVER_LEASE_FILE) != 0 && errno != ENOENT) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (vpn_server_lease_lock_release(lock_fd) != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0)
		errno = saved_errno;
	return result;
}


static int
get_xl2tpd_vpns_active(void)
{
	return ((nvram_match("vpns_enable", "1") && nvram_match("vpns_type", "1")) || 
		 nvram_match("l2tp_srv_t", "1"));
}

static int
gen_vpns_pppd_options(int vpns_type)
{
	FILE *fp;
	int i_mppe, i_auth, i_vuse, i_dhcp, i_items;
	char *lanip, *wins, *dns1, *dns2;
	const char *vpns_opt = VPN_SERVER_PPPD_OPTIONS;

	i_auth = nvram_get_int("vpns_auth");
	i_mppe = nvram_get_int("vpns_mppe");
	i_vuse = nvram_get_int("vpns_vuse");
	i_dhcp = is_dhcpd_enabled(0);

	lanip  = nvram_safe_get("lan_ipaddr");

	if (!(fp = fopen(vpns_opt, "w")))
		return -1;

	fprintf(fp, "name %s\n", get_our_hostname());
	fprintf(fp, "auth\n");
	fprintf(fp, "refuse-eap\n");
	fprintf(fp, "refuse-pap\n");
	fprintf(fp, "refuse-mschap\n");

	if (i_auth == 0) {
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "require-mschap-v2\n");
	}

	fprintf(fp, "default-asyncmap\n");

	/* looks like pptp also likes them */
	fprintf(fp, "nopcomp noaccomp\n");

	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	if (i_mppe == 3) {
		fprintf(fp, "nomppe nomppc\n");
	} else {
		if (i_mppe == 1) {
			fprintf(fp, "+mppe\n");
			fprintf(fp, "-mppe-40\n");
			fprintf(fp, "+mppe-128\n");
		} else if (i_mppe == 2) {
			fprintf(fp, "+mppe\n");
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "-mppe-128\n");
		} else {
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "+mppe-128\n");
		}
		fprintf(fp, "nomppe-stateful\n");
	}

	// DNS Server
	i_items = 0;
	if (i_dhcp) {
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		if (is_valid_ipv4(dns1)) {
			i_items++;
			fprintf(fp, "ms-dns %s\n", dns1);
		}
		if (is_valid_ipv4(dns2) && strcmp(dns2, dns1)) {
			i_items++;
			fprintf(fp, "ms-dns %s\n", dns2);
		}
	}

	if (i_items < 1)
		fprintf(fp, "ms-dns %s\n", lanip);

	// WINS Server
	i_items = 0;
	if (i_dhcp) {
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins)) {
			i_items++;
			fprintf(fp, "ms-wins %s\n", wins);
		}
	}

#if defined(APP_SMBD) || defined(APP_NMBD)
	if ((i_items < 1) && nvram_get_int("wins_enable"))
		fprintf(fp, "ms-wins %s\n", lanip);
#endif

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("vpns_mtu", 1450, 1000, 1460));
	fprintf(fp, "mru %d\n", nvram_safe_get_int("vpns_mru", 1450, 1000, 1460));
	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "nodefaultroute\n");

	if (i_vuse == 0)
		fprintf(fp, "proxyarp\n");

	if (vpns_type == 1) {
		// L2TP: Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	/* echo failures (6*20s) */
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);
	fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "ip-up-script %s\n", VPNS_PPP_UP_SCRIPT);
	fprintf(fp, "ip-down-script %s\n", VPNS_PPP_DW_SCRIPT);
	fprintf(fp, "minunit %d\n", VPN_SERVER_PPP_UNIT);

	fclose(fp);

	chmod(vpns_opt, 0644);

	return 0;
}

static int
start_vpn_server_locked(void)
{
	FILE *fp;
	int i, i_type, i_vuse;
	char *vpns_cfg = "/etc/pptpd.conf";
	const char *vpns_sec = "/tmp/ppp/chap-secrets";
	unsigned int vaddr, vmask, vp_b, vp_e;
	struct in_addr pool_in;

	if (nvram_invmatch("vpns_enable", "1") || get_ap_mode())
		return 0;

	if (clear_vpns_lease_file() != 0) {
		logmessage(VPNS_LOG_NAME, "cannot clear VPN lease inventory safely (errno=%d)", errno);
		return -1;
	}
	i_type = nvram_get_int("vpns_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		return start_openvpn_server();
#endif

	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", VPNS_PPP_UP_SCRIPT);
	symlink("/sbin/rc", VPNS_PPP_DW_SCRIPT);

	i_vuse = nvram_get_int("vpns_vuse");

	get_vpns_pool(i_vuse, &vaddr, &vmask, &vp_b, &vp_e);

	if (i_type != 1) {
		if (!(fp = fopen(vpns_cfg, "w")))
			return -1;
		
		fprintf(fp, "option %s\n", VPN_SERVER_PPPD_OPTIONS);
		fprintf(fp, "connections %d\n", MAX_CLIENTS_NUM);
		
		pool_in.s_addr = htonl(vaddr);
		fprintf(fp, "localip %s\n", inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((vaddr & vmask) | vp_b);
		fprintf(fp, "remoteip %s-%u\n", inet_ntoa(pool_in), vp_e);
		
		fclose(fp);
		
		chmod(vpns_cfg, 0644);
	}

	gen_vpns_pppd_options(i_type);

	/* create /tmp/ppp/chap-secrets */
	fp = fopen(vpns_sec, "w+");
	if (fp) {
		char *acl_user, *acl_pass;
		char acl_user_var[32], acl_pass_var[32], acl_addr_var[32];
		unsigned int vp_a;
		int i_max = nvram_get_int("vpns_num_x");
		
		if (i_max > MAX_CLIENTS_NUM) i_max = MAX_CLIENTS_NUM;
		for (i = 0; i < i_max; i++) {
			snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
			snprintf(acl_pass_var, sizeof(acl_pass_var), "vpns_pass_x%d", i);
			acl_user = nvram_safe_get(acl_user_var);
			acl_pass = nvram_safe_get(acl_pass_var);
			if (*acl_user && *acl_pass) {
				snprintf(acl_addr_var, sizeof(acl_addr_var), "vpns_addr_x%d", i);
				vp_a = (unsigned int)nvram_get_int(acl_addr_var);
				if (vp_a >= vp_b && vp_a <= vp_e ) {
					pool_in.s_addr = htonl((vaddr & vmask) | vp_a);
					strcpy(acl_addr_var, inet_ntoa(pool_in));
				} else
					strcpy(acl_addr_var, "*");
				
				fprintf(fp, "\"%s\"	*	\"%s\"	%s\n", acl_user, acl_pass, acl_addr_var);
			}
		}
		fclose(fp);
		
		chmod(vpns_sec, 0600);
	}

	if (i_type == 1) {
		nvram_set_int_temp("l2tp_srv_t", 1);
		
		safe_start_xl2tpd_locked();
	} else {
		nvram_set_int_temp("l2tp_srv_t", 0);
		
		/* execute pptpd daemon */
		return eval("/usr/sbin/pptpd", "-c", vpns_cfg);
	}

	return 0;
}

int
start_vpn_server(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNS_LOG_NAME, "cannot lock VPN server lifecycle for start (errno=%d)", errno);
		return -1;
	}
	result = start_vpn_server_locked();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(VPNS_LOG_NAME, "cannot release VPN server lifecycle lock after start (errno=%d)", release_errno);
		if (result == 0) {
			result = -1;
			saved_errno = release_errno;
		}
	}
	if (result != 0 && saved_errno != 0)
		errno = saved_errno;
	return result;
}

static void
stop_vpn_server_locked(void)
{
	int i;
	char pppd_pid[32];
	char* svcs[] = { "bcrelay", "pptpd", NULL,  NULL };

	if (get_xl2tpd_vpns_active()) {
		svcs[2] = "xl2tpd";
		xl2tpd_killed_vpns = 1;
	}

	kill_services(svcs, 5, 1);

	/* force kill all clients pppd */
	for (i=0; i<MAX_CLIENTS_NUM; i++) {
		snprintf(pppd_pid, sizeof(pppd_pid), "/var/run/ppp%d.pid", VPN_SERVER_PPP_UNIT+i);
		kill_pidfile_s(pppd_pid, SIGKILL);
	}

	nvram_set_int_temp("l2tp_srv_t", 0);

	unlink(VPNS_PPP_UP_SCRIPT);
	unlink(VPNS_PPP_DW_SCRIPT);

#if defined(APP_OPENVPN)
	stop_openvpn_server();
#endif

	if (clear_vpns_lease_file() != 0)
		logmessage(VPNS_LOG_NAME, "cannot clear VPN lease inventory safely (errno=%d)", errno);
}

void
stop_vpn_server(void)
{
	int lock_fd;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNS_LOG_NAME, "cannot lock VPN server lifecycle for stop (errno=%d)", errno);
		return;
	}
	stop_vpn_server_locked();
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0)
		logmessage(VPNS_LOG_NAME, "cannot release VPN server lifecycle lock after stop (errno=%d)", errno);
}

void
restart_vpn_server(void)
{
	int lock_fd, start_status;

	if (get_ap_mode())
		return;
	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNS_LOG_NAME, "cannot lock VPN server lifecycle for restart (errno=%d)", errno);
		return;
	}

	xl2tpd_killed_vpns = 0;
	/* Keep teardown, config publication and service restoration atomic across
	 * independent RC applets and PID 1 notifications. */
	stop_vpn_server_locked();
	start_status = start_vpn_server_locked();
	if (start_status != 0)
		logmessage(VPNS_LOG_NAME, "VPN server restart failed (status=%d, errno=%d)", start_status, errno);

	restart_firewall();
#if defined(APP_NFSD)
	reload_nfsd();
#endif
	/* Restore L2TP WAN client/VPNC before releasing the lifecycle lock. */
	if (xl2tpd_killed_vpns && (nvram_match("l2tp_wan_t", "1") || nvram_match("l2tp_cli_t", "1")))
		safe_start_xl2tpd_locked();

	if (vpn_server_lifecycle_lock_release(lock_fd) != 0)
		logmessage(VPNS_LOG_NAME, "cannot release VPN server lifecycle lock after restart (errno=%d)", errno);
}

static void
reapply_vpn_server_locked(void)
{
	int i_type;

	if (nvram_invmatch("vpns_enable", "1") || get_ap_mode())
		return;

	i_type = nvram_get_int("vpns_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		restart_openvpn_server();
	else
#endif
		gen_vpns_pppd_options(i_type);
}

void
reapply_vpn_server(void)
{
	int lock_fd;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNS_LOG_NAME, "cannot lock VPN server lifecycle for reapply (errno=%d)", errno);
		return;
	}
	reapply_vpn_server_locked();
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0)
		logmessage(VPNS_LOG_NAME, "cannot release VPN server lifecycle lock after reapply (errno=%d)", errno);
}


/* The pppd callback is accepted only while its daemon is still the direct
 * parent and its per-interface pid file names that same process. An orphaned
 * callback waits for lifecycle teardown, then fails this identity check. */
static int
vpns_callback_session_is_current(const char *ifname)
{
	const char *pid_text;
	pid_t callback_pid, parent_pid, pidfile_pid;
	char pidfile[64], pid_text_file[32];
	int fd = -1, result = -1, saved_errno;
	struct stat st;
	size_t used = 0;
	ssize_t count;

	if (!firewall_ppp_interface_is_safe(ifname, VPN_SERVER_PPP_UNIT)) {
		errno = EINVAL;
		return -1;
	}
	pid_text = safe_getenv("PPPD_PID");
	if (vpn_server_callback_pid_parse(pid_text, &callback_pid) != 0)
		return 0;

	parent_pid = getppid();
	if (!vpn_server_callback_pids_match(callback_pid, parent_pid, callback_pid))
		return 0;

	if (snprintf(pidfile, sizeof(pidfile), "/var/run/%s.pid", ifname) < 0) {
		errno = EINVAL;
		return -1;
	}
	fd = open(pidfile, O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != 0 || st.st_nlink != 1 ||
	    (st.st_mode & 022) != 0) {
		errno = EPERM;
		goto failed;
	}

	for (;;) {
		count = read(fd, pid_text_file + used,
			     sizeof(pid_text_file) - 1 - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto failed;
		}
		if (count == 0)
			break;
		used += (size_t)count;
		if (used >= sizeof(pid_text_file) - 1) {
			errno = EOVERFLOW;
			goto failed;
		}
	}
	if (used < 2 || pid_text_file[used - 1] != '\n') {
		errno = EINVAL;
		goto failed;
	}
	pid_text_file[--used] = '\0';
	if (vpn_server_callback_pid_parse(pid_text_file, &pidfile_pid) != 0)
		goto failed;
	result = vpn_server_callback_pids_match(callback_pid, parent_pid,
						pidfile_pid);

	if (close(fd) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	return result;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

/* The lifecycle lock remains held across all callback side effects. */
static int
vpns_callback_lifecycle_enter(const char *ifname, int *lifecycle_fd)
{
	int fd, current, saved_errno;

	if (!lifecycle_fd) {
		errno = EINVAL;
		return -1;
	}
	*lifecycle_fd = -1;
	fd = vpn_server_lifecycle_lock_acquire();
	if (fd < 0)
		return -1;
	current = vpns_callback_session_is_current(ifname);
	if (current != 1) {
		saved_errno = errno ? errno : EIO;
		if (vpn_server_lifecycle_lock_release(fd) != 0) {
			logmessage(VPNS_LOG_NAME,
				   "cannot release VPN lifecycle lock after callback identity rejection (errno=%d)",
				   errno ? errno : EIO);
			return -1;
		}
		if (current < 0)
			errno = saved_errno;
		return current;
	}
	*lifecycle_fd = fd;
	return 1;
}

static int
vpns_firewall_permission(char *ifname, int add)
{
	char *argv[8];
	int status;

	/* Keep runtime rule mutation aligned with the strict lease-file parser. */
	if (!firewall_ppp_interface_is_safe(ifname, VPN_SERVER_PPP_UNIT)) {
		logmessage(VPNS_LOG_NAME, "rejecting invalid VPN firewall interface token");
		return -1;
	}

	argv[0] = "/bin/iptables";
	argv[1] = add ? "-A" : "-D";
	argv[2] = IPT_CHAIN_NAME_VPN_LIST;
	argv[3] = "-i";
	argv[4] = ifname;
	argv[5] = "-j";
	argv[6] = "ACCEPT";
	argv[7] = NULL;

	/* _eval returns an ordinary exit code (or errno/wait status), not system()'s
	 * encoded wait status. Surface failed add/delete operations in the RC log. */
	status = _eval(argv, NULL, 0, NULL);
	if (status != 0) {
		logmessage(VPNS_LOG_NAME,
			   "failed to %s firewall permission for %s (status %d)",
			   add ? "add" : "remove", ifname, status);
		return status;
	}

	return 0;
}


/* Caller holds VPN_SERVER_LEASE_LOCK_FILE across firewall deletion and pruning. */
static int
vpns_prune_lease_file(const char *disconnected_ifname, int *remaining_clients)
{
	int lease_fd = -1, temp_fd = -1, line_status, saved_errno, temp_created = 0;
	FILE *input = NULL, *output = NULL;
	struct stat lease_stat;
	char temp_path[] = "/tmp/.vpns.leases.XXXXXX";
	char line[VPN_SERVER_LEASE_LINE_SIZE];
	char ifname[VPN_SERVER_LEASE_IFNAME_SIZE];
	size_t line_length;

	if (!disconnected_ifname || !remaining_clients ||
	    !firewall_ppp_interface_is_safe(disconnected_ifname, VPN_SERVER_PPP_UNIT)) {
		errno = EINVAL;
		return -1;
	}
	*remaining_clients = 0;

	/* Never turn a missing/unreadable source into an empty active-client list. */
	lease_fd = open(VPN_SERVER_LEASE_FILE, O_RDONLY | O_NOFOLLOW);
	if (lease_fd < 0)
		goto failed;
	if (fstat(lease_fd, &lease_stat) != 0)
		goto failed;
	if (!S_ISREG(lease_stat.st_mode) || lease_stat.st_uid != 0 ||
	    lease_stat.st_nlink != 1 || (lease_stat.st_mode & 077) != 0) {
		errno = EPERM;
		goto failed;
	}
	input = fdopen(lease_fd, "r");
	if (!input)
		goto failed;
	lease_fd = -1;

	/* Do not create a replacement until the source inode passed validation. */
	temp_fd = mkstemp(temp_path);
	if (temp_fd < 0)
		goto failed;
	temp_created = 1;
	if (fchmod(temp_fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;
	output = fdopen(temp_fd, "w");
	if (!output)
		goto failed;
	temp_fd = -1;

	for (;;) {
		line_status = vpn_server_lease_read_line(input, line, sizeof(line),
						 &line_length);
		if (line_status == 0)
			break;
		if (line_status < 0)
			goto failed;
		if (vpn_server_lease_parse_line(line, line_length, ifname,
						 sizeof(ifname)) != 0 ||
		    !firewall_ppp_interface_is_safe(ifname, VPN_SERVER_PPP_UNIT)) {
			errno = EINVAL;
			goto failed;
		}
		if (vpn_server_lease_should_keep(ifname, disconnected_ifname)) {
			if (fwrite(line, 1, line_length, output) != line_length) {
				if (errno == 0)
					errno = EIO;
				goto failed;
			}
			++(*remaining_clients);
		}
	}
	if (ferror(input)) {
		if (errno == 0)
			errno = EIO;
		goto failed;
	}
	if (fclose(input) != 0) {
		input = NULL;
		if (errno == 0)
			errno = EIO;
		goto failed;
	}
	input = NULL;

	if (fflush(output) != 0 || ferror(output)) {
		if (errno == 0)
			errno = EIO;
		goto failed;
	}
	if (fclose(output) != 0) {
		output = NULL;
		if (errno == 0)
			errno = EIO;
		goto failed;
	}
	output = NULL;

	if (rename(temp_path, VPN_SERVER_LEASE_FILE) != 0)
		goto failed;
	temp_path[0] = '\0';
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (input)
		(void)fclose(input);
	if (output)
		(void)fclose(output);
	if (lease_fd >= 0)
		(void)close(lease_fd);
	if (temp_fd >= 0)
		(void)close(temp_fd);
	if (temp_created && temp_path[0] && unlink(temp_path) != 0 && errno != ENOENT)
		logmessage(VPNS_LOG_NAME,
			   "cannot remove temporary VPN lease inventory %s (errno=%d)",
			   temp_path, errno);
	*remaining_clients = 0;
	errno = saved_errno;
	return -1;
}

void
get_vpns_pool(int i_vuse, unsigned int *ip_v, unsigned int *ip_m, unsigned int *vp_b, unsigned int *vp_e)
{
	unsigned int lsnet, lpool[2];

	if (i_vuse == 0) {
		*ip_v = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
		*ip_m = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
		lsnet = ~(*ip_m) - 1;
		
		lpool[0] = (unsigned int)nvram_safe_get_int("vpns_cli0", 245, 1, 254);
		lpool[1] = (unsigned int)nvram_safe_get_int("vpns_cli1", 254, 2, 254);
		if (lpool[0] >= lsnet)
			lpool[0] = (lsnet - 1);
		if (lpool[1] > lsnet)
			lpool[1] = lsnet;
		if (lpool[1] <= lpool[0])
			lpool[1] = lpool[0] + 1;
		*ip_v = (*ip_v & *ip_m) | lpool[0];
		lpool[0] += 1;
	} else {
		*ip_v = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
		*ip_m = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
		*ip_v = (*ip_v & *ip_m) | 1;
		
		lpool[0] = 2;
		lpool[1] = lpool[0] + MAX_CLIENTS_NUM - 1;
	}

	*vp_b = lpool[0];
	*vp_e = lpool[1];
}

void
vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add)
{
	int i, i_max;
	char *acl_user, *acl_rnet, *acl_rmsk, *lnet, *lmsk;
	char acl_user_var[22], acl_rnet_var[22], acl_rmsk_var[22];

	lnet = nvram_safe_get("lan_ipaddr");
	lmsk = nvram_safe_get("lan_netmask");

	i_max = nvram_get_int("vpns_num_x");
	if (i_max > MAX_CLIENTS_NUM) i_max = MAX_CLIENTS_NUM;
	for (i = 0; i < i_max; i++) {
		snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
		snprintf(acl_rnet_var, sizeof(acl_rnet_var), "vpns_rnet_x%d", i);
		snprintf(acl_rmsk_var, sizeof(acl_rmsk_var), "vpns_rmsk_x%d", i);
		acl_user = nvram_safe_get(acl_user_var);
		acl_rnet = nvram_safe_get(acl_rnet_var);
		acl_rmsk = nvram_safe_get(acl_rmsk_var);
		if (*acl_user && strcmp(acl_user, cname) == 0 && is_valid_ipv4(acl_rnet) && is_valid_ipv4(acl_rmsk)) {
			if (!is_same_subnet2(acl_rnet, lnet, acl_rmsk, lmsk)) {
				if (add)
					route_add(ifname, 0, acl_rnet, gw, acl_rmsk);
				else
					route_del(ifname, 0, acl_rnet, gw, acl_rmsk);
			}
			
			break;
		}
	}
}

int
ipup_vpns_main(int argc, char **argv)
{
	int i_cast, i_vuse, lock_fd, firewall_status, lifecycle_fd, callback_state;
	int cleanup_failed = 0, saved_errno, record_length;
	char *peer_name;
	char lease_record[VPN_SERVER_LEASE_LINE_SIZE];
	char lease_ifname[VPN_SERVER_LEASE_IFNAME_SIZE];
	char *svcs[] = { "bcrelay", NULL };
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

	if (argc < 7 || !argv || !argv[1] || !argv[5] || !argv[6])
		return -1;
	if (!firewall_ppp_interface_is_safe(argv[1], VPN_SERVER_PPP_UNIT)) {
		logmessage(VPNS_LOG_NAME, "rejecting VPN up event with invalid PPP interface token");
		return -1;
	}
	callback_state = vpns_callback_lifecycle_enter(argv[1], &lifecycle_fd);
	if (callback_state <= 0) {
		if (callback_state == 0)
			logmessage(VPNS_LOG_NAME, "ignoring stale VPN up callback for %s", argv[1]);
		else
			logmessage(VPNS_LOG_NAME, "cannot authenticate VPN up callback for %s (errno=%d)",
				   argv[1], errno ? errno : EIO);
		return callback_state == 0 ? 0 : -1;
	}
	peer_name = safe_getenv("PEERNAME");
	if (!peer_name || !*peer_name) {
		logmessage(VPNS_LOG_NAME, "rejecting VPN up event without a peer name");
		(void)vpn_server_lifecycle_lock_release(lifecycle_fd);
		return -1;
	}
	record_length = snprintf(lease_record, sizeof(lease_record), "%s %s %s %s\n",
				 argv[1], argv[5], argv[6], peer_name);
	if (record_length < 0 || (size_t)record_length >= sizeof(lease_record) ||
	    vpn_server_lease_parse_line(lease_record, (size_t)record_length,
					lease_ifname, sizeof(lease_ifname)) != 0 ||
	    strcmp(lease_ifname, argv[1]) != 0) {
		logmessage(VPNS_LOG_NAME, "rejecting malformed VPN lease record for %s", argv[1]);
		errno = EINVAL;
		(void)vpn_server_lifecycle_lock_release(lifecycle_fd);
		return -1;
	}
	logmessage(VPNS_LOG_NAME, "peer %s (%s) connected - ifname: %s, local IP: %s",
		   argv[6], peer_name, argv[1], argv[5]);
	umask(0000);

	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(VPNS_LOG_NAME, "cannot lock VPN lease inventory for connection (errno=%d)", saved_errno);
		(void)vpn_server_lifecycle_lock_release(lifecycle_fd);
		errno = saved_errno;
		return -1;
	}
	firewall_status = vpns_firewall_permission(argv[1], 1);
	if (firewall_status != 0) {
		saved_errno = errno ? errno : EIO;
		if (vpn_server_lease_lock_release(lock_fd) != 0)
			logmessage(VPNS_LOG_NAME, "cannot release VPN lease lock after firewall-add failure (errno=%d)", errno);
		if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0)
			logmessage(VPNS_LOG_NAME, "cannot release VPN lifecycle lock after firewall-add failure (errno=%d)", errno);
		errno = saved_errno;
		return -1;
	}
	if (append_vpns_lease_record(lease_record, (size_t)record_length) != 0) {
		saved_errno = errno ? errno : EIO;
		if (vpns_firewall_permission(argv[1], 0) != 0)
			logmessage(VPNS_LOG_NAME, "cannot roll back VPN firewall permission for %s", argv[1]);
		if (vpn_server_lease_lock_release(lock_fd) != 0)
			logmessage(VPNS_LOG_NAME, "cannot release VPN lease lock after append failure (errno=%d)", errno);
		if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0)
			logmessage(VPNS_LOG_NAME, "cannot release VPN lifecycle lock after append failure (errno=%d)", errno);
		errno = saved_errno;
		logmessage(VPNS_LOG_NAME, "cannot persist VPN lease record for %s (errno=%d)", argv[1], saved_errno);
		return -1;
	}
	if (vpn_server_lease_lock_release(lock_fd) != 0) {
		logmessage(VPNS_LOG_NAME, "cannot release VPN lease lock after successful connection record (errno=%d)", errno);
		cleanup_failed = 1;
	}

	i_vuse = nvram_get_int("vpns_vuse");
	if (i_vuse) {
		char *ifconfig_argv[] = { "ifconfig", argv[1], "-multicast", NULL };
		_eval(ifconfig_argv, NULL, 0, NULL);
	}
	vpns_route_to_remote_lan(peer_name, argv[1], NULL, 1);
	set_vpn_balancing(argv[1], 1);
	i_cast = nvram_get_int("vpns_cast");
	if (pids(svcs[0])) {
		if (i_cast == 0)
			kill_services(svcs, 3, 1);
	} else if (i_vuse == 0) {
		if (i_cast == 1 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", IFNAME_BR, "-o", "ppp[1-5][0-9]", "-n");
		if (i_cast == 2 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", "ppp[1-5][0-9]", "-o", IFNAME_BR, "-n");
	}
	if (check_if_file_exist(script_name)) {
		char *hook_argv[] = {
			(char *)script_name, "up", argv[1], argv[5], argv[6], peer_name, NULL
		};
		_eval(hook_argv, NULL, 0, NULL);
	}
	if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0) {
		logmessage(VPNS_LOG_NAME, "cannot release VPN lifecycle lock after connection callback (errno=%d)", errno);
		cleanup_failed = 1;
	}
	return cleanup_failed ? -1 : 0;
}

int
ipdown_vpns_main(int argc, char **argv)
{
	int i_clients = 0, firewall_status = -1, lease_status = -1;
	int cleanup_failed = 0, lock_fd, lifecycle_fd, callback_state;
	char *peer_name;
	char *svcs[] = { "bcrelay", NULL };
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

	if (argc < 7 || !argv || !argv[1] || !argv[5] || !argv[6])
		return -1;
	if (!firewall_ppp_interface_is_safe(argv[1], VPN_SERVER_PPP_UNIT)) {
		logmessage(VPNS_LOG_NAME, "rejecting VPN down event with invalid PPP interface token");
		return -1;
	}
	callback_state = vpns_callback_lifecycle_enter(argv[1], &lifecycle_fd);
	if (callback_state <= 0) {
		if (callback_state == 0)
			logmessage(VPNS_LOG_NAME, "ignoring stale VPN down callback for %s", argv[1]);
		else
			logmessage(VPNS_LOG_NAME, "cannot authenticate VPN down callback for %s (errno=%d)",
				   argv[1], errno ? errno : EIO);
		return callback_state == 0 ? 0 : -1;
	}
	peer_name = safe_getenv("PEERNAME");
	logmessage(VPNS_LOG_NAME, "peer %s (%s) disconnected", argv[6], peer_name);
	umask(0000);

	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0) {
		logmessage(VPNS_LOG_NAME, "cannot lock VPN lease inventory for disconnect (errno=%d)", errno);
		cleanup_failed = 1;
	} else {
		firewall_status = vpns_firewall_permission(argv[1], 0);
		if (firewall_status != 0)
			cleanup_failed = 1;
		lease_status = vpns_prune_lease_file(argv[1], &i_clients);
		if (lease_status != 0) {
			logmessage(VPNS_LOG_NAME,
				   "cannot safely prune VPN lease inventory (errno=%d); preserving existing file",
				   errno);
			cleanup_failed = 1;
		}
		if (vpn_server_lease_lock_release(lock_fd) != 0) {
			logmessage(VPNS_LOG_NAME, "cannot release VPN lease lock after disconnect (errno=%d)", errno);
			cleanup_failed = 1;
		}
	}
	vpns_route_to_remote_lan(peer_name, argv[1], NULL, 0);
	if (check_if_file_exist(script_name)) {
		char *hook_argv[] = {
			(char *)script_name, "down", argv[1], argv[5], argv[6], peer_name, NULL
		};
		_eval(hook_argv, NULL, 0, NULL);
	}
	if (((lease_status == 0 && i_clients == 0) ||
	     nvram_get_int("vpns_cast") == 0) && pids(svcs[0]))
		kill_services(svcs, 3, 1);
	if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0) {
		logmessage(VPNS_LOG_NAME, "cannot release VPN lifecycle lock after disconnect callback (errno=%d)", errno);
		cleanup_failed = 1;
	}
	return cleanup_failed ? -1 : 0;
}

