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
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/swap.h>
#include <net/ethernet.h>

#include <dev_info.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "notify_rc.h"
#include "nvram_linux.h"
#include "service_start_status.h"
#include "modem_wan_start_policy.h"
#include "usb_hotplug_event_policy.h"
#include "process_inventory.h"
#include "service_wait_policy.h"
#include "service_daemon_wait_policy.h"
#include "storage_operation_lock.h"
#include "samba_share_policy.h"
#include "samba_config_mode_policy.h"
#include "wan_nvram_status_policy.h"
#include "service_connection_limit_policy.h"
#include "samba_fastpath_policy.h"
#if defined (APP_SMBD)
#include "samba_config_file.h"
#include "samba_daemon_policy.h"
#include "samba_process_policy.h"
#include "samba_config_transaction.h"
#endif
#include "samba_passdb_policy.h"
#include "ftp_share_policy.h"
#include "ftp_passive_port_policy.h"
#include "ftp_transfer_rate_policy.h"
#include "ftp_config_file.h"
#include "ftp_daemon_policy.h"
#include "storage_mount_inventory_policy.h"
#include "storage_optware_start_policy.h"
#include "storage_deferred_start_policy.h"
#include "storage_ejected_inventory_policy.h"
#include "storage_home_link_policy.h"
#include "storage_nfs_export_policy.h"
#include "storage_service_stop_policy.h"
#include "storage_service_enable_policy.h"
#include "firefly_config_file.h"
#include "minidlna_config_file.h"
#include "switch.h"

#ifndef PROC_SWAPS_FILE
#define PROC_SWAPS_FILE "/proc/swaps"
#endif

void
spindown_hdd(char *sd_dev)
{
	char *argv[3];
	int spindown_status;

	if (!sd_dev)
		return;

	if (strncmp(sd_dev, "sd", 2) != 0)
		return;

	argv[0] = "/sbin/spindown.sh";
	argv[1] = sd_dev;
	argv[2] = NULL;
	spindown_status = _eval(argv, NULL, 0, NULL);
	if (spindown_status != 0)
		logmessage("Storage", "disk spindown failed for %s (status=%d)",
			   sd_dev, spindown_status);
}

static int storage_optware_result_fd = -1;
static int storage_optware_start_ack_pending;

static int create_mp_link_status(char *search_dir, char *link_path,
				 int force_first_valid);
static int storage_rollback_stor_apps_after_stop_failure(
	const char *operation, int stop_status);

static int
is_valid_storage_device(const char *devname)
{
	if (strncmp(devname, "/dev/sd", 7) == 0
#if defined (USE_MMC_SUPPORT)
	 || strncmp(devname, "/dev/mmcblk", 11) == 0
#endif
	    )
		return 1;

	return 0;
}

/*
 * A failed mount inventory must never be confused with an empty one.
 * Dependent services stay stopped until /proc/mounts can be read reliably.
 */
static int
storage_has_mounted_devices(const char *operation)
{
	int mount_count;

	mount_count = count_stor_mountpoint();
	if (!storage_mount_inventory_is_complete(mount_count)) {
		logmessage("Storage", "cannot determine mounted storage from %s while %s; refusing storage-dependent service start",
			   PROC_MOUNTS_FILE, operation);
		return -1;
	}

	return storage_mount_inventory_has_mounts(mount_count) ? 1 : 0;
}

/*
 * A failed /proc scan is UNKNOWN, not proof that a storage-backed daemon
 * disappeared. Preserve this distinction across eject and restart decisions.
 */
static enum process_presence
storage_daemon_presence(const char *daemon, int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	int saved_error;

	if (!daemon || !*daemon) {
		errno = EINVAL;
		if (scan_error)
			*scan_error = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	presence = process_inventory_scan(daemon, &snapshot);
	saved_error = snapshot.error > 0 ? snapshot.error : EIO;
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (presence == PROCESS_PRESENCE_UNKNOWN)
		errno = saved_error;
	return presence;
}

/*
 * Launcher exit status and daemon presence are separate postconditions:
 * scripts may fail, and a successful launcher can exit before/without a
 * daemon being visible. Report both rather than silently accepting either.
 */
#if defined (APP_MINIDLNA) || defined (APP_FIREFLY) || defined (APP_TRMD) || defined (APP_ARIA)
static int
storage_log_daemon_start_result(const char *apps_name, int launch_status,
				enum process_presence process_state,
				int process_error)
{
	int status = launch_status;
	int process_status;

	if (launch_status != 0)
		logmessage(apps_name, "launch command failed (status=%d)", launch_status);

	if (process_state == PROCESS_PRESENCE_RUNNING) {
		if (launch_status == 0)
			logmessage(apps_name, "daemon is started");
		else
			logmessage(apps_name, "daemon appeared despite a failed launch command; rolling back fresh start");
	} else if (process_state == PROCESS_PRESENCE_ABSENT) {
		logmessage(apps_name, "daemon is absent in post-launch process inventory");
		status = rc_first_service_start_status(status, ENOENT);
	} else {
		process_status = process_error > 0 ? process_error : EIO;
		logmessage(apps_name, "cannot verify daemon startup (scan error=%d)",
			   process_status);
		status = rc_first_service_start_status(status, process_status);
	}

	return status;
}
#endif

#if defined (APP_FTPD) || defined (APP_MINIDLNA) || defined (APP_FIREFLY)
static int
storage_daemon_wait_is_running(void *context, const char *daemon)
{
	(void)context;
	return storage_daemon_presence(daemon, NULL);
}

static int
storage_daemon_wait_sleep_us(void *context, unsigned int microseconds)
{
	(void)context;
	return service_wait_microseconds(microseconds);
}

static int
storage_wait_for_daemon(const char *daemon)
{
	static const struct service_daemon_wait_ops ops = {
		storage_daemon_wait_is_running,
		storage_daemon_wait_sleep_us
	};

	return service_daemon_wait_for_stable_presence(daemon, 30, &ops, NULL);
}
#endif

#if defined (APP_FTPD)
int
is_ftp_run(void)
{
	return storage_daemon_presence("vsftpd", NULL);
}

static int
stop_ftp_locked(int *final_presence)
{
	char* svcs[] = { "vsftpd", NULL };
	int status, final_state;

	status = kill_services(svcs, 3, 1);
	final_state = is_ftp_run();
	if (final_presence)
		*final_presence = final_state;
	if (storage_service_stop_complete(status, final_state) != 0) {
		logmessage("FTP server", "vsftpd did not stop cleanly or process state is unknown");
		return -1;
	}
	return 0;
}

/* Only a fresh failed start is eligible for rollback; RUNNING was
 * rejected before launch and existing daemons are never torn down here. */
static void
ftp_rollback_failed_start(int start_errno, const char *operation)
{
	int stop_status;
	int scan_error;
	struct process_snapshot snapshot;
	enum process_presence presence;

	memset(&snapshot, 0, sizeof(snapshot));
	stop_status = stop_ftp_locked(NULL);
	presence = process_inventory_scan("vsftpd", &snapshot);
	scan_error = snapshot.error;
	if (ftp_failed_start_may_leave_daemon(presence)) {
		logmessage("FTP server",
			   "vsftpd may remain active after failed %s (rollback_status=%d, state=%d, scan_error=%d)",
			   operation, stop_status, presence, scan_error);
	} else if (stop_status != 0) {
		logmessage("FTP server",
			   "vsftpd is absent after failed %s rollback but stop reported status=%d",
			   operation, stop_status);
	}
	process_snapshot_release(&snapshot);
	errno = start_errno > 0 ? start_errno : EIO;
}

int
stop_ftp(void)
{
	int storage_lock, status, is_run_before, final_presence;
	int firewall_status = 0;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("FTP server", "cannot acquire storage operation lock; refusing standalone FTP stop");
		return -1;
	}

	is_run_before = is_ftp_run();
	status = stop_ftp_locked(&final_presence);
	if (ftp_stop_should_reconcile_firewall(final_presence)) {
		firewall_status = control_ftp_fw(is_run_before);
		if (firewall_status != 0)
			logmessage("FTP server", "firewall/UPnP refresh failed after FTP stop (status=%d)",
				   firewall_status);
	}
	status = rc_first_service_start_status(status, firewall_status);
	file_unlock(storage_lock);
	return status;
}

static int
storage_read_integer_status(const char *name, int missing_value, int *value)
{
	char *text = NULL;
	int api_result, saved_errno;

	if (!name || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_get_checked(name, &text);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(api_result, saved_errno,
					    text, missing_value, value);
}

static int
storage_ack_event_marker_status(const char *name)
{
	int write_status, saved_errno;

	if (!name || !name[0])
		return EINVAL;
	errno = 0;
	write_status = nvram_set_int_temp(name, 0);
	saved_errno = errno;
	if (write_status == 0)
		return 0;
	if (saved_errno > 0)
		return saved_errno;
	return write_status > 0 ? write_status : EIO;
}

static int
storage_read_nvram_string(void *context, const char *name, char **value)
{
	(void)context;
	return nvram_get_checked(name, value);
}


static int
storage_read_transaction_nvram_string(void *context, const char *name,
				      char **value)
{
	if (!context || !name || !value) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_get((struct nvram_transaction *)context,
				     name, value);
}

static int
storage_read_string_status(const char *name, char *value, size_t value_size)
{
	int status;

	status = rc_wan_nvram_copy_value(storage_read_nvram_string, NULL,
					 name, value, value_size);
	if (status != 0)
		errno = status;
	return status;
}


static int
storage_read_transaction_integer_status(struct nvram_transaction *transaction,
					const char *name, int missing_value,
					int *value)
{
	char *text = NULL;
	int api_result, saved_errno;

	if (!transaction || !name || !value) {
		errno = EINVAL;
		return EINVAL;
	}
	errno = 0;
	api_result = nvram_transaction_get(transaction, name, &text);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(api_result, saved_errno,
					    text, missing_value, value);
}


static int
storage_read_transaction_string_status(struct nvram_transaction *transaction,
				       const char *name, char *value,
				       size_t value_size)
{
	int status;

	if (!transaction || !name || !value || value_size == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	status = rc_wan_nvram_copy_value(storage_read_transaction_nvram_string,
					 transaction, name, value, value_size);
	if (status != 0)
		errno = status;
	return status;
}



static int
storage_service_enabled_status(const char *key, const char *service,
			       int *enabled)
{
	char configured_value[NVRAM_MAX_VALUE_LEN + 1] = {0};
	int read_status, status;

	if (!enabled)
		return EINVAL;
	*enabled = 0;

	read_status = storage_read_string_status(
		key, configured_value, sizeof(configured_value));
	status = storage_service_enable_value_status(
		read_status, configured_value, enabled);
	if (status != 0) {
		logmessage(service,
			   "cannot read service enable setting %s; refusing startup (status=%d)",
			   key ? key : "(null)", status);
		return status;
	}

	return 0;
}


int
write_vsftpd_conf(void)
{
	FILE *fp = NULL;
	char staged_path[64];
	int i_maxuser = 0, max_user_value = 0;
	int ftp_mode_value = 0, ftp_log_value = 0;
	int ftp_anon_rate_value = 0, ftp_anon_rate = 0;
	int ftp_min_value = 50000, ftp_max_value = 50100;
	int ftp_min_port = 50000, ftp_max_port = 50100;
	int max_user_status, ftp_mode_status, ftp_log_status;
	int ftp_min_status, ftp_max_status, ftp_passive_port_status;
	int config_status, saved_errno, i_ftp_mode, ftp_anon_rate_status;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status;
#endif
	const char *banner_product;

	ftp_mode_status = storage_read_integer_status(
		"st_ftp_mode", 0, &ftp_mode_value);
	i_ftp_mode = ftp_share_mode_normalize(ftp_mode_value);
	ftp_log_status = storage_read_integer_status(
		"st_ftp_log", 0, &ftp_log_value);
	ftp_anon_rate_status = 0;
	if (ftp_transfer_rate_setting_required(i_ftp_mode)) {
		ftp_anon_rate_status = storage_read_integer_status(
			"st_ftp_anmr", 0, &ftp_anon_rate_value);
		ftp_anon_rate_status = ftp_transfer_rate_value_status(
			ftp_anon_rate_status, ftp_anon_rate_value, 10000, &ftp_anon_rate);
	}
	max_user_status = storage_read_integer_status(
		"st_max_user", 0, &max_user_value);
	max_user_status = service_connection_limit_value_status(
		max_user_status, max_user_value, MAX_CLIENTS_NUM, &i_maxuser);
	ftp_min_status = storage_read_integer_status(
		"st_ftp_pmin", 50000, &ftp_min_value);
	ftp_max_status = storage_read_integer_status(
		"st_ftp_pmax", 50100, &ftp_max_value);
	ftp_passive_port_status = ftp_passive_port_range_status(
		ftp_min_status, ftp_min_value, ftp_max_status, ftp_max_value,
		&ftp_min_port, &ftp_max_port);
	config_status = ftp_mode_status != 0 ? ftp_mode_status :
		ftp_log_status != 0 ? ftp_log_status :
		max_user_status != 0 ? max_user_status :
		ftp_passive_port_status != 0 ? ftp_passive_port_status : ftp_anon_rate_status;
	if (config_status != 0) {
		saved_errno = config_status > 0 ? config_status : EIO;
		logmessage("FTP server",
			   "cannot read FTP configuration; preserving active configuration (errno %d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		saved_errno = ipv6_type_status > 0 ? ipv6_type_status : EIO;
		logmessage("FTP server",
			   "cannot determine IPv6 listener mode; preserving active configuration (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
#endif

	if (ftp_config_file_open("/etc/vsftpd.conf", staged_path,
				 sizeof(staged_path), &fp) != 0)
		return -1;

	banner_product = nvram_safe_get("productid");
	if (!samba_conf_text_is_safe(banner_product) || !*banner_product)
		banner_product = "Padavan";

#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		fprintf(fp, "listen_ipv6=%s\n", "YES");
		fprintf(fp, "listen=%s\n", "NO");
	} else
#endif
		fprintf(fp, "listen=%s\n", "YES");

	fprintf(fp, "background=%s\n", "YES");
	fprintf(fp, "connect_from_port_20=%s\n", "NO");
	fprintf(fp, "pasv_enable=%s\n", "YES");
	fprintf(fp, "pasv_min_port=%d\n", ftp_min_port);
	fprintf(fp, "pasv_max_port=%d\n", ftp_max_port);
	fprintf(fp, "use_sendfile=%s\n", "YES");
#if defined (SUPPORT_FTPD_SSL)
	fprintf(fp, "ssl_enable=%s\n", "NO");
#endif

	if (i_ftp_mode == 1 || i_ftp_mode == 3) {
		fprintf(fp, "local_enable=%s\n", "NO");
		fprintf(fp, "anonymous_enable=%s\n", "YES");
		if (i_ftp_mode == 1){
			fprintf(fp, "anon_upload_enable=%s\n", "YES");
			fprintf(fp, "anon_mkdir_write_enable=%s\n", "YES");
			fprintf(fp, "anon_other_write_enable=%s\n", "YES");
			fprintf(fp, "anon_umask=%s\n", "000");
		}
	} else {
		fprintf(fp, "local_enable=%s\n", "YES");
		fprintf(fp, "local_umask=%s\n", "000");
		fprintf(fp, "anonymous_enable=%s\n", (i_ftp_mode == 2) ? "NO" : "YES");
	}

	if (i_ftp_mode == 3 || i_ftp_mode == 4)
		fprintf(fp, "anon_max_rate=%u\n", (unsigned int)ftp_anon_rate * 1024U);

	/* Do not run vsftpd unprivileged workers as root. */
	fprintf(fp, "nopriv_user=%s\n", "nobody");
	fprintf(fp, "write_enable=%s\n", "YES");
	fprintf(fp, "chroot_local_user=%s\n", "YES");
	fprintf(fp, "allow_writable_root=%s\n", "YES");
	fprintf(fp, "check_shell=%s\n", "NO");
	fprintf(fp, "xferlog_enable=%s\n", "NO");
	fprintf(fp, "syslog_enable=%s\n", (ftp_log_value == 0) ? "NO" : "YES");
	fprintf(fp, "force_dot_files=%s\n", "YES");
	fprintf(fp, "dirmessage_enable=%s\n", "YES");
	fprintf(fp, "hide_ids=%s\n", "YES");
	fprintf(fp, "utf8=%s\n", "YES");
	fprintf(fp, "idle_session_timeout=%d\n", 600);

	fprintf(fp, "max_clients=%d\n", i_maxuser);
	fprintf(fp, "max_per_ip=%d\n", i_maxuser);
	fprintf(fp, "ftpd_banner=Welcome to %s FTP service.\n", banner_product);
	
	if (ftp_config_file_publish(&fp, staged_path, "/etc/vsftpd.conf") != 0)
		return -1;

	return 0;
}

static int
ftp_wait_for_daemon(const char *daemon)
{
	return storage_wait_for_daemon(daemon);
}

static int
ftp_read_enabled_status(int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, read_status, status;
	int parsed = 1;

	if (!enabled) {
		errno = EINVAL;
		return -1;
	}
	*enabled = 0;
	errno = 0;
	api_result = nvram_get_checked("enable_ftp", &value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_int_status(api_result, saved_errno,
						   value, 1, &parsed);
	status = ftp_enable_value_status(read_status, parsed, enabled);
	if (status != 0) {
		errno = status;
		return -1;
	}
	return 0;
}


int
run_ftp(void)
{
	int saved_errno, command_status;
	int process_state;
	int start_errno, ftp_enabled;

	if (ftp_read_enabled_status(&ftp_enabled) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("FTP server",
			   "cannot determine FTP enable state; refusing startup (errno %d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (!ftp_enabled)
		return 0;

	process_state = is_ftp_run();
	if (process_state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		saved_errno = errno ? errno : EIO;
		logmessage("FTP server", "startup skipped because daemon presence is unknown (errno %d)", saved_errno);
		errno = saved_errno;
		return -1;
	}

	if (write_vsftpd_conf() != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("FTP server", "configuration publication failed; startup skipped (errno %d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	command_status = eval("/sbin/vsftpd");
	if (command_status != 0) {
		start_errno = errno ? errno : EIO;
		logmessage("FTP server", "vsftpd start command failed (status %d)", command_status);
		ftp_rollback_failed_start(start_errno, "start");
		return -1;
	}
	if (ftp_wait_for_daemon("vsftpd") != 0) {
		start_errno = errno ? errno : ETIMEDOUT;
		logmessage("FTP server", "vsftpd did not remain running after startup (errno %d)",
			   start_errno);
		ftp_rollback_failed_start(start_errno, "start");
		return -1;
	}

	logmessage("FTP server", "daemon is started");
	return 0;
}

int
control_ftp_fw(int is_run_before)
{
	int is_run_after = is_ftp_run();

	/*
	 * Rebuild from checked configuration on every presence transition. Also
	 * force reconciliation after a confirmed stop even when the daemon was
	 * already absent: stale FTP/NAT/UPnP rules can outlive a crashed daemon.
	 */
	if (ftp_firewall_reconcile_needed(is_run_before, is_run_after))
		return restart_firewall();

	return 0;
}

int
restart_ftpd(void)
{
	int storage_lock, is_run_before, final_presence;
	int stop_status, mount_status, start_status = 0, restart_status = -1;
	int firewall_status = 0, ftp_enabled;

	/* Serialize stop/inventory/start with hotplug and safe storage removal. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("FTP server", "cannot acquire storage operation lock; refusing FTP restart");
		return -1;
	}

	if (ftp_read_enabled_status(&ftp_enabled) != 0) {
		int saved_errno = errno ? errno : EIO;

		logmessage("FTP server",
			   "cannot determine FTP enable state; refusing restart before daemon stop (errno %d)",
			   saved_errno);
		errno = saved_errno;
		goto out;
	}

	is_run_before = is_ftp_run();
	stop_status = stop_ftp_locked(&final_presence);
	if (stop_status != 0) {
		logmessage("FTP server", "restart aborted because existing daemon did not stop");
		if (ftp_stop_should_reconcile_firewall(final_presence)) {
			firewall_status = control_ftp_fw(is_run_before);
			if (firewall_status != 0)
				logmessage("FTP server", "firewall/UPnP refresh failed after incomplete FTP stop (status=%d)",
					   firewall_status);
		}
		restart_status = rc_first_service_start_status(stop_status, firewall_status);
		goto out;
	}

	errno = 0;
	mount_status = storage_has_mounted_devices("restarting FTP");
	if (mount_status < 0) {
		start_status = errno > 0 ? errno : EIO;
		logmessage("FTP server",
			   "restart cannot verify mounted storage; refusing FTP start (status=%d)",
			   start_status);
	} else if (mount_status > 0) {
		start_status = run_ftp();
		if (start_status != 0)
			logmessage("FTP server", "FTP restart failed (status=%d)",
				   start_status);
	}

	/*
	 * A completed stop may have removed FTP port/UPnP state even when storage
	 * is absent, its inventory is unavailable, or the replacement daemon
	 * failed to start. Always reconcile firewall state after that stop.
	 */
	if (ftp_stop_should_reconcile_firewall(final_presence)) {
		firewall_status = control_ftp_fw(is_run_before);
		if (firewall_status != 0)
			logmessage("FTP server", "firewall/UPnP refresh failed after FTP restart (status=%d)",
				   firewall_status);
	}

	restart_status = rc_first_service_start_status(start_status, firewall_status);
out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

#if defined (APP_SMBD)
static int
samba_service_account_is_safe(const char *name,
			      const char *router_username)
{
	return samba_acl_user_is_safe(name) &&
	       !router_account_name_is_reserved(name, router_username);
}
static int
samba_write_all(int fd, const char *data, size_t length)
{
	size_t offset = 0;
	ssize_t written;

	while (offset < length) {
		written = write(fd, data + offset, length - offset);
		if (written < 0 && errno == EINTR)
			continue;
		if (written <= 0)
			return -1;
		offset += (size_t)written;
	}

	return 0;
}

/* Pass plaintext credentials only through a private unlinked stdin file. */
static int
samba_run_smbpasswd(const char *passdb_path, const char *username,
		    const char *password)
{
	char input_path[] = "/tmp/.smbpasswd-input.XXXXXX";
	char newline = '\n';
	int input_fd = -1, status;
	pid_t child, waited;

	if (!samba_passdb_password_is_safe(password))
		return -1;

	input_fd = mkstemp(input_path);
	if (input_fd < 0)
		return -1;

	if (fchmod(input_fd, 0600) != 0 ||
	    samba_write_all(input_fd, password, strlen(password)) != 0 ||
	    samba_write_all(input_fd, &newline, 1) != 0 ||
	    samba_write_all(input_fd, password, strlen(password)) != 0 ||
	    samba_write_all(input_fd, &newline, 1) != 0 ||
	    lseek(input_fd, 0, SEEK_SET) < 0 ||
	    unlink(input_path) != 0)
		goto failed;

	child = fork();
	if (child < 0)
		goto failed;

	if (child == 0) {
		if (input_fd != STDIN_FILENO) {
			if (dup2(input_fd, STDIN_FILENO) < 0)
				_exit(126);
			close(input_fd);
		}
#if defined (APP_SMBD36)
		if (passdb_path && *passdb_path)
			execl("/bin/smbpasswd", "smbpasswd", "-s", "-f",
			      passdb_path, username, (char *)NULL);
#else
		(void)passdb_path;
#endif
		execl("/bin/smbpasswd", "smbpasswd", "-s",
		      username, (char *)NULL);
		_exit(127);
	}

	close(input_fd);
	input_fd = -1;

	do {
		waited = waitpid(child, &status, 0);
	} while (waited < 0 && errno == EINTR);

	if (waited != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return -1;

	return 0;

failed:
	if (input_fd >= 0)
		close(input_fd);
	unlink(input_path);
	return -1;
}

#if defined (APP_SMBD36)
static int
samba_prepare_private_passdb(char *staged_path, size_t staged_path_size)
{
	int fd, close_result, path_len;

	path_len = snprintf(staged_path, staged_path_size,
			    "/etc/samba/.smbpasswd.XXXXXX");
	if (path_len < 0 || (size_t)path_len >= staged_path_size)
		return -1;

	fd = mkstemp(staged_path);
	if (fd < 0)
		return -1;
	if (fchmod(fd, 0600) != 0) {
		close(fd);
		unlink(staged_path);
		return -1;
	}
	close_result = close(fd);
	if (close_result != 0) {
		unlink(staged_path);
		return -1;
	}

	return 0;
}

#endif

int
check_existed_share(const char *string, const char *config_path)
{
	FILE *tp;
	char buf[4096], target[258];
	int target_len, found = 0, result = 0;

	if (!string || !*string || !config_path || !*config_path)
		return -1;

	target_len = snprintf(target, sizeof(target), "[%s]", string);
	if (target_len < 0 || target_len >= (int)sizeof(target))
		return -1;

	tp = fopen(config_path, "r");
	if (!tp)
		return -1;

	while (fgets(buf, sizeof(buf), tp) != NULL) {
		if (strncmp(buf, target, (size_t)target_len) == 0 &&
		    (buf[target_len] == '\n' || buf[target_len] == '\r' ||
		     buf[target_len] == '\0')) {
			found = 1;
			break;
		}
	}
	if (ferror(tp))
		result = -1;
	if (fclose(tp) != 0)
		result = -1;

	return result == 0 ? found : -1;
}

static int
samba_read_integer_status(const char *name, int missing_value, int *value)
{
	return storage_read_integer_status(name, missing_value, value);
}


static int
samba_read_boolean_status(const char *name, int missing_value, int *enabled)
{
	int value = 0;
	int read_status, status;

	if (!enabled)
		return EINVAL;
	*enabled = 0;
	read_status = samba_read_integer_status(name, missing_value, &value);
	status = samba_boolean_value_status(read_status, value, enabled);
	if (status != 0)
		errno = status;
	return status == 0 ? 0 : -1;
}


static int
samba_read_mode_status(int missing_value, int *mode)
{
	int status;

	if (!mode) {
		errno = EINVAL;
		return -1;
	}
	status = samba_read_integer_status("st_samba_mode", missing_value, mode);
	if (status != 0) {
		errno = status;
		return -1;
	}
	if (!samba_config_mode_is_valid(*mode)) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}


/* Configuration generation historically treats a missing mode as disabled. */
static int
samba_read_config_mode_status(int *mode)
{
	return samba_read_mode_status(0, mode);
}


/* Runtime startup historically treats missing global/mode flags as enabled. */
static int
samba_read_runtime_enabled_status(int *enabled)
{
	int enable_value = 1, mode_value = 1;
	int enable_read_status, mode_read_status = 0;
	int status;

	if (!enabled) {
		errno = EINVAL;
		return -1;
	}
	*enabled = 0;
	enable_read_status = samba_read_integer_status(
		"enable_samba", 1, &enable_value);
	if (enable_read_status == 0 && enable_value == 1)
		mode_read_status = samba_read_integer_status(
			"st_samba_mode", 1, &mode_value);

	status = samba_runtime_enabled_status(enable_read_status, enable_value,
					      mode_read_status, mode_value,
					      enabled);
	if (status != 0) {
		errno = status;
		return -1;
	}
	return 0;
}


static int
write_smb_conf_unlocked(const char *router_username,
                        char **snapshot_account_names,
                        int snapshot_account_count)
{
	FILE *fp = NULL;
	char staged_path[64];
	int i_maxuser = 0, i_maxuser_value = 0, i_maxuser_status;
	int i_smb_mode, result = 0, account_status, saved_errno = 0;
	int account_list_use_snapshot = 0;
	char local_router_username[128];
	const char *safe_router_username = router_username;
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;

	account_status = samba_passdb_account_list_source_status(
		snapshot_account_names != NULL, snapshot_account_count,
		MAX_ACCOUNT_NUM, &account_list_use_snapshot);
	if (account_status != 0) {
		errno = account_status;
		return -1;
	}

	if (samba_read_config_mode_status(&i_smb_mode) != 0) {
		int mode_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot read a valid Samba share mode; preserving active configuration (errno %d)",
			   mode_error);
		errno = mode_error;
		return -1;
	}

	/*
	 * Private per-user shares must exclude the router's administrative account.
	 * run_samba() supplies its preflight snapshot; standalone callers acquire
	 * one checked copy before any staged configuration file is opened.
	 */
	if (i_smb_mode != 0 && i_smb_mode != 1 && i_smb_mode != 3 &&
	    !safe_router_username) {
		account_status = storage_read_string_status(
			"http_username", local_router_username,
			sizeof(local_router_username));
		if (account_status != 0) {
			logmessage("Samba Server",
				   "cannot read router account identity; refusing private share publication (errno %d)",
				   account_status);
			errno = account_status;
			return -1;
		}
		safe_router_username = local_router_username;
	}

	if (samba_config_file_open(SAMBA_CONF, staged_path,
				   sizeof(staged_path), &fp) != 0)
		return -1;
	if (write_smb_conf_header(fp) != 0) {
		result = -1;
		goto confpage;
	}

	/* share mode */
	if (i_smb_mode == 1 || i_smb_mode == 3) {
#if defined (APP_SMBD36)
		fprintf(fp, "map to guest = %s\n", "Bad Password");
#else
		fprintf(fp, "security = %s\n", "SHARE");
#endif
		fprintf(fp, "guest ok = %s\n", "yes");
		fprintf(fp, "guest only = yes\n");
		fprintf(fp, "guest account = %s\n", SAMBA_GUEST_ACCOUNT);
	} else if (i_smb_mode == 4) {
#if !defined (APP_SMBD36)
		fprintf(fp, "security = %s\n", "USER");
#endif
		fprintf(fp, "guest ok = %s\n", "no");
		fprintf(fp, "map to guest = Bad User\n");
		fprintf(fp, "hide unreadable = yes\n");
	} else if (i_smb_mode == 0) {
		/* Disabled mode intentionally publishes a config with no shares. */
		goto confpage;
	} else {
		errno = EINVAL;
		result = -1;
		goto confpage;
	}

	fprintf(fp, "writeable = yes\n");
	fprintf(fp, "directory mode = 0777\n");
	fprintf(fp, "create mask = 0777\n");
	fprintf(fp, "force directory mode = 0777\n");

	/* max users */
	i_maxuser_value = 0;
	i_maxuser_status = storage_read_integer_status(
		"st_max_user", 0, &i_maxuser_value);
	i_maxuser_status = service_connection_limit_value_status(
		i_maxuser_status, i_maxuser_value, MAX_CLIENTS_NUM, &i_maxuser);
	if (i_maxuser_status != 0) {
		int read_error = i_maxuser_status > 0 ? i_maxuser_status : EIO;

		logmessage("Samba Server",
			   "cannot read max-connection limit; preserving active configuration (errno %d)",
			   read_error);
		errno = read_error;
		result = -1;
		goto confpage;
	}

	fprintf(fp, "max connections = %d\n", i_maxuser);
	fprintf(fp, "use spnego = no\n");		// ASUS add
	fprintf(fp, "client use spnego = no\n");	// ASUS add
	fprintf(fp, "null passwords = yes\n");		// ASUS add
	fprintf(fp, "strict allocate = no\n");		// ASUS add
	fprintf(fp, "use sendfile = yes\n");
	fprintf(fp, "dos filemode = yes\n");
	fprintf(fp, "dos filetimes = yes\n");
	fprintf(fp, "dos filetime resolution = yes\n");
	fprintf(fp, "access based share enum = yes\n");
	fprintf(fp, "veto files = /Thumbs.db/.DS_Store/._*/.apdisk/.TemporaryItems/");
	fprintf(fp, "delete veto files = yes\n");
	fprintf(fp, "\n");

	/*
	 * A complete empty inventory is valid when no storage is attached; only
	 * an incomplete/unknown scan must block replacing the active config.
	 */
	if (read_disk_data_checked(&disks_info) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Samba Server",
			   "cannot read complete storage inventory; preserving active configuration (errno %d)",
			   saved_errno);
		result = -1;
		goto confpage;
	}

	/* share */
	if (i_smb_mode == 1) {
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				char *mount_folder;
				char *last_slash;
				const char *disk_tag;

				if (follow_partition->mount_point == NULL)
					continue;

				last_slash = strrchr(follow_partition->mount_point, '/');
				if (last_slash == NULL || last_slash[1] == '\0')
					continue;
				mount_folder = last_slash + 1;
				disk_tag = follow_disk->tag ? follow_disk->tag : "";
				if (!samba_share_name_is_safe(mount_folder) ||
				    !samba_mount_path_is_safe(follow_partition->mount_point) ||
				    !samba_conf_text_is_safe(disk_tag)) {
					logmessage("Samba Server", "skip share with unsafe storage metadata");
					continue;
				}

				fprintf(fp, "[%s]\n", mount_folder);
				fprintf(fp, "comment = %s's %s\n", disk_tag, mount_folder);
				fprintf(fp, "path = %s\n", follow_partition->mount_point);
				fprintf(fp, "guest ok = yes\n");
				fprintf(fp, "writeable = yes\n");
				fprintf(fp, "directory mode = 0777\n");
				fprintf(fp, "create mask = 0777\n");
				fprintf(fp, "map archive = no\n");
				fprintf(fp, "map hidden = no\n");
				fprintf(fp, "map read only = no\n");
				fprintf(fp, "map system = no\n");
				fprintf(fp, "store dos attributes = yes\n\n");
			}
		}
	} else if (i_smb_mode == 3) {
		fprintf(fp, "[%s]\n", "Media");
		fprintf(fp, "comment = %s\n", "Root share for all media");
		fprintf(fp, "path = %s\n", POOL_MOUNT_ROOT);
		fprintf(fp, "guest ok = yes\n");
		fprintf(fp, "writeable = yes\n");
		fprintf(fp, "directory mode = 0777\n");
		fprintf(fp, "create mask = 0777\n");
		fprintf(fp, "map archive = no\n");
		fprintf(fp, "map hidden = no\n");
		fprintf(fp, "map read only = no\n");
		fprintf(fp, "map system = no\n");
		fprintf(fp, "store dos attributes = yes\n\n");
	} else {
		int n, acc_num = 0, sh_num=0, account_list_owned = 0;
		char **account_list = NULL;

		/*
		 * run_samba() supplies the same account generation used for passdb.
		 * Standalone writers fetch their own complete checked account list.
		 */
		if (account_list_use_snapshot) {
			account_list = snapshot_account_names;
			acc_num = snapshot_account_count;
		} else {
			account_list_owned = 1;
			if (get_account_list(&acc_num, &account_list) < 0) {
				if (account_list_owned)
					free_2_dimension_list(&acc_num, &account_list);
				result = -1;
				goto confpage;
			}
		}

		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list = NULL;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					free_2_dimension_list(&sh_num, &folder_list);
					if (account_list_owned)
						free_2_dimension_list(&acc_num, &account_list);
					result = -1;
					goto confpage;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first, share_exists;
					int guest_right;
					int account_rights[MAX_ACCOUNT_NUM];
					char share[256];

					if (!samba_mount_path_is_safe(follow_partition->mount_point) ||
					    !samba_share_name_is_safe(folder_list[n])) {
						logmessage("Samba Server", "skip share with unsafe storage metadata");
						continue;
					}
					guest_right = get_permission(SMB_GUEST_USER, follow_partition->mount_point, folder_list[n], "cifs");

					/*
					 * Read each account's ACL exactly once per share. Repeated
					 * lookups while emitting valid/invalid/read/write lists can
					 * observe different ACL generations and publish contradictory
					 * Samba access lists.
					 */
					for (i = 0; i < acc_num; ++i) {
						if (!samba_service_account_is_safe(account_list[i], safe_router_username)) {
							account_rights[i] = 0;
							continue;
						}
						account_rights[i] = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
					}
					snprintf(share, sizeof(share), "%s", folder_list[n]);
					
					if (fclose(fp) != 0) {
						fp = NULL;
						free_2_dimension_list(&sh_num, &folder_list);
						if (account_list_owned)
							free_2_dimension_list(&acc_num, &account_list);
						result = -1;
						goto confpage;
					}
					fp = NULL;

					share_exists = check_existed_share(share, staged_path);
					if (share_exists < 0) {
						free_2_dimension_list(&sh_num, &folder_list);
						if (account_list_owned)
							free_2_dimension_list(&acc_num, &account_list);
						result = -1;
						goto confpage;
					}
					if (share_exists) {
						i = 1;
						memset(share, 0, sizeof(share));
						snprintf(share, sizeof(share), "%.*s(%d)", (int)sizeof(share) - 13, folder_list[n], i);
						while ((share_exists = check_existed_share(share, staged_path)) > 0) {
							++i;
							snprintf(share, sizeof(share), "%.*s(%d)", (int)sizeof(share) - 13, folder_list[n], i);
						}
						if (share_exists < 0) {
							free_2_dimension_list(&sh_num, &folder_list);
							if (account_list_owned)
								free_2_dimension_list(&acc_num, &account_list);
							result = -1;
							goto confpage;
						}
					}

					fp = fopen(staged_path, "a");
					if (fp == NULL) {
						free_2_dimension_list(&sh_num, &folder_list);
						if (account_list_owned)
							free_2_dimension_list(&acc_num, &account_list);
						result = -1;
						goto confpage;
					}
					fprintf(fp, "[%s]\n", share);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					fprintf(fp, /*(guest_right == 2) ? "writeable = yes\n" : */"writeable = no\n");
					if (!samba_acl_right_is_denied(guest_right))
						fprintf(fp, "guest ok = yes\n");
					
					fprintf(fp, "valid users = ");
					first = 1;
					if (!samba_acl_right_is_denied(guest_right)) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						if (!samba_service_account_is_safe(account_list[i], safe_router_username))
							continue;
						right = account_rights[i];
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "invalid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						if (!samba_service_account_is_safe(account_list[i], safe_router_username))
							continue;
						right = account_rights[i];
						/* A write-only grant must not be negated by invalid users. */
						if (!samba_acl_right_is_denied(right))
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					/*
					 * A write-list member may also be in read list: Samba 3.6
					 * gives write-list membership precedence for that user.
					 */
					fprintf(fp, "read list = ");
					first = 1;
					if (samba_acl_right_can_read(guest_right)) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						if (!samba_service_account_is_safe(account_list[i], safe_router_username))
							continue;
						right = account_rights[i];
						if (!samba_acl_right_can_read(right))
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "write list = ");
					first = 1;
					if (samba_acl_right_can_write(guest_right)) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						if (!samba_service_account_is_safe(account_list[i], safe_router_username))
							continue;
						right = account_rights[i];
						if (!samba_acl_right_can_write(right))
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
		
		if (account_list_owned)
			free_2_dimension_list(&acc_num, &account_list);
	}

confpage:
	if (result != 0) {
		if (saved_errno == 0)
			saved_errno = errno ? errno : EIO;
		samba_config_file_discard(&fp, staged_path);
		free_disk_data(disks_info);
		errno = saved_errno;
		return -1;
	}
	if (samba_config_file_publish(&fp, staged_path, SAMBA_CONF) != 0) {
		saved_errno = errno ? errno : EIO;
		free_disk_data(disks_info);
		errno = saved_errno;
		return -1;
	}
	free_disk_data(disks_info);
	return 0;
}

int
write_smb_conf(void)
{
	int lock_fd, result, saved_errno;

	lock_fd = samba_config_transaction_lock_acquire(
		SAMBA_CONFIG_TRANSACTION_LOCK_FILE);
	if (lock_fd < 0) {
		logmessage("Samba Server", "cannot lock Samba configuration publication (errno %d)",
			   errno ? errno : EIO);
		return -1;
	}

	if (samba_config_transaction_prepare_parent_directory(
		SAMBA_CONFIG_BASE_DIRECTORY) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Samba Server",
			   "unsafe or unavailable private Samba state directory; refusing config recovery (errno %d)",
			   saved_errno);
		if (samba_config_transaction_lock_release(lock_fd) != 0)
			logmessage("Samba Server", "cannot release Samba config lock after directory validation failure");
		errno = saved_errno;
		return -1;
	}

	if (samba_config_transaction_recover(
		SAMBA_CONFIG_TRANSACTION_DIRECTORY,
		SAMBA_CONFIG_TRANSACTION_PASSDB,
		SAMBA_CONFIG_TRANSACTION_CONFIG) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Samba Server", "cannot recover interrupted Samba passdb/config publication (errno %d)",
			   saved_errno);
		samba_config_transaction_lock_release(lock_fd);
		errno = saved_errno;
		return -1;
	}

	result = write_smb_conf_unlocked(NULL, NULL, 0);
	if (samba_config_transaction_lock_release(lock_fd) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Samba Server", "cannot release Samba configuration lock (errno %d)",
			   saved_errno);
		if (result == 0)
			result = -1;
		errno = saved_errno;
	}
	return result;
}

static void
clean_smbd_trash(void)
{
	int i;
	const char *locks[] = {
		"account_policy.tdb",
		"connections.tdb",
		"group_mapping.tdb",
		"brlock.tdb",
		"locking.tdb",
		"sessionid.tdb",
		NULL
	};

	for (i=0; locks[i] && *locks[i]; i++)
		doSystem("rm -f /var/locks/%s", locks[i]);

#if defined (APP_SMBD36)
	doSystem("rm -f %s", "/var/log.*");
#else
	doSystem("rm -f %s", "/var/*.log");
#endif
	doSystem("rm -f %s", "/var/log/*");
}

void
config_smb_fastpath(int check_pid)
{
	in_addr_t lan_ip = 0;
	int write_status;

	if (nvram_match("st_samba_fp", "1")) {
		if (!check_pid) {
			lan_ip = get_lan_ip4();
		} else {
			struct process_snapshot snapshot;
			enum process_presence state;
			int scan_error;

			memset(&snapshot, 0, sizeof(snapshot));
			state = process_inventory_scan("smbd", &snapshot);
			scan_error = snapshot.error;
			process_snapshot_release(&snapshot);
			if (samba_fastpath_should_enable(1, state)) {
				lan_ip = get_lan_ip4();
			} else if (state == PROCESS_PRESENCE_UNKNOWN) {
				logmessage("Samba Server",
					   "cannot verify smbd for fastpath; disabling acceleration (errno %d)",
					   scan_error ? scan_error : EIO);
			} else if (state != PROCESS_PRESENCE_ABSENT) {
				logmessage("Samba Server",
					   "invalid smbd inventory state for fastpath; disabling acceleration");
			}
		}
		if (lan_ip == INADDR_ANY)
			lan_ip = 0;
	}

	write_status = fput_int("/proc/net/netfilter/nf_fp_smb", ntohl(lan_ip));
	if (write_status != 0)
		logmessage("Samba Server",
			   "failed to update SMB fastpath state (status=%d)",
			   write_status);
}

static int
samba_require_daemon_absent(const char *daemon, const char *stage)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int scan_error, status;

	memset(&snapshot, 0, sizeof(snapshot));
	state = process_inventory_scan(daemon, &snapshot);
	scan_error = snapshot.error;
	status = samba_process_require_absent(state, scan_error);
	process_snapshot_release(&snapshot);
	if (status != 0)
		logmessage("Samba Server",
			   "%s is not confirmed ABSENT %s (errno %d)",
			   daemon, stage ? stage : "", errno ? errno : EIO);
	return status;
}

static int
stop_samba_locked(int force_stop)
{
	char* svcs[] = { "smbd",
#if defined (APP_SMBD36)
	"wsdd2" ,
#endif
	 "nmbd", NULL };

	const int nmbdidx = sizeof(svcs) / sizeof(svcs[0]) - 2;
	int stop_status, stop_errno, fastpath_status, wins_enabled = 0;

	if (!force_stop) {
		if (samba_read_boolean_status("wins_enable", 0,
					      &wins_enabled) != 0) {
			int read_error = errno ? errno : EIO;

			logmessage("Samba Server",
				   "cannot read WINS enable state; refusing selective daemon stop (errno %d)",
				   read_error);
			errno = read_error;
			return -1;
		}
		if (wins_enabled)
			svcs[nmbdidx] = NULL;
	}

	stop_status = kill_services(svcs, 5, 1);
	stop_errno = errno;
	fastpath_status = fput_int("/proc/net/netfilter/nf_fp_smb", 0);

	if (fastpath_status != 0 && fastpath_status != ENOENT)
		logmessage("Samba Server",
			   "failed to clear SMB fastpath state during stop (status=%d)",
			   fastpath_status);

	if (stop_status != 0) {
		if (stop_errno != 0)
			errno = stop_errno;
		logmessage("Samba Server", "one or more requested Samba daemons did not stop");
		return stop_status;
	}

	/* Never declare a stop successful on a partial or UNKNOWN inventory. */
	if (samba_require_daemon_absent("smbd", "after stop") != 0)
		return -1;
#if defined (APP_SMBD36)
	if (samba_require_daemon_absent("wsdd2", "after stop") != 0)
		return -1;
#endif
	if (!svcs[nmbdidx])
		goto fastpath_result;
	if (samba_require_daemon_absent("nmbd", "after stop") != 0)
		return -1;

	clean_smbd_trash();

fastpath_result:
	stop_status = samba_fastpath_stop_result(stop_status, fastpath_status);
	if (fastpath_status != 0 && fastpath_status != ENOENT)
		errno = fastpath_status > 0 ? fastpath_status : EIO;
	return stop_status;
}

int
stop_samba(int force_stop)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Samba Server", "cannot acquire storage operation lock; refusing standalone Samba stop");
		return -1;
	}

	status = stop_samba_locked(force_stop);
	file_unlock(storage_lock);
	return status;
}

static enum process_presence
samba_daemon_is_running(void *context, const char *daemon)
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

static int
samba_daemon_sleep(void *context, unsigned int microseconds)
{
	(void)context;
	return service_wait_microseconds(microseconds);
}

static int
samba_wait_for_daemon(const char *daemon)
{
	static const struct samba_daemon_wait_ops ops = {
		samba_daemon_is_running,
		samba_daemon_sleep
	};

	return samba_daemon_wait_for_stable_presence(daemon, 30, &ops, NULL);
}

/* A successful command is not enough: require process presence across a settling interval. */
static enum process_presence
samba_validate_identity(void *context,const struct process_identity *identity)
{
	const char *daemon=(const char *)context;
	if(!daemon||!identity){errno=EINVAL;return PROCESS_PRESENCE_UNKNOWN;}
	return process_identity_validate(daemon,identity);
}
static int samba_send_signal(void *context,pid_t pid,int sig){(void)context;return kill(pid,sig);}
/* Remove only a newly launched daemon when its startup postcondition fails. */
static void
samba_rollback_failed_daemon_start(const char *daemon, int start_errno)
{
	char *services[2];
	int stop_status;
	int scan_error = 0;
	struct process_snapshot snapshot;
	enum process_presence presence;

	services[0] = (char *)daemon;
	services[1] = NULL;
	memset(&snapshot, 0, sizeof(snapshot));
	stop_status = kill_services(services, 5, 1);
	presence = process_inventory_scan(daemon, &snapshot);
	scan_error = snapshot.error;
	if (samba_failed_start_may_leave_daemon(presence)) {
		logmessage("Samba Server",
			   "%s may remain active after failed start (rollback_status=%d, state=%d, scan_error=%d)",
			   daemon, stop_status, presence, scan_error);
	} else if (stop_status != 0) {
		logmessage("Samba Server",
			   "%s is absent after failed-start rollback but stop reported status=%d",
			   daemon, stop_status);
	}
	process_snapshot_release(&snapshot);
	errno = start_errno > 0 ? start_errno : EIO;
}

static int
samba_ensure_daemon(const char *daemon,
		    const struct process_snapshot *initial_snapshot)
{
	const char *daemon_path;
	int command_status;
	int launched_new_daemon = 0;
	int start_errno;
	enum samba_daemon_action action;
	struct process_snapshot current_snapshot;
	enum process_presence current_presence;

	if (!daemon ||
	    (strcmp(daemon, "nmbd") && strcmp(daemon, "smbd") &&
	     strcmp(daemon, "wsdd2")) ||
	    !initial_snapshot) {
		errno = EINVAL;
		return -1;
	}

	action = samba_daemon_action_for_presence(initial_snapshot->state);
	if (action == SAMBA_DAEMON_ACTION_ABORT) {
		errno = EIO;
		logmessage("Samba Server",
			   "%s process state UNKNOWN; refusing start/reload", daemon);
		return -1;
	}

	if (action == SAMBA_DAEMON_ACTION_RELOAD) {
		command_status = samba_daemon_signal_snapshot(
			initial_snapshot, SIGHUP, samba_validate_identity,
			samba_send_signal, (void *)daemon);
	} else {
		memset(&current_snapshot, 0, sizeof(current_snapshot));
		current_presence = process_inventory_scan(daemon, &current_snapshot);
		if (current_presence == PROCESS_PRESENCE_UNKNOWN) {
			errno = current_snapshot.error ? current_snapshot.error : EIO;
			process_snapshot_release(&current_snapshot);
			logmessage("Samba Server",
				   "%s inventory UNKNOWN before launch", daemon);
			return -1;
		}
		if (current_presence != PROCESS_PRESENCE_ABSENT) {
			process_snapshot_release(&current_snapshot);
			errno = EBUSY;
			logmessage("Samba Server",
				   "%s appeared before launch; refusing duplicate", daemon);
			return -1;
		}
		process_snapshot_release(&current_snapshot);

		errno = 0;
		launched_new_daemon = 1;
		if (!strcmp(daemon, "nmbd")) {
			daemon_path = "/sbin/nmbd";
			command_status = eval(daemon_path, "-D", "-s", "/etc/smb.conf");
		} else if (!strcmp(daemon, "smbd")) {
			daemon_path = "/sbin/smbd";
			command_status = eval(daemon_path, "-D", "-s", "/etc/smb.conf");
		} else {
			daemon_path = "/sbin/wsdd2";
			command_status = eval(daemon_path, "-d", "-w");
		}
	}

	if (command_status != 0) {
		start_errno = errno ? errno : EIO;
		logmessage("Samba Server",
			   "%s %s command failed (status %d)", daemon,
			   action == SAMBA_DAEMON_ACTION_RELOAD ? "reload" : "start",
			   command_status);
		if (launched_new_daemon)
			samba_rollback_failed_daemon_start(daemon, start_errno);
		else
			errno = start_errno;
		return -1;
	}

	if (samba_wait_for_daemon(daemon) != 0) {
		start_errno = errno ? errno : ETIMEDOUT;
		logmessage("Samba Server", "%s not stable after %s (errno %d)",
			   daemon,
			   action == SAMBA_DAEMON_ACTION_RELOAD ? "reload" : "start",
			   start_errno);
		if (launched_new_daemon)
			samba_rollback_failed_daemon_start(daemon, start_errno);
		else
			errno = start_errno;
		return -1;
	}

	return 0;
}

int run_samba(void)
{
	int lock_fd, sh_num = 0, account_count_value = 0;
	int account_count_status, account_value_status, i, j;
	int account_snapshot_status = 0, account_snapshot_end_status;
	int passwd_status, passwd_error;
	int account_snapshot_end_errno;
	int credential_failed = 0, daemon_failed = 0, result = -1;
	int runtime_enabled;
	struct nvram_transaction account_snapshot;
	char tmpuser[40], tmp2[40];
	char router_username[128];
	char account_usernames[MAX_ACCOUNT_NUM][128];
	char account_passwords[MAX_ACCOUNT_NUM][128];
	char *account_list_snapshot[MAX_ACCOUNT_NUM];
	enum process_presence smbd_presence, nmbd_presence;
	struct process_snapshot smbd_snapshot, nmbd_snapshot;
#if defined (APP_SMBD36)
	struct process_snapshot wsdd2_snapshot;
	int saved_errno, transaction_active = 0;
	char staged_passdb[128];
#endif
	if (samba_read_runtime_enabled_status(&runtime_enabled) != 0) {
		int config_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot determine Samba enable state; preserving current daemon/configuration state (errno %d)",
			   config_error);
		errno = config_error;
		return -1;
	}
	if (!runtime_enabled)
		return 0;

	memset(&smbd_snapshot, 0, sizeof(smbd_snapshot));
	memset(&nmbd_snapshot, 0, sizeof(nmbd_snapshot));
#if defined (APP_SMBD36)
	memset(&wsdd2_snapshot, 0, sizeof(wsdd2_snapshot));
#endif

	lock_fd = samba_config_transaction_lock_acquire(
		SAMBA_CONFIG_TRANSACTION_LOCK_FILE);
	if (lock_fd < 0) {
		logmessage("Samba Server", "cannot lock Samba service publication (errno %d)",
			   errno ? errno : EIO);
		return -1;
	}

	memset(router_username, 0, sizeof(router_username));
	memset(account_usernames, 0, sizeof(account_usernames));
	memset(account_passwords, 0, sizeof(account_passwords));

	/*
	 * Read the count and every credential from one NVRAM generation.
	 * Individual checked getters use separate transactions and can otherwise
	 * interleave with a UI update, producing a mixed passdb generation.
	 */
	memset(&account_snapshot, 0, sizeof(account_snapshot));
	account_snapshot.lock_fd = -1;
	if (nvram_transaction_begin(&account_snapshot) != 0) {
		int snapshot_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot begin credential NVRAM snapshot; preserving current passdb and configuration (errno %d)",
			   snapshot_error);
		errno = snapshot_error;
		goto out;
	}

	account_count_status = storage_read_transaction_integer_status(
		&account_snapshot, "acc_num", 0, &account_count_value);
	account_snapshot_status = samba_passdb_account_count_status(
		account_count_status, account_count_value, MAX_ACCOUNT_NUM, &sh_num);
	if (account_snapshot_status != 0) {
		int account_error = account_snapshot_status > 0 ?
			account_snapshot_status : EIO;

		logmessage("Samba Server",
			   "cannot read a valid account count; preserving current passdb and configuration (errno %d)",
			   account_error);
		goto account_snapshot_done;
	}

	/*
	 * Copy cache-backed values before the next transaction read. Any read,
	 * validation or transaction-close failure prevents passdb publication.
	 */
	if (sh_num > 0) {
		account_value_status = storage_read_transaction_string_status(
			&account_snapshot, "http_username",
			router_username, sizeof(router_username));
		if (account_value_status != 0) {
			account_snapshot_status = account_value_status;
			logmessage("Samba Server",
				   "cannot read router account identity; preserving passdb and configuration (errno %d)",
				   account_value_status);
			goto account_snapshot_done;
		}
	}

	for (i = 0; i < sh_num; i++) {
		snprintf(tmpuser, sizeof(tmpuser), "acc_username%d", i);
		account_value_status = storage_read_transaction_string_status(
			&account_snapshot, tmpuser,
			account_usernames[i], sizeof(account_usernames[i]));
		if (account_value_status != 0) {
			account_snapshot_status = account_value_status;
			logmessage("Samba Server",
				   "cannot read SMB account identity %s; preserving passdb and configuration (errno %d)",
				   tmpuser, account_value_status);
			goto account_snapshot_done;
		}

		account_list_snapshot[i] = account_usernames[i];
		if (!samba_service_account_is_safe(
			    account_usernames[i], router_username))
			continue;

		/* Preserve the existing duplicate-identity rejection semantics. */
		for (j = 0; j < i; j++) {
			if (!strcmp(account_usernames[i], account_usernames[j])) {
				account_snapshot_status = EEXIST;
				logmessage("Samba Server",
					   "duplicate SMB account identity %s; preserving passdb and configuration",
					   account_usernames[i]);
				goto account_snapshot_done;
			}
		}

		snprintf(tmp2, sizeof(tmp2), "acc_password%d", i);
		account_value_status = storage_read_transaction_string_status(
			&account_snapshot, tmp2,
			account_passwords[i], sizeof(account_passwords[i]));
		if (account_value_status != 0) {
			account_snapshot_status = account_value_status;
			logmessage("Samba Server",
				   "cannot read SMB account password for %s; preserving passdb and configuration (errno %d)",
				   account_usernames[i], account_value_status);
			goto account_snapshot_done;
		}
		if (!samba_passdb_password_is_safe(account_passwords[i])) {
			account_snapshot_status = EINVAL;
			logmessage("Samba Server",
				   "unsafe SMB account password for %s; preserving passdb and configuration",
				   account_usernames[i]);
			goto account_snapshot_done;
		}
	}

account_snapshot_done:
	errno = 0;
	account_snapshot_end_status = nvram_transaction_end(&account_snapshot);
	account_snapshot_end_errno = errno;
	account_snapshot_status = samba_passdb_snapshot_end_status(
		account_snapshot_status, account_snapshot_end_status,
		account_snapshot_end_errno);
	if (account_snapshot_status != 0) {
		int snapshot_error = account_snapshot_status > 0 ?
			account_snapshot_status : EIO;

		logmessage("Samba Server",
			   "credential NVRAM snapshot is incomplete; preserving current passdb and configuration (errno %d)",
			   snapshot_error);
		errno = snapshot_error;
		goto out;
	}

	smbd_presence = process_inventory_scan("smbd", &smbd_snapshot);
	if (smbd_presence == PROCESS_PRESENCE_UNKNOWN) {
		errno = smbd_snapshot.error ? smbd_snapshot.error : EIO;
		logmessage("Samba Server", "smbd inventory UNKNOWN; refusing publication");
		goto out;
	}
	nmbd_presence = process_inventory_scan("nmbd", &nmbd_snapshot);
	if (nmbd_presence == PROCESS_PRESENCE_UNKNOWN) {
		errno = nmbd_snapshot.error ? nmbd_snapshot.error : EIO;
		logmessage("Samba Server", "nmbd inventory UNKNOWN; refusing publication");
		goto out;
	}
#if defined (APP_SMBD36)
	if (process_inventory_scan("wsdd2", &wsdd2_snapshot) == PROCESS_PRESENCE_UNKNOWN) {
		errno = wsdd2_snapshot.error ? wsdd2_snapshot.error : EIO;
		logmessage("WSDD2", "inventory UNKNOWN; refusing publication");
		goto out;
	}
#endif
	if (samba_config_transaction_prepare_parent_directory(
		SAMBA_CONFIG_BASE_DIRECTORY) != 0) {
		int directory_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "unsafe or unavailable private Samba state directory; service was not reloaded (errno %d)",
			   directory_error);
		errno = directory_error;
		goto out;
	}
	if (samba_config_transaction_recover(
		SAMBA_CONFIG_TRANSACTION_DIRECTORY,
		SAMBA_CONFIG_TRANSACTION_PASSDB,
		SAMBA_CONFIG_TRANSACTION_CONFIG) != 0) {
		logmessage("Samba Server", "interrupted Samba passdb/config transaction could not be recovered (errno %d)",
			   errno ? errno : EIO);
		goto out;
	}


	passwd_status = recreate_passwd_unix(0);
	if (passwd_status != 0) {
		passwd_error = errno ? errno :
			(passwd_status > 0 ? passwd_status : EIO);
		logmessage("Samba Server",
			   "Unix account database rebuild failed; preserving existing Samba passdb/configuration (status=%d, errno=%d)",
			   passwd_status, passwd_error);
		errno = passwd_error;
		goto out;
	}

	if (nmbd_presence == PROCESS_PRESENCE_ABSENT &&
	    smbd_presence == PROCESS_PRESENCE_ABSENT) {
		/* Preserve the account database; clear only volatile TDB/staging files. */
		doSystem("rm -f %s %s", "/etc/samba/*.tdb", "/etc/samba/.smbpasswd.*");
		clean_smbd_trash();
	}

#if defined (APP_SMBD36)
	if (samba_prepare_private_passdb(staged_passdb, sizeof(staged_passdb)) != 0) {
		logmessage("Samba Server", "unable to prepare private passdb; service was not reloaded");
		goto out;
	}
#endif

	for (i = 0; i < sh_num; i++) {
		if (!samba_service_account_is_safe(
			    account_usernames[i], router_username))
			continue;

		if (samba_run_smbpasswd(
#if defined (APP_SMBD36)
			staged_passdb,
#else
			NULL,
#endif
			account_usernames[i], account_passwords[i]) != 0) {
			credential_failed = 1;
			break;
		}
	}

	if (credential_failed) {
#if defined (APP_SMBD36)
		unlink(staged_passdb);
#endif
		logmessage("Samba Server", "credential provisioning failed; existing passdb and configuration were preserved");
		goto out;
	}

#if defined (APP_SMBD36)
	/*
	 * Keep rollback snapshots for both files until the new passdb and matching
	 * smb.conf are published. PREPARED is recoverable after an abrupt exit;
	 * COMMITTED makes the pair authoritative and leaves only cleanup work.
	 */
	if (samba_config_transaction_begin(
		SAMBA_CONFIG_TRANSACTION_DIRECTORY,
		SAMBA_CONFIG_TRANSACTION_PASSDB,
		SAMBA_CONFIG_TRANSACTION_CONFIG) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_passdb);
		logmessage("Samba Server", "unable to prepare paired passdb/config transaction (errno %d)",
			   saved_errno);
		errno = saved_errno;
		goto out;
	}
	transaction_active = 1;

	if (rename(staged_passdb, SAMBA_CONFIG_TRANSACTION_PASSDB) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_passdb);
		logmessage("Samba Server", "unable to publish staged passdb (errno %d)",
			   saved_errno);
		errno = saved_errno;
		goto out;
	}

	if (write_smb_conf_unlocked(router_username, account_list_snapshot, sh_num) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Samba Server", "configuration publication failed; paired rollback requested (errno %d)",
			   saved_errno);
		errno = saved_errno;
		goto out;
	}

	{
		int commit_status = samba_config_transaction_commit(
			SAMBA_CONFIG_TRANSACTION_DIRECTORY,
			SAMBA_CONFIG_TRANSACTION_PASSDB,
			SAMBA_CONFIG_TRANSACTION_CONFIG);
		if (commit_status < 0) {
			saved_errno = errno ? errno : EIO;
			logmessage("Samba Server", "unable to commit paired passdb/config publication (errno %d)",
				   saved_errno);
			errno = saved_errno;
			goto out;
		}
		transaction_active = 0;
		if (commit_status > 0)
			logmessage("Samba Server", "passdb/config pair committed; transaction snapshot cleanup will be retried");
	}
#else
	if (write_smb_conf_unlocked(router_username, account_list_snapshot, sh_num) != 0) {
		logmessage("Samba Server", "configuration generation failed; service was not reloaded");
		goto out;
	}
#endif

	/*
	 * Clear stale kernel state before daemon transitions. The process-aware
	 * helper enables acceleration only when smbd is already confirmed RUNNING.
	 */
	config_smb_fastpath(1);

	if (samba_ensure_daemon("nmbd", &nmbd_snapshot) != 0)
		daemon_failed = 1;
	if (samba_ensure_daemon("smbd", &smbd_snapshot) != 0)
		daemon_failed = 1;

#if defined (APP_SMBD36)
	if (samba_ensure_daemon("wsdd2", &wsdd2_snapshot) != 0)
		logmessage("WSDD2", "optional discovery daemon reload/start failed");
	else
		logmessage("WSDD2", "daemon is started");
#endif

	/* Recheck actual smbd presence after start/reload before enabling offload. */
	config_smb_fastpath(1);

	if (daemon_failed) {
		logmessage("Samba Server", "one or more required daemons failed reload/start postconditions");
		goto out;
	}
	logmessage("Samba Server", "daemon is started");
	result = 0;

out:
	/* Remove credential material from the stack on every locked exit path. */
	memset(account_passwords, 0, sizeof(account_passwords));
	memset(account_usernames, 0, sizeof(account_usernames));
	memset(router_username, 0, sizeof(router_username));
	process_snapshot_release(&smbd_snapshot);
	process_snapshot_release(&nmbd_snapshot);
#if defined (APP_SMBD36)
	process_snapshot_release(&wsdd2_snapshot);
	if (transaction_active) {
		saved_errno = errno ? errno : EIO;
		if (samba_config_transaction_abort(
			SAMBA_CONFIG_TRANSACTION_DIRECTORY,
			SAMBA_CONFIG_TRANSACTION_PASSDB,
			SAMBA_CONFIG_TRANSACTION_CONFIG) != 0)
			logmessage("Samba Server", "CRITICAL: paired publication rollback failed; recovery journal and snapshots remain at %s (errno %d)",
				   SAMBA_CONFIG_TRANSACTION_DIRECTORY, errno ? errno : EIO);
		else
			logmessage("Samba Server", "paired passdb/config publication rolled back");
		errno = saved_errno;
	}
#endif
	if (samba_config_transaction_lock_release(lock_fd) != 0) {
		logmessage("Samba Server", "cannot release Samba service publication lock (errno %d)",
			   errno ? errno : EIO);
		if (result == 0)
			result = -1;
	}
	return result;
}

int restart_smbd(void)
{
	int storage_lock, samba_status = 0;
	int stop_status, mount_status, restart_status = -1;
	int samba_enabled, wins_enabled;
	enum samba_storage_restart_action restart_action;

	/* Keep Samba's stop/inventory/start transaction inside the eject barrier. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Samba Server", "cannot acquire storage operation lock; refusing Samba restart");
		return -1;
	}

	if (samba_read_runtime_enabled_status(&samba_enabled) != 0) {
		int config_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot determine Samba enable state; refusing restart before daemon stop (errno %d)",
			   config_error);
		errno = config_error;
		goto out;
	}
	if (!samba_enabled &&
	    samba_read_boolean_status("wins_enable", 1, &wins_enabled) != 0) {
		int config_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot determine WINS enable state; refusing restart before daemon stop (errno %d)",
			   config_error);
		errno = config_error;
		goto out;
	}

	stop_status = stop_samba_locked(1);
	if (stop_status != 0) {
		logmessage("Samba Server", "restart aborted because existing daemons did not stop");
		restart_status = stop_status;
		goto out;
	}

	/* kill_services() success is not a substitute for complete procfs absence. */
	if (samba_require_daemon_absent("smbd", "after restart stop") != 0 ||
	    samba_require_daemon_absent("nmbd", "after restart stop") != 0)
		goto out;
#if defined (APP_SMBD36)
	if (samba_require_daemon_absent("wsdd2", "after restart stop") != 0)
		goto out;
#endif

	mount_status = storage_has_mounted_devices("restarting Samba");
	if (mount_status < 0) {
		logmessage("Samba Server", "restart aborted because mounted storage state is unknown");
		goto out;
	}
	restart_action = samba_storage_restart_action_for(samba_enabled,
						  mount_status);
	if (restart_action == SAMBA_STORAGE_RESTART_ACTION_ABORT) {
		logmessage("Samba Server", "restart aborted because Samba/storage state is invalid");
		errno = EIO;
		goto out;
	}
	if (restart_action == SAMBA_STORAGE_RESTART_ACTION_START_SAMBA) {
		samba_status = run_samba();
	} else if (restart_action == SAMBA_STORAGE_RESTART_ACTION_START_WINS) {
		/* WINS is independent of storage-backed Samba shares. */
		samba_status = start_wins();
	} else {
		errno = EIO;
		goto out;
	}
	if (samba_status != 0) {
		restart_status = samba_status;
		goto out;
	}

	restart_status = samba_status;
out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

#if defined (APP_NFSD)
static int
storage_nfs_export_transaction_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}


static int
storage_nfs_export_transaction_get(void *context, const char *param_name,
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
storage_nfs_export_transaction_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}


static int
write_nfsd_exports(void)
{
	FILE *procpt = NULL, *fp = NULL;
	struct stat exports_link_info, exports_target_info;
	char line[512], devname[32], mpname[256], fstype[32], fsmode[256];
	char acl_lan[32], acl_vpn[32], staged_path[] = "/etc/.exports.XXXXXX";
	char acl_lan_addr[32], acl_lan_mask[32];
	char acl_vpn_addr[32];
	const char *exports_link = "/etc/storage/exports";
	const char *exports_file = "/etc/exports";
	const char *exports_rule = STORAGE_NFS_EXPORT_DEFAULT_OPTIONS;
	char *nfsmm;
	int fd = -1, read_error, close_error, parse_error = 0;
	int close_status, output_error = 0, saved_errno = 0;
	int exports_link_lstat_status, exports_link_stat_status = -1;
	int exports_link_errno = 0, exports_target_errno = 0;
	int exports_override_state;
	int acl_ipv4_status, acl_vpn_status = 0, acl_nvram_status;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status, acl_ipv6_status = 0;
	char acl_addr6[INET6_ADDRSTRLEN + 5], acl_len6[8];
	char acl_addr6_export[INET6_ADDRSTRLEN + 4];
#endif

	/*
	 * Only a genuinely absent override permits generated exports. A dangling
	 * symlink, non-regular target or inaccessible path is an operator error;
	 * never silently replace that intent with broader generated exports.
	 */
	exports_link_lstat_status = lstat(exports_link, &exports_link_info);
	if (exports_link_lstat_status != 0) {
		exports_link_errno = errno ? errno : EIO;
	} else if (S_ISLNK(exports_link_info.st_mode)) {
		exports_link_stat_status = stat(exports_link, &exports_target_info);
		if (exports_link_stat_status != 0)
			exports_target_errno = errno ? errno : EIO;
	}

	exports_override_state = storage_nfs_export_override_state(
		exports_link_lstat_status, exports_link_errno,
		exports_link_lstat_status == 0 && S_ISREG(exports_link_info.st_mode),
		exports_link_lstat_status == 0 && S_ISLNK(exports_link_info.st_mode),
		exports_link_stat_status,
		exports_link_stat_status == 0 && S_ISREG(exports_target_info.st_mode));
	if (exports_override_state < 0) {
		if (exports_link_lstat_status != 0)
			saved_errno = exports_link_errno;
		else if (S_ISLNK(exports_link_info.st_mode) &&
			 exports_link_stat_status != 0)
			saved_errno = exports_target_errno;
		else
			saved_errno = EINVAL;
		logmessage("Storage", "refusing invalid custom NFS exports path %s (errno %d)",
			   exports_link, saved_errno);
		errno = saved_errno;
		return -1;
	}

	/*
	 * Preserve a valid operator-provided exports file as a symlink, but
	 * publish the replacement atomically so a failed symlink/rename leaves
	 * the old NFS configuration intact.
	 */
	if (exports_override_state == 1) {
		fd = mkstemp(staged_path);
		if (fd < 0)
			return -1;
		if (close(fd) != 0) {
			saved_errno = errno;
			unlink(staged_path);
			errno = saved_errno;
			return -1;
		}
		if (unlink(staged_path) != 0) {
			saved_errno = errno;
			unlink(staged_path);
			errno = saved_errno;
			return -1;
		}
		if (symlink(exports_link, staged_path) != 0) {
			saved_errno = errno;
			unlink(staged_path);
			errno = saved_errno;
			return -1;
		}
		if (rename(staged_path, exports_file) != 0) {
			saved_errno = errno;
			unlink(staged_path);
			errno = saved_errno;
			logmessage("Storage", "cannot publish custom NFS exports link (errno %d)", saved_errno);
			return -1;
		}
		return 0;
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		saved_errno = ipv6_type_status > 0 ? ipv6_type_status : EIO;
		logmessage("Storage",
			   "refusing generated NFS exports without verified IPv6 service state (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}
#endif

	/*
	 * Read every address, prefix and VPN selector under the same NVRAM lock.
	 * Separate getters could combine an old LAN address with a new netmask,
	 * widening the generated NFS client ACL across a configuration update.
	 */
	acl_nvram_status = storage_nfs_export_read_acl_snapshot(
		&acl_snapshot, storage_nfs_export_transaction_begin,
		storage_nfs_export_transaction_get,
		storage_nfs_export_transaction_end, &acl_transaction);
	if (acl_nvram_status != 0)
		goto acl_nvram_error;
	strcpy(acl_lan_addr, acl_snapshot.lan_address);
	strcpy(acl_lan_mask, acl_snapshot.lan_netmask);
	if (!is_valid_ipv4(acl_lan_addr) || !is_valid_ipv4(acl_lan_mask)) {
		strcpy(acl_lan_addr, acl_snapshot.fallback_address);
		strcpy(acl_lan_mask, acl_snapshot.fallback_netmask);
	}
	acl_nvram_status = storage_nfs_export_parse_selector(
		acl_snapshot.vpn_enabled, 1U, &vpn_enabled);
	if (acl_nvram_status != 0)
		goto acl_nvram_error;
	acl_nvram_status = storage_nfs_export_parse_selector(
		acl_snapshot.vpn_use, 2U, &vpn_use);
	if (acl_nvram_status != 0)
		goto acl_nvram_error;
	acl_lan[0] = '\0';
	acl_ipv4_status = storage_nfs_export_format_ipv4_acl(
		acl_lan_addr, acl_lan_mask, acl_lan, sizeof(acl_lan));
	if (acl_ipv4_status < 0) {
		saved_errno = EINVAL;
		logmessage("Storage", "refusing to publish NFS exports with an invalid IPv4 LAN ACL");
		errno = saved_errno;
		return -1;
	}

#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		strcpy(acl_addr6, acl_snapshot.ipv6_address);
		strcpy(acl_len6, acl_snapshot.ipv6_prefix);
		acl_ipv6_status = storage_nfs_export_format_ipv6_acl(
			acl_addr6, acl_len6, acl_addr6_export,
			sizeof(acl_addr6_export));
		if (acl_ipv6_status < 0) {
			saved_errno = EINVAL;
			logmessage("Storage", "refusing to publish NFS exports with an invalid IPv6 LAN ACL");
			errno = saved_errno;
			return -1;
		}
	}
#endif

	acl_vpn[0] = '\0';
	if (strcmp(acl_snapshot.router_mode, "3") != 0 &&
	    vpn_enabled && vpn_use) {
		strcpy(acl_vpn_addr, acl_snapshot.vpn_address);
#if defined (APP_OPENVPN)
		acl_nvram_status = storage_nfs_export_parse_selector(
			acl_snapshot.vpn_type, 2U, &vpn_type);
		if (acl_nvram_status != 0)
			goto acl_nvram_error;
		acl_nvram_status = storage_nfs_export_parse_selector(
			acl_snapshot.vpn_openvpn_mode, 1U, &vpn_openvpn_mode);
		if (acl_nvram_status != 0)
			goto acl_nvram_error;
		if (vpn_type == 2) {
			if (vpn_openvpn_mode == 1)
				acl_vpn_status = storage_nfs_export_format_ipv4_acl(
					acl_vpn_addr, VPN_SERVER_SUBNET_MASK,
					acl_vpn, sizeof(acl_vpn));
		} else
#endif
			acl_vpn_status = storage_nfs_export_format_ipv4_acl(
				acl_vpn_addr, VPN_SERVER_SUBNET_MASK,
				acl_vpn, sizeof(acl_vpn));
		if (acl_vpn_status < 0) {
			saved_errno = EINVAL;
			logmessage("Storage", "refusing to publish NFS exports with an invalid IPv4 VPN ACL");
			errno = saved_errno;
			return -1;
		}
		if (strcmp(acl_lan, acl_vpn) == 0)
			acl_vpn[0] = '\0';
	}

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (!procpt) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot open %s while generating NFS exports (errno %d)",
			   PROC_MOUNTS_FILE, saved_errno);
		errno = saved_errno;
		return -1;
	}

	fd = mkstemp(staged_path);
	if (fd < 0) {
		saved_errno = errno ? errno : EIO;
		fclose(procpt);
		errno = saved_errno;
		return -1;
	}
	if (fchmod(fd, 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		fclose(procpt);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}
	fp = fdopen(fd, "w");
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		fclose(procpt);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}
	fd = -1;

	fprintf(fp, "# %s\n\n", "auto-created file");

	while (fgets(line, sizeof(line), procpt)) {
		if (storage_nfs_export_parse_mount_line(line,
				devname, sizeof(devname), mpname, sizeof(mpname),
				fstype, sizeof(fstype), fsmode, sizeof(fsmode)) != 0) {
			parse_error = 1;
			break;
		}

		if (strcmp(fstype, "fuseblk") == 0)
			continue;

		if (!is_valid_storage_device(devname))
			continue;

		if (strncmp(mpname, "/media/", 7) == 0) {
			nfsmm = (strncmp(fsmode, "ro", 2) == 0) ? "ro" : "rw";
			fprintf(fp, "%s\t", mpname);
			fprintf(fp, " %s(%s,%s)", acl_lan, nfsmm, exports_rule);
#if defined (USE_IPV6)
			if ((ipv6_type != IPV6_DISABLED) && acl_ipv6_status > 0)
				fprintf(fp, " %s(%s,%s)", acl_addr6_export, nfsmm, exports_rule);
#endif
			if (acl_vpn[0])
				fprintf(fp, " %s(%s,%s)", acl_vpn, nfsmm, exports_rule);
			fprintf(fp, "\n");
		}
	}

	read_error = ferror(procpt);
	close_status = fclose(procpt);
	procpt = NULL;
	close_error = (close_status != 0);
	if (!storage_nfs_export_may_publish(1, read_error, close_error, parse_error)) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "refusing to publish incomplete %s while generating NFS exports (read error=%d, close error=%d, malformed/truncated row=%d)",
			   PROC_MOUNTS_FILE, read_error, close_error, parse_error);
		fclose(fp);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}

	if (ferror(fp) || fflush(fp) != 0)
		output_error = 1;
	if (fclose(fp) != 0)
		output_error = 1;
	fp = NULL;
	if (output_error) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot complete staged NFS exports (errno %d)", saved_errno);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}

	if (rename(staged_path, exports_file) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot publish generated NFS exports (errno %d)", saved_errno);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}

	return 0;

acl_nvram_error:
	saved_errno = storage_nfs_export_errno_or_eio(acl_nvram_status);
	logmessage("Storage",
		   "refusing to publish NFS exports because ACL NVRAM input cannot be read (status=%d)",
		   saved_errno);
	errno = saved_errno;
	return -1;
}

void unload_nfsd(void)
{
	module_smart_unload("nfsd", 0);
	module_smart_unload("exportfs", 0);
	module_smart_unload("lockd", 0);
	module_smart_unload("sunrpc", 0);
}

/*
 * The storage lock is also the safe-removal barrier. The *_locked helpers
 * are only for callers which already hold it (hotplug startup/removal and
 * restart_nfsd); public RC applets acquire it themselves. This prevents a
 * standalone run_nfsd/stop_nfsd command from restarting the export server
 * while another process is unmounting its backing device.
 */
static int
stop_nfsd_locked(void)
{
	return eval("/usr/bin/nfsd.sh", "stop");
}

int stop_nfsd(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Storage", "cannot acquire storage operation lock; refusing NFS stop");
		return -1;
	}

	status = stop_nfsd_locked();
	file_unlock(storage_lock);
	return status;
}

static int
nfsd_enabled_config_read(int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!enabled)
		return EINVAL;
	errno = 0;
	api_result = nvram_get_checked("nfsd_enable", &value);
	saved_errno = errno;
	status = storage_nfs_enabled_status(api_result, saved_errno,
					    value, enabled);
	if (status != 0) {
		logmessage("Storage",
			   "cannot read NFS enable configuration; preserving current daemon state (errno %d)",
			   status);
		errno = status;
	}
	return status;
}

static int
run_nfsd_enabled_locked(int enabled)
{
	int export_status;
	int command_status = -1;

	if (!enabled)
		return 0;

	export_status = write_nfsd_exports();
	if (!storage_nfs_export_may_start(1, export_status)) {
		logmessage("Storage", "NFS start skipped because exports could not be generated safely");
		return storage_nfs_start_status(export_status, command_status);
	}

	command_status = eval("/usr/bin/nfsd.sh", "start");
	if (command_status != 0)
		logmessage("Storage", "NFS daemon start failed (status %d)", command_status);

	return storage_nfs_start_status(export_status, command_status);
}

static int
run_nfsd_locked(void)
{
	int enabled, config_status;

	config_status = nfsd_enabled_config_read(&enabled);
	if (config_status != 0)
		return -1;
	return run_nfsd_enabled_locked(enabled);
}

int
run_nfsd(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Storage", "cannot acquire storage operation lock; refusing NFS start");
		return -1;
	}

	status = run_nfsd_locked();
	file_unlock(storage_lock);
	return status;
}

static int
reload_nfsd_enabled_locked(int enabled)
{
	int export_status;
	int command_status;

	if (!enabled)
		return 0;

	export_status = write_nfsd_exports();
	if (!storage_nfs_export_may_start(1, export_status)) {
		logmessage("Storage", "NFS reload skipped because exports could not be generated safely");
		return -1;
	}

	command_status = eval("/usr/bin/nfsd.sh", "reload");
	if (command_status != 0)
		logmessage("Storage", "NFS daemon reload failed (status %d)", command_status);

	return command_status == 0 ? 0 : -1;
}

static int
reload_nfsd_locked(void)
{
	int enabled, config_status;

	config_status = nfsd_enabled_config_read(&enabled);
	if (config_status != 0)
		return -1;
	return reload_nfsd_enabled_locked(enabled);
}

int
reload_nfsd(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Storage", "cannot acquire storage operation lock; refusing NFS reload");
		return -1;
	}

	status = reload_nfsd_locked();
	file_unlock(storage_lock);
	return status;
}

int restart_nfsd(void)
{
	int storage_lock, mount_count, status;
	int nfsd_enabled, config_status;

	/* Serialize inventory, stop and restart with safe storage removal. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Storage", "cannot acquire storage operation lock; refusing NFS restart");
		return -1;
	}

	config_status = nfsd_enabled_config_read(&nfsd_enabled);
	if (config_status != 0) {
		logmessage("Storage",
			   "NFS restart aborted before stop because enable configuration is unreadable");
		file_unlock(storage_lock);
		return -1;
	}

	mount_count = count_stor_mountpoint();
	if (mount_count < 0) {
		logmessage("Storage", "cannot determine mounted storage from %s during NFS restart; keeping NFS support running",
			   PROC_MOUNTS_FILE);
		file_unlock(storage_lock);
		return -1;
	}

	if (stop_nfsd_locked() != 0) {
		logmessage("Storage", "NFS server stop failed; refusing restart or module unload");
		file_unlock(storage_lock);
		return -1;
	}

	if (nfsd_enabled && storage_mount_inventory_has_mounts(mount_count)) {
		if (service_wait_seconds(1) != 0) {
			status = errno > 0 ? errno : EIO;
			logmessage("Storage", "NFS restart settling wait failed (status %d); refusing restart",
				   status);
			/* PID 1 only treats negative action_result as failure. */
			status = service_wait_failure_status(status);
		} else {
			status = run_nfsd_enabled_locked(nfsd_enabled);
		}
	} else {
		unload_nfsd();
		status = 0;
	}

	file_unlock(storage_lock);
	return status;
}
#endif

int create_mp_link(char *search_dir, char *link_path, int force_first_valid)
{
	FILE *procpt;
	char line[512], devname[32], mpname[256], fstype[32];
	char mount_options[256], target_path[512];
	int link_created, path_len;
	int read_error, close_error, parse_error;
	int first_error, operation_error, close_status;

	if (!search_dir || !search_dir[0] || !link_path || !link_path[0]) {
		errno = EINVAL;
		return 0;
	}

	link_created = 0;
	read_error = 0;
	close_error = 0;
	parse_error = 0;
	first_error = 0;

	errno = 0;
	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (!procpt) {
		errno = storage_nfs_export_errno_or_eio(errno);
		return 0;
	}

	while (1) {
		errno = 0;
		if (!fgets(line, sizeof(line), procpt))
			break;

		if (storage_nfs_export_parse_mount_line(
			    line, devname, sizeof(devname), mpname, sizeof(mpname),
			    fstype, sizeof(fstype), mount_options,
			    sizeof(mount_options)) != 0) {
			parse_error = 1;
			first_error = storage_nfs_export_first_error(first_error, EINVAL);
			break;
		}

		/* Continue validating the complete inventory after choosing a target. */
		if (link_created)
			continue;

#if 0
		if (only_ext_xfs) {
			if (strcmp(fstype, "xfs") && strcmp(fstype, "exfat") && strncmp(fstype, "ext", 3))
				continue;
		}
#endif
		if (!is_valid_storage_device(devname))
			continue;

		if (strncmp(mpname, "/media/", 7) == 0) {
			path_len = snprintf(target_path, sizeof(target_path), "%s/%s", mpname, search_dir);
			if (path_len < 0) {
				first_error = storage_nfs_export_first_error(
					first_error, storage_nfs_export_errno_or_eio(errno));
				continue;
			}
			if (path_len >= (int)sizeof(target_path)) {
				first_error = storage_nfs_export_first_error(first_error, ENAMETOOLONG);
				continue;
			}

			if (!force_first_valid) {
				errno = 0;
				if (check_if_dir_exist(target_path)) {
					errno = 0;
					if (symlink(target_path, link_path) == 0)
						link_created = 1;
					else
						first_error = storage_nfs_export_first_error(
							first_error,
							storage_nfs_export_errno_or_eio(errno));
				} else {
					operation_error = errno;
					if (operation_error == 0)
						operation_error = ENOTDIR;
					first_error = storage_nfs_export_first_error(
							first_error, operation_error);
				}
			} else {
				errno = 0;
				if (mkdir_if_none(target_path, "777")) {
					errno = 0;
					if (symlink(target_path, link_path) == 0)
						link_created = 1;
					else
						first_error = storage_nfs_export_first_error(
							first_error,
							storage_nfs_export_errno_or_eio(errno));
				} else {
					operation_error = errno ? errno : EEXIST;
					first_error = storage_nfs_export_first_error(
							first_error, operation_error);
				}
			}
		}
	}

	read_error = ferror(procpt) ? 1 : 0;
	operation_error = read_error ? errno : 0;
	errno = 0;
	close_status = fclose(procpt);
	close_error = close_status != 0;
	if (read_error)
		first_error = storage_nfs_export_first_error(
			first_error, storage_nfs_export_errno_or_eio(operation_error));
	if (close_error)
		first_error = storage_nfs_export_first_error(
			first_error, storage_nfs_export_errno_or_eio(errno));

	if (!storage_nfs_export_inventory_complete(read_error, close_error,
						   parse_error)) {
		if (link_created) {
			errno = 0;
			if (unlink(link_path) != 0)
				first_error = storage_nfs_export_first_error(
					first_error,
					storage_nfs_export_errno_or_eio(errno));
		}
		logmessage("Storage", "incomplete /proc/mounts inventory; refusing storage link");
		errno = storage_nfs_export_errno_or_eio(first_error);
		return 0;
	}

	if (link_created) {
		errno = 0;
		return 1;
	}

	errno = storage_nfs_export_errno_or_eio(first_error ? first_error : ENOENT);
	return 0;
}

static int
create_mp_link_status(char *search_dir, char *link_path, int force_first_valid)
{
	int status;

	errno = 0;
	if (create_mp_link(search_dir, link_path, force_first_valid)) {
		errno = 0;
		return 0;
	}

	status = storage_nfs_export_errno_or_eio(errno);
	errno = status;
	return status;
}

#if defined (APP_MINIDLNA)
int is_dms_support(void)
{
	return check_if_file_exist("/usr/bin/minidlnad");
}

int is_dms_run(void)
{
	if (!is_dms_support())
		return 0;

	return storage_daemon_presence("minidlnad", NULL);
}

static int
stop_dms_locked(void)
{
	char* svcs[] = { "minidlnad", NULL };
	int command_status;

	if (!is_dms_support())
		return 0;

	command_status = kill_services(svcs, 5, 1);
	if (storage_service_stop_complete(command_status, is_dms_run()) != 0) {
		logmessage("MiniDLNA", "daemon did not stop cleanly");
		return -1;
	}

	return 0;
}

int
stop_dms(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("MiniDLNA", "cannot acquire storage operation lock; refusing standalone stop");
		return -1;
	}

	status = stop_dms_locked();
	file_unlock(storage_lock);
	return storage_service_stop_complete(status, is_dms_run());
}

struct storage_minidlna_config_snapshot {
	int notify_interval;
	int root_container;
	int force_sort;
	char media_source1[NVRAM_MAX_VALUE_LEN + 1];
	char media_source2[NVRAM_MAX_VALUE_LEN + 1];
	char media_source3[NVRAM_MAX_VALUE_LEN + 1];
};


static int
storage_read_minidlna_config_status(
	struct storage_minidlna_config_snapshot *snapshot)
{
	struct nvram_transaction transaction;
	int status, end_status, end_errno;

	if (!snapshot) {
		errno = EINVAL;
		return EINVAL;
	}
	memset(snapshot, 0, sizeof(*snapshot));
	if (nvram_transaction_begin(&transaction) != 0) {
		status = errno ? errno : EIO;
		errno = status;
		return status;
	}

	status = storage_read_transaction_integer_status(
		&transaction, "dlna_disc", 0, &snapshot->notify_interval);
	if (status == 0)
		status = storage_read_transaction_integer_status(
			&transaction, "dlna_root", 0, &snapshot->root_container);
	if (status == 0)
		status = rc_wan_nvram_copy_value(
			storage_read_transaction_nvram_string, &transaction,
			"dlna_src1", snapshot->media_source1,
			sizeof(snapshot->media_source1));
	if (status == 0)
		status = rc_wan_nvram_copy_value(
			storage_read_transaction_nvram_string, &transaction,
			"dlna_src2", snapshot->media_source2,
			sizeof(snapshot->media_source2));
	if (status == 0)
		status = rc_wan_nvram_copy_value(
			storage_read_transaction_nvram_string, &transaction,
			"dlna_src3", snapshot->media_source3,
			sizeof(snapshot->media_source3));
	if (status == 0)
		status = storage_read_transaction_integer_status(
			&transaction, "dlna_sort", 0, &snapshot->force_sort);

	errno = 0;
	end_status = nvram_transaction_end(&transaction);
	end_errno = errno;
	if (status == 0 && end_status != 0)
		status = end_errno ? end_errno : EIO;
	if (status != 0) {
		memset(snapshot, 0, sizeof(*snapshot));
		errno = status;
		return status;
	}
	return 0;
}


int update_minidlna_conf(const char *link_path, const char *conf_path)
{
	struct storage_minidlna_config_snapshot snapshot;
	struct minidlna_config_values values;
	const char *computer_name;
	int dlna_disc, status;

	status = storage_read_minidlna_config_status(&snapshot);
	if (status != 0) {
		logmessage("MiniDLNA",
			   "cannot read complete configuration snapshot; preserving active configuration (status=%d)",
			   status);
		return status;
	}
	computer_name = get_our_hostname();
	if (computer_name == NULL)
		return EIO;

	values.media_source1 = snapshot.media_source1;
	values.media_source2 = snapshot.media_source2;
	values.media_source3 = snapshot.media_source3;
	if (!*values.media_source1 && !*values.media_source2 &&
	    !*values.media_source3)
		values.media_source1 = "/media";

	dlna_disc = snapshot.notify_interval;
	if (dlna_disc < 10)
		dlna_disc = 10;
	if (dlna_disc > 10800)
		dlna_disc = 10800;

	values.interface_name = IFNAME_BR;
	values.notify_interval = dlna_disc;
	values.root_container = snapshot.root_container;
	values.force_sort = snapshot.force_sort;
	values.friendly_name = computer_name;
	values.database_dir = link_path;

	status = minidlna_config_publish(conf_path, &values);
	if (status != 0)
		logmessage("MiniDLNA", "configuration publication failed (status=%d)", status);
	return status;
}

/*
 * Every caller below runs under storage.lock (integrated hotplug startup or
 * a restart wrapper). A failed fresh launch must not leave an untracked
 * daemon behind; the stop callback is service-specific and re-inventory is
 * authoritative. Existing daemons are rejected before the launch point.
 */
static int stop_itunes_locked(void);
static int stop_torrent_locked(void);
static int stop_aria_locked(void);

static int
storage_rollback_failed_start(const char *apps_name, const char *daemon,
			      int start_status, int start_errno,
			      int (*stop_daemon)(void))
{
	int stop_status;
	int scan_error;
	struct process_snapshot snapshot;
	enum process_presence presence;

	memset(&snapshot, 0, sizeof(snapshot));
	stop_status = stop_daemon();
	presence = process_inventory_scan(daemon, &snapshot);
	scan_error = snapshot.error;
	if (storage_service_failed_start_may_leave_daemon(presence)) {
		logmessage(apps_name,
			   "%s may remain active after failed start (rollback_status=%d, state=%d, scan_error=%d)",
			   daemon, stop_status, presence, scan_error);
	} else if (stop_status != 0) {
		logmessage(apps_name,
			   "%s is absent after failed-start rollback but stop reported status=%d",
			   daemon, stop_status);
	}
	process_snapshot_release(&snapshot);
	errno = start_errno > 0 ? start_errno : EIO;
	return start_status;
}

int run_dms(int force_rescan)
{
	int db_rescan_mode = 0;
	int enable_status;
	int mac_status;
	int process_state;
	int process_error;
	int config_status;
	int launch_status;
	int launch_errno;
	int start_status;
	int start_errno;
	int link_status;
	unsigned char mac_bin[ETHER_ADDR_LEN] = {0};
	char mac_str[16];
	char apps_enabled[8] = {0};
	char mac_address[32] = {0};
	char *apps_name = "Media Server";
	char *link_path = "/mnt/minidlna";
	char *conf_path = "/etc/minidlna.conf";
	char *dest_dir = ".dms";
	char *minidlna_argv[] = {
		"/usr/bin/minidlnad",
		"-f", conf_path,
		"-s", NULL,
		NULL,	/* -U */
		NULL
	};

	enable_status = storage_read_string_status(
		"apps_dms", apps_enabled, sizeof(apps_enabled));
	if (enable_status != 0) {
		logmessage("minidlna", "cannot read service enable flag; refusing startup (status=%d)",
			   enable_status);
		return enable_status;
	}
	if (strcmp(apps_enabled, "1") != 0)
		return 0;

	if (!is_dms_support())
		return 0;

	process_state = is_dms_run();
	if (process_state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		logmessage("minidlna", "startup skipped because process inventory is unknown");
		return errno ? errno : EIO;
	}
	if (!force_rescan) {
		config_status = storage_read_integer_status(
			"dlna_rescan", 0, &db_rescan_mode);
		if (config_status != 0) {
			logmessage(apps_name,
				   "cannot read database rescan mode; aborting daemon start (status=%d)",
				   config_status);
			return config_status;
		}
	}
	mac_status = storage_read_string_status(
		"il0macaddr", mac_address, sizeof(mac_address));
	if (mac_status != 0) {
		logmessage(apps_name,
			   "cannot read MiniDLNA server identity; aborting daemon start (status=%d)",
			   mac_status);
		return mac_status;
	}
	/* Reject malformed identity text before changing the database link or config. */
	if (!ether_atoe(mac_address, mac_bin)) {
		logmessage(apps_name, "invalid MiniDLNA server identity; aborting daemon start");
		errno = EINVAL;
		return EINVAL;
	}
	minidlna_argv[4] = ether_etoa3(mac_bin, mac_str);

	unlink(link_path);
	link_status = create_mp_link_status(dest_dir, link_path, 0);
	if (link_status != 0) {
		link_status = create_mp_link_status(dest_dir, link_path, 1);
		if (link_status != 0) {
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%s) on any volumes (status=%d)!",
				   dest_dir, link_status);
			return link_status;
		}
	}

	config_status = update_minidlna_conf(link_path, conf_path);
	if (config_status != 0) {
		logmessage(apps_name, "refusing daemon launch after configuration failure");
		return config_status;
	}

	if (force_rescan || db_rescan_mode == 2)
		minidlna_argv[5] = "-R";
	else if (db_rescan_mode == 1)
		minidlna_argv[5] = "-U";

	errno = 0;
	launch_status = _eval(minidlna_argv, NULL, 0, NULL);
	launch_errno = errno;
	process_error = 0;
	if (launch_status == 0) {
		if (storage_wait_for_daemon("minidlnad") == 0)
			process_state = PROCESS_PRESENCE_RUNNING;
		else {
			process_error = errno ? errno : EIO;
			process_state = process_error == ETIMEDOUT ?
				PROCESS_PRESENCE_ABSENT : PROCESS_PRESENCE_UNKNOWN;
		}
	} else {
		errno = 0;
		process_state = is_dms_run();
		if (process_state == PROCESS_PRESENCE_UNKNOWN)
			process_error = errno ? errno : EIO;
	}
	start_status = storage_log_daemon_start_result(
		apps_name, launch_status, (enum process_presence)process_state,
		process_error);
	if (start_status != 0) {
		start_errno = launch_status != 0 ?
			(launch_errno ? launch_errno : EIO) :
			(process_error ? process_error : (errno ? errno : start_status));
		return storage_rollback_failed_start(
			apps_name, "minidlnad", start_status, start_errno,
			stop_dms_locked);
	}
	return 0;
}

int restart_dms(int force_rescan)
{
	int storage_lock, stop_status, mount_status;
	int restart_status = 0;

	/* Serialize media-server restart with hotplug and safe storage removal. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("MiniDLNA", "cannot acquire storage operation lock; refusing restart");
		return errno ? errno : EIO;
	}

	stop_status = stop_dms_locked();
	if (stop_status != 0) {
		logmessage("MiniDLNA", "restart aborted because the existing daemon did not stop");
		restart_status = stop_status;
		goto out;
	}

	mount_status = storage_has_mounted_devices("restarting DLNA");
	if (mount_status < 0) {
		logmessage("MiniDLNA", "restart aborted because mounted storage state is unknown");
		restart_status = errno ? errno : EIO;
		goto out;
	}
	if (mount_status > 0)
		restart_status = run_dms(force_rescan);

out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

#if defined (APP_FIREFLY)
int is_itunes_support(void)
{
	return check_if_file_exist("/usr/bin/mt-daapd");
}

int is_itunes_run(void)
{
	if (!is_itunes_support())
		return 0;

	return storage_daemon_presence("mt-daapd", NULL);
}

static int
stop_itunes_locked(void)
{
	char* svcs[] = { "mt-daapd", NULL };
	int command_status;

	if (!is_itunes_support())
		return 0;

	command_status = kill_services(svcs, 5, 1);
	if (storage_service_stop_complete(command_status, is_itunes_run()) != 0) {
		logmessage("Firefly", "daemon did not stop cleanly");
		return -1;
	}

	return 0;
}

int
stop_itunes(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Firefly", "cannot acquire storage operation lock; refusing standalone stop");
		return -1;
	}

	status = stop_itunes_locked();
	file_unlock(storage_lock);
	return storage_service_stop_complete(status, is_itunes_run());
}

static int
storage_read_firefly_credentials_status(char *username, size_t username_size,
					char *password, size_t password_size)
{
	struct nvram_transaction transaction;
	int status, end_status, end_errno, saved_errno;

	if (!username || username_size == 0 || !password || password_size == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	username[0] = '\0';
	password[0] = '\0';

	if (nvram_transaction_begin(&transaction) != 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		return saved_errno;
	}

	status = rc_wan_nvram_copy_value(storage_read_transaction_nvram_string,
					 &transaction, "http_username",
					 username, username_size);
	if (status == 0)
		status = rc_wan_nvram_copy_value(storage_read_transaction_nvram_string,
						 &transaction, "http_passwd",
						 password, password_size);
	saved_errno = status;
	errno = 0;
	end_status = nvram_transaction_end(&transaction);
	end_errno = errno;
	if (saved_errno == 0 && end_status != 0)
		saved_errno = end_errno ? end_errno : EIO;
	if (saved_errno != 0) {
		username[0] = '\0';
		password[0] = '\0';
		errno = saved_errno;
		return saved_errno;
	}

	return 0;
}


static int
update_firefly_conf(const char *link_path, const char *conf_path,
		    const char *conf_file)
{
	char conf_full[128];
	char username[256], password[256];
	int path_length, credential_status;

	path_length = snprintf(conf_full, sizeof(conf_full), "%s/%s",
			       conf_path, conf_file);
	if (path_length < 0 || (size_t)path_length >= sizeof(conf_full))
		return ENAMETOOLONG;

	credential_status = storage_read_firefly_credentials_status(
		username, sizeof(username), password, sizeof(password));
	if (credential_status != 0) {
		logmessage("Firefly",
			   "cannot read web credentials; refusing configuration publication (status=%d)",
			   credential_status);
		return credential_status;
	}

	return firefly_config_publish(conf_full, conf_full, link_path,
				      username, password);
}

int run_itunes(void)
{
	char *apps_name = "iTunes Server";
	char *link_path = "/mnt/firefly";
	char *conf_path = "/etc/storage/firefly";
	char *conf_file = "mt-daapd.conf";
	char *dest_dir = ".itunes";
	char conf_new[64], conf_old[64];
	int process_state;
	int process_error;
	int launch_status;
	int launch_errno;
	int start_status;
	int start_errno;
	int config_status;
	int path_length;
	int link_status;
	int service_enabled, enable_status;

	enable_status = storage_service_enabled_status(
		"apps_itunes", "iTunes Server", &service_enabled);
	if (enable_status != 0)
		return enable_status;
	if (!service_enabled)
		return 0;

	if (!is_itunes_support())
		return 0;

	process_state = is_itunes_run();
	if (process_state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		logmessage("mt-daapd", "startup skipped because process inventory is unknown");
		return errno ? errno : EIO;
	}

	unlink(link_path);
	link_status = create_mp_link_status(dest_dir, link_path, 0);
	if (link_status != 0) {
		link_status = create_mp_link_status(dest_dir, link_path, 1);
		if (link_status != 0) {
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%s) on any volumes (status=%d)!",
				   dest_dir, link_status);
			return link_status;
		}
	}

	if (mkdir(conf_path, 0755) != 0 && errno != EEXIST) {
		config_status = errno ? errno : EIO;
		logmessage(apps_name, "configuration directory creation failed (status=%d)", config_status);
		return config_status;
	}

	path_length = snprintf(conf_old, sizeof(conf_old), "%s/%s", "/etc/storage", conf_file);
	if (path_length < 0 || (size_t)path_length >= sizeof(conf_old)) {
		logmessage(apps_name, "legacy configuration path is too long");
		return ENAMETOOLONG;
	}
	path_length = snprintf(conf_new, sizeof(conf_new), "%s/%s", conf_path, conf_file);
	if (path_length < 0 || (size_t)path_length >= sizeof(conf_new)) {
		logmessage(apps_name, "configuration path is too long");
		return ENAMETOOLONG;
	}
	if (!check_if_file_exist(conf_new) && check_if_file_exist(conf_old) &&
	    rename(conf_old, conf_new) != 0) {
		config_status = errno ? errno : EIO;
		logmessage(apps_name, "legacy configuration migration failed (status=%d)", config_status);
		return config_status;
	}

	config_status = update_firefly_conf(link_path, conf_path, conf_file);
	if (config_status != 0) {
		logmessage(apps_name, "configuration publication failed (status=%d)", config_status);
		return config_status;
	}

	errno = 0;
	launch_status = eval("/usr/bin/mt-daapd", "-c", conf_new);
	launch_errno = errno;
	process_error = 0;
	if (launch_status == 0) {
		if (storage_wait_for_daemon("mt-daapd") == 0)
			process_state = PROCESS_PRESENCE_RUNNING;
		else {
			process_error = errno ? errno : EIO;
			process_state = process_error == ETIMEDOUT ?
				PROCESS_PRESENCE_ABSENT : PROCESS_PRESENCE_UNKNOWN;
		}
	} else {
		errno = 0;
		process_state = is_itunes_run();
		if (process_state == PROCESS_PRESENCE_UNKNOWN)
			process_error = errno ? errno : EIO;
	}
	start_status = storage_log_daemon_start_result(
		apps_name, launch_status, (enum process_presence)process_state,
		process_error);
	if (start_status != 0) {
		start_errno = launch_status != 0 ?
			(launch_errno ? launch_errno : EIO) :
			(process_error ? process_error : (errno ? errno : start_status));
		return storage_rollback_failed_start(
			apps_name, "mt-daapd", start_status, start_errno,
			stop_itunes_locked);
	}
	return 0;
}

int restart_itunes(void)
{
	int storage_lock, stop_status, mount_status;
	int restart_status = 0;

	/* Serialize Firefly's storage-backed database with safe storage removal. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Firefly", "cannot acquire storage operation lock; refusing restart");
		return errno ? errno : EIO;
	}

	stop_status = stop_itunes_locked();
	if (stop_status != 0) {
		logmessage("Firefly", "restart aborted because the existing daemon did not stop");
		restart_status = stop_status;
		goto out;
	}

	mount_status = storage_has_mounted_devices("restarting Firefly");
	if (mount_status < 0) {
		logmessage("Firefly", "restart aborted because mounted storage state is unknown");
		restart_status = errno ? errno : EIO;
		goto out;
	}
	if (mount_status > 0)
		restart_status = run_itunes();

out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

#if defined (APP_TRMD)
int is_torrent_support(void)
{
	return check_if_file_exist("/usr/bin/transmission-daemon");
}

int is_torrent_run(void)
{
	if (!is_torrent_support())
		return 0;

	return storage_daemon_presence("transmission-daemon", NULL);
}

static int stop_torrent_locked(void)
{
	char *svcs[] = { "transmission-daemon", NULL };
	enum process_presence process_state;
	int command_status;

	if (!is_torrent_support())
		return 0;

	process_state = is_torrent_run();
	if (process_state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (process_state != PROCESS_PRESENCE_RUNNING) {
		if (!errno)
			errno = EIO;
		logmessage("Transmission", "refusing name-wide stop because daemon presence is unknown");
		return -1;
	}

	/* Use the shared identity-snapshot stop transaction instead of the
	 * script's pidof/killall path, which cannot prove its PID snapshot. */
	command_status = kill_services(svcs, 25, 1);
	if (storage_service_stop_complete(command_status, is_torrent_run()) != 0) {
		logmessage("Transmission", "daemon did not stop cleanly or process state is unknown");
		return -1;
	}

	return 0;
}

int
stop_torrent(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Transmission", "cannot acquire storage operation lock; refusing standalone stop");
		return -1;
	}

	status = stop_torrent_locked();
	file_unlock(storage_lock);
	return storage_service_stop_complete(status, is_torrent_run());
}

int run_torrent(void)
{
	char *apps_name = "Transmission";
	char *link_path = "/mnt/transmission";
	char *dest_dir = "transmission";
	int process_state;
	int process_error;
	int launch_status;
	int launch_errno;
	int start_status;
	int start_errno;
	int link_status;
	int service_enabled, enable_status;

	enable_status = storage_service_enabled_status(
		"trmd_enable", "Transmission", &service_enabled);
	if (enable_status != 0)
		return enable_status;
	if (!service_enabled)
		return 0;

	if (!is_torrent_support())
		return 0;

	process_state = is_torrent_run();
	if (process_state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		logmessage("transmission-daemon", "startup skipped because process inventory is unknown");
		return errno ? errno : EIO;
	}

	unlink(link_path);
	link_status = create_mp_link_status(dest_dir, link_path, 0);
	if (link_status != 0) {
		logmessage(apps_name, "Cannot start: please create dir \"%s\" on target volume (status=%d)!",
			   dest_dir, link_status);
		return link_status;
	}

	errno = 0;
	launch_status = eval("/usr/bin/transmission.sh", "start");
	launch_errno = errno;
	errno = 0;
	process_state = is_torrent_run();
	process_error = process_state == PROCESS_PRESENCE_UNKNOWN ?
		(errno ? errno : EIO) : 0;
	start_status = storage_log_daemon_start_result(
		apps_name, launch_status, (enum process_presence)process_state,
		process_error);
	if (start_status != 0) {
		start_errno = launch_status != 0 ?
			(launch_errno ? launch_errno : EIO) :
			(process_error ? process_error : (errno ? errno : start_status));
		return storage_rollback_failed_start(
			apps_name, "transmission-daemon", start_status, start_errno,
			stop_torrent_locked);
	}
	return 0;
}

int restart_torrent(void)
{
	int storage_lock, is_run_before, is_run_after;
	int stop_status, mount_status, firewall_status;
	int restart_status = 0;

	/* Hold the storage eject barrier through Transmission and firewall updates. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Transmission", "cannot acquire storage operation lock; refusing restart");
		return errno ? errno : EIO;
	}

	is_run_before = is_torrent_run();
	stop_status = stop_torrent_locked();
	if (stop_status != 0) {
		logmessage("Transmission", "restart aborted because the existing daemon did not stop");
		restart_status = stop_status;
		goto out;
	}

	mount_status = storage_has_mounted_devices("restarting Transmission");
	if (mount_status > 0)
		restart_status = run_torrent();
	else if (mount_status < 0) {
		logmessage("Transmission", "restart aborted because mounted storage state is unknown");
		restart_status = errno ? errno : EIO;
	}

	is_run_after = is_torrent_run();
	if (storage_service_process_presence_changed(is_run_before, is_run_after)) {
		firewall_status = restart_firewall();
		if (firewall_status != 0)
			logmessage("Transmission", "firewall/UPnP refresh failed after restart (status=%d)",
				   firewall_status);
		restart_status = rc_first_service_start_status(restart_status, firewall_status);
	}

out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

#if defined (APP_ARIA)
int is_aria_support(void)
{
	return check_if_file_exist("/usr/bin/aria2c");
}

int is_aria_run(void)
{
	if (!is_aria_support())
		return 0;

	return storage_daemon_presence("aria2c", NULL);
}

static int stop_aria_locked(void)
{
	char *svcs[] = { "aria2c", NULL };
	enum process_presence process_state;
	int command_status;

	if (!is_aria_support())
		return 0;

	process_state = is_aria_run();
	if (process_state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (process_state != PROCESS_PRESENCE_RUNNING) {
		if (!errno)
			errno = EIO;
		logmessage("Aria", "refusing name-wide stop because daemon presence is unknown");
		return -1;
	}

	/* Preserve the same PID/start-time validation used by other storage
	 * daemons; the legacy script signals a fresh pidof/killall name lookup. */
	command_status = kill_services(svcs, 15, 1);
	if (storage_service_stop_complete(command_status, is_aria_run()) != 0) {
		logmessage("Aria", "daemon did not stop cleanly or process state is unknown");
		return -1;
	}

	return 0;
}

int
stop_aria(void)
{
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Aria", "cannot acquire storage operation lock; refusing standalone stop");
		return -1;
	}

	status = stop_aria_locked();
	file_unlock(storage_lock);
	return storage_service_stop_complete(status, is_aria_run());
}

int run_aria(void)
{
	char *apps_name = "Aria";
	char *link_path = "/mnt/aria";
	char *dest_dir = "aria";
	int process_state;
	int process_error;
	int launch_status;
	int launch_errno;
	int start_status;
	int start_errno;
	int link_status;
	int service_enabled, enable_status;

	enable_status = storage_service_enabled_status(
		"aria_enable", "Aria", &service_enabled);
	if (enable_status != 0)
		return enable_status;
	if (!service_enabled)
		return 0;

	if (!is_aria_support())
		return 0;

	process_state = is_aria_run();
	if (process_state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_state != PROCESS_PRESENCE_ABSENT) {
		logmessage("aria2c", "startup skipped because process inventory is unknown");
		return errno ? errno : EIO;
	}

	unlink(link_path);
	link_status = create_mp_link_status(dest_dir, link_path, 0);
	if (link_status != 0) {
		logmessage(apps_name, "Cannot start: please create dir \"%s\" on target volume (status=%d)!",
			   dest_dir, link_status);
		return link_status;
	}

	errno = 0;
	launch_status = eval("/usr/bin/aria.sh", "start");
	launch_errno = errno;
	errno = 0;
	process_state = is_aria_run();
	process_error = process_state == PROCESS_PRESENCE_UNKNOWN ?
		(errno ? errno : EIO) : 0;
	start_status = storage_log_daemon_start_result(
		apps_name, launch_status, (enum process_presence)process_state,
		process_error);
	if (start_status != 0) {
		start_errno = launch_status != 0 ?
			(launch_errno ? launch_errno : EIO) :
			(process_error ? process_error : (errno ? errno : start_status));
		return storage_rollback_failed_start(
			apps_name, "aria2c", start_status, start_errno,
			stop_aria_locked);
	}
	return 0;
}

int restart_aria(void)
{
	int storage_lock, is_run_before, is_run_after;
	int stop_status, mount_status, firewall_status;
	int restart_status = 0;

	/* Hold the storage eject barrier through Aria and firewall updates. */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Aria", "cannot acquire storage operation lock; refusing restart");
		return errno ? errno : EIO;
	}

	is_run_before = is_aria_run();
	stop_status = stop_aria_locked();
	if (stop_status != 0) {
		logmessage("Aria", "restart aborted because the existing daemon did not stop");
		restart_status = stop_status;
		goto out;
	}

	mount_status = storage_has_mounted_devices("restarting Aria");
	if (mount_status > 0)
		restart_status = run_aria();
	else if (mount_status < 0) {
		logmessage("Aria", "restart aborted because mounted storage state is unknown");
		restart_status = errno ? errno : EIO;
	}

	is_run_after = is_aria_run();
	if (storage_service_process_presence_changed(is_run_before, is_run_after)) {
		firewall_status = restart_firewall();
		if (firewall_status != 0)
			logmessage("Aria", "firewall/UPnP refresh failed after restart (status=%d)",
				   firewall_status);
		restart_status = rc_first_service_start_status(restart_status, firewall_status);
	}

out:
	file_unlock(storage_lock);
	return restart_status;
}
#endif

int
stop_ftpsamba(void)
{
#if defined (APP_FTPD) || defined (APP_SMBD)
	int storage_lock, status = 0;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("Storage", "cannot acquire storage operation lock; refusing FTP/Samba stop");
		return -1;
	}

#if defined (APP_FTPD)
	{
		int is_run_before, final_presence, firewall_status = 0;

		is_run_before = is_ftp_run();
		status = stop_ftp_locked(&final_presence);
		if (ftp_stop_should_reconcile_firewall(final_presence)) {
			firewall_status = control_ftp_fw(is_run_before);
			if (firewall_status != 0)
				logmessage("Storage", "firewall/UPnP refresh failed after FTP/Samba stop (status=%d)",
					   firewall_status);
			status = rc_first_service_start_status(status, firewall_status);
		}
	}
#endif
#if defined (APP_SMBD)
	status = storage_service_stop_status_merge(status, stop_samba_locked(0));
#endif

	file_unlock(storage_lock);
	return status;
#else
	return 0;
#endif
}

struct storage_ejected_mount_entry {
	char device[32];
	char mountpoint[256];
	struct storage_ejected_mount_entry *next;
};

static void
free_storage_ejected_mounts(struct storage_ejected_mount_entry *entry)
{
	struct storage_ejected_mount_entry *next;

	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
}

struct storage_ejected_partition_entry {
	char device[32];
	struct storage_ejected_partition_entry *next;
};

static void
free_storage_ejected_partitions(struct storage_ejected_partition_entry *entry)
{
	struct storage_ejected_partition_entry *next;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
}


struct storage_ejected_swap_entry {
	char path[256];
	char device[32];
	struct storage_ejected_swap_entry *next;
};

static void
free_storage_ejected_swaps(struct storage_ejected_swap_entry *entry)
{
	struct storage_ejected_swap_entry *next;

	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
}

/* Stage raw removable-device swap entries before deciding that eject is done. */
static int
storage_stage_ejected_swaps(struct storage_ejected_swap_entry **staged)
{
	FILE *procpt;
	char line[512], swap_path[256], device[32];
	int parse_status, parse_error, read_error, close_status, close_error;
	struct storage_ejected_swap_entry *head, *tail, *entry;

	if (!staged) {
		errno = EINVAL;
		return -1;
	}
	*staged = NULL;
	procpt = fopen(PROC_SWAPS_FILE, "r");
	if (!procpt) {
		logmessage("Storage", "cannot read %s before ejected-device cleanup (errno %d)",
			   PROC_SWAPS_FILE, errno);
		return -1;
	}

	head = tail = NULL;
	parse_error = 0;
	if (!fgets(line, sizeof(line), procpt) ||
	    storage_ejected_parse_swaps_header(line) != 0)
		parse_error = 1;
	while (!parse_error && fgets(line, sizeof(line), procpt)) {
		if (storage_ejected_parse_swap_line(line, swap_path,
						    sizeof(swap_path)) != 0) {
			parse_error = 1;
			break;
		}
		parse_status = storage_ejected_swap_device_name(swap_path, device,
							       sizeof(device));
		if (parse_status < 0) {
			parse_error = 1;
			break;
		}
		/*
		 * Keep absolute swap-file paths too. A swap file under a removable
		 * mount is otherwise invisible to raw-device matching and keeps the
		 * stale mount busy forever after physical unplug. Non-removable
		 * /dev nodes and relative paths are not safe to associate by path.
		 */
		if (parse_status == 0 &&
		    (swap_path[0] != '/' || strncmp(swap_path, "/dev/", 5) == 0))
			continue;

		entry = (struct storage_ejected_swap_entry *)malloc(sizeof(*entry));
		if (!entry) {
			parse_error = 1;
			break;
		}
		strcpy(entry->path, swap_path);
		if (parse_status == 1)
			strcpy(entry->device, device);
		else
			entry->device[0] = '\0';
		entry->next = NULL;
		if (tail)
			tail->next = entry;
		else
			head = entry;
		tail = entry;
	}
	read_error = ferror(procpt);
	close_status = fclose(procpt);
	close_error = (close_status != 0);
	if (!storage_ejected_inventory_complete(read_error, close_error,
						parse_error)) {
		logmessage("Storage", "incomplete %s before ejected-device cleanup (read error=%d, close error=%d, malformed/truncated row or allocation failure=%d); leaving swap state untouched",
			   PROC_SWAPS_FILE, read_error, close_error, parse_error);
		free_storage_ejected_swaps(head);
		return -1;
	}
	*staged = head;
	return 0;
}


/* Stage a complete partition inventory before any ejected mount is changed. */
static int
storage_stage_ejected_partitions(struct storage_ejected_partition_entry **staged)
{
	FILE *procpt;
	char line[512], device[32];
	int parse_status, parse_error, read_error, close_status, close_error;
	struct storage_ejected_partition_entry *head, *tail, *entry;

	if (!staged) {
		errno = EINVAL;
		return -1;
	}
	*staged = NULL;
	procpt = fopen(PROC_PARTITIONS_FILE, "r");
	if (!procpt) {
		logmessage("Storage", "cannot read %s before ejected-device cleanup (errno %d)",
			   PROC_PARTITIONS_FILE, errno);
		return -1;
	}

	head = tail = NULL;
	parse_error = 0;
	while (fgets(line, sizeof(line), procpt)) {
		parse_status = storage_ejected_parse_partition_line(line, device,
								    sizeof(device));
		if (parse_status < 0) {
			parse_error = 1;
			break;
		}
		if (parse_status == 0)
			continue;
		entry = (struct storage_ejected_partition_entry *)malloc(sizeof(*entry));
		if (!entry) {
			parse_error = 1;
			break;
		}
		strcpy(entry->device, device);
		entry->next = NULL;
		if (tail)
			tail->next = entry;
		else
			head = entry;
		tail = entry;
	}

	read_error = ferror(procpt);
	close_status = fclose(procpt);
	close_error = (close_status != 0);
	if (!storage_ejected_inventory_complete(read_error, close_error, parse_error)) {
		logmessage("Storage", "incomplete %s before ejected-device cleanup (read error=%d, close error=%d, malformed/truncated row or allocation failure=%d); leaving mounts untouched",
			   PROC_PARTITIONS_FILE, read_error, close_error, parse_error);
		free_storage_ejected_partitions(head);
		return -1;
	}
	*staged = head;
	return 0;
}

static int
storage_ejected_partition_is_active(const char *device,
				    const struct storage_ejected_partition_entry *entries)
{
	const struct storage_ejected_partition_entry *entry;
	if (!device || !*device)
		return 0;
	for (entry = entries; entry; entry = entry->next) {
		if (strcmp(entry->device, device) == 0)
			return 1;
	}
	return 0;
}

static int
storage_ejected_swap_file_is_on_ejected_mount(
	const char *swap_path,
	const struct storage_ejected_mount_entry *mounts,
	const struct storage_ejected_partition_entry *partitions)
{
	const struct storage_ejected_mount_entry *mount;

	for (mount = mounts; mount; mount = mount->next) {
		if (!storage_ejected_partition_is_active(mount->device + 5,
							 partitions) &&
		    storage_ejected_swap_path_is_on_mount(swap_path, mount->mountpoint))
			return 1;
	}
	return 0;
}

static int
restore_home_dir(void)
{
	int unlink_status, unlink_error, symlink_status = 0, symlink_error = 0;
	int status;

	if (check_if_dir_exist("/home/admin"))
		return 0;

	errno = 0;
	unlink_status = unlink("/home/admin");
	unlink_error = errno;
	if (unlink_status == 0 || unlink_error == ENOENT) {
		errno = 0;
		symlink_status = symlink("/home/root", "/home/admin");
		symlink_error = errno;
	}

	status = storage_home_link_restore_status(
		0, unlink_status, unlink_error, symlink_status, symlink_error);
	if (status != 0) {
		errno = status;
		logmessage("Storage",
			   "failed to restore /home/admin link to /home/root (status=%d)",
			   status);
	}
	return status;
}

/* Retry failed physical-unplug cleanup without entering the app-start path. */
static void
storage_schedule_ejected_cleanup_retry(void)
{
	int marker_status, marker_errno, retry_status, saved_errno;

	saved_errno = errno;
	errno = 0;
	marker_status = nvram_set_int_temp("usb_unplug_ms", 1);
	if (marker_status != 0) {
		marker_errno = errno > 0 ? errno :
			(marker_status > 0 ? marker_status : EIO);
		retry_status = rc_notify_schedule_deferred_hotplug_retry(
			"on_unplug_mass_storage");
		if (retry_status != 0)
			logmessage("Storage",
				   "cannot persist unplug retry marker or queue PID 1 retry (marker_status=%d marker_errno=%d queue_status=%d)",
				   marker_status, marker_errno, retry_status);
		else
			logmessage("Storage",
				   "unplug retry marker write failed (status=%d); PID 1 retry queued",
				   marker_status);
	}
	alarm(5);
	errno = saved_errno;
}

static int
umount_ejected_locked(int storage_apps_stopped, int restart_storage_apps)
{
	FILE *procpt;
	char line[512], devname[32], mpname[256];
	int active, result, saved_errno, hook_status;
	int swapoff_status, swap_status;
	int mount_read_error, mount_close_error, mount_parse_error;
	int has_ejected_mounts, has_ejected_swaps, stop_status, rollback_status;
	int mount_count, start_status, home_status;
	struct storage_ejected_mount_entry *head, *tail, *entry;
	struct storage_ejected_partition_entry *partitions = NULL;
	struct storage_ejected_swap_entry *swaps = NULL, *swap_entry;

	head = tail = NULL;
	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (!procpt) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot read %s for ejected-device cleanup (errno %d)",
			   PROC_MOUNTS_FILE, saved_errno);
		(void)restore_home_dir();
		errno = saved_errno;
		return -1;
	}

	/*
	 * Collect and close the complete mount inventory before performing any
	 * unmount. A short/truncated row, read error, close error or allocation
	 * failure must not be mistaken for an absent device.
	 */
	mount_parse_error = 0;
	while (fgets(line, sizeof(line), procpt)) {
		if (storage_ejected_parse_mount_line(line, devname, sizeof(devname),
						     mpname, sizeof(mpname)) != 0) {
			mount_parse_error = 1;
			break;
		}
		if (!is_valid_storage_device(devname))
			continue;

		entry = (struct storage_ejected_mount_entry *)malloc(sizeof(*entry));
		if (!entry) {
			mount_parse_error = 1;
			break;
		}
		strcpy(entry->device, devname);
		strcpy(entry->mountpoint, mpname);
		entry->next = NULL;
		if (tail)
			tail->next = entry;
		else
			head = entry;
		tail = entry;
	}
	mount_read_error = ferror(procpt);
	mount_close_error = (fclose(procpt) != 0);
	if (!storage_ejected_inventory_complete(mount_read_error,
						mount_close_error,
						mount_parse_error)) {
		logmessage("Storage", "incomplete %s during ejected-device cleanup (read error=%d, close error=%d, malformed/truncated row or allocation failure=%d)",
			   PROC_MOUNTS_FILE, mount_read_error, mount_close_error,
			   mount_parse_error);
		saved_errno = errno ? errno : EIO;
		free_storage_ejected_mounts(head);
		(void)restore_home_dir();
		errno = saved_errno;
		return -1;
	}

	if (storage_stage_ejected_swaps(&swaps) != 0) {
		saved_errno = errno ? errno : EIO;
		free_storage_ejected_mounts(head);
		(void)restore_home_dir();
		errno = saved_errno;
		return -1;
	}
	if (!head && !swaps) {
		free_storage_ejected_swaps(swaps);
		return restore_home_dir() == 0 ? 0 : -1;
	}
	if (storage_stage_ejected_partitions(&partitions) != 0) {
		saved_errno = errno ? errno : EIO;
		free_storage_ejected_mounts(head);
		free_storage_ejected_swaps(swaps);
		(void)restore_home_dir();
		errno = saved_errno;
		return -1;
	}

	/*
	 * Physical removal is reported after the device has disappeared. Stop
	 * storage-backed daemons before dropping their stale mount paths, then
	 * restart them only after the remaining mount inventory is known.
	 */
	has_ejected_mounts = 0;
	for (entry = head; entry; entry = entry->next) {
		active = storage_ejected_partition_is_active(entry->device + 5,
							     partitions);
		if (!active) {
			has_ejected_mounts = 1;
			break;
		}
	}
	has_ejected_swaps = 0;
	for (swap_entry = swaps; swap_entry; swap_entry = swap_entry->next) {
		if (swap_entry->device[0]) {
			if (!storage_ejected_partition_is_active(swap_entry->device, partitions)) {
				has_ejected_swaps = 1;
				break;
			}
		} else if (storage_ejected_swap_file_is_on_ejected_mount(
				   swap_entry->path, head, partitions)) {
			has_ejected_swaps = 1;
			break;
		}
	}
	if (!head && !has_ejected_swaps) {
		free_storage_ejected_partitions(partitions);
		free_storage_ejected_swaps(swaps);
		return restore_home_dir() == 0 ? 0 : -1;
	}

	/*
	 * A removable disk may back a raw swap partition without any mounted
	 * filesystem. Recheck every swap device absent from /proc/partitions,
	 * including swap-only disks that have no /proc/mounts entry.
	 */
	swapoff_status = 0;
	saved_errno = 0;
	for (swap_entry = swaps; swap_entry; swap_entry = swap_entry->next) {
		if (swap_entry->device[0]) {
			if (storage_ejected_partition_is_active(swap_entry->device, partitions))
				continue;
			swap_status = try_device_swapoff(swap_entry->device);
		} else {
			if (!storage_ejected_swap_file_is_on_ejected_mount(
				    swap_entry->path, head, partitions))
				continue;
			swap_status = swapoff(swap_entry->path);
		}
		if (swap_status != 0) {
			saved_errno = errno ? errno : EIO;
			logmessage("Storage", "refusing ejected-device cleanup because swapoff failed for %s (status %d, errno %d)",
				   swap_entry->path, swap_status, saved_errno);
			swapoff_status = -1;
		}
	}

	if (has_ejected_mounts && !storage_apps_stopped) {
		errno = 0;
		stop_status = stop_stor_apps();
		if (stop_status != 0) {
			saved_errno = errno ? errno : EIO;
			rollback_status = storage_rollback_stor_apps_after_stop_failure(
				"physical hotplug eject", stop_status);
			logmessage("Storage",
				   "ejected-device cleanup aborted because storage services did not stop (stop=%d, rollback=%d)",
				   stop_status, rollback_status);
			free_storage_ejected_mounts(head);
			free_storage_ejected_partitions(partitions);
			free_storage_ejected_swaps(swaps);
			(void)restore_home_dir();
			errno = saved_errno;
			return -1;
		}
	}

	result = 0;
	if (!storage_ejected_may_unmount_device(1, swapoff_status)) {
		logmessage("Storage", "skipping all ejected-device unmounts because swapoff did not complete");
		result = -1;
	} else {
	for (entry = head; entry; entry = entry->next) {
		active = storage_ejected_partition_is_active(entry->device + 5,
							     partitions);
		if (active)
			continue;

		hook_status = eval("/usr/bin/opt-umount.sh", entry->device,
				   entry->mountpoint);
		if (hook_status != 0) {
			logmessage("Storage", "refusing to unmount ejected device %s from %s because Optware teardown failed (status %d)",
				   entry->device, entry->mountpoint, hook_status);
			result = -1;
			continue;
		}

		if (umount(entry->mountpoint) != 0) {
			saved_errno = errno;
			logmessage("Storage", "failed to unmount ejected device %s from %s (errno %d)",
				   entry->device, entry->mountpoint, saved_errno);
			result = -1;
		} else {
			(void)rmdir(entry->mountpoint);
		}
	}
	}

	free_storage_ejected_mounts(head);
	free_storage_ejected_partitions(partitions);
	free_storage_ejected_swaps(swaps);

	if (has_ejected_mounts && restart_storage_apps) {
		/*
		 * A failed Optware hook or unmount leaves an ejected mountpoint in
		 * place. Do not relaunch daemons against that stale mount; retry the
		 * cleanup first, then restart only after every ejected mount is gone.
		 */
		if (result != 0) {
			logmessage("Storage", "leaving storage applications stopped because physical eject cleanup was incomplete; scheduling unmount retry");
		} else {
			errno = 0;
			mount_count = count_stor_mountpoint();
			if (mount_count > 0) {
				start_status = start_stor_apps();
				if (start_status != 0) {
					logmessage("Storage", "storage applications failed to restart after physical eject (status=%d)",
						   start_status);
					nvram_set_int_temp("usb_hotplug_ms", 1);
					alarm(5);
					result = -1;
				}
			} else if (mount_count == 0) {
#if defined(APP_NFSD)
				unload_nfsd();
#endif
			} else {
				logmessage("Storage", "remaining storage mount inventory is unknown after physical eject; leaving storage applications stopped");
				result = -1;
			}
		}
	}

	if (result != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	home_status = restore_home_dir();
	if (home_status != 0) {
		if (saved_errno == 0)
			saved_errno = home_status;
		result = -1;
	}
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	else
		errno = 0;
	return result;
}

int
umount_ejected(void)
{
	int storage_lock, result, saved_errno;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot acquire storage operation lock for ejected-device cleanup (errno %d)",
			   saved_errno);
		storage_schedule_ejected_cleanup_retry();
		errno = saved_errno;
		return -1;
	}

	result = umount_ejected_locked(0, 1);
	file_unlock(storage_lock);
	if (result != 0)
		storage_schedule_ejected_cleanup_retry();
	return result;
}

struct storage_unmount_mount_entry {
	char device[32];
	char mountpoint[256];
	struct storage_unmount_mount_entry *next;
};

static void
free_storage_unmount_mounts(struct storage_unmount_mount_entry *entry)
{
	struct storage_unmount_mount_entry *next;

	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
}

/*
 * Take one complete storage-mount snapshot for the entire safe-removal
 * operation. No per-partition hook/unmount may run until this succeeds.
 */
static int
storage_stage_unmount_mounts(struct storage_unmount_mount_entry **staged)
{
	FILE *procpt;
	char line[512], devname[32], mpname[256];
	int mount_read_error, mount_close_error, mount_parse_error;
	struct storage_unmount_mount_entry *head, *tail, *entry;

	if (!staged) {
		errno = EINVAL;
		return -1;
	}
	*staged = NULL;

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (!procpt) {
		logmessage("Storage", "cannot read %s before safe removal (errno %d)",
			   PROC_MOUNTS_FILE, errno);
		return -1;
	}

	head = tail = NULL;
	mount_parse_error = 0;
	while (fgets(line, sizeof(line), procpt)) {
		if (storage_ejected_parse_mount_line(line, devname, sizeof(devname),
						     mpname, sizeof(mpname)) != 0) {
			mount_parse_error = 1;
			break;
		}
		if (!is_valid_storage_device(devname))
			continue;

		entry = (struct storage_unmount_mount_entry *)malloc(sizeof(*entry));
		if (!entry) {
			mount_parse_error = 1;
			break;
		}
		strcpy(entry->device, devname);
		strcpy(entry->mountpoint, mpname);
		entry->next = NULL;
		if (tail)
			tail->next = entry;
		else
			head = entry;
		tail = entry;
	}

	mount_read_error = ferror(procpt);
	mount_close_error = (fclose(procpt) != 0);
	if (!storage_ejected_inventory_complete(mount_read_error,
						mount_close_error,
						mount_parse_error)) {
		logmessage("Storage", "incomplete %s before safe removal (read error=%d, close error=%d, malformed/truncated row or allocation failure=%d); leaving mounts untouched",
			   PROC_MOUNTS_FILE, mount_read_error,
			   mount_close_error, mount_parse_error);
		free_storage_unmount_mounts(head);
		return -1;
	}

	*staged = head;
	return 0;
}

static int
storage_unmount_staged_disk_mounts(const char *disk,
				   struct storage_unmount_mount_entry *mounts)
{
	struct storage_unmount_mount_entry *entry;
	int result, hook_status;

	if (!disk || !*disk)
		return -1;

	result = 0;
	for (entry = mounts; entry; entry = entry->next) {
		if (!storage_ejected_mount_matches_disk(entry->device, disk))
			continue;

		hook_status = eval("/usr/bin/opt-umount.sh", entry->device,
				   entry->mountpoint);
		if (hook_status != 0) {
			logmessage("Storage", "refusing to unmount %s from %s because Optware teardown failed (status %d)",
				   entry->device, entry->mountpoint, hook_status);
			result = -1;
			continue;
		}
		if (umount(entry->mountpoint) != 0) {
			logmessage("Storage", "failed to unmount %s from %s (errno %d)",
				   entry->device, entry->mountpoint, errno);
			result = -1;
		} else {
			(void)rmdir(entry->mountpoint);
		}
	}

	return result;
}

int
count_stor_mountpoint(void)
{
	struct storage_unmount_mount_entry *mounts, *entry;
	int count;

	mounts = NULL;
	if (storage_stage_unmount_mounts(&mounts) != 0)
		return storage_mount_inventory_result(0, 1);

	count = 0;
	for (entry = mounts; entry; entry = entry->next)
		count++;
	free_storage_unmount_mounts(mounts);
	return storage_mount_inventory_result(count, 0);
}

static int
umount_stor_path_range(disk_info_t *disks_info, int port_b, int port_e,
		       int filter_ports, const char *dev_name, int do_spindown)
{
	disk_info_t *follow_disk;
	partition_info_t *follow_partition;
	struct storage_unmount_mount_entry *mounts;
	int result, swapoff_status, swap_status, disk_status, saved_errno;

	if (!disks_info) {
		logmessage("Storage", "cannot inspect storage topology before safe removal");
		return -1;
	}

	/*
	 * Validate one mount inventory for every selected disk before changing
	 * swap or mount state; later partitions must not trigger late reads.
	 */
	if (storage_stage_unmount_mounts(&mounts) != 0)
		return -1;

	/*
	 * Resolve all selected swaps before unmounting any selected disk. A
	 * later unknown/failed swap record must not follow an earlier unmount.
	 */
	swapoff_status = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (filter_ports &&
		    ((int)follow_disk->port_root < port_b ||
		     (int)follow_disk->port_root > port_e))
			continue;
		if (dev_name &&
		    (!follow_disk->device || strcmp(follow_disk->device, dev_name) != 0))
			continue;

		for (follow_partition = follow_disk->partitions;
		     follow_partition != NULL; follow_partition = follow_partition->next) {
			if (!follow_partition->swapon)
				continue;

			swap_status = try_device_swapoff(follow_partition->device);
			if (swap_status != 0) {
				saved_errno = errno;
				logmessage("Storage", "failed to disable swap on /dev/%s (status %d, errno %d)",
					   follow_partition->device ? follow_partition->device : "(unknown)",
					   swap_status, saved_errno);
				swapoff_status = -1;
			}
		}
	}

	if (!storage_ejected_may_unmount_device(1, swapoff_status)) {
		logmessage("Storage", "skipping all selected unmounts because swapoff did not complete");
		free_storage_unmount_mounts(mounts);
		return -1;
	}

	result = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (filter_ports &&
		    ((int)follow_disk->port_root < port_b ||
		     (int)follow_disk->port_root > port_e))
			continue;
		if (dev_name &&
		    (!follow_disk->device || strcmp(follow_disk->device, dev_name) != 0))
			continue;
		if (!follow_disk->device || !*follow_disk->device) {
			logmessage("Storage", "cannot safely remove a disk with no device identity");
			result = -1;
			continue;
		}

		disk_status = storage_unmount_staged_disk_mounts(follow_disk->device, mounts);
		if (do_spindown && follow_disk->port_root != MMC_VIRT_PORT_ID) {
			if (disk_status == 0) {
				spindown_hdd(follow_disk->device);
			} else {
				logmessage("Storage", "skipping spindown for %s because unmount did not complete",
					   follow_disk->device);
			}
		}
		if (disk_status != 0)
			result = -1;
	}

	free_storage_unmount_mounts(mounts);
	return result;
}

int
umount_stor_path(disk_info_t *disks_info, int port, const char *dev_name, int do_spindown)
{
	return umount_stor_path_range(disks_info, port, port, port != 0,
				      dev_name, do_spindown);
}

int
stop_stor_apps(void)
{
	int need_restart_fw = 0;
#if defined (APP_FTPD)
	int ftp_was_present, ftp_final_presence;
#endif
	int stop_status = 0;
	int firewall_status;

#if defined (APP_NFSD)
	if (stop_nfsd_locked() != 0) {
		logmessage("Storage", "NFS server did not stop cleanly; storage removal must be aborted");
		stop_status = -1;
	}
#endif
#if defined (APP_SMBD)
	if (stop_samba_locked(0) != 0) {
		logmessage("Storage", "Samba server did not stop cleanly; storage removal must be aborted");
		stop_status = -1;
	}
#endif
#if defined (APP_FTPD)
	/* Preserve refresh for an initially present/unknown daemon, and also
	 * remove stale FTP rules when the daemon was already absent. */
	ftp_was_present = is_ftp_run() != PROCESS_PRESENCE_ABSENT;
	if (stop_ftp_locked(&ftp_final_presence) != 0) {
		logmessage("Storage", "FTP server did not stop cleanly; storage removal must be aborted");
		stop_status = -1;
	}
	need_restart_fw |= ftp_storage_stop_needs_firewall_refresh(
		ftp_was_present, ftp_final_presence);
#endif
#if defined (APP_MINIDLNA)
	if (stop_dms_locked() != 0) {
		logmessage("Storage", "storage removal must be aborted because MiniDLNA did not stop");
		stop_status = -1;
	}
#endif
#if defined (APP_FIREFLY)
	if (stop_itunes_locked() != 0) {
		logmessage("Storage", "storage removal must be aborted because Firefly did not stop");
		stop_status = -1;
	}
#endif
#if defined (APP_TRMD)
	need_restart_fw |= is_torrent_run();
	if (stop_torrent_locked() != 0) {
		logmessage("Storage", "storage removal must be aborted because Transmission did not stop");
		stop_status = -1;
	}
#endif
#if defined (APP_ARIA)
	need_restart_fw |= is_aria_run();
	if (stop_aria_locked() != 0) {
		logmessage("Storage", "storage removal must be aborted because Aria did not stop");
		stop_status = -1;
	}
#endif

	if (need_restart_fw) {
		firewall_status = restart_firewall();
		if (firewall_status != 0)
			logmessage("Storage", "firewall/UPnP refresh failed after storage stop (status=%d)",
				   firewall_status);
		stop_status = rc_first_service_start_status(stop_status, firewall_status);
	}

	return stop_status;
}

int
start_stor_apps(void)
{
	int need_restart_fw = 0;
	int start_status = 0;
	int firewall_status;
#if defined (APP_FTPD) || defined (APP_SMBD) || defined (APP_NFSD) || \
    defined (APP_MINIDLNA) || defined (APP_FIREFLY) || \
    defined (APP_TRMD) || defined (APP_ARIA)
	int service_status;
#endif

#if defined (APP_FTPD)
	service_status = run_ftp();
	if (service_status != 0)
		logmessage("Storage", "FTP startup did not reach its daemon postconditions");
	start_status = rc_first_service_start_status(start_status, service_status);
	need_restart_fw |= (is_ftp_run() != PROCESS_PRESENCE_ABSENT);
#endif
#if defined (APP_SMBD)
	service_status = run_samba();
	if (service_status != 0)
		logmessage("Storage", "Samba startup did not reach its daemon postconditions");
	start_status = rc_first_service_start_status(start_status, service_status);
#endif
#if defined (APP_NFSD)
	service_status = run_nfsd_locked();
	if (service_status != 0)
		logmessage("Storage", "NFS startup did not complete successfully");
	start_status = rc_first_service_start_status(start_status, service_status);
#endif
#if defined (APP_MINIDLNA)
	service_status = run_dms(0);
	start_status = rc_first_service_start_status(start_status, service_status);
#endif
#if defined (APP_FIREFLY)
	service_status = run_itunes();
	start_status = rc_first_service_start_status(start_status, service_status);
#endif
#if defined (APP_TRMD)
	service_status = run_torrent();
	start_status = rc_first_service_start_status(start_status, service_status);
	need_restart_fw |= (is_torrent_run() != PROCESS_PRESENCE_ABSENT);
#endif
#if defined (APP_ARIA)
	service_status = run_aria();
	start_status = rc_first_service_start_status(start_status, service_status);
	need_restart_fw |= (is_aria_run() != PROCESS_PRESENCE_ABSENT);
#endif

	if (need_restart_fw) {
		firewall_status = restart_firewall();
		if (firewall_status != 0)
			logmessage("Storage", "firewall/UPnP refresh failed after storage start (status=%d)",
				   firewall_status);
		start_status = rc_first_service_start_status(start_status, firewall_status);
	}

	return start_status;
}


/* A failed service-stop preflight leaves the storage mounted. Restore the
 * configured storage daemons that were stopped earlier in the sequence before
 * releasing the operation lock; start routines are tri-state/idempotent and
 * refuse duplicate launches when a daemon is still RUNNING or UNKNOWN. */
static int
storage_rollback_stor_apps_after_stop_failure(const char *operation,
						      int stop_status)
{
	int saved_errno, start_status, result;

	if (stop_status == 0)
		return 0;

	saved_errno = errno ? errno : EIO;
	start_status = start_stor_apps();
	result = storage_service_stop_status_merge(stop_status, start_status);
	if (start_status != 0)
		logmessage("Storage",
			   "storage application rollback after %s was incomplete (stop=%d, restart=%d)",
			   operation ? operation : "failed stop", stop_status,
			   start_status);
	errno = saved_errno;
	return result;
}


static int
storage_optware_start_result_read(void)
{
	unsigned char child_status = 1;
	ssize_t bytes_read;
	int result;

	if (storage_optware_result_fd < 0)
		return STORAGE_OPTWARE_START_CHILD_NONE;

	do {
		bytes_read = read(storage_optware_result_fd, &child_status,
				  sizeof(child_status));
	} while (bytes_read < 0 && errno == EINTR);

	if (bytes_read < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return STORAGE_OPTWARE_START_CHILD_RUNNING;
		logmessage("Storage", "cannot read deferred Optware startup result (errno %d)", errno);
		bytes_read = 0;
	}

	result = storage_optware_start_decode_result((int)bytes_read,
						     (int)child_status);
	close(storage_optware_result_fd);
	storage_optware_result_fd = -1;

	return result;
}

static int
storage_optware_start_async(void)
{
	int pipe_fd[2], command_status;
	unsigned char child_result;
	ssize_t written;
	pid_t child;

	/* A live result channel owns this single startup attempt. */
	if (storage_optware_result_fd >= 0)
		return -1;

	if (pipe(pipe_fd) != 0) {
		logmessage("Storage", "cannot create deferred Optware startup result pipe (errno %d)", errno);
		return -1;
	}

	if (storage_optware_configure_result_pipe(pipe_fd[0], pipe_fd[1]) != 0)
		goto failed;

	child = fork();
	if (child < 0)
		goto failed;

	if (child == 0) {
		close(pipe_fd[0]);
		command_status = system("/usr/bin/opt-start.sh");
		child_result = (command_status != -1 &&
				WIFEXITED(command_status) &&
				WEXITSTATUS(command_status) == 0) ? 0 : 1;
		do {
			written = write(pipe_fd[1], &child_result, sizeof(child_result));
		} while (written < 0 && errno == EINTR);
		close(pipe_fd[1]);
		/* Wake PID 1 after publishing the result; SIGCHLD remains safely reaped there. */
		(void)kill(1, SIGALRM);
		_exit(child_result == 0 ? 0 : 1);
	}

	close(pipe_fd[1]);
	storage_optware_result_fd = pipe_fd[0];
	logmessage("Storage", "started deferred Optware service startup (pid %d)", (int)child);
	return 0;

failed:
	logmessage("Storage", "cannot launch deferred Optware startup worker (errno %d)", errno);
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	return -1;
}

static int
try_start_stor_apps(void)
{
	int storage_lock;
	int mounted_devices;
	int need_optware = 0, hotplug_value = 0;
	int optware_status, optware_error;
	int hotplug_status, hotplug_error;
	int marker_status, ack_status;
	int child_result;
	int integrated_start_status = 0;

	/*
	 * Serialize integrated storage-service startup with safe removal, but do
	 * not run the external Optware hooks under storage.lock. Their worker owns
	 * only the separate Optware lifecycle lock after this lock is released.
	 */
	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		logmessage("Storage", "cannot acquire storage operation lock; deferring storage application startup");
		return -1;
	}

	mounted_devices = storage_has_mounted_devices("starting storage applications");
	if (mounted_devices < 0) {
		file_unlock(storage_lock);
		return -1;
	}

	/*
	 * Inspect the asynchronous Optware child before retrying integrated
	 * services. A five-second wakeup must not duplicate FTP/Samba/NFS starts
	 * while the previous Optware attempt is still running.
	 */
	/*
	 * Snapshot the pending Optware request before consuming the asynchronous
	 * worker result. A failed NVRAM read must not look like "not requested",
	 * otherwise the child completion can be discarded and startup stranded.
	 */
	if (mounted_devices > 0) {
		hotplug_status = storage_read_integer_status(
			"usb_hotplug_ms", 0, &hotplug_value);
		if (hotplug_status != 0) {
			hotplug_error = hotplug_status > 0 ? hotplug_status : EIO;
			logmessage("Storage",
				   "cannot read USB storage startup event; preserving pending state for retry (status=%d)",
				   hotplug_error);
			file_unlock(storage_lock);
			errno = hotplug_error;
			return -1;
		}
		optware_status = storage_read_integer_status(
			"usb_opt_start", 0, &need_optware);
		if (optware_status != 0) {
			optware_error = optware_status > 0 ? optware_status : EIO;
			logmessage("Storage",
				   "cannot read Optware startup request; preserving pending state for retry (status=%d)",
				   optware_error);
			file_unlock(storage_lock);
			errno = optware_error;
			return -1;
		}
	}

	child_result = storage_optware_start_result_read();
	if (child_result == STORAGE_OPTWARE_START_CHILD_SUCCESS)
		storage_optware_start_ack_pending = 1;
	if (child_result == STORAGE_OPTWARE_START_CHILD_RUNNING) {
		file_unlock(storage_lock);
		return 1;
	}

	if (mounted_devices == 0) {
		/* A complete empty inventory makes any old hotplug request stale. */
		ack_status = storage_ack_event_marker_status("usb_hotplug_ms");
		if (ack_status != 0)
			logmessage("Storage",
				   "cannot acknowledge stale USB storage startup event (status=%d)",
				   ack_status);
		marker_status = storage_ack_event_marker_status("usb_opt_start");
		if (marker_status != 0)
			logmessage("Storage",
				   "cannot acknowledge stale Optware startup request (status=%d)",
				   marker_status);
		ack_status = rc_first_service_start_status(ack_status, marker_status);
		marker_status = storage_optware_ack_state(1, marker_status,
							  &storage_optware_start_ack_pending);
		ack_status = rc_first_service_start_status(ack_status, marker_status);
		file_unlock(storage_lock);
		if (ack_status != 0) {
			errno = ack_status > 0 ? ack_status : EIO;
			return -1;
		}
		return 0;
	}

	/*
	 * Keep usb_hotplug_ms pending when a required integrated daemon fails.
	 * The normal deferred alarm will retry it; only a successful start
	 * acknowledges the event. Optware runs after the storage lock is released.
	 */
	if (hotplug_value == 1) {
		integrated_start_status = start_stor_apps();
		if (rc_service_start_event_is_complete(integrated_start_status)) {
			marker_status = storage_ack_event_marker_status("usb_hotplug_ms");
			if (marker_status != 0) {
				logmessage("Storage",
					   "cannot acknowledge USB storage startup event; preserving it for retry (status=%d)",
					   marker_status);
				integrated_start_status = rc_first_service_start_status(
					integrated_start_status, marker_status);
			}
		} else {
			logmessage("Storage", "integrated storage application startup failed; preserving USB storage startup event for retry (status=%d)",
				   integrated_start_status);
		}
	}

	/* External init scripts must never inherit storage.lock. */
	file_unlock(storage_lock);

	if (storage_optware_start_ack_pending) {
		marker_status = need_optware ?
			storage_ack_event_marker_status("usb_opt_start") : 0;
		ack_status = storage_optware_ack_state(need_optware,
						       marker_status,
						       &storage_optware_start_ack_pending);
		if (ack_status != 0)
			logmessage("Storage",
				   "Optware completed but its request marker could not be acknowledged; suppressing duplicate startup and retrying (status=%d)",
				   ack_status);
		return rc_first_service_start_status(integrated_start_status,
					     ack_status);
	}
	if (child_result == STORAGE_OPTWARE_START_CHILD_FAILURE) {
		if (need_optware)
			logmessage("Storage", "deferred Optware startup failed; preserving pending state for retry");
		return rc_first_service_start_status(integrated_start_status,
						     need_optware ? -1 : 0);
	}

	if (!need_optware)
		return integrated_start_status;
	if (storage_optware_start_async() != 0)
		return rc_first_service_start_status(integrated_start_status, -1);

	return rc_first_service_start_status(integrated_start_status, 1);
}

void
on_deferred_hotplug_dev(void)
{
#if defined (USE_USB_SUPPORT)
	int plug_modem = 0;
	int unplug_modem = 0;
	int printer_start_status;
	int modem_unplug_status = 0;
	int modem_plug_status = 0;
	int modem_rule_value = 0, modem_rule_status = 0;
	int printer_unplug_value = 0, printer_hotplug_value = 0;
	int printer_unplug_status, printer_hotplug_status, printer_marker_status;
	int printer_unplug_pending = 0, printer_hotplug_pending = 0;
	int modem_unplug_value = 0, modem_hotplug_value = 0;
	int modem_unplug_marker_status, modem_hotplug_marker_status;
	int modem_marker_status, modem_unplug_pending = 0, modem_hotplug_pending = 0;
	int marker_read_error;
	static int printer_unplug_action_done;
	static int printer_hotplug_action_done;
	static int modem_unplug_action_done;
	static int modem_hotplug_action_done;
	unsigned int modem_retry_mask;
#endif
	int storage_start_result;
#if defined (USE_STORAGE)
	int storage_unplug_status;
	int storage_unplug_value = 0, storage_hotplug_value = 0;
	int storage_optware_value = 0, storage_unplug_pending = 0;
	int storage_unplug_read_status, storage_hotplug_read_status;
	int storage_optware_read_status, storage_dispatch_status;
	int storage_dispatch_error, storage_should_start = 0;
	static int storage_unplug_action_done;
#else
	char storage_hotplug_text[NVRAM_MAX_VALUE_LEN + 1] = {0};
	char storage_optware_text[NVRAM_MAX_VALUE_LEN + 1] = {0};
	int storage_hotplug_enabled = 0, storage_optware_enabled = 0;
	int storage_hotplug_read_status, storage_optware_read_status;
	int storage_dispatch_status, storage_dispatch_error;
	int storage_should_start = 0;
#endif

#if defined (USE_STORAGE)
	/*
	 * Read event markers with status before acknowledging or dispatching them.
	 * nvram_match() turns a backend error into a false marker and can lose the
	 * only wakeup for a pending Optware/storage start.
	 */
	storage_unplug_read_status = storage_read_integer_status(
		"usb_unplug_ms", 0, &storage_unplug_value);
	if (storage_unplug_read_status != 0) {
		storage_unplug_pending = 1;
	} else if (storage_unplug_value != 1) {
		storage_unplug_action_done = 0;
	} else {
		storage_unplug_status = 0;
		if (storage_deferred_event_action_should_run(1, storage_unplug_action_done))
			storage_unplug_status = umount_ejected();
		storage_dispatch_error = 0;
		if (storage_unplug_status == 0)
			storage_dispatch_error = storage_ack_event_marker_status("usb_unplug_ms");
		storage_unplug_status = storage_deferred_event_ack_status(
			1, storage_unplug_status, storage_dispatch_error,
			&storage_unplug_action_done);
		if (storage_unplug_status != 0) {
			storage_unplug_pending = 1;
			logmessage("Storage", "deferred physical-unplug cleanup/ack retry failed (status=%d)",
				   storage_unplug_status);
			alarm(5);
		}
	}

	storage_hotplug_read_status = storage_read_integer_status(
		"usb_hotplug_ms", 0, &storage_hotplug_value);
	storage_optware_read_status = storage_read_integer_status(
		"usb_opt_start", 0, &storage_optware_value);
	storage_dispatch_status = storage_deferred_start_dispatch_status(
		storage_unplug_read_status, storage_unplug_pending,
		storage_hotplug_read_status, storage_hotplug_value == 1,
		storage_optware_read_status, storage_optware_value == 1,
		storage_optware_result_fd >= 0, &storage_should_start);
	if (storage_dispatch_status != 0) {
		storage_dispatch_error = storage_dispatch_status > 0 ?
			storage_dispatch_status : EIO;
		logmessage("Storage",
			   "cannot read deferred storage event markers; preserving pending events for retry (status=%d)",
			   storage_dispatch_error);
		alarm(5);
	} else if (storage_should_start) {
		storage_start_result = try_start_stor_apps();
		if (storage_start_result != 0)
			alarm(5);
	}
#else
	/*
	 * Builds without integrated storage support can still have Optware
	 * startup requests. Read both markers with status so a failed NVRAM
	 * backend cannot silently discard the only deferred-start wakeup.
	 */
	storage_hotplug_read_status = storage_read_string_status(
		"usb_hotplug_ms", storage_hotplug_text, sizeof(storage_hotplug_text));
	storage_hotplug_read_status = storage_service_enable_value_status(
		storage_hotplug_read_status, storage_hotplug_text,
		&storage_hotplug_enabled);
	storage_optware_read_status = storage_read_string_status(
		"usb_opt_start", storage_optware_text, sizeof(storage_optware_text));
	storage_optware_read_status = storage_service_enable_value_status(
		storage_optware_read_status, storage_optware_text,
		&storage_optware_enabled);
	storage_dispatch_status = storage_deferred_start_dispatch_status(
		0, 0,
		storage_hotplug_read_status, storage_hotplug_enabled,
		storage_optware_read_status, storage_optware_enabled,
		storage_optware_result_fd >= 0, &storage_should_start);
	if (storage_dispatch_status != 0) {
		storage_dispatch_error = storage_dispatch_status > 0 ?
			storage_dispatch_status : EIO;
		logmessage("Storage",
			   "cannot read deferred Optware startup markers; preserving pending requests for retry (status=%d)",
			   storage_dispatch_error);
		alarm(5);
	} else if (storage_should_start) {
		storage_start_result = try_start_stor_apps();
		if (storage_start_result != 0)
			alarm(5);
	}
#endif

#if defined (USE_USB_SUPPORT)
	printer_unplug_status = storage_read_integer_status(
		"usb_unplug_lp", 0, &printer_unplug_value);
	printer_hotplug_status = storage_read_integer_status(
		"usb_hotplug_lp", 0, &printer_hotplug_value);
	printer_marker_status = usb_hotplug_event_pair_status(
		printer_unplug_status, printer_unplug_value,
		printer_hotplug_status, printer_hotplug_value,
		&printer_unplug_pending, &printer_hotplug_pending);
	if (printer_marker_status != 0) {
		marker_read_error = printer_marker_status > 0 ?
			printer_marker_status : EIO;
		logmessage("USB Printer",
			   "cannot read deferred printer event markers; preserving pending events for retry (status=%d)",
			   marker_read_error);
		alarm(5);
	} else {
		if (!printer_unplug_pending)
			printer_unplug_action_done = 0;
		if (!printer_hotplug_pending)
			printer_hotplug_action_done = 0;
		if (printer_unplug_pending) {
			printer_unplug_status = 0;
			if (usb_hotplug_event_action_should_run(printer_unplug_pending,
							 printer_unplug_action_done)) {
				if (!usb_port_module_used("usblp"))
					printer_unplug_status = stop_usb_printer_spoolers();
			}
			printer_marker_status = 0;
			if (printer_unplug_status == 0)
				printer_marker_status = storage_ack_event_marker_status("usb_unplug_lp");
			printer_unplug_status = usb_hotplug_event_ack_status(
				printer_unplug_pending, printer_unplug_status, printer_marker_status,
				&printer_unplug_action_done);
			if (printer_unplug_status != 0) {
				logmessage("USB Printer",
					   "printer spooler stop or unplug-event acknowledgement failed; keeping event queued for retry (status=%d)",
					   printer_unplug_status);
				alarm(5);
			}
		}
		if (printer_hotplug_pending) {
			printer_start_status = 0;
			if (usb_hotplug_event_action_should_run(printer_hotplug_pending,
							 printer_hotplug_action_done))
				printer_start_status = try_start_usb_printer_spoolers();
			if (printer_start_status == 0) {
				printer_marker_status = storage_ack_event_marker_status("usb_hotplug_lp");
				printer_start_status = usb_hotplug_event_ack_status(
					printer_hotplug_pending, 0, printer_marker_status,
					&printer_hotplug_action_done);
			}
			if (printer_start_status != 0) {
				logmessage("USB Printer",
					   "deferred printer startup/acknowledgement failed; keeping event queued for retry (status=%d)",
					   printer_start_status);
				alarm(5);
			}
		}
	}

	modem_unplug_marker_status = storage_read_integer_status(
		"usb_unplug_md", 0, &modem_unplug_value);
	modem_hotplug_marker_status = storage_read_integer_status(
		"usb_hotplug_md", 0, &modem_hotplug_value);
	modem_marker_status = usb_hotplug_event_pair_status(
		modem_unplug_marker_status, modem_unplug_value,
		modem_hotplug_marker_status, modem_hotplug_value,
		&modem_unplug_pending, &modem_hotplug_pending);
	if (modem_marker_status != 0) {
		marker_read_error = modem_marker_status > 0 ?
			modem_marker_status : EIO;
		logmessage("USB hotplug",
			   "cannot read deferred modem event markers; preserving pending events for retry (status=%d)",
			   marker_read_error);
		alarm(5);
	} else {
		if (!modem_unplug_pending)
			modem_unplug_action_done = 0;
		if (!modem_hotplug_pending)
			modem_hotplug_action_done = 0;
		if (modem_unplug_pending) {
			modem_rule_status = storage_read_integer_status(
				"modem_rule", 0, &modem_rule_value);
			modem_rule_status = modem_wan_unplug_event_status(
				modem_rule_status, modem_rule_value, &unplug_modem);
			if (modem_rule_status != 0) {
				/* Keep detach queued and block a coalesced replug. */
				unplug_modem = 1;
				modem_unplug_status = modem_rule_status;
				logmessage("USB hotplug",
					   "cannot read modem WAN policy; deferring unplug transition (status=%d)",
					   modem_rule_status);
			}
		}
		if (modem_unplug_pending && modem_rule_status == 0 && unplug_modem) {
			if (usb_hotplug_event_action_should_run(modem_unplug_pending,
							 modem_unplug_action_done))
				modem_unplug_status = try_wan_reconnect(1, 0);
			else
				modem_unplug_status = 0;
			if (modem_unplug_status != 0) {
				modem_unplug_status = usb_hotplug_event_ack_status(
					modem_unplug_pending, modem_unplug_status, 0,
					&modem_unplug_action_done);
				logmessage("USB hotplug",
					   "WAN reconnect after USB modem unplug failed; keeping event queued (status=%d)",
					   modem_unplug_status);
			} else {
				marker_read_error = storage_ack_event_marker_status("usb_unplug_md");
				modem_unplug_status = usb_hotplug_event_ack_status(
					modem_unplug_pending, 0, marker_read_error,
					&modem_unplug_action_done);
				if (modem_unplug_status != 0) {
					logmessage("USB hotplug",
						   "cannot acknowledge modem unplug after reconnect; keeping event queued (status=%d)",
						   modem_unplug_status);
				} else {
					modem_unplug_pending = 0;
				}
			}
		} else if (modem_unplug_pending && modem_rule_status == 0 && !unplug_modem) {
			/* No WAN transition is required; acknowledge the policy no-op. */
			marker_read_error = storage_ack_event_marker_status("usb_unplug_md");
			if (marker_read_error != 0) {
				modem_unplug_status = marker_read_error;
				logmessage("USB hotplug",
					   "cannot acknowledge modem unplug policy no-op (status=%d)",
					   marker_read_error);
			} else {
				modem_unplug_pending = 0;
			}
		}
		if (modem_hotplug_pending)
			plug_modem = 1;
	}

	if (plug_modem && !unplug_modem && !modem_unplug_pending) {
		if (usb_hotplug_event_action_should_run(modem_hotplug_pending,
						 modem_hotplug_action_done))
			modem_plug_status = try_start_usb_modem_to_wan();
		else
			modem_plug_status = 0;
		if (modem_plug_status != 0) {
			modem_plug_status = usb_hotplug_event_ack_status(
				modem_hotplug_pending, modem_plug_status, 0,
				&modem_hotplug_action_done);
			logmessage("USB hotplug",
				   "USB modem WAN startup failed; keeping event queued (status=%d)",
				   modem_plug_status);
		} else {
			marker_read_error = storage_ack_event_marker_status("usb_hotplug_md");
			modem_plug_status = usb_hotplug_event_ack_status(
				modem_hotplug_pending, 0, marker_read_error,
				&modem_hotplug_action_done);
			if (modem_plug_status != 0) {
				logmessage("USB hotplug",
					   "cannot acknowledge modem replug after startup; keeping event queued (status=%d)",
					   modem_plug_status);
			} else {
				modem_hotplug_pending = 0;
			}
		}
	}

	modem_retry_mask = modem_wan_hotplug_retry_mask(
		unplug_modem || modem_unplug_pending, modem_hotplug_pending,
		modem_unplug_status, modem_plug_status);
	if (modem_retry_mask != 0) {
		logmessage("USB hotplug",
			   "deferring modem WAN hotplug transition for retry (mask=%u)",
			   modem_retry_mask);
		alarm(5);
	}
#endif
}

int
safe_remove_stor_device(int port_b, int port_e, const char *dev_name, int do_spindown)
{
	int storage_lock, retry_notify_status;
	int port, has_mounted_port = 0, has_swapon_port = 0;
	int has_requested_disk = 0;
	int unmount_status = 0, ejected_status = 0, mount_count = 0;
	int start_status = 0, retry_storage_start = 0;
	int stop_status, rollback_status, saved_errno = 0;
	int result = -1;
	disk_info_t *disks_info = NULL, *follow_disk;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		saved_errno = errno ? errno : EBUSY;
		logmessage("Storage", "cannot acquire storage operation lock; refusing safe removal");
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	if (read_disk_data_checked(&disks_info) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot obtain a complete disk inventory; refusing targeted safe removal (errno %d)",
			   saved_errno);
		goto out;
	}

	has_requested_disk = (dev_name == NULL);
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		port = (int)follow_disk->port_root;
		if (port < port_b || port > port_e)
			continue;
		if (storage_ejected_disk_matches_request(dev_name, follow_disk->device))
			has_requested_disk = 1;
		if (follow_disk->mounted_number > 0)
			has_mounted_port = 1;
		if (follow_disk->swapon_number > 0)
			has_swapon_port = 1;
	}

	if (!has_requested_disk) {
		saved_errno = ENODEV;
		logmessage("Storage", "requested disk %s is not present in the complete storage inventory; refusing safe removal",
			   dev_name ? dev_name : "(all)");
		goto out;
	}

	if (has_mounted_port || has_swapon_port) {
		if (has_mounted_port) {
			errno = 0;
			stop_status = stop_stor_apps();
			if (stop_status != 0) {
				saved_errno = errno ? errno : EIO;
				rollback_status = storage_rollback_stor_apps_after_stop_failure(
					"targeted safe removal", stop_status);
				logmessage("Storage",
					   "safe removal aborted because storage services did not stop (stop=%d, rollback=%d)",
					   stop_status, rollback_status);
				goto out;
			}
		}

		/* Stage one inventory for the whole requested port range. */
		errno = 0;
		unmount_status = umount_stor_path_range(disks_info, port_b, port_e,
							1, dev_name, do_spindown);
		if (unmount_status != 0 && saved_errno == 0)
			saved_errno = errno ? errno : EIO;

		if (has_mounted_port) {
			errno = 0;
			ejected_status = umount_ejected_locked(1, 0);
			if (ejected_status != 0 && saved_errno == 0)
				saved_errno = errno ? errno : EIO;
			if (unmount_status != 0 || ejected_status != 0)
				logmessage("Storage", "safe removal incomplete; one or more mount/swap releases failed");

			errno = 0;
			mount_count = count_stor_mountpoint();
			if (mount_count < 0 && saved_errno == 0)
				saved_errno = errno ? errno : EIO;
			if (storage_ejected_may_restart_apps(unmount_status, ejected_status, mount_count)) {
				errno = 0;
				start_status = start_stor_apps();
				if (start_status != 0) {
					if (saved_errno == 0)
						saved_errno = errno ? errno : EIO;
					logmessage("Storage", "storage services failed to restart after safe removal (status=%d)", start_status);
					retry_storage_start = storage_ejected_storage_start_retry_required(
						mount_count, start_status);
				}
#if defined(APP_NFSD)
			} else if (mount_count == 0 && unmount_status == 0 && ejected_status == 0) {
				unload_nfsd();
#endif
			} else if (mount_count < 0 || unmount_status != 0 || ejected_status != 0) {
				logmessage("Storage", "storage applications remain stopped because safe removal did not complete (mounts=%d, unmount=%d, ejected=%d)",
					   mount_count, unmount_status, ejected_status);
			}
		} else if (unmount_status != 0) {
			logmessage("Storage", "safe removal incomplete; active swap could not be disabled or mounted data could not be unmounted");
		}
	}

	result = storage_ejected_target_remove_complete(
		unmount_status, ejected_status, mount_count, start_status) ? 0 : -1;
	if (result != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;

out:
	free_disk_data(disks_info);
	file_unlock(storage_lock);
	if (retry_storage_start) {
		/*
		 * This entry point can run in a short-lived rc applet. Its alarm
		 * cannot drive PID 1's deferred storage callback; enqueue the retry
		 * through the shared notification queue after releasing storage_lock.
		 */
		errno = 0;
		retry_notify_status = notify_rc_status("on_hotplug_mass_storage");
		if (retry_notify_status != 0) {
			int notify_errno = errno ? errno : EIO;

			logmessage("Storage",
				   "cannot queue storage application restart after safe removal (status=%d, errno=%d)",
				   retry_notify_status, notify_errno);
			if (saved_errno == 0)
				saved_errno = notify_errno;
		} else {
			logmessage("Storage",
				   "queued storage application restart with PID 1 after safe removal (start status=%d)",
				   start_status);
		}
	}
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}
	errno = 0;
	return 0;
}

int
safe_remove_all_stor_devices(int do_spindown)
{
	int storage_lock;
	disk_info_t *disks_info = NULL;
	int path_status, fallback_status, mount_count;
	int stop_status, rollback_status, home_status;
	int result = -1, saved_errno = 0;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		saved_errno = errno ? errno : EBUSY;
		logmessage("Storage", "cannot acquire storage operation lock; refusing eject-all operation");
		errno = saved_errno;
		return -1;
	}
	errno = 0;
	if (read_disk_data_checked(&disks_info) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "cannot obtain a complete disk inventory; refusing eject-all operation (errno %d)", saved_errno);
		goto out;
	}
	stop_status = stop_stor_apps();
	if (stop_status != 0) {
		saved_errno = errno ? errno : EIO;
		rollback_status = storage_rollback_stor_apps_after_stop_failure(
			"eject-all", stop_status);
		logmessage("Storage",
			   "eject-all aborted because storage services did not stop (stop=%d, rollback=%d)",
			   stop_status, rollback_status);
		goto out;
	}

	/* A complete empty inventory still needs the generic mounts fallback. */
	errno = 0;
	path_status = disks_info ? umount_stor_path(disks_info, 0, NULL, do_spindown) : 0;
	if (path_status != 0)
		saved_errno = errno ? errno : EIO;
	free_disk_data(disks_info);
	disks_info = NULL;
	if (path_status == 0) {
		errno = 0;
		fallback_status = umount_all_storage();
		if (fallback_status != 0) {
			if (saved_errno == 0)
				saved_errno = errno ? errno : EIO;
		}
	} else {
		/* Do not bypass a failed Optware stop hook with generic unmount. */
		fallback_status = -1;
		logmessage("Storage", "per-device unmount was incomplete; refusing fallback unmount");
	}

	errno = 0;
	mount_count = count_stor_mountpoint();
	result = storage_safe_remove_complete(path_status, fallback_status, mount_count);
	if (result != 0) {
		if (saved_errno == 0)
			saved_errno = errno ? errno : EIO;
		logmessage("Storage", "eject incomplete/unknown (unmount=%d fallback=%d mounts=%d); refusing storage persistence",
			   path_status, fallback_status, mount_count);
	}
#if defined(APP_NFSD)
	if (result == 0)
		unload_nfsd();
#endif
	/* Home-link restoration failure must not skip independent NFS teardown. */
	home_status = restore_home_dir();
	if (home_status != 0) {
		if (saved_errno == 0)
			saved_errno = home_status;
		if (result == 0)
			result = -1;
	}
out:
	free_disk_data(disks_info);
	file_unlock(storage_lock);
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	else
		errno = 0;
	return result;
}

