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
#include <limits.h>
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#include <dev_info.h>
#include <usb_info.h>
#include <process_inventory.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "service_start_status.h"
#include "detached_daemon_start_policy.h"
#include "service_wait_policy.h"
#include "switch.h"
#include "gpio_pins.h"
#include "storage_hotplug_name_policy.h"
#include "modem_wan_start_policy.h"
#include "usb_printer_nvram_policy.h"
#include "usb_printer_p910nd_policy.h"
#include "usb_printer_child_reap_policy.h"
#include "usb_printer_executable_policy.h"
#include "service_lifecycle_policy.h"

static int
usb_printer_nvram_get_checked(void *context, const char *name, char **value)
{
    (void)context;
    return nvram_get_checked(name, value);
}

static int
read_usb_printer_setting_status(const char *name, char *value, size_t size)
{
    return usb_printer_nvram_copy_setting_status(
        usb_printer_nvram_get_checked, NULL, name, value, size);
}

int
safe_remove_usb_device(int port, const char *dev_name)
{
	unsigned int parsed_devnum;
	int modem_devnum = 0;

	if (port < 0 || port > BOARD_NUM_USB_PORTS) {
		errno = EINVAL;
		return -1;
	}
	if (port == 0) {
		if (dev_name && strncmp(dev_name, "sd", 2) != 0) {
			errno = EINVAL;
			return -1;
		}
		return safe_remove_stor_device(1, BOARD_NUM_USB_PORTS, dev_name, 1);
	}

	if (dev_name && strncmp(dev_name, "sd", 2) != 0) {
		if (storage_hotplug_parse_decimal_token(dev_name, 1, INT_MAX,
							&parsed_devnum) != 0) {
			errno = EINVAL;
			return -1;
		}
		modem_devnum = (int)parsed_devnum;
	}

	if (modem_devnum) {
		usb_info_t *usb_info, *follow_usb;
		int has_modem_port = 0, modem_remove_status, wan_reconnect_status;
		int modem_identity_status, is_selected_modem = 0;

		usb_info = get_usb_info();
		for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
			if ((follow_usb->port_root == port) &&
			    (follow_usb->id_devnum == modem_devnum) &&
			    (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
			     follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH))
				has_modem_port = 1;
		}
		free_usb_info(usb_info);

		if (!has_modem_port) {
			errno = ENODEV;
			return -1;
		}

		modem_identity_status = get_usb_modem_dev_wan_status(
			0, modem_devnum, &is_selected_modem);
		if (modem_identity_status != 0) {
			errno = modem_identity_status;
			logmessage(LOGNAME,
				   "cannot verify selected USB modem identity before eject (status=%d)",
				   modem_identity_status);
			return -1;
		}
		if (!is_selected_modem) {
			errno = ENODEV;
			return -1;
		}

		modem_remove_status = safe_remove_usb_modem();
		if (modem_remove_status != 0) {
			if (modem_remove_status > 0)
				errno = modem_remove_status;
			else if (!errno)
				errno = EIO;
			logmessage(LOGNAME,
				   "skipping WAN reconnect after incomplete USB modem removal (status=%d)",
				   modem_remove_status);
			return -1;
		}
		errno = 0;
		wan_reconnect_status = try_wan_reconnect(0, 0);
		if (wan_reconnect_status != 0) {
			if (wan_reconnect_status > 0)
				errno = wan_reconnect_status;
			else if (!errno)
				errno = EIO;
			logmessage(LOGNAME,
				   "USB modem was removed but WAN reconnect failed (status=%d, errno=%d)",
				   wan_reconnect_status, errno);
			return -1;
		}
		errno = 0;
		return 0;
	}

	return safe_remove_stor_device(port, port, dev_name, 1);
}

/* A successful exec is only a launch acknowledgement; the daemon may exit
 * immediately after parsing its configuration or binding its socket. */
static int
usb_printer_daemon_identity_executable_matches(
	const char *process_name, const struct process_identity *identity,
	const char *expected_executable, int *check_error)
{
	char proc_path[64], executable_path[512];
	int path_length, saved_error, status;
	ssize_t length;
	enum process_presence identity_state;

	if (!process_name || !process_name[0] || !identity ||
	    !expected_executable || !expected_executable[0] || !check_error) {
		errno = EINVAL;
		return -1;
	}
	*check_error = 0;
	path_length = snprintf(proc_path, sizeof(proc_path), "/proc/%ld/exe",
			       (long)identity->pid);
	if (path_length < 0 || path_length >= (int)sizeof(proc_path)) {
		*check_error = ENAMETOOLONG;
		errno = *check_error;
		return -1;
	}

	errno = 0;
	length = readlink(proc_path, executable_path, sizeof(executable_path) - 1);
	if (length < 0) {
		saved_error = errno > 0 ? errno : EIO;
		identity_state = process_identity_validate(process_name, identity);
		if (identity_state == PROCESS_PRESENCE_ABSENT)
			return 0;
		*check_error = identity_state == PROCESS_PRESENCE_UNKNOWN ?
			(errno > 0 ? errno : saved_error) : saved_error;
		errno = *check_error;
		return -1;
	}
	if ((size_t)length >= sizeof(executable_path) - 1) {
		*check_error = ENAMETOOLONG;
		errno = *check_error;
		return -1;
	}
	executable_path[length] = '\0';

	/* Revalidate PID/start-time and argv identity after following /proc/PID/exe. */
	identity_state = process_identity_validate(process_name, identity);
	if (identity_state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (identity_state != PROCESS_PRESENCE_RUNNING) {
		*check_error = errno > 0 ? errno : EIO;
		errno = *check_error;
		return -1;
	}

	status = usb_printer_executable_path_matches(expected_executable,
						      executable_path);
	if (status < 0) {
		*check_error = errno > 0 ? errno : EINVAL;
		errno = *check_error;
		return -1;
	}
	return status;
}

/* argv[0] is user-selectable at execve(); bind these dedicated printer daemons
 * to their ROMFS executable so an unrelated process cannot suppress startup. */
static enum process_presence
usb_printer_daemon_scan(void *context, int *scan_error)
{
	const char *process_name = (const char *)context;
	const char *expected_executable;
	struct process_snapshot snapshot;
	enum process_presence presence;
	int saved_error, check_error, unknown_error = 0, match, found = 0;

	if (!process_name || !process_name[0]) {
		if (scan_error)
			*scan_error = EINVAL;
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	expected_executable = usb_printer_executable_path(process_name);
	if (!expected_executable) {
		saved_error = errno > 0 ? errno : EINVAL;
		if (scan_error)
			*scan_error = saved_error;
		errno = saved_error;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	presence = process_inventory_scan(process_name, &snapshot);
	saved_error = snapshot.error > 0 ? snapshot.error : EIO;
	if (presence == PROCESS_PRESENCE_RUNNING) {
		size_t i;

		if (snapshot.count == 0) {
			presence = PROCESS_PRESENCE_UNKNOWN;
			saved_error = EIO;
		} else {
			for (i = 0; i < snapshot.count; i++) {
				check_error = 0;
				match = usb_printer_daemon_identity_executable_matches(
					process_name, &snapshot.items[i],
					expected_executable, &check_error);
				if (match > 0) {
					found = 1;
					break;
				}
				if (match < 0 && unknown_error == 0)
					unknown_error = check_error > 0 ?
						check_error : EIO;
			}
			if (found)
				presence = PROCESS_PRESENCE_RUNNING;
			else if (unknown_error != 0) {
				presence = PROCESS_PRESENCE_UNKNOWN;
				saved_error = unknown_error;
			} else
				presence = PROCESS_PRESENCE_ABSENT;
		}
	} else if (presence != PROCESS_PRESENCE_ABSENT &&
		   presence != PROCESS_PRESENCE_UNKNOWN) {
		presence = PROCESS_PRESENCE_UNKNOWN;
		saved_error = EIO;
	}
	process_snapshot_release(&snapshot);
	if (scan_error)
		*scan_error = presence == PROCESS_PRESENCE_UNKNOWN ?
			saved_error : 0;
	if (presence == PROCESS_PRESENCE_UNKNOWN)
		errno = saved_error;

	return presence;
}

static int
usb_printer_service_identity_matches(void *context,
				     const struct process_identity *identity,
				     int *check_error)
{
	const char *process_name = (const char *)context;
	const char *expected_executable;

	expected_executable = usb_printer_executable_path(process_name);
	if (!expected_executable) {
		*check_error = errno > 0 ? errno : EINVAL;
		return -1;
	}
	return usb_printer_daemon_identity_executable_matches(
		process_name, identity, expected_executable, check_error);
}

static enum process_presence
usb_printer_service_scan(void *context, const char *process_name,
			 struct process_snapshot *snapshot)
{
	struct process_snapshot candidates;
	enum process_presence presence;

	(void)context;
	if (!snapshot || !process_name || !process_name[0]) {
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	presence = process_inventory_scan(process_name, &candidates);
	presence = usb_printer_filter_process_snapshot(
		&candidates, snapshot, usb_printer_service_identity_matches,
		(void *)process_name);
	process_snapshot_release(&candidates);
	if (presence == PROCESS_PRESENCE_UNKNOWN)
		errno = snapshot->error > 0 ? snapshot->error : EIO;
	return presence;
}

static enum process_presence
usb_printer_service_validate(void *context, const char *process_name,
			     const struct process_identity *identity)
{
	int check_error = 0;
	int matched;

	(void)context;
	matched = usb_printer_service_identity_matches((void *)process_name,
						       identity, &check_error);
	if (matched > 0)
		return PROCESS_PRESENCE_RUNNING;
	if (matched == 0)
		return PROCESS_PRESENCE_ABSENT;
	errno = check_error > 0 ? check_error : EIO;
	return PROCESS_PRESENCE_UNKNOWN;
}

static int
usb_printer_service_send_signal(void *context,
				const struct process_identity *identity,
				int signal_number)
{
	(void)context;
	if (signal_number != SIGTERM && signal_number != SIGKILL) {
		errno = EINVAL;
		return -1;
	}
	if (kill(identity->pid, signal_number) == 0 || errno == ESRCH)
		return 0;
	return -1;
}

static int
usb_printer_service_sleep(void *context, unsigned int seconds)
{
	(void)context;
	return service_wait_seconds(seconds);
}

static int
usb_printer_kill_services(char *service_names[], int wait_timeout,
			  int forcekill)
{
	static const struct service_lifecycle_ops ops = {
		usb_printer_service_scan,
		usb_printer_service_validate,
		usb_printer_service_send_signal,
		usb_printer_service_sleep,
		NULL
	};
	int result;

	result = service_lifecycle_stop(service_names, wait_timeout, forcekill,
					&ops, NULL);
	if (result != SERVICE_LIFECYCLE_STOPPED)
		logmessage(LOGNAME,
			   "executable-bound USB printer stop incomplete (status=%d)",
			   result);
	return result;
}

static int
usb_printer_daemon_wait(void *context, unsigned int seconds)
{
	(void)context;
	return service_wait_seconds(seconds);
}

static int
usb_printer_daemon_postcondition(const char *process_name)
{
	int status;

	status = detached_daemon_start_postcondition_with_ops(
		0, usb_printer_daemon_scan, usb_printer_daemon_wait,
		(void *)process_name,
		PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS,
		PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS,
		PROCESS_PRESENCE_START_PROBE_ATTEMPTS);
	if (status != 0) {
		if (status < 0)
			status = errno > 0 ? errno : EIO;
		errno = status;
		logmessage(LOGNAME,
			   "%s exec succeeded but daemon readiness was not confirmed (status=%d)",
			   process_name, status);
	}
	return status;
}

static int
usb_printer_daemon_rollback_stop(void *context)
{
	char *service_names[2];

	if (!context) {
		errno = EINVAL;
		return EINVAL;
	}
	service_names[0] = (char *)context;
	service_names[1] = NULL;
	return usb_printer_kill_services(service_names, 3, 1);
}

static int
usb_printer_daemon_start_failure_recover(const char *process_name,
					 int start_status)
{
	int start_errno = errno > 0 ? errno :
		(start_status > 0 ? start_status : EIO);
	int stop_status = 0, scan_error = 0, saved_errno, result;
	enum process_presence final_state = PROCESS_PRESENCE_UNKNOWN;

	result = detached_daemon_start_rollback_with_ops(
		start_status, start_errno, usb_printer_daemon_rollback_stop,
		usb_printer_daemon_scan, (void *)process_name,
		&stop_status, &final_state, &scan_error);
	saved_errno = errno;
	if (final_state == PROCESS_PRESENCE_ABSENT) {
		logmessage(LOGNAME,
			   "%s startup rollback confirmed process absent (stop_status=%d)",
			   process_name, stop_status);
	} else {
		logmessage(LOGNAME,
			   "%s startup rollback incomplete (primary_status=%d, stop_status=%d, process_state=%d, scan_error=%d)",
			   process_name, start_status, stop_status, final_state, scan_error);
	}
	errno = saved_errno;
	return result;
}

/* Launch a detached daemon and report both execve and post-exec readiness
 * errors. The close-on-exec pipe distinguishes exec success from failure. */
static int
start_usb_printer_daemon(char *const argv[], const char *process_name,
                         const char *pid_file)
{
	int exec_pipe[2];
	int pipe_flags, exec_error = 0, read_error = 0, saved_error;
	int scan_error = 0, start_status;
	pid_t child;
	ssize_t count;
	size_t received = 0, written = 0;
	enum process_presence presence;
	enum process_presence_start_action start_action;

	if (!argv || !argv[0] || !process_name || !process_name[0] ||
	    !pid_file || !pid_file[0]) {
		errno = EINVAL;
		return EINVAL;
	}

	/*
	 * A hotplug completion retry can arrive after a previous start succeeded
	 * but its NVRAM acknowledgement failed. Never unlink the live daemon's
	 * PID file or fork a duplicate; only a complete ABSENT scan authorizes
	 * stale PID-file cleanup and a new launch.
	 */
	presence = usb_printer_daemon_scan((void *)process_name, &scan_error);
	start_action = process_presence_start_action_for_state(presence);
	start_status = process_presence_start_status_for_action(
		start_action, scan_error, 0);
	if (start_status != 0) {
		errno = start_status;
		logmessage(LOGNAME,
			   "%s process inventory is unknown; refusing duplicate-safe startup (status=%d)",
			   process_name, start_status);
		return start_status;
	}
	if (start_action == PROCESS_PRESENCE_START_NOTIFY) {
		logmessage(LOGNAME, "%s is already running; skipping duplicate launch",
			   process_name);
		return 0;
	}

	if (unlink(pid_file) < 0 && errno != ENOENT) {
		saved_error = errno > 0 ? errno : EIO;
		errno = saved_error;
		logmessage(LOGNAME,
			   "cannot remove stale %s PID file %s (status=%d)",
			   process_name, pid_file, saved_error);
		return saved_error;
	}
	if (pipe(exec_pipe) < 0)
		return errno;

	pipe_flags = fcntl(exec_pipe[1], F_GETFD);
	if (pipe_flags < 0) {
		saved_error = errno;
		close(exec_pipe[0]);
		close(exec_pipe[1]);
		return saved_error;
	}
	if (fcntl(exec_pipe[1], F_SETFD, pipe_flags | FD_CLOEXEC) < 0) {
		saved_error = errno;
		close(exec_pipe[0]);
		close(exec_pipe[1]);
		return saved_error;
	}

	child = fork();
	if (child < 0) {
		saved_error = errno;
		close(exec_pipe[0]);
		close(exec_pipe[1]);
		return saved_error;
	}
	if (child == 0) {
		close(exec_pipe[0]);
		execv(argv[0], argv);
		exec_error = errno;
		while (written < sizeof(exec_error)) {
			count = write(exec_pipe[1], (char *)&exec_error + written,
				      sizeof(exec_error) - written);
			if (count < 0 && errno == EINTR)
				continue;
			if (count <= 0)
				break;
			written += (size_t)count;
		}
		_exit(127);
	}

	close(exec_pipe[1]);
	while (received < sizeof(exec_error)) {
		count = read(exec_pipe[0], (char *)&exec_error + received,
			     sizeof(exec_error) - received);
		if (count < 0 && errno == EINTR)
			continue;
		if (count < 0) {
			read_error = errno;
			break;
		}
		if (count == 0)
			break;
		received += (size_t)count;
	}
	close(exec_pipe[0]);

	if (read_error != 0)
		return usb_printer_daemon_start_failure_recover(process_name,
							 read_error);
	if (received == 0) {
		start_status = usb_printer_daemon_postcondition(process_name);
		if (start_status != 0)
			return usb_printer_daemon_start_failure_recover(
				process_name, start_status);
		return 0;
	}
	if (received == sizeof(exec_error)) {
		int child_status, reap_status;
		int saved_exec_error = exec_error > 0 ? exec_error : EIO;

		/* execv() failed and the child has exited; reap it before returning. */
		reap_status = usb_printer_reap_failed_exec_child(child, &child_status, waitpid);
		if (reap_status != 0)
			logmessage(LOGNAME,
				   "cannot reap failed USB printer exec child (status=%d)",
				   reap_status);
		errno = saved_exec_error;
		return saved_exec_error;
	}

	/* A partial status write can only come from the failed-exec _exit path. */
	{
		int child_status, reap_status;

		reap_status = usb_printer_reap_failed_exec_child(child, &child_status, waitpid);
		if (reap_status != 0)
			logmessage(LOGNAME,
				   "cannot reap partially reported USB printer exec child (status=%d)",
				   reap_status);
	}
	errno = EIO;
	return usb_printer_daemon_start_failure_recover(process_name, EIO);
}

#if defined(SRV_U2EC)
int
start_u2ec(void)
{
	int start_status, setting_status, enable_mode;
	int marker_result, marker_errno, marker_status;
	char setting[16];
	char *argv[] = { "/usr/sbin/u2ec", NULL };

	setting_status = read_usb_printer_setting_status("u2ec_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read u2ec_enable; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	setting_status = usb_printer_enable_mode_status(setting, 1, &enable_mode);
	if (setting_status != 0) {
		logmessage("USB Printer", "invalid u2ec_enable selector; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	if (enable_mode == 0)
		return 0;

	start_status = start_usb_printer_daemon(argv, "u2ec", "/var/run/u2ec.pid");
	if (start_status != 0) {
		logmessage("USB Printer", "u2ec exec failed (status=%d)", start_status);
		return start_status;
	}

	errno = 0;
	marker_result = nvram_set("apps_u2ec_ex", "1");
	marker_errno = errno;
	marker_status = usb_printer_nvram_write_status(marker_result, marker_errno);
	if (marker_status != 0) {
		errno = marker_status;
		logmessage("USB Printer",
			   "u2ec started but runtime marker publication failed (status=%d); keeping hotplug action retryable",
			   marker_status);
		return marker_status;
	}
	return 0;
}

int
stop_u2ec(void)
{
	char* svcs[] = { "u2ec", NULL };
	return usb_printer_kill_services(svcs, 3, 1);
}
#endif

#if defined(SRV_LPRD)
int
start_lpd(void)
{
	int start_status, setting_status, enable_mode;
	char setting[16];
	char *argv[] = { "/usr/sbin/lpd", NULL };

	setting_status = read_usb_printer_setting_status("lprd_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read lprd_enable; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	setting_status = usb_printer_enable_mode_status(setting, 1, &enable_mode);
	if (setting_status != 0) {
		logmessage("USB Printer", "invalid lprd_enable selector; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	if (enable_mode == 0)
		return 0;

	start_status = start_usb_printer_daemon(argv, "lpd", "/var/run/lpdparent.pid");
	if (start_status != 0)
		logmessage("USB Printer", "lpd exec failed (status=%d)", start_status);

	return start_status;
}

int
stop_lpd(void)
{
	char* svcs[] = { "lpd", NULL };

	/*
	 * Keep the last PID record: LPRng validates PID/start_time/executable
	 * before treating it as a live duplicate. Unlinking here can race a
	 * freshly restarted daemon publishing its replacement record.
	 */
	return usb_printer_kill_services(svcs, 3, 1);
}
#endif

int
start_p910nd(char *devlp)
{
	int start_status, setting_status, enable_mode;
	char setting[16], port[2], process_name[sizeof("p9100d")];

	setting_status = read_usb_printer_setting_status("rawd_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read rawd_enable; refusing printer daemon start for %s (status=%d)", devlp, setting_status);
		return setting_status;
	}
	setting_status = usb_printer_enable_mode_status(setting, 2, &enable_mode);
	if (setting_status != 0) {
		logmessage("USB Printer", "invalid rawd_enable selector for %s (status=%d)",
			   devlp ? devlp : "(null)", setting_status);
		return setting_status;
	}
	if (enable_mode == 0)
		return 0;

	start_status = usb_printer_p910nd_port_from_device(
		devlp, port, sizeof(port));
	if (start_status != 0) {
		logmessage("USB Printer",
			   "invalid raw printer device path %s (status=%d)",
			   devlp ? devlp : "(null)", start_status);
		return start_status;
	}
	start_status = usb_printer_p910nd_process_name(
		port[0], process_name, sizeof(process_name));
	if (start_status != 0)
		return start_status;

	if (enable_mode == 2)
		start_status = eval("/usr/sbin/p910nd", "-b", "-f", devlp, port);
	else
		start_status = eval("/usr/sbin/p910nd", "-f", devlp, port);

	if (start_status != 0) {
		logmessage("USB Printer", "p910nd launcher failed for %s (status=%d)",
			   devlp, start_status);
		return start_status;
	}

	/*
	 * p910nd forks and reports success from the parent before the child
	 * acquires its lock and binds the printer socket. Its argv[0] becomes
	 * p9100d, so verify that detached child before acknowledging startup.
	 */
	start_status = usb_printer_daemon_postcondition(process_name);
	if (start_status != 0) {
		/* eval() observes only the daemonizing parent; stop a late child. */
		start_status = usb_printer_daemon_start_failure_recover(
			process_name, start_status);
		logmessage("USB Printer",
			   "p910nd daemon readiness failed for %s (status=%d)",
			   devlp, start_status);
	}

	return start_status;
}

int
stop_p910nd(void)
{
	char* svcs[] = {
		"p910nd",
		"p9100d", "p9101d", "p9102d", "p9103d", "p9104d",
		"p9105d", "p9106d", "p9107d", "p9108d", "p9109d",
		NULL
	};
	return usb_printer_kill_services(svcs, 3, 1);
}

static int
exec_printer_daemons(int call_fw)
{
	int i, has_printer = 0;
	int start_status, first_status = 0;
	char *opt_printer_script = "/opt/bin/on_hotplug_printer.sh";
	char dev_lp[16];

	for (i = 0; i < 10; i++) {
		sprintf(dev_lp, "/dev/usb/lp%d", i);
		if (check_if_dev_exist(dev_lp)) {
			has_printer = 1;
			if (call_fw && check_if_file_exist(opt_printer_script)) {
				start_status = doSystem("%s %s", opt_printer_script, dev_lp);
				if (start_status != 0)
					logmessage("USB Printer", "optional printer hook failed for %s (status=%d)",
						   dev_lp, start_status);
				first_status = rc_first_service_start_status(first_status, start_status);
			}
			start_status = start_p910nd(dev_lp);
			first_status = rc_first_service_start_status(first_status, start_status);
		}
	}

	if (has_printer) {
#if defined(SRV_U2EC)
		start_status = start_u2ec();
		first_status = rc_first_service_start_status(first_status, start_status);
#endif
#if defined(SRV_LPRD)
		start_status = start_lpd();
		first_status = rc_first_service_start_status(first_status, start_status);
#endif
	}

	return first_status;
}

static void
record_usb_printer_stop_status(int *first_status, const char *daemon,
			       int status)
{
	if (status == 0)
		return;
	logmessage("USB Printer", "%s stop failed (status=%d)", daemon, status);
	if (*first_status == 0)
		*first_status = status;
}

int
stop_usb_printer_spoolers(void)
{
	int stop_status = 0;

#if defined(SRV_U2EC)
	record_usb_printer_stop_status(&stop_status, "u2ec", stop_u2ec());
#endif
#if defined(SRV_LPRD)
	record_usb_printer_stop_status(&stop_status, "lpd", stop_lpd());
#endif
	record_usb_printer_stop_status(&stop_status, "p910nd", stop_p910nd());
	return stop_status;
}

int
restart_usb_printer_spoolers(void)
{
	int stop_status = stop_usb_printer_spoolers();

	if (stop_status != 0) {
		logmessage("USB Printer", "printer restart aborted because an existing daemon did not stop (status=%d)",
			   stop_status);
		return stop_status;
	}
	return exec_printer_daemons(0);
}

int
try_start_usb_printer_spoolers(void)
{
	int stop_status = stop_usb_printer_spoolers();

	if (stop_status != 0) {
		logmessage("USB Printer", "printer start aborted because an existing daemon did not stop (status=%d)",
			   stop_status);
		return stop_status;
	}
	return exec_printer_daemons(1);
}

int
try_start_usb_modem_to_wan(void)
{
	int modem_prio, modem_devnum, has_link;
	int modem_config_status, modem_identity_status, link_status, wisp_status;
	int is_modem_wan = 0;
	const char *wisp_ifname = NULL;

	if (get_ap_mode())
		return 0;

	/* Read priority and modem readiness as one checked configuration snapshot. */
	modem_config_status = get_modem_wan_start_config_status(
		&modem_prio, &modem_devnum);
	if (modem_config_status != 0) {
		logmessage("USB hotplug",
			   "cannot read modem WAN startup configuration; preserving event for retry (status=%d)",
			   modem_config_status);
		return modem_config_status;
	}
	if (modem_prio < 1)
		return 0;

	/* Do not auto-start when the current modem-WAN identity is unknown. */
	modem_identity_status = get_usb_modem_wan_status(0, &is_modem_wan);
	if (!modem_wan_auto_start_allowed(modem_identity_status, is_modem_wan)) {
		if (modem_identity_status != 0) {
			logmessage("USB hotplug",
				   "refusing modem auto-start because WAN modem identity cannot be read (status=%d)",
				   modem_identity_status);
			return modem_identity_status;
		}
		return 0;
	}

	/* check modem enabled and ready */
	if (!modem_devnum)
		return 0;

	if (modem_prio == 2) {
		wisp_status = get_apcli_wisp_ifname_status(&wisp_ifname);
		if (wisp_status != 0) {
			errno = wisp_status;
			logmessage("USB hotplug",
				   "refusing modem auto-start because AP-client WISP configuration is unavailable (status=%d)",
				   wisp_status);
			return wisp_status;
		}
		if (wisp_ifname)
			return 0;

		errno = 0;
		has_link = get_wan_ether_link_direct(0);
		link_status = modem_wan_link_probe_error(has_link, errno);
		if (link_status != 0) {
			errno = link_status;
			logmessage("USB hotplug",
				   "refusing modem auto-start because WAN Ethernet link state is unknown (status=%d)",
				   link_status);
			return link_status;
		}

		if (has_link)
			return 0;
	}

	logmessage("USB hotplug", "try start USB Modem as WAN connection...");

	return try_wan_reconnect(1, 0);
}

