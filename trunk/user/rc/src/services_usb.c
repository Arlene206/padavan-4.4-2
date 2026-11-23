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
static enum process_presence
usb_printer_daemon_scan(void *context, int *scan_error)
{
	const char *process_name = (const char *)context;
	struct process_snapshot snapshot;
	enum process_presence presence;
	int saved_error;

	if (!process_name || !process_name[0]) {
		if (scan_error)
			*scan_error = EINVAL;
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	presence = process_inventory_scan(process_name, &snapshot);
	saved_error = snapshot.error > 0 ? snapshot.error : EIO;
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (presence == PROCESS_PRESENCE_UNKNOWN)
		errno = saved_error;
	return presence;
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
		return read_error;
	if (received == 0)
		return usb_printer_daemon_postcondition(process_name);
	if (received == sizeof(exec_error))
		return exec_error;
	return EIO;
}

#if defined(SRV_U2EC)
int
start_u2ec(void)
{
	int start_status, setting_status;
	int marker_result, marker_errno, marker_status;
	char setting[16];
	char *argv[] = { "/usr/sbin/u2ec", NULL };

	setting_status = read_usb_printer_setting_status("u2ec_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read u2ec_enable; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	if (strcmp(setting, "0") == 0)
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
	return kill_services(svcs, 3, 1);
}
#endif

#if defined(SRV_LPRD)
int
start_lpd(void)
{
	int start_status, setting_status;
	char setting[16];
	char *argv[] = { "/usr/sbin/lpd", NULL };

	setting_status = read_usb_printer_setting_status("lprd_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read lprd_enable; refusing printer daemon start (status=%d)", setting_status);
		return setting_status;
	}
	if (strcmp(setting, "0") == 0)
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
	return kill_services(svcs, 3, 1);
}
#endif

int
start_p910nd(char *devlp)
{
	int start_status, setting_status;
	char setting[16];

	setting_status = read_usb_printer_setting_status("rawd_enable", setting, sizeof(setting));
	if (setting_status != 0) {
		logmessage("USB Printer", "cannot read rawd_enable; refusing printer daemon start for %s (status=%d)", devlp, setting_status);
		return setting_status;
	}
	if (strcmp(setting, "0") == 0)
		return 0;

	if (strcmp(setting, "2") == 0)
		start_status = eval("/usr/sbin/p910nd", "-b", "-f", devlp, "0");
	else
		start_status = eval("/usr/sbin/p910nd", "-f", devlp, "0");

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
	start_status = usb_printer_daemon_postcondition("p9100d");
	if (start_status != 0)
		logmessage("USB Printer",
			   "p910nd daemon readiness failed for %s (status=%d)",
			   devlp, start_status);

	return start_status;
}

int
stop_p910nd(void)
{
	char* svcs[] = { "p910nd", NULL };
	return kill_services(svcs, 3, 1);
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

