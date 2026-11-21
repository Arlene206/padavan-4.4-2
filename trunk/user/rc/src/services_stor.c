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
#include "nvram_linux.h"
#include "service_start_status.h"
#include "modem_wan_start_policy.h"
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
#include "ftp_config_file.h"
#include "ftp_daemon_policy.h"
#include "storage_mount_inventory_policy.h"
#include "storage_optware_start_policy.h"
#include "storage_ejected_inventory_policy.h"
#include "storage_nfs_export_policy.h"
#include "storage_service_stop_policy.h"
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

	if (!sd_dev)
		return;

	if (strncmp(sd_dev, "sd", 2) != 0)
		return;

	argv[0] = "/sbin/spindown.sh";
	argv[1] = sd_dev;
	argv[2] = NULL;
	_eval(argv, NULL, 0, NULL);
}

static int storage_optware_result_fd = -1;

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
stop_ftp_locked(void)
{
	char* svcs[] = { "vsftpd", NULL };
	int status;

	status = kill_services(svcs, 3, 1);
	if (storage_service_stop_complete(status, is_ftp_run()) != 0) {
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
	stop_status = stop_ftp_locked();
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
	int storage_lock, status;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock < 0) {
		logmessage("FTP server", "cannot acquire storage operation lock; refusing standalone FTP stop");
		return -1;
	}

	status = stop_ftp_locked();
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


int
write_vsftpd_conf(void)
{
	FILE *fp = NULL;
	char staged_path[64];
	int i_maxuser = 0, max_user_value = 0;
	int ftp_mode_value = 0, ftp_log_value = 0;
	int max_user_status, ftp_mode_status, ftp_log_status;
	int config_status, saved_errno, i_ftp_mode;
	const char *banner_product;

	ftp_mode_status = storage_read_integer_status(
		"st_ftp_mode", 0, &ftp_mode_value);
	ftp_log_status = storage_read_integer_status(
		"st_ftp_log", 0, &ftp_log_value);
	max_user_status = storage_read_integer_status(
		"st_max_user", 0, &max_user_value);
	max_user_status = service_connection_limit_value_status(
		max_user_status, max_user_value, MAX_CLIENTS_NUM, &i_maxuser);
	config_status = ftp_mode_status != 0 ? ftp_mode_status :
		ftp_log_status != 0 ? ftp_log_status : max_user_status;
	if (config_status != 0) {
		saved_errno = config_status > 0 ? config_status : EIO;
		logmessage("FTP server",
			   "cannot read FTP configuration; preserving active configuration (errno %d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	if (ftp_config_file_open("/etc/vsftpd.conf", staged_path,
				 sizeof(staged_path), &fp) != 0)
		return -1;

	banner_product = nvram_safe_get("productid");
	if (!samba_conf_text_is_safe(banner_product) || !*banner_product)
		banner_product = "Padavan";

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED) {
		fprintf(fp, "listen_ipv6=%s\n", "YES");
		fprintf(fp, "listen=%s\n", "NO");
	} else
#endif
		fprintf(fp, "listen=%s\n", "YES");

	fprintf(fp, "background=%s\n", "YES");
	fprintf(fp, "connect_from_port_20=%s\n", "NO");
	fprintf(fp, "pasv_enable=%s\n", "YES");
	fprintf(fp, "pasv_min_port=%d\n", nvram_safe_get_int("st_ftp_pmin", 50000, 1, 65535));
	fprintf(fp, "pasv_max_port=%d\n", nvram_safe_get_int("st_ftp_pmax", 50100, 1, 65535));
	fprintf(fp, "use_sendfile=%s\n", "YES");
#if defined (SUPPORT_FTPD_SSL)
	fprintf(fp, "ssl_enable=%s\n", "NO");
#endif

	i_ftp_mode = ftp_share_mode_normalize(ftp_mode_value);
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
		fprintf(fp, "anon_max_rate=%u\n", nvram_safe_get_int("st_ftp_anmr", 0, 0, 1000*10) * 1024);

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

	if (storage_service_process_presence_changed(is_run_before, is_run_after) &&
	    (is_run_before == PROCESS_PRESENCE_UNKNOWN ||
	     is_run_after == PROCESS_PRESENCE_UNKNOWN ||
	     (is_run_before == PROCESS_PRESENCE_ABSENT &&
	      is_run_after == PROCESS_PRESENCE_RUNNING)) &&
	    nvram_match("ftpd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		return restart_firewall();

	return 0;
}

int
restart_ftpd(void)
{
	int storage_lock, is_run_before;
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
	stop_status = stop_ftp_locked();
	if (stop_status != 0) {
		logmessage("FTP server", "restart aborted because existing daemon did not stop");
		restart_status = stop_status;
		goto out;
	}

	mount_status = storage_has_mounted_devices("restarting FTP");
	if (mount_status < 0) {
		logmessage("FTP server", "restart aborted because mounted storage state is unknown");
		goto out;
	}
	if (mount_status > 0) {
		start_status = run_ftp();
		if (start_status != 0) {
			restart_status = start_status;
			goto out;
		}
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
samba_service_account_is_safe(const char *name)
{
	return samba_acl_user_is_safe(name) &&
	       !router_account_name_is_reserved(name, nvram_safe_get("http_username"));
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
write_smb_conf_unlocked(void)
{
	FILE *fp = NULL;
	char staged_path[64];
	int i_maxuser = 0, i_maxuser_value = 0, i_maxuser_status;
	int i_smb_mode, result = 0;
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;

	if (samba_read_config_mode_status(&i_smb_mode) != 0) {
		int mode_error = errno ? errno : EIO;

		logmessage("Samba Server",
			   "cannot read a valid Samba share mode; preserving active configuration (errno %d)",
			   mode_error);
		errno = mode_error;
		return -1;
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

	disks_info = read_disk_data();
	if (!disks_info) {
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
		int n, acc_num = 0, sh_num=0;
		char **account_list;
		
		// get the account list
		if (get_account_list(&acc_num, &account_list) < 0) {
			free_2_dimension_list(&acc_num, &account_list);
			result = -1;
			goto confpage;
		}
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					free_2_dimension_list(&sh_num, &folder_list);
					free_2_dimension_list(&acc_num, &account_list);
					result = -1;
					goto confpage;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first, share_exists;
					int guest_right;
					char share[256];

					if (!samba_mount_path_is_safe(follow_partition->mount_point) ||
					    !samba_share_name_is_safe(folder_list[n])) {
						logmessage("Samba Server", "skip share with unsafe storage metadata");
						continue;
					}
					guest_right = get_permission(SMB_GUEST_USER, follow_partition->mount_point, folder_list[n], "cifs");
					snprintf(share, sizeof(share), "%s", folder_list[n]);
					
					if (fclose(fp) != 0) {
						fp = NULL;
						free_2_dimension_list(&sh_num, &folder_list);
						free_2_dimension_list(&acc_num, &account_list);
						result = -1;
						goto confpage;
					}
					fp = NULL;

					share_exists = check_existed_share(share, staged_path);
					if (share_exists < 0) {
						free_2_dimension_list(&sh_num, &folder_list);
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
							free_2_dimension_list(&acc_num, &account_list);
							result = -1;
							goto confpage;
						}
					}

					fp = fopen(staged_path, "a");
					if (fp == NULL) {
						free_2_dimension_list(&sh_num, &folder_list);
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
						if (!samba_service_account_is_safe(account_list[i]))
							continue;
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
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
						if (!samba_service_account_is_safe(account_list[i]))
							continue;
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
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
						if (!samba_service_account_is_safe(account_list[i]))
							continue;
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
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
						if (!samba_service_account_is_safe(account_list[i]))
							continue;
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
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
		
		free_2_dimension_list(&acc_num, &account_list);
	}

confpage:
	if (result != 0) {
		samba_config_file_discard(&fp, staged_path);
		free_disk_data(disks_info);
		return -1;
	}
	if (samba_config_file_publish(&fp, staged_path, SAMBA_CONF) != 0) {
		free_disk_data(disks_info);
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

	result = write_smb_conf_unlocked();
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
	int stop_status, wins_enabled = 0;

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
	fput_int("/proc/net/netfilter/nf_fp_smb", 0);

	if (stop_status != 0) {
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
		return 0;
	if (samba_require_daemon_absent("nmbd", "after stop") != 0)
		return -1;

	clean_smbd_trash();
	return 0;
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
	int lock_fd, sh_num, i, j;
	int credential_failed = 0, daemon_failed = 0, result = -1;
	int runtime_enabled;
	char tmpuser[40], tmp2[40];
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

	/* Inventory every daemon before account, config or cleanup mutation. */
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

	recreate_passwd_unix(0);

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

	sh_num = nvram_safe_get_int("acc_num", 0, 0, MAX_ACCOUNT_NUM);
	for (i = 0; i < sh_num; i++) {
		snprintf(tmpuser, sizeof(tmpuser), "acc_username%d", i);
		snprintf(tmp2, sizeof(tmp2), "acc_password%d", i);
		if (!samba_service_account_is_safe(nvram_safe_get(tmpuser)))
			continue;

		/* Reject duplicate NVRAM identities instead of writing ambiguous rows. */
		for (j = 0; j < i; j++) {
			char previous_user[40];

			snprintf(previous_user, sizeof(previous_user), "acc_username%d", j);
			if (!strcmp(nvram_safe_get(tmpuser), nvram_safe_get(previous_user))) {
				credential_failed = 1;
				break;
			}
		}
		if (credential_failed)
			break;

		if (!samba_passdb_password_is_safe(nvram_safe_get(tmp2)) ||
		    samba_run_smbpasswd(
#if defined (APP_SMBD36)
			staged_passdb,
#else
			NULL,
#endif
			nvram_safe_get(tmpuser), nvram_safe_get(tmp2)) != 0) {
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

	if (write_smb_conf_unlocked() != 0) {
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
	if (write_smb_conf_unlocked() != 0) {
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
write_nfsd_exports(void)
{
	FILE *procpt = NULL, *fp = NULL;
	struct stat exports_link_info, exports_target_info;
	char line[512], devname[32], mpname[256], fstype[32], fsmode[256];
	char acl_lan[32], acl_vpn[32], staged_path[] = "/etc/.exports.XXXXXX";
	const char *exports_link = "/etc/storage/exports";
	const char *exports_file = "/etc/exports";
	const char *exports_rule = STORAGE_NFS_EXPORT_DEFAULT_OPTIONS;
	char *nfsmm, *acl_addr, *acl_mask;
	int fd = -1, read_error, close_error, parse_error = 0;
	int close_status, output_error = 0, saved_errno = 0;
	int exports_link_lstat_status, exports_link_stat_status = -1;
	int exports_link_errno = 0, exports_target_errno = 0;
	int exports_override_state;
	int acl_ipv4_status, acl_vpn_status = 0;
#if defined (USE_IPV6)
	int ipv6_type, acl_ipv6_status = 0;
	char *acl_addr6 = NULL, *acl_len6 = NULL;
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

	acl_addr = nvram_safe_get("lan_ipaddr_t");
	acl_mask = nvram_safe_get("lan_netmask_t");
	if (!is_valid_ipv4(acl_addr) || !is_valid_ipv4(acl_mask)) {
		acl_addr = nvram_safe_get("lan_ipaddr");
		acl_mask = nvram_safe_get("lan_netmask");
	}

	acl_lan[0] = 0;
	acl_ipv4_status = storage_nfs_export_format_ipv4_acl(
		acl_addr, acl_mask, acl_lan, sizeof(acl_lan));
	if (acl_ipv4_status < 0) {
		saved_errno = EINVAL;
		logmessage("Storage", "refusing to publish NFS exports with an invalid IPv4 LAN ACL");
		fclose(fp);
		fclose(procpt);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}

#if defined (USE_IPV6)
	ipv6_type = get_ipv6_type();
	if (ipv6_type != IPV6_DISABLED) {
		acl_addr6 = nvram_safe_get("ip6_lan_addr");
		acl_len6 = nvram_safe_get("ip6_lan_size");
		acl_ipv6_status = storage_nfs_export_format_ipv6_acl(
			acl_addr6, acl_len6, acl_addr6_export,
			sizeof(acl_addr6_export));
		if (acl_ipv6_status < 0) {
			saved_errno = EINVAL;
			logmessage("Storage", "refusing to publish NFS exports with an invalid IPv6 LAN ACL");
			fclose(fp);
			fclose(procpt);
			unlink(staged_path);
			errno = saved_errno;
			return -1;
		}
	}
#endif

	acl_vpn[0] = 0;
	if (!get_ap_mode() && nvram_get_int("vpns_enable") && nvram_get_int("vpns_vuse")) {
		acl_addr = nvram_safe_get("vpns_vnet");
		acl_mask = VPN_SERVER_SUBNET_MASK;
#if defined (APP_OPENVPN)
		if (nvram_get_int("vpns_type") == 2) {
			if (nvram_get_int("vpns_ov_mode") == 1)
				acl_vpn_status = storage_nfs_export_format_ipv4_acl(
					acl_addr, acl_mask, acl_vpn, sizeof(acl_vpn));
		} else
#endif
			acl_vpn_status = storage_nfs_export_format_ipv4_acl(
				acl_addr, acl_mask, acl_vpn, sizeof(acl_vpn));

		if (acl_vpn_status < 0) {
			saved_errno = EINVAL;
			logmessage("Storage", "refusing to publish NFS exports with an invalid IPv4 VPN ACL");
			fclose(fp);
			fclose(procpt);
			unlink(staged_path);
			errno = saved_errno;
			return -1;
		}
		if (strcmp(acl_lan, acl_vpn) == 0)
			acl_vpn[0] = 0;
	}

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

	return storage_daemon_presence("minidlna", NULL);
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

int update_minidlna_conf(const char *link_path, const char *conf_path)
{
	struct minidlna_config_values values;
	const char *computer_name;
	int dlna_disc, dlna_root, status;

	computer_name = get_our_hostname();
	if (computer_name == NULL)
		return EIO;

	dlna_disc = nvram_get_int("dlna_disc");
	dlna_root = nvram_get_int("dlna_root");
	values.media_source1 = nvram_safe_get("dlna_src1");
	values.media_source2 = nvram_safe_get("dlna_src2");
	values.media_source3 = nvram_safe_get("dlna_src3");
	if (!*values.media_source1 && !*values.media_source2 &&
	    !*values.media_source3)
		values.media_source1 = "/media";

	if (dlna_disc < 10)
		dlna_disc = 10;
	if (dlna_disc > 10800)
		dlna_disc = 10800;

	values.interface_name = IFNAME_BR;
	values.notify_interval = dlna_disc;
	values.root_container = dlna_root;
	values.force_sort = nvram_get_int("dlna_sort");
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
	int db_rescan_mode;
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

	if (!nvram_match("apps_dms", "1"))
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

	ether_atoe(nvram_safe_get("il0macaddr"), mac_bin);
	minidlna_argv[4] = ether_etoa3(mac_bin, mac_str);

	db_rescan_mode = nvram_get_int("dlna_rescan");
	if (force_rescan || db_rescan_mode == 2)
		minidlna_argv[5] = "-R";
	else if (db_rescan_mode == 1)
		minidlna_argv[5] = "-U";

	errno = 0;
	launch_status = _eval(minidlna_argv, NULL, 0, NULL);
	launch_errno = errno;
	process_error = 0;
	if (launch_status == 0) {
		if (storage_wait_for_daemon("minidlna") == 0)
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
			apps_name, "minidlna", start_status, start_errno,
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
update_firefly_conf(const char *link_path, const char *conf_path,
		    const char *conf_file)
{
	char conf_full[128];
	int path_length;

	path_length = snprintf(conf_full, sizeof(conf_full), "%s/%s",
			       conf_path, conf_file);
	if (path_length < 0 || (size_t)path_length >= sizeof(conf_full))
		return ENAMETOOLONG;

	return firefly_config_publish(conf_full, conf_full, link_path,
				      nvram_safe_get("http_username"),
				      nvram_safe_get("http_passwd"));
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

	if (!nvram_match("apps_itunes", "1"))
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

	if (!nvram_match("trmd_enable", "1"))
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
	if (storage_service_process_presence_changed(is_run_before, is_run_after) &&
	    nvram_match("fw_enable_x", "1")) {
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

	if (!nvram_match("aria_enable", "1"))
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
	if (storage_service_process_presence_changed(is_run_before, is_run_after) &&
	    nvram_match("fw_enable_x", "1")) {
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
	status = stop_ftp_locked();
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

static void
restore_home_dir(void)
{
	if (!check_if_dir_exist("/home/admin")) {
		unlink("/home/admin");
		symlink("/home/root", "/home/admin");
	}
}

/* Retry failed physical-unplug cleanup without entering the app-start path. */
static void
storage_schedule_ejected_cleanup_retry(void)
{
	nvram_set_int_temp("usb_unplug_ms", 1);
	alarm(5);
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
	int mount_count, start_status;
	struct storage_ejected_mount_entry *head, *tail, *entry;
	struct storage_ejected_partition_entry *partitions = NULL;
	struct storage_ejected_swap_entry *swaps = NULL, *swap_entry;

	head = tail = NULL;
	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (!procpt) {
		saved_errno = errno;
		logmessage("Storage", "cannot read %s for ejected-device cleanup (errno %d)",
			   PROC_MOUNTS_FILE, saved_errno);
		restore_home_dir();
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
		free_storage_ejected_mounts(head);
		restore_home_dir();
		return -1;
	}

	if (storage_stage_ejected_swaps(&swaps) != 0) {
		free_storage_ejected_mounts(head);
		restore_home_dir();
		return -1;
	}
	if (!head && !swaps) {
		free_storage_ejected_swaps(swaps);
		restore_home_dir();
		return 0;
	}
	if (storage_stage_ejected_partitions(&partitions) != 0) {
		free_storage_ejected_mounts(head);
		free_storage_ejected_swaps(swaps);
		restore_home_dir();
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
		restore_home_dir();
		return 0;
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
			restore_home_dir();
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

	restore_home_dir();
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
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= is_ftp_run();
	if (stop_ftp_locked() != 0) {
		logmessage("Storage", "FTP server did not stop cleanly; storage removal must be aborted");
		stop_status = -1;
	}
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

	if (need_restart_fw && nvram_match("fw_enable_x", "1")) {
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
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= (is_ftp_run() == PROCESS_PRESENCE_RUNNING);
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
	need_restart_fw |= (is_torrent_run() == PROCESS_PRESENCE_RUNNING);
#endif
#if defined (APP_ARIA)
	service_status = run_aria();
	start_status = rc_first_service_start_status(start_status, service_status);
	need_restart_fw |= (is_aria_run() == PROCESS_PRESENCE_RUNNING);
#endif

	if (need_restart_fw && nvram_match("fw_enable_x", "1")) {
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
	int need_optware = 0;
	int optware_status, optware_error;
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
	if (child_result == STORAGE_OPTWARE_START_CHILD_RUNNING) {
		file_unlock(storage_lock);
		return 1;
	}

	if (mounted_devices == 0) {
		/* A complete empty inventory makes any old hotplug request stale. */
		nvram_set_int_temp("usb_hotplug_ms", 0);
		nvram_set_int_temp("usb_opt_start", 0);
		file_unlock(storage_lock);
		return 0;
	}

	/*
	 * Keep usb_hotplug_ms pending when a required integrated daemon fails.
	 * The normal deferred alarm will retry it; only a successful start
	 * acknowledges the event. Optware runs after the storage lock is released.
	 */
	if (nvram_match("usb_hotplug_ms", "1")) {
		integrated_start_status = start_stor_apps();
		if (rc_service_start_event_is_complete(integrated_start_status)) {
			nvram_set_int_temp("usb_hotplug_ms", 0);
		} else {
			logmessage("Storage", "integrated storage application startup failed; preserving USB storage startup event for retry (status=%d)",
				   integrated_start_status);
		}
	}

	/* External init scripts must never inherit storage.lock. */
	file_unlock(storage_lock);

	if (child_result == STORAGE_OPTWARE_START_CHILD_SUCCESS) {
		if (need_optware)
			nvram_set_int_temp("usb_opt_start", 0);
		return integrated_start_status;
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
	unsigned int modem_retry_mask;
#endif
	int storage_start_result;
#if defined (USE_STORAGE)
	int storage_unplug_status;
#endif

#if defined (USE_STORAGE)
	/*
	 * Finish any stale physical-unplug mounts before trying to start
	 * storage-backed daemons. Failed cleanup re-arms usb_unplug_ms.
	 */
	if (nvram_match("usb_unplug_ms", "1")) {
		nvram_set_int_temp("usb_unplug_ms", 0);
		storage_unplug_status = umount_ejected();
		if (storage_unplug_status != 0)
			logmessage("Storage", "deferred physical-unplug cleanup retry failed (status=%d)",
				   storage_unplug_status);
	}
#endif

	if (
#if defined (USE_STORAGE)
	    !nvram_match("usb_unplug_ms", "1") &&
#endif
	    (nvram_match("usb_hotplug_ms", "1") ||
	     nvram_match("usb_opt_start", "1") ||
	     storage_optware_result_fd >= 0)) {
		storage_start_result = try_start_stor_apps();
		if (storage_start_result != 0)
			alarm(5);
	}

#if defined (USE_USB_SUPPORT)
	if (nvram_match("usb_unplug_lp", "1"))
	{
		nvram_set_int_temp("usb_unplug_lp", 0);
		if (!usb_port_module_used("usblp"))
			stop_usb_printer_spoolers();
	}

	if (nvram_match("usb_hotplug_lp", "1"))
	{
		nvram_set_int_temp("usb_hotplug_lp", 0);
		printer_start_status = try_start_usb_printer_spoolers();
		if (printer_start_status != 0) {
			logmessage("USB Printer",
				   "deferred printer startup failed; preserving pending event for retry (status=%d)",
				   printer_start_status);
			nvram_set_int_temp("usb_hotplug_lp", 1);
			alarm(5);
		}
	}

	if (nvram_match("usb_unplug_md", "1"))
	{
		modem_rule_status = storage_read_integer_status(
			"modem_rule", 0, &modem_rule_value);
		modem_rule_status = modem_wan_unplug_event_status(
			modem_rule_status, modem_rule_value, &unplug_modem);
		if (modem_rule_status == 0) {
			/* Only acknowledge detach after the modem policy was read. */
			nvram_set_int_temp("usb_unplug_md", 0);
		} else {
			/* Keep detach queued and block a coalesced replug until retry. */
			unplug_modem = 1;
			modem_unplug_status = modem_rule_status;
			logmessage("USB hotplug",
				   "cannot read modem WAN policy; deferring unplug transition (status=%d)",
				   modem_rule_status);
		}
	}

	if (nvram_match("usb_hotplug_md", "1"))
	{
		plug_modem = 1;
		/*
		 * If detach and re-enumeration arrived together, leave the plug event
		 * pending until the unplug reconnect has completed successfully.
		 */
		if (!unplug_modem)
			nvram_set_int_temp("usb_hotplug_md", 0);
	}

	if (unplug_modem) {
		if (modem_rule_status == 0) {
			modem_unplug_status = try_wan_reconnect(1, 0);
			if (modem_unplug_status != 0)
				logmessage("USB hotplug",
					   "WAN reconnect after USB modem unplug failed (status=%d)",
					   modem_unplug_status);
	
		}
	}
	if (plug_modem && !unplug_modem) {
		modem_plug_status = try_start_usb_modem_to_wan();
		if (modem_plug_status != 0)
			logmessage("USB hotplug",
				   "USB modem WAN startup failed (status=%d)",
				   modem_plug_status);
	}

	modem_retry_mask = modem_wan_hotplug_retry_mask(unplug_modem, plug_modem,
						 modem_unplug_status, modem_plug_status);
	if (modem_retry_mask & MODEM_WAN_HOTPLUG_RETRY_UNPLUG)
		nvram_set_int_temp("usb_unplug_md", 1);
	if (modem_retry_mask & MODEM_WAN_HOTPLUG_RETRY_PLUG)
		nvram_set_int_temp("usb_hotplug_md", 1);
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
	int storage_lock;
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
		nvram_set_int_temp("usb_hotplug_ms", 1);
		logmessage("Storage", "queued storage application retry after safe removal; retrying in five seconds");
		alarm(5);
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
	int stop_status, rollback_status;
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
	restore_home_dir();
#if defined(APP_NFSD)
	if (result == 0)
		unload_nfsd();
#endif
out:
	free_disk_data(disks_info);
	file_unlock(storage_lock);
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	else
		errno = 0;
	return result;
}

