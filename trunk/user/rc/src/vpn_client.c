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
#include <stdint.h>
#include <fcntl.h>

#include "rc.h"
#include <process_inventory.h>
#include "process_presence_start_policy.h"
#include "vpn_client_restart_policy.h"
#include "vpn_client_nvram_policy.h"
#include "route_status_policy.h"
#include "ppp_options_file.h"
#include "vpn_server_lifecycle_lock.h"

#define VPNC_LOG_NAME		"VPN client"
#define VPNC_PPP_LINK_NAME	"vpnc"
#define VPNC_PPP_UP_SCRIPT	"/tmp/ppp/ip-up.vpnc"
#define VPNC_PPP_DW_SCRIPT	"/tmp/ppp/ip-down.vpnc"

static int xl2tpd_killed_vpnc = 0;

static int
control_xl2tpd(const char *cmd, const char *lac)
{
	char buf[32];
	int control_fd, length, saved_errno;
	ssize_t written;

	if (!cmd || !lac) {
		errno = EINVAL;
		return -1;
	}
	length = snprintf(buf, sizeof(buf), "%s %s", cmd, lac);
	if (length < 0 || length >= (int)sizeof(buf)) {
		errno = EMSGSIZE;
		return -1;
	}

	control_fd = open("/var/run/xl2tpd-control", O_WRONLY, 0600);
	if (control_fd < 0)
		return -1;

	do {
		written = write(control_fd, buf, (size_t)length);
	} while (written < 0 && errno == EINTR);
	saved_errno = 0;
	if (written < 0)
		saved_errno = errno ? errno : EIO;
	else if (written != length)
		saved_errno = EIO;

	if (close(control_fd) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}

static int
get_xl2tpd_vpnc_active(void)
{
	return ((nvram_match("vpnc_enable", "1") && nvram_match("vpnc_type", "1")) || 
		 nvram_match("l2tp_cli_t", "1"));
}

int
vpn_client_nvram_set_temp_status(void *context, const char *name,
				 const char *value)
{
	int api_result, saved_errno, status;

	(void)context;
	errno = 0;
	api_result = nvram_set_temp(name, value);
	saved_errno = errno;
	status = vpn_client_nvram_write_status(api_result, saved_errno);
	if (status != 0)
		errno = status;
	return status;
}


static int
start_vpn_client_locked(void)
{
	FILE *fp;
	int i_type, i_mppe, i_auth, xl2tpd_status, saved_errno, cleanup_status;
	char vpnc_peer_copy[256], vpnc_peer_line[256];
	char *vpnc_peer, *vpnc_opt, tmp[256];

	if (nvram_invmatch("vpnc_enable", "1") || get_ap_mode())
		return 1;

	vpnc_peer = nvram_safe_get("vpnc_peer");
	saved_errno = vpn_client_nvram_copy_text(
		vpnc_peer, vpnc_peer_copy, sizeof(vpnc_peer_copy));
	if (saved_errno != 0 || vpnc_peer_copy[0] == '\0') {
		if (saved_errno == 0)
			saved_errno = EINVAL;
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client startup: peer is empty or oversized (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	vpnc_peer = safe_pppd_line(vpnc_peer_copy, vpnc_peer_line,
				   sizeof(vpnc_peer_line));
	if (!vpnc_peer) {
		saved_errno = errno ? errno : EINVAL;
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client startup: peer cannot be safely encoded (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	saved_errno = vpn_client_nvram_clear_runtime_state(
		vpn_client_nvram_set_temp_status, NULL);
	if (saved_errno != 0) {
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client startup: cannot clear stale runtime DNS/state (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	i_type = nvram_get_int("vpnc_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		return start_openvpn_client();
#endif
	vpnc_opt = VPN_CLIENT_PPPD_OPTIONS;

	if (ppp_options_directory_ready("/tmp/ppp") != 0 ||
	    ppp_options_symlink_ready(VPNC_PPP_UP_SCRIPT, "/sbin/rc") != 0 ||
	    ppp_options_symlink_ready(VPNC_PPP_DW_SCRIPT, "/sbin/rc") != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client startup because PPP callback directory/links are unsafe (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	i_auth = nvram_get_int("vpnc_auth");
	i_mppe = nvram_get_int("vpnc_mppe");

	/* Credential-bearing options must be private before truncation. */
	fp = ppp_options_file_open(vpnc_opt);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME, "cannot safely open private PPP options (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	fprintf(fp, "noauth\n");
	fprintf(fp, "user '%s'\n", safe_pppd_line(nvram_safe_get("vpnc_user"), tmp, sizeof(tmp)));
	fprintf(fp, "password '%s'\n", safe_pppd_line(nvram_safe_get("vpnc_pass"), tmp, sizeof(tmp)));
	fprintf(fp, "refuse-eap\n");

	if (i_auth == 1) {
		/* MS-CHAPv2 */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
	}
	else if (i_auth == 2) {
		/* CHAP */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	else if (i_auth == 3) {
		/* PAP */
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	if (i_type != 1) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n", vpnc_peer_line);
		fprintf(fp, "route_rdgw %d\n", (nvram_match("vpnc_dgw", "1")) ? 2 : 0);
		fprintf(fp, "persist\n");
		fprintf(fp, "linkname %s\n", VPNC_PPP_LINK_NAME);
	}

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("vpnc_mtu", 1450, 1000, 1460));
	fprintf(fp, "mru %d\n", nvram_safe_get_int("vpnc_mru", 1450, 1000, 1460));

	fprintf(fp, "maxfail %d\n", 0);		// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff %d\n", 10);	// pppd re-call time (10s)

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "noipdefault\n");
	fprintf(fp, "usepeerdns\n");

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

	if (i_type == 1)
	{
		// Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	/* echo failures (6*20s) */
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);
	fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "ip-up-script %s\n", VPNC_PPP_UP_SCRIPT);
	fprintf(fp, "ip-down-script %s\n", VPNC_PPP_DW_SCRIPT);

	fprintf(fp, "minunit %d\n", VPNC_PPP_UNIT);
	fprintf(fp, "ktune\n");

	/* user specific options */
	fprintf(fp, "%s\n", nvram_safe_get("vpnc_pppd"));

	if (ppp_options_file_finish(fp, vpnc_opt) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME, "cannot flush or close private PPP options (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	if (i_type == 1)
	{
		saved_errno = vpn_client_nvram_set_temp_status(
			NULL, "l2tp_cli_t", "1");
		if (saved_errno != 0) {
			logmessage(VPNC_LOG_NAME,
				   "refusing L2TP VPN client startup: cannot publish client ownership marker (status=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
		xl2tpd_status = safe_start_xl2tpd_locked();
		if (xl2tpd_status == 1) {
			if (control_xl2tpd("c", "VPNC") != 0) {
				saved_errno = errno ? errno : EIO;
				cleanup_status = vpn_client_nvram_set_temp_status(
					NULL, "l2tp_cli_t", "0");
				if (cleanup_status != 0)
					logmessage(VPNC_LOG_NAME,
						   "cannot roll back L2TP VPN client ownership marker (status=%d)",
						   cleanup_status);
				logmessage(VPNC_LOG_NAME,
					   "cannot connect VPNC tunnel through xl2tpd control FIFO (errno=%d)",
					   saved_errno);
				errno = saved_errno;
				return -1;
			}
		} else if (xl2tpd_status < 0) {
			saved_errno = errno ? errno : EIO;
			cleanup_status = vpn_client_nvram_set_temp_status(
				NULL, "l2tp_cli_t", "0");
			if (cleanup_status != 0)
				logmessage(VPNC_LOG_NAME,
					   "cannot roll back L2TP VPN client ownership marker (status=%d)",
					   cleanup_status);
			logmessage(VPNC_LOG_NAME, "cannot start or verify shared xl2tpd daemon (errno=%d)", saved_errno);
			errno = saved_errno;
			return -1;
		}
	}
	else
	{
		saved_errno = vpn_client_nvram_set_temp_status(
			NULL, "l2tp_cli_t", "0");
		if (saved_errno != 0) {
			logmessage(VPNC_LOG_NAME,
				   "refusing PPP VPN client startup: cannot clear L2TP ownership marker (status=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
		return eval("/usr/sbin/pppd", "file", vpnc_opt);
	}

	return 0;
}

int
start_vpn_client(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNC_LOG_NAME, "cannot lock VPN server lifecycle for client start (errno=%d)", errno);
		return -1;
	}
	result = start_vpn_client_locked();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME, "cannot release VPN server lifecycle lock after client start (errno=%d)",
			   release_errno);
		if (result >= 0) {
			result = -1;
			saved_errno = release_errno;
		}
	}
	if (result < 0 && saved_errno)
		errno = saved_errno;
	return result;
}

static int
stop_vpn_client_ppp(int stop_xl2tpd)
{
	char pidfile[64];
	const char *pidfile_paths[2];
	char *svcs[] = { "xl2tpd", NULL };
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/ppp-%s.pid",
			  VPNC_PPP_LINK_NAME);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	pidfile_paths[0] = pidfile;
	pidfile_paths[1] = NULL;
	return kill_services_with_pidfiles(stop_xl2tpd ? svcs : NULL,
					   pidfile_paths, "pppd", 0, 5, 1);
}

static int
stop_vpn_client_locked(void)
{
	int stop_status, scan_error, control_status;
	int cleanup_status = 0, route_status;
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;

	/* Do not mutate PPP or VPN client state if the shared daemon's presence
	 * cannot be established from a complete process inventory. */
	action = PROCESS_PRESENCE_START_NEW;
	if (get_xl2tpd_vpnc_active()) {
		presence = process_inventory_scan("xl2tpd", &snapshot);
		scan_error = snapshot.error;
		process_snapshot_release(&snapshot);
		action = process_presence_start_action_for_state(presence);
		if (action == PROCESS_PRESENCE_START_ABORT) {
			int saved_errno = scan_error ? scan_error : EIO;
			logmessage(VPNC_LOG_NAME,
				   "refusing VPN client cleanup because xl2tpd process inventory is unknown (errno=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
	}

	stop_status = stop_vpn_client_ppp(0);
	if (stop_status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client cleanup: PPP link PID identity is unknown or active (status=%d)",
			   stop_status);
		return stop_status;
	}
	if (action == PROCESS_PRESENCE_START_NOTIFY) {
		control_status = control_xl2tpd("d", "VPNC");
		if (control_status != 0) {
			int saved_errno = errno ? errno : EIO;
			logmessage(VPNC_LOG_NAME,
				   "cannot disconnect VPNC tunnel through xl2tpd control FIFO (errno=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
	}

#if defined(APP_OPENVPN)
	stop_status = stop_openvpn_client();
	if (stop_status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client cleanup because OpenVPN stop is incomplete (status=%d, errno=%d)",
			   stop_status, errno ? errno : EIO);
		return stop_status;
	}
#endif

	route_status = vpn_client_nvram_set_temp_status(
		NULL, "l2tp_cli_t", "0");
	cleanup_status = route_status_accumulate(cleanup_status, route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot clear L2TP VPN client ownership marker after stop (status=%d)",
			   route_status);

	route_status = vpn_client_nvram_set_temp_status(
		NULL, "vpnc_state_t", "0");
	cleanup_status = route_status_accumulate(cleanup_status, route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot clear VPN client state after stop (status=%d)",
			   route_status);

	unlink(VPNC_PPP_UP_SCRIPT);
	unlink(VPNC_PPP_DW_SCRIPT);
	return cleanup_status;
}

/*
 * The caller already owns vpn_server_lifecycle_lock. Do not call the public
 * stop_vpn_client() wrapper here: POSIX F_SETLK locks are process-associated,
 * so an inner F_UNLCK/close would release the caller's outer lock too.
 */
int
stop_vpn_client_under_lifecycle_lock(void)
{
	return stop_vpn_client_locked();
}

int
stop_vpn_client(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNC_LOG_NAME,
			   "cannot lock VPN server lifecycle for client stop (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	result = stop_vpn_client_locked();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "cannot release VPN server lifecycle lock after client stop (errno=%d)",
			   release_errno);
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
stop_vpn_client_force(void)
{
	int stop_status;

	if (!get_xl2tpd_vpnc_active())
		return 0;
	stop_status = stop_vpn_client_ppp(1);
	if (stop_status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "refusing forced VPN client restart: PPP/xl2tpd identity preflight failed (status=%d)",
			   stop_status);
		return stop_status;
	}
	xl2tpd_killed_vpnc = 1;
	return 0;
}

static int
restart_vpn_client_locked(void)
{
	int stop_status, start_status, restore_status, firewall_status;
	int saved_errno = 0;

	if (get_ap_mode())
		return 0;

	xl2tpd_killed_vpnc = 0;
	stop_status = stop_vpn_client_force();
	if (stop_status != 0)
		return stop_status;
	stop_status = stop_vpn_client_locked();
	if (stop_status != 0) {
		saved_errno = errno ? errno : EIO;
		/*
		 * The force-stop above already removed VPNC's xl2tpd daemon.
		 * A later identity-aware cleanup failure must not strand an
		 * independently configured WAN L2TP client or L2TP server.
		 */
		if (vpn_client_restart_should_restore_shared_xl2tpd(
			    xl2tpd_killed_vpnc,
			    nvram_match("l2tp_wan_t", "1"),
			    nvram_match("l2tp_srv_t", "1"))) {
			restore_status = safe_start_xl2tpd_locked();
			if (restore_status < 0) {
				int restore_errno = errno ? errno : EIO;
				logmessage(VPNC_LOG_NAME,
					   "cannot restore shared xl2tpd after incomplete VPN client teardown (status=%d, errno=%d)",
					   restore_status, restore_errno);
			}
		}
		errno = saved_errno;
		return stop_status;
	}

	restore_status = restore_dns_from_vpnc();
	if (restore_status != 0) {
		start_status = restore_status;
		saved_errno = restore_status;
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client restart because the previous DNS snapshot could not be cleared (status=%d)",
			   restore_status);
	} else {
		sleep(1);
		start_status = start_vpn_client_locked();
		if (start_status != 0)
			saved_errno = errno ? errno : EIO;
	}
	firewall_status = restart_firewall();
	if (start_status == 0 && firewall_status != 0) {
		start_status = firewall_status;
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "VPN client restarted but firewall/UPnP refresh failed (status=%d)",
			   firewall_status);
	}

	/* restore L2TP WAN client or L2TP VPNS */
	if (xl2tpd_killed_vpnc &&
	    (nvram_match("l2tp_wan_t", "1") || nvram_match("l2tp_srv_t", "1"))) {
		restore_status = safe_start_xl2tpd_locked();
		if (restore_status < 0 && start_status == 0) {
			start_status = -1;
			saved_errno = errno ? errno : EIO;
		}
	}
	if (start_status != 0 && saved_errno != 0)
		errno = saved_errno;
	return start_status;
}

int
restart_vpn_client(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(VPNC_LOG_NAME, "cannot lock VPN server lifecycle for client restart (errno=%d)", errno);
		return -1;
	}
	result = restart_vpn_client_locked();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME, "cannot release VPN server lifecycle lock after client restart (errno=%d)",
			   release_errno);
		if (result == 0) {
			result = -1;
			saved_errno = release_errno;
		}
	}
	if (result != 0 && saved_errno)
		errno = saved_errno;
	return result;
}

int
restore_dns_from_vpnc(void)
{
	char *vpnc_dns = NULL;
	int api_result, saved_errno, status;

	errno = 0;
	api_result = nvram_get_checked("vpnc_dns_t", &vpnc_dns);
	saved_errno = errno;
	status = vpn_client_nvram_read_status(api_result, saved_errno,
					     vpnc_dns != NULL);
	if (status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "cannot read VPN DNS snapshot for resolver cleanup (status=%d)",
			   status);
		errno = status;
		return status;
	}
	if (api_result == 0 || vpnc_dns[0] == '\0')
		return 0;

	status = vpn_client_nvram_set_temp_status(NULL, "vpnc_dns_t", "");
	if (status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "cannot clear VPN DNS snapshot before resolver rebuild (status=%d)",
			   status);
		errno = status;
		return status;
	}

	refresh_resolvconf_nonfatal("VPN client DNS removal", 0);
	return 0;
}

static int
vpnc_route_dgw(char *ifname, char *gw, int add)
{
	int status = 0, route_status;

	if (!nvram_match("vpnc_dgw", "1"))
		return 0;
	if (!ifname || !ifname[0] || !gw || !gw[0])
		return EINVAL;

	route_status = add ?
		route_add(ifname, 0, "0.0.0.0", gw, "128.0.0.0") :
		route_del(ifname, 0, "0.0.0.0", gw, "128.0.0.0");
	if (add)
		route_status = route_status_idempotent_add(route_status);
	else
		route_status = route_status_idempotent_delete(route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME, "default VPN route %s failed on %s (status=%d)",
			   add ? "add" : "delete", ifname, route_status);
	status = route_status_accumulate(status, route_status);

	route_status = add ?
		route_add(ifname, 0, "128.0.0.0", gw, "128.0.0.0") :
		route_del(ifname, 0, "128.0.0.0", gw, "128.0.0.0");
	if (add)
		route_status = route_status_idempotent_add(route_status);
	else
		route_status = route_status_idempotent_delete(route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME, "upper-half VPN route %s failed on %s (status=%d)",
			   add ? "add" : "delete", ifname, route_status);
	status = route_status_accumulate(status, route_status);

	return status;
}

static int
vpnc_route_to_remote_lan(char *ifname, char *gw, int add)
{
	char *rnet = nvram_safe_get("vpnc_rnet");
	char *rmsk = nvram_safe_get("vpnc_rmsk");
	char *lnet = nvram_safe_get("lan_ipaddr");
	char *lmsk = nvram_safe_get("lan_netmask");
	int route_status;

	if (!is_valid_ipv4(rnet) || !is_valid_ipv4(rmsk) ||
	    is_same_subnet2(rnet, lnet, rmsk, lmsk))
		return 0;
	if (!ifname || !ifname[0] || !gw || !gw[0])
		return EINVAL;

	route_status = add ?
		route_add(ifname, 0, rnet, gw, rmsk) :
		route_del(ifname, 0, rnet, gw, rmsk);
	if (add)
		route_status = route_status_idempotent_add(route_status);
	else
		route_status = route_status_idempotent_delete(route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME, "remote-LAN VPN route %s failed on %s (status=%d)",
			   add ? "add" : "delete", ifname, route_status);
	return route_status;
}

int
ipup_vpnc_main(int argc, char **argv)
{
	int status = 0, route_status;
	char buf[256];
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	umask(0000);

	route_status = vpnc_route_to_remote_lan(ifname, gw, 1);
	status = route_status_accumulate(status, route_status);
	route_status = vpnc_route_dgw(ifname, gw, 1);
	status = route_status_accumulate(status, route_status);

	set_vpn_balancing(ifname, 0);

	route_status = vpn_client_nvram_set_temp_status(
		NULL, "vpnc_state_t", "1");
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot publish VPN client up state (status=%d)",
			   route_status);

	buf[0] = 0;
	if (nvram_get_int("vpnc_pdns") > 0) {
		char *value;
		
		value = getenv("DNS1");
		if (value)
			snprintf(buf, sizeof(buf), "%s", value);
		value = getenv("DNS2");
		if (value && strcmp(value, buf) != 0) {
			int buf_len = strlen(buf);
			snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", value);
		}
	}

	route_status = vpn_client_nvram_set_temp_status(
		NULL, "vpnc_dns_t", buf);
	status = route_status_accumulate(status, route_status);
	if (route_status == 0)
		refresh_resolvconf_nonfatal("VPN client DNS update", 0);
	else
		logmessage(VPNC_LOG_NAME,
			   "cannot publish VPN client DNS snapshot (status=%d)",
			   route_status);

	if (check_if_file_exist(script_name)) {
		route_status = doSystem("%s %s", script_name, "up");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(VPNC_LOG_NAME, "VPN client up hook failed (status=%d)", route_status);
	}

	return status;
}

int
ipdown_vpnc_main(int argc, char **argv)
{
	int status = 0, route_status;
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	umask(0000);

	route_status = vpnc_route_dgw(ifname, gw, 0);
	status = route_status_accumulate(status, route_status);
	route_status = vpnc_route_to_remote_lan(ifname, gw, 0);
	status = route_status_accumulate(status, route_status);

	route_status = vpn_client_nvram_set_temp_status(
		NULL, "vpnc_state_t", "0");
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot clear VPN client up state (status=%d)",
			   route_status);

	route_status = restore_dns_from_vpnc();
	status = route_status_accumulate(status, route_status);

	if (check_if_file_exist(script_name)) {
		route_status = doSystem("%s %s", script_name, "down");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(VPNC_LOG_NAME, "VPN client down hook failed (status=%d)", route_status);
	}

	return status;
}

