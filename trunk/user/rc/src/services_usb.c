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
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#include <dev_info.h>
#include <usb_info.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "service_start_status.h"
#include "switch.h"
#include "gpio_pins.h"

void
safe_remove_usb_device(int port, const char *dev_name)
{
	int modem_devnum = 0;

	if (dev_name && strncmp(dev_name, "sd", 2) != 0) {
		modem_devnum = atoi(dev_name);
		if (modem_devnum < 0)
			modem_devnum = 0;
	}

	if (port >= 1 && port <= BOARD_NUM_USB_PORTS) {
		if (modem_devnum) {
			usb_info_t *usb_info, *follow_usb;
			int has_modem_port = 0;

			usb_info = get_usb_info();
			for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
				if ((follow_usb->port_root == port) &&
				    (follow_usb->id_devnum == modem_devnum) &&
				    (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
				     follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH))
					has_modem_port |= 1;
			}
			free_usb_info(usb_info);

			if (has_modem_port) {
				if (get_usb_modem_dev_wan(0, modem_devnum)) {
					int modem_remove_status;

					modem_remove_status = safe_remove_usb_modem();
					if (modem_remove_status == 0)
						try_wan_reconnect(0, 0);
					else
						logmessage(LOGNAME,
							   "skipping WAN reconnect after incomplete USB modem removal (status=%d)",
							   modem_remove_status);
				}
			}
		} else {
			safe_remove_stor_device(port, port, dev_name, 1);
		}

	} else if (port == 0) {
		safe_remove_stor_device(1, BOARD_NUM_USB_PORTS, NULL, 1);
	}
}

#if defined(SRV_U2EC)
int
start_u2ec(void)
{
	int start_status;

	if (nvram_match("u2ec_enable", "0"))
		return 0;

	unlink("/var/run/u2ec.pid");
	start_status = system("/usr/sbin/u2ec &");
	if (start_status != 0) {
		logmessage("USB Printer", "u2ec launcher failed (status=%d)", start_status);
		return start_status;
	}

	nvram_set("apps_u2ec_ex", "1");
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
	int start_status;

	if (nvram_match("lprd_enable", "0"))
		return 0;

	unlink("/var/run/lpdparent.pid");
	start_status = system("/usr/sbin/lpd &");
	if (start_status != 0)
		logmessage("USB Printer", "lpd launcher failed (status=%d)", start_status);

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
	int start_status;

	if (nvram_match("rawd_enable", "0"))
		return 0;

	if (nvram_match("rawd_enable", "2"))
		start_status = eval("/usr/sbin/p910nd", "-b", "-f", devlp, "0");
	else
		start_status = eval("/usr/sbin/p910nd", "-f", devlp, "0");

	if (start_status != 0)
		logmessage("USB Printer", "p910nd launcher failed for %s (status=%d)",
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

void
try_start_usb_modem_to_wan(void)
{
	int modem_prio, has_link;

	if (get_ap_mode())
		return;

	/* check modem prio mode  */
	modem_prio = nvram_get_int("modem_prio");
	if (modem_prio < 1)
		return;

	/* check modem already selected to WAN */
	if (get_usb_modem_wan(0))
		return;

	/* check modem enabled and ready */
	if (!get_modem_devnum())
		return;

	if (modem_prio == 2) {
		if (get_apcli_wisp_ifname())
			return;
		
		has_link = get_wan_ether_link_direct(0);
		if (has_link < 0)
			has_link = 0;
		
		if (has_link)
			return;
	}

	logmessage("USB hotplug", "try start USB Modem as WAN connection...");

	try_wan_reconnect(1, 0);
}

