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
#include <unistd.h>
#include <string.h>
#include <net/route.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>

#include "rc.h"
#include <nvram_linux.h>
#include "vpn_server_lifecycle_lock.h"
#include "route_status_policy.h"
#include "ppp_options_file.h"
#include "ppp_wan_option_policy.h"
#include "wan_ifname_policy.h"
#include "ppp_option_value_policy.h"
#include "ppp_callback_identity_policy.h"
#include "ppp_callback_generation_policy.h"
#include "ppp_callback_state_file.h"
#include "ppp_wan_ipup_policy.h"
#include "dhcp_lease_policy.h"
#include "wan_nvram_status_policy.h"
#include <process_inventory.h>
#include "xl2tpd_start_policy.h"
#if defined (APP_RPL2TP)
#include "l2tp_config_file.h"
#endif

#define PPP_WAN_SESSION_STATE_FILE "/tmp/ppp/wan-session.state"
#define PPP_WAN_SESSION_LOCK_FILE "/tmp/ppp/wan-session.lock"
#define PPP_WAN_SESSION_LOCK_ATTEMPTS 500

/* Keep the lock inode stable while generation state is atomically replaced. */
static int
ppp_wan_session_state_lock(unsigned long *generation, int allow_empty)
{
	if (ppp_options_directory_ready("/tmp/ppp") != 0)
		return -1;
	return ppp_callback_state_file_lock(PPP_WAN_SESSION_LOCK_FILE,
					    PPP_WAN_SESSION_STATE_FILE,
					    generation, allow_empty,
					    PPP_WAN_SESSION_LOCK_ATTEMPTS);
}

static int
ppp_wan_session_state_unlock(int fd)
{
	return ppp_callback_state_file_unlock(fd);
}

static int
ppp_wan_session_generation_change(unsigned long *generation_out)
{
	int fd, saved_errno;
	unsigned long current, next;

	fd = ppp_wan_session_state_lock(&current, 1);
	if (fd < 0)
		return -1;
	if (ppp_callback_generation_next(current, &next) != 0)
		goto failed_locked;
	if (ppp_callback_state_file_publish(PPP_WAN_SESSION_STATE_FILE, next) != 0)
		goto failed_locked;
	if (ppp_wan_session_state_unlock(fd) != 0)
		return -1;
	if (generation_out)
		*generation_out = next;
	return 0;

failed_locked:
	saved_errno = errno ? errno : EIO;
	(void)ppp_wan_session_state_unlock(fd);
	errno = saved_errno;
	return -1;
}

int
invalidate_wan_ppp_callback_session(void)
{
	if (ppp_wan_session_generation_change(NULL) != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown: cannot invalidate PPP callback session (errno=%d)",
			   errno ? errno : EIO);
		return -1;
	}
	return 0;
}

static int
ppp_wan_callback_session_enter(int argc, char **argv, int *state_fd)
{
	int fd;
	unsigned long current;

	if (!state_fd) {
		errno = EINVAL;
		return -1;
	}
	*state_fd = -1;
	if (argc < 7 || !argv || !argv[6] || !argv[6][0]) {
		errno = EINVAL;
		return -1;
	}
	fd = ppp_wan_session_state_lock(&current, 0);
	if (fd < 0)
		return -1;
	if (!ppp_callback_generation_matches(argv[6], current)) {
		if (ppp_wan_session_state_unlock(fd) != 0)
			return -1;
		return 0;
	}
	*state_fd = fd;
	return 1;
}


static int
get_wan_ppp_peer_copy(int unit, char *peer, size_t peer_size)
{
	int status;

	status = get_wan_unit_value_copy_status(unit, "ppp_peer", peer, peer_size);
	if (status != 0) {
		errno = status;
		return status;
	}
	if (peer[0])
		return 0;
	status = get_wan_unit_value_copy_status(unit, "man_gateway", peer, peer_size);
	if (status != 0)
		errno = status;
	return status;
}


#define XL2TPD_CONFIG_VALUE_SIZE 256

struct xl2tpd_config_snapshot {
	char vpns_enable[XL2TPD_CONFIG_VALUE_SIZE];
	char vpns_type[XL2TPD_CONFIG_VALUE_SIZE];
	char vpnc_enable[XL2TPD_CONFIG_VALUE_SIZE];
	char vpnc_type[XL2TPD_CONFIG_VALUE_SIZE];
	char vpnc_dgw[XL2TPD_CONFIG_VALUE_SIZE];
	char wan_l2tpd[XL2TPD_CONFIG_VALUE_SIZE];
	char vpnc_peer[XL2TPD_CONFIG_VALUE_SIZE];
	char vpnc_user[XL2TPD_CONFIG_VALUE_SIZE];
};

static int
xl2tpd_snapshot_transaction_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
xl2tpd_snapshot_transaction_get(void *context, const char *param_name,
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
xl2tpd_snapshot_transaction_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

static int
xl2tpd_read_config_snapshot(struct xl2tpd_config_snapshot *snapshot)
{
	static const char *const names[8] = {
		"vpns_enable", "vpns_type", "vpnc_enable", "vpnc_type",
		"vpnc_dgw", "wan_l2tpd", "vpnc_peer", "vpnc_user"
	};
	struct rc_wan_nvram_snapshot_field fields[8];
	struct nvram_transaction transaction;
	char *destinations[8];
	unsigned int i;

	if (!snapshot)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));
	destinations[0] = snapshot->vpns_enable;
	destinations[1] = snapshot->vpns_type;
	destinations[2] = snapshot->vpnc_enable;
	destinations[3] = snapshot->vpnc_type;
	destinations[4] = snapshot->vpnc_dgw;
	destinations[5] = snapshot->wan_l2tpd;
	destinations[6] = snapshot->vpnc_peer;
	destinations[7] = snapshot->vpnc_user;
	for (i = 0; i < 8; ++i) {
		fields[i].param_name = names[i];
		fields[i].destination = destinations[i];
		fields[i].destination_size = XL2TPD_CONFIG_VALUE_SIZE;
		}

	return rc_wan_nvram_copy_snapshot(
		fields, 8, xl2tpd_snapshot_transaction_begin,
		xl2tpd_snapshot_transaction_get,
		xl2tpd_snapshot_transaction_end, &transaction);
}


static int
ppp_wan_launch_snapshot_transaction_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
ppp_wan_launch_snapshot_transaction_get(void *context,
					const char *param_name,
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
ppp_wan_launch_snapshot_transaction_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

static int
ppp_wan_launch_mode_snapshot_read(int wan_proto,
				  struct ppp_wan_launch_mode *mode)
{
	char multiwan_value[8];
#if defined (APP_RPL2TP)
	char wan_l2tpd_value[8];
#endif
	struct rc_wan_nvram_snapshot_field fields[2];
	struct nvram_transaction transaction;
	unsigned int field_count = 1;
	int status;

	if (!mode) {
		errno = EINVAL;
		return EINVAL;
	}
	memset(mode, 0, sizeof(*mode));
	multiwan_value[0] = '\0';
	fields[0].param_name = "pppoemwan_enable";
	fields[0].destination = multiwan_value;
	fields[0].destination_size = sizeof(multiwan_value);
#if defined (APP_RPL2TP)
	wan_l2tpd_value[0] = '\0';
	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
		fields[1].param_name = "wan_l2tpd";
		fields[1].destination = wan_l2tpd_value;
		fields[1].destination_size = sizeof(wan_l2tpd_value);
		field_count = 2;
	}
#else
	(void)wan_proto;
#endif

	status = rc_wan_nvram_copy_snapshot(
		fields, field_count,
		ppp_wan_launch_snapshot_transaction_begin,
		ppp_wan_launch_snapshot_transaction_get,
		ppp_wan_launch_snapshot_transaction_end, &transaction);
	if (status != 0) {
		errno = status;
		return status;
	}

#if defined (APP_RPL2TP)
	status = ppp_wan_launch_mode_resolve(
		0, multiwan_value, wan_l2tpd_value, 1,
		(wan_proto == IPV4_WAN_PROTO_L2TP), mode);
#else
	status = ppp_wan_launch_mode_resolve(
		0, multiwan_value, NULL, 0, 0, mode);
#endif
	if (status != 0)
		errno = status;
	return status;
}

static int
ppp_l2tp_runtime_marker_set_status(const char *name, int enabled)
{
	int api_result, saved_errno, status;

	if (!name ||
	    (strcmp(name, "l2tp_cli_t") != 0 &&
	     strcmp(name, "l2tp_wan_t") != 0) ||
	    (enabled != 0 && enabled != 1)) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_set_int_temp(name, enabled);
	saved_errno = errno;
	status = xl2tpd_start_policy_write_status(api_result, saved_errno);
	if (status != 0)
		errno = status;
	return status;
}

int
safe_start_xl2tpd_locked(void)
{
	FILE *fp;
	int unit, wan_proto;
	int use_vpn_server, use_wan_lac, use_vpnc;
	int saved_errno, scan_error, presence_status, launch_status;
	int wan_value_status, config_status, marker_status;
	int vpnc_marker_published = 0, vpnc_marker_rollback_status;
	int vpnc_default_route;
	struct xl2tpd_config_snapshot config_snapshot;
	struct xl2tpd_config_flags config_flags;
	unsigned int vpn_server_address, vpn_server_mask;
	unsigned int vpn_pool_start, vpn_pool_end;
	struct process_snapshot process_snapshot;
	enum process_presence daemon_presence;
	const char *invalid_value = NULL;
	const char *l2tp_conf = "/etc/xl2tpd.conf";
	char hostname[256], wan_peer[256], wan_user[256], wan_value_check[256];
	char vpnc_peer[256], vpnc_user[256];

	hostname[0] = wan_peer[0] = wan_user[0] = '\0';
	vpnc_peer[0] = vpnc_user[0] = '\0';
	unit = 0; /* todo */
	config_status = xl2tpd_read_config_snapshot(&config_snapshot);
	if (config_status != 0) {
		saved_errno = config_status;
		logmessage(LOGNAME,
			   "cannot read coherent xl2tpd selector/credential snapshot; refusing config update (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	/*
	 * A failed protocol read is not proof that the WAN is not L2TP. Refuse
	 * to replace the daemon configuration rather than silently omitting a
	 * required WAN access-concentrator section.
	 */
	wan_value_status = get_wan_unit_proto_checked_status(unit, &wan_proto);
	if (wan_value_status != 0) {
		saved_errno = wan_value_status;
		logmessage(LOGNAME,
			   "cannot read configured WAN protocol; refusing xl2tpd config update (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	config_status = xl2tpd_start_policy_config_flags(
		0, wan_proto, IPV4_WAN_PROTO_L2TP,
#if defined (APP_RPL2TP)
		1,
#else
		0,
#endif
		config_snapshot.vpns_enable, config_snapshot.vpns_type,
		config_snapshot.vpnc_enable, config_snapshot.vpnc_type,
		config_snapshot.vpnc_dgw, config_snapshot.wan_l2tpd,
		&config_flags);
	if (config_status != 0) {
		errno = config_status;
		return -1;
	}
	use_vpn_server = config_flags.use_vpn_server;
	use_wan_lac = config_flags.use_wan_lac;
	use_vpnc = config_flags.use_vpnc;
	vpnc_default_route = config_flags.vpnc_default_route;

	/* UNKNOWN is not proof of a running daemon; reject before config I/O. */
	daemon_presence = process_inventory_scan("xl2tpd", &process_snapshot);
	scan_error = process_snapshot.error;
	process_snapshot_release(&process_snapshot);
	presence_status = xl2tpd_start_policy_result(daemon_presence);
	if (presence_status < 0) {
		saved_errno = scan_error ? scan_error : EIO;
		logmessage(LOGNAME, "cannot verify xl2tpd process state; refusing config/start operation (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	/* Validate all NVRAM-derived, line-oriented values before truncating
	 * the live daemon configuration. Printable credentials/spaces remain
	 * supported, but controls cannot inject additional L2TP options. */
	if (use_vpn_server &&
	    ppp_config_line_value_copy(get_our_hostname(), hostname, sizeof(hostname)) != 0)
		invalid_value = "hostname";
	if (!invalid_value && use_wan_lac) {
		wan_value_status = get_wan_ppp_peer_copy(unit, wan_peer, sizeof(wan_peer));
		if (wan_value_status != 0) {
			errno = wan_value_status;
			invalid_value = "wan peer";
		} else if (ppp_config_line_value_copy(wan_peer, wan_value_check,
						     sizeof(wan_value_check)) != 0) {
			invalid_value = "wan peer";
		}
	}
	if (!invalid_value && use_wan_lac) {
		wan_value_status = get_wan_unit_value_copy_status(
			unit, "pppoe_username", wan_user, sizeof(wan_user));
		if (wan_value_status != 0) {
			errno = wan_value_status;
			invalid_value = "wan username";
		} else if (ppp_config_line_value_copy(wan_user, wan_value_check,
						      sizeof(wan_value_check)) != 0) {
			invalid_value = "wan username";
		}
	}
	if (!invalid_value && use_vpnc &&
	    ppp_config_line_value_copy(config_snapshot.vpnc_peer, vpnc_peer, sizeof(vpnc_peer)) != 0)
		invalid_value = "VPN client peer";
	if (!invalid_value && use_vpnc &&
	    ppp_config_line_value_copy(config_snapshot.vpnc_user, vpnc_user, sizeof(vpnc_user)) != 0)
		invalid_value = "VPN client username";
	if (invalid_value) {
		saved_errno = errno ? errno : EINVAL;
		logmessage(LOGNAME, "refusing unsafe or oversized L2TP config value %s (errno=%d)",
			   invalid_value, saved_errno);
		errno = saved_errno;
		return -1;
	}

	/*
	 * Resolve the VPN server pool before opening the live configuration.
	 * The checked reader owns its NVRAM snapshot; a failure must not leave
	 * xl2tpd with a truncated or partially regenerated configuration.
	 */
	if (use_vpn_server &&
	    get_vpns_pool(&vpn_server_address, &vpn_server_mask,
			  &vpn_pool_start, &vpn_pool_end) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot read or validate VPN server address pool; refusing xl2tpd config (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	if (!(fp = fopen(l2tp_conf, "w"))) {
		perror(l2tp_conf);
		return -1;
	}


	fprintf(fp,
		"[global]\n"
		"port = 1701\n"
		"access control = no\n"
		"rand source = dev\n\n");

	if (use_vpn_server) {
		char sa_v[INET_ADDRSTRLEN], sp_b[INET_ADDRSTRLEN], sp_e[INET_ADDRSTRLEN];
		struct in_addr pool_in;

		pool_in.s_addr = htonl(vpn_server_address);
		strcpy(sa_v, inet_ntoa(pool_in));

		pool_in.s_addr = htonl((vpn_server_address & vpn_server_mask) | vpn_pool_start);
		strcpy(sp_b, inet_ntoa(pool_in));

		pool_in.s_addr = htonl((vpn_server_address & vpn_server_mask) | vpn_pool_end);
		strcpy(sp_e, inet_ntoa(pool_in));

		fprintf(fp, "[lns default]\n");
		fprintf(fp, "hostname = %s\n", hostname);
		fprintf(fp, "local ip = %s\n", sa_v);
		fprintf(fp, "ip range = %s-%s\n", sp_b, sp_e);
		fprintf(fp, "pppoptfile = %s\n", VPN_SERVER_PPPD_OPTIONS);
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp,
			"pass peer = yes\n"
			"exclusive = yes\n"
			"hidden bit = no\n"
			"length bit = yes\n"
			"require chap = yes\n"
			"refuse pap = yes\n\n");

	}

	if (use_wan_lac) {
		char options[64], lac_name[8];

		snprintf(lac_name, sizeof(lac_name), "ISP%d", unit);
		snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);

		fprintf(fp, "[lac %s]\n", lac_name);
		fprintf(fp, "pppoptfile = %s\n", options);
		fprintf(fp, "lns = %s\n", wan_peer);
		fprintf(fp, "name = %s\n", wan_user);
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route_rdgw = %d\n", 1);
		fprintf(fp,
			"autodial = yes\n"
			"redial = yes\n"
			"redial timeout = 15\n"
			"tx bps = 100000000\n"
			"rx bps = 100000000\n\n");

	}

	if (use_vpnc) {
		fprintf(fp, "[lac %s]\n", "VPNC");
		fprintf(fp, "pppoptfile = %s\n", VPN_CLIENT_PPPD_OPTIONS);
		fprintf(fp, "lns = %s\n", vpnc_peer);
		fprintf(fp, "name = %s\n", vpnc_user);
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route_rdgw = %d\n", vpnc_default_route);
		fprintf(fp,
			"autodial = yes\n"
			"redial = yes\n"
			"redial timeout = 15\n"
			"tx bps = 100000000\n"
			"rx bps = 100000000\n\n");

	}

	if (fflush(fp) != 0 || ferror(fp)) {
		saved_errno = errno ? errno : EIO;
		(void)fclose(fp);
		(void)unlink(l2tp_conf);
		logmessage(LOGNAME, "cannot write L2TP configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (fclose(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)unlink(l2tp_conf);
		logmessage(LOGNAME, "cannot close L2TP configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (chmod(l2tp_conf, 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)unlink(l2tp_conf);
		logmessage(LOGNAME, "cannot set L2TP configuration permissions (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (use_vpnc) {
		marker_status = ppp_l2tp_runtime_marker_set_status("l2tp_cli_t", 1);
		if (marker_status != 0) {
			saved_errno = marker_status;
			logmessage(LOGNAME,
				   "refusing xl2tpd launch: cannot publish VPN-client ownership marker (status=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
		vpnc_marker_published = 1;
	}

	/*
	 * Keep the process-presence sentinel (1 = already running) distinct
	 * from eval()'s positive child exit statuses.  A failed daemon launch
	 * must never be reported as a successfully running shared service.
	 */
	launch_status = 0;
	if (presence_status == 0)
		launch_status = eval("/usr/sbin/xl2tpd", "-c", (char *)l2tp_conf);
	presence_status = xl2tpd_start_policy_completion_result(presence_status,
							       launch_status);
	if (presence_status < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot start xl2tpd after confirmed absence (status=%d, errno=%d)",
			   launch_status, saved_errno);
		/*
		 * This helper is shared by WAN PPP and VPN-server startup too.
		 * Roll back the client dependency here rather than relying on the
		 * direct VPN-client caller to repair state after this function fails.
		 */
		if (xl2tpd_start_policy_vpnc_marker_rollback_required(
			    vpnc_marker_published, presence_status)) {
			vpnc_marker_rollback_status =
				ppp_l2tp_runtime_marker_set_status("l2tp_cli_t", 0);
			if (vpnc_marker_rollback_status != 0)
				logmessage(LOGNAME,
					   "cannot roll back VPN-client xl2tpd ownership marker after daemon-start failure (status=%d)",
					   vpnc_marker_rollback_status);
		}
		errno = saved_errno;
		return -1;
	}
	return presence_status;
}

int
safe_start_xl2tpd(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(LOGNAME, "cannot lock VPN server lifecycle for L2TP daemon start (errno=%d)", errno);
		return -1;
	}
	result = safe_start_xl2tpd_locked();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release VPN server lifecycle lock after L2TP daemon start (errno=%d)",
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

#if defined (APP_RPL2TP)
struct rpl2tp_config_values {
	const char *options;
	const char *peer;
};

static int
rpl2tp_config_write(FILE *fp, void *opaque)
{
	const struct rpl2tp_config_values *config =
		(const struct rpl2tp_config_values *)opaque;

	if (!fp || !config || !config->options || !config->peer) {
		errno = EINVAL;
		return -1;
	}
	if (fprintf(fp,
		"# automatically generated\n"
		"global\n"
		"load-handler \"sync-pppd.so\"\n"
		"load-handler \"cmd.so\"\n\n"
		"section sync-pppd\n"
		"lac-pppd-opts \"file %s\"\n\n"
		"section peer\n"
		"peername %s\n"
		"lac-handler sync-pppd\n"
		"persist yes\n"
		"maxfail 0\n"
		"holdoff 15\n"
		"route_rdgw 1\n"
		"hide-avps no\n\n"
		"section cmd\n\n",
		config->options, config->peer) < 0) {
		if (!errno)
			errno = EIO;
		return -1;
	}
	return 0;
}

static int
start_rpl2tp(int unit)
{
	struct rpl2tp_config_values config;
	char options[64], peer[256], peer_check[256];
	int status, saved_errno, length;

	status = get_wan_ppp_peer_copy(unit, peer, sizeof(peer));
	if (status == 0 && ppp_config_line_value_copy(peer, peer_check, sizeof(peer_check)) != 0)
		status = errno ? errno : EINVAL;
	if (status != 0) {
		saved_errno = status > 0 ? status : EIO;
		logmessage(LOGNAME, "refusing unsafe or unreadable rp-l2tp peer (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	length = snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);
	if (length < 0 || (size_t)length >= sizeof(options)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	config.options = options;
	config.peer = peer;
	if (l2tp_config_file_publish("/etc/l2tp", "l2tp.conf",
				     rpl2tp_config_write, &config) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "cannot safely publish rp-l2tp configuration (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	/* Launch rp-l2tp only after the complete config is atomically published. */
	status = eval("/usr/sbin/l2tpd");
	if (status != 0) {
		logmessage(LOGNAME, "rp-l2tp daemon launch failed (status=%d)", status);
		return status;
	}

	sleep(1);
	status = eval("/usr/sbin/l2tp-control", "start-session 0.0.0.0");
	if (xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(status)) {
		char *svcs_l2tp[] = { "l2tpd", NULL };
		int stop_status;

		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "rp-l2tp start-session failed (status=%d); stopping newly launched daemon",
			   status);
		stop_status = kill_services(svcs_l2tp, 5, 1);
		if (stop_status != 0)
			logmessage(LOGNAME,
				   "rp-l2tp daemon rollback after session failure was incomplete (status=%d)",
				   stop_status);
		status = xl2tpd_start_policy_rpl2tp_failure_status(status, stop_status);
		errno = saved_errno;
	}
	return status;
}
#endif

char *
safe_pppd_line(const char *line, char *buf, size_t buf_size)
{
	if (ppp_option_text_escape(line, buf, buf_size) != 0)
		return NULL;

	return buf;
}

static int
ppp_options_write_quoted(FILE *fp, const char *option, const char *value,
			 char *buf, size_t buf_size)
{
	if (!fp || !option || ppp_option_value_escape(value, buf, buf_size) != 0)
		return -1;

	if (fprintf(fp, "%s '%s'\n", option, buf) < 0) {
		if (!errno)
			errno = EIO;
		return -1;
	}
	return 0;
}

static int
ppp_options_write_text(FILE *fp, const char *value, char *buf, size_t buf_size)
{
	char *escaped;

	if (!fp)
		return -1;
	escaped = safe_pppd_line(value, buf, buf_size);
	if (!escaped)
		return -1;
	if (fprintf(fp, "%s\n", escaped) < 0) {
		if (!errno)
			errno = EIO;
		return -1;
	}
	return 0;
}

static int
ppp_wan_option_read_int(void *context, const char *param_name, int *value)
{
	int *unit = (int *)context;
	int status;

	if (!unit)
		return EINVAL;
	status = get_wan_unit_value_int_status(*unit, param_name, value);
	if (status != 0)
		errno = status;
	return status;
}


static int
ppp_wan_option_read_text(void *context, const char *param_name,
			 char *value, size_t value_size)
{
	int *unit = (int *)context;
	int status;

	if (!unit)
		return EINVAL;
	status = get_wan_unit_value_copy_status(*unit, param_name,
					value, value_size);
	if (status != 0)
		errno = status;
	return status;
}


static int
launch_wan_pppd_locked(int unit, int wan_proto,
		      const struct ppp_wan_launch_mode *launch_mode)
{
	FILE *fp;
	struct ppp_wan_option_values option_values;
	struct ppp_wan_option_text_values text_values;
	int auth_type, mtu, mru, mtu_max, mtu_def, mru_def, mru_max;
	int option_status, text_status, saved_errno, launch_status, cleanup_status;
	int wan_marker_published = 0, marker_rollback_status;
	unsigned long callback_generation;
	char callback_generation_text[sizeof(unsigned long) * CHAR_BIT + 1];
	char pppoe_ifname[16];
	const char *failed_option = "unknown";
	char options[64], tmp[256];
	char *svcs[] = { NULL, NULL };
	char *svcs_pppd[] = { "pppd", NULL };

	if (unit < 0 || !launch_mode) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Resolve every integer that affects authentication, encryption, DNS or
	 * PPP negotiation before invalidating the previous session, stopping a
	 * transport daemon, or opening the shared options file.
	 */
	option_status = ppp_wan_option_values_read(
		(wan_proto == IPV4_WAN_PROTO_PPPOE),
		(wan_proto == IPV4_WAN_PROTO_PPTP),
		(wan_proto == IPV4_WAN_PROTO_L2TP),
		ppp_wan_option_read_int, &unit, &option_values);
	if (option_status != 0) {
		saved_errno = option_status > 0 ? option_status : EIO;
		logmessage(LOGNAME,
			   "refusing WAN PPP launch because a required option read failed (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	text_status = ppp_wan_option_text_values_read(
		(wan_proto == IPV4_WAN_PROTO_PPPOE),
		(wan_proto == IPV4_WAN_PROTO_PPTP),
		ppp_wan_option_read_text, &unit, &text_values);
	if (text_status != 0) {
		saved_errno = text_status > 0 ? text_status : EIO;
		logmessage(LOGNAME,
			   "refusing WAN PPP launch because a required text option read failed (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	/* The nic-* line is parsed as a separate pppd option. Never allow an
	 * NVRAM interface name to inject another privileged PPP option. */
	if (wan_proto == IPV4_WAN_PROTO_PPPOE) {
		saved_errno = wan_ifname_copy(pppoe_ifname, sizeof(pppoe_ifname),
					      get_man_ifname(unit));
		if (saved_errno != 0) {
			logmessage(LOGNAME,
				   "refusing PPPoE launch with invalid interface name (status=%d)",
				   saved_errno);
			errno = saved_errno;
			return -1;
		}
	}

	/*
	 * Stop the previous L2TP transport before opening/truncating the shared
	 * PPP options file. A surviving daemon may still read this file when it
	 * starts a session, so a failed stop must prevent all config publication.
	 */
	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
#if defined (APP_RPL2TP)
		if (launch_mode->use_rpl2tp)
			svcs[0] = "l2tpd";
		else
#endif
			svcs[0] = "xl2tpd";

		if (kill_services(svcs, 5, 1) != 0) {
			saved_errno = EBUSY;
			logmessage(LOGNAME, "refusing L2TP WAN launch: %s did not stop", svcs[0]);
			errno = saved_errno;
			return -1;
		}
	}

	/*
	 * Multi-WAN startup replaces all current PPP sessions. Keep this broad
	 * process stop under the VPN lifecycle barrier: VPN server callbacks and
	 * server start/stop use the same lock and must not overlap the kill.
	 */
	if (wan_proto != IPV4_WAN_PROTO_L2TP &&
	    launch_mode->multiwan_enabled) {
		if (kill_services(svcs_pppd, 5, 1) != 0) {
			saved_errno = EBUSY;
			logmessage(LOGNAME, "refusing multi-WAN PPP launch: pppd did not stop");
			errno = saved_errno;
			return -1;
		}
	}

	/*
	 * Fence callbacks only after the previous L2TP/multi-WAN processes have
	 * stopped successfully. A failed stop must leave a surviving session's
	 * ipparam token valid. Keep this before publishing shared PPP options.
	 */
	if (ppp_wan_session_generation_change(&callback_generation) != 0 ||
	    ppp_callback_generation_format(callback_generation, callback_generation_text,
					   sizeof(callback_generation_text)) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot establish WAN PPP callback generation (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);

	/* Keep the credential-bearing directory and file private before writing. */
	if (ppp_options_directory_ready("/tmp/ppp") != 0) {
		saved_errno = errno ? errno : EPERM;
		logmessage(LOGNAME, "refusing unsafe PPP options directory (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (!(fp = ppp_options_file_open(options))) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot open private PPP options file (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	failed_option = "user";
	if (ppp_options_write_quoted(fp, "user", text_values.username, tmp, sizeof(tmp)) != 0)
		goto invalid_options;
	failed_option = "password";
	if (ppp_options_write_quoted(fp, "password", text_values.password, tmp, sizeof(tmp)) != 0)
		goto invalid_options;
	fprintf(fp, "refuse-eap\n");

	auth_type = option_values.auth_type;
	if (auth_type == 3) {
		/* MS-CHAPv2 */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
	}
	else if (auth_type == 2) {
		/* CHAP */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	else if (auth_type == 1) {
		/* PAP */
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	mtu     = 1492;
	mtu_def = 1492;
	mtu_max = 1492;

	mru     = 1492;
	mru_def = 1492;
	mru_max = 1492;

	if (wan_proto == IPV4_WAN_PROTO_PPTP) {
		mtu_def = 1400;
		mtu_max = 1476;
		mru_def = 1400;
		mru_max = 1500;
		mtu = option_values.mtu;
		mru = option_values.mru;
		
		fprintf(fp, "plugin pptp.so\n");
		failed_option = "pptp_server";
		if (ppp_options_write_quoted(fp, "pptp_server", text_values.peer, tmp, sizeof(tmp)) != 0)
			goto invalid_options;
		fprintf(fp, "route_rdgw %d\n", 1);
	}

	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
		mtu_def = 1460;
		mtu_max = 1460;
		mru_def = 1460;
		mru_max = 1500;
		mtu = option_values.mtu;
		mru = option_values.mru;
		
		// L2TP: Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	if (wan_proto == IPV4_WAN_PROTO_PPPOE) {
		int demand;
		char *pppoe_ac, *pppoe_sv;
		
		mtu = option_values.mtu;
		mru = option_values.mru;
		
		fprintf(fp, "plugin rp-pppoe.so\n");
		fprintf(fp, "nic-%s\n", pppoe_ifname);
		
		pppoe_ac = text_values.access_concentrator;
		if (*pppoe_ac) {
			failed_option = "rp_pppoe_ac";
			if (ppp_options_write_quoted(fp, "rp_pppoe_ac", pppoe_ac, tmp, sizeof(tmp)) != 0)
				goto invalid_options;
		}
		
		pppoe_sv = text_values.service;
		if (*pppoe_sv) {
			failed_option = "rp_pppoe_service";
			if (ppp_options_write_quoted(fp, "rp_pppoe_service", pppoe_sv, tmp, sizeof(tmp)) != 0)
				goto invalid_options;
		}
		
		demand = option_values.pppoe_idletime;
		if (demand > 0 && option_values.pppoe_demand > 0) {
			fprintf(fp, "idle %d ", demand);
			if (option_values.pppoe_txonly_x > 0)
				fprintf(fp, "tx_only ");
			fprintf(fp, "demand\n");
		}
	}

	if (wan_proto != IPV4_WAN_PROTO_L2TP)
		fprintf(fp, "persist\n");

	fprintf(fp, "maxfail %d\n", 0);		// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff %d\n", 10);	// pppd re-call time(10s)

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "noipdefault\n");

	if (mtu < 1000) mtu = mtu_def;
	if (mtu > mtu_max) mtu = mtu_max;
	if (mru < 1000) mru = mru_def;
	if (mru > mru_max) mru = mru_max;
	fprintf(fp, "mtu %d\n", mtu);
	fprintf(fp, "mru %d\n", mru);

	if (option_values.dnsenable_x > 0)
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "default-asyncmap\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automatically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	if (wan_proto != IPV4_WAN_PROTO_PPPOE) {
		int mppe_mode = option_values.mppe_mode;
		if (mppe_mode == 0) {
			fprintf(fp, "nomppe nomppc\n");
		} else {
			if (mppe_mode == 2) {
				fprintf(fp, "+mppe\n");
				fprintf(fp, "+mppe-40\n");
				fprintf(fp, "-mppe-128\n");
			} else if (mppe_mode == 3) {
				fprintf(fp, "+mppe\n");
				fprintf(fp, "-mppe-40\n");
				fprintf(fp, "+mppe-128\n");
			} else {
				fprintf(fp, "+mppe-40\n");
				fprintf(fp, "+mppe-128\n");
			}
			fprintf(fp, "nomppe-stateful\n");
		}
	} else {
		fprintf(fp, "nomppe nomppc\n");
	}

	if (option_values.lcp_echo > 0) {
		/* echo failures (6*20s) */
		fprintf(fp, "lcp-echo-interval %d\n", 20);
		fprintf(fp, "lcp-echo-failure %d\n", 6);
	}
	if (option_values.adaptive_lcp > 0)
		fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "minunit %d\n", unit);
	fprintf(fp, "linkname wan%d\n", unit);
	fprintf(fp, "ktune\n");

#if defined (USE_IPV6)
	/* Enable IPv6CP */
	if (is_wan_ipv6_type_sit() == 0 && is_wan_ipv6_if_ppp())
		fprintf(fp, "+ipv6\n");
#endif

	/* user specific options */
	failed_option = "ppp_pppd";
	if (ppp_options_write_text(fp, text_values.user_options, tmp, sizeof(tmp)) != 0)
		goto invalid_options;
	if (fprintf(fp, "ipparam %s\n", callback_generation_text) < 0) {
		if (!errno)
			errno = EIO;
		goto invalid_options;
	}

	if (ppp_options_file_finish(fp, options) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot finalize PPP options file (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	launch_status = 0;
	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
#if defined (APP_RPL2TP)
		if (launch_mode->use_rpl2tp) {
			saved_errno = ppp_l2tp_runtime_marker_set_status("l2tp_wan_t", 0);
			if (saved_errno != 0) {
				logmessage(LOGNAME,
					   "refusing RPL2TP WAN launch: cannot clear xl2tpd ownership marker (status=%d)",
					   saved_errno);
				errno = saved_errno;
				launch_status = -1;
			} else {
				launch_status = start_rpl2tp(unit);
			}
		} else
#endif
		{
			saved_errno = ppp_l2tp_runtime_marker_set_status("l2tp_wan_t", 1);
			if (saved_errno != 0) {
				logmessage(LOGNAME,
					   "refusing xl2tpd WAN launch: cannot publish WAN ownership marker (status=%d)",
					   saved_errno);
				errno = saved_errno;
				launch_status = -1;
			} else {
				/* Publish before daemon start so shared-stop decisions see
				 * this dependency; undo it if this launch does not succeed. */
				wan_marker_published = 1;
				launch_status = xl2tpd_start_policy_wan_launch_result(
					safe_start_xl2tpd_locked());
			}
		}
	} else {
		if (launch_mode->multiwan_enabled)
			launch_status = eval("/usr/bin/mwan.sh", "--rc-lifecycle-locked");
		else
			launch_status = eval("/usr/sbin/pppd", "file", options);
	}

	if (launch_status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "WAN PPP launch command failed (status=%d, errno=%d)",
			   launch_status, saved_errno);
		if (ppp_wan_session_generation_change(NULL) != 0)
			logmessage(LOGNAME, "failed to invalidate callback token after PPP launch failure (errno=%d)", errno);
		/*
		 * Multi-WAN may have launched earlier PPP sessions before a later
		 * pppd invocation failed. Invalidate callbacks first, then roll back
		 * the partial session set while holding the lifecycle lock.
		 */
		if (wan_proto != IPV4_WAN_PROTO_L2TP &&
		    launch_mode->multiwan_enabled) {
			cleanup_status = kill_services(svcs_pppd, 5, 1);
			if (cleanup_status != 0)
				logmessage(LOGNAME,
					   "failed to stop partial multi-WAN PPP sessions after launch failure (status=%d)",
					   cleanup_status);
		}
		/*
		 * The WAN marker is published before xl2tpd start so concurrent
		 * lifecycle decisions preserve this dependency. Do not leave a
		 * phantom dependency if config publication or daemon launch fails.
		 */
		if (xl2tpd_start_policy_wan_marker_rollback_required(
			    wan_marker_published, launch_status)) {
			marker_rollback_status = ppp_l2tp_runtime_marker_set_status(
				"l2tp_wan_t", 0);
			if (marker_rollback_status != 0)
				logmessage(LOGNAME,
					   "cannot roll back WAN xl2tpd ownership marker after launch failure (status=%d)",
					   marker_rollback_status);
		}
		errno = saved_errno;
		return -1;
	}

	return 0;

invalid_options:
	saved_errno = errno ? errno : EINVAL;
	(void)fclose(fp);
	(void)unlink(options);
	logmessage(LOGNAME, "refusing unsafe or oversized PPP option %s (errno=%d)",
		   failed_option, saved_errno);
	errno = saved_errno;
	return -1;
}
int
launch_wan_pppd(int unit, int wan_proto)
{
	struct ppp_wan_launch_mode launch_mode;
	int lock_fd, result, saved_errno, mode_status;

	mode_status = ppp_wan_launch_unit_status(unit);
	if (mode_status != 0) {
		errno = mode_status;
		return -1;
	}
	mode_status = ppp_wan_launch_mode_snapshot_read(wan_proto, &launch_mode);
	if (mode_status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN PPP launch mode snapshot (status=%d)",
			   mode_status);
		errno = mode_status;
		return -1;
	}
	/* Ordinary single-WAN PPPoE/PPTP launch has no shared-process stop. */
	if (wan_proto != IPV4_WAN_PROTO_L2TP && !launch_mode.multiwan_enabled)
		return launch_wan_pppd_locked(unit, wan_proto, &launch_mode);

	/*
	 * L2TP shares xl2tpd with the VPN server; multi-WAN also stops all pppd
	 * sessions from mwan.sh. Both paths must hold the common lifecycle lock.
	 */
	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(LOGNAME, "cannot lock VPN server lifecycle for WAN PPP launch (errno=%d)",
			   errno);
		return -1;
	}
	result = launch_wan_pppd_locked(unit, wan_proto, &launch_mode);
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release VPN server lifecycle lock after WAN PPP launch (errno=%d)",
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
preset_wan_ppp_routes(char *ppp_ifname, int unit)
{
	int status = 0, route_status, primary = 0;

	/* Do not guess whether this link owns the default route on read failure. */
	route_status = get_wan_unit_value_int_status(unit, "primary", &primary);
	if (route_status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN primary-route setting for unit %d (status=%d)",
			   unit, route_status);
		status = route_status_accumulate(status, route_status);
	} else if (primary == 1) {
		route_status = route_add(ppp_ifname, 1, "0.0.0.0", "0.0.0.0", "0.0.0.0");
		route_status = route_status_idempotent_add(route_status);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to restore PPP default route on %s (error=%d)",
				   ppp_ifname, route_status);
	}

	/* Continue installing interface-dependent routes after a prior failure. */
	route_status = add_static_wan_routes(ppp_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to restore PPP static routes on %s (error=%d)",
			   ppp_ifname, route_status);

	return status;
}

int
ppp_ifindex(char *ifname)
{
	int index;

	if (ppp_callback_parse_index(ifname, "ppp", VPNC_PPP_UNIT, &index) != 0)
		return -1;

	return index;
}

static int
ppp_wan_unit(char *linkname)
{
	int unit;

	/* start_wan() currently manages logical WAN unit 0 only. Reject callbacks
	 * claiming an unknown WAN unit rather than partially parsing LINKNAME. */
	if (ppp_callback_parse_index(linkname, "wan", 1, &unit) != 0)
		return -1;

	return unit;
}

static int
is_valid_wan_ppp_idx(int ifindex)
{
	if (ifindex < 0 || ifindex >= VPNC_PPP_UNIT)
		return 0;
	return 1;
}

struct ppp_wan_ipup_context {
	char *ifname;
	int unit;
	char *remote_ip;
	char *dns;
	struct nvram_transaction *transaction;
};

static int
ppp_wan_ipup_set_interface_address(void *opaque, const char *ipaddr,
				   const char *netmask)
{
	struct ppp_wan_ipup_context *context =
		(struct ppp_wan_ipup_context *)opaque;

	return ifconfig(context->ifname, IFUP, (char *)ipaddr, (char *)netmask);
}

static int
ppp_wan_ipup_set_nvram_value_status(void *opaque, const char *param_name,
				    const char *value)
{
	struct ppp_wan_ipup_context *context =
		(struct ppp_wan_ipup_context *)opaque;
	char wan_param[64];
	int length, api_result, saved_errno;

	if (!context || !context->transaction || !param_name ||
	    !param_name[0] || !value)
		return EINVAL;
	length = snprintf(wan_param, sizeof(wan_param), "wan%d_%s",
			  context->unit, param_name);
	if (length < 0)
		return errno ? errno : EIO;
	if ((size_t)length >= sizeof(wan_param))
		return ENAMETOOLONG;
	errno = 0;
	api_result = nvram_transaction_set(context->transaction, wan_param,
					   (char *)value, 1);
	saved_errno = errno;
	if (api_result < 0)
		return saved_errno ? saved_errno : EIO;
	return api_result;
}

static int
ppp_wan_ipup_get_nvram_value(void *opaque, const char *param_name,
			     char **value)
{
	struct ppp_wan_ipup_context *context =
		(struct ppp_wan_ipup_context *)opaque;
	char wan_param[64];
	int length;

	if (!context || !context->transaction || !param_name ||
	    !param_name[0] || !value) {
		errno = EINVAL;
		return -1;
	}
	length = snprintf(wan_param, sizeof(wan_param), "wan%d_%s",
			  context->unit, param_name);
	if (length < 0)
		return -1;
	if ((size_t)length >= sizeof(wan_param)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return nvram_transaction_get(context->transaction, wan_param, value);
}


static int
ppp_wan_ipup_publish_address(void *opaque, const char *ipaddr,
			     const char *netmask)
{
	static const char *const param_names[] = {
		"ipaddr", "netmask", "ifname_t", "gateway", "dns"
	};
	struct ppp_wan_ipup_context *context =
		(struct ppp_wan_ipup_context *)opaque;
	struct dhcp_lease_nvram_value values[5];
	struct nvram_transaction transaction;
	char previous_text[5][256];
	unsigned int i;
	int status, end_status, rollback_status = 0;
	int publish_attempted = 0;
	int recovery_status, recovery_detail = 0;

	if (!context || !context->ifname || !context->ifname[0] ||
	    !context->remote_ip || !context->dns)
		return EINVAL;
	values[0].value = ipaddr;
	values[1].value = netmask;
	values[2].value = context->ifname;
	values[3].value = context->remote_ip;
	values[4].value = context->dns;
	for (i = 0; i < 5; ++i) {
		values[i].name = param_names[i];
		values[i].previous_value = previous_text[i];
	}

	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;
	context->transaction = &transaction;
	status = 0;
	for (i = 0; i < 5; ++i) {
		status = rc_wan_nvram_copy_value(
			ppp_wan_ipup_get_nvram_value, context, param_names[i],
			previous_text[i], sizeof(previous_text[i]));
		if (status != 0)
			break;
	}
	if (status == 0) {
		/* Keep the lock from the coherent old-tuple read through publish. */
		publish_attempted = 1;
		status = dhcp_lease_nvram_publish(
			values, 5, ppp_wan_ipup_set_nvram_value_status,
			context, &rollback_status);
	}
	errno = 0;
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		else if (publish_attempted && rollback_status == 0)
			rollback_status = end_status;
	}
	context->transaction = NULL;
	if (status == 0)
		return 0;
	if (!publish_attempted)
		return status;

	/* Restore the full old tuple under a fresh NVRAM transaction. */
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0) {
		recovery_status = errno ? errno : EIO;
	} else {
		context->transaction = &transaction;
		recovery_status = dhcp_lease_nvram_restore(
			values, 5, ppp_wan_ipup_set_nvram_value_status,
			context, &recovery_detail);
		errno = 0;
		if (nvram_transaction_end(&transaction) != 0) {
			end_status = errno ? errno : EIO;
			if (recovery_status == 0)
				recovery_status = end_status;
			else if (recovery_detail == 0)
				recovery_detail = end_status;
		}
		context->transaction = NULL;
	}
	if (recovery_status != 0)
		logmessage(LOGNAME,
			   "failed to restore previous PPP WAN NVRAM snapshot on unit %d after ip-up publication failure (publish=%d restore=%d detail=%d)",
			   context->unit, status, recovery_status, recovery_detail);
	return status;
}

static int
ppp_wan_ipup_clear_interface_address(void *opaque)
{
	struct ppp_wan_ipup_context *context =
		(struct ppp_wan_ipup_context *)opaque;
	int status;

	if (!context || !context->ifname || !context->ifname[0])
		return EINVAL;
	status = ifconfig(context->ifname, 0, NULL, NULL);
	if (status != 0)
		logmessage(LOGNAME,
			   "failed to take down PPP interface %s after ip-up snapshot publication failure (status=%d)",
			   context->ifname, status);
	return status;
}

static const struct ppp_wan_ipup_ops ppp_wan_ipup_ops = {
	ppp_wan_ipup_set_interface_address,
	ppp_wan_ipup_publish_address,
	ppp_wan_ipup_clear_interface_address
};

int
ipup_main(int argc, char **argv)
{
	int unit, ppp_idx, interface_status, route_status, unlock_status;
	int callback_fd, callback_state;
	struct ppp_wan_ipup_context interface_context;
	char buf[256], *value, *local_ip, *remote_ip;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");
	char *ppp_mask = "255.255.255.255";

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	callback_state = ppp_wan_callback_session_enter(argc, argv, &callback_fd);
	if (callback_state <= 0)
		return callback_state == 0 ? 0 : -1;

	umask(0000);

	local_ip = getenv("IPLOCAL");
	remote_ip = getenv("IPREMOTE");
	buf[0] = 0;
	value = getenv("DNS1");
	if (value)
		snprintf(buf, sizeof(buf), "%s", value);
	value = getenv("DNS2");
	if (value && strcmp(value, buf) != 0) {
		int buf_len = strlen(buf);
		snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", value);
	}
	interface_context.ifname = ppp_ifname;
	interface_context.unit = unit;
	interface_context.remote_ip = remote_ip;
	interface_context.dns = buf;
	interface_context.transaction = NULL;
	interface_status = ppp_wan_ipup_apply_address(
		local_ip, remote_ip, ppp_mask, &ppp_wan_ipup_ops, &interface_context);
	if (interface_status != 0) {
		logmessage(LOGNAME,
			   "refusing PPP ip-up state publication on %s because interface setup or the complete NVRAM snapshot failed (status=%d)",
			   ppp_ifname, interface_status);
		if (ppp_wan_session_state_unlock(callback_fd) != 0)
			return -1;
		return interface_status;
	}

	route_status = wan_up(ppp_ifname, unit, 0);

	set_pppoe_balancing();

	if (route_status != 0)
		logmessage(get_wan_unit_value(unit, "proto_t"),
			   "Connected but WAN route setup is incomplete (status=%d)",
			   route_status);
	else
		logmessage(get_wan_unit_value(unit, "proto_t"), "Connected");

	unlock_status = ppp_wan_session_state_unlock(callback_fd);
	return ppp_wan_ipup_finish_status(route_status, unlock_status);
}

int
ipdown_main(int argc, char **argv)
{
	int unit, ppp_idx, teardown_status, restore_status;
	int route_status, unlock_status;
	int callback_fd, callback_state;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	callback_state = ppp_wan_callback_session_enter(argc, argv, &callback_fd);
	if (callback_state <= 0)
		return callback_state == 0 ? 0 : -1;

	umask(0000);

	teardown_status = wan_down(ppp_ifname, unit, 0);
	restore_status = preset_wan_ppp_routes(ppp_ifname, unit);
	route_status = route_status_accumulate(teardown_status, restore_status);

	if (route_status != 0)
		logmessage(get_wan_unit_value(unit, "proto_t"),
			   "Disconnected but WAN route cleanup/restoration is incomplete (status=%d)",
			   route_status);
	else
		logmessage(get_wan_unit_value(unit, "proto_t"), "Disconnected");

	unlock_status = ppp_wan_session_state_unlock(callback_fd);
	return route_status_finish(route_status, unlock_status);
}

#if defined (USE_IPV6)
int
ipv6up_main(int argc, char **argv)
{
	int unit, ppp_idx, ipv6_status;
	int callback_fd, callback_state;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	callback_state = ppp_wan_callback_session_enter(argc, argv, &callback_fd);
	if (callback_state <= 0)
		return callback_state == 0 ? 0 : -1;
	if (!is_wan_ipv6_if_ppp()) {
		return ppp_wan_session_state_unlock(callback_fd);
	}


	umask(0000);

	ipv6_status = wan6_up(ppp_ifname, unit);

	if (ppp_wan_session_state_unlock(callback_fd) != 0)
		return -1;
	return ipv6_status;
}

int
ipv6down_main(int argc, char **argv)
{
	int unit, ppp_idx, ipv6_status;
	int callback_fd, callback_state;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	callback_state = ppp_wan_callback_session_enter(argc, argv, &callback_fd);
	if (callback_state <= 0)
		return callback_state == 0 ? 0 : -1;
	if (!is_wan_ipv6_if_ppp()) {
		return ppp_wan_session_state_unlock(callback_fd);
	}


	umask(0000);

	ipv6_status = wan6_down(ppp_ifname, unit);

	refresh_resolvconf_nonfatal("PPP IPv6 down", 0);

	if (ppp_wan_session_state_unlock(callback_fd) != 0)
		return -1;
	return ipv6_status;
}
#endif

