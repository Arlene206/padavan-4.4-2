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
#include "ppp_callback_link_policy.h"
#include "ppp_option_value_policy.h"
#include "vpn_server_lifecycle_lock.h"
#include "vpn_server_callback_policy.h"
#include "vpn_client_callback_policy.h"
#include <process_pidfile.h>

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
vpn_client_read_transaction_int(struct nvram_transaction *transaction,
                                const char *name, int default_value,
                                int minimum, int maximum, int *result)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	status = vpn_client_nvram_read_int_status(
		api_result, saved_errno, value, default_value,
		minimum, maximum, result);
	if (status != 0)
		errno = status;
	return status;
}

static int
vpn_client_read_transaction_clamped_int(struct nvram_transaction *transaction,
                                        const char *name, int default_value,
                                        int minimum, int maximum, int *result)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	status = vpn_client_nvram_read_clamped_int_status(
		api_result, saved_errno, value, default_value,
		minimum, maximum, result);
	if (status != 0)
		errno = status;
	return status;
}

static int
vpn_client_read_transaction_text(struct nvram_transaction *transaction,
                                 const char *name, char *destination,
                                 size_t destination_size)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	status = vpn_client_nvram_copy_status(
		api_result, saved_errno, value, destination, destination_size);
	if (status != 0)
		errno = status;
	return status;
}

static int
vpn_client_read_transaction_alloc_text(struct nvram_transaction *transaction,
                                       const char *name, char **destination)
{
	char *value = NULL;
	char *copy;
	int api_result, saved_errno, status;

	if (!transaction || !name || !destination) {
		errno = EINVAL;
		return EINVAL;
	}
	*destination = NULL;
	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	status = vpn_client_nvram_read_status(
		api_result, saved_errno, api_result == 1 && value != NULL);
	if (status != 0) {
		errno = status;
		return status;
	}
	copy = strdup(api_result == 1 ? value : "");
	if (!copy) {
		status = errno ? errno : ENOMEM;
		errno = status;
		return status;
	}
	*destination = copy;
	return 0;
}

struct vpn_client_start_config {
	int enabled;
	int type;
	int auth;
	int mppe;
	int mtu;
	int mru;
	int default_route;
	char peer[256];
	char peer_line[256];
	char user_line[256];
	char pass_line[256];
	char *pppd_options;
};

/* Read every PPP selector and credential from one checked NVRAM snapshot. */
static int
vpn_client_read_start_config_snapshot(struct vpn_client_start_config *config)
{
	struct nvram_transaction transaction;
	struct vpn_client_start_config pending;
	char user[256], password[256], *custom_options = NULL;
	int status = 0, result = -1, saved_errno = 0, end_status;

	if (!config) {
		errno = EINVAL;
		return -1;
	}
	memset(&pending, 0, sizeof(pending));
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_enable", 0, 0, 1, &pending.enabled);
	if (status != 0)
		goto finish_transaction;
	if (!pending.enabled) {
		result = 0;
		goto finish_transaction;
	}
	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_type", 0, 0, 2, &pending.type);
	if (status != 0)
		goto finish_transaction;
#if defined(APP_OPENVPN)
	if (pending.type == 2) {
		result = 0;
		goto finish_transaction;
	}
#endif
	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_auth", 0, 0, 3, &pending.auth);
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_mppe", 0, 0, 3, &pending.mppe);
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_clamped_int(
		&transaction, "vpnc_mtu", 1450, 1000, 1460, &pending.mtu);
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_clamped_int(
		&transaction, "vpnc_mru", 1450, 1000, 1460, &pending.mru);
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_dgw", 0, 0, 1, &pending.default_route);
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_text(
		&transaction, "vpnc_peer", pending.peer, sizeof(pending.peer));
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_text(
		&transaction, "vpnc_user", user, sizeof(user));
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_text(
		&transaction, "vpnc_pass", password, sizeof(password));
	if (status != 0)
		goto finish_transaction;
	status = vpn_client_read_transaction_alloc_text(
		&transaction, "vpnc_pppd", &custom_options);
	if (status != 0)
		goto finish_transaction;

	if (!pending.peer[0]) {
		status = EINVAL;
		errno = status;
		goto finish_transaction;
	}
	if (ppp_option_value_escape(pending.peer, pending.peer_line,
				    sizeof(pending.peer_line)) != 0 ||
	    ppp_option_value_escape(user, pending.user_line,
				    sizeof(pending.user_line)) != 0 ||
	    ppp_option_value_escape(password, pending.pass_line,
				    sizeof(pending.pass_line)) != 0 ||
	    ppp_option_text_escape_alloc(custom_options,
					  &pending.pppd_options) != 0) {
		status = errno ? errno : EINVAL;
		goto finish_transaction;
	}
	result = 0;

finish_transaction:
	if (result != 0)
		saved_errno = errno ? errno : (status ? status : EIO);
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	free(custom_options);
	if (result != 0) {
		free(pending.pppd_options);
		memset(user, 0, sizeof(user));
		memset(password, 0, sizeof(password));
		errno = saved_errno;
		return -1;
	}
	memset(user, 0, sizeof(user));
	memset(password, 0, sizeof(password));
	*config = pending;
	return 0;
}

/* Capture all selectors before touching the shared daemon or PPP state. */
static int
get_xl2tpd_vpnc_active(int *active)
{
	struct nvram_transaction transaction;
	int enabled = 0, type = 0, l2tp_client = 0;
	int status, saved_errno = 0, end_status;

	if (!active) {
		errno = EINVAL;
		return -1;
	}
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	status = vpn_client_read_transaction_int(
		&transaction, "vpnc_enable", 0, 0, 1, &enabled);
	if (status == 0)
		status = vpn_client_read_transaction_int(
			&transaction, "vpnc_type", 0, 0, 2, &type);
	if (status == 0)
		status = vpn_client_read_transaction_int(
			&transaction, "l2tp_cli_t", 0, 0, 1, &l2tp_client);
	if (status != 0)
		saved_errno = status;

	end_status = nvram_transaction_end(&transaction);
	if (status == 0 && end_status != 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}

	*active = (enabled && type == 1) || l2tp_client;
	return 0;
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

int
vpn_client_clear_runtime_state_status(void)
{
	int status = vpn_client_nvram_clear_runtime_state(
		vpn_client_nvram_set_temp_status, NULL);

	if (status != 0)
		errno = status;
	return status;
}


static int
start_vpn_client_locked(void)
{
	FILE *fp;
	int i_type, i_mppe, i_auth, xl2tpd_status, saved_errno, cleanup_status;
	struct vpn_client_start_config config;
	char *vpnc_opt;

	if (get_ap_mode())
		return 1;
	if (vpn_client_read_start_config_snapshot(&config) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "cannot read/validate VPN client startup configuration; refusing startup (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (!config.enabled)
		return 1;

	i_type = config.type;
#if defined(APP_OPENVPN)
	if (i_type == 2)
		return start_openvpn_client();
#endif
	cleanup_status = vpn_client_clear_runtime_state_status();
	if (cleanup_status != 0) {
		free(config.pppd_options);
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client startup: cannot clear stale runtime DNS/state (status=%d)",
			   cleanup_status);
		errno = cleanup_status;
		return -1;
	}

	vpnc_opt = VPN_CLIENT_PPPD_OPTIONS;
	if (ppp_options_directory_ready("/tmp/ppp") != 0 ||
	    ppp_options_symlink_ready(VPNC_PPP_UP_SCRIPT, "/sbin/rc") != 0 ||
	    ppp_options_symlink_ready(VPNC_PPP_DW_SCRIPT, "/sbin/rc") != 0) {
		saved_errno = errno ? errno : EIO;
		free(config.pppd_options);
		logmessage(VPNC_LOG_NAME,
			   "refusing PPP VPN startup because callback directory/links are unsafe (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	i_auth = config.auth;
	i_mppe = config.mppe;
	fp = ppp_options_file_open(vpnc_opt);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		free(config.pppd_options);
		logmessage(VPNC_LOG_NAME, "cannot safely open private PPP options (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	fprintf(fp, "noauth\n");
	fprintf(fp, "user '%s'\n", config.user_line);
	fprintf(fp, "password '%s'\n", config.pass_line);
	fprintf(fp, "refuse-eap\n");
	if (i_auth == 1) {
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
	} else if (i_auth == 2) {
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	} else if (i_auth == 3) {
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	if (i_type != 1) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n", config.peer_line);
		fprintf(fp, "route_rdgw %d\n", config.default_route ? 2 : 0);
		fprintf(fp, "persist\n");
		fprintf(fp, "linkname %s\n", VPNC_PPP_LINK_NAME);
	}
	fprintf(fp, "mtu %d\n", config.mtu);
	fprintf(fp, "mru %d\n", config.mru);
	fprintf(fp, "maxfail %d\n", 0);		// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff %d\n", 10);	// pppd re-call time (10s)
	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "noipdefault\n");
	fprintf(fp, "usepeerdns\n");
	fprintf(fp, "default-asyncmap\n");
	fprintf(fp, "nopcomp noaccomp\n");
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
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);
	fprintf(fp, "lcp-echo-adaptive\n");
	fprintf(fp, "ip-up-script %s\n", VPNC_PPP_UP_SCRIPT);
	fprintf(fp, "ip-down-script %s\n", VPNC_PPP_DW_SCRIPT);
	fprintf(fp, "minunit %d\n", VPNC_PPP_UNIT);
	fprintf(fp, "ktune\n");
	fprintf(fp, "%s\n", config.pppd_options);

	if (ppp_options_file_finish(fp, vpnc_opt) != 0) {
		saved_errno = errno ? errno : EIO;
		free(config.pppd_options);
		logmessage(VPNC_LOG_NAME, "cannot flush or close private PPP options (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
	free(config.pppd_options);
	config.pppd_options = NULL;

	if (i_type == 1) {
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
						   "cannot roll back L2TP client ownership marker (status=%d)",
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
					   "cannot roll back L2TP client ownership marker (status=%d)",
					   cleanup_status);
			logmessage(VPNC_LOG_NAME, "cannot start or verify shared xl2tpd daemon (errno=%d)", saved_errno);
			errno = saved_errno;
			return -1;
		}
	} else {
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
	int cleanup_status = 0, route_status, config_status;
	int xl2tpd_vpnc_active = 0;
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;

	/* Unknown NVRAM ownership is just as unsafe as unknown process presence. */
	config_status = get_xl2tpd_vpnc_active(&xl2tpd_vpnc_active);
	if (config_status != 0) {
		int saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "refusing VPN client cleanup: xl2tpd ownership is unknown (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	action = PROCESS_PRESENCE_START_NEW;
	if (xl2tpd_vpnc_active) {
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

	route_status = restore_dns_from_vpnc();
	cleanup_status = route_status_accumulate(cleanup_status, route_status);
	if (route_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot restore DNS state after VPN client stop (status=%d)",
			   route_status);

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

	if (ppp_callback_link_remove_from_directory(
		    "/tmp/ppp", "/sbin/rc", VPNC_PPP_UP_SCRIPT) != 0) {
		route_status = errno ? errno : EIO;
		cleanup_status = route_status_accumulate(cleanup_status, route_status);
		logmessage(VPNC_LOG_NAME,
			   "cannot safely remove VPN up callback link (status=%d)",
			   route_status);
	}
	if (ppp_callback_link_remove_from_directory(
		    "/tmp/ppp", "/sbin/rc", VPNC_PPP_DW_SCRIPT) != 0) {
		route_status = errno ? errno : EIO;
		cleanup_status = route_status_accumulate(cleanup_status, route_status);
		logmessage(VPNC_LOG_NAME,
			   "cannot safely remove VPN down callback link (status=%d)",
			   route_status);
	}
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
	int stop_status, config_status, xl2tpd_vpnc_active = 0;

	config_status = get_xl2tpd_vpnc_active(&xl2tpd_vpnc_active);
	if (config_status != 0) {
		int saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "refusing forced VPN client restart: xl2tpd ownership is unknown (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (!xl2tpd_vpnc_active)
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
vpn_client_read_shared_xl2tpd_dependencies(int *wan_l2tp_enabled,
                                           int *vpn_server_l2tp_enabled)
{
	struct nvram_transaction transaction;
	int wan_enabled = 0, server_enabled = 0;
	int status, saved_errno = 0, end_status;

	if (!wan_l2tp_enabled || !vpn_server_l2tp_enabled) {
		errno = EINVAL;
		return -1;
	}
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	status = vpn_client_read_transaction_int(
		&transaction, "l2tp_wan_t", 0, 0, 1, &wan_enabled);
	if (status == 0)
		status = vpn_client_read_transaction_int(
			&transaction, "l2tp_srv_t", 0, 0, 1, &server_enabled);
	if (status != 0)
		saved_errno = status;

	end_status = nvram_transaction_end(&transaction);
	if (status == 0 && end_status != 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}

	*wan_l2tp_enabled = wan_enabled;
	*vpn_server_l2tp_enabled = server_enabled;
	return 0;
}

static int
vpn_client_restore_shared_xl2tpd_locked(const char *phase)
{
	int dependency_status, saved_errno = 0;
	int wan_l2tp_enabled = 0, vpn_server_l2tp_enabled = 0;
	int restore_status;

	if (!xl2tpd_killed_vpnc)
		return 0;

	dependency_status = vpn_client_read_shared_xl2tpd_dependencies(
		&wan_l2tp_enabled, &vpn_server_l2tp_enabled);
	if (dependency_status != 0)
		saved_errno = errno ? errno : EIO;

	if (!vpn_client_restart_should_attempt_restore_shared_xl2tpd(
		    xl2tpd_killed_vpnc, dependency_status,
		    wan_l2tp_enabled, vpn_server_l2tp_enabled))
		return 0;

	/* Unknown dependency state requires conservative daemon restoration. */
	restore_status = safe_start_xl2tpd_locked();
	if (restore_status < 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot restore shared xl2tpd daemon %s (status=%d, errno=%d)",
			   phase, restore_status, errno ? errno : EIO);
	if (dependency_status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "cannot verify shared xl2tpd dependencies %s; restoration was attempted (errno=%d)",
			   phase, saved_errno);
		errno = saved_errno;
		return -1;
	}
	return restore_status < 0 ? restore_status : 0;
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
		 * Forced stop may already have removed the shared xl2tpd daemon.
		 * Restore on unreadable dependency state as well as active users.
		 */
		restore_status = vpn_client_restore_shared_xl2tpd_locked(
			"after incomplete VPN client teardown");
		if (restore_status < 0) {
			int restore_errno = errno ? errno : EIO;
			logmessage(VPNC_LOG_NAME,
				   "cannot restore shared xl2tpd after incomplete VPN client teardown (status=%d, errno=%d)",
				   restore_status, restore_errno);
		}
		errno = saved_errno;
		return stop_status;
	}

	sleep(1);
	start_status = start_vpn_client_locked();
	if (start_status != 0)
		saved_errno = errno ? errno : EIO;
	firewall_status = restart_firewall();
	if (start_status == 0 && firewall_status != 0) {
		start_status = firewall_status;
		saved_errno = errno ? errno : EIO;
		logmessage(VPNC_LOG_NAME,
			   "VPN client restarted but firewall/UPnP refresh failed (status=%d)",
			   firewall_status);
	}

	/* Recreate shared xl2tpd when dependencies are present or unreadable. */
	restore_status = vpn_client_restore_shared_xl2tpd_locked(
		"after VPN client restart");
	if (restore_status < 0 && start_status == 0) {
		start_status = -1;
		saved_errno = errno ? errno : EIO;
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
	int api_result, saved_errno, status, domain_status;

	errno = 0;
	api_result = nvram_get_checked("vpnc_dns_t", &vpnc_dns);
	saved_errno = errno;
	status = vpn_client_nvram_read_status(api_result, saved_errno,
					     vpnc_dns != NULL);
	if (status != 0) {
		logmessage(VPNC_LOG_NAME,
			   "cannot read VPN DNS snapshot for resolver cleanup (status=%d)",
			   status);
	} else if (api_result == 1 && vpnc_dns[0] != '\0') {
		status = vpn_client_nvram_set_temp_status(NULL, "vpnc_dns_t", "");
		if (status != 0)
			logmessage(VPNC_LOG_NAME,
				   "cannot clear VPN DNS snapshot before resolver rebuild (status=%d)",
				   status);
		else
			refresh_resolvconf_nonfatal("VPN client DNS removal", 0);
	}

	/* Domain state is independent: attempt cleanup even after DNS errors. */
	domain_status = vpn_client_nvram_clear_domain_snapshot(
		vpn_client_nvram_set_temp_status, NULL);
	if (domain_status != 0)
		logmessage(VPNC_LOG_NAME,
			   "cannot clear VPN domain snapshot (status=%d)",
			   domain_status);
	status = route_status_accumulate(status, domain_status);
	if (status != 0)
		errno = status;
	return status;
}

/* NVRAM getter pointers may share a cache buffer; copy before the next read.
 * Missing string keys keep the legacy empty-value behavior. */
static int
vpn_client_read_string_setting_checked(const char *name,
                                        char *destination,
                                        size_t destination_size)
{
    char *value = NULL;
    int api_result, saved_errno, status;

    if (!name || !destination || destination_size == 0) {
        errno = EINVAL;
        return EINVAL;
    }
    errno = 0;
    api_result = nvram_get_checked(name, &value);
    saved_errno = errno;
    status = vpn_client_nvram_copy_status(
        api_result, saved_errno, value, destination, destination_size);
    if (status != 0) {
        errno = status;
        logmessage(VPNC_LOG_NAME,
                   "cannot read VPN route setting %s (status=%d)",
                   name, status);
    }
    return status;
}

static int
vpn_client_read_match_setting_checked(const char *name,
                                      const char *expected,
                                      int default_match, int *matches)
{
    char *value = NULL;
    int api_result, saved_errno, status;

    if (!name || !expected || !matches ||
        (default_match != 0 && default_match != 1)) {
        errno = EINVAL;
        return EINVAL;
    }
    errno = 0;
    api_result = nvram_get_checked(name, &value);
    saved_errno = errno;
    status = vpn_client_nvram_match_status(
        api_result, saved_errno, value, expected, default_match, matches);
    if (status != 0) {
        errno = status;
        logmessage(VPNC_LOG_NAME,
                   "cannot read VPN route selector %s (status=%d)",
                   name, status);
    }
    return status;
}

static int
vpnc_route_dgw(char *ifname, char *gw, int add)
{
	int status = 0, route_status, use_default_gateway = 0;

	status = vpn_client_read_match_setting_checked(
		"vpnc_dgw", "1", 0, &use_default_gateway);
	if (status != 0)
		return status;
	if (!use_default_gateway)
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
	char rnet[16], rmsk[16], lnet[16], lmsk[16];
	int status, route_status;

	status = vpn_client_read_string_setting_checked(
		"vpnc_rnet", rnet, sizeof(rnet));
	if (status != 0)
		return status;
	status = vpn_client_read_string_setting_checked(
		"vpnc_rmsk", rmsk, sizeof(rmsk));
	if (status != 0)
		return status;
	status = vpn_client_read_string_setting_checked(
		"lan_ipaddr", lnet, sizeof(lnet));
	if (status != 0)
		return status;
	status = vpn_client_read_string_setting_checked(
		"lan_netmask", lmsk, sizeof(lmsk));
	if (status != 0)
		return status;

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

/*
 * A PPP callback runs in a child of pppd and may outlive a client stop.
 * Serialize with client/WAN/server lifecycle transitions, then authenticate
 * both the direct parent and its current per-interface identity pidfile.
 */
static int
vpn_client_callback_session_is_current(const char *ifname)
{
	const char *pid_text;
	pid_t parent_pid;
	char pidfile[64];
	struct process_identity identity;
	enum process_presence state;
	int path_length;

	if (!vpn_client_callback_interface_is_valid(ifname)) {
		errno = EINVAL;
		return -1;
	}

	pid_text = safe_getenv("PPPD_PID");
	parent_pid = getppid();
	path_length = snprintf(pidfile, sizeof(pidfile),
			       "/var/run/%s.pid", ifname);
	if (path_length < 0 || path_length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	state = process_pidfile_read_identity(pidfile, "pppd", &identity);
	if (state != PROCESS_PRESENCE_RUNNING) {
		if (state == PROCESS_PRESENCE_ABSENT)
			errno = ESRCH;
		else if (!errno)
			errno = EIO;
		return -1;
	}

	return vpn_client_callback_session_matches(
		ifname, pid_text, parent_pid, identity.pid);
}

/* Hold the same inode lock as start/stop/restart through every side effect. */
static int
vpn_client_callback_lifecycle_enter(const char *ifname, int *lifecycle_fd)
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

	current = vpn_client_callback_session_is_current(ifname);
	if (current != 1) {
		saved_errno = errno ? errno : EIO;
		if (vpn_server_lifecycle_lock_release(fd) != 0) {
			logmessage(VPNC_LOG_NAME,
				   "cannot release VPN client lifecycle lock after callback identity rejection (errno=%d)",
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

int
ipup_vpnc_main(int argc, char **argv)
{
	int status = 0, route_status, api_result, read_errno;
	int pdns_status, vpnc_pdns = 0;
	int lifecycle_fd, callback_state;
	char *pdns_value = NULL;
	char buf[256];
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	if (argc < 2 || !argv || !argv[1] || !ifname ||
	    strcmp(argv[1], ifname) != 0) {
		errno = EINVAL;
		return -1;
	}
	callback_state = vpn_client_callback_lifecycle_enter(
		argv[1], &lifecycle_fd);
	if (callback_state <= 0) {
		if (callback_state == 0)
			logmessage(VPNC_LOG_NAME,
				   "ignoring stale VPN client up callback for %s", argv[1]);
		else
			logmessage(VPNC_LOG_NAME,
				   "cannot authenticate VPN client up callback for %s (errno=%d)",
				   argv[1], errno ? errno : EIO);
		return callback_state == 0 ? 0 : -1;
	}

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
			   "cannot publish VPN client up state (status=%d)", route_status);

	/* A failed policy read must not erase the previous DNS snapshot. */
	errno = 0;
	api_result = nvram_get_checked("vpnc_pdns", &pdns_value);
	read_errno = errno;
	pdns_status = vpn_client_nvram_read_dns_policy_status(
		api_result, read_errno, pdns_value, &vpnc_pdns);
	if (pdns_status != 0) {
		status = route_status_accumulate(status, pdns_status);
		logmessage(VPNC_LOG_NAME,
			   "cannot read VPN DNS push policy; preserving existing resolver state (status=%d)",
			   pdns_status);
	}

	buf[0] = 0;
	if (pdns_status == 0) {
		if (vpnc_pdns > 0) {
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
	}

	if (check_if_file_exist(script_name)) {
		route_status = doSystem("%s %s", script_name, "up");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(VPNC_LOG_NAME, "VPN client up hook failed (status=%d)", route_status);
	}

	if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0) {
		int release_errno = errno ? errno : EIO;

		logmessage(VPNC_LOG_NAME,
			   "cannot release VPN client lifecycle lock after up callback (errno=%d)",
			   release_errno);
		if (status == 0) {
			status = -1;
			errno = release_errno;
		}
	}
	return status;
}
int
ipdown_vpnc_main(int argc, char **argv)
{
	int status = 0, route_status;
	int lifecycle_fd, callback_state;
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	if (argc < 2 || !argv || !argv[1] || !ifname ||
	    strcmp(argv[1], ifname) != 0) {
		errno = EINVAL;
		return -1;
	}
	callback_state = vpn_client_callback_lifecycle_enter(
		argv[1], &lifecycle_fd);
	if (callback_state <= 0) {
		if (callback_state == 0)
			logmessage(VPNC_LOG_NAME,
				   "ignoring stale VPN client down callback for %s", argv[1]);
		else
			logmessage(VPNC_LOG_NAME,
				   "cannot authenticate VPN client down callback for %s (errno=%d)",
				   argv[1], errno ? errno : EIO);
		return callback_state == 0 ? 0 : -1;
	}

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

	if (vpn_server_lifecycle_lock_release(lifecycle_fd) != 0) {
		int release_errno = errno ? errno : EIO;

		logmessage(VPNC_LOG_NAME,
			   "cannot release VPN client lifecycle lock after down callback (errno=%d)",
			   release_errno);
		if (status == 0) {
			status = -1;
			errno = release_errno;
		}
	}
	return status;
}

