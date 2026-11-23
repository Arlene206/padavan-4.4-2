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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>

#include <rstats.h>
#if defined (USE_STORAGE)
#include <disk_initial.h>
#endif

#include "rc.h"
#include "modem_module_reload_policy.h"
#include "modem_wan_start_policy.h"
#include "storage_hotplug_name_policy.h"
#include "firewall_startup_policy.h"
#include "firewall_nvram_match_policy.h"
#include "rc_nvram_policy.h"
#include "wan_nvram_status_policy.h"
#include "ddns_enabled_policy.h"
#include "router_mode_nvram_policy.h"
#include "rc_nvram_legacy_migration_policy.h"
#include "storage_persistence_policy.h"
#include "service_start_status.h"
#include "crontab_update_status.h"
#include "wireless_module_load_policy.h"
#include "vpn_server_lifecycle_lock.h"
#include "rc_notify_batch_policy.h"
#include "service_wait_policy.h"
#include "init_router_result_policy.h"
#include "router_runtime_nvram_reset_policy.h"
#include "wireless_radio_policy.h"
#include "ddns_restart_policy.h"
#include "timezone_env_policy.h"
#if defined (USE_IPV6)
#include "ipv6_restart_policy.h"
#endif
#include <rc_notify_queue.h>
#include "gpio_pins.h"
#include "switch.h"
#include <ralink_priv.h>
#include <gpioutils.h>

extern struct nvram_pair router_defaults[];

/* Keep NVRAM device operations injectable for the host-side policy test. */
static int
rc_nvram_policy_clear(void *context)
{
    (void)context;
    return nvram_clear();
}

static int
rc_nvram_policy_get(void *context, const char *name, char **value)
{
    (void)context;
    return nvram_get_checked(name, value);
}

static int
rc_nvram_policy_set(void *context, const char *name, const char *value)
{
    (void)context;
    return nvram_set(name, value);
}

static int
rc_nvram_policy_commit(void *context)
{
    (void)context;
    return nvram_commit();
}

static int
rc_router_runtime_nvram_set(void *context, const char *name,
			    const char *string_value, int integer_value,
			    int is_integer)
{
	(void)context;
	if (is_integer)
		return nvram_set_int_temp(name, integer_value);
	return nvram_set_temp(name, string_value);
}

static int
reset_router_runtime_nvram_state(int sw_mode)
{
#define ROUTER_RUNTIME_STRING(name, value) { name, value, 0, 0 }
#define ROUTER_RUNTIME_INTEGER(name, value) { name, NULL, value, 1 }
	const struct router_runtime_nvram_entry entries[] = {
		ROUTER_RUNTIME_STRING("login_timestamp", "0000000000"),
		ROUTER_RUNTIME_INTEGER("networkmap_fullscan", 0),
		ROUTER_RUNTIME_INTEGER("link_internet", 2),
		ROUTER_RUNTIME_INTEGER("link_wan", 0),
		ROUTER_RUNTIME_INTEGER("led_front_t", 1),
		ROUTER_RUNTIME_INTEGER("led_ether_t", 1),
		ROUTER_RUNTIME_INTEGER("reload_svc_wl", 0),
		ROUTER_RUNTIME_INTEGER("reload_svc_rt", 0),
		ROUTER_RUNTIME_INTEGER("usb_hotplug_ms", 0),
		ROUTER_RUNTIME_INTEGER("usb_hotplug_lp", 0),
		ROUTER_RUNTIME_INTEGER("usb_hotplug_md", 0),
		ROUTER_RUNTIME_INTEGER("usb_unplug_lp", 0),
		ROUTER_RUNTIME_INTEGER("usb_unplug_md", 0),
		ROUTER_RUNTIME_INTEGER("usb_opt_start", 0),
		ROUTER_RUNTIME_INTEGER("l2tp_wan_t", 0),
		ROUTER_RUNTIME_INTEGER("l2tp_cli_t", 0),
		ROUTER_RUNTIME_INTEGER("l2tp_srv_t", 0),
		ROUTER_RUNTIME_INTEGER("vpnc_state_t", 0),
		ROUTER_RUNTIME_STRING("deferred_wanup_t", "0000000000"),
		ROUTER_RUNTIME_STRING("vpnc_dns_t", ""),
		ROUTER_RUNTIME_STRING("vpnc_dom_t", ""),
		ROUTER_RUNTIME_STRING("viptv_ifname", ""),
		ROUTER_RUNTIME_STRING(RSTATS_NVKEY_24,
				      (sw_mode != 3) ? IFDESC_WAN : IFDESC_LAN),
		ROUTER_RUNTIME_STRING(RSTATS_NVKEY_DM, IFDESC_WAN)
	};
#undef ROUTER_RUNTIME_STRING
#undef ROUTER_RUNTIME_INTEGER

	return router_runtime_nvram_reset_apply(
		entries, sizeof(entries) / sizeof(entries[0]),
		rc_router_runtime_nvram_set, NULL);
}

static const struct rc_nvram_policy_ops rc_nvram_ops = {
    rc_nvram_policy_clear,
    rc_nvram_policy_get,
    rc_nvram_policy_set,
    rc_nvram_policy_commit
};

/* static values */

static int nvram_modem_type = 0;
static int nvram_modem_rule = 0;

#if defined (USE_USB_SUPPORT)
static int rc_boot_nvram_read_int_status(const char *name,
					 int missing_value, int *value);
#endif
static int nvram_ipv6_type = 0;
static unsigned int rc_deferred_hotplug_retry_mask = 0;

static int
nvram_restore_defaults(void)
{
	int restore_defaults;
	int need_commit, ipv6_type_status;
#if defined (USE_USB_SUPPORT)
	int modem_settings_status, modem_rule = 0, modem_type = 0;
#endif
	char lan_mac[] = "FFFF";
	unsigned char buffer[2] = {0};
	int i_offset = get_wired_mac_e2p_offset(0) + 4;

	if (flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, 2) == 0)
		snprintf(lan_mac, sizeof(lan_mac), "%02X%02X",
			 buffer[0] & 0xff, buffer[1] & 0xff);

	/* A failed NVRAM read must not be mistaken for an absent first-boot key. */
	if (rc_nvram_should_restore_defaults(&rc_nvram_ops, NULL,
					     &restore_defaults) < 0) {
		logmessage(LOGNAME, "NVRAM restore-state read failed; aborting router startup");
		return -1;
	}

	need_commit = rc_nvram_apply_defaults(restore_defaults, router_defaults,
					      lan_mac, &rc_nvram_ops, NULL);
	if (need_commit < 0) {
		logmessage(LOGNAME, "NVRAM clear/default write failed; aborting router startup");
		return -1;
	}

	klogctl(8, NULL, nvram_get_int("console_loglevel"));

	/* Load modem settings as one checked pair so a backend error cannot
	 * become a false zero baseline for a later destructive module reload. */
#if defined (USE_USB_SUPPORT)
	modem_settings_status = rc_modem_read_nvram_settings_status(
		rc_boot_nvram_read_int_status, &modem_rule, &modem_type);
	if (modem_settings_status != 0) {
		errno = modem_settings_status;
		logmessage(LOGNAME,
			   "cannot read USB modem configuration; aborting router startup (status=%d)",
			   modem_settings_status);
		return -1;
	}
	nvram_modem_type = modem_type;
	nvram_modem_rule = modem_rule;
#else
	nvram_modem_type = 0;
	nvram_modem_rule = 0;
#endif
	ipv6_type_status = get_ipv6_type_status(&nvram_ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "cannot read IPv6 service configuration; aborting router startup (status=%d)",
			   ipv6_type_status);
		return -1;
	}

	return need_commit;
}

static int
load_boot_module(const char *module_name, const char *module_param)
{
	static const rc_module_load_ops_t ops = {
		module_smart_load,
		is_module_loaded_status
	};
	int status;

	status = rc_module_load_verified(module_name, module_param, &ops);
	if (status != 0)
		logmessage(LOGNAME,
			   "boot hardware module load failed or module is unavailable: %s",
			   module_name);
	return status;
}

static int
load_wireless_module(const char *module_name)
{
	static const rc_module_load_ops_t ops = {
		module_smart_load,
		is_module_loaded_status
	};
	int status;

	status = rc_module_load_verified(module_name, NULL, &ops);
	if (status != 0)
		logmessage(LOGNAME,
			   "wireless driver module load failed or module is unavailable: %s",
			   module_name);
	return status;
}

static int
load_wireless_modules(void)
{
	int start_status = 0;

#if defined (USE_RT2860V2_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("rt2860v2_ap"));
#endif

#if defined (USE_MT7628_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("mt7628_ap"));
#endif

#if defined (USE_MT76X2_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("mt76x2_ap"));
#endif

#if defined (USE_MT7615_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("mt_7615e"));
#endif

#if defined (USE_RT3090_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("rt3090_ap"));
#elif defined (USE_RT5392_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("rt5392_ap"));
#elif defined (USE_MT76X3_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("mt76x3_ap"));
#endif

#if defined (USE_RT5592_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("rt5592_ap"));
#elif defined (USE_RT3593_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("rt3593_ap"));
#elif defined (USE_MT7610_AP)
	start_status = rc_first_service_start_status(start_status, load_wireless_module("mt7610_ap"));
#endif

	return start_status;
}

#if defined (USE_USB_SUPPORT)
static int
rc_boot_nvram_read_int_status(const char *name, int missing_value, int *value)
{
	char *text = NULL;
	int api_result, saved_errno, status;

	if (!name || !value) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(name, &text);
	saved_errno = errno;
	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					      text, missing_value, value);
	if (status != 0)
		errno = status;
	return status;
}

static int
load_usb_modules(void)
{
	int start_status = 0;
	int modem_rule = 0, modem_type = 0, usb3_disable = 0, nvram_status;
	int is_ap_mode = 0, ap_mode_status;

	/* load usb printer module before storage */
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("usblp", NULL));
	/* load usb storage module */
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("usb_storage", NULL));

	/*
	 * Do not turn NVRAM read failures into modem_rule=0 or modem_type=0.
	 * Preserve the absent-key defaults, but fail the boot module gate on
	 * backend errors and malformed persisted integers.
	 */
	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "cannot determine AP/router mode during USB module setup (status=%d); skipping modem-specific module selection",
			   ap_mode_status);
		start_status = rc_first_service_start_status(start_status,
							     ap_mode_status);
	} else if (!is_ap_mode) {
		nvram_status = rc_boot_nvram_read_int_status("modem_rule", 0,
							     &modem_rule);
		if (nvram_status != 0) {
			logmessage(LOGNAME,
				   "cannot read USB modem rule during boot (status=%d)",
				   nvram_status);
			start_status = rc_first_service_start_status(start_status,
							     nvram_status);
		} else if (modem_rule > 0) {
			nvram_status = rc_boot_nvram_read_int_status("modem_type", 0,
								     &modem_type);
			if (nvram_status != 0) {
				logmessage(LOGNAME,
					   "cannot read USB modem type during boot (status=%d)",
					   nvram_status);
				start_status = rc_first_service_start_status(start_status,
								     nvram_status);
			} else {
				start_status = rc_first_service_start_status(
					start_status, reload_modem_modules(modem_type, 0));
			}
		}
	}

	/* start usb host */
#if defined (USE_USB_XHCI)
	{
		char xhci_param[32];

		/*
		 * Do not collapse an XHCI module-parameter read failure into an
		 * apparently valid zero. Keep the legacy zero fallback so USB host
		 * initialization can continue, but surface the failed read in the
		 * aggregate hardware-module startup status.
		 */
		nvram_status = rc_boot_nvram_read_int_status("usb3_disable", 0,
							     &usb3_disable);
		if (nvram_status != 0) {
			logmessage(LOGNAME,
				   "cannot read USB 3.0 disable setting during boot (status=%d); using default 0",
				   nvram_status);
			start_status = rc_first_service_start_status(start_status,
							     nvram_status);
			usb3_disable = 0;
		}
		snprintf(xhci_param, sizeof(xhci_param), "%s=%d",
			 "usb3_disable", usb3_disable);
		start_status = rc_first_service_start_status(start_status,
							     load_boot_module("xhci_hcd", xhci_param));
		start_status = rc_first_service_start_status(start_status,
							     load_boot_module("xhci_mtk", NULL));
	}
#else
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("ehci_hcd", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("ohci_hcd", NULL));
#endif
	return start_status;
}
#endif

#if defined (USE_ATA_SUPPORT)
static int
load_ata_modules(void)
{
	/* start ahci device */
	return load_boot_module("ahci", NULL);
}
#endif

#if defined (USE_MMC_SUPPORT)
static int
load_mmc_modules(void)
{
	int start_status = 0;

	/* start mmc block device */
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("mmc_block", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("sdhci", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("sdhci-pltfm", NULL));
	/* start mmc host */
#if defined (USE_MTK_MMC)
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("mt7621-sd", NULL));
#endif
	return start_status;
}
#endif

#if defined (USE_MTK_AES)
static int
load_crypto_modules(void)
{
	/* start aes engine */
	return load_boot_module("mtk_aes", NULL);
}
#endif

#if defined (USE_IPSET)
static int
load_ipset_modules(void)
{
	int start_status = 0;

	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("xt_set", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("ip_set_hash_ip", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("ip_set_hash_mac", NULL));
	start_status = rc_first_service_start_status(start_status,
						     load_boot_module("ip_set_hash_net", NULL));
	return start_status;
}
#endif

static void
set_timezone(void)
{
	time_zone_x_mapping();
	setenv_tz();
	setkernel_tz();
}

static void
init_gpio_leds_buttons(void)
{
	/* allow WiFi hw-led  */
	LED_CONTROL(LED_WIFI, LED_ON);
	/* show PWR soft-led  */
	LED_CONTROL(LED_PWR, LED_ON);
}

static int
rc_wan0_nvram_get_owned(void *context, const char *param_name,
			char **value, int *present)
{
	char *cached_value = NULL;
	int api_result, saved_errno, status;

	(void)context;
	if (!param_name || !value || !present)
		return EINVAL;
	*value = NULL;
	*present = 0;

	errno = 0;
	api_result = nvram_get_checked(param_name, &cached_value);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !cached_value)
		return EIO;

	/* nvram_get_checked() returns shared cache storage; keep an owned copy. */
	*value = (char *)malloc(strlen(cached_value) + 1);
	if (!*value)
		return ENOMEM;
	memcpy(*value, cached_value, strlen(cached_value) + 1);
	*present = 1;
	return 0;
}

static int
rc_wan0_nvram_set_default(void *context, const char *param_name,
			  const char *value)
{
	int api_result, saved_errno;

	(void)context;
	errno = 0;
	api_result = nvram_set_temp(param_name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
set_wan0_runtime_value_status(void *context, const char *param_name,
			      const char *value)
{
	int status;

	(void)context;
	status = set_wan_unit_value_status(0, param_name, value);
	if (status != 0)
		logmessage(LOGNAME,
			   "cannot initialize WAN0 runtime field %s (status=%d)",
			   param_name ? param_name : "(null)", status);
	return status;
}

static int
set_wan0_vars(void)
{
	static const struct rc_wan_nvram_value runtime_values[] = {
		{ "unit", "0" },
		{ "primary", "1" },
		{ "ifname", IFNAME_WAN },
		{ "uptime", "0000000000" },
		{ "dltime", "0000000000" },
		{ "bytes_rx", "00000000000000000000" },
		{ "bytes_tx", "00000000000000000000" },
		{ "ifname_t", "" }
	};
	struct nvram_pair *np;
	char tmp[100], prefix[16];
	int status;

	snprintf(prefix, sizeof(prefix), "wan%d_", 0);

	/* Preserve existing WAN0 overrides, but never ignore a failed copy. */
	for (np = router_defaults; np->name; np++) {
		if (strncmp(np->name, "wan_", 4) != 0)
			continue;
		status = rc_wan_nvram_copy_missing_default(
			rc_wan0_nvram_get_owned, rc_wan0_nvram_set_default,
			NULL, strcat_r(prefix, &np->name[4], tmp),
			np->name, np->value);
		if (status != 0) {
			logmessage(LOGNAME,
				   "cannot initialize WAN0 default %s (status=%d)",
				   tmp, status);
			return status;
		}
	}

	/*
	 * WAN identity and callback state are required before any network service
	 * can start. The tested required-value helper stops at the first failed
	 * write, preserving its errno-style status for init_router().
	 */
	return rc_wan_nvram_set_required_values(
		runtime_values,
		(unsigned int)(sizeof(runtime_values) / sizeof(runtime_values[0])),
		set_wan0_runtime_value_status, NULL);
}

static int
nvram_legacy_set(void *context, const char *name, const char *value)
{
	(void)context;
	return nvram_set(name, value);
}
static int
nvram_legacy_unset(void *context, const char *name)
{
	(void)context;
	return nvram_unset(name);
}
static const struct rc_nvram_legacy_migration_ops nvram_legacy_migration_ops = {
	nvram_legacy_set, nvram_legacy_unset
};

/* Return 0 when absent, 1 when present, or a negative errno on read failure. */
static int
nvram_legacy_get_copy(const char *name, char **value)
{
	char *stored_value = NULL;
	int result;
	if (!value)
		return -EINVAL;
	*value = NULL;
	result = rc_nvram_policy_read(&rc_nvram_ops, NULL, name, &stored_value);
	if (result < 0)
		return -EIO;
	if (result == 0)
		return 0;
	if (!stored_value)
		return -EIO;
	*value = strdup(stored_value);
	return *value ? 1 : -ENOMEM;
}
static int
nvram_legacy_publish_and_unset(const char *source_key,
	const struct rc_nvram_legacy_migration_write *writes,
	unsigned int write_count, int *changed)
{
	return rc_nvram_legacy_publish_then_unset(
		&nvram_legacy_migration_ops, NULL, source_key, writes, write_count, changed);
}
static int
nvram_migrate_legacy_setting(const char *source_key, const char *target_key,
	const char *fixed_value, int positive_only, int *changed)
{
	struct rc_nvram_legacy_migration_write write;
	char *source_value = NULL, *target_value = NULL;
	const char *publish_value;
	unsigned int write_count = 0;
	int result, status;

	result = nvram_legacy_get_copy(source_key, &source_value);
	if (result <= 0)
		return result < 0 ? -result : 0;
	publish_value = fixed_value ? fixed_value : source_value;
	if (source_value[0] && (!positive_only || atoi(source_value) > 0)) {
		result = nvram_legacy_get_copy(target_key, &target_value);
		if (result < 0) {
			status = -result;
			goto done;
		}
		if (result == 0 || !target_value[0]) {
			write.name = target_key;
			write.value = publish_value;
			write_count = 1;
		}
	}
	status = nvram_legacy_publish_and_unset(
		source_key, write_count ? &write : NULL, write_count, changed);
done:
	free(source_value);
	free(target_value);
	return status;
}
static int
nvram_migrate_legacy_front_leds(int *changed)
{
	struct rc_nvram_legacy_migration_write writes[2];
	char *source_value = NULL;
	unsigned int write_count = 0;
	int result, front_leds, status;
	result = nvram_legacy_get_copy("front_leds", &source_value);
	if (result <= 0)
		return result < 0 ? -result : 0;
	front_leds = atoi(source_value);
	if (front_leds > 2) {
		writes[write_count].name = "front_led_all";
		writes[write_count++].value = "0";
	}
	if (front_leds == 4 || front_leds == 2) {
		writes[write_count].name = "front_led_pwr";
		writes[write_count++].value = "0";
	}
	status = nvram_legacy_publish_and_unset(
		"front_leds", write_count ? writes : NULL, write_count, changed);
	free(source_value);
	return status;
}
static void
nvram_cleanup_obsolete_legacy_key(const char *name, int *changed)
{
	char *value = NULL;
	int found, status;
	found = nvram_legacy_get_copy(name, &value);
	free(value);
	if (found <= 0) {
		if (found < 0)
			logmessage(LOGNAME, "Unable to inspect obsolete NVRAM key %s (status=%d)",
				   name, -found);
		return;
	}
	status = nvram_legacy_unset(NULL, name);
	if (status == 0)
		*changed = 1;
	else
		logmessage(LOGNAME, "Unable to remove obsolete NVRAM key %s (status=%d)",
			   name, status);
}
static int
nvram_convert_old_params(int *changed)
{
	static const char *const obsolete_keys[] = {
		"wan0_heartbeat_x", "wan0_3g_pin", "lan_route", "wan0_route",
		"wan_route", "wan_dns_t", "wan_proto_t", "wan_ipaddr_t",
		"wan_netmask_t", "wan_gateway_t", "wan_ifname_t", "wan_status_t",
		"wan_subnet_t", "lan_subnet_t", "link_lan", "rt_mcastrate",
		"wl_mcastrate"
	};
	unsigned int i;
	int status;
	if (!changed)
		return EINVAL;
	*changed = 0;
	status = nvram_migrate_legacy_front_leds(changed);
	if (status != 0)
		goto failed;
	status = nvram_migrate_legacy_setting("pppoe_dhcp_route",
		"wan_pppoe_man", NULL, 0, changed);
	if (status != 0)
		goto failed;
	status = nvram_migrate_legacy_setting("mr_ttl_fix", "wan_ttl_fix",
		"2", 1, changed);
	if (status != 0)
		goto failed;
	status = nvram_migrate_legacy_setting("wan_heartbeat_x",
		"wan_ppp_peer", NULL, 0, changed);
	if (status != 0)
		goto failed;
	status = nvram_migrate_legacy_setting("wan_3g_pin", "modem_pin",
		NULL, 0, changed);
	if (status != 0)
		goto failed;
	for (i = 0; i < sizeof(obsolete_keys) / sizeof(obsolete_keys[0]); ++i)
		nvram_cleanup_obsolete_legacy_key(obsolete_keys[i], changed);
	return 0;
failed:
	logmessage(LOGNAME,
		   "Router initialization stopped: legacy NVRAM migration failed (status=%d); original keys are retained",
		   status);
	return status;
}

static int
nvram_convert_misc_values(void)
{
	char buff[64];
	char *sw_mode_value = NULL;
	int sw_mode, status, api_result, saved_errno;
	int ntpc_counter_reset_status, ntp_ready_reset_status;

	/* A failed read must not be mistaken for an absent/invalid mode. */
	errno = 0;
	api_result = nvram_get_checked("sw_mode", &sw_mode_value);
	saved_errno = errno;
	status = rc_router_mode_nvram_read_status(
		api_result, saved_errno, sw_mode_value != NULL);
	if (status != 0) {
		logmessage(LOGNAME,
			   "router mode NVRAM read failed (status=%d)", status);
		return status;
	}

	/* Missing or invalid values retain the historical gateway fallback. */
	sw_mode = (api_result == 1) ? rc_router_mode_parse(sw_mode_value) : 0;

	/* Publish a complete router-mode policy before any dependent boot setup. */
	status = rc_router_mode_nvram_configure(
		sw_mode, rc_nvram_policy_set, NULL, &sw_mode);
	if (status != 0) {
		logmessage(LOGNAME,
			   "router mode NVRAM publication failed (status=%d)", status);
		return status;
	}

	if (sw_mode != 3)
		LED_CONTROL(LED_ROUTER, LED_ON);

#if BOARD_HAS_5G_RADIO
	if (strlen(nvram_wlan_get(1, "ssid")) < 1)
		nvram_wlan_set(1, "ssid", DEF_WLAN_5G_SSID);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get(1, "ssid"));
	nvram_wlan_set(1, "ssid2", buff);

	if (strlen(nvram_wlan_get(1, "wpa_mode")) < 1)
		nvram_wlan_set_int(1, "wpa_mode", 0);

#if BOARD_HAS_5G_11AC
	if (strlen(nvram_wlan_get(1, "gmode")) < 1)
		nvram_wlan_set_int(1, "gmode", 4); // a/n/ac Mixed

	if (nvram_wlan_get_int(1, "HT_BW") > 3)
		nvram_wlan_set_int(1, "HT_BW", 3);
#else
	if (strlen(nvram_wlan_get(1, "gmode")) < 1)
		nvram_wlan_set_int(1, "gmode", 2); // a/n Mixed

	if (nvram_wlan_get_int(1, "HT_BW") > 1)
		nvram_wlan_set_int(1, "HT_BW", 1);
#endif

	if (nvram_wlan_get_int(1, "stream_tx") > BOARD_NUM_ANT_5G_TX)
		nvram_wlan_set_int(1, "stream_tx", BOARD_NUM_ANT_5G_TX);

	if (nvram_wlan_get_int(1, "stream_rx") > BOARD_NUM_ANT_5G_RX)
		nvram_wlan_set_int(1, "stream_rx", BOARD_NUM_ANT_5G_RX);
#endif

	if (strlen(nvram_wlan_get(0, "ssid")) < 1)
		nvram_wlan_set(0, "ssid", DEF_WLAN_2G_SSID);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get(0, "ssid"));
	nvram_wlan_set(0, "ssid2", buff);

	if (strlen(nvram_wlan_get(0, "gmode")) < 1)
		nvram_wlan_set_int(0, "gmode", 2); // b/g/n Mixed

	if (nvram_wlan_get_int(0, "HT_BW") > 1)
		nvram_wlan_set_int(0, "HT_BW", 1);

	if (nvram_wlan_get_int(0, "stream_tx") > BOARD_NUM_ANT_2G_TX)
		nvram_wlan_set_int(0, "stream_tx", BOARD_NUM_ANT_2G_TX);

	if (nvram_wlan_get_int(0, "stream_rx") > BOARD_NUM_ANT_2G_RX)
		nvram_wlan_set_int(0, "stream_rx", BOARD_NUM_ANT_2G_RX);

	/*
	 * ntpc_counter and ntp_ready jointly identify a successful NTP step
	 * during this boot. A stale persistent ntp_ready=1 must never survive a
	 * failed counter/readiness reset and authorize time-based watchdog work.
	 */
	ntpc_counter_reset_status = nvram_set_temp("ntpc_counter", "0000000000");
	ntp_ready_reset_status = nvram_set_int("ntp_ready", 0);
	if (!init_router_ntp_state_reset_succeeded(ntpc_counter_reset_status,
						   ntp_ready_reset_status)) {
		logmessage(LOGNAME,
			   "Router initialization stopped: cannot reset per-boot NTP readiness state (ntpc_counter=%d, ntp_ready=%d)",
			   ntpc_counter_reset_status, ntp_ready_reset_status);
		return INIT_ROUTER_RESULT_FATAL;
	}
	/*
	 * These per-boot event and runtime markers must not retain a previous
	 * boot's values. Attempt every reset, but do not continue startup when
	 * any write failed: stale hotplug/reconnect state can trigger false work.
	 */
	status = reset_router_runtime_nvram_state(sw_mode);
	if (status != 0) {
		logmessage(LOGNAME,
			   "Router initialization stopped: cannot reset volatile runtime NVRAM state (status=%d)",
			   status);
		return status;
	}

	/* Do not continue boot with a partially published WAN0 configuration. */
	status = set_wan0_vars();
	if (status != 0)
		return status;
	status = set_usb_modem_dev_wan(0, 0);
	if (status != 0) {
		logmessage(LOGNAME,
			   "Router initialization stopped: cannot clear USB modem WAN selection (status=%d)",
			   status);
		return status;
	}
	/*
	 * WAN reset stages the protocol, route, credentials and runtime address
	 * tuple. Do not let PID 1 continue boot with a partially reset WAN.
	 */
	reset_wan_vars();
	status = get_wan_vars_reset_status();
	if (status != 0) {
		logmessage(LOGNAME,
			   "Router initialization stopped: WAN reset failed (status=%d)",
			   status);
		return status;
	}

	/* Setup LAN identity and runtime address state before any service starts. */
	status = reset_lan_vars_status();
	if (status != 0)
		return status;
	status = reset_lan_temp_status();
	if (status != 0)
		return status;

	time_zone_x_mapping();
	return 0;
}

int
write_storage_to_mtd(void)
{
	return doSystem("/sbin/mtd_storage.sh %s", "save");
}

int
erase_storage(void)
{
	return doSystem("/sbin/mtd_storage.sh %s", "erase");
}

int
erase_nvram(void)
{
	if (rc_nvram_erase_and_commit(&rc_nvram_ops, NULL) < 0) {
		logmessage(LOGNAME, "NVRAM reset flag could not be durably committed");
		return -1;
	}

	return 0;
}

static int
flash_firmware(void)
{
	const char *script_name = SCRIPT_SHUTDOWN;
	char* svcs[] = { "l2tpd", "xl2tpd", "pppd", NULL };
	int lock_fd, stop_status = 0, release_status, saved_errno = 0;
	int vpn_client_stop_status, vpn_server_stop_status, residual_stop_status;
	int eapol_stop_status, kabinet_stop_status, auth_stop_status, pppoe_stop_status;
	int storage_time_status, storage_mtd_status;
	int flash_status;

	/* PPPD callback children share this lock; stop only after in-flight
	 * VPN callbacks have completed, then stale children fail PID identity. */
	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(LOGNAME, "firmware update aborted: cannot lock VPN server lifecycle (errno=%d)", errno);
		return -1;
	}
	/*
	 * Firmware flash must stop both VPN endpoints while holding the shared
	 * lifecycle lock. The public wrappers would reacquire and unlock this
	 * process-associated F_SETLK lock, so use their lock-owned entry points.
	 */
	vpn_client_stop_status = stop_vpn_client_under_lifecycle_lock();
	if (vpn_client_stop_status != 0 && stop_status == 0) {
		stop_status = vpn_client_stop_status;
		saved_errno = errno ? errno : EIO;
	}
	vpn_server_stop_status = stop_vpn_server_under_lifecycle_lock();
	if (vpn_server_stop_status != 0 && stop_status == 0) {
		stop_status = vpn_server_stop_status;
		saved_errno = errno ? errno : EIO;
	}
	/* Catch residual legacy L2TP/PPP names after identity-bound endpoint stops. */
	residual_stop_status = kill_services(svcs, 6, 1);
	if (residual_stop_status != 0 && stop_status == 0) {
		stop_status = residual_stop_status;
		saved_errno = errno ? errno : EIO;
	}
	release_status = vpn_server_lifecycle_lock_release(lock_fd);
	if (release_status != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "firmware update cannot release VPN server lifecycle lock (errno=%d)",
			   release_errno);
		if (stop_status == 0) {
			stop_status = -1;
			saved_errno = release_errno;
		}
	}
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "firmware update aborted: VPN client/server or residual L2TP/PPP services did not stop (client=%d, server=%d, residual=%d)",
			   vpn_client_stop_status, vpn_server_stop_status, residual_stop_status);
		if (saved_errno)
			errno = saved_errno;
		return -1;
	}
	if (stop_services(0) != 0) {
		logmessage(LOGNAME, "firmware update aborted: checked services did not stop");
		return -1;
	}
	eapol_stop_status = stop_auth_eapol();
	kabinet_stop_status = stop_auth_kabinet();
	auth_stop_status = stop_8021x_all();
	pppoe_stop_status = set_passthrough_pppoe(0);
	if (eapol_stop_status != 0 || kabinet_stop_status != 0 ||
	    auth_stop_status != 0 || pppoe_stop_status != 0) {
		logmessage(LOGNAME,
			   "firmware update aborted: WAN/AP authentication or PPPoE relay daemons did not stop (EAPOL=%d, kabinet=%d, 802.1X=%d, relay=%d)",
			   eapol_stop_status, kabinet_stop_status, auth_stop_status, pppoe_stop_status);
		return -1;
	}
	if (check_if_file_exist(script_name)) {
		int script_status = doSystem("%s %d", script_name, 0);
		if (script_status != 0) {
			logmessage(LOGNAME, "firmware update aborted: custom shutdown script failed (status=%d)",
				   script_status);
			return -1;
		}
	}
#if defined (USE_STORAGE)
	if (safe_remove_all_stor_devices(0) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "firmware update aborted: storage eject incomplete (errno=%d)", saved_errno);
		errno = saved_errno;
		return -1;
	}
#endif
#if defined (USE_USB_SUPPORT)
	if (stop_usb_printer_spoolers() != 0) {
		logmessage(LOGNAME, "firmware update aborted: USB printer daemons did not stop");
		return -1;
	}
#endif
	if (stop_igmpproxy(NULL) != 0) {
		logmessage(LOGNAME, "firmware update aborted: IPTV proxy daemons did not stop");
		return -1;
	}
	/* Stop the watchdog only after every other fallible preparation is complete. */
	if (stop_misc() != 0) {
		int watchdog_status, watchdog_errno;
		logmessage(LOGNAME, "firmware update aborted: miscellaneous services did not stop");
		watchdog_status = start_watchdog();
		watchdog_errno = errno ? errno : EIO;
		if (watchdog_status != 0)
			logmessage(LOGNAME, "firmware update abort could not restore watchdog (status=%d, errno=%d)",
				   watchdog_status, watchdog_errno);
		return -1;
	}
	storage_time_status = storage_save_time(60);
	storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status) ?
		write_storage_to_mtd() : -1;
	if (!storage_persistence_complete(storage_time_status, storage_mtd_status)) {
		int storage_errno, watchdog_status, watchdog_errno;
		storage_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "firmware update aborted: storage persistence failed (time status=%d, MTD status=%d)",
			   storage_time_status, storage_mtd_status);
		watchdog_status = start_watchdog();
		watchdog_errno = errno ? errno : EIO;
		if (watchdog_status != 0)
			logmessage(LOGNAME, "firmware update abort could not restore watchdog (status=%d, errno=%d)",
				   watchdog_status, watchdog_errno);
		errno = storage_errno;
		return -1;
	}
	sync();
	/* Do not start an irreversible flash until the sync settling interval
	 * has elapsed; service_wait_seconds retries the unslept remainder on
	 * EINTR instead of silently shortening this safety window. */
	if (service_wait_seconds(1) != 0) {
		int wait_errno = errno ? errno : EIO;
		int watchdog_status, watchdog_errno;

		logmessage(LOGNAME, "firmware update aborted: pre-flash settling wait failed (errno=%d)",
			   wait_errno);
		watchdog_status = start_watchdog();
		watchdog_errno = errno ? errno : EIO;
		if (watchdog_status != 0)
			logmessage(LOGNAME, "pre-flash wait failed and watchdog recovery also failed (status=%d, errno=%d)",
				   watchdog_status, watchdog_errno);
		errno = wait_errno;
		return -1;
	}
	errno = 0;
	flash_status = eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME);
	if (flash_status != 0) {
		int flash_errno = errno ? errno : EIO;
		int watchdog_status, watchdog_errno;

		logmessage(LOGNAME, "firmware write command failed (status=%d, errno=%d)",
			   flash_status, flash_errno);
		watchdog_status = start_watchdog();
		watchdog_errno = errno ? errno : EIO;
		if (watchdog_status != 0)
			logmessage(LOGNAME, "firmware write failed and watchdog recovery also failed (status=%d, errno=%d)",
				   watchdog_status, watchdog_errno);
		errno = flash_errno;
		return -1;
	}
	return 0;
}

static void
storage_load_time(void)
{
#if !defined (USE_RTC_HCTOSYS)
	FILE *fp;
	char buf[32];
	struct tm storage_tm;
	time_t storage_time = 0;

	fp = fopen("/etc/storage/system_time", "r");
	if (fp) {
		if (fgets(buf, sizeof(buf), fp))
			storage_time = (time_t)strtoul(buf, NULL, 0);
		fclose(fp);
	}

	if (storage_time > 0) {
		localtime_r(&storage_time, &storage_tm);
		if (storage_tm.tm_year >= (SYS_START_YEAR - 1900)) {
			storage_time = mktime(&storage_tm);
			storage_time += 5; // add delta 5 sec (~boot time)
			stime(&storage_time);
		}
	}
#endif
}

int
storage_save_time(time_t delta)
{
#if !defined (USE_RTC_HCTOSYS)
	FILE *fp = NULL;
	int fd = -1;
	int saved_errno;
	time_t now_time;
	struct tm now_tm;
	char temp_path[] = "/etc/storage/.system_time.XXXXXX";

	if (nvram_match("stime_stored", "0"))
		return 0;

	errno = 0;
	if (time(&now_time) == (time_t)-1) {
		if (!errno)
			errno = EIO;
		return -1;
	}
	errno = 0;
	if (!localtime_r(&now_time, &now_tm)) {
		if (!errno)
			errno = EINVAL;
		return -1;
	}
	if (now_tm.tm_year <= (SYS_START_YEAR - 1900))
		return 0;
	if ((delta > 0 && now_time > (time_t)(LONG_MAX - (long)delta)) ||
	    (delta < 0 && now_time < (time_t)(LONG_MIN - (long)delta))) {
		errno = EOVERFLOW;
		return -1;
	}
	fd = mkstemp(temp_path);
	if (fd < 0)
		return -1;
	if (fchmod(fd, 0644) != 0)
		goto fail;
	fp = fdopen(fd, "w");
	if (!fp)
		goto fail;
	fd = -1;
	if (fprintf(fp, "%lu", (unsigned long)(now_time + delta)) < 0 ||
	    fflush(fp) != 0 || fsync(fileno(fp)) != 0)
		goto fail;
	if (fclose(fp) != 0) {
		fp = NULL;
		goto fail;
	}
	fp = NULL;
	if (rename(temp_path, "/etc/storage/system_time") != 0)
		goto fail;
	return 0;

fail:
	saved_errno = errno ? errno : EIO;
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	(void)unlink(temp_path);
	errno = saved_errno;
	return -1;
#else
	(void)delta;
	return 0;
#endif
}

void 
setenv_tz(void)
{
	char *time_zone = NULL;
	char timezone_env[64];
	int read_result, read_errno, status;

	errno = 0;
	read_result = nvram_get_checked("time_zone_x", &time_zone);
	read_errno = errno;
	status = rc_timezone_env_format_status(
		read_result, read_errno, time_zone,
		timezone_env, sizeof(timezone_env));
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot update timezone environment because NVRAM read or value validation failed (status=%d)",
			   status);
		return;
	}

	/* setenv() copies the value, so a failed update preserves the old TZ. */
	if (setenv("TZ", timezone_env + 3, 1) != 0) {
		status = errno ? errno : EIO;
		errno = status;
		logmessage(LOGNAME,
			   "cannot publish timezone environment (status=%d)", status);
	}
}

void 
setkernel_tz(void)
{
	time_t now;
	struct tm gm, local;
	struct timezone tz;
	struct timeval *tvp = NULL;
	static int tz_minuteswest_last = -1;

	/* Update kernel timezone */
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);

	gm.tm_isdst = local.tm_isdst;
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;

	if (tz_minuteswest_last == tz.tz_minuteswest)
		return;

	tz_minuteswest_last = tz.tz_minuteswest;

	settimeofday(tvp, &tz);
}

#if defined (BOARD_K2P)
static void led_control(int gpio_led, int flag);

void 
LED_CONTROL(int gpio_led, int flag)
{
	switch (gpio_led)
	{
		case LED_PWR:
			led_control(LED_WIFI, !flag);
			led_control(LED_WAN, 0);
			break;
		case LED_WIFI:
			led_control(LED_PWR, !flag);
			led_control(LED_WAN, 0);
			break;
		case LED_WAN:
			led_control(LED_PWR, 0);
			led_control(LED_WIFI, !flag);
			break;
		default:
			break;
	}
	led_control(gpio_led, flag);
}

static void
led_control(int gpio_led, int flag)
#else
void 
LED_CONTROL(int gpio_led, int flag)
#endif
{
	int front_led_x = 1;

	if (!(search_gpio_led() & gpio_led))
		return;

	switch (gpio_led)
	{
	case LED_ROUTER:
		break;
	case LED_WAN:
		front_led_x = nvram_get_int("front_led_wan");
		break;
	case LED_LAN:
		front_led_x = nvram_get_int("front_led_lan");
		break;
	case LED_WIFI:
		front_led_x = nvram_get_int("front_led_wif");
		break;
	case LED_SW2G:
		front_led_x = nvram_get_int("front_led_wif");
		if (front_led_x) {
			flag = (is_radio_on_rt()) ? LED_ON : LED_OFF;
		}
		break;
	case LED_SW5G:
		front_led_x = nvram_get_int("front_led_wif");
		if (front_led_x) {
			flag = (is_radio_on_wl()) ? LED_ON : LED_OFF;
		}
		break;
	case LED_USB:
	case LED_USB2:
#if defined (USE_USB_SUPPORT)
		front_led_x = nvram_get_int("front_led_usb");
#else
		front_led_x = 0;
#endif
		break;
	case LED_PWR:
		front_led_x = nvram_get_int("front_led_pwr");
		break;
	default:
		return;
	}

	if (front_led_x == 0)
		flag = LED_OFF;

	if (nvram_get_int("front_led_all") == 0 && gpio_led != LED_PWR)
		flag = LED_OFF;

	if (flag != LED_OFF && !nvram_get_int("led_front_t"))
		flag = LED_OFF;

	gpio_led_set(gpio_led, flag);
}

int
init_crontab(void)
{
	int ret = 0;
#if defined (APP_SCUT)
	int command_status, command_errno, update_status;

	errno = 0;
	command_status = system("/sbin/check_crontab.sh a/1 a a a a scutclient_watchcat.sh");
	command_errno = errno;
	update_status = rc_crontab_update_status_from_wait(command_status, command_errno);
	ret = rc_crontab_update_status_merge(ret, update_status);
#endif
	return ret;
}

static int
rc_read_firewall_boot_enabled(const char *param_name, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !enabled) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = firewall_nvram_match_status(api_result, saved_errno,
					     value, "1", 0, enabled);
	if (status != 0)
		errno = status;
	return status;
}

int 
init_router(void)
{
	int log_remote, is_ap_mode, nvram_need_commit;
	int ap_mode_status;
	int storage_save_status, dnsmasq_start_status;
	int boot_modules_status, boot_services_status;
	int rwfs_mount_status;
	int switch_vlan_status = 0;
	int nvram_setup_status, legacy_migration_status;
	int legacy_migration_changed = 0;
	int ipv6_disable_required = 0;
	int ipv6_init_status = 0;

#if defined (USE_RTL8367)
	rtl8367_node();
#endif
#if defined (USE_MTK_ESW) || defined (USE_MTK_GSW)
	mtk_esw_node();
#endif

	legacy_migration_status = nvram_convert_old_params(&legacy_migration_changed);
	if (legacy_migration_status != 0) {
		logmessage(LOGNAME,
			   "Router initialization stopped: legacy NVRAM migration is incomplete (status=%d)",
			   legacy_migration_status);
		return INIT_ROUTER_RESULT_FATAL;
	}

	nvram_need_commit = nvram_restore_defaults();
	if (nvram_need_commit < 0) {
		logmessage(LOGNAME, "Router initialization stopped: NVRAM defaults are not trustworthy");
		return INIT_ROUTER_RESULT_FATAL;
	}
	if (legacy_migration_changed)
		nvram_need_commit = 1;

	get_eeprom_params();

	nvram_setup_status = nvram_convert_misc_values();
	if (nvram_setup_status != 0) {
		logmessage(LOGNAME,
			   "Router initialization stopped: WAN0 NVRAM setup failed (status=%d)",
			   nvram_setup_status);
		return INIT_ROUTER_RESULT_FATAL;
	}

	init_gpio_leds_buttons();

	if (rc_nvram_commit_if_needed(nvram_need_commit, &rc_nvram_ops, NULL) < 0) {
		logmessage(LOGNAME, "Router initialization stopped: NVRAM defaults commit failed");
		return INIT_ROUTER_RESULT_FATAL;
	}

	rwfs_mount_status = mount_rwfs_partition();

	if (gen_ralink_config_2g(0) < 0) {
		logmessage(LOGNAME, "Router initialization stopped: failed to generate 2.4GHz wireless configuration");
		return INIT_ROUTER_RESULT_FATAL;
	}
#if BOARD_HAS_5G_RADIO
	if (gen_ralink_config_5g(0) < 0) {
		logmessage(LOGNAME, "Router initialization stopped: failed to generate 5GHz wireless configuration");
		return INIT_ROUTER_RESULT_FATAL;
	}
#endif
	boot_modules_status = load_wireless_modules();
#if defined (USE_MMC_SUPPORT)
	boot_modules_status = rc_first_service_start_status(
		boot_modules_status, load_mmc_modules());
#endif
#if defined (USE_USB_SUPPORT)
	boot_modules_status = rc_first_service_start_status(
		boot_modules_status, load_usb_modules());
#endif
#if defined (USE_ATA_SUPPORT)
	boot_modules_status = rc_first_service_start_status(
		boot_modules_status, load_ata_modules());
#endif
#if defined (USE_MTK_AES)
	boot_modules_status = rc_first_service_start_status(
		boot_modules_status, load_crypto_modules());
#endif
#if defined (USE_IPSET)
	boot_modules_status = rc_first_service_start_status(
		boot_modules_status, load_ipset_modules());
#endif
	boot_services_status = rc_first_service_start_status(
		rwfs_mount_status, boot_modules_status);
	if (boot_modules_status != 0)
		logmessage(LOGNAME,
			   "Router initialization continuing with one or more unavailable hardware modules (first status=%d)",
			   boot_modules_status);

{
		int passwd_status = recreate_passwd_unix(1);

		if (passwd_status != 0)
			logmessage(LOGNAME, "Unix account database/password provisioning failed (status=%d)",
				   passwd_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, passwd_status);
	}

	set_timezone();
	set_pagecache_reclaim();

	storage_load_time();

	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!log_remote) {
		int logger_start_status = start_logger(1);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, logger_start_status);
	}

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		/*
		 * Unknown mode must not enter the router WAN/firewall path. Use
		 * AP-safe bridge configuration and retain DEGRADED boot status.
		 */
		is_ap_mode = 1;
		logmessage(LOGNAME,
			   "cannot determine AP/router mode during boot (status=%d); using AP-safe network setup and blocking WAN startup",
			   ap_mode_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, ap_mode_status);
	}

	{
		int loopback_start_status = init_loopback();
		if (loopback_start_status != 0)
			logmessage(LOGNAME, "loopback boot initialization incomplete (status=%d)",
				   loopback_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, loopback_start_status);
	}
	{
		int bridge_start_status = init_bridge(is_ap_mode, &switch_vlan_status);
		if (bridge_start_status != 0)
			logmessage(LOGNAME, "LAN bridge boot initialization incomplete (status=%d)", bridge_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, bridge_start_status);
	}
#if defined (USE_IPV6)
	ipv6_disable_required = (nvram_ipv6_type == IPV6_DISABLED);
	ipv6_init_status = init_ipv6();
	boot_services_status = rc_first_service_start_status(
		boot_services_status, ipv6_init_status);
#endif
	set_cpu_affinity();

	{
		int detect_link_start_status = start_detect_link();
		if (detect_link_start_status != 0)
			logmessage(LOGNAME, "detect_link boot launch failed (status=%d)",
				   detect_link_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, detect_link_start_status);
	}
	{
		int detect_internet_start_status = start_detect_internet(0);
		if (detect_internet_start_status != 0)
			logmessage(LOGNAME, "detect_internet boot launch failed (status=%d)",
				   detect_internet_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, detect_internet_start_status);
	}
	{
		int lan_start_status = start_lan(is_ap_mode, 0);
		if (lan_start_status != 0)
			logmessage(LOGNAME, "LAN boot startup incomplete (status=%d)", lan_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, lan_start_status);
	}

	if (log_remote) {
		int logger_start_status = start_logger(1);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, logger_start_status);
	}

	dnsmasq_start_status = start_dns_dhcpd(is_ap_mode);
	if (dnsmasq_start_status != 0)
		logmessage(LOGNAME,
			   "Router initialization continuing without DNSMasq (start status=%d)",
			   dnsmasq_start_status);
	boot_services_status = rc_first_service_start_status(
		boot_services_status, dnsmasq_start_status);
#if defined (APP_SMBD) || defined (APP_NMBD)
	{
		int wins_start_status = start_wins();
		if (wins_start_status != 0)
			logmessage(LOGNAME, "WINS boot startup failed (status=%d)",
				   wins_start_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, wins_start_status);
	}
#endif

	if (!is_ap_mode) {
		int nat_restore_status;
		int nat_required = 0;
		int ipv4_filter_status;
		int ipv6_filter_status = 0;
		int firewall_enabled = 0;
		int firewall_enable_status, nat_enable_status;
		int ipv6_filter_required = 0;
		int ipv4_forward_disable_status;
#if defined (USE_IPV6)
		int ipv6_forward_disable_status;
#endif

		firewall_enable_status = rc_read_firewall_boot_enabled(
			"fw_enable_x", &firewall_enabled);
		nat_enable_status = rc_read_firewall_boot_enabled(
			"wan_nat_x", &nat_required);
		if (firewall_enable_status != 0) {
			logmessage(LOGNAME,
				   "Cannot read firewall enable state during boot (status=%d)",
				   firewall_enable_status);
			boot_services_status = rc_first_service_start_status(
				boot_services_status, firewall_enable_status);
		}
		if (nat_enable_status != 0) {
			logmessage(LOGNAME,
				   "Cannot read NAT enable state during boot (status=%d)",
				   nat_enable_status);
			boot_services_status = rc_first_service_start_status(
				boot_services_status, nat_enable_status);
		}
		nat_restore_status = ipt_nat_default();
		ipv4_filter_status = ipt_filter_default();
#if defined (USE_IPV6)
		ipv6_filter_required = !ipv6_disable_required;
		ipv6_filter_status = ip6t_filter_default();
#endif
		if (nat_restore_status != 0)
			logmessage(LOGNAME, "Initial IPv4 NAT table restore failed (NAT enabled=%d)",
				   nat_required);

		if (!firewall_startup_policy_allows_wan_checked(
							firewall_enable_status,
							nat_enable_status,
							firewall_enabled,
							ipv4_filter_status,
							ipv6_filter_required,
							ipv6_filter_status,
							nat_required,
							nat_restore_status,
							switch_vlan_status,
							ipv6_disable_required,
							ipv6_init_status)) {
			if (firewall_enable_status != 0 || nat_enable_status != 0)
				logmessage(LOGNAME,
					   "Blocking WAN startup: firewall/NAT enable state could not be read (firewall=%d NAT=%d)",
					   firewall_enable_status, nat_enable_status);
			else if (switch_vlan_status != 0)
				logmessage(LOGNAME, "Blocking WAN startup: required switch VLAN isolation could not be verified (status=%d)",
					   switch_vlan_status);
			else if (ipv6_disable_required && ipv6_init_status != 0)
				logmessage(LOGNAME, "Blocking WAN startup: IPv6 is configured disabled but global disable sysctls could not be verified (status=%d)",
					   ipv6_init_status);
			else
				logmessage(LOGNAME, "Blocking WAN startup: required NAT table or restrictive IPv4/IPv6 firewall filters could not be verified");
			ipv4_forward_disable_status = set_ipv4_forward(0);
			if (ipv4_forward_disable_status != 0) {
				logmessage(LOGNAME, "failed to disable IPv4 forwarding after firewall startup rejection (status=%d)",
					   ipv4_forward_disable_status);
				boot_services_status = rc_first_service_start_status(
					boot_services_status, ipv4_forward_disable_status);
			}
#if defined (USE_IPV6)
			ipv6_forward_disable_status = set_interface_conf_int("ipv6", "all", "forwarding", 0);
			if (ipv6_forward_disable_status != 0) {
				logmessage(LOGNAME, "failed to disable IPv6 forwarding after firewall startup rejection (status=%d)",
					   ipv6_forward_disable_status);
				boot_services_status = rc_first_service_start_status(
					boot_services_status, ipv6_forward_disable_status);
			}
#endif
		} else {
			int wan_start_status = start_wan();
			if (wan_start_status != 0)
				logmessage(LOGNAME, "WAN boot startup incomplete (status=%d)", wan_start_status);
			boot_services_status = rc_first_service_start_status(
				boot_services_status, wan_start_status);
		}
	}

	{
		int service_start_status = start_services_once(is_ap_mode);

		boot_services_status = rc_first_service_start_status(boot_services_status,
								      service_start_status);
	}

	notify_leds_detect_link();

	{
		int optware_start_status = start_rwfs_optware();

		boot_services_status = rc_first_service_start_status(
			boot_services_status, optware_start_status);
		if (optware_start_status != 0)
			logmessage(LOGNAME,
				   "initial MTD Optware worker launch failed; retry is scheduled (status=%d)",
				   optware_start_status);
	}
	{
		int crontab_status = init_crontab();

		if (rc_crontab_update_requires_persistence(crontab_status)) {
			int crond_status;

			storage_save_status = write_storage_to_mtd();
			if (storage_save_status != 0) {
				boot_services_status = rc_first_service_start_status(
					boot_services_status, storage_save_status);
				logmessage(LOGNAME, "initial crontab changed but MTD storage persistence failed (status=%d)",
					   storage_save_status);
			}
			/* Activate the in-memory crontab even if flash persistence failed. */
			crond_status = restart_crond();
			if (crond_status != 0) {
				boot_services_status = rc_first_service_start_status(
					boot_services_status, crond_status);
				logmessage(LOGNAME,
					   "crond restart after initial crontab update failed (status=%d)",
					   crond_status);
			}
		} else if (crontab_status != 0) {
			boot_services_status = rc_first_service_start_status(
				boot_services_status, crontab_status);
			logmessage(LOGNAME, "initial crontab update failed (status=%d)",
				   crontab_status);
		}
	}
	// system ready
	/* Install the firmware CA trust store before autostart can launch AliDDNS. */
	{
		int copyscripts_status = system("/usr/bin/copyscripts.sh");

		if (copyscripts_status != 0)
			logmessage(LOGNAME,
				   "startup script/CA trust-store preparation failed (wait status=%d)",
				   copyscripts_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, copyscripts_status);
	}
	if (check_if_file_exist("/etc/storage/started_script.sh")) {
		int started_script_status = system("/etc/storage/started_script.sh &");

		if (started_script_status != 0)
			logmessage(LOGNAME, "persistent user startup script launch failed (wait status=%d)",
				   started_script_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, started_script_status);
	}
	{
		int autostart_status = system("/usr/bin/autostart.sh &");

		if (autostart_status != 0)
			logmessage(LOGNAME, "firmware optional-service autostart launch failed (wait status=%d)",
				   autostart_status);
		boot_services_status = rc_first_service_start_status(
			boot_services_status, autostart_status);
	}
	if (boot_services_status != 0)
		logmessage(LOGNAME,
			   "one or more boot service startup operations failed (first status=%d)",
			   boot_services_status);

	return init_router_result_from_boot_status(boot_services_status);
}

/*
 * level {0: reboot, 1: halt, 2: power-off}
 */
void
shutdown_router(int level)
{
	int use_halt = (level == 1) ? 1 : 0;
	int is_ap_mode = 1;
	int ap_mode_status;
	int storage_remove_status = 0;
	int misc_stop_status, service_stop_status = 0, printer_stop_status = 0;
	int wan_stop_status, lan_wan_stop_status = 0;
	int auth_stop_status, logger_stop_status = 0;
	int wifi_wl_stop_status = 0, wifi_rt_stop_status = 0;
	int lan_stop_status = 0;
	int forward_stop_status = 0;
	int optware_stop_status = 0;
	int rwfs_unmount_status = 0;
	int shutdown_script_status = 0;
	int storage_time_status, storage_mtd_status;
	const char *script_name = SCRIPT_SHUTDOWN;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		is_ap_mode = 1;
		logmessage(LOGNAME,
			   "cannot determine AP/router mode during shutdown (status=%d); using AP-safe LAN teardown and skipping storage persistence",
			   ap_mode_status);
	}

	/* Prevent a late Optware start hook from racing service shutdown. */
	optware_stop_status = stop_rwfs_optware();
	if (optware_stop_status != 0)
		logmessage(LOGNAME, "MTD Optware startup worker did not stop cleanly; continuing power-down without storage persistence");
	misc_stop_status = stop_misc();
	if (misc_stop_status != 0)
		logmessage(LOGNAME, "miscellaneous service shutdown incomplete; continuing power-down without storage persistence");

	if (level < 2) {
		service_stop_status = stop_services(use_halt);
		if (service_stop_status != 0)
			logmessage(LOGNAME, "service shutdown incomplete; continuing system power-down without storage persistence");
	}

	if (check_if_file_exist(script_name)) {
		shutdown_script_status = doSystem("%s %d", script_name, level);
		if (shutdown_script_status != 0)
			logmessage(LOGNAME, "custom shutdown script failed (status=%d); continuing power-down without storage persistence",
				   shutdown_script_status);
	}

#if defined (USE_STORAGE)
	storage_remove_status = safe_remove_all_stor_devices(use_halt);
	if (storage_remove_status != 0)
		logmessage(LOGNAME, "storage eject incomplete; continuing power-down without storage persistence");
#endif

#if defined (USE_USB_SUPPORT)
	printer_stop_status = stop_usb_printer_spoolers();
	if (printer_stop_status != 0)
		logmessage(LOGNAME, "USB printer shutdown incomplete; continuing power-down without storage persistence");
#endif
	LED_CONTROL(LED_USB, LED_OFF);
	LED_CONTROL(LED_USB2, LED_OFF);

	wan_stop_status = stop_wan();
	if (wan_stop_status != 0)
		logmessage(LOGNAME, "WAN shutdown incomplete; continuing power-down without storage persistence");
	if (level < 2) {
		lan_wan_stop_status = stop_services_lan_wan();
		if (lan_wan_stop_status != 0)
			logmessage(LOGNAME, "LAN/WAN service shutdown incomplete; continuing power-down without storage persistence");
		forward_stop_status = set_ipv4_forward(0);
		if (forward_stop_status != 0)
			logmessage(LOGNAME, "IPv4 forwarding shutdown incomplete (status=%d)", forward_stop_status);
		lan_wan_stop_status = rc_first_service_start_status(
			lan_wan_stop_status, forward_stop_status);
	}

	auth_stop_status = stop_8021x_all();
	if (auth_stop_status != 0)
		logmessage(LOGNAME, "802.1X shutdown incomplete; continuing power-down without storage persistence");

	if (level < 2) {
		logger_stop_status = stop_logger();
		if (logger_stop_status != 0)
			logmessage(LOGNAME, "logger shutdown incomplete; continuing power-down without storage persistence");
	}

	/* Radio teardown is a prerequisite for safe storage persistence. */
	wifi_wl_stop_status = stop_wifi_all_wl();
	if (wifi_wl_stop_status != 0)
		logmessage(LOGNAME, "5GHz radio shutdown incomplete; continuing power-down without storage persistence");
	wifi_rt_stop_status = stop_wifi_all_rt();
	if (wifi_rt_stop_status != 0)
		logmessage(LOGNAME, "2.4GHz radio shutdown incomplete; continuing power-down without storage persistence");

	/*
	 * LAN teardown is part of the persistence consistency boundary. In AP
	 * mode this also waits for the identity-bound DHCP release/deconfig path;
	 * an UNKNOWN or incomplete stop must not publish a storage snapshot.
	 */
	if (level < 2) {
		lan_stop_status = stop_lan(is_ap_mode);
		if (lan_stop_status != 0)
			logmessage(LOGNAME, "LAN shutdown incomplete; continuing power-down without storage persistence (status=%d)",
				   lan_stop_status);
	}

	/* Stop Optware hooks and detach RWFS before publishing MTD storage. */
	rwfs_unmount_status = umount_rwfs_partition();
	if (rwfs_unmount_status != 0)
		logmessage(LOGNAME, "RWFS/Optware teardown incomplete; continuing power-down without storage persistence");

	LED_CONTROL(LED_WAN, LED_OFF);

	if (ap_mode_status == 0 &&
	    storage_remove_status == 0 && misc_stop_status == 0 &&
	    service_stop_status == 0 && printer_stop_status == 0 &&
	    wan_stop_status == 0 && lan_wan_stop_status == 0 &&
	    auth_stop_status == 0 && logger_stop_status == 0 &&
	    wifi_wl_stop_status == 0 && wifi_rt_stop_status == 0 &&
	    lan_stop_status == 0 && optware_stop_status == 0 &&
	    rwfs_unmount_status == 0 && shutdown_script_status == 0) {
		storage_time_status = storage_save_time(10);
		storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status) ?
			write_storage_to_mtd() : -1;
		if (!storage_persistence_complete(storage_time_status, storage_mtd_status))
			logmessage(LOGNAME, "storage MTD persistence failed (time status=%d, MTD status=%d); continuing power-down",
				   storage_time_status, storage_mtd_status);
	} else {
		logmessage(LOGNAME, "skipping storage MTD persistence because shutdown-hook, LAN/service teardown, RWFS/Optware or safe-eject completion was not verified");
	}


	if (use_halt) {
		module_smart_unload("hw_nat", 0);
	}

	LED_CONTROL(LED_LAN, LED_OFF);
	LED_CONTROL(LED_PWR, LED_OFF);
}

static int
rc_manual_toggle_radio_status(int is_5g, int requested_on)
{
	int was_on;
	int effective_request;
	int changed;

	if (is_5g) {
		was_on = is_radio_on_wl();
		effective_request = requested_on < 0 ? !was_on : !!requested_on;
		changed = manual_toggle_radio_wl(requested_on);
	} else {
		was_on = is_radio_on_rt();
		effective_request = requested_on < 0 ? !was_on : !!requested_on;
		changed = manual_toggle_radio_rt(requested_on);
	}

	return wireless_radio_notification_result(was_on, effective_request,
						    changed);
}

static int
rc_manual_change_radio_status(int is_5g, int requested_on)
{
	int was_enabled = 0;
	int status;

	status = manual_change_radio_status(is_5g, requested_on, &was_enabled);
	/* Preserve checked-NVRAM and watchdog-settling errors. */
	if (status < 0)
		return status;
	return wireless_radio_notification_result(was_enabled, requested_on,
						    status);
}

static int
rc_notify_apply_radio_control(int is_5g, int radio_on, int manual)
{
	int was_on;
	int change_reported;
	int status;

	if (is_5g) {
		was_on = is_radio_on_wl();
		change_reported = control_radio_wl(radio_on, manual);
	} else {
		was_on = is_radio_on_rt();
		change_reported = control_radio_rt(radio_on, manual);
	}

	status = wireless_radio_notification_result(was_on, radio_on,
						    change_reported);
	if (status != 0)
		logmessage(LOGNAME, "%s radio notification incomplete (requested=%d, was_on=%d, changed=%d)",
			   is_5g ? "5GHz" : "2.4GHz", !!radio_on, !!was_on,
			   change_reported);
	return status;
}


#if defined (USE_USB_SUPPORT) || defined (USE_STORAGE)
static int
rc_notify_set_deferred_hotplug_marker(const char *event_name, const char *marker_name)
{
	int marker_status = nvram_set_int_temp(marker_name, 1);
	if (marker_status != 0) {
		rc_deferred_hotplug_retry_mask = rc_notify_hotplug_retry_record(
			rc_deferred_hotplug_retry_mask, event_name, marker_status);
		logmessage(LOGNAME, "cannot persist deferred event %s in %s (status=%d); retry scheduled",
			   event_name, marker_name, marker_status);
	}
	return marker_status;
}
#endif


int
rc_notify_schedule_deferred_hotplug_retry(const char *event_name)
{
	unsigned int retry_flag;
	int saved_errno;

	retry_flag = rc_notify_hotplug_retry_flag(event_name);
	if (retry_flag == 0)
		return EINVAL;

	if (getpid() == 1) {
		rc_deferred_hotplug_retry_mask = rc_notify_hotplug_retry_record(
			rc_deferred_hotplug_retry_mask, event_name, EIO);
		alarm(5);
		return 0;
	}

	if (notify_rc_status(event_name) == 0)
		return 0;
	saved_errno = errno > 0 ? errno : EIO;
	logmessage(LOGNAME,
		   "cannot queue deferred hotplug retry %s from short-lived process (errno=%d)",
		   event_name, saved_errno);
	errno = saved_errno;
	return saved_errno;
}


void
rc_notify_retry_deferred_hotplug_events(void)
{
	static const struct { const char *event_name; unsigned int flag; } retry_events[] = {
		{ "on_hotplug_usb_printer", RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG },
		{ "on_unplug_usb_printer", RC_NOTIFY_HOTPLUG_RETRY_PRINTER_UNPLUG },
		{ "on_hotplug_usb_modem", RC_NOTIFY_HOTPLUG_RETRY_MODEM_PLUG },
		{ "on_unplug_usb_modem", RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG },
		{ "on_hotplug_mass_storage", RC_NOTIFY_HOTPLUG_RETRY_STORAGE_PLUG },
		{ "on_unplug_mass_storage", RC_NOTIFY_HOTPLUG_RETRY_STORAGE_UNPLUG }
	};
	unsigned int pending = rc_deferred_hotplug_retry_mask, queued = 0;
	int queue_fd, i, marker_status, signal_status, saved_errno;
	if (!pending) return;
	queue_fd = rc_notify_queue_open_dir(DIR_RC_NOTIFY, 0);
	if (queue_fd < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot open RC notification queue for deferred USB retry (errno=%d)", saved_errno);
		alarm(5);
		return;
	}
	for (i = 0; i < (int)(sizeof(retry_events) / sizeof(retry_events[0])); ++i) {
		if (!(pending & retry_events[i].flag)) continue;
		marker_status = rc_notify_queue_touch_at(queue_fd, retry_events[i].event_name, 0);
		if (marker_status < 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot requeue deferred USB event %s (errno=%d)", retry_events[i].event_name, saved_errno);
			continue;
		}
		queued |= retry_events[i].flag;
	}
	close(queue_fd);
	if (queued) {
		signal_status = kill(1, SIGUSR1);
		if (signal_status != 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot wake PID 1 for deferred USB event retry (errno=%d)", saved_errno);
		}
		rc_deferred_hotplug_retry_mask = rc_notify_hotplug_retry_complete(pending, queued, signal_status);
	}
	if (rc_deferred_hotplug_retry_mask) alarm(5);
}

void
handle_notifications(void)
{
	int i, stop_handle = 0, queue_pending = 0;
	int notify_dir_fd, incomplete_dir_fd, queue_fd;
	DIR *directory;
	struct dirent *pending_entry;

	notify_dir_fd = rc_notify_queue_open_dir(DIR_RC_NOTIFY, 0);
	if (notify_dir_fd < 0)
		return;
	incomplete_dir_fd = rc_notify_queue_open_dir(DIR_RC_INCOMPLETE, 0);
	if (incomplete_dir_fd < 0) {
		close(notify_dir_fd);
		return;
	}
	directory = fdopendir(notify_dir_fd);
	if (!directory) {
		close(notify_dir_fd);
		close(incomplete_dir_fd);
		return;
	}
	queue_fd = dirfd(directory);
	if (queue_fd < 0) {
		closedir(directory);
		close(incomplete_dir_fd);
		return;
	}

	// handle max 10 requests at once (prevent deadlock)
	for (i = 0; i < RC_NOTIFY_BATCH_LIMIT; i++)
	{
		struct dirent *entry;
		int action_result = 0;
		
		entry = readdir(directory);
		if (!entry)
			break;
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		
		if (!rc_notify_event_name_is_safe(entry->d_name))
			continue;
		if (rc_notify_queue_marker_is_safe_at(queue_fd, entry->d_name, 0) != 0) {
			unlinkat(queue_fd, entry->d_name, 0);
			continue;
		}
		if (unlinkat(queue_fd, entry->d_name, 0) != 0)
			continue;

		printf("rc notification: %s\n", entry->d_name);

		/* Take the appropriate action. */
		if (!strcmp(entry->d_name, RCN_RESTART_REBOOT))
		{
			stop_handle = 1;
			sys_exit();
		}
		else if (!strcmp(entry->d_name, "flash_firmware"))
		{
			stop_handle = 1;
			action_result = rc_notify_merge_action_result(
				action_result, flash_firmware());
		}
#if defined (USE_IPV6)
		else if (!strcmp(entry->d_name, RCN_RESTART_IPV6))
		{
			int is_ap_mode, ap_mode_status;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "IPv6 restart skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				action_result = rc_notify_merge_action_result(
					action_result, ap_mode_status);
			} else if (!is_ap_mode) {
				int ipv6_restart_status = full_restart_ipv6(nvram_ipv6_type);

				action_result = rc_notify_merge_action_result(
					action_result, ipv6_restart_status);
				if (ipv6_restart_should_publish_type(ipv6_restart_status)) {
					int updated_ipv6_type, type_read_status;

					type_read_status = get_ipv6_type_status(&updated_ipv6_type);
					if (type_read_status == 0)
						nvram_ipv6_type = updated_ipv6_type;
					else {
						errno = type_read_status;
						logmessage(LOGNAME,
							   "IPv6 restart completed but service type cache could not be refreshed (status=%d)",
							   type_read_status);
						action_result = rc_notify_merge_action_result(
							action_result, type_read_status);
					}
				}
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_RADV) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_dhcpd());
		}
#endif
		else if (!strcmp(entry->d_name, RCN_RESTART_WAN))
		{
			action_result = rc_notify_merge_action_result(
				action_result, full_restart_wan());
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_LAN))
		{
			action_result = rc_notify_merge_action_result(action_result, full_restart_lan());
		}
		else if (!strcmp(entry->d_name, "stop_whole_wan"))
		{
			action_result = rc_notify_merge_action_result(action_result, stop_wan());
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_IPTV))
		{
			int is_ap_mode, ap_mode_status;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "IPTV restart skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				action_result = rc_notify_merge_action_result(
					action_result, ap_mode_status);
			} else {
				action_result = rc_notify_merge_action_result(
					action_result, restart_iptv(is_ap_mode));
				if (!is_ap_mode)
					action_result = rc_notify_merge_action_result(
						action_result, restart_firewall());
			}
		}
		else if(!strcmp(entry->d_name, "deferred_wan_connect"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, deferred_wan_connect());
		}
		else if(!strcmp(entry->d_name, "auto_wan_reconnect"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, auto_wan_reconnect());
		}
		else if(!strcmp(entry->d_name, "auto_wan_reconnect_pause"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, auto_wan_reconnect_pause());
		}
		else if(!strcmp(entry->d_name, "manual_wan_reconnect"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, manual_wan_reconnect());
		}
		else if(!strcmp(entry->d_name, "manual_wan_disconnect"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, manual_wan_disconnect());
		}
		else if(!strcmp(entry->d_name, "manual_wisp_reassoc"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, manual_wisp_reassoc());
		}
		else if(!strcmp(entry->d_name, RCN_MANUAL_DDNS_UPDATE))
		{
			action_result = rc_notify_merge_action_result(action_result,
										  notify_ddns_manual_update());
		}
		else if(!strcmp(entry->d_name, "manual_ddns_hostname_check"))
		{
			manual_ddns_hostname_check();
		}
#if defined (USE_USB_SUPPORT)
		else if (!strcmp(entry->d_name, RCN_RESTART_MODEM))
		{
			int wan_stopped = 0;
			int stop_failed = 0;
			int wan_stop_status = 0;
			int modules_reloaded = 0;
			int modem_identity_status;
			int modem_settings_status;
			int need_restart_wan = 0;
			int modem_rule = 0, modem_type = 0;

			modem_settings_status = rc_modem_read_nvram_settings_status(
				rc_boot_nvram_read_int_status, &modem_rule, &modem_type);
			if (modem_settings_status != 0) {
				logmessage("USB",
					   "refusing modem module/service restart: cannot read modem configuration (status=%d)",
					   modem_settings_status);
				action_result = rc_notify_merge_action_result(
					action_result, modem_settings_status);
				stop_failed = 1;
			}

			if (!stop_failed) {
				modem_identity_status = get_usb_modem_wan_status(
					0, &need_restart_wan);
				if (!modem_wan_restart_allowed(modem_identity_status)) {
					logmessage("USB",
						   "refusing modem module/service restart: WAN modem identity is unknown (status=%d)",
						   modem_identity_status);
					action_result = rc_notify_merge_action_result(
						action_result, modem_identity_status);
					stop_failed = 1;
				}
			}
			if (!stop_failed && nvram_modem_rule != modem_rule)
			{
				if (need_restart_wan) {
					wan_stop_status = stop_wan();
					if (wan_stop_status != 0) {
						action_result = rc_notify_merge_action_result(
							action_result, wan_stop_status);
						stop_failed = 1;
					} else {
						wan_stopped = 1;
					}
				}
				if (!stop_failed) {
					nvram_modem_rule = modem_rule;
					if (modem_rule > 0) {
						modules_reloaded = 1;
						action_result = rc_notify_merge_action_result(
							action_result, reload_modem_modules(modem_type, 1));
					} else {
						unload_modem_modules();
					}
				}
			}
			if (!stop_failed && nvram_modem_type != modem_type)
			{
				if (nvram_modem_type == 3 || modem_type == 3) {
					if (modem_rule > 0 && !modules_reloaded) {
						if (need_restart_wan && !wan_stopped) {
							wan_stop_status = stop_wan();
							if (wan_stop_status != 0) {
								action_result = rc_notify_merge_action_result(
									action_result, wan_stop_status);
								stop_failed = 1;
							} else {
								wan_stopped = 1;
							}
						}
						if (!stop_failed)
							action_result = rc_notify_merge_action_result(
								action_result, reload_modem_modules(modem_type, 1));
					}
				}
				if (!stop_failed)
					nvram_modem_type = modem_type;
			}
			if (!stop_failed && need_restart_wan)
				action_result = rc_notify_merge_action_result(
					action_result, full_restart_wan());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SPOOLER) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_usb_printer_spoolers());
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_printer"))
		{
			// deferred run usb printer daemons
			action_result = rc_notify_merge_action_result(
				action_result,
				rc_notify_set_deferred_hotplug_marker("on_hotplug_usb_printer", "usb_hotplug_lp"));
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_printer"))
		{
			// deferred stop usb printer daemons
			action_result = rc_notify_merge_action_result(
				action_result,
				rc_notify_set_deferred_hotplug_marker("on_unplug_usb_printer", "usb_unplug_lp"));
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_modem"))
		{
			// deferred run usb modem to wan
			action_result = rc_notify_merge_action_result(
				action_result,
				rc_notify_set_deferred_hotplug_marker("on_hotplug_usb_modem", "usb_hotplug_md"));
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_modem"))
		{
			// deferred restart wan
			action_result = rc_notify_merge_action_result(
				action_result,
				rc_notify_set_deferred_hotplug_marker("on_unplug_usb_modem", "usb_unplug_md"));
			alarm(5);
		}
#endif
#if defined (USE_STORAGE)
		else if (!strcmp(entry->d_name, "on_hotplug_mass_storage"))
		{
			// deferred run stor apps
			action_result = rc_notify_merge_action_result(
				action_result,
				rc_notify_set_deferred_hotplug_marker("on_hotplug_mass_storage", "usb_hotplug_ms"));
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_mass_storage"))
		{
			action_result = umount_ejected();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_HDDTUNE) == 0)
		{
			int hddtune_status = system("/sbin/hddtune.sh");

			if (hddtune_status != 0)
				logmessage(LOGNAME,
					   "HDD tuning script failed (wait status=%d)",
					   hddtune_status);
			action_result = rc_notify_merge_action_result(action_result,
							      hddtune_status);
			set_pagecache_reclaim();
		}
#if defined(APP_FTPD)
		else if (strcmp(entry->d_name, RCN_RESTART_FTPD) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_ftpd());
		}
#endif
#if defined(APP_SMBD)
		else if (strcmp(entry->d_name, RCN_RESTART_SMBD) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_smbd());
		}
#endif
#if defined(APP_NFSD)
		else if (strcmp(entry->d_name, RCN_RESTART_NFSD) == 0)
		{
			action_result = restart_nfsd();
		}
#endif
#if defined(APP_MINIDLNA)
		else if (strcmp(entry->d_name, "restart_dms_rescan") == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_dms(1));
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DMS) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_dms(0));
		}
#endif
#if defined(APP_FIREFLY)
		else if (strcmp(entry->d_name, RCN_RESTART_ITUNES) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_itunes());
		}
#endif
#if defined(APP_TRMD)
		else if (strcmp(entry->d_name, RCN_RESTART_TRMD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_torrent());
		}
#endif
#if defined(APP_ARIA)
		else if (strcmp(entry->d_name, RCN_RESTART_ARIA) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_aria());
		}
#endif
#endif
		else if (strcmp(entry->d_name, RCN_RESTART_HTTPD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_httpd());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TELNETD) == 0)
		{
			int stop_status = stop_telnetd();
			action_result = rc_notify_merge_action_result(action_result, stop_status);
			if (stop_status == 0)
				action_result = rc_notify_merge_action_result(action_result, start_telnetd());
		}
#if defined(APP_SSHD)
		else if (strcmp(entry->d_name, RCN_RESTART_SSHD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_sshd());
		}
#endif
#if defined(APP_SCUT)
		else if (strcmp(entry->d_name, RCN_RESTART_SCUT) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_scutclient());
		}
		else if (strcmp(entry->d_name, "stop_scutclient") == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, stop_scutclient());
		}
#endif
#if defined(APP_MENTOHUST)
		else if (strcmp(entry->d_name, RCN_RESTART_MENTOHUST) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_mentohust());
		}
		else if (strcmp(entry->d_name, "stop_mentohust") == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, stop_mentohust());
		}
#endif
#if defined(APP_TTYD)
		else if (strcmp(entry->d_name, RCN_RESTART_TTYD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_ttyd());
		}
#endif
#if defined(APP_SHADOWSOCKS)
		else if (strcmp(entry->d_name, RCN_RESTART_SHADOWSOCKS) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_ss());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SS_TUNNEL) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_ss_tunnel());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_CHNROUTE_UPD) == 0)
		{
			update_chnroute();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_GFWLIST_UPD) == 0)
		{
			update_gfwlist();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DLINK) == 0)
		{
			update_dlink();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_REDLINK) == 0)
		{
			reset_dlink();
		}
#endif
#if defined(APP_VLMCSD)
		else if (strcmp(entry->d_name, RCN_RESTART_VLMCSD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_vlmcsd());
			action_result = rc_notify_merge_action_result(action_result, restart_dhcpd());
		}
#endif
#if defined(APP_IPERF3)
		else if (strcmp(entry->d_name, RCN_RESTART_IPERF3) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_iperf3());
		}
#endif
#if defined(APP_ALIDDNS)
		else if (strcmp(entry->d_name, RCN_RESTART_ALIDDNS) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_aliddns());
		}
#endif
#if defined(APP_ZEROTIER)
		else if (strcmp(entry->d_name, RCN_RESTART_ZEROTIER) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_zerotier());
		}
#endif
#if defined(APP_DDNSTO)
		else if (strcmp(entry->d_name, RCN_RESTART_DDNSTO) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_ddnsto());
		}
#endif

#if defined(APP_ALDRIVER)
		else if (strcmp(entry->d_name, RCN_RESTART_ALDRIVER) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_aldriver());
		}
#endif
#if defined(APP_WIREGUARD)
		else if (strcmp(entry->d_name, RCN_RESTART_WIREGUARD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_wireguard());
		}
#endif
#if defined(APP_SQM)
		else if (strcmp(entry->d_name, RCN_RESTART_SQM) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_sqm());
		}
#endif
#if defined(APP_ADBYBY)
		else if (strcmp(entry->d_name, RCN_RESTART_ADBYBY) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_adbyby());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_UPDATEADB) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, update_adb());
		}
#endif
#if defined(APP_ADGUARDHOME)
		else if (strcmp(entry->d_name, RCN_RESTART_ADGUARDHOME) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_adguardhome());
		}
#endif
#if defined(APP_SMARTDNS)
		else if (strcmp(entry->d_name, RCN_RESTART_SMARTDNS) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_smartdns());
		}
#endif
#if defined(APP_FRP)
		else if (strcmp(entry->d_name, RCN_RESTART_FRP) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_frp());
		}
#endif
#if defined(APP_DNSFORWARDER)
		else if (strcmp(entry->d_name, RCN_RESTART_DNSFORWARDER) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_dnsforwarder());
		}
#endif
#if defined(APP_SMBD) || defined(APP_NMBD)
		else if (strcmp(entry->d_name, RCN_RESTART_NMBD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_nmbd());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_WINS) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_nmbd());
			action_result = rc_notify_merge_action_result(action_result, restart_dhcpd());
			action_result = rc_notify_merge_action_result(action_result, reapply_vpn_server());
		}
#endif
		else if (strcmp(entry->d_name, RCN_RESTART_LLTD) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_lltd());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_ADSC) == 0)
		{
			action_result = restart_infosvr();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_CROND) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_crond());
		}
		else if (strcmp(entry->d_name, RCN_REAPPLY_VPNSVR) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, reapply_vpn_server());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_VPNSVR) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_vpn_server());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_VPNCLI) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_vpn_client());
		}
		else if (strcmp(entry->d_name, "start_vpn_client") == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, start_vpn_client());
		}
		else if (strcmp(entry->d_name, "stop_vpn_client") == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, stop_vpn_client());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DDNS) == 0)
		{
			int is_ap_mode, ap_mode_status, ddns_status;
			int ddns_enabled_api_result, ddns_enabled_status, ddns_enabled_errno;
			int ddns_enabled;
			char *ddns_enabled_value = NULL;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "DDNS restart skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				ddns_status = ap_mode_status;
			} else {
				errno = 0;
				ddns_enabled_api_result = nvram_get_checked("ddns_enable_x",
								    &ddns_enabled_value);
				ddns_enabled_errno = errno;
				ddns_enabled_status = inadyn_ddns_enabled_status(
					ddns_enabled_api_result, ddns_enabled_errno,
					ddns_enabled_value, &ddns_enabled);
				if (ddns_enabled_status != 0) {
					logmessage(LOGNAME,
						   "DDNS restart skipped because enable state cannot be read (status=%d)",
						   ddns_enabled_status);
					ddns_status = ddns_enabled_status;
				} else {
					ddns_status = rc_ddns_restart_run(is_ap_mode,
								 ddns_enabled, start_ddns, stop_ddns);
				}
			}
			action_result = rc_notify_merge_action_result(action_result, ddns_status);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DI) == 0)
		{
			int is_ap_mode, ap_mode_status;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "Internet detection skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				action_result = rc_notify_merge_action_result(
					action_result, ap_mode_status);
			} else if (is_ap_mode || has_wan_ip4(0)) {
				action_result = rc_notify_merge_action_result(
					action_result, notify_run_detect_internet(2));
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DHCPD) == 0)
		{
			int is_ap_mode, ap_mode_status;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "DHCP service restart skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				action_result = rc_notify_merge_action_result(
					action_result, ap_mode_status);
			} else if (is_ap_mode) {
				int hosts_status = update_hosts_ap();

				action_result = rc_notify_merge_action_result(action_result, hosts_status);
				if (hosts_status == 0)
					action_result = rc_notify_merge_action_result(action_result, restart_dhcpd());
			} else {
				action_result = rc_notify_merge_action_result(action_result, restart_dhcpd());
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_UPNP) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_upnp());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SWITCH_CFG) == 0)
		{
			int is_ap_mode, ap_mode_status;

			ap_mode_status = get_ap_mode_status(&is_ap_mode);
			if (ap_mode_status != 0) {
				logmessage(LOGNAME,
					   "bridge configuration skipped because AP/router mode cannot be verified (status=%d)",
					   ap_mode_status);
				action_result = rc_notify_merge_action_result(
					action_result, ap_mode_status);
			} else {
				action_result = rc_notify_merge_action_result(
					action_result, config_bridge(is_ap_mode));
			}
			action_result = rc_notify_merge_action_result(action_result, switch_config_base());
			action_result = rc_notify_merge_action_result(action_result, switch_config_storm());
			action_result = rc_notify_merge_action_result(action_result, switch_config_link());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SWITCH_VLAN) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_switch_config_vlan());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SYSLOG) == 0)
		{
			int logger_stop_status = stop_logger();
			if (logger_stop_status != 0) {
				logmessage(LOGNAME, "syslog restart aborted because existing logger daemons did not stop (status=%d)",
					   logger_stop_status);
				action_result = -1;
			} else if (start_logger(0) != 0) {
				logmessage(LOGNAME, "syslog restart failed while starting replacement logger daemons");
				action_result = -1;
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_WDG) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, restart_watchdog_cpu());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TWEAKS) == 0)
		{
			notify_leds_detect_link();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_NETFILTER) == 0)
		{
			int conntrack_status;
			int router_mode_status = update_router_mode();

			if (router_mode_status == 0) {
				action_result = rc_notify_merge_action_result(
					action_result, reload_nat_modules());
				action_result = rc_notify_merge_action_result(
					action_result, restart_firewall());
			} else {
				logmessage(LOGNAME,
					   "skipping NAT/firewall reload because router-mode NVRAM refresh failed (status=%d)",
					   router_mode_status);
				action_result = rc_notify_merge_action_result(action_result, -1);
			}
			conntrack_status = flush_conntrack_table(NULL);
			if (conntrack_status != 0)
				logmessage(LOGNAME, "conntrack flush after netfilter restart failed (status=%d)",
					   conntrack_status);
			action_result = rc_notify_merge_action_result(action_result, conntrack_status);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_FIREWALL) == 0)
		{
			action_result = rc_notify_merge_action_result(action_result, reload_nat_modules());
			action_result = rc_notify_merge_action_result(
				action_result, restart_firewall());
		}
		else if (strcmp(entry->d_name, "restart_firewall_wan") == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_firewall());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_NTPC) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, notify_watchdog_time());
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TIME) == 0)
		{
			int logger_stop_status = stop_logger();
			int crond_status;
			if (logger_stop_status != 0) {
				logmessage(LOGNAME, "time update could not restart logger because existing daemons did not stop (status=%d)",
					   logger_stop_status);
				action_result = -1;
			}
			set_timezone();
			action_result = rc_notify_merge_action_result(
				action_result, notify_watchdog_time());
			notify_rstats_time();
			if (logger_stop_status == 0 && start_logger(0) != 0) {
				logmessage(LOGNAME, "time update failed while starting replacement logger daemons");
				action_result = -1;
			}
			crond_status = restart_crond();
			if (crond_status != 0) {
				logmessage(LOGNAME,
					   "time update failed while restarting crond (status=%d)",
					   crond_status);
				action_result = rc_notify_merge_action_result(
					action_result, crond_status);
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SYSCTL) == 0)
		{
			action_result = rc_notify_merge_action_result(
				action_result, restart_all_sysctl());
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_WIFI5))
		{
			int radio_on = get_enabled_radio_wl();
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			if (restart_wifi_wl(radio_on, 1) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_WIFI2))
		{
			int radio_on = get_enabled_radio_rt();
			if (radio_on)
				radio_on = is_radio_allowed_rt();
			if (restart_wifi_rt(radio_on, 1) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl"))
		{
			int guest_on = is_guest_allowed_wl();
			if (control_guest_wl(guest_on, 1) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt"))
		{
			int guest_on = is_guest_allowed_rt();
			if (control_guest_rt(guest_on, 1) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl_on"))
		{
			if (control_guest_wl(1, 0) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl_off"))
		{
			if (control_guest_wl(0, 0) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt_on"))
		{
			if (control_guest_rt(1, 0) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt_off"))
		{
			if (control_guest_rt(0, 0) < 0)
				action_result = -1;
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl"))
		{
			int radio_on = get_enabled_radio_wl();
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(1, radio_on, 1));
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt"))
		{
			int radio_on = get_enabled_radio_rt();
			if (radio_on)
				radio_on = is_radio_allowed_rt();
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(0, radio_on, 1));
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl_on"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(1, 1, 0));
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl_off"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(1, 0, 0));
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt_on"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(0, 1, 0));
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt_off"))
		{
			action_result = rc_notify_merge_action_result(
				action_result, rc_notify_apply_radio_control(0, 0, 0));
		}
		else if (!strcmp(entry->d_name, "control_wifi_config_wl"))
		{
#if BOARD_HAS_5G_RADIO
			if (gen_ralink_config_5g(0) < 0)
				action_result = -1;
#endif
		}
		else if (!strcmp(entry->d_name, "control_wifi_config_rt"))
		{
			if (gen_ralink_config_2g(0) < 0)
				action_result = -1;
		}
		else
		{
			dbg("WARNING: rc notified of unrecognized event `%s'.\n", entry->d_name);
		}
		
		/*
		 * Retain the incomplete marker only when another safe request
		 * for this event was queued while the current action was running.
		 */
		/* Keep the in-flight marker after any failed notification operation. */
		if (action_result < 0) {
			logmessage(LOGNAME, "Notification %s failed; completion marker retained", entry->d_name);
		} else if (rc_notify_queue_marker_is_safe_at(queue_fd, entry->d_name, 0) != 0) {
			unlinkat(queue_fd, entry->d_name, 0);
			unlinkat(incomplete_dir_fd, entry->d_name, 0);
		}
		if (stop_handle)
			break;
	}

	/*
	 * Standard signals coalesce. If a burst filled this bounded batch, the
	 * original SIGUSR1 may already have been consumed; explicitly reschedule
	 * one more pass when a safe request remains in the queue.
	 */
	if (i >= RC_NOTIFY_BATCH_LIMIT && !stop_handle) {
		for (;;) {
			errno = 0;
			pending_entry = readdir(directory);
			if (!pending_entry) {
				if (errno != 0)
					logmessage(LOGNAME, "cannot inspect remaining RC notifications (errno=%d)", errno);
				break;
			}
			if (!rc_notify_event_name_is_safe(pending_entry->d_name))
				continue;
			if (rc_notify_queue_marker_is_safe_at(queue_fd, pending_entry->d_name, 0) == 0) {
				queue_pending = 1;
				break;
			}
		}
	}
	if (rc_notify_batch_should_requeue(i, queue_pending, stop_handle) &&
	    kill(1, SIGUSR1) != 0)
		logmessage(LOGNAME, "cannot reschedule remaining RC notifications (errno=%d)", errno);

	closedir(directory);
	close(incomplete_dir_fd);
}

typedef struct {
	const char *name;
	int (*main)(int argc, char *argv[]);
} applet_rc_t;

static const applet_rc_t applets_rc[] = {
	{ "udhcpc.script",	udhcpc_main		},
	{ "udhcpc_lan.script",	udhcpc_lan_main		},
	{ "udhcpc_viptv.script",udhcpc_viptv_main	},
	{ "zcip.script",	zcip_main		},
	{ "zcip_viptv.script",	zcip_viptv_main		},
	{ "wpacli.script",	wpacli_main		},
	{ "ip-up",		ipup_main		},
	{ "ip-down",		ipdown_main		},
#if defined (USE_IPV6)
	{ "ipv6-up",		ipv6up_main		},
	{ "ipv6-down",		ipv6down_main		},
	{ "dhcp6c.script",	dhcp6c_main		},
#endif
	{ "ip-up.vpns",		ipup_vpns_main		},
	{ "ip-down.vpns",	ipdown_vpns_main	},
	{ "ip-up.vpnc",		ipup_vpnc_main		},
	{ "ip-down.vpnc",	ipdown_vpnc_main	},

#if defined (APP_OPENVPN)
	{ SCRIPT_OVPN_SERVER,	ovpn_server_script_main	},
	{ SCRIPT_OVPN_CLIENT,	ovpn_client_script_main	},
	{ "ovpn_export_client",	ovpn_server_expcli_main	},
#endif
#if defined (USE_MMC_SUPPORT)
	{ "mdev_mmc",		mdev_mmc_main		},
#endif
#if defined (USE_BLK_DEV_SD)
	{ "mdev_sd",		mdev_sd_main		},
#endif
#if defined (USE_USB_SUPPORT)
	{ "mdev_sg",		mdev_sg_main		},
	{ "mdev_sr",		mdev_sr_main		},
	{ "mdev_lp",		mdev_lp_main		},
	{ "mdev_tty",		mdev_tty_main		},
	{ "mdev_wdm",		mdev_wdm_main		},
	{ "mdev_net",		mdev_net_main		},
	{ "zerocd",		zerocd_main		},
#endif
	{ "ddns_updated",	ddns_updated_main	},
	{ "ntpc_updated",	ntpc_updated_main	},
	{ "ntpc_syncnow",	ntpc_syncnow_main	},

	{ "detect_wan",		detect_wan_main		},
	{ "detect_link",	detect_link_main	},
	{ "detect_internet",	detect_internet_main	},

	{ "watchdog",		watchdog_main		},
	{ "gpio_btn",		btn_main		},
	{ "rstats",		rstats_main		},

#if defined (USE_MTK_ESW) || defined (USE_MTK_GSW)
	{ "mtk_esw",		mtk_esw_main		},
#endif
#if defined (USE_RTL8367)
	{ "rtl8367",		rtl8367_main		},
#endif
#if defined (USE_RT3352_MII)
	{ "inicd",		inicd_main		},
#endif
	{ NULL, NULL }
};


int
main(int argc, char **argv)
{
	int ret;
	char *base = strrchr(argv[0], '/');
	const applet_rc_t *app;
	base = base ? base + 1 : argv[0];

	/* init */
	if (!strcmp(base, "init")) {
		if (getpid() != 1 ) {
			dbg("error: %s must be run as PID 1!\n", base);
			return -1;
		}
		init_main_loop();
		return 0;
	}

	/* stub for early kernel hotplug */
	if (!strcmp(base, "hotplug")) {
		return 0;
	}

	if (!strcmp(base, "reboot")) {
		return sys_exit();
	}

	if (!strcmp(base, "shutdown") || !strcmp(base, "halt")) {
		return sys_stop();
	}

	if (!strcmp(base, "rc")) {
		dbg("error: cannot run rc directly!\n");
		return EINVAL;
	}

	/* Set TZ for all rc programs */
	setenv_tz();

	/* Start applets */
	for (app = applets_rc; app->name; app++) {
		if (strcmp(base, app->name) == 0)
			return app->main(argc, argv);
	}

	ret = 0;
	if (!strcmp(base, "reset_to_defaults")) {
		if (erase_nvram() == 0)
			return sys_exit();
		ret = -1;
	}
	else if (!strcmp(base, "run_telnetd")) {
		ret = run_telnetd();
	}
	else if (!strcmp(base, "run_ftpsamba")) {
#if defined(APP_SMBD)
		ret = restart_smbd();
#else
		;
#endif
#if defined(APP_FTPD)
		{
			int ftp_status = restart_ftpd();
			if (ret == 0)
				ret = ftp_status;
		}
#endif
	}
#if defined(APP_SMBD)
	else if (!strcmp(base, "run_samba")) {
		ret = restart_smbd();
	}
#endif
#if defined(APP_FTPD)
	else if (!strcmp(base, "run_ftp")) {
		ret = restart_ftpd();
	}
#endif
#if defined(APP_NFSD)
	else if (!strcmp(base, "run_nfsd")) {
		ret = run_nfsd();
	}
#endif
#if defined(APP_MINIDLNA)
	else if (!strcmp(base, "run_minidlna")) {
		ret = restart_dms(0);
	}
#endif
#if defined(APP_FIREFLY)
	else if (!strcmp(base, "run_firefly")) {
		ret = restart_itunes();
	}
#endif
#if defined(APP_TRMD)
	else if (!strcmp(base, "run_transmission")) {
		ret = restart_torrent();
	}
#endif
#if defined(APP_ARIA)
	else if (!strcmp(base, "run_aria")) {
		ret = restart_aria();
	}
#endif
#if defined(APP_FTPD)
	else if (!strcmp(base, "stop_ftp")) {
		ret = stop_ftp();
	}
#endif
#if defined(APP_SMBD)
	else if (!strcmp(base, "stop_samba")) {
		ret = stop_samba(0);
	}
#endif
	else if (!strcmp(base, "stop_ftpsamba")) {
		ret = stop_ftpsamba();
	}
#if defined(APP_NFSD)
	else if (!strcmp(base, "stop_nfsd")) {
		ret = stop_nfsd();
	}
#endif
#if defined(APP_MINIDLNA)
	else if (!strcmp(base, "stop_minidlna")) {
		ret = stop_dms();
	}
#endif
#if defined(APP_FIREFLY)
	else if (!strcmp(base, "stop_firefly")) {
		ret = stop_itunes();
	}
#endif
#if defined(APP_TRMD)
	else if (!strcmp(base, "stop_transmission")) {
		ret = stop_torrent();
	}
#endif
#if defined(APP_ARIA)
	else if (!strcmp(base, "stop_aria")) {
		ret = stop_aria();
	}
#endif
	else if (!strcmp(base, "start_ddns")) {
		ret = start_ddns(1);
	}
	else if (!strcmp(base, "stop_wan")) {
		ret = notify_rc_status("manual_wan_disconnect");
	}
	else if (!strcmp(base, "restart_wan")) {
		ret = notify_rc_status("manual_wan_reconnect");
	}
	else if (!strcmp(base, "restart_dns")) {
		ret = restart_dns();
	}
	else if (!strcmp(base, "restart_dhcpd")) {
		ret = restart_dhcpd();
	}
	else if (!strcmp(base, "restart_vpn_server")) {
		ret = restart_vpn_server();
	}
	else if (!strcmp(base, "restart_vpn_client")) {
		ret = restart_vpn_client();
	}
	else if (!strcmp(base, "restart_networkmap")) {
		ret = restart_networkmap();
	}
	else if (!strcmp(base, "restart_firewall")) {
		ret = restart_firewall();
	}
	else if (!strcmp(base, "radio2_toggle")) {
		ret = rc_manual_toggle_radio_status(0, -1);
	}
	else if (!strcmp(base, "radio2_toggle_on")) {
		ret = rc_manual_toggle_radio_status(0, 1);
	}
	else if (!strcmp(base, "radio2_toggle_off")) {
		ret = rc_manual_toggle_radio_status(0, 0);
	}
	else if (!strcmp(base, "radio2_enable")) {
		ret = rc_manual_change_radio_status(0, 1);
	}
	else if (!strcmp(base, "radio2_disable")) {
		ret = rc_manual_change_radio_status(0, 0);
	}
	else if (!strcmp(base, "radio2_guest_enable")) {
		ret = manual_change_guest_rt(1) < 0 ? -1 : 0;
	}
	else if (!strcmp(base, "radio2_guest_disable")) {
		ret = manual_change_guest_rt(0) < 0 ? -1 : 0;
	}
	else if (!strcmp(base, "radio2_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wireless_mac(0, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wireless_mac(0);
		}
	}
	else if (!strcmp(base, "radio2_restart")) {
		int radio_on = get_enabled_radio_rt();
		if (radio_on)
			radio_on = is_radio_allowed_rt();
		ret = restart_wifi_rt(radio_on, 1);
	}
#if BOARD_HAS_5G_RADIO
	else if (!strcmp(base, "radio5_toggle")) {
		ret = rc_manual_toggle_radio_status(1, -1);
	}
	else if (!strcmp(base, "radio5_toggle_on")) {
		ret = rc_manual_toggle_radio_status(1, 1);
	}
	else if (!strcmp(base, "radio5_toggle_off")) {
		ret = rc_manual_toggle_radio_status(1, 0);
	}
	else if (!strcmp(base, "radio5_enable")) {
		ret = rc_manual_change_radio_status(1, 1);
	}
	else if (!strcmp(base, "radio5_disable")) {
		ret = rc_manual_change_radio_status(1, 0);
	}
	else if (!strcmp(base, "radio5_guest_enable")) {
		ret = manual_change_guest_wl(1) < 0 ? -1 : 0;
	}
	else if (!strcmp(base, "radio5_guest_disable")) {
		ret = manual_change_guest_wl(0) < 0 ? -1 : 0;
	}
	else if (!strcmp(base, "radio5_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wireless_mac(1, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wireless_mac(1);
		}
	}
	else if (!strcmp(base, "radio5_restart")) {
		int radio_on = get_enabled_radio_wl();
		if (radio_on)
			radio_on = is_radio_allowed_wl();
		ret = restart_wifi_wl(radio_on, 1);
	}
#endif
	else if (!strcmp(base, "lan_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wired_mac(0, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wired_mac(0);
		}
	}
	else if (!strcmp(base, "wan_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wired_mac(1, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wired_mac(1);
		}
	}
	else if (!strcmp(base, "leds_front")) {
		if (argc > 1) {
			int power_on = atoi(argv[1]);
			show_hide_front_leds(power_on);
		} else {
			printf("Usage: %s <show>\n\n", base);
		}
	}
	else if (!strcmp(base, "leds_ether")) {
		if (argc > 1) {
			int power_on = atoi(argv[1]);
			show_hide_ether_leds(power_on);
		} else {
			printf("Usage: %s <show>\n\n", base);
		}
	}
#if defined (USE_USB_SUPPORT)
	else if (!strcmp(base, "ejusb")) {
		int port = 0;
		unsigned int parsed_port = 0;
		char *devn = NULL;

		if (argc > 3)
			ret = EINVAL;
		else if (argc > 1) {
			if (strncmp(argv[1], "sd", 2) == 0) {
				if (argc > 2)
					ret = EINVAL;
				else
					devn = argv[1];
			} else if (storage_hotplug_parse_decimal_token(
					   argv[1], 0, BOARD_NUM_USB_PORTS,
					   &parsed_port) != 0) {
				ret = EINVAL;
			} else {
				port = (int)parsed_port;
				if (argc > 2)
					devn = argv[2];
			}
		}
		if (ret == 0)
			ret = safe_remove_usb_device(port, devn);
	}
	else if (!strcmp(base, "ejusb1")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = (argc > 2) ? EINVAL : safe_remove_usb_device(1, devn);
	}
#if (BOARD_NUM_USB_PORTS > 1)
	else if (!strcmp(base, "ejusb2")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = (argc > 2) ? EINVAL : safe_remove_usb_device(2, devn);
	}
#endif
#endif
#if defined (USE_ATA_SUPPORT)
	else if (!strcmp(base, "ejata")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = safe_remove_stor_device(ATA_VIRT_PORT_ID, ATA_VIRT_PORT_ID, devn, 1);
	}
#endif
#if defined (USE_MMC_SUPPORT)
	else if (!strcmp(base, "ejmmc")) {
		ret = safe_remove_stor_device(MMC_VIRT_PORT_ID, MMC_VIRT_PORT_ID, NULL, 0);
	}
#endif
#if defined (USE_STORAGE)
	else if (!strcmp(base, "ejall")) {
		ret = safe_remove_all_stor_devices(1);
	}
#endif
	else if (!strcmp(base, "pids")) {
		if (argc > 1)
			ret = pids_main(argv[1]);
		else
			ret = EINVAL;
	}
	else {
		printf("Unknown applet: %s\n", base);
		ret = EINVAL;
	}

	return ret;
}

