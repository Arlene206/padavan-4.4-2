#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include <usb_info.h>
#include <disk_share.h>

#include "rc.h"
#include <nvram_linux.h>
#include "modem_wan_start_policy.h"
#include <process_inventory.h>
#include "ppp_options_file.h"
#include "ppp_option_value_policy.h"
#include "wan_nvram_status_policy.h"
#include "modem_node_identity_policy.h"
#include "modem_module_reload_policy.h"
#include "modem_qmi_credentials_file.h"
#include "modem_qmi_status_file.h"

#define MODEM_SCRIPTS_DIR	"/etc_ro"
#define MAX_USB_NODE		(15)
#define MAX_QMI_TRIES		(3)

static int
get_modem_vid_pid(const char *modem_node, int *vid, int *pid)
{
	char usb_port_id[64], usb_vid[8], usb_pid[8];

	// get USB port.
	if(!get_usb_port_by_device(modem_node, usb_port_id, sizeof(usb_port_id)))
		return 0;

	// get VID.
	if(!get_usb_vid(usb_port_id, usb_vid, sizeof(usb_vid)))
		return 0;

	// get PID.
	if(!get_usb_pid(usb_port_id, usb_pid, sizeof(usb_pid)))
		return 0;

	*vid = strtol(usb_vid, NULL, 16);
	*pid = strtol(usb_pid, NULL, 16);

	return 1;
}

static int
usb_modem_nvram_transaction_get(void *context, const char *param_name,
				char **value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)context;

	if (!transaction) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_get(transaction, param_name, value);
}


static int
usb_modem_nvram_transaction_begin(void *context)
{
	if (!context) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_begin((struct nvram_transaction *)context);
}


static int
usb_modem_nvram_transaction_end(void *context)
{
	if (!context) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_end((struct nvram_transaction *)context);
}

struct usb_modem_start_config {
	char modem_prio[32];
	char modem_rule[32];
	char modem_type[32];
	int modem_prio_present;
	int modem_rule_present;
	int modem_type_present;
};

/* Snapshot one NVRAM generation; copy the rotating-cache value immediately. */
static int
usb_modem_read_start_config_field(struct nvram_transaction *transaction,
				  const char *name, char *destination,
				  size_t destination_size, int *present)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!transaction || !name || !destination || destination_size == 0 ||
	    !present)
		return EINVAL;
	destination[0] = '\0';
	*present = 0;
	errno = 0;
	api_result = usb_modem_nvram_transaction_get(transaction, name, &value);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !value)
		return EIO;
	status = rc_wan_nvram_copy_text(value, destination, destination_size);
	if (status == 0)
		*present = 1;
	return status;
}

static int
usb_modem_read_start_config_snapshot(struct usb_modem_start_config *config)
{
	struct nvram_transaction transaction;
	int status, end_status, api_result, saved_errno;

	if (!config)
		return EINVAL;
	memset(config, 0, sizeof(*config));
	errno = 0;
	api_result = usb_modem_nvram_transaction_begin(&transaction);
	saved_errno = errno;
	status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status != 0)
		return status;
	status = usb_modem_read_start_config_field(
		&transaction, "modem_prio", config->modem_prio,
		sizeof(config->modem_prio), &config->modem_prio_present);
	if (status == 0)
		status = usb_modem_read_start_config_field(
			&transaction, "modem_rule", config->modem_rule,
			sizeof(config->modem_rule), &config->modem_rule_present);
	if (status == 0)
		status = usb_modem_read_start_config_field(
			&transaction, "modem_type", config->modem_type,
			sizeof(config->modem_type), &config->modem_type_present);
	errno = 0;
	api_result = usb_modem_nvram_transaction_end(&transaction);
	saved_errno = errno;
	end_status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status == 0)
		status = end_status;
	if (status != 0)
		memset(config, 0, sizeof(*config));
	return status;
}

static int
usb_modem_read_manual_node_index_status(int *manual_index_out)
{
	struct nvram_transaction transaction;
	char *value = NULL;
	int api_result, saved_errno, status, end_status, configured_index = 0;

	if (!manual_index_out)
		return EINVAL;
	*manual_index_out = -1;

	errno = 0;
	api_result = usb_modem_nvram_transaction_begin(&transaction);
	saved_errno = errno;
	status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status != 0)
		return status;

	errno = 0;
	api_result = usb_modem_nvram_transaction_get(&transaction, "modem_node", &value);
	saved_errno = errno;
	status = rc_wan_nvram_read_int_status(
		api_result, saved_errno, value, 0, &configured_index);

	errno = 0;
	api_result = usb_modem_nvram_transaction_end(&transaction);
	saved_errno = errno;
	end_status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status == 0)
		status = end_status;

	return modem_node_manual_index_status(status, configured_index,
					      manual_index_out);
}


static int
write_pppd_ras_credential(FILE *fp, const char *option, const char *value,
			  char *encoded, size_t encoded_size)
{
	if (!fp || !option ||
	    ppp_option_value_escape(value, encoded, encoded_size) != 0)
		return -1;

	if (fprintf(fp, "%s '%s'\n", option, encoded) < 0) {
		if (!errno)
			errno = EIO;
		return -1;
	}

	return 0;
}

static int
write_pppd_ras_conf(const char* call_path, const char *modem_node, int unit)
{
	FILE *fp;
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[4];
	int modem_type, vid = 0, pid = 0, saved_errno;
	int snapshot_status, modem_type_status;
	char modem_type_value[32], user[256], pass[256], isp[256];
	const char *failed_credential = NULL;
	char tmp[256], *connect;

	if (!get_modem_vid_pid(modem_node, &vid, &pid))
		return 0;

	fields[0].param_name = "modem_type";
	fields[0].destination = modem_type_value;
	fields[0].destination_size = sizeof(modem_type_value);
	fields[1].param_name = "modem_user";
	fields[1].destination = user;
	fields[1].destination_size = sizeof(user);
	fields[2].param_name = "modem_pass";
	fields[2].destination = pass;
	fields[2].destination_size = sizeof(pass);
	fields[3].param_name = "modem_isp";
	fields[3].destination = isp;
	fields[3].destination_size = sizeof(isp);

	snapshot_status = rc_wan_nvram_copy_snapshot(
		fields, 4, usb_modem_nvram_transaction_begin,
		usb_modem_nvram_transaction_get,
		usb_modem_nvram_transaction_end, &transaction);
	if (snapshot_status != 0) {
		saved_errno = snapshot_status;
		logmessage(LOGNAME,
			   "cannot snapshot USB modem PPP peer settings (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return 0;
	}

	modem_type_status = rc_wan_nvram_parse_int_value(
		modem_type_value, &modem_type);
	if (modem_type_status != 0) {
		saved_errno = modem_type_status;
		logmessage(LOGNAME,
			   "invalid USB modem transport type for PPP peer file (status=%d)",
			   saved_errno);
		errno = saved_errno;
		return 0;
	}

	if (!(fp = ppp_options_file_open(call_path)))
		return 0;

	fprintf(fp, "/dev/%s\n", modem_node);
	fprintf(fp, "crtscts\n");
	fprintf(fp, "modem\n");
	fprintf(fp, "noauth\n");

	if (strlen(user) > 0) {
		failed_credential = "username";
		if (write_pppd_ras_credential(fp, "user", user, tmp, sizeof(tmp)) != 0)
			goto invalid_credential;
	}
	if (strlen(pass) > 0) {
		failed_credential = "password";
		if (write_pppd_ras_credential(fp, "password", pass, tmp, sizeof(tmp)) != 0)
			goto invalid_credential;
	}

	if(!strcmp(isp, "Virgin") || !strcmp(isp, "CDMA-UA")){
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("modem_mtu", 1500, 1000, 1500));
	fprintf(fp, "mru %d\n", 1500);

	fprintf(fp, "persist\n");
	fprintf(fp, "maxfail %d\n", 0);
	fprintf(fp, "holdoff %d\n", 10);

	fprintf(fp, "nopcomp noaccomp\n");
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	fprintf(fp, "noipdefault\n");

	if (nvram_invmatch("modem_dnsa", "0"))
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "minunit %d\n", RAS_PPP_UNIT);
	fprintf(fp, "linkname wan%d\n", unit);

	if (nvram_get_int("modem_dbg") == 1)
		fprintf(fp, "debug\n");

	connect = "Generic_conn.scr";

	if (modem_type == 1) {
		connect = "EVDO_conn.scr";
	} else if( modem_type == 2) {
		connect = "td_conn.scr";
	} else {
		if (vid == 0x0b05 && pid == 0x0302) // T500
			connect = "t500_conn.scr";
		else if (vid == 0x0421 && pid == 0x0612) // CS-15
			connect = "t500_conn.scr";
		else if (vid == 0x106c && pid == 0x3716)
			connect = "verizon_conn.scr";
		else if (vid == 0x1410 && pid == 0x4400)
			connect = "rogers_conn.scr";
	}

	fprintf(fp, "%s \"/bin/comgt -d /dev/%s -s %s/ppp/3g/%s\"\n", "connect", modem_node, MODEM_SCRIPTS_DIR, connect);
	fprintf(fp, "%s \"/bin/comgt -d /dev/%s -s %s/ppp/3g/%s\"\n", "disconnect", modem_node, MODEM_SCRIPTS_DIR, "Generic_disconn.scr");

	if (ppp_options_file_finish(fp, call_path) != 0)
		return 0;

	return 1;

invalid_credential:
	saved_errno = errno ? errno : EINVAL;
	(void)fclose(fp);
	if (unlink(call_path) != 0 && errno != ENOENT)
		logmessage(LOGNAME, "cannot remove rejected USB modem PPP peer file (errno=%d)", errno);
	logmessage(LOGNAME, "refusing unsafe USB modem PPP %s value (errno=%d)",
		   failed_credential ? failed_credential : "credential", saved_errno);
	errno = saved_errno;
	return 0;
}

static int
find_modem_node(const char* pattern, int fetch_pref, int fetch_devnum, int fetch_index, int *devnum_out)
{
	FILE *fp;
	int i, node_pref, node_devnum;
	char node_fname[64];
	
	if (devnum_out)
		*devnum_out = 0;
	
	for (i=0; i<MAX_USB_NODE; i++) {
		if (!modem_node_manual_index_matches(fetch_index, i))
			continue;
		node_pref = 0;
		node_devnum = 0;
		sprintf(node_fname, "%s/%s%d", MODEM_NODE_DIR, pattern, i);
		fp = fopen(node_fname, "r");
		if (fp) {
			char buf[32];
			while (fgets(buf, sizeof(buf), fp)) {
				int tmp;
				char *ptr;
				if ((ptr = strchr(buf, '\n')))
					*ptr = 0;
				tmp = get_param_int(buf, "pref=", 10, -1);
				if (tmp >= 0) {
					node_pref = tmp;
				} else {
					tmp = get_param_int(buf, "devnum=", 10, -1);
					if (tmp >= 0) {
						node_devnum = tmp;
						if (devnum_out)
							*devnum_out = tmp;
					}
				}
			}
			fclose(fp);
			
			
			if (fetch_index >= 0) {
				if (i == fetch_index)
					return i;
			} else {
				if (fetch_devnum && fetch_devnum != node_devnum)
					continue;
				
				if (!fetch_pref || node_pref > 0)
					return i;
			}
		}
	}
	return -1;
}

static int
find_modem_node_by_usb_identity(const char *pattern,
				const char *expected_port_id,
				int expected_devnum,
				int manual_index,
				int require_preferred)
{
	FILE *fp;
	int i, node_pref, node_devnum, have_pref, have_devnum, have_port_id, read_error;
	long parsed_devnum;
	char *endptr;
	char node_fname[64], node_port_id[64], buf[96], *newline;
	size_t port_len;
	int path_len;

	if (!modem_node_usb_identity_is_valid(expected_port_id, expected_devnum))
		return -1;

	for (i = 0; i < MAX_USB_NODE; i++) {
		path_len = snprintf(node_fname, sizeof(node_fname), "%s/%s%d",
				    MODEM_NODE_DIR, pattern, i);
		if (path_len < 0 || path_len >= (int)sizeof(node_fname))
			return -1;

		fp = fopen(node_fname, "r");
		if (!fp)
			continue;

		node_pref = 0;
		node_devnum = 0;
		have_pref = 0;
		have_devnum = 0;
		have_port_id = 0;
		read_error = 0;
		node_port_id[0] = '\0';

		while (fgets(buf, sizeof(buf), fp) != NULL) {
			newline = strpbrk(buf, "\r\n");
			if (!newline) {
				read_error = 1;
				break;
			}
			*newline = '\0';

			if (!strncmp(buf, "pref=", 5)) {
				if (have_pref || (buf[5] != '0' && buf[5] != '1') ||
				    buf[6] != '\0') {
					read_error = 1;
					break;
				}
				node_pref = buf[5] - '0';
				have_pref = 1;
			} else if (!strncmp(buf, "devnum=", 7)) {
				const char *digit;
				if (have_devnum || !buf[7]) {
					read_error = 1;
					break;
				}
				for (digit = buf + 7; *digit; digit++) {
					if (*digit < '0' || *digit > '9') {
						read_error = 1;
						break;
					}
				}
				if (read_error)
					break;
				errno = 0;
				parsed_devnum = strtol(buf + 7, &endptr, 10);
				if (errno != 0 || *endptr || parsed_devnum < 1 ||
				    parsed_devnum > 127) {
					read_error = 1;
					break;
				}
				node_devnum = (int)parsed_devnum;
				have_devnum = 1;
			} else if (!strncmp(buf, "portid=", 7)) {
				if (have_port_id) {
					read_error = 1;
					break;
				}
				port_len = strlen(buf + 7);
				if (port_len == 0 || port_len >= sizeof(node_port_id)) {
					read_error = 1;
					break;
				}
				memcpy(node_port_id, buf + 7, port_len + 1);
				have_port_id = 1;
			}
		}

		if (ferror(fp))
			read_error = 1;
		if (fclose(fp) != 0)
			read_error = 1;

		if (!read_error && have_pref && have_devnum && have_port_id &&
		    modem_node_usb_identity_node_is_eligible(
			    expected_port_id, expected_devnum, node_port_id, node_devnum,
			    i, node_pref, manual_index, require_preferred))
			return i;
	}

	return -1;
}

static int
get_modem_node_by_usb_identity(const char *pattern,
			       const char *expected_port_id,
			       int expected_devnum)
{
	int manual_index = -1, valid_node, status;

	status = usb_modem_read_manual_node_index_status(&manual_index);
	if (status != 0) {
		errno = status;
		return -2;
	}

	/* A manual index is a hard pin, but it must belong to this USB device. */
	if (manual_index >= 0)
		return find_modem_node_by_usb_identity(pattern, expected_port_id,
					       expected_devnum, manual_index, 0);

	/* Preserve the legacy preferred-control-port selection within this modem. */
	valid_node = find_modem_node_by_usb_identity(pattern, expected_port_id,
					     expected_devnum, -1, 1);
	if (valid_node < 0)
		valid_node = find_modem_node_by_usb_identity(pattern, expected_port_id,
					     expected_devnum, -1, 0);

	return valid_node;
}

static int
get_modem_node(const char *pattern, int devnum, int manual_index,
	       int *devnum_out)
{
	int valid_node = -1;

	if (manual_index >= 0) {
		/* Manual selection is a hard pin; never fall back to another node. */
		valid_node = find_modem_node(pattern, 0, 0, manual_index, devnum_out);
	} else {
		valid_node = find_modem_node(pattern, 1, devnum, -1, devnum_out);
		if (valid_node < 0)
			valid_node = find_modem_node(pattern, 0, devnum, -1, devnum_out);
	}
	return valid_node;
}

static int
get_modem_node_ras(char node_name[16], int *devnum_out,
			   int *selection_status_out)
{
	int valid_node, manual_index = -1, status;

	if (!selection_status_out)
		return 0;
	*selection_status_out = 0;
	status = usb_modem_read_manual_node_index_status(&manual_index);
	if (status != 0) {
		*selection_status_out = status;
		return 0;
	}

	/* Check ACM first, then serial, using the same manual-index snapshot. */
	valid_node = get_modem_node("ttyACM", 0, manual_index, devnum_out);
	if (valid_node >= 0) {
		sprintf(node_name, "ttyACM%d", valid_node);
		return 1;
	}
	valid_node = get_modem_node("ttyUSB", 0, manual_index, devnum_out);
	if (valid_node >= 0) {
		sprintf(node_name, "ttyUSB%d", valid_node);
		return 1;
	}
	return 0;
}

static int
is_usbnet_has_module(char* ndis_ifname, const char *module_name)
{
	DIR *dir;
	int has_module = 0;
	char driver_path[128];

	snprintf(driver_path, sizeof(driver_path), "/sys/class/net/%s/device/driver/module/drivers/usb:%s", ndis_ifname, module_name);
	dir = opendir(driver_path);
	if (dir) {
		has_module = 1;
		closedir(dir);
	}

	return has_module;
}

static int
is_ready_modem_ras(int *devnum_out, int *selection_status_out)
{
	char node_name[16];

	if (!selection_status_out)
		return 0;
	*selection_status_out = 0;
	if (get_modem_node_ras(node_name, devnum_out, selection_status_out))
		return 1;
	return 0;
}

static int
is_ready_modem_ndis(int *devnum_out)
{
	char ndis_ifname[16];

	if (get_modem_ndis_ifname(ndis_ifname, devnum_out) && is_interface_exist(ndis_ifname))
		return 1;

	return 0;
}

int
get_modem_wan_start_config_status(int *modem_priority_out,
				  int *modem_devnum_out)
{
	struct usb_modem_start_config config;
	int modem_priority, modem_rule, modem_type, modem_devnum = 0;
	int status, node_selection_status = 0;

	if (!modem_priority_out || !modem_devnum_out)
		return EINVAL;
	*modem_priority_out = 0;
	*modem_devnum_out = 0;
	status = usb_modem_read_start_config_snapshot(&config);
	if (status != 0)
		return status;
	status = rc_wan_nvram_parse_int_value(config.modem_prio, &modem_priority);
	if (status != 0)
		return status;
	status = rc_wan_nvram_parse_int_value(config.modem_rule, &modem_rule);
	if (status != 0)
		return status;
	status = rc_wan_nvram_parse_int_value(config.modem_type, &modem_type);
	if (status != 0)
		return status;
	if (modem_rule > 0) {
		if (modem_type == 3) {
			if (!is_ready_modem_ndis(&modem_devnum))
				modem_devnum = 0;
		} else {
			if (!is_ready_modem_ras(&modem_devnum, &node_selection_status)) {
				if (node_selection_status != 0)
					return node_selection_status;
				modem_devnum = 0;
			}
		}
	}
	*modem_priority_out = modem_priority;
	*modem_devnum_out = modem_devnum;
	return 0;
}

int
get_modem_hotplug_notification_status(int is_ndis, int *should_notify)
{
	struct usb_modem_start_config config;
	int status;

	if (!should_notify || (is_ndis != 0 && is_ndis != 1))
		return EINVAL;
	*should_notify = 0;
	status = usb_modem_read_start_config_snapshot(&config);
	if (status != 0)
		return status;
	return modem_wan_hotplug_notification_enabled(
		config.modem_prio_present, config.modem_prio,
		config.modem_rule_present, config.modem_rule,
		config.modem_type_present, config.modem_type,
		is_ndis, should_notify);
}

int
get_modem_devnum(void)
{
	int modem_priority, modem_devnum, status;

	status = get_modem_wan_start_config_status(&modem_priority, &modem_devnum);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read USB modem WAN configuration; refusing readiness result (status=%d)",
			   status);
		errno = status;
		return 0;
	}
	return modem_devnum;
}

int
get_modem_ndis_ifname(char ndis_ifname[16], int *devnum_out)
{
	int valid_node = 0;

	valid_node = find_modem_node("wwan", 0, 0, -1, devnum_out); // first exist node
	if (valid_node >= 0) {
		sprintf(ndis_ifname, "wwan%d", valid_node);
		return 1;
	} else {
		valid_node = find_modem_node("weth", 0, 0, -1, devnum_out); // first exist node
		if (valid_node >= 0) {
			sprintf(ndis_ifname, "weth%d", valid_node);
			return 1;
		}
	}

	return 0;
}

static int
get_qmi_handle(const char *fn)
{
	int qmi_handle = -1;
	int status;

	status = modem_qmi_status_file_read(fn, &qmi_handle);
	if (status != 0) {
		errno = status;
		return -1;
	}
	return qmi_handle;
}

static int
qmi_control_network_start(const char *device_path)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[5];
	struct modem_qmi_credentials_file credentials_file;
	char pin_code[65] = {0};
	char modem_nets_text[32] = {0};
	char username[257] = {0};
	char password[257] = {0};
	char apn[257] = {0};
	char client_id_arg[32] = {0};
	char *qmi_nets;
	int modem_nets = 0, qmi_client_id = -1, pdh_handle = -1;
	int snapshot_status, status, cleanup_status, start_status;
	int unlink_status, attempt_status, ambiguous_session, handle_read_status, i;
	int result = 1;

	fields[0].param_name = "modem_pin";
	fields[0].destination = pin_code;
	fields[0].destination_size = sizeof(pin_code);
	fields[1].param_name = "modem_nets";
	fields[1].destination = modem_nets_text;
	fields[1].destination_size = sizeof(modem_nets_text);
	fields[2].param_name = "modem_user";
	fields[2].destination = username;
	fields[2].destination_size = sizeof(username);
	fields[3].param_name = "modem_pass";
	fields[3].destination = password;
	fields[3].destination_size = sizeof(password);
	fields[4].param_name = "modem_apn";
	fields[4].destination = apn;
	fields[4].destination_size = sizeof(apn);

	/* Read all QMI secrets and selectors from one checked NVRAM generation. */
	snapshot_status = rc_wan_nvram_copy_snapshot(
		fields, 5, usb_modem_nvram_transaction_begin,
		usb_modem_nvram_transaction_get,
		usb_modem_nvram_transaction_end, &transaction);
	if (snapshot_status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot QMI modem settings; refusing network start (status=%d)",
			   snapshot_status);
		goto done;
	}
	status = rc_wan_nvram_parse_int_value(modem_nets_text, &modem_nets);
	if (status != 0) {
		logmessage(LOGNAME,
			   "invalid QMI modem network-mode setting (status=%d)", status);
		goto done;
	}
	/* Reject corrupt selectors before uqmi can broaden them to "all". */
	if (!modem_qmi_network_mode_is_valid(modem_nets)) {
		status = EINVAL;
		logmessage(LOGNAME,
			   "unsupported QMI modem network-mode setting %d (expected 0..9)",
			   modem_nets);
		goto done;
	}

	/* A valid PDH file represents a prior session; never erase it and start another. */
	errno = 0;
	pdh_handle = get_qmi_handle(QMI_HANDLE_PDH);
	status = modem_qmi_existing_pdh_start_status(pdh_handle, errno);
	if (status != 0) {
		if (status == EBUSY)
			logmessage(LOGNAME, "existing QMI packet-data handle %d; refusing duplicate session start", pdh_handle);
		else
			logmessage(LOGNAME, "cannot validate existing QMI packet-data handle (status=%d); refusing network start", status);
		goto done;
	}

	/* uqmi consumes and unlinks this file; the SIM PIN never appears in argv. */
	if (pin_code[0]) {
		char *pin_argv[] = {
			"/bin/uqmi", "-d", (char *)device_path,
			"--private-credentials-file", credentials_file.path, NULL
		};

		if (modem_qmi_credentials_file_create(pin_code, NULL, NULL,
						      &credentials_file) != 0) {
			status = errno > 0 ? errno : EIO;
			logmessage(LOGNAME,
				   "cannot stage private QMI SIM PIN file (status=%d)", status);
			goto done;
		}
		status = modem_qmi_command_status(_eval(pin_argv, NULL, 0, NULL));
		cleanup_status = modem_qmi_credentials_file_cleanup(&credentials_file);
		if (cleanup_status != 0) {
			logmessage(LOGNAME,
				   "cannot remove private QMI SIM PIN staging directory (status=%d)",
				   cleanup_status);
			goto done;
		}
		if (status != 0) {
			logmessage(LOGNAME, "uqmi SIM PIN verification failed (status=%d)", status);
			goto done;
		}
	}

	/* set interface format as 802.3 */
	{
		char *format_argv[] = {
			"/bin/uqmi", "-d", (char *)device_path,
			"--set-data-format", "802.3", NULL
		};
		status = modem_qmi_command_status(_eval(format_argv, NULL, 0, NULL));
		if (status != 0) {
			logmessage(LOGNAME, "uqmi failed to set 802.3 data format (status=%d)", status);
			goto done;
		}
	}

	/* setup network modes; an empty legacy NVRAM value still means zero/all. */
	qmi_nets = "all";
	switch (modem_nets) {
	case 9: qmi_nets = "td-scdma"; break;
	case 8: qmi_nets = "cdma"; break;
	case 7: qmi_nets = "gsm"; break;
	case 6: qmi_nets = "gsm,umts"; break;
	case 5: qmi_nets = "umts,gsm"; break;
	case 4: qmi_nets = "umts"; break;
	case 3: qmi_nets = "lte,umts,gsm"; break;
	case 2: qmi_nets = "lte,umts"; break;
	case 1: qmi_nets = "lte"; break;
	}
	{
		char *mode_argv[] = {
			"/bin/uqmi", "-d", (char *)device_path,
			"--set-network-modes", qmi_nets, NULL
		};
		status = modem_qmi_command_status(_eval(mode_argv, NULL, 0, NULL));
		if (status != 0) {
			logmessage(LOGNAME, "uqmi failed to set requested network modes (status=%d)", status);
			goto done;
		}
	}

	/* Reuse the persisted WDS CID; allocating a second one leaks the old CID. */
	errno = 0;
	qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);
	if (modem_qmi_client_id_is_valid(qmi_client_id)) {
		logmessage(LOGNAME, "reusing persisted WDS client ID %d", qmi_client_id);
	} else if (modem_qmi_client_id_should_allocate(qmi_client_id, errno)) {
		char *client_argv[] = {
			"/bin/uqmi", "-d", (char *)device_path,
			"--get-client-id", "wds", NULL
		};

		status = _eval(client_argv, NULL, 0, NULL);
		if (status != 0) {
			logmessage(LOGNAME,
				   "uqmi failed to acquire and persist WDS client ID (status=%d); refusing network start",
				   status);
			goto done;
		}
		errno = 0;
		qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);
		if (!modem_qmi_client_id_is_valid(qmi_client_id)) {
			status = errno > 0 ? errno : EINVAL;
			logmessage(LOGNAME, "uqmi reported success without a valid one-byte WDS client ID (status=%d)", status);
			goto done;
		}
	} else {
		status = errno > 0 ? errno : EINVAL;
		logmessage(LOGNAME, "persisted WDS client ID is invalid or unreadable (status=%d); refusing to overwrite it", status);
		goto done;
	}
	snprintf(client_id_arg, sizeof(client_id_arg), "wds,%d", qmi_client_id);

	for (i = 0; i < MAX_QMI_TRIES; i++) {
		char *start_argv[20];
		int argc = 0;
		int has_credentials = username[0] && password[0];

		memset(&credentials_file, 0, sizeof(credentials_file));
		start_argv[argc++] = "/bin/uqmi";
		start_argv[argc++] = "-d";
		start_argv[argc++] = (char *)device_path;
		if (qmi_client_id >= 0) {
			start_argv[argc++] = "--set-client-id";
			start_argv[argc++] = client_id_arg;
		}
		start_argv[argc++] = "--keep-client-id";
		start_argv[argc++] = "wds";
		if (has_credentials) {
			if (modem_qmi_credentials_file_create(NULL, username, password,
							      &credentials_file) != 0) {
				status = errno > 0 ? errno : EIO;
				logmessage(LOGNAME,
					   "cannot stage private QMI authentication file (status=%d)",
					   status);
				goto done;
			}
			start_argv[argc++] = "--private-credentials-file";
			start_argv[argc++] = credentials_file.path;
		}
		start_argv[argc++] = "--start-network";
		start_argv[argc++] = apn;
		start_argv[argc++] = "--autoconnect";
		start_argv[argc] = NULL;
		start_status = _eval(start_argv, NULL, 0, NULL);
		errno = 0;
		pdh_handle = get_qmi_handle(QMI_HANDLE_PDH);
		handle_read_status = errno;
		attempt_status = modem_qmi_start_attempt_status(start_status,
							       pdh_handle);
		ambiguous_session = modem_qmi_start_attempt_is_ambiguous(start_status,
								       pdh_handle, handle_read_status);

		if (has_credentials) {
			cleanup_status = modem_qmi_credentials_file_cleanup(&credentials_file);
			if (cleanup_status != 0) {
				logmessage(LOGNAME,
					   "cannot remove private QMI authentication staging directory (status=%d)",
					   cleanup_status);
				goto done;
			}
		}
		if (attempt_status == 0) {
			result = 0;
			break;
		}
		logmessage(LOGNAME,
			   "uqmi start-network did not publish a valid packet-data handle (command status=%d, handle=%d)",
			   start_status, pdh_handle);

		/* Preserve valid or unreadable PDH state if a session may exist. */
		if (modem_qmi_start_attempt_preserve_handle(ambiguous_session,
								   pdh_handle, handle_read_status))
			goto done;

		/* Remove any partial/invalid publication before a retry. */
		unlink_status = unlink(QMI_HANDLE_PDH);
		cleanup_status = modem_qmi_handle_file_unlink_status(unlink_status,
								     errno);
		if (cleanup_status != 0) {
			logmessage(LOGNAME,
				   "cannot remove failed QMI packet-data handle (status=%d); aborting retries",
				   cleanup_status);
			goto done;
		}
		/*
		 * A failed command with a published handle may have created a live
		 * session. Do not launch a duplicate session blindly.
		 */
		if (ambiguous_session)
			goto done;
		sleep(1);
	}

done:
	modem_qmi_credentials_wipe(pin_code, sizeof(pin_code));
	modem_qmi_credentials_wipe(username, sizeof(username));
	modem_qmi_credentials_wipe(password, sizeof(password));
	modem_qmi_credentials_wipe(apn, sizeof(apn));
	modem_qmi_credentials_wipe(modem_nets_text, sizeof(modem_nets_text));
	modem_qmi_credentials_wipe(client_id_arg, sizeof(client_id_arg));
	return result;
}


static int
qmi_control_network(const char *control_node, int is_start)
{
	int qmi_client_id = -1;
	int path_len;
	int command_status, release_status;
	int unlink_status, cleanup_status, result = 0;
	char device_path[64];

	if (!control_node || !*control_node)
		return 1;
	path_len = snprintf(device_path, sizeof(device_path), "/dev/%s", control_node);
	if (path_len < 0 || path_len >= (int)sizeof(device_path))
		return 1;

	if (is_start)
		return qmi_control_network_start(device_path);

	/* A failed stop leaves the session and its state files intact for retry. */
	{
		char *stop_argv[] = {
			"/bin/uqmi", "-d", device_path, "--stop-network",
			"0xffffffff", "--autoconnect", NULL
		};
		command_status = modem_qmi_command_status(
			_eval(stop_argv, NULL, 0, NULL));
		if (command_status != 0) {
			logmessage(LOGNAME,
				   "uqmi stop-network failed (status=%d); preserving QMI session state",
				   command_status);
			return 1;
		}
	}

	/* The network is stopped; remove its PDH marker but check the cleanup. */
	unlink_status = unlink(QMI_HANDLE_PDH);
	cleanup_status = modem_qmi_handle_file_unlink_status(unlink_status, errno);
	if (!modem_qmi_client_id_release_allowed(cleanup_status)) {
		logmessage(LOGNAME,
			   "cannot remove stopped QMI packet-data handle (status=%d); preserving WDS client ID for recovery",
			   cleanup_status);
		return 1;
	}

	/* Release the client ID only after stop-network and PDH cleanup succeed. */
	errno = 0;
	qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);
	if (qmi_client_id >= 0) {
		char client_id_arg[32];

		if (!modem_qmi_client_id_is_valid(qmi_client_id)) {
			logmessage(LOGNAME, "invalid persisted WDS client ID %d; preserving it for recovery", qmi_client_id);
			return 1;
		}
		char *release_argv[] = {
			"/bin/uqmi", "-d", device_path, "--set-client-id",
			client_id_arg, "--release-client-id", "wds", NULL
		};
		snprintf(client_id_arg, sizeof(client_id_arg), "wds,%d", qmi_client_id);
		release_status = modem_qmi_command_status(
			_eval(release_argv, NULL, 0, NULL));
		if (release_status != 0) {
			logmessage(LOGNAME,
				   "uqmi release-client-id failed (status=%d); preserving client ID for retry",
				   release_status);
			return 1;
		}
	} else if (errno != ENOENT) {
		logmessage(LOGNAME, "cannot read persisted WDS client ID (status=%d); preserving it for recovery", errno > 0 ? errno : EIO);
		return 1;
	}

	/* A missing ID needs no release, but stale-file cleanup is checked. */
	unlink_status = unlink(QMI_CLIENT_ID);
	cleanup_status = modem_qmi_handle_file_unlink_status(unlink_status, errno);
	if (cleanup_status != 0) {
		logmessage(LOGNAME,
			   "cannot remove released QMI client ID (status=%d)",
			   cleanup_status);
		result = 1;
	}
	return result;
}
static int
mbim_control_network(const char* control_node, int is_start)
{
	/* MBIM device control not supported yet (libmbim depends from large libglib) */
	return 1;
}

static int
ncm_control_network(const char* control_node, int is_start)
{
	return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s",
		control_node, MODEM_SCRIPTS_DIR, (is_start) ? "NCM_conn.scr" : "NCM_disconn.scr");
}

static int
sierra_control_network(const char* control_node, int is_start)
{
	return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s",
		control_node, MODEM_SCRIPTS_DIR, (is_start) ? "Sierra_conn.scr" : "Sierra_disconn.scr");
}

#if 0
static int
ncm_control_network(const char* control_node, int is_start)
{
	FILE *fp;
	int result = 1;
	char node_path[32], node_msg[64];

	if (is_start) {
		char *apn = nvram_safe_get("modem_apn");
		if (strlen(apn) < 1)
			apn = "internet";
		snprintf(node_msg, sizeof(node_msg), "AT^NDISDUP=1,%d,\"%s\"\r\n", 1, apn);
	} else
		snprintf(node_msg, sizeof(node_msg), "AT^NDISDUP=1,%d\r\n", 0);

	snprintf(node_path, sizeof(node_path), "/dev/%s", control_node);
	fp = fopen(node_path, "wb");
	if (fp) {
		if (fwrite(node_msg, 1, strlen(node_msg), fp) > 0)
			result = 0;
		
		fclose(fp);
	}

	node_msg[strlen(node_msg) - 2] = 0; /* get rid of '\r\n' */

	if (!result) {
		if (is_start)
			sleep(1);
	} else {
		logmessage(LOGNAME, "NCM message %s to node %s: %s", node_msg, node_path, "FAILED!");
	}

	return result;
}
#endif

static int
ndis_control_network(char *ndis_ifname, int devnum, int is_start,
		     int *control_required)
{
	int valid_node, manual_node_status = 0;
	char control_node_wdm[16] = {0}, control_node_tty[16] = {0};
	char ndis_port_id[64] = {0};

	if (!control_required)
		return -1;
	*control_required = 0;

	/*
	 * A WDM node is usable only when its published physical parent and USB
	 * address match the selected NDIS interface; never take the first WDM
	 * device on a multi-modem hub.
	 */
	if (get_usb_port_by_device(ndis_ifname, ndis_port_id, sizeof(ndis_port_id))) {
		ndis_port_id[sizeof(ndis_port_id) - 1] = '\0';
		valid_node = find_modem_node_by_usb_identity("cdc-wdm",
						       ndis_port_id, devnum, -1, 0);
		if (valid_node >= 0)
			snprintf(control_node_wdm, sizeof(control_node_wdm),
				 "cdc-wdm%d", valid_node);

		/*
		 * TTY fallback obeys the same physical identity. A manual index or
		 * preferred port on another modem must never control this interface.
		 */
		valid_node = get_modem_node_by_usb_identity("ttyUSB", ndis_port_id, devnum);
		if (valid_node == -2)
			manual_node_status = errno > 0 ? errno : EIO;
		else if (valid_node >= 0)
			snprintf(control_node_tty, sizeof(control_node_tty),
				 "ttyUSB%d", valid_node);
	}

	if (strlen(control_node_wdm) > 0) {
		if (is_usbnet_has_module(ndis_ifname, "qmi_wwan")) {
			*control_required = 1;
			return qmi_control_network(control_node_wdm, is_start);
		}

		if (is_usbnet_has_module(ndis_ifname, "cdc_mbim"))
			return mbim_control_network(control_node_wdm, is_start);

		if (is_usbnet_has_module(ndis_ifname, "huawei_cdc_ncm")) {
			*control_required = 1;
			return ncm_control_network(control_node_wdm, is_start);
		}
	}

	if (is_usbnet_has_module(ndis_ifname, "cdc_ncm")) {
		if (manual_node_status != 0) {
			*control_required = 1;
			logmessage(LOGNAME,
				   "cannot read manual NDIS control-node selection (status=%d)",
				   manual_node_status);
			return manual_node_status;
		}
		if (strlen(control_node_tty) > 0) {
			*control_required = 1;
			return ncm_control_network(control_node_tty, is_start);
		}
	}

	if (is_usbnet_has_module(ndis_ifname, "sierra_net")) {
		if (manual_node_status != 0) {
			*control_required = 1;
			logmessage(LOGNAME,
				   "cannot read manual NDIS control-node selection (status=%d)",
				   manual_node_status);
			return manual_node_status;
		}
		if (strlen(control_node_tty) > 0) {
			*control_required = 1;
			return sierra_control_network(control_node_tty, is_start);
		}
	}

	return 1;
}

#if 0
static void
unlink_modem_ras(void)
{
	int i;
	char node_fname[64];

	for (i=0; i<MAX_USB_NODE; i++)
	{
		snprintf(node_fname, sizeof(node_fname), "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/ttyACM%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}

static void
unlink_modem_ndis(void)
{
	int i;
	char node_fname[64];

	for (i=0; i<MAX_USB_NODE; i++)
	{
		snprintf(node_fname, sizeof(node_fname), "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/cdc-wdm%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/weth%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/wwan%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}
#endif

static int
modem_module_unload_callback(const char *module_name, void *context)
{
	int loaded_status, remaining_status, unload_status = 0;
	int saved_errno = 0, status;

	loaded_status = is_module_loaded_status(module_name);
	remaining_status = loaded_status;
	if (loaded_status == 1) {
		errno = 0;
		unload_status = module_smart_unload(module_name, 1);
		saved_errno = errno;
		/* module_smart_unload() returns 1 when refcount > 0 without
		 * removing the module. Re-read sysfs and require confirmed absence.
		 */
		remaining_status = is_module_loaded_status(module_name);
	}
	status = rc_modem_module_unload_attempt_status(loaded_status,
						      unload_status, saved_errno,
						      remaining_status);
	if (status != 0) {
		errno = status;
		return 0;
	}
	if (loaded_status == 1 && remaining_status == 0 && context)
		++*(size_t *)context;
	return 1;
}

static int
unload_modem_driver_group(const char *group_name,
			  const char *const *module_names,
			  size_t module_count)
{
	size_t successful_count = 0, unloaded_count = 0;
	int status;

	status = rc_modem_module_unload_group_status(module_names,
						     module_count,
						     modem_module_unload_callback,
						     &unloaded_count, &successful_count);
	if (unloaded_count > 0)
		sleep(1);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot unload USB modem %s driver group (status=%d)",
			   group_name, status);
	return status;
}

static int
remove_qmi_client_id_marker(void)
{
	if (unlink(QMI_CLIENT_ID) == 0 || errno == ENOENT)
		return 0;
	return errno > 0 ? errno : EIO;
}

int
unload_modem_modules(void)
{
	static const char *const modules[] = {
		"rndis_host", "qmi_wwan", "cdc_mbim", "huawei_cdc_ncm",
		"cdc_ncm", "cdc_ether", "cdc_acm", "sierra_net",
		"sierra", "qcserial", "option"
	};
	int status;

	status = unload_modem_driver_group("all", modules,
					  sizeof(modules) / sizeof(modules[0]));
	if (status != 0)
		return status;
	status = remove_qmi_client_id_marker();
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot remove inactive QMI client-ID marker (status=%d)",
			   status);
	return status;
}

static int
load_modem_module_group(const char *group_name,
		const char *const *module_names,
		const char *const *module_params,
		size_t module_count)
{
	static const rc_modem_module_load_ops_t ops = {
		module_smart_load,
		is_module_loaded_status
	};
	size_t available = 0;
	int status;

	status = rc_modem_module_load_group_verified(module_names, module_params,
						     module_count, &ops,
						     &available);
	if (status != 0)
		logmessage(LOGNAME,
			   "no usable %s USB modem driver module is available (checked=%lu)",
			   group_name, (unsigned long)module_count);
	else
		logmessage(LOGNAME,
			   "USB modem %s driver readiness: %lu/%lu alternatives available",
			   group_name, (unsigned long)available,
			   (unsigned long)module_count);

	return status;
}

int
reload_modem_modules(int modem_type, int reload)
{
	static const char *const ndis_modules[] = {
		"cdc_ether", "rndis_host", "qmi_wwan", "cdc_ncm",
		"huawei_cdc_ncm", "cdc_mbim", "sierra_net"
	};
	static const char *const ndis_params[] = {
		NULL, NULL, NULL, "prefer_mbim=0", NULL, NULL, NULL
	};
	static const char *const ras_modules[] = {
		"cdc_acm", "qcserial", "sierra", "option"
	};
	static const char *const serial_aux_modules[] = {
		"qcserial", "sierra", "option"
	};
	int status = 0;
	size_t i;

	/* Remove shared serial/control drivers first. Do not load the new family
	 * or discard the QMI client marker if any required unload is uncertain. */
	status = unload_modem_driver_group("serial/control", ras_modules,
					  sizeof(ras_modules) / sizeof(ras_modules[0]));
	if (status != 0)
		goto done;

	if (modem_type != 3) {
		status = unload_modem_driver_group("NDIS", ndis_modules,
					  sizeof(ndis_modules) / sizeof(ndis_modules[0]));
		if (status != 0)
			goto done;
	}

	status = remove_qmi_client_id_marker();
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot remove QMI client-ID marker before driver reload (status=%d)",
			   status);
		goto done;
	}

	if (modem_type == 3) {
		status = load_modem_module_group("NDIS", ndis_modules,
						 ndis_params,
						 sizeof(ndis_modules) / sizeof(ndis_modules[0]));
		/* Serial AT/control ports remain supplemental optional drivers. */
		for (i = 0; i < sizeof(serial_aux_modules) / sizeof(serial_aux_modules[0]); i++)
			(void)module_smart_load(serial_aux_modules[i], NULL);
	} else {
		status = load_modem_module_group("RAS", ras_modules, NULL,
						 sizeof(ras_modules) / sizeof(ras_modules[0]));
	}
done:
	if (reload)
		sleep(1);
	return status;
}

int
notify_modem_on_wan_ether_link_changed(int has_link)
{
	int modem_priority = 0, modem_devnum = 0;
	int is_wisp_active = 0, is_modem_wan = 0, reconnect = 0;
	int status, saved_errno;

	/*
	 * Use one checked start-configuration snapshot for both modem priority
	 * and device readiness. An NVRAM read error must not look like priority
	 * zero and silently suppress the link-change recovery event.
	 */
	status = get_modem_wan_start_config_status(&modem_priority,
						   &modem_devnum);
	if (status != 0) {
		logmessage(LOGNAME,
			   "skipping modem WAN reconnect: configuration read failed (status=%d)",
			   status);
		return status;
	}
	if (modem_priority != 2 || modem_devnum <= 0)
		return 0;

	/* The legacy boolean wrapper treats unknown WISP identity as active. */
	status = get_wan_wisp_active_status(&is_wisp_active, NULL);
	if (status != 0) {
		logmessage(LOGNAME,
			   "skipping modem WAN reconnect: WISP identity read failed (status=%d)",
			   status);
		return status;
	}
	if (is_wisp_active)
		return 0;

	status = get_usb_modem_wan_status(0, &is_modem_wan);
	if (status != 0) {
		logmessage(LOGNAME,
			   "skipping modem WAN reconnect: modem identity read failed (status=%d)",
			   status);
		return status;
	}

	status = modem_wan_link_change_reconnect_enabled(
		modem_priority, modem_devnum, is_modem_wan, has_link, &reconnect);
	if (status != 0) {
		logmessage(LOGNAME,
			   "skipping modem WAN reconnect: invalid link-change state (status=%d)",
			   status);
		return status;
	}
	if (!reconnect)
		return 0;

	/* Do not drop the one-shot link recovery when PID 1 cannot queue it. */
	errno = 0;
	status = notify_rc_status("auto_wan_reconnect");
	if (status != 0) {
		saved_errno = errno > 0 ? errno : EIO;
		logmessage(LOGNAME,
			   "failed to queue modem WAN reconnect after Ethernet link change (status=%d, errno=%d)",
			   status, saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	return 0;
}

#if 0
void
notify_modem_on_internet_state_changed(int has_internet)
{
	int modem_identity_status, is_modem_wan = 0;

	if (has_internet)
		return;

	if (nvram_get_int("modem_prio") != 3)
		return;

	modem_identity_status = get_usb_modem_wan_status(0, &is_modem_wan);
	if (modem_identity_status != 0 || is_modem_wan)
		return;

	if (!get_modem_devnum())
		return;

	notify_rc("auto_wan_reconnect");
}
#endif

static int
usb_modem_require_no_active_process(const char *process_name)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	int scan_error, saved_errno;

	presence = process_inventory_scan(process_name, &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;

	if (presence == PROCESS_PRESENCE_RUNNING)
		saved_errno = EBUSY;
	else
		saved_errno = scan_error ? scan_error : EIO;
	errno = saved_errno;
	logmessage(LOGNAME,
		   "refusing USB modem removal while %s is active or unknown (status=%d, errno=%d)",
		   process_name, presence, saved_errno);
	return -1;
}

static int
usb_modem_read_transport_status(int *is_ndis)
{
	char *value = NULL;
	int modem_type = 0;
	int api_result, saved_errno, status;

	if (!is_ndis) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked("modem_type", &value);
	saved_errno = errno;
	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					      value, 0, &modem_type);
	status = rc_wan_nvram_modem_is_ndis_status(status, modem_type,
						   is_ndis);
	if (status != 0)
		errno = status;
	return status;
}


int
safe_remove_usb_modem(void)
{
	const char *pidfiles[2];
	int stop_status, publish_status;
	int modem_type_status, is_ndis = 0;

	/* Never terminate unrelated USB mode-switch or storage eject jobs. */
	if (usb_modem_require_no_active_process("usb_modeswitch") != 0 ||
	    usb_modem_require_no_active_process("eject") != 0)
		return -1;

	/*
	 * Removal must choose the correct NDIS vs RAS teardown path. A failed
	 * NVRAM read cannot safely default to RAS and leave an NDIS lease active.
	 */
	modem_type_status = usb_modem_read_transport_status(&is_ndis);
	if (modem_type_status != 0) {
		logmessage(LOGNAME,
			   "refusing USB modem removal: configured transport is unknown (status=%d)",
			   modem_type_status);
		errno = modem_type_status;
		return -1;
	}

	if (is_ndis) {
		/* NDIS DHCP belongs to WAN unit 0, not every udhcpc instance. */
		stop_status = stop_udhcpc_wan(0);
		if (stop_status != 0) {
			logmessage(LOGNAME, "refusing NDIS modem removal: WAN DHCP identity stop failed (status=%d)",
				   stop_status);
			return stop_status;
		}
		stop_status = stop_wan_usbnet();
		if (stop_status != 0) {
			logmessage(LOGNAME, "refusing NDIS modem removal: modem network teardown failed (status=%d)",
				   stop_status);
			return stop_status;
		}
	} else {
		/* RAS pppd publishes linkname=wan0 in its identity-bound link PID file. */
		pidfiles[0] = "/var/run/ppp-wan0.pid";
		pidfiles[1] = NULL;
		stop_status = kill_services_with_pidfiles(NULL, pidfiles, "pppd", 0, 10, 1);
		if (stop_status != 0) {
			logmessage(LOGNAME, "refusing RAS modem removal: PPP link identity stop failed (status=%d)",
				   stop_status);
			return stop_status;
		}
	}

	publish_status = set_usb_modem_dev_wan(0, 0);
	if (publish_status != 0) {
		logmessage(LOGNAME,
			   "USB modem processes stopped, but WAN modem selection could not be cleared (status=%d)",
			   publish_status);
		return publish_status;
	}
	return 0;
}

int
launch_wan_modem_ras(int unit)
{
	char node_name[16] = {0};
	char call_file[16];
	char call_path[32];
	int saved_errno, launch_status, node_selection_status = 0;

	snprintf(call_file, sizeof(call_file), "modem.wan%d", unit);
	snprintf(call_path, sizeof(call_path), "%s/%s", PPP_PEERS_DIR, call_file);

	/* Fail closed even when this entry point is called outside start_wan(). */
	if (ppp_options_directory_ready("/tmp/ppp") != 0 ||
	    ppp_options_directory_ready(PPP_PEERS_DIR) != 0) {
		saved_errno = errno ? errno : EPERM;
		logmessage(LOGNAME, "refusing USB modem RAS peer file in unsafe directory (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	if (get_modem_node_ras(node_name, NULL, &node_selection_status)) {
		if (write_pppd_ras_conf(call_path, node_name, unit)) {
			logmessage(LOGNAME, "select RAS modem interface %s to pppd", node_name);
			errno = 0;
			launch_status = eval("/usr/sbin/pppd", "call", call_file);
			if (!modem_wan_ras_peer_cleanup_required(launch_status))
				return launch_status;

			/* Never leave credentials in a peer file after a failed launch. */
			saved_errno = errno;
			if (unlink(call_path) != 0 && errno != ENOENT)
				logmessage(LOGNAME,
					   "cannot remove failed USB modem RAS peer file (status=%d, errno=%d)",
					   launch_status, errno);
			errno = saved_errno;
			return launch_status;
		}
	}

	if (node_selection_status != 0) {
		saved_errno = node_selection_status;
		logmessage(LOGNAME,
			   "cannot read manual USB modem RAS node selection (status=%d)",
			   saved_errno);
		if (unlink(call_path) != 0 && errno != ENOENT)
			logmessage(LOGNAME,
				   "cannot remove stale USB modem RAS peer after selection error (errno=%d)",
				   errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* Never leave a stale peer with old credentials available for a later call. */
	if (unlink(call_path) != 0 && errno != ENOENT)
		logmessage(LOGNAME, "cannot remove stale USB modem RAS peer file (errno=%d)", errno);
	logmessage(LOGNAME, "unable to open RAS modem script!");
	return 1;
}

static int
usb_modem_wan_identity_set_value(void *context, const char *param_name,
				 const char *value)
{
	int *unit = (int *)context;

	if (!unit)
		return EINVAL;
	return set_wan_unit_value_status(*unit, param_name, value);
}


int
launch_wan_usbnet(int unit)
{
	int modem_devnum = 0;
	int migration_status, interface_status, firewall_status, control_status;
	int control_required = 0, dhcp_status, startup_status;
	int identity_status, identity_rollback_status, error_status, result_status;
	char ndis_ifname[16] = {0};

	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum) &&
	    is_interface_exist(ndis_ifname)) {
		int ndis_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);

		migration_status = check_upnp_wanif_changed(ndis_ifname);
		if (migration_status != 0) {
			error_status = set_wan_unit_error_status(unit, 1);
			logmessage(LOGNAME,
				   "aborting USB modem WAN startup: miniupnpd did not stop before interface migration (status=%d)",
				   migration_status);
			return modem_wan_start_status(migration_status, error_status);
		}

		identity_status = modem_wan_publish_ndis_identity(
			usb_modem_wan_identity_set_value, &unit, ndis_ifname,
			&identity_rollback_status);
		if (identity_status != 0) {
			error_status = set_wan_unit_error_status(unit, 1);
			if (identity_rollback_status != 0)
				logmessage(LOGNAME,
					   "USB modem WAN identity rollback incomplete for unit %d (status=%d)",
					   unit, identity_rollback_status);
			logmessage(LOGNAME,
				   "aborting USB modem WAN startup: cannot publish NDIS protocol/interface identity (status=%d)",
				   identity_status);
			return modem_wan_start_status(identity_status, error_status);
		}

		interface_status = doSystem("ifconfig %s mtu %d up %s", ndis_ifname, ndis_mtu, "0.0.0.0");
		if (interface_status != 0) {
			error_status = set_wan_unit_error_status(unit, 1);
			identity_rollback_status = modem_wan_clear_ndis_identity(
				usb_modem_wan_identity_set_value, &unit);
			if (identity_rollback_status != 0)
				logmessage(LOGNAME,
					   "USB modem WAN interface identity rollback failed after ifconfig error (status=%d)",
					   identity_rollback_status);
			logmessage(LOGNAME,
				   "USB modem WAN interface %s bring-up failed (status=%d)",
				   ndis_ifname, interface_status);
			result_status = modem_wan_start_status(migration_status, interface_status);
			result_status = modem_wan_start_status(result_status, identity_rollback_status);
			return modem_wan_start_status(result_status, error_status);
		}
		firewall_status = start_firewall_ex();
		if (firewall_status != 0)
			logmessage(LOGNAME,
				   "USB modem WAN firewall startup incomplete (status=%d)",
				   firewall_status);
		startup_status = modem_wan_start_status(migration_status, firewall_status);

		control_status = ndis_control_network(ndis_ifname, modem_devnum, 1,
						     &control_required);
		if (modem_ndis_control_failed(control_required, control_status)) {
			error_status = set_wan_unit_error_status(unit, 1);
			logmessage(LOGNAME,
				   "USB modem control-session startup failed on %s (status=%d)",
				   ndis_ifname, control_status);
			result_status = modem_wan_start_status(startup_status, control_status);
			return modem_wan_start_status(result_status, error_status);
		}
		if (control_required)
			sleep(1);

		dhcp_status = start_udhcpc_wan(ndis_ifname, unit, 0);
		error_status = set_wan_unit_error_status(unit, dhcp_status != 0);
		if (dhcp_status != 0)
			logmessage(LOGNAME,
				   "USB modem WAN DHCP client launch failed on %s (status=%d)",
				   ndis_ifname, dhcp_status);
		result_status = modem_wan_start_status(startup_status, dhcp_status);
		return modem_wan_start_status(result_status, error_status);
	}

	identity_status = set_wan_unit_value_status(unit, "ifname_t", "");
	error_status = set_wan_unit_error_status(unit, 1);
	if (identity_status != 0)
		logmessage(LOGNAME,
			   "cannot clear unavailable USB modem WAN interface identity for unit %d (status=%d)",
			   unit, identity_status);
	logmessage(LOGNAME, "USB modem WAN startup failed: selected NDIS interface is unavailable");
	return -1;
}

int
stop_wan_usbnet(void)
{
	int modem_devnum = 0;
	int interface_present, control_status = 0, control_required = 0;
	int ifdown_status;
	int result = 0;
	char ndis_ifname[16] = {0};

	if (!get_modem_ndis_ifname(ndis_ifname, &modem_devnum))
		return 0;

	interface_present = is_interface_exist(ndis_ifname);
	if (interface_present && !is_usbnet_has_module(ndis_ifname, "cdc_mbim")) {
		control_status = ndis_control_network(ndis_ifname, modem_devnum, 0,
						     &control_required);
		if (modem_ndis_control_failed(control_required, control_status)) {
			logmessage(LOGNAME, "failed to disconnect USB modem control session on %s (status=%d)",
				   ndis_ifname, control_status);
			result = -1;
		}
	}

	if (interface_present) {
		ifdown_status = ifconfig(ndis_ifname, 0, "0.0.0.0", NULL);
		if (ifdown_status != 0 && ifdown_status != ENODEV) {
			logmessage(LOGNAME, "failed to bring USB modem interface %s down (status=%d)",
				   ndis_ifname, ifdown_status);
			result = -1;
		}
	}

	return result;
}

static const struct ums_ma_addon_t {
	int vid;
	int pid;
	const char *uMa[10];
} ums_ma_addon[] = {
	{ 0x05c6, 0x1000, {"AnyDATA", "CELOT", "Co.,Ltd", "DGT", "SAMSUNG", "SSE", "StrongRising", "Vertex", NULL} },
	{ 0x0408, 0xf000, {"Yota", NULL} },
	{ 0x0471, 0x1210, {"Philips", NULL} },
	{ 0,      0,      {NULL} }
};

int
launch_usb_modeswitch(int vid, int pid, int inquire)
{
	char eject_file[96], addon[32], vid_arg[8], pid_arg[8];
	char *argv[10];
	const struct ums_ma_addon_t *ua;
	int i, path_len, argc = 0;

	if (vid < 0 || vid > 0xffff || pid < 0 || pid > 0xffff)
		return 1;

	addon[0] = 0;

	for (ua = &ums_ma_addon[0]; ua->vid; ua++) {
		if (ua->vid == vid && ua->pid == pid) {
			usb_info_t *usb_info, *follow_usb;
			
			usb_info = get_usb_info();
			for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
				if (follow_usb->dev_vid == vid && follow_usb->dev_pid == pid && follow_usb->manuf) {
					for (i = 0; ua->uMa[i]; i++) {
						if (strncmp(follow_usb->manuf, ua->uMa[i], strlen(ua->uMa[i])) == 0) {
							snprintf(addon, sizeof(addon), ":uMa=%s", ua->uMa[i]);
							break;
						}
					}
					break;
				}
			}
			free_usb_info(usb_info);
			
			break;
		}
	}

	/* First, check custom rule in /etc/storage. */
	path_len = snprintf(eject_file, sizeof(eject_file), "/etc/storage/%04x:%04x",
			    (unsigned int)vid, (unsigned int)pid);
	if (path_len < 0 || path_len >= (int)sizeof(eject_file))
		return 1;
	if (!check_if_file_exist(eject_file)) {
		path_len = snprintf(eject_file, sizeof(eject_file),
				   "%s/usb_modeswitch.d/%04x:%04x%s",
				   MODEM_SCRIPTS_DIR, (unsigned int)vid,
				   (unsigned int)pid, addon);
		if (path_len < 0 || path_len >= (int)sizeof(eject_file))
			return 1;
		if (!check_if_file_exist(eject_file)) {
			logmessage("usb_modeswitch", "no rule for device %04x:%04x", vid, pid);
			return 1;
		}
	}

	snprintf(vid_arg, sizeof(vid_arg), "0x%04x", (unsigned int)vid);
	snprintf(pid_arg, sizeof(pid_arg), "0x%04x", (unsigned int)pid);
	argv[argc++] = "/bin/usb_modeswitch";
	if (inquire) {
		argv[argc++] = "-D";
		argv[argc++] = "-I";
	}
	argv[argc++] = "-v";
	argv[argc++] = vid_arg;
	argv[argc++] = "-p";
	argv[argc++] = pid_arg;
	argv[argc++] = "-c";
	argv[argc++] = eject_file;
	argv[argc] = NULL;

	return _eval(argv, NULL, 0, NULL);
}

static int
find_usb_device(int vid, int pid)
{
	int i_found = 0;
	usb_info_t *usb_info, *follow_usb;

	usb_info = get_usb_info();
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_vid == vid && follow_usb->dev_pid == pid) {
			i_found = 1;
			break;
		}
	}
	free_usb_info(usb_info);

	return i_found;
}

static void
catch_sig_zerocd(int sig)
{
	if (sig == SIGTERM)
	{
		exit(0);
	}
}

int
zerocd_main(int argc, char **argv)
{
	int i, i_vid, i_pid, res;

	if (argc != 3) {
		printf("Usage: %s [vid] [pid]\n", argv[0]);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTERM, catch_sig_zerocd);

	daemon(0, 0);

	i_vid = strtol(argv[1], NULL, 16);
	i_pid = strtol(argv[2], NULL, 16);

	res = launch_usb_modeswitch(i_vid, i_pid, 1);
	if (res == 0) {
		for (i = 0; i < 6; i++) {
			sleep(1);
			if (!find_usb_device(i_vid, i_pid)) {
				res = 1;
				break;
			}
		}
		if (res == 0)
			launch_usb_modeswitch(i_vid, i_pid, 0);
	}

	return 0;
}
