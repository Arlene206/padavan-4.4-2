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

#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "storage_optware_start_policy.h"

#include <ralink_priv.h>
#include <flash_mtd.h>

#include "rc.h"
#include "service_lifecycle_policy.h"

#define XSTR(s) STR(s)
#define STR(s) #s

long
uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

int
rand_seed_by_time(void)
{
	time_t atime;

	/* make a random number and set the top and bottom bits */
	time(&atime);
	srand((unsigned long)atime);

	return rand();
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *
mac_conv(const char *mac_nvkey, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if (idx < 0)
		snprintf(name, sizeof(name), "%s", mac_nvkey);
	else
		snprintf(name, sizeof(name), "%s%d", mac_nvkey, idx);

	mac = nvram_safe_get(name);

	buf[0] = 0;
	if (strlen(mac) == 12) {
		for (i = 0, j = 0; i < 12; i++) {
			if (i != 0 && (i%2) == 0)
				buf[j++] = ':';
			buf[j++] = mac[i];
		}
		buf[j] = 0;	// oleg patch
	}

	if (strcasecmp(buf, "FF:FF:FF:FF:FF:FF") == 0 || strcmp(buf, "00:00:00:00:00:00") == 0)
		buf[0] = 0;

	return (buf);
}

/* convert mac address format from XX:XX:XX:XX:XX:XX to XXXXXXXXXXXX */
char *
mac_conv2(const char *mac_nvkey, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if (idx < 0)
		snprintf(name, sizeof(name), "%s", mac_nvkey);
	else
		snprintf(name, sizeof(name), "%s%d", mac_nvkey, idx);

	mac = nvram_safe_get(name);

	buf[0] = 0;
	if (strlen(mac) == 17 && strcasecmp(buf, "FF:FF:FF:FF:FF:FF") && strcmp(buf, "00:00:00:00:00:00")) {
		for(i = 0, j = 0; i < 17; ++i) {
			if (i%3 != 2){
				buf[j] = mac[i];
				++j;
			}
			buf[j] = 0;
		}
	}

	return (buf);
}

int
valid_subver(char subfs)
{
	printf("validate subfs: %c\n", subfs);	// tmp test
	if(((subfs >= 'a') && (subfs <= 'z' )) || ((subfs >= 'A') && (subfs <= 'Z' )))
		return 1;
	else
		return 0;
}

void
get_eeprom_params(void)
{
#if defined (VENDOR_ASUS)
	int i;
#endif
	int i_offset, i_ret;
	unsigned char buffer[32];
	unsigned char ea[ETHER_ADDR_LEN];
	char macaddr_wl[]  = "00:11:22:33:44:55";
	char macaddr_rt[]  = "00:11:22:33:44:56";
	char macaddr_lan[] = "00:11:22:33:44:55";
	char macaddr_wan[] = "00:11:22:33:44:56";
	char country_code[4];
	char regspec_code[8];
	char wps_pin[12];
	char productid[16];
	char fwver[16], fwver_sub[32];

#if (BOARD_5G_IN_SOC || !BOARD_HAS_5G_RADIO)
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (i_ret >= 0 && !(buffer[0] & 0x01))
		ether_etoa(buffer, macaddr_wl);

#if BOARD_2G_AS_WSOC
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (i_ret >= 0 && !(buffer[0] & 0x01))
		ether_etoa(buffer, macaddr_rt);

	i_offset = get_wired_mac_e2p_offset(0);
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (buffer[0] & 0x01) {
		if (ether_atoe(macaddr_wl, ea)) {
			memcpy(buffer, ea, ETHER_ADDR_LEN);
			strcpy(macaddr_lan, macaddr_wl);
			if (i_ret >= 0)
				flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, ea, ETHER_ADDR_LEN);
		}
	} else {
		ether_etoa(buffer, macaddr_lan);
	}

	/* store wired LAN MAC */
	memcpy(buffer+ETHER_ADDR_LEN, buffer, ETHER_ADDR_LEN);

	if (get_wired_mac_is_single()) {
		buffer[5] |= 0x03;	// last 2 bits reserved for MBSSID, use 0x03 for WAN (ra1: 0x01, apcli0: 0x02)
		ether_etoa(buffer, macaddr_wan);
	} else {
		i_offset = get_wired_mac_e2p_offset(1);
		memset(buffer, 0xff, ETHER_ADDR_LEN);
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
		if ((buffer[0] & 0x01)
#if defined (USE_SINGLE_MAC)
		    /* compare LAN/WAN MAC OID */
		    || memcmp(buffer, buffer+ETHER_ADDR_LEN, 3) != 0
#endif
		    ) {
			memcpy(buffer, buffer+ETHER_ADDR_LEN, ETHER_ADDR_LEN);
			buffer[5] |= 0x03;
			ether_etoa(buffer, macaddr_wan);
#if !defined (USE_SINGLE_MAC)
			if (i_ret >= 0)
				flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
#endif
		} else {
			ether_etoa(buffer, macaddr_wan);
		}
	}

	nvram_set_temp("il0macaddr", macaddr_lan); // LAN
	nvram_set_temp("il1macaddr", macaddr_wan); // WAN
	nvram_set_temp("wl_macaddr", macaddr_wl);  // 5GHz
	nvram_set_temp("rt_macaddr", macaddr_rt);  // 2.4GHz

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS country code. */
	memset(country_code, 0, sizeof(country_code));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_COUNTRY_CODE, country_code, 2);
	if (i_ret < 0) {
		strcpy(country_code, "GB");
	} else {
		country_code[2] = 0;
		for (i = 1; i >= 0; i--) {
			if ((unsigned char)country_code[i] > 0x7f)
				country_code[i] = 0;
		}
		if (country_code[0] == 0)
			strcpy(country_code, "GB");
	}
#else
	strcpy(country_code, "GB");
#endif

	if (strlen(nvram_wlan_get(0, "country_code")) == 0)
		nvram_wlan_set(0, "country_code", country_code);

	if (strlen(nvram_wlan_get(1, "country_code")) == 0)
		nvram_wlan_set(1, "country_code", country_code);

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS RegSpec code. */
	memset(regspec_code, 0, sizeof(regspec_code));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_REGSPEC_CODE, regspec_code, 4);
	if (i_ret < 0) {
		strcpy(regspec_code, "CE");
	} else {
		regspec_code[4] = 0;
		for (i = 3; i >= 0; i--) {
			if ((unsigned char)regspec_code[i] > 0x7f)
				regspec_code[i] = 0;
		}
		if (!check_regspec_code(regspec_code))
			strcpy(regspec_code, "CE");
	}
#else
	strcpy(regspec_code, "CE");
#endif
	nvram_set_temp("regspec_code", regspec_code);

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS pin code. */
	memset(wps_pin, 0, sizeof(wps_pin));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_PIN_CODE, wps_pin, 8);
	if (i_ret < 0) {
		strcpy(wps_pin, "12345670");
	} else {
		wps_pin[8] = 0;
		for (i = 7; i >= 0; i--) {
			if ((unsigned char)wps_pin[i] < 0x30 ||
			    (unsigned char)wps_pin[i] > 0x39)
				wps_pin[i] = 0;
		}
		if (wps_pin[0] == 0)
			strcpy(wps_pin, "12345670");
	}
#else
	strcpy(wps_pin, "12345670");
#endif
	nvram_set_temp("secret_code", wps_pin);

#if defined(USE_RT3352_MII)
 #define EEPROM_INIC_SIZE (512)
 #define EEPROM_INIC_ADDR 0x8000
	{
		char eeprom[EEPROM_INIC_SIZE];
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, EEPROM_INIC_ADDR, eeprom, sizeof(eeprom));
		if (i_ret < 0) {
			dbg("READ iNIC EEPROM: Out of scope!\n");
		} else {
			FILE *fp;
			if((fp = fopen("/etc/Wireless/iNIC/iNIC_e2p.bin", "w"))) {
				fwrite(eeprom, sizeof(eeprom), 1, fp);
				fclose(fp);
			}
		}
	}
#endif

	/* read firmware header */
	strcpy(fwver, "3.0.0.0");
	strcpy(fwver_sub, fwver);
	snprintf(productid, sizeof(productid), "%s", BOARD_PID);
	memset(buffer, 0, sizeof(buffer));
	i_ret = flash_mtd_read(MTD_PART_NAME_KERNEL, 0x20, buffer, 32);
	if (i_ret < 0) {
		nvram_set_temp("productid", "unknown");
		nvram_set_temp("firmver", "unknown");
	} else {
		strncpy(productid, buffer + 4, 12);
		productid[12] = 0;
		
		if(valid_subver(buffer[27]))
			sprintf(fwver_sub, "%d.%d.%d.%d%c", buffer[0], buffer[1], buffer[2], buffer[3], buffer[27]);
		else
			sprintf(fwver_sub, "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3]);
		
		sprintf(fwver, "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3]);
	}

#if defined(FWBLDSTR)
	if (strlen(FWBLDSTR) > 0 && strlen(FWBLDSTR) <= 4) {
		strcat(fwver_sub, "-");
		strcat(fwver_sub, FWBLDSTR);
	}
#endif
#if defined(FWREVSTR)
	if (strlen(FWREVSTR) > 0 && strlen(FWREVSTR) <= 8) {
		strcat(fwver_sub, "_");
		strcat(fwver_sub, FWREVSTR);
	}
#endif
	nvram_set_temp("productid", trim_r(productid));
	nvram_set_temp("firmver", trim_r(fwver));
	nvram_set_temp("firmver_sub", trim_r(fwver_sub));

#if 0
#if defined (VENDOR_ASUS)
	memset(buffer, 0, 4);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_BOOT_VER, buffer, 4);
	if (i_ret == 0) {
		char blver[32];
		snprintf(blver, sizeof(blver), "%s-0%c-0%c-0%c-0%c", trim_r(productid), buffer[0], buffer[1], buffer[2], buffer[3]);
		nvram_set_temp("blver", trim_r(blver));
	}
#endif
#endif

	// TXBF, not used yet
	{
		int i, count_0xff = 0;
		unsigned char txbf_para[33];
		memset(txbf_para, 0, sizeof(txbf_para));
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_TXBF_PARA, txbf_para, 33);
		if (i_ret < 0) {
			dbg("READ TXBF PARA address: Out of scope\n");
		} else {
			for (i = 0; i < 33; i++) {
				if (txbf_para[i] == 0xff)
					count_0xff++;
			}
		}
		nvram_wlan_set_int(1, "txbf_en", (count_0xff == 33) ? 0 : 1);
		if(count_0xff !=33)
		{
		 count_0xff = 0;
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, 0x81A0, txbf_para, 33);
		if (i_ret < 0) {
			dbg("READ TXBF PARA address: Out of scope\n");
		} else {
			for (i = 0; i < 33; i++) {
				if (txbf_para[i] == 0xff)
					count_0xff++;
			}
		}
		nvram_wlan_set_int(1, "txbf_en", (count_0xff == 33) ? 0 : 1);
		}
	}
}

void
update_router_mode(void)
{
	if (nvram_get_int("sw_mode") != 3) {
		if (nvram_get_int("wan_nat_x") == 0)
			nvram_set_int("sw_mode", 4);	// Gateway mode
		else
			nvram_set_int("sw_mode", 1);	// Router mode
	}
}

void
set_pagecache_reclaim(void)
{
	int pagecache_ratio = 100;
	int pagecache_reclaim = nvram_get_int("pcache_reclaim");

	switch (pagecache_reclaim)
	{
	case 1:
		pagecache_ratio = 30;
		break;
	case 2:
		pagecache_ratio = 50;
		break;
	case 3:
		pagecache_ratio = 70;
		break;
	case 4:
		pagecache_ratio = 85;
		break;
	}

	fput_int("/proc/sys/vm/pagecache_ratio", pagecache_ratio);
}

void
restart_all_sysctl(void)
{
	if (!get_ap_mode()) {
		set_nf_conntrack();
		set_tcp_syncookies();
		set_igmp_mld_version();
		set_passthrough_pppoe(1);
	}

#if defined(APP_SMBD)
	config_smb_fastpath(1);
#endif
}

void
char_to_ascii(char *output, char *input)
{
	int i;
	char tmp[10];
	char *ptr;

	ptr = output;
	for (i = 0; i < strlen(input); i++) {
		if ((input[i] >= '0' && input[i] <= '9')
			||(input[i] >= 'A' && input[i] <= 'Z')
			||(input[i] >= 'a' && input[i] <= 'z')
			|| input[i] == '!' || input[i] == '*'
			|| input[i] == '(' || input[i] == ')'
			|| input[i] == '_' || input[i] == '-'
			|| input[i] == '\'' || input[i] == '.') {
			*ptr = input[i];
			ptr ++;
		} else {
			sprintf(tmp, "%%%.02X", input[i]);
			strcpy(ptr, tmp);
			ptr += 3;
		}
	}
	*(ptr) = '\0';
}

int
fput_string(const char *name, const char *value)
{
	FILE *fp;

	fp = fopen(name, "w");
	if (fp) {
		fputs(value, fp);
		fclose(fp);
		return 0;
	} else {
		return errno;
	}
}

int
fput_int(const char *name, int value)
{
	char svalue[32];
	sprintf(svalue, "%d", value);
	return fput_string(name, svalue);
}

unsigned int
get_param_int_hex(const char *param)
{
	unsigned int retVal = 0;

	/* check HEX */
	if (strlen(param) > 2 && param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))
		retVal = strtoul(param+2, NULL, 16);
	else
		retVal = atoi(param);

	return retVal;
}

static int
is_param_forbidden(const char *line, const char **forbid_list)
{
	while (*forbid_list) {
		if (strncmp(line, *forbid_list, strlen(*forbid_list)) == 0)
			return 1;
		forbid_list++;
	}
	return 0;
}

void
load_user_config(FILE *fp, const char *dir_name, const char *file_name, const char **forbid_list)
{
	FILE *fp_user;
	char line[256], real_path[128];
	int path_len;

	if (!fp || !dir_name || !file_name)
		return;

	path_len = snprintf(real_path, sizeof(real_path), "%s/%s", dir_name, file_name);
	if (path_len < 0 || path_len >= (int)sizeof(real_path))
		return;

	fp_user = fopen(real_path, "r");
	if (fp_user) {
		while (fgets(line, sizeof(line), fp_user)) {
			size_t len = strlen(line);

			if (len == 0 || line[0] == '\r' || line[0] == '\n' ||
			    line[0] == '#' || line[0] == ';')
				continue;

			if (forbid_list && is_param_forbidden(line, forbid_list))
				continue;

			if (line[len - 1] != '\n') {
				if (len + 1 >= sizeof(line))
					continue;
				line[len++] = '\n';
				line[len] = '\0';
			}

			fputs(line, fp);
		}

		fclose(fp_user);
	}
}

static int
is_safe_module_name(const char *name)
{
	const unsigned char *p = (const unsigned char *)name;

	if (!p || !*p)
		return 0;
	for (; *p; p++) {
		if (!isalnum(*p) && *p != '_' && *p != '-')
			return 0;
	}
	return 1;
}

static int
module_parameter_path(char *path, size_t path_size,
		      const char *module_name, const char *module_param)
{
	int path_len;

	if (!is_safe_module_name(module_name) ||
	    !is_safe_module_name(module_param))
		return -1;
	path_len = snprintf(path, path_size, "/sys/module/%s/parameters/%s",
			    module_name, module_param);
	if (path_len < 0 || path_len >= (int)path_size)
		return -1;
	return 0;
}

int
is_module_loaded(const char *module_name)
{
	DIR *dir_to_open;
	char mod_path[64];
	int path_len;

	if (!is_safe_module_name(module_name))
		return 0;
	path_len = snprintf(mod_path, sizeof(mod_path), "/sys/module/%s", module_name);
	if (path_len < 0 || path_len >= (int)sizeof(mod_path))
		return 0;
	dir_to_open = opendir(mod_path);
	if (dir_to_open) {
		closedir(dir_to_open);
		return 1;
	}

	return 0;
}

int
get_module_refcount(const char *module_name)
{
	FILE *fp;
	char mod_path[64], mod_refval[16], *end;
	long refcount;
	int path_len;

	if (!is_safe_module_name(module_name))
		return -1;
	path_len = snprintf(mod_path, sizeof(mod_path), "/sys/module/%s/refcnt", module_name);
	if (path_len < 0 || path_len >= (int)sizeof(mod_path))
		return -1;
	fp = fopen(mod_path, "r");
	if (!fp)
		return -1;

	if (!fgets(mod_refval, sizeof(mod_refval), fp)) {
		fclose(fp);
		return -1;
	}
	fclose(fp);

	errno = 0;
	refcount = strtol(mod_refval, &end, 10);
		if (errno == ERANGE || end == mod_refval || (*end && *end != '\n') ||
	    refcount < 0 || refcount > INT_MAX)
		return -1;

	return (int)refcount;
}

int
module_smart_load(const char *module_name, const char *module_param)
{
	char *param_copy = NULL, *cursor, *token;
	char *argv[32];
	int argc = 0, ret;

	if (!is_safe_module_name(module_name))
		return 0;
	if (is_module_loaded(module_name))
		return 0;

	argv[argc++] = "modprobe";
	argv[argc++] = "-q";
	argv[argc++] = (char *)module_name;

	if (module_param && *module_param) {
		param_copy = strdup(module_param);
		if (!param_copy)
			return 0;
		cursor = param_copy;
		while ((token = strsep(&cursor, " \t\r\n")) != NULL) {
			const unsigned char *p;
			if (!*token)
				continue;
			if (token[0] == '-' || argc >= (int)(sizeof(argv) / sizeof(argv[0])) - 1) {
				free(param_copy);
				return 0;
			}
			for (p = (const unsigned char *)token; *p; p++) {
				if (*p < 32 || *p == 127) {
					free(param_copy);
					return 0;
				}
			}
			argv[argc++] = token;
		}
	}
	argv[argc] = NULL;
	ret = _eval(argv, NULL, 0, NULL);
	free(param_copy);

	return (ret == 0) ? 1 : 0;
}

int
module_smart_unload(const char *module_name, int recurse_unload)
{
	int ret, refcount;
	char *modprobe_argv[] = { "modprobe", "-r", (char *)module_name, NULL };
	char *rmmod_argv[] = { "rmmod", (char *)module_name, NULL };

	if (!is_safe_module_name(module_name))
		return 0;
	refcount = get_module_refcount(module_name);

	/* check module not loaded */
	if (refcount < 0)
		return 0;

	/* check module is used */
	if (refcount > 0)
		return 1;

	ret = recurse_unload ? _eval(modprobe_argv, NULL, 0, NULL) :
		_eval(rmmod_argv, NULL, 0, NULL);

	return (ret == 0) ? 1 : 0;
}

int
module_param_get(const char *module_name, const char *module_param, char *param_value, size_t param_value_size)
{
	FILE *fp;
	char mod_path[256];
	size_t len;
	int next_char;

	if (!param_value || param_value_size < 2 || param_value_size > INT_MAX ||
	    module_parameter_path(mod_path, sizeof(mod_path), module_name, module_param) < 0)
		return -1;

	fp = fopen(mod_path, "r");
	if (!fp)
		return -1;

	if (!fgets(param_value, (int)param_value_size, fp)) {
		fclose(fp);
		return -1;
	}

	len = strlen(param_value);
	if (len > 0 && param_value[len - 1] == '\n') {
		param_value[--len] = '\0';
	} else if (len == param_value_size - 1) {
		next_char = fgetc(fp);
		if (next_char != EOF && next_char != '\n') {
			fclose(fp);
			param_value[0] = '\0';
			return -1;
		}
	}

	if (ferror(fp)) {
		fclose(fp);
		param_value[0] = '\0';
		return -1;
	}
	fclose(fp);

	return 0;
}
int
module_param_set_int(const char *module_name, const char *module_param, int param_value)
{
	char mod_path[256];

	if (module_parameter_path(mod_path, sizeof(mod_path), module_name, module_param) < 0)
		return -1;

	return fput_int(mod_path, param_value);
}

void
oom_score_adjust(pid_t pid, int oom_score_adj)
{
	char proc_path[40];
	int path_len;

	if (pid <= 0)
		return;
	path_len = snprintf(proc_path, sizeof(proc_path), "/proc/%u/oom_score_adj", (unsigned int)pid);
	if (path_len < 0 || path_len >= (int)sizeof(proc_path))
		return;
	fput_int(proc_path, oom_score_adj);
}

#if defined (USE_NAND_FLASH)
void mount_rwfs_partition(void)
{
	int mtd_rwfs;
	char dev_ubi[16];
	const char *mp_rwfs = MP_MTD_RWFS;

	if (nvram_get_int("mtd_rwfs_mount") == 0)
		return;

	mtd_rwfs = mtd_dev_idx(MTD_PART_NAME_RWFS);
	if (mtd_rwfs < 3)
		return;

	module_smart_load("ubi", NULL);

	if (!is_module_loaded("ubi"))
		return;

	if (doSystem("ubiattach -p /dev/mtd%d -d %d", mtd_rwfs, mtd_rwfs) == 0) {
		mkdir(mp_rwfs, 0755);
		snprintf(dev_ubi, sizeof(dev_ubi), "/dev/ubi%d_0", mtd_rwfs);
		mount(dev_ubi, mp_rwfs, "ubifs", 0, NULL);
	}
}

void umount_rwfs_partition(void)
{
	int mtd_rwfs;
	const char *mp_rwfs = MP_MTD_RWFS;

	if (check_if_dir_exist(mp_rwfs)) {
		/*
		 * opt-umount uses the nonblocking lifecycle lock. If MTD startup
		 * still owns it, preserve the mounted filesystem and UBI attachment.
		 */
		if (doSystem("/usr/bin/opt-umount.sh %s %s", "/dev/ubi", mp_rwfs) != 0) {
			logmessage(LOGNAME, "refusing to unmount RWFS: Optware teardown did not complete");
			return;
		}

		if (umount(mp_rwfs) == 0) {
			rmdir(mp_rwfs);
		} else if (errno != EINVAL && errno != ENOENT) {
			logmessage(LOGNAME, "cannot unmount RWFS at %s (errno %d); keeping UBI attached",
				   mp_rwfs, errno);
			return;
		}
	}

	mtd_rwfs = mtd_dev_idx(MTD_PART_NAME_RWFS);
	if (mtd_rwfs < 3)
		return;

	if (!is_module_loaded("ubi"))
		return;

	if (doSystem("ubidetach -p /dev/mtd%d 2>/dev/null", mtd_rwfs) != 0)
		logmessage(LOGNAME, "failed to detach RWFS UBI volume on MTD%d", mtd_rwfs);
}

static int rwfs_optware_result_fd = -1;
static int rwfs_optware_start_pending = 0;
static pid_t rwfs_optware_worker_pid = -1;
static unsigned int rwfs_optware_retry_delay =
	STORAGE_OPTWARE_RETRY_INITIAL_SECONDS;
static long rwfs_optware_retry_at = 0;
static char rwfs_optware_device[16];

static void
rwfs_optware_schedule_retry(void)
{
	if (rwfs_optware_retry_at != 0)
		return;

	rwfs_optware_retry_at = uptime() + (long)rwfs_optware_retry_delay;
	rwfs_optware_retry_delay =
		storage_optware_retry_next_delay(rwfs_optware_retry_delay);
}

static void
rwfs_optware_start_reset_child_signals(void)
{
	int signal_number;
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);
	for (signal_number = 1; signal_number < _NSIG; signal_number++) {
		if (signal_number == SIGKILL || signal_number == SIGSTOP)
			continue;
		(void)sigaction(signal_number, &action, NULL);
	}
}

static int
rwfs_optware_start_result_read(void)
{
	unsigned char child_status = 1;
	ssize_t bytes_read;
	int result;

	if (rwfs_optware_result_fd < 0)
		return STORAGE_OPTWARE_START_CHILD_NONE;

	do {
		bytes_read = read(rwfs_optware_result_fd, &child_status,
				  sizeof(child_status));
	} while (bytes_read < 0 && errno == EINTR);

	if (bytes_read < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return STORAGE_OPTWARE_START_CHILD_RUNNING;
		logmessage(LOGNAME, "cannot read MTD Optware startup result (errno %d)", errno);
		bytes_read = 0;
	}

	result = storage_optware_start_decode_result((int)bytes_read,
						     (int)child_status);
	close(rwfs_optware_result_fd);
	rwfs_optware_result_fd = -1;
	rwfs_optware_worker_pid = -1;

	return result;
}

static int
rwfs_optware_start_worker(void)
{
	int pipe_fd[2], flags, command_status, command_len;
	char command[128];
	unsigned char child_result;
	ssize_t written;
	pid_t child;

	/* The parent owns at most one outstanding MTD startup result channel. */
	if (rwfs_optware_result_fd >= 0)
		return 0;

	command_len = snprintf(command, sizeof(command),
			       "/usr/bin/opt-mount.sh %s %s",
			       rwfs_optware_device, MP_MTD_RWFS);
	if (command_len < 0 || command_len >= (int)sizeof(command)) {
		logmessage(LOGNAME, "MTD Optware startup command is too long");
		return -1;
	}

	if (pipe(pipe_fd) != 0) {
		logmessage(LOGNAME, "cannot create MTD Optware result pipe (errno %d)", errno);
		return -1;
	}

	flags = fcntl(pipe_fd[0], F_GETFL, 0);
	if (flags < 0 || fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK) != 0)
		goto failed;
	flags = fcntl(pipe_fd[1], F_GETFD);
	if (flags < 0 || fcntl(pipe_fd[1], F_SETFD, flags | FD_CLOEXEC) != 0)
		goto failed;

	child = fork();
	if (child < 0)
		goto failed;

	if (child == 0) {
		sigset_t child_signal_mask;

		close(pipe_fd[0]);
		if (setpgid(0, 0) != 0) {
			child_result = 1;
			do {
				written = write(pipe_fd[1], &child_result, sizeof(child_result));
			} while (written < 0 && errno == EINTR);
			close(pipe_fd[1]);
			(void)kill(1, SIGALRM);
			_exit(1);
		}
		rwfs_optware_start_reset_child_signals();
		sigemptyset(&child_signal_mask);
		if (sigprocmask(SIG_SETMASK, &child_signal_mask, NULL) != 0) {
			child_result = 1;
			do {
				written = write(pipe_fd[1], &child_result, sizeof(child_result));
			} while (written < 0 && errno == EINTR);
			close(pipe_fd[1]);
			(void)kill(1, SIGALRM);
			_exit(1);
		}

		command_status = system(command);
		child_result = (command_status != -1 &&
				WIFEXITED(command_status) &&
				WEXITSTATUS(command_status) == 0) ? 0 : 1;
		do {
			written = write(pipe_fd[1], &child_result, sizeof(child_result));
		} while (written < 0 && errno == EINTR);
		close(pipe_fd[1]);
		/* Wake PID 1 only after the actual opt-mount result is published. */
		(void)kill(1, SIGALRM);
		_exit(child_result == 0 ? 0 : 1);
	}

	/* Create a private process group so shutdown can stop the whole hook tree. */
	if (setpgid(child, child) != 0 && errno != EACCES && errno != ESRCH)
		logmessage(LOGNAME, "cannot set MTD Optware worker process group (errno %d)", errno);
	close(pipe_fd[1]);
	rwfs_optware_result_fd = pipe_fd[0];
	rwfs_optware_worker_pid = child;
	logmessage(LOGNAME, "started MTD Optware startup worker (pid %d)", (int)child);
	return 0;

failed:
	logmessage(LOGNAME, "cannot launch MTD Optware startup worker (errno %d)", errno);
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	return -1;
}

/*
 * Called only in PID 1's deferred event loop; never blocks on external hooks.
 * Poll every five seconds, but exponentially back off actual relaunch attempts.
 */
int
retry_rwfs_optware(void)
{
	int child_result;

	if (!rwfs_optware_start_pending)
		return 0;

	child_result = rwfs_optware_start_result_read();
	if (child_result == STORAGE_OPTWARE_START_CHILD_RUNNING)
		return 1;
	if (child_result == STORAGE_OPTWARE_START_CHILD_SUCCESS) {
		rwfs_optware_start_pending = 0;
		rwfs_optware_retry_at = 0;
		rwfs_optware_retry_delay = STORAGE_OPTWARE_RETRY_INITIAL_SECONDS;
		logmessage(LOGNAME, "MTD Optware startup completed successfully");
		return 0;
	}
	if (child_result == STORAGE_OPTWARE_START_CHILD_FAILURE)
		logmessage(LOGNAME, "MTD Optware startup failed; preserving pending state for retry");

	rwfs_optware_schedule_retry();
	if (uptime() < rwfs_optware_retry_at)
		return 1;

	rwfs_optware_retry_at = 0;
	if (rwfs_optware_start_worker() != 0) {
		logmessage(LOGNAME, "MTD Optware startup worker could not be relaunched; will retry");
		rwfs_optware_schedule_retry();
	}

	return 1;
}

void start_rwfs_optware(void)
{
	int mtd_rwfs;

	if (rwfs_optware_start_pending || !check_if_dir_exist(MP_MTD_RWFS))
		return;

	mtd_rwfs = mtd_dev_idx(MTD_PART_NAME_RWFS);
	if (mtd_rwfs < 3)
		return;

	snprintf(rwfs_optware_device, sizeof(rwfs_optware_device),
		 "/dev/ubi%d_0", mtd_rwfs);
	rwfs_optware_start_pending = 1;
	if (rwfs_optware_start_worker() != 0) {
		logmessage(LOGNAME, "initial MTD Optware startup launch failed; will retry");
		rwfs_optware_schedule_retry();
	}
	/* Poll the nonblocking result channel even if the worker does not signal. */
	alarm(5);
}

/*
 * Stop the boot worker before ordinary services during shutdown. The worker
 * and its shell/hooks share a private process group; terminate that group
 * before stopping router services. RWFS teardown remains the fail-closed
 * backstop if a hook cannot be stopped or the lifecycle lock stays busy.
 */
void
stop_rwfs_optware(void)
{
	sigset_t child_mask, old_mask;
	pid_t child, waited;
	int status, group_status;

	rwfs_optware_start_pending = 0;
	rwfs_optware_retry_at = 0;
	rwfs_optware_retry_delay = STORAGE_OPTWARE_RETRY_INITIAL_SECONDS;

	sigemptyset(&child_mask);
	sigaddset(&child_mask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &child_mask, &old_mask) != 0) {
		logmessage(LOGNAME, "cannot block SIGCHLD while stopping MTD Optware startup (errno %d)", errno);
		return;
	}

	child = rwfs_optware_worker_pid;
	if (child > 0) {
		do {
			waited = waitpid(child, &status, WNOHANG);
		} while (waited < 0 && errno == EINTR);

		if (waited == 0 || waited == child ||
		    (waited < 0 && errno == ECHILD)) {
			group_status = kill(-child, SIGTERM);
			if (group_status != 0 && waited == 0)
				(void)kill(child, SIGTERM);
			sleep(1);

			if (waited == 0) {
				do {
					waited = waitpid(child, &status, WNOHANG);
				} while (waited < 0 && errno == EINTR);
			}
			if (waited == 0) {
				if (kill(-child, SIGKILL) != 0)
					(void)kill(child, SIGKILL);
			} else {
				/* The worker may have exited with background hooks still in its group. */
				(void)kill(-child, SIGKILL);
			}
		} else if (waited < 0) {
			logmessage(LOGNAME, "cannot inspect MTD Optware startup worker (errno %d)", errno);
		}
	}

	rwfs_optware_worker_pid = -1;
	if (rwfs_optware_result_fd >= 0) {
		close(rwfs_optware_result_fd);
		rwfs_optware_result_fd = -1;
	}
	if (sigprocmask(SIG_SETMASK, &old_mask, NULL) != 0)
		logmessage(LOGNAME, "cannot restore SIGCHLD mask after stopping MTD Optware startup (errno %d)", errno);
}
#else
inline void mount_rwfs_partition(void) {}
inline void umount_rwfs_partition(void) {}
inline void start_rwfs_optware(void) {}
inline int retry_rwfs_optware(void) { return 0; }
inline void stop_rwfs_optware(void) {}
#endif

static enum process_presence
service_lifecycle_scan(void *context, const char *service_name,
		       struct process_snapshot *snapshot)
{
	(void)context;
	return process_inventory_scan(service_name, snapshot);
}

static enum process_presence
service_lifecycle_validate(void *context, const char *service_name,
			   const struct process_identity *identity)
{
	(void)context;
	return process_identity_validate(service_name, identity);
}

static int
service_lifecycle_send_signal(void *context,
			      const struct process_identity *identity,
			      int signal_number)
{
	(void)context;
	if (signal_number != SIGTERM && signal_number != SIGKILL)
		return -1;
	if (kill(identity->pid, signal_number) == 0 || errno == ESRCH)
		return 0;
	return -1;
}

static void
service_lifecycle_sleep(void *context, unsigned int seconds)
{
	(void)context;
	sleep(seconds);
}

int
kill_services(char* svc_name[], int wtimeout, int forcekill)
{
	static const struct service_lifecycle_ops ops = {
		service_lifecycle_scan,
		service_lifecycle_validate,
		service_lifecycle_send_signal,
		service_lifecycle_sleep
	};
	int i, result;

	result = service_lifecycle_stop(svc_name, wtimeout, forcekill, &ops, NULL);
	if (result != SERVICE_LIFECYCLE_STOPPED) {
		for (i = 0; svc_name && svc_name[i] && *svc_name[i]; i++) {
			struct process_snapshot snapshot;
			enum process_presence state;

			state = process_inventory_scan(svc_name[i], &snapshot);
			if (state == PROCESS_PRESENCE_UNKNOWN)
				logmessage(LOGNAME, "service inventory unknown: %s (errno %d)",
					   svc_name[i], snapshot.error);
			else if (state == PROCESS_PRESENCE_RUNNING)
				logmessage(LOGNAME, "service stop incomplete: %s has %lu process(es)",
					   svc_name[i], (unsigned long)snapshot.count);
			process_snapshot_release(&snapshot);
		}
	}
	return result;
}

int
kill_process_pidfile(char *pidfile, int wtimeout, int forcekill)
{
	int i, result = -1;

	if (wtimeout < 1)
		wtimeout = 1;

	for (i=0; i<wtimeout; i++) {
		if (kill_pidfile(pidfile) != 0)
			break;
		result = 0; // process exist
		sleep(1);
	}

	if (i == wtimeout && forcekill) {
		if (kill_pidfile(pidfile) == 0)
			kill_pidfile_s(pidfile, SIGKILL);
	}

	return result;
}

int
create_file(const char *fn)
{
	int fd = open(fn, O_RDWR | O_CREAT, 0666);
	if (fd >= 0) {
		close(fd);
		return 0;
	}

	return 1;
}

int
check_if_dir_exist(const char *dirpath)
{
	struct stat stat_buf;

	if (!stat(dirpath, &stat_buf))
		return S_ISDIR(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_file_exist(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf))
		return S_ISREG(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_dev_exist(const char *devpath)
{
	struct stat stat_buf;

	if (!stat(devpath, &stat_buf))
		return (S_ISCHR(stat_buf.st_mode) | S_ISBLK(stat_buf.st_mode));
	else
		return 0;
}

static int
mkdir_component_if_missing(const char *path, mode_t mode, int *created)
{
	struct stat st;

	if (created)
		*created = 0;
	if (mkdir(path, mode) == 0) {
		if (created)
			*created = 1;
		return 0;
	}
	if (errno != EEXIST)
		return -1;
	if (stat(path, &st) < 0)
		return -1;
	if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	return 0;
}

int
mkdir_if_none(const char *dirpath, const char *mode)
{
	char *path, *p, *end;
	struct stat st;
	unsigned long parsed_mode;
	size_t len;
	int ret = 0, created;

	if (!dirpath || !*dirpath || !mode || !*mode)
		return 0;

	errno = 0;
	parsed_mode = strtoul(mode, &end, 8);
	if (errno == ERANGE || end == mode || *end || parsed_mode > 07777)
		return 0;

	path = strdup(dirpath);
	if (!path)
		return 0;

	len = strlen(path);
	while (len > 1 && path[len - 1] == '/')
		path[--len] = '\0';

	if (stat(path, &st) == 0) {
		free(path);
		return 0;
	}
	if (errno != ENOENT) {
		free(path);
		return 0;
	}

	if (len == 1 && path[0] == '/') {
		free(path);
		return 0;
	}

	for (p = path + (path[0] == '/' ? 1 : 0); *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (*path && mkdir_component_if_missing(path, 0777, NULL) < 0)
				goto out;
			*p = '/';
		}
	}

	if (mkdir_component_if_missing(path, (mode_t)parsed_mode, &created) == 0)
		ret = created;

out:
	free(path);
	return ret;
}

int
rename_if_dir_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[257], newpath[257];
	int old_len, new_len, renamed;

	if (!dir || !subdir)
		return 0;

	dirp = opendir(dir);
	if (!dirp)
		return 0;

	while ((direntp = readdir(dirp))) {
		if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir)) {
			old_len = snprintf(oldpath, sizeof(oldpath), "%s/%s", dir, direntp->d_name);
			new_len = snprintf(newpath, sizeof(newpath), "%s/%s", dir, subdir);
			renamed = 0;
			if (old_len >= 0 && old_len < (int)sizeof(oldpath) &&
			    new_len >= 0 && new_len < (int)sizeof(newpath))
				renamed = (rename(oldpath, newpath) == 0);
			closedir(dirp);
			return renamed;
		}
	}

	closedir(dirp);
	return 0;
}

char *
if_dircase_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[257];
	int path_len;
	char *result;

	if (!dir || !subdir)
		return NULL;

	dirp = opendir(dir);
	if (!dirp)
		return NULL;

	while ((direntp = readdir(dirp))) {
		if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir)) {
			path_len = snprintf(oldpath, sizeof(oldpath), "%s/%s", dir, direntp->d_name);
			if (path_len < 0 || path_len >= (int)sizeof(oldpath)) {
				closedir(dirp);
				return NULL;
			}
			result = strdup(oldpath);
			closedir(dirp);
			return result;
		}
	}

	closedir(dirp);
	return NULL;
}

unsigned long
file_size(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf) && S_ISREG(stat_buf.st_mode))
		return ((unsigned long) stat_buf.st_size);

	return 0;
}

// 1: add, 0: remove.
int
get_hotplug_action(const char *action)
{
	if (!strcmp(action, "remove"))
		return 0;

	return 1;
}
