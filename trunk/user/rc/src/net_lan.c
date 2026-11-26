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
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include "rc.h"
#include <nvram_linux.h>
#include "dnsmasq_domain_policy.h"
#include "dhcp_lease_policy.h"
#include "lan_kernel_name_policy.h"
#include "lan_udhcpc_hostname_policy.h"
#include "lan_dhcp_callback_policy.h"
#include "wan_nvram_status_policy.h"
#include "route_status_policy.h"
#include "service_wait_policy.h"
#include <process_pidfile.h>
#include <rc_notify_queue.h>
#include "process_presence_start_policy.h"
#include "udhcpc_start_readiness.h"
#include "dnsmasq_file.h"
#include "dnsmasq_lifecycle_lock.h"
#include "switch.h"
#include "switch_status_policy.h"
#include "service_start_status.h"
#include "bridge_config_policy.h"
#include "bridge_init_policy.h"
#include "udhcpc_script_link_policy.h"
#include "wireless_module_load_policy.h"
#include <gpioutils.h>

#if BOARD_RAM_SIZE < 32
#define SHRINK_TX_QUEUE_LEN	(300)
#elif BOARD_RAM_SIZE < 64
#define SHRINK_TX_QUEUE_LEN	(600)
#endif

static char udhcpc_lan_state[16] = {0};

static int
lan_sethostname_status(const char *name, size_t length)
{
	if (sethostname(name, length) == 0)
		return 0;
	return errno ? errno : EIO;
}

static int
lan_setdomainname_status(const char *name, size_t length)
{
	if (setdomainname(name, length) == 0)
		return 0;
	return errno ? errno : EIO;
}

static const lan_kernel_name_ops_t lan_kernel_name_ops = {
	lan_sethostname_status,
	lan_setdomainname_status
};

static const rc_udhcpc_script_link_ops_t udhcpc_lan_script_link_ops = {
	symlink,
	readlink
};

static int lan_nvram_get_checked(void *context, const char *param_name, char **value);
static int lan_dhcp_snapshot_begin(void *opaque);
static int lan_dhcp_snapshot_get(void *opaque, const char *name, char **value);
static int lan_dhcp_snapshot_end(void *opaque);

/*
 * Keep NVRAM read failures distinct from absent keys before applying bridge
 * configuration. These wrappers share the same parser/error policy as WAN.
 */
static int
lan_nvram_get_int_status(const char *param_name, int *value)
{
	char *stored_value = NULL;
	int api_result, saved_errno;

	if (!param_name || !param_name[0] || !value)
		return EINVAL;

	errno = 0;
	api_result = nvram_get_checked(param_name, &stored_value);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(api_result, saved_errno,
					    stored_value, 0, value);
}

static int
lan_nvram_get_match_status(const char *param_name, const char *expected,
			   int *matches)
{
	char *stored_value = NULL;
	int api_result, saved_errno, read_status;

	if (!param_name || !param_name[0] || !expected || !matches)
		return EINVAL;
	*matches = 0;

	errno = 0;
	api_result = nvram_get_checked(param_name, &stored_value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (read_status != 0)
		return rc_wan_nvram_match_status(read_status, NULL,
						expected, matches);
	if (api_result == 0)
		stored_value = "";
	return rc_wan_nvram_match_status(0, stored_value, expected, matches);
}

static int
lan_wan_bridge_mode_status(int *bridge_mode)
{
	int configured_mode = 0;
	int status;

	if (!bridge_mode)
		return EINVAL;
	status = lan_nvram_get_int_status("wan_stb_x", &configured_mode);
	return rc_wan_nvram_int_range_status(status, configured_mode, 0, 7,
					     SWAPI_WAN_BRIDGE_DISABLE,
					     bridge_mode);
}

static int
lan_wan_bridge_iso_mode_status(int bridge_mode, int *bwan_isolation)
{
	int configured_isolation = 0;
	int status;

	if (!bwan_isolation)
		return EINVAL;
	if (bridge_mode == SWAPI_WAN_BRIDGE_DISABLE) {
		*bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;
		return 0;
	}
	status = lan_nvram_get_int_status("wan_stb_iso", &configured_isolation);
	return rc_wan_nvram_int_range_status(
		status, configured_isolation, 0, 2,
		SWAPI_WAN_BWAN_ISOLATION_NONE, bwan_isolation);
}

static int
lan_nvram_get_int_before_vlan_apply(const char *param_name, int *value)
{
	int status;

	status = lan_nvram_get_int_status(param_name, value);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read switch VLAN setting %s before rule updates (status=%d)",
			   param_name ? param_name : "(null)", status);
		errno = status;
	}
	return status;
}

static int
lan_nvram_get_int_before_switch_apply(const char *param_name, int *value)
{
	int status;

	status = lan_nvram_get_int_status(param_name, value);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read switch setting %s before hardware configuration (status=%d)",
			   param_name ? param_name : "(null)", status);
		errno = status;
	}
	return status;
}

static int
lan_nvram_get_int_range_logged_status(const char *param_name,
				      int minimum, int maximum,
				      int fallback, int *normalized)
{
	int configured_value = 0;
	int status;

	status = lan_nvram_get_int_status(param_name, &configured_value);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read switch storm-control setting %s (status=%d)",
			   param_name ? param_name : "(null)", status);
		errno = status;
		return status;
	}
	return rc_wan_nvram_int_range_status(
		status, configured_value, minimum, maximum,
		fallback, normalized);
}

in_addr_t get_lan_ip4(void)
{
	return get_interface_addr4(IFNAME_BR);
}

int
has_lan_ip4(void)
{
	if (get_lan_ip4() != INADDR_ANY)
		return 1;

	return 0;
}

int
has_lan_gw4(void)
{
	if (is_valid_ipv4(nvram_safe_get("lan_gateway_t")))
		return 1;

	return 0;
}

int
add_static_lan_routes(char *lan_ifname)
{
	return control_static_routes(SR_PREFIX_LAN, lan_ifname, 1);
}

int
del_static_lan_routes(char *lan_ifname)
{
	return control_static_routes(SR_PREFIX_LAN, lan_ifname, 0);
}

int
init_loopback(void)
{
	int status;

	/* The route depends on a successfully configured loopback address. */
	status = ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");
	if (status != 0) {
		logmessage(LOGNAME, "loopback interface setup failed (status=%d)", status);
		return status;
	}

	status = route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
	if (status != 0)
		logmessage(LOGNAME, "loopback route installation failed (status=%d)", status);

	return status;
}

static int
init_bridge_add_bridge(const char *bridge_ifname)
{
	int status;
	int saved_errno;
	int bridge_probe_status;

	status = br_add_del_bridge(bridge_ifname, 1);
	if (status == 0)
		return 0;

	saved_errno = errno;
	bridge_probe_status = (status < 0 && saved_errno == EEXIST) ?
		br_bridge_is_empty(bridge_ifname) : -1;
	status = rc_bridge_init_add_bridge_result(status, saved_errno,
						  bridge_probe_status);
	if (status != 0)
		errno = saved_errno;
	return status;
}

static int
init_bridge_load_module_verified(const char *module_name, const char *module_param)
{
	static const rc_module_load_ops_t ops = {
		module_smart_load,
		is_module_loaded_status
	};

	return rc_module_load_verified(module_name, module_param, &ops);
}

static int
bridge_wireless_nvram_read_int(void *context, const char *name, int *value)
{
	(void)context;
	return wireless_nvram_get_int_status(name, value);
}

int
init_bridge(int is_ap_mode, int *switch_vlan_status)
{
	rc_bridge_init_status_t bridge_status;
	int operation_status;
	int rt_ap_start_status = 0;
	int wl_ap_start_status = 0;
	int rt_guest_was_up = 0;
	int wl_guest_was_up = 0;
	int rt_guest_state_status;
	int wl_guest_state_status;
	int rt_guest_start_attempted = 0;
	int wl_guest_start_attempted = 0;
	int guest_isolation_status;
	int rt_radio_on = 0;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	int wan_bridge_mode = SWAPI_WAN_BRIDGE_DISABLE;
	int wan_port_uses_vlan = 0;
#endif
#if !defined(USE_RT3352_MII)
	int rt_mode_x = 0;
#endif
#if BOARD_HAS_5G_RADIO
	int wl_radio_on = 0;
	int wl_mode_x = 0;
#endif
	rc_bridge_wireless_state_t wireless_state;
	char lan_hwaddr[24];

	rc_bridge_init_status_reset(&bridge_status);
	if (switch_vlan_status)
		*switch_vlan_status = 0;

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	/*
	 * The router-mode bridge member depends on wan_stb_x. The legacy getter
	 * maps a failed checked read to DISABLE, which can attach eth2 instead of
	 * eth2.1. Resolve it before touching the switch or network interfaces.
	 */
	if (!is_ap_mode) {
		operation_status = lan_wan_bridge_mode_status(&wan_bridge_mode);
		operation_status = rc_bridge_init_wan_port_uses_vlan(
			operation_status, wan_bridge_mode, SWAPI_WAN_BRIDGE_DISABLE,
			&wan_port_uses_vlan);
		if (operation_status != 0) {
			errno = operation_status > 0 ? operation_status : EIO;
			logmessage(LOGNAME,
				   "cannot select LAN bridge WAN member before bridge setup (status=%d)",
				   operation_status);
			return operation_status;
		}
	}
#endif

	/* Checked selector snapshot precedes all switch and interface side effects. */
	operation_status = rc_bridge_wireless_state_read(
		bridge_wireless_nvram_read_int, NULL,
#if !defined(USE_RT3352_MII)
		1,
#else
		0,
#endif
#if BOARD_HAS_5G_RADIO
		1,
#else
		0,
#endif
		&wireless_state);
	if (operation_status != 0) {
		errno = operation_status > 0 ? operation_status : EIO;
		logmessage(LOGNAME,
			   "cannot read wireless radio selectors before bridge setup (status=%d)",
			   operation_status);
		return operation_status;
	}
	rt_radio_on = wireless_state.rt_radio_on;
#if !defined(USE_RT3352_MII)
	rt_mode_x = wireless_state.rt_mode_x;
#endif
#if BOARD_HAS_5G_RADIO
	wl_radio_on = wireless_state.wl_radio_on;
	wl_mode_x = wireless_state.wl_mode_x;
#endif

	/*
	 * The NVRAM getter returns shared cache storage. Switch configuration
	 * reads more keys before the MAC is applied, so retain an owned copy.
	 */
	operation_status = rc_wan_nvram_copy_value(
		lan_nvram_get_checked, NULL, "lan_hwaddr",
		lan_hwaddr, sizeof(lan_hwaddr));
	if (operation_status != 0) {
		errno = operation_status;
		logmessage(LOGNAME,
			   "cannot read LAN hardware address before bridge setup (status=%d)",
			   operation_status);
		return operation_status;
	}

	if (!is_ap_mode)
	{
		/* set switch bridge mode and vlan isolation */
		operation_status = switch_config_vlan(1);
		if (switch_vlan_status)
			*switch_vlan_status = operation_status;
		rc_bridge_init_record(&bridge_status, "switch VLAN setup", operation_status);
	}
	else
	{
		/* set switch bridge mode to LLLLL */
		operation_status = phy_bridge_mode(SWAPI_WAN_BRIDGE_DISABLE_WAN, SWAPI_WAN_BWAN_ISOLATION_NONE);
		rc_bridge_init_record(&bridge_status, "AP switch bridge mode", operation_status);
	}

#if BOARD_RAM_SIZE < 64
	doSystem("ifconfig %s txqueuelen %d", IFNAME_MAC, SHRINK_TX_QUEUE_LEN);
#endif
	operation_status = set_interface_hwaddr(IFNAME_MAC, lan_hwaddr);
	rc_bridge_init_record(&bridge_status, "set MAC interface address", operation_status);
	operation_status = ifconfig(IFNAME_MAC, IFUP, NULL, NULL);
	rc_bridge_init_record(&bridge_status, "bring up MAC interface", operation_status);

	operation_status = switch_config_base();
	rc_bridge_init_record(&bridge_status, "switch base configuration", operation_status);
	operation_status = switch_config_storm();
	rc_bridge_init_record(&bridge_status, "switch storm-control configuration", operation_status);
	operation_status = switch_config_link();
	rc_bridge_init_record(&bridge_status, "switch port link configuration", operation_status);

	/* power up all switch PHY */
	operation_status = phy_ports_power(1);
	rc_bridge_init_record(&bridge_status, "power up switch PHYs", operation_status);

#if defined (USE_SINGLE_MAC)
	if (!is_ap_mode)
	{
		/* create VLAN1/2 */
		operation_status = create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
		rc_bridge_init_record(&bridge_status, "create MAC VLAN1", operation_status);
		operation_status = create_vlan_iface(IFNAME_MAC, 2, -1, -1, NULL, 0);
		rc_bridge_init_record(&bridge_status, "create MAC VLAN2", operation_status);
	}
#if defined (AP_MODE_LAN_TAGGED)
	else
	{
		/* create VLAN1 */
		operation_status = create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
		rc_bridge_init_record(&bridge_status, "create AP MAC VLAN1", operation_status);
	}
#endif
#endif

#if defined (USE_RT3352_MII)
	/* create VLAN3 for guest AP */
	operation_status = create_vlan_iface(IFNAME_MAC, INIC_GUEST_VLAN_VID, -1, -1, lan_hwaddr, 1);
	rc_bridge_init_record(&bridge_status, "create iNIC guest VLAN", operation_status);
#endif

#if BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC)
#if !defined (USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces and fix iNIC issue (common PLL config) */
		operation_status = gen_ralink_config_2g(1);
		rc_bridge_init_record(&bridge_status, "prepare 2.4GHz wireless configuration", operation_status);
		operation_status = ifconfig(IFNAME_2G_MAIN, IFUP, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "bring up 2.4GHz main interface for radio configuration", operation_status);
	}
#endif
#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces */
		operation_status = gen_ralink_config_5g(1);
		rc_bridge_init_record(&bridge_status, "prepare 5GHz wireless configuration", operation_status);
		operation_status = ifconfig(IFNAME_5G_MAIN, IFUP, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "bring up 5GHz main interface for radio configuration", operation_status);
	}
#endif
#else /* BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC) */
#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces and fix iNIC issue (common PLL config) */
		operation_status = gen_ralink_config_5g(1);
		rc_bridge_init_record(&bridge_status, "prepare 5GHz wireless configuration", operation_status);
		operation_status = ifconfig(IFNAME_5G_MAIN, IFUP, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "bring up 5GHz main interface for radio configuration", operation_status);
	}
#endif
#if !defined (USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces */
		operation_status = gen_ralink_config_2g(1);
		rc_bridge_init_record(&bridge_status, "prepare 2.4GHz wireless configuration", operation_status);
		operation_status = ifconfig(IFNAME_2G_MAIN, IFUP, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "bring up 2.4GHz main interface for radio configuration", operation_status);
	}
#endif
#endif

	operation_status = init_bridge_add_bridge(IFNAME_BR);
	rc_bridge_init_record(&bridge_status, "create LAN bridge", operation_status);
	operation_status = br_set_stp(IFNAME_BR, 0);
	rc_bridge_init_record(&bridge_status, "set LAN bridge STP", operation_status);
	operation_status = br_set_fd(IFNAME_BR, 2);
	rc_bridge_init_record(&bridge_status, "set LAN bridge forward delay", operation_status);
	operation_status = set_interface_hwaddr(IFNAME_BR, lan_hwaddr);
	rc_bridge_init_record(&bridge_status, "set LAN bridge address", operation_status);

	if (!is_ap_mode) {
		/* add eth2 (or eth2.1) to bridge */
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
		if (wan_port_uses_vlan) {
			operation_status = create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
			rc_bridge_init_record(&bridge_status, "create GMAC2 VLAN1", operation_status);
			operation_status = br_add_del_if(IFNAME_BR, "eth2.1", 1);
			rc_bridge_init_record(&bridge_status, "attach GMAC2 VLAN1", operation_status);
		} else
#endif
		{
			operation_status = br_add_del_if(IFNAME_BR, IFNAME_LAN, 1);
			rc_bridge_init_record(&bridge_status, "attach router LAN port", operation_status);
		}
	} else {
#if defined (AP_MODE_LAN_TAGGED)
		/* add eth2 (or eth2.1) to bridge */
		operation_status = br_add_del_if(IFNAME_BR, IFNAME_LAN, 1);
		rc_bridge_init_record(&bridge_status, "attach AP LAN port", operation_status);
#else
		/* add only eth2 to bridge */
		operation_status = br_add_del_if(IFNAME_BR, IFNAME_MAC, 1);
		rc_bridge_init_record(&bridge_status, "attach AP MAC port", operation_status);
#endif
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
		/* add eth3 to bridge */
		operation_status = ifconfig(IFNAME_MAC2, IFUP, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "bring up GMAC2 secondary interface", operation_status);
		operation_status = br_add_del_if(IFNAME_BR, IFNAME_MAC2, 1);
		rc_bridge_init_record(&bridge_status, "attach GMAC2 secondary interface", operation_status);
#if defined (USE_HW_NAT)
		/* Hardware offload is optional; report accurately without failing bridge startup. */
		operation_status = init_bridge_load_module_verified("hw_nat", "ttl_regen=0");
		if (operation_status != 0)
			logmessage(LOGNAME, "optional hw_nat bridge offload unavailable (status=%d); continuing without offload",
				   operation_status);
		else
			logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", "Enabled, L2 bridge offload");
#endif
#endif
	}

#if defined(USE_RT3352_MII)
	{
		char inic_param[80];
		snprintf(inic_param, sizeof(inic_param), "miimaster=%s mode=%s syncmiimac=%d bridge=%d max_fw_upload=%d",
			IFNAME_MAC, "ap", 0, 1, 10);
		operation_status = init_bridge_load_module_verified("iNIC_mii", inic_param);
		rc_bridge_init_record(&bridge_status, "required iNIC MII module", operation_status);
	}
#endif

/*
	 * ENODEV means an optional guest VIF is absent yet (known DOWN). Other
	 * ioctl/socket failures are unknown; do not start that radio's AP.
	 */
	rt_guest_state_status = wireless_guest_rt_guest_is_up(&rt_guest_was_up);
	rc_bridge_init_record(&bridge_status,
		"inspect 2.4GHz guest AP before startup", rt_guest_state_status);
	wl_guest_state_status = wireless_guest_wl_guest_is_up(&wl_guest_was_up);
	rc_bridge_init_record(&bridge_status,
		"inspect 5GHz guest AP before startup", wl_guest_state_status);

#if BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC)
	if (rt_guest_state_status == 0) {
		rt_guest_start_attempted = rt_radio_on != 0;
		rt_ap_start_status = start_wifi_ap_rt(rt_radio_on);
	} else {
		rt_ap_start_status = rt_guest_state_status;
	}
	rc_bridge_init_record(&bridge_status, "2.4GHz AP radio startup", rt_ap_start_status);
	operation_status = start_wifi_wds_rt(rt_radio_on);
	rc_bridge_init_record(&bridge_status, "2.4GHz WDS radio startup", operation_status);
	operation_status = start_wifi_apcli_rt(rt_radio_on);
	rc_bridge_init_record(&bridge_status, "2.4GHz APCLI radio startup", operation_status);
#if BOARD_HAS_5G_RADIO
	if (wl_guest_state_status == 0) {
		wl_guest_start_attempted = wl_radio_on != 0;
		wl_ap_start_status = start_wifi_ap_wl(wl_radio_on);
	} else {
		wl_ap_start_status = wl_guest_state_status;
	}
	rc_bridge_init_record(&bridge_status, "5GHz AP radio startup", wl_ap_start_status);
	operation_status = start_wifi_wds_wl(wl_radio_on);
	rc_bridge_init_record(&bridge_status, "5GHz WDS radio startup", operation_status);
	operation_status = start_wifi_apcli_wl(wl_radio_on);
	rc_bridge_init_record(&bridge_status, "5GHz APCLI radio startup", operation_status);
#endif
#else /* BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC) */
#if BOARD_HAS_5G_RADIO
	if (wl_guest_state_status == 0) {
		wl_guest_start_attempted = wl_radio_on != 0;
		wl_ap_start_status = start_wifi_ap_wl(wl_radio_on);
	} else {
		wl_ap_start_status = wl_guest_state_status;
	}
	rc_bridge_init_record(&bridge_status, "5GHz AP radio startup", wl_ap_start_status);
	operation_status = start_wifi_wds_wl(wl_radio_on);
	rc_bridge_init_record(&bridge_status, "5GHz WDS radio startup", operation_status);
	operation_status = start_wifi_apcli_wl(wl_radio_on);
	rc_bridge_init_record(&bridge_status, "5GHz APCLI radio startup", operation_status);
#endif
	if (rt_guest_state_status == 0) {
		rt_guest_start_attempted = rt_radio_on != 0;
		rt_ap_start_status = start_wifi_ap_rt(rt_radio_on);
	} else {
		rt_ap_start_status = rt_guest_state_status;
	}
	rc_bridge_init_record(&bridge_status, "2.4GHz AP radio startup", rt_ap_start_status);
	operation_status = start_wifi_wds_rt(rt_radio_on);
	rc_bridge_init_record(&bridge_status, "2.4GHz WDS radio startup", operation_status);
	operation_status = start_wifi_apcli_rt(rt_radio_on);
	rc_bridge_init_record(&bridge_status, "2.4GHz APCLI radio startup", operation_status);
#endif

	if (rt_radio_on)
		LED_CONTROL(LED_SW2G, LED_ON);

#if BOARD_HAS_5G_RADIO
	if (wl_radio_on)
		LED_CONTROL(LED_SW5G, LED_ON);
#endif

	sleep(1);

#if BOARD_RAM_SIZE < 64
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_MAIN, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_GUEST, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_APCLI, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS0, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS1, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS2, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS3, SHRINK_TX_QUEUE_LEN);
#endif

	operation_status = ifconfig(IFNAME_BR, IFUP, NULL, NULL);
	rc_bridge_init_record(&bridge_status, "bring up LAN bridge", operation_status);

#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3)) {
		usleep(500000);
		operation_status = ifconfig(IFNAME_5G_MAIN, 0, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "restore 5GHz main interface down", operation_status);
		operation_status = gen_ralink_config_5g(0);
		rc_bridge_init_record(&bridge_status, "restore 5GHz wireless configuration", operation_status);
	}

	if (wl_radio_on)
		update_vga_clamp_wl(1);
#endif

#if !defined(USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3)) {
		usleep(500000);
		operation_status = ifconfig(IFNAME_2G_MAIN, 0, NULL, NULL);
		rc_bridge_init_record(&bridge_status, "restore 2.4GHz main interface down", operation_status);
		operation_status = gen_ralink_config_2g(0);
		rc_bridge_init_record(&bridge_status, "restore 2.4GHz wireless configuration", operation_status);
	}

	if (rt_radio_on)
		update_vga_clamp_rt(1);
#endif

	guest_isolation_status = restart_guest_lan_isolation();
	rc_bridge_init_record(&bridge_status, "guest LAN isolation", guest_isolation_status);

	/* Direct init_bridge AP startup bypasses restart_wifi_* fail-closed handling. */
	if (rt_ap_start_status != 0 || wl_ap_start_status != 0 ||
	    guest_isolation_status != 0) {
		operation_status = wireless_guest_init_bridge_fail_closed(
			rt_guest_was_up, wl_guest_was_up,
			rt_guest_start_attempted, wl_guest_start_attempted, 1);
		rc_bridge_init_record(&bridge_status,
			"guest AP rollback after LAN bridge startup failure",
			operation_status);
	}

	operation_status = nvram_set_int_temp("reload_svc_wl", 0);
	rc_bridge_init_record(&bridge_status,
			      "clear wireless service reload marker",
			      operation_status);
	operation_status = nvram_set_int_temp("reload_svc_rt", 0);
	rc_bridge_init_record(&bridge_status,
			      "clear router service reload marker",
			      operation_status);

	if (bridge_status.first_status != 0)
		logmessage(LOGNAME,
			   "LAN bridge initialization incomplete: %s failed first (status=%d; failed_operations=%u)",
			   bridge_status.first_operation ? bridge_status.first_operation : "unknown",
			   bridge_status.first_status, bridge_status.failure_count);
	return rc_bridge_init_result(&bridge_status);
}

int
config_bridge(int is_ap_mode)
{
    const char *wired_ifname;
    const char *first_failure = NULL;
    int multicast_router, multicast_querier, igmp_static_port;
    int igmp_snoop, wired_m2u, mr_enable = 0;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
    int bridge_mode;
#endif
    int failure_count = 0;
    int status;
    static const rc_bridge_config_ops_t ops = {
        br_set_param_int,
        brport_set_m2u,
        phy_igmp_static_port,
        phy_igmp_snooping
    };

    status = lan_nvram_get_int_status("ether_igmp", &igmp_snoop);
    if (status != 0) {
        logmessage(LOGNAME,
                   "cannot read LAN IGMP snooping setting before bridge configuration (status=%d)",
                   status);
        errno = status;
        return status;
    }
    status = lan_nvram_get_int_status("ether_m2u", &wired_m2u);
    if (status != 0) {
        logmessage(LOGNAME,
                   "cannot read LAN multicast-to-unicast setting before bridge configuration (status=%d)",
                   status);
        errno = status;
        return status;
    }

    if (!is_ap_mode) {
        igmp_static_port = -1;
        status = lan_nvram_get_match_status("mr_enable_x", "1", &mr_enable);
        if (status != 0) {
            logmessage(LOGNAME,
                       "cannot read multicast-router setting before bridge configuration (status=%d)",
                       status);
            errno = status;
            return status;
        }
        if (mr_enable) {
            multicast_router = 2;
            multicast_querier = 0;
        } else {
            multicast_router = 1;
            multicast_querier = 1;
        }
        wired_ifname = IFNAME_LAN;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
        status = lan_wan_bridge_mode_status(&bridge_mode);
        if (status != 0) {
            logmessage(LOGNAME,
                       "cannot read WAN bridge mode before LAN bridge configuration (status=%d)",
                       status);
            errno = status;
            return status;
        }
        if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE)
            wired_ifname = "eth2.1";
#endif
    } else {
        status = lan_nvram_get_int_status("ether_uport", &igmp_static_port);
        if (status != 0) {
            logmessage(LOGNAME,
                       "cannot read LAN IGMP static port before bridge configuration (status=%d)",
                       status);
            errno = status;
            return status;
        }
        multicast_router = 0;
        multicast_querier = 1;
#if defined (AP_MODE_LAN_TAGGED)
        wired_ifname = IFNAME_LAN;
#else
        wired_ifname = IFNAME_MAC;
#endif
    }

    status = rc_bridge_config_apply(
        &ops, IFNAME_BR, wired_ifname, multicast_router, multicast_querier,
        1, igmp_snoop ? 1 : 0,
        (igmp_snoop && wired_m2u == 1) ? 1 : 0,
        igmp_static_port,
        (igmp_snoop && wired_m2u == 2) ? 1 : 0,
        &failure_count, &first_failure);
    if (status != 0)
        logmessage(LOGNAME,
                   "LAN bridge multicast setup incomplete: %s failed first (status=%d; failed_operations=%d)",
                   first_failure ? first_failure : "validation",
                   status, failure_count);
    return status;
}

int
switch_config_link(void)
{
	rc_bridge_init_status_t link_status;
	int operation_status;
	int i, i_flow_mode, i_link_mode;
	char nvram_param[20];

	rc_bridge_init_status_reset(&link_status);
	i_link_mode = nvram_safe_get_int("ether_link_wan", SWAPI_LINK_SPEED_MODE_AUTO,
			SWAPI_LINK_SPEED_MODE_AUTO, SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF);
	i_flow_mode = nvram_safe_get_int("ether_flow_wan", SWAPI_LINK_FLOW_CONTROL_TX_RX,
			SWAPI_LINK_FLOW_CONTROL_TX_RX, SWAPI_LINK_FLOW_CONTROL_DISABLE);
	operation_status = phy_set_link_port(SWAPI_PORT_WAN, i_link_mode, i_flow_mode);
	rc_bridge_init_record(&link_status, "WAN link control", operation_status);

	for (i = 0; i < BOARD_NUM_ETH_EPHY-1; i++) {
		snprintf(nvram_param, sizeof(nvram_param), "ether_link_lan%d", i+1);
		i_link_mode = nvram_safe_get_int(nvram_param, SWAPI_LINK_SPEED_MODE_AUTO,
				SWAPI_LINK_SPEED_MODE_AUTO, SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF);
		snprintf(nvram_param, sizeof(nvram_param), "ether_flow_lan%d", i+1);
		i_flow_mode = nvram_safe_get_int(nvram_param, SWAPI_LINK_FLOW_CONTROL_TX_RX,
				SWAPI_LINK_FLOW_CONTROL_TX_RX, SWAPI_LINK_FLOW_CONTROL_DISABLE);
		operation_status = phy_set_link_port(SWAPI_PORT_LAN1+i, i_link_mode, i_flow_mode);
		rc_bridge_init_record(&link_status, "LAN link control", operation_status);
	}

	if (link_status.first_status != 0)
		logmessage(LOGNAME,
			   "switch port link configuration incomplete: %s failed first (status=%d; failed_operations=%u)",
			   link_status.first_operation ? link_status.first_operation : "unknown",
			   link_status.first_status, link_status.failure_count);
	return rc_bridge_init_result(&link_status);
}

int
switch_config_base(void)
{
	rc_bridge_init_status_t base_status;
	int operation_status;
	int read_status;
	int ether_jumbo, ether_green, ether_eee;

	rc_bridge_init_status_reset(&base_status);
	read_status = lan_nvram_get_int_before_switch_apply(
		"ether_jumbo", &ether_jumbo);
	if (read_status != 0)
		return read_status;
	read_status = lan_nvram_get_int_before_switch_apply(
		"ether_green", &ether_green);
	if (read_status != 0)
		return read_status;
	read_status = lan_nvram_get_int_before_switch_apply(
		"ether_eee", &ether_eee);
	if (read_status != 0)
		return read_status;

	/* LED indication is best-effort; it does not affect forwarding setup. */
	update_ether_leds();

	operation_status = phy_jumbo_frames(ether_jumbo);
	rc_bridge_init_record(&base_status, "jumbo frame setting", operation_status);
	operation_status = phy_green_ethernet(ether_green);
	rc_bridge_init_record(&base_status, "green Ethernet setting", operation_status);
	operation_status = phy_eee_lpi(ether_eee);
	rc_bridge_init_record(&base_status, "EEE LPI setting", operation_status);

	if (base_status.first_status != 0)
		logmessage(LOGNAME,
			   "switch base configuration incomplete: %s failed first (status=%d; failed_operations=%u)",
			   base_status.first_operation ? base_status.first_operation : "unknown",
			   base_status.first_status, base_status.failure_count);
	return rc_bridge_init_result(&base_status);
}

int
switch_config_storm(void)
{
	rc_bridge_init_status_t storm_status;
	int operation_status;
	int read_status;
	int controlrate_unknown_unicast;
	int controlrate_unknown_multicast;
	int controlrate_multicast;
	int controlrate_broadcast;

	rc_bridge_init_status_reset(&storm_status);
	read_status = lan_nvram_get_int_range_logged_status(
		"controlrate_unknown_unicast", 1, 1024, 1024,
		&controlrate_unknown_unicast);
	if (read_status != 0)
		return read_status;
	read_status = lan_nvram_get_int_range_logged_status(
		"controlrate_unknown_multicast", 1, 1024, 1024,
		&controlrate_unknown_multicast);
	if (read_status != 0)
		return read_status;
	read_status = lan_nvram_get_int_range_logged_status(
		"controlrate_multicast", 1, 1024, 1024,
		&controlrate_multicast);
	if (read_status != 0)
		return read_status;
	read_status = lan_nvram_get_int_range_logged_status(
		"controlrate_broadcast", 1, 1024, 1024,
		&controlrate_broadcast);
	if (read_status != 0)
		return read_status;

	operation_status = phy_storm_unicast_unknown(controlrate_unknown_unicast);
	rc_bridge_init_record(&storm_status, "unknown-unicast storm control", operation_status);
	operation_status = phy_storm_multicast_unknown(controlrate_unknown_multicast);
	rc_bridge_init_record(&storm_status, "unknown-multicast storm control", operation_status);
	operation_status = phy_storm_multicast(controlrate_multicast);
	rc_bridge_init_record(&storm_status, "multicast storm control", operation_status);
	operation_status = phy_storm_broadcast(controlrate_broadcast);
	rc_bridge_init_record(&storm_status, "broadcast storm control", operation_status);

	if (storm_status.first_status != 0)
		logmessage(LOGNAME,
			   "switch storm-control configuration incomplete: %s failed first (status=%d; failed_operations=%u)",
			   storm_status.first_operation ? storm_status.first_operation : "unknown",
			   storm_status.first_status, storm_status.failure_count);
	return rc_bridge_init_result(&storm_status);
}

int
switch_config_vlan(int first_call)
{
	rc_bridge_init_status_t vlan_status;
	int operation_status;
	int status;
	int bridge_mode, bwan_isolation, is_vlan_filter;
	int vlan_vid[SWAPI_VLAN_RULE_NUM] = {0};
	int vlan_pri[SWAPI_VLAN_RULE_NUM] = {0};
	int vlan_tag[SWAPI_VLAN_RULE_NUM] = {0};
	unsigned int vrule;

	rc_bridge_init_status_reset(&vlan_status);
	status = lan_wan_bridge_mode_status(&bridge_mode);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN bridge mode before switch VLAN configuration (status=%d)",
			   status);
		errno = status;
		return status;
	}
	status = lan_wan_bridge_iso_mode_status(bridge_mode, &bwan_isolation);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read WAN bridge isolation before switch VLAN configuration (status=%d)",
			   status);
		errno = status;
		return status;
	}

	status = lan_nvram_get_match_status("vlan_filter", "1", &is_vlan_filter);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read VLAN filter flag before switch VLAN configuration (status=%d)",
			   status);
		errno = status;
		return status;
	}
	if (is_vlan_filter) {
#if defined(USE_MTK_ESW)
		/* MT7620 and MT7628 ESW not support port matrix + security */
		if (bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
			bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;
#endif
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_cpu", &vlan_vid[SWAPI_VLAN_RULE_WAN_INET]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_iptv", &vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_lan1", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN1]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_lan2", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN2]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_lan3", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN3]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_vid_lan4", &vlan_vid[SWAPI_VLAN_RULE_WAN_LAN4]);
		if (status != 0)
			return status;

		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_cpu", &vlan_pri[SWAPI_VLAN_RULE_WAN_INET]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_INET] &= 0x07;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_iptv", &vlan_pri[SWAPI_VLAN_RULE_WAN_IPTV]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_IPTV] &= 0x07;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_lan1", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN1]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN1] &= 0x07;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_lan2", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN2]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN2] &= 0x07;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_lan3", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN3]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN3] &= 0x07;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_pri_lan4", &vlan_pri[SWAPI_VLAN_RULE_WAN_LAN4]);
		if (status != 0)
			return status;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN4] &= 0x07;

		vlan_tag[SWAPI_VLAN_RULE_WAN_INET] = 0;
		vlan_tag[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_tag_lan1", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN1]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_tag_lan2", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN2]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_tag_lan3", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN3]);
		if (status != 0)
			return status;
		status = lan_nvram_get_int_before_vlan_apply(
			"vlan_tag_lan4", &vlan_tag[SWAPI_VLAN_RULE_WAN_LAN4]);
		if (status != 0)
			return status;

		if (is_vlan_vid_valid(vlan_vid[SWAPI_VLAN_RULE_WAN_INET]))
			vlan_tag[SWAPI_VLAN_RULE_WAN_INET] = 1;
		else
			vlan_vid[SWAPI_VLAN_RULE_WAN_INET] = 0;

		if (is_vlan_vid_valid(vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV]))
			vlan_tag[SWAPI_VLAN_RULE_WAN_IPTV] = 1;
		else
			vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
	}

	/* set vlan rule before change bridge mode! */
	for (vrule = 0; vrule < SWAPI_VLAN_RULE_NUM; vrule++) {
		operation_status = phy_vlan_rule_set(vrule, vlan_vid[vrule], vlan_pri[vrule], vlan_tag[vrule]);
		rc_bridge_init_record(&vlan_status, "VLAN rule", operation_status);
	}

	operation_status = phy_bridge_mode(bridge_mode, bwan_isolation);
	rc_bridge_init_record(&vlan_status, "switch bridge mode", operation_status);

#if defined(USE_RT3352_MII)
	if (!first_call) {
		// clear isolation iNIC port from all LAN ports
		if (is_interface_up(IFNAME_INIC_MAIN) && get_mlme_radio_rt()) {
			operation_status = phy_isolate_inic(0);
			if (operation_status != 0)
				logmessage(LOGNAME, "optional iNIC isolation reset failed (status=%d)", operation_status);
		}
	}
#endif

	if (vlan_status.first_status != 0)
		logmessage(LOGNAME,
			   "switch VLAN initialization incomplete: %s failed first (status=%d; failed_operations=%u)",
			   vlan_status.first_operation ? vlan_status.first_operation : "unknown",
			   vlan_status.first_status, vlan_status.failure_count);
	return rc_bridge_init_result(&vlan_status);
}

int
restart_switch_config_vlan(void)
{
	int start_status = 0;
	int operation_status;
	int is_ap_mode, ap_mode_status;
#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	int pvid_wan_before, pvid_wan_after;
#endif

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "switch VLAN restart aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return 0;

#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	pvid_wan_before = phy_vlan_pvid_wan_get();
#endif

	notify_reset_detect_link();
	operation_status = switch_config_vlan(0);
	start_status = rc_first_service_start_status(start_status, operation_status);

#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	pvid_wan_after = phy_vlan_pvid_wan_get();
	if (pvid_wan_before < 0 || pvid_wan_after < 0) {
		logmessage(LOGNAME, "unable to compare WAN switch PVID after VLAN reconfiguration (%d, %d)",
			   pvid_wan_before, pvid_wan_after);
	} else if (switch_status_vlan_pvid_changed(pvid_wan_before, pvid_wan_after)) {
		operation_status = full_restart_wan();
		start_status = rc_first_service_start_status(start_status, operation_status);
	}
#endif
	return start_status;
}

int
is_vlan_vid_valid(int vlan_vid)
{
	if (vlan_vid == 2)
		return 1;
	return (vlan_vid >= MIN_EXT_VLAN_VID && vlan_vid < 4095) ? 1 : 0;
}

void
update_ether_leds(void)
{
#if (BOARD_NUM_ETH_LEDS > 1)
	int led0, led1, led_enabled, status;

	status = lan_nvram_get_int_status("ether_led0", &led0);
	if (status == 0)
		status = lan_nvram_get_int_status("ether_led1", &led1);
	if (status == 0)
		status = lan_nvram_get_int_status("led_ether_t", &led_enabled);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read Ethernet LED configuration; preserving hardware state (status=%d)",
			   status);
		errno = status;
		return;
	}

	if (!led_enabled) {
		led0 = SWAPI_LED_OFF;
		led1 = SWAPI_LED_OFF;
	}
#if BOARD_ETH_LED_SWAP
	phy_led_mode_green(led1);
	phy_led_mode_yellow(led0);
#else
	phy_led_mode_green(led0);
	phy_led_mode_yellow(led1);
#endif
#elif (BOARD_NUM_ETH_LEDS == 1)
	int led0, led_enabled, status;

	status = lan_nvram_get_int_status("ether_led0", &led0);
	if (status == 0)
		status = lan_nvram_get_int_status("led_ether_t", &led_enabled);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read Ethernet LED configuration; preserving hardware state (status=%d)",
			   status);
		errno = status;
		return;
	}

	if (!led_enabled)
		led0 = SWAPI_LED_OFF;
#if BOARD_ETH_LED_SWAP
	phy_led_mode_yellow(led0);
#else
	phy_led_mode_green(led0);
#endif
#endif
}

struct lan_nvram_set_context {
	int temporary;
};

static int
lan_nvram_get_checked(void *context, const char *param_name, char **value)
{
	(void)context;
	return nvram_get_checked(param_name, value);
}

static int
lan_nvram_set_value_status(void *context, const char *param_name,
			   const char *value)
{
	const struct lan_nvram_set_context *set_context =
		(const struct lan_nvram_set_context *)context;
	int api_result, saved_errno;

	errno = 0;
	if (set_context && set_context->temporary)
		api_result = nvram_set_temp(param_name, value);
	else
		api_result = nvram_set(param_name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

int
reset_lan_temp_status(void)
{
	char lan_ipaddr[16], lan_netmask[16];
	struct nvram_transaction snapshot_transaction;
	struct rc_wan_nvram_snapshot_field snapshot_fields[2];
	struct rc_wan_nvram_value persistent_values[2];
	struct rc_wan_nvram_value runtime_values[5];
	const struct lan_nvram_set_context persistent_context = { 0 };
	const struct lan_nvram_set_context temporary_context = { 1 };
	unsigned int persistent_count = 0;
	int status;

	/*
	 * Defaulting and runtime publication operate on one persistent subnet
	 * generation; never combine an address and mask across separate commits.
	 */
	snapshot_fields[0].param_name = "lan_ipaddr";
	snapshot_fields[0].destination = lan_ipaddr;
	snapshot_fields[0].destination_size = sizeof(lan_ipaddr);
	snapshot_fields[1].param_name = "lan_netmask";
	snapshot_fields[1].destination = lan_netmask;
	snapshot_fields[1].destination_size = sizeof(lan_netmask);
	status = rc_wan_nvram_copy_snapshot(
		snapshot_fields, 2, lan_dhcp_snapshot_begin,
		lan_dhcp_snapshot_get, lan_dhcp_snapshot_end,
		&snapshot_transaction);
	if (status != 0)
		goto failed;

	/*
	 * Preserve the legacy defaulting rules, but never continue with a
	 * partially written persistent address/mask pair.
	 */
	if (!lan_ipaddr[0]) {
		status = rc_wan_nvram_copy_text(DEF_LAN_ADDR, lan_ipaddr,
					       sizeof(lan_ipaddr));
		if (status != 0)
			goto failed;
		status = rc_wan_nvram_copy_text(DEF_LAN_MASK, lan_netmask,
					       sizeof(lan_netmask));
		if (status != 0)
			goto failed;
		persistent_values[persistent_count].param_name = "lan_ipaddr";
		persistent_values[persistent_count++].value = lan_ipaddr;
		persistent_values[persistent_count].param_name = "lan_netmask";
		persistent_values[persistent_count++].value = lan_netmask;
	} else if (!lan_netmask[0]) {
		status = rc_wan_nvram_copy_text(DEF_LAN_MASK, lan_netmask,
					       sizeof(lan_netmask));
		if (status != 0)
			goto failed;
		persistent_values[persistent_count].param_name = "lan_netmask";
		persistent_values[persistent_count++].value = lan_netmask;
	}

	if (persistent_count != 0) {
		status = rc_wan_nvram_set_required_values(
			persistent_values, persistent_count,
			lan_nvram_set_value_status, (void *)&persistent_context);
		if (status != 0)
			goto failed;
	}

	runtime_values[0].param_name = "lan_ipaddr_t";
	runtime_values[0].value = lan_ipaddr;
	runtime_values[1].param_name = "lan_netmask_t";
	runtime_values[1].value = lan_netmask;
	runtime_values[2].param_name = "lan_gateway_t";
	runtime_values[2].value = "";
	runtime_values[3].param_name = "lan_domain_t";
	runtime_values[3].value = "";
	runtime_values[4].param_name = "lan_dns_t";
	runtime_values[4].value = "";

	status = rc_wan_nvram_set_required_values(
		runtime_values, 5, lan_nvram_set_value_status,
		(void *)&temporary_context);
	if (status != 0)
		goto failed;
	return 0;

failed:
	if (status <= 0)
		status = EIO;
	errno = status;
	logmessage(LOGNAME,
		   "cannot initialize required LAN address/runtime NVRAM state (status=%d)",
		   status);
	return status;
}

int
reset_lan_vars_status(void)
{
	char lan_hwaddr[24];
	struct rc_wan_nvram_value value;
	const struct lan_nvram_set_context persistent_context = { 0 };
	int status;

	status = rc_wan_nvram_copy_value(lan_nvram_get_checked, NULL,
					 "il0macaddr", lan_hwaddr,
					 sizeof(lan_hwaddr));
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot read required LAN hardware address (status=%d)",
			   status);
		return status;
	}

	value.param_name = "lan_hwaddr";
	value.value = lan_hwaddr;
	status = rc_wan_nvram_set_required_values(
		&value, 1, lan_nvram_set_value_status,
		(void *)&persistent_context);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot publish required LAN hardware address (status=%d)",
			   status);
	}
	return status;
}

void
reset_lan_temp(void)
{
	(void)reset_lan_temp_status();
}

void
reset_lan_vars(void)
{
	(void)reset_lan_vars_status();
}

static int
create_hosts_lan(const char *lan_ipaddr, const char *lan_dname)
{
	char hostname_backup[] = "/etc/hostname.rollback.XXXXXX";
	char hosts_backup[] = "/etc/hosts.rollback.XXXXXX";
	FILE *hostname_fp = NULL, *hosts_fp = NULL;
	char hostname_temp[] = "/etc/hostname.XXXXXX";
	char hosts_temp[] = "/etc/hosts.XXXXXX";
	char *lan_hname;
	int saved_errno = 0;
	int hosts_lock = -1;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status, lan_addr6_status;
	char lan_addr6_host[INET6_ADDRSTRLEN] = {0};
#endif

	lan_hname = get_our_hostname();
	if (!lan_hname) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		logmessage(LOGNAME,
			   "cannot read local hostname before LAN hostname publication (status=%d)",
			   saved_errno);
		return saved_errno;
	}

	if (!dhcp_lease_ipv4_is_valid(lan_ipaddr) || !lan_dname ||
	    (*lan_dname && !dnsmasq_valid_server_domain(lan_dname)) ||
	    !is_valid_hostname(lan_hname)) {
		errno = EINVAL;
		logmessage(LOGNAME, "refusing unsafe LAN hostname input");
		return EINVAL;
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "refusing to publish LAN hostname files without verified IPv6 service state (status=%d)",
			   ipv6_type_status);
		return ipv6_type_status;
	}
	if (ipv6_type != IPV6_DISABLED) {
		lan_addr6_status = get_lan_addr6_host_status(
			lan_addr6_host, sizeof(lan_addr6_host));
		if (lan_addr6_status != 0) {
			errno = lan_addr6_status;
			logmessage(LOGNAME,
				   "refusing to publish hostname files without a verified LAN IPv6 address snapshot (status=%d)",
				   lan_addr6_status);
			return lan_addr6_status;
		}
	}
#endif

	/* Stage both files completely before replacing either live pathname. */
	hostname_fp = dnsmasq_file_open(hostname_temp);
	if (!hostname_fp)
		goto failed;
	hosts_fp = dnsmasq_file_open(hosts_temp);
	if (!hosts_fp)
		goto failed;

	fprintf(hostname_fp, "%s\n", lan_hname);
	fprintf(hosts_fp, "%s %s %s\n", "127.0.0.1", "localhost.localdomain", "localhost");
	if (strlen(lan_dname) > 0)
		fprintf(hosts_fp, "%s %s.%s %s\n", lan_ipaddr, lan_hname, lan_dname, lan_hname);
	else
		fprintf(hosts_fp, "%s %s\n", lan_ipaddr, lan_hname);
	fprintf(hosts_fp, "%s %s\n", lan_ipaddr, "my.router");

	/* Preserve administrator-defined host entries across AP lease updates. */
	if (dnsmasq_file_is_safe_regular("/tmp/hosts.static") == 0) {
		saved_errno = load_user_config(hosts_fp, "/tmp", "hosts.static", NULL);
		if (saved_errno != 0)
			goto failed;
	} else if (errno != ENOENT) {
		logmessage(LOGNAME, "skipping unsafe static DHCP host snapshot (errno=%d)", errno);
	}

#if defined (USE_IPV6)
	if (ipv6_type != IPV6_DISABLED) {
		fprintf(hosts_fp, "%s %s %s\n", "::1", "localhost.localdomain", "localhost");
		if (lan_addr6_host[0]) {
			if (strlen(lan_dname) > 0)
				fprintf(hosts_fp, "%s %s.%s %s\n", lan_addr6_host, lan_hname, lan_dname, lan_hname);
			else
				fprintf(hosts_fp, "%s %s\n", lan_addr6_host, lan_hname);
			fprintf(hosts_fp, "%s %s\n", lan_addr6_host, "my.router");
		}
	}
#endif

	hosts_lock = dnsmasq_lifecycle_lock_acquire_path(
		DNSMASQ_HOSTNAME_LOCK_FILE, DNSMASQ_LIFECYCLE_LOCK_WAIT_SECONDS);
	if (hosts_lock < 0) {
		saved_errno = errno ? errno : EIO;
		goto failed;
	}
	{
		FILE *staged_hostname_fp = hostname_fp;
		FILE *staged_hosts_fp = hosts_fp;
		hostname_fp = NULL;
		hosts_fp = NULL;
		if (dnsmasq_file_publish_pair(staged_hostname_fp, hostname_temp,
					      "/etc/hostname", hostname_backup,
					      staged_hosts_fp, hosts_temp,
					      "/etc/hosts", hosts_backup, 0644) != 0) {
			saved_errno = errno ? errno : EIO;
			goto failed;
		}
	}

	/* Retry backup cleanup once while the unique rollback names remain live. */
	if (dnsmasq_file_backup_discard(hostname_backup) != 0)
		logmessage(LOGNAME, "cannot remove LAN hostname rollback snapshot '%s' (errno=%d)",
			   hostname_backup, errno ? errno : EIO);
	if (dnsmasq_file_backup_discard(hosts_backup) != 0)
		logmessage(LOGNAME, "cannot remove LAN hosts rollback snapshot '%s' (errno=%d)",
			   hosts_backup, errno ? errno : EIO);

	/* Do not expose a new kernel name until both on-disk files are complete. */
	saved_errno = lan_kernel_names_apply(lan_hname, lan_dname,
					     &lan_kernel_name_ops);
	if (saved_errno != 0)
		logmessage(LOGNAME,
			   "kernel hostname/domain update failed (status=%d)",
			   saved_errno);

	if (dnsmasq_lifecycle_lock_release(hosts_lock) != 0) {
		int release_status = errno ? errno : EIO;

		if (saved_errno == 0)
			saved_errno = release_status;
		else
			logmessage(LOGNAME,
				   "hostname file lock release also failed (status=%d; first_error=%d)",
				   release_status, saved_errno);
	}
	hosts_lock = -1;
	if (saved_errno != 0) {
		errno = saved_errno;
		return saved_errno;
	}
	return 0;

failed:
	if (hosts_lock >= 0) {
		if (dnsmasq_lifecycle_lock_release(hosts_lock) != 0)
			logmessage(LOGNAME, "cannot release hostname file lock (errno=%d)", errno ? errno : EIO);
		hosts_lock = -1;
	}
	if (!saved_errno)
		saved_errno = errno ? errno : EIO;
	if (hostname_fp)
		fclose(hostname_fp);
	if (hosts_fp)
		fclose(hosts_fp);
	if (dnsmasq_file_discard(hostname_temp) != 0)
		logmessage(LOGNAME, "cannot remove LAN hostname stage '%s' (errno=%d)",
			   hostname_temp, errno ? errno : EIO);
	if (dnsmasq_file_discard(hosts_temp) != 0)
		logmessage(LOGNAME, "cannot remove LAN hosts stage '%s' (errno=%d)",
			   hosts_temp, errno ? errno : EIO);
	if (hostname_backup[0])
		logmessage(LOGNAME, "retaining LAN hostname rollback snapshot '%s' for recovery", hostname_backup);
	if (hosts_backup[0])
		logmessage(LOGNAME, "retaining LAN hosts rollback snapshot '%s' for recovery", hosts_backup);
	logmessage(LOGNAME, "cannot stage/publish LAN hostname files (errno=%d)", saved_errno);
	errno = saved_errno;
	return saved_errno;
}

int
update_hosts_ap(void)
{
	char lan_ipaddr[16];
	char lan_dname[256];
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[2];
	int status;

	/*
	 * AP hostname files must pair the current lease address with the domain
	 * published by that same bound callback, never across DHCP renewals.
	 */
	fields[0].param_name = "lan_ipaddr_t";
	fields[0].destination = lan_ipaddr;
	fields[0].destination_size = sizeof(lan_ipaddr);
	fields[1].param_name = "lan_domain_t";
	fields[1].destination = lan_dname;
	fields[1].destination_size = sizeof(lan_dname);
	status = rc_wan_nvram_copy_snapshot(
		fields, 2, lan_dhcp_snapshot_begin,
		lan_dhcp_snapshot_get, lan_dhcp_snapshot_end,
		&transaction);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot snapshot AP LAN host identity (status=%d)",
			   status);
		return status;
	}

	return create_hosts_lan(lan_ipaddr, lan_dname);
}

int
start_lan(int is_ap_mode, int do_wait)
{
	char lan_ipaddr[16];
	char lan_netmsk[16];
	char lan_dname[256];
	struct nvram_transaction identity_transaction;
	struct rc_wan_nvram_snapshot_field identity_fields[3];
	unsigned int identity_count = 2;
	char *lan_ifname = IFNAME_BR;
	int start_status = 0;
	int operation_status;
	int phy_wait_status = 0;
	int metadata_status = 0;
	int lan_proto_dhcp = 0;
	int lan_proto_status;
#if defined (USE_IPV6)
	int ipv6_type, ipv6_type_status;
#endif

	/*
	 * Snapshot the LAN address, mask and (in AP mode) domain together.
	 * Owned buffers prevent cache reuse; one NVRAM transaction also prevents
	 * a concurrent config commit from mixing subnet generations.
	 */
	identity_fields[0].param_name = "lan_ipaddr";
	identity_fields[0].destination = lan_ipaddr;
	identity_fields[0].destination_size = sizeof(lan_ipaddr);
	identity_fields[1].param_name = "lan_netmask";
	identity_fields[1].destination = lan_netmsk;
	identity_fields[1].destination_size = sizeof(lan_netmsk);
	if (is_ap_mode) {
		identity_fields[2].param_name = "lan_domain";
		identity_fields[2].destination = lan_dname;
		identity_fields[2].destination_size = sizeof(lan_dname);
		identity_count = 3;
	}
	operation_status = rc_wan_nvram_copy_snapshot(
		identity_fields, identity_count,
		lan_dhcp_snapshot_begin, lan_dhcp_snapshot_get,
		lan_dhcp_snapshot_end, &identity_transaction);
	if (operation_status != 0) {
		errno = operation_status;
		logmessage(LOGNAME,
			   "cannot snapshot LAN identity before interface activation (status=%d)",
			   operation_status);
		return operation_status;
	}

	/* Bring up and configure the LAN bridge. */
	operation_status = ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
	if (operation_status != 0)
		logmessage(LOGNAME, "LAN bridge address setup failed (status=%d)",
			   operation_status);
	start_status = rc_first_service_start_status(start_status, operation_status);

	/*
	* Configure DHCP connection. The DHCP client will run
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address
	* renew and release.
	*/
	if (is_ap_mode) {
		{
			int hosts_status = create_hosts_lan(lan_ipaddr, lan_dname);
			if (hosts_status != 0)
				logmessage(LOGNAME,
					   "initial AP hostname publication failed; DNSMasq preparation will retry (status=%d)",
					   hosts_status);
		}

		lan_proto_status = lan_nvram_get_match_status("lan_proto_x", "1",
						   &lan_proto_dhcp);
		lan_proto_status = lan_dhcp_ap_mode_resolve(
			lan_proto_status, lan_proto_dhcp, &lan_proto_dhcp);
		if (lan_proto_status != 0) {
			logmessage(LOGNAME,
				   "cannot verify AP LAN protocol mode; refusing DHCP/manual setup (status=%d)",
				   lan_proto_status);
			start_status = rc_first_service_start_status(
				start_status, lan_proto_status);
		} else if (lan_proto_dhcp) {

			/* The callback script is required for bound/renew/deconfig events. */
			operation_status = rc_udhcpc_script_link_ensure(
				SCRIPT_UDHCPC_LAN, "/sbin/rc", &udhcpc_lan_script_link_ops);
			if (operation_status != 0)
				logmessage(LOGNAME,
					   "LAN DHCP event-handler link setup failed; refusing client launch (status=%d)",
					   operation_status);
			start_status = rc_first_service_start_status(start_status, operation_status);

			if (operation_status == 0) {
				/* early fill XXX_t fields */
				metadata_status = update_lan_status(0);
				if (metadata_status != 0)
					logmessage(LOGNAME,
						   "LAN runtime metadata refresh failed; refusing DHCP client launch (status=%d)",
						   metadata_status);
				start_status = rc_first_service_start_status(
					start_status, metadata_status);

				/* Complete the PHY link-settling interval despite signal interruptions. */
				if (do_wait) {
					phy_wait_status = service_wait_seconds(1);
					if (phy_wait_status != 0) {
						phy_wait_status = errno > 0 ? errno : EIO;
						errno = phy_wait_status;
						logmessage(LOGNAME,
							   "LAN PHY link readiness wait failed; refusing DHCP client launch (status=%d)",
							   phy_wait_status);
					}
				}
				start_status = rc_first_service_start_status(start_status, phy_wait_status);

				/* The monitor notification is independent of the mandatory PHY wait. */
				operation_status = notify_run_detect_internet(60);
				if (operation_status != 0)
					logmessage(LOGNAME, "LAN Internet monitor startup incomplete (status=%d)", operation_status);
				start_status = rc_first_service_start_status(start_status, operation_status);

				/* A monitor notification failure must not bypass or suppress the PHY gate. */
				if (rc_lan_ap_dhcp_start_allowed_after_phy_wait(phy_wait_status) &&
				    metadata_status == 0) {
					/* start DHCP daemon only when its event handler is verified */
					operation_status = start_udhcpc_lan(lan_ifname);
					if (operation_status != 0)
						logmessage(LOGNAME, "LAN DHCP client launch failed (status=%d)",
							   operation_status);
				}
				start_status = rc_first_service_start_status(start_status, operation_status);
			}
		} else {

			/* manual config lan gateway and dns */
			operation_status = lan_up_manual(lan_ifname, lan_dname);
			if (operation_status != 0)
				logmessage(LOGNAME, "manual LAN gateway/resolver setup failed (status=%d)",
					   operation_status);
			start_status = rc_first_service_start_status(start_status, operation_status);

			/* DI wakeup after 2 secs; retain launch/inventory/signal errors. */
			operation_status = notify_run_detect_internet(2);
			if (operation_status != 0)
				logmessage(LOGNAME, "LAN Internet monitor notification incomplete (status=%d)", operation_status);
			start_status = rc_first_service_start_status(start_status, operation_status);
		}
	} else {

		/* install lan specific static routes */
		operation_status = add_static_lan_routes(lan_ifname);
		if (operation_status != 0)
			logmessage(LOGNAME, "LAN static route installation failed (status=%d)",
				   operation_status);
		start_status = rc_first_service_start_status(start_status, operation_status);

		/* Publish all runtime identity fields as one rollback-capable tuple. */
		metadata_status = update_lan_status(0);
		if (metadata_status != 0)
			logmessage(LOGNAME,
				   "LAN runtime metadata refresh failed (status=%d)",
				   metadata_status);
		start_status = rc_first_service_start_status(
			start_status, metadata_status);
	}

#if defined (USE_IPV6)
	ipv6_type_status = get_ipv6_type_status(&ipv6_type);
	if (ipv6_type_status != 0) {
		errno = ipv6_type_status;
		logmessage(LOGNAME,
			   "cannot reload LAN IPv6 address without verified service state (status=%d)",
			   ipv6_type_status);
		operation_status = ipv6_type_status;
	} else if (ipv6_type != IPV6_DISABLED) {
		operation_status = reload_lan_addr6();
		if (operation_status != 0)
			logmessage(LOGNAME, "LAN IPv6 address reload failed (status=%d)",
				   operation_status);
	} else {
		operation_status = 0;
	}
	start_status = rc_first_service_start_status(start_status, operation_status);
#endif

	operation_status = config_bridge(is_ap_mode);
	start_status = rc_first_service_start_status(start_status, operation_status);
	return start_status;
}

static int
lan_ap_teardown_abort_restore(int original_status, const char *reason)
{
	int restore_status, resume_status = 0;

	/*
	 * stop_lan() has already released the AP DHCP client. Re-run the normal
	 * AP bring-up path so a failed teardown cannot leave the still-configured
	 * LAN bridge without its DHCP client or monitor.
	 */
	logmessage(LOGNAME, "restoring AP LAN after aborted teardown (%s, status=%d)",
		   reason, original_status);
	restore_status = start_lan(1, 1);
	if (restore_status != 0) {
		logmessage(LOGNAME, "AP LAN rollback incomplete after %s (status=%d)",
			   reason, restore_status);
		/* start_lan() may fail before reaching its monitor notification. */
		resume_status = notify_run_detect_internet(2);
		if (resume_status != 0)
			logmessage(LOGNAME, "AP Internet monitor resume after rollback failed (status=%d)",
				   resume_status);
	}

	return rc_first_service_start_status(
		rc_first_service_start_status(original_status, restore_status),
		resume_status);
}

int
stop_lan(int is_ap_mode)
{
	int conntrack_status = 0, route_status = 0, ipv6_status = 0;
	int ap_dhcp_stop_status = -1;
	int gateway_status;
	char ap_gateway[16] = "";
	if (is_ap_mode) {
		char *svcs[] = { "detect_wan", NULL };
		const char *pidfiles[] = { "/var/run/udhcpc_lan.pid", NULL };
		int status, pause_status, monitor_resume_status;

		/* Save the installed AP default-route identity before the DHCP
		 * deconfig callback can update its temporary NVRAM metadata. */
		gateway_status = rc_wan_nvram_copy_value(
			lan_nvram_get_checked, NULL, "lan_gateway_t",
			ap_gateway, sizeof(ap_gateway));
		if (gateway_status != 0) {
			errno = gateway_status;
			logmessage(LOGNAME,
				   "cannot read AP gateway before LAN teardown (status=%d)",
				   gateway_status);
			return gateway_status;
		}

		pause_status = notify_pause_detect_internet();
		if (pause_status != 0) {
			logmessage(LOGNAME,
				   "refusing LAN teardown because Internet monitor pause failed (status=%d)",
				   pause_status);
			return pause_status;
		}
		ap_dhcp_stop_status = stop_udhcpc_lan();
		status = ap_dhcp_stop_status;
		if (status == 0)
			status = kill_services(svcs, 3, 1);
		if (status != 0) {
			if (rc_lan_ap_teardown_recovery_required(
				    ap_dhcp_stop_status, status)) {
				status = lan_ap_teardown_abort_restore(status,
							       "WAN detector stop");
			} else {
				/* DHCP stop failed or the client identity is unknown.
				 * Do not launch a second client; just resume detection. */
				monitor_resume_status = notify_run_detect_internet(2);
				if (monitor_resume_status != 0)
					logmessage(LOGNAME,
						   "Internet monitor resume failed after aborted AP DHCP stop (status=%d)",
						   monitor_resume_status);
				status = rc_first_service_start_status(status,
								       monitor_resume_status);
			}
			logmessage(LOGNAME,
				   "refusing LAN teardown because AP DHCP/monitor preflight was incomplete (status=%d)",
				   status);
			return status;
		}

		/*
		 * AP manual mode installs a default route through lan_gateway.
		 * Remove the last active gateway before a restart can add a new one.
		 * DHCP deconfig may already have removed it, so ESRCH is harmless.
		 */
		if (is_valid_ipv4(ap_gateway)) {
			route_status = route_status_idempotent_delete(
				route_del(IFNAME_BR, 0, "0.0.0.0", ap_gateway, "0.0.0.0"));
			if (route_status != 0) {
				logmessage(LOGNAME,
					   "AP default gateway route cleanup failed (status=%d); restoring LAN",
					   route_status);
				status = lan_ap_teardown_abort_restore(
					route_status, "gateway route cleanup");
				return status;
			}
		}
	} else {
		char lan_ip[16];
		int lan_ip_status;

		/*
		 * A failed read must not turn conntrack cleanup into a false
		 * success. Retain the error but continue independent teardown.
		 */
		lan_ip_status = rc_wan_nvram_copy_value(
			lan_nvram_get_checked, NULL, "lan_ipaddr_t",
			lan_ip, sizeof(lan_ip));
		if (lan_ip_status != 0) {
			errno = lan_ip_status;
			conntrack_status = lan_ip_status;
			logmessage(LOGNAME,
				   "cannot read old LAN address for conntrack cleanup (status=%d)",
				   lan_ip_status);
		} else if (is_valid_ipv4(lan_ip)) {
			conntrack_status = flush_conntrack_table(lan_ip);
			if (conntrack_status != 0)
				logmessage(LOGNAME, "LAN conntrack cleanup failed (status=%d)",
					   conntrack_status);
		}

		/* Remove static routes and retain failure while continuing teardown. */
		route_status = clear_if_route4(IFNAME_BR);
		if (route_status != 0)
			logmessage(LOGNAME, "LAN route cleanup failed (status=%d)", route_status);
	}

#if defined (USE_IPV6)
	ipv6_status = clear_lan_addr6();
	if (ipv6_status != 0)
		logmessage(LOGNAME, "LAN IPv6 address cleanup failed (status=%d)", ipv6_status);
#endif

	/* Bring down LAN interface */
	{
		int bridge_status = ifconfig(IFNAME_BR, 0, NULL, NULL);
		int teardown_status = rc_first_service_start_status(
			rc_first_service_start_status(
				rc_first_service_start_status(conntrack_status, route_status),
				ipv6_status),
			bridge_status);

		if (is_ap_mode && rc_lan_ap_teardown_recovery_required(
				    ap_dhcp_stop_status,
				    rc_first_service_start_status(ipv6_status,
								  bridge_status)))
			teardown_status = lan_ap_teardown_abort_restore(
				teardown_status, "IPv6/bridge cleanup");
		return teardown_status;
	}
}

static void
lan_restart_record_status(int *first_status, const char *operation, int status)
{
	if (status != 0)
		logmessage(LOGNAME, "LAN restart %s failed (status=%d)", operation, status);
	*first_status = rc_first_service_start_status(*first_status, status);
}

int
full_restart_lan(void)
{
	int is_wan_err = 0, is_lan_stp = 0;
	int is_ap_mode, log_remote, lan_status;
	int ap_mode_status, wan_err_status, lan_stp_status;
	int api_result, saved_errno;
	int dnsmasq_lock, dnsmasq_lock_errno, release_status;
	int dnsmasq_status, dnsmasq_start_status, vpn_status;
	int lan_start_status, wait_status;
	int logger_stop_status = 0, lltd_stop_status;
	int infosvr_stop_status, networkmap_stop_status;
	int restart_status = 0;
	char *lan_stp_value = NULL;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "LAN restart aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!is_ap_mode) {
		wan_err_status = get_wan_unit_value_int_status(0, "err",
							      &is_wan_err);
		if (wan_err_status != 0) {
			logmessage(LOGNAME,
				   "LAN restart aborted because WAN error state cannot be read (status=%d)",
				   wan_err_status);
			return wan_err_status;
		}

		errno = 0;
		api_result = nvram_get_checked("lan_stp", &lan_stp_value);
		saved_errno = errno;
		lan_stp_status = rc_wan_nvram_read_int_status(
			api_result, saved_errno, lan_stp_value, 0, &is_lan_stp);
		if (lan_stp_status != 0) {
			logmessage(LOGNAME,
				   "LAN restart aborted because bridge STP state cannot be read (status=%d)",
				   lan_stp_status);
			return lan_stp_status;
		}
	}

	/* Stop VPN under its own lifecycle lock before taking the DNSMasq lock.
	 * WAN teardown holds VPN while refreshing DNS, so the global order is
	 * VPN -> DNSMasq; acquiring them in reverse order can deadlock both paths. */
	vpn_status = stop_vpn_server();
	if (vpn_status != 0) {
		logmessage(LOGNAME, "LAN restart aborted because VPN server teardown was incomplete (status=%d)",
			   vpn_status);
		return vpn_status;
	}

	/* Serialize LAN teardown/reconfiguration with independent DNSMasq RC applets. */
	dnsmasq_lock = dnsmasq_lifecycle_lock_acquire();
	if (dnsmasq_lock < 0) {
		dnsmasq_lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire DNSMasq lifecycle lock after VPN preflight (errno=%d)",
			   dnsmasq_lock_errno);
		/* VPN was stopped successfully; restore it because LAN work never began. */
		restart_status = dnsmasq_lock_errno;
		lan_restart_record_status(&restart_status, "restore VPN after DNSMasq lock failure",
					  restart_vpn_server());
		errno = dnsmasq_lock_errno;
		return restart_status;
	}

	/* Record every status-bearing stop; continue safe teardown and preserve
	 * the first error for PID 1's incomplete-notification accounting. */
	if (log_remote) {
		logger_stop_status = stop_logger();
		lan_restart_record_status(&restart_status, "stop logger", logger_stop_status);
	}
	lltd_stop_status = stop_lltd();
	lan_restart_record_status(&restart_status, "stop LLTD", lltd_stop_status);
	infosvr_stop_status = stop_infosvr();
	lan_restart_record_status(&restart_status, "stop infosvr", infosvr_stop_status);
	networkmap_stop_status = stop_networkmap();
	lan_restart_record_status(&restart_status, "stop networkmap", networkmap_stop_status);
	lan_restart_record_status(&restart_status, "stop miniupnpd", stop_upnp());
	/* Preserve DNSMasq status for its start gate and aggregate it separately. */
	dnsmasq_status = stop_dns_dhcpd_locked();
	lan_restart_record_status(&restart_status, "stop DNSMasq", dnsmasq_status);
	lan_status = stop_lan(is_ap_mode);
	lan_restart_record_status(&restart_status, "stop LAN services", lan_status);
	if (lan_status != 0) {
		logmessage(LOGNAME, "LAN restart aborted because previous LAN services did not stop (status=%d)",
			   lan_status);
		/*
		 * stop_lan() can report a route, conntrack, IPv6 or bridge error
		 * after it has already changed the router-mode interface state.
		 * Since this restart is aborting, bring the old LAN configuration
		 * back before restarting DNSMasq and returning to PID 1.
		 * AP-mode rollback is handled inside stop_lan() because it must
		 * preserve the DHCP client's stop/lease state.
		 */
		if (!is_ap_mode)
			lan_restart_record_status(&restart_status,
						  "restore LAN interface after aborted router-mode teardown",
						  start_lan(0, 1));
		if (dnsmasq_status == 0) {
			dnsmasq_start_status = start_dns_dhcpd_locked(is_ap_mode);
			lan_restart_record_status(&restart_status, "restore DNSMasq after aborted LAN restart",
						  dnsmasq_start_status);
		}
		release_status = dnsmasq_lifecycle_lock_release(dnsmasq_lock);
		if (release_status != 0) {
			dnsmasq_lock_errno = errno ? errno : EIO;
			logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock after aborted LAN restart (errno=%d)",
				   dnsmasq_lock_errno);
			release_status = dnsmasq_lock_errno;
		}
		lan_restart_record_status(&restart_status, "release DNSMasq lock after aborted LAN restart",
					  release_status);
		/* VPN stopped cleanly before this attempt; restore it after releasing
		 * the DNSMasq lock so firewall/DNS callbacks cannot invert lock order. */
		lan_restart_record_status(&restart_status, "restore VPN after aborted LAN restart",
					  restart_vpn_server());

		/* The early-abort path must restore every ancillary daemon already
		 * stopped above. Start helpers are idempotent and fail closed on an
		 * unknown process inventory; only confirmed stops are restarted. */
		if (log_remote && rc_service_restart_after_stop(logger_stop_status))
			lan_restart_record_status(&restart_status, "restore logger after aborted LAN restart",
						  start_logger(0));
		if (rc_service_restart_after_stop(lltd_stop_status))
			lan_restart_record_status(&restart_status, "restore LLTD after aborted LAN restart",
						  start_lltd());
		if (rc_service_restart_after_stop(infosvr_stop_status))
			lan_restart_record_status(&restart_status, "restore infosvr after aborted LAN restart",
						  start_infosvr());
		if (rc_service_restart_after_stop(networkmap_stop_status))
			lan_restart_record_status(&restart_status, "restore networkmap after aborted LAN restart",
						  start_networkmap(1));
		return restart_status;
	}

	reset_lan_vars();

	if (!is_ap_mode) {
		lan_restart_record_status(&restart_status, "disable LAN bridge STP",
					  br_set_stp(IFNAME_BR, 0));
		lan_restart_record_status(&restart_status, "set LAN bridge forward delay for restart",
					  br_set_fd(IFNAME_BR, 2));
	}

	/* down and up all LAN ports link; always attempt the recovery step. */
	lan_restart_record_status(&restart_status, "power down LAN ports",
				  phy_ports_lan_power(0));
	wait_status = service_wait_seconds(1);
	if (wait_status != 0) {
		wait_status = errno > 0 ? errno : EIO;
		errno = wait_status;
	}
	lan_restart_record_status(&restart_status, "wait for LAN PHY power cycle",
				  wait_status);
	/* Always restore PHY power, even when the settling wait reports an error. */
	lan_restart_record_status(&restart_status, "power up LAN ports",
				  phy_ports_lan_power(1));

	lan_start_status = start_lan(is_ap_mode, 1);
	lan_restart_record_status(&restart_status, "start LAN services", lan_start_status);

	/* start logger if remote */
	if (log_remote)
		lan_restart_record_status(&restart_status, "start logger", start_logger(0));

#if defined(APP_SMBD)
	/* update SMB fastpath owner address */
	config_smb_fastpath(1);
#endif

	/* Never start a second DNSMasq while the previous process survived stop. */
	if (dnsmasq_status == 0) {
		dnsmasq_start_status = start_dns_dhcpd_locked(is_ap_mode);
		dnsmasq_status = dnsmasq_start_status;
		lan_restart_record_status(&restart_status, "restart DNSMasq", dnsmasq_start_status);
	} else {
		logmessage(LOGNAME, "DNSMasq restart skipped because the previous process did not stop (status=%d)",
			   dnsmasq_status);
	}
	/* Release before later WAN/VPN work can request a DNS cache reload. */
	release_status = dnsmasq_lifecycle_lock_release(dnsmasq_lock);
	if (release_status != 0) {
		dnsmasq_lock_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release DNSMasq lifecycle lock after LAN restart (errno=%d)",
			   dnsmasq_lock_errno);
		if (dnsmasq_status == 0)
			dnsmasq_status = dnsmasq_lock_errno;
		release_status = dnsmasq_lock_errno;
	}
	lan_restart_record_status(&restart_status, "release DNSMasq lock after LAN restart",
				  release_status);
	if (!is_ap_mode) {
		if (is_lan_stp) {
			lan_restart_record_status(&restart_status, "restore LAN bridge STP",
						  br_set_stp(IFNAME_BR, 1));
			lan_restart_record_status(&restart_status, "restore LAN bridge forward delay",
						  br_set_fd(IFNAME_BR, 15));
		}
		
		if (is_wan_err) {
			lan_restart_record_status(&restart_status, "restart WAN after LAN restart",
						  full_restart_wan());
			lan_restart_record_status(&restart_status, "start VPN server after WAN restart",
						  start_vpn_server());
		} else {
			/* restart vpn server, firewall and miniupnpd */
			lan_restart_record_status(&restart_status, "restart VPN server",
						  restart_vpn_server());
		}
	}

	/* restart igmpproxy, udpxy, xupnpd */
	if (!is_wan_err)
		lan_restart_record_status(&restart_status, "restart IPTV services",
					  restart_iptv(is_ap_mode));

#if defined(APP_NFSD)
	/* reload NFS server exports */
	lan_restart_record_status(&restart_status, "reload NFS exports", reload_nfsd());
#endif

#if defined(APP_SMBD) || defined(APP_NMBD)
	lan_restart_record_status(&restart_status, "reload NetBIOS services", reload_nmbd());
#endif

	lan_restart_record_status(&restart_status, "start infosvr", start_infosvr());
	lan_restart_record_status(&restart_status, "start LLTD", start_lltd());

	/* start ARP network scanner */
	lan_restart_record_status(&restart_status, "start networkmap", start_networkmap(1));

	/* force httpd logout */
	lan_restart_record_status(&restart_status, "restart IAPP daemon",
				  system("/usr/bin/iappd.sh restart"));
	{
		char *httpd_services[] = { "httpd", NULL };
		lan_restart_record_status(&restart_status, "notify HTTPD logout",
					  signal_services_by_identity(httpd_services, SIGUSR1));
	}

	return restart_status;
}

int
lan_up_manual(char *lan_ifname, char *lan_dname)
{
	FILE *fp;
	int lock, saved_errno, notify_status, gateway_status;
	int dns_count = 0;
	int status = 0;
	int metadata_status;
	char gateway_ip[16], dns1_ip[16], dns2_ip[16];
	struct nvram_transaction resolver_transaction;
	struct rc_wan_nvram_snapshot_field resolver_fields[3];
	char resolv_temp[] = "/etc/resolv.conf.XXXXXX";

	/* This value is also written to /etc/hosts and is resolver syntax, not
	 * an opaque hostname. Reject directive/newline injection before any side
	 * effect or resolver publication. */
	if (!lan_ifname || !*lan_ifname || !lan_dname ||
	    (*lan_dname && !dnsmasq_valid_server_domain(lan_dname))) {
		errno = EINVAL;
		logmessage(LOGNAME, "refusing unsafe manual LAN resolver domain");
		return EINVAL;
	}

	/*
	 * Resolve all static resolver inputs from one checked NVRAM generation
	 * before adding routes or replacing resolv.conf. A missing key is empty;
	 * a backend/close/overflow error must preserve the old resolver.
	 */
	resolver_fields[0].param_name = "lan_gateway";
	resolver_fields[0].destination = gateway_ip;
	resolver_fields[0].destination_size = sizeof(gateway_ip);
	resolver_fields[1].param_name = "lan_dns1";
	resolver_fields[1].destination = dns1_ip;
	resolver_fields[1].destination_size = sizeof(dns1_ip);
	resolver_fields[2].param_name = "lan_dns2";
	resolver_fields[2].destination = dns2_ip;
	resolver_fields[2].destination_size = sizeof(dns2_ip);
	gateway_status = rc_wan_nvram_copy_snapshot(
		resolver_fields, 3, lan_dhcp_snapshot_begin,
		lan_dhcp_snapshot_get, lan_dhcp_snapshot_end,
		&resolver_transaction);
	if (gateway_status != 0) {
		errno = gateway_status;
		logmessage(LOGNAME,
			   "cannot snapshot manual LAN resolver inputs before configuration (status=%d)",
			   gateway_status);
		return gateway_status;
	}

	lock = file_lock("resolv");
	if (lock < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire resolver publication lock; refusing manual LAN update (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* Set default route to gateway if specified. */
	if (is_valid_ipv4(gateway_ip)) {
		int route_status = route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

		/* A prior attempt may have installed the route before resolver
		 * publication failed. Treat an identical existing route as ready. */
		route_status = route_status_idempotent_add(route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "manual LAN gateway route setup failed (status=%d)",
				   route_status);
		status = rc_first_service_start_status(status, route_status);
	}

	/* Keep the old resolver visible until the complete replacement is ready. */
	fp = dnsmasq_file_open(resolv_temp);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove manual LAN resolver stage '%s' after open failure (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		file_unlock(lock);
		logmessage(LOGNAME, "cannot stage manual LAN resolver configuration (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return rc_first_service_start_status(status, saved_errno);
	}

	if (*lan_dname)
		fprintf(fp, "domain %s\n", lan_dname);

	if (is_valid_ipv4(dns1_ip)) {
		fprintf(fp, "nameserver %s\n", dns1_ip);
		dns_count++;
	}

	if (is_valid_ipv4(dns2_ip)) {
		fprintf(fp, "nameserver %s\n", dns2_ip);
		dns_count++;
	}

	if (dhcp_lease_static_resolver_uses_gateway(
			1, dns_count, is_valid_ipv4(gateway_ip)))
		fprintf(fp, "nameserver %s\n", gateway_ip);

	if (dnsmasq_file_publish(fp, resolv_temp, DNS_RESOLV_CONF, 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove failed manual LAN resolver stage '%s' (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		file_unlock(lock);
		logmessage(LOGNAME, "cannot publish manual LAN resolver configuration (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return rc_first_service_start_status(status, saved_errno);
	}

	file_unlock(lock);

	/* Sync time; include watchdog notification failure in manual LAN status. */
	notify_status = notify_watchdog_time();
	if (notify_status != 0)
		logmessage(LOGNAME, "LAN watchdog time notification incomplete (status=%d)", notify_status);
	status = rc_first_service_start_status(status, notify_status);

	/* Publish all four runtime identity fields transactionally. */
	metadata_status = update_lan_status(0);
	if (metadata_status != 0)
		logmessage(LOGNAME,
			   "manual LAN runtime metadata publication failed (status=%d)",
			   metadata_status);
	status = rc_first_service_start_status(status, metadata_status);
	return status;
}

static int
lan_publish_resolver_auto(char *lan_gateway, char *lan_dname, char *lan_dns)
{
	FILE *fp;
	int lock, status = 0, dns_count = 0, dns_mode = 0;
	char dns_mode_text[8], dns1_ip[16], dns2_ip[16];
	char word[100], *next;
	struct nvram_transaction resolver_transaction;
	struct rc_wan_nvram_snapshot_field resolver_fields[3];
	char resolv_temp[] = "/etc/resolv.conf.XXXXXX";
	char resolv_backup[] = "/etc/resolv.conf.rollback.XXXXXX";
	int recovery_status = 0;

	/*
	 * Snapshot the DNS selector and static addresses as one NVRAM generation.
	 * Failed reads must not turn into the legacy "static DNS" default and
	 * replace the resolver with an incomplete configuration.
	 */
	resolver_fields[0].param_name = "lan_dns_x";
	resolver_fields[0].destination = dns_mode_text;
	resolver_fields[0].destination_size = sizeof(dns_mode_text);
	resolver_fields[1].param_name = "lan_dns1";
	resolver_fields[1].destination = dns1_ip;
	resolver_fields[1].destination_size = sizeof(dns1_ip);
	resolver_fields[2].param_name = "lan_dns2";
	resolver_fields[2].destination = dns2_ip;
	resolver_fields[2].destination_size = sizeof(dns2_ip);
	status = rc_wan_nvram_copy_snapshot(
		resolver_fields, 3, lan_dhcp_snapshot_begin,
		lan_dhcp_snapshot_get, lan_dhcp_snapshot_end,
		&resolver_transaction);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot snapshot LAN DHCP resolver settings (status=%d)",
			   status);
		return status;
	}
	status = rc_wan_nvram_parse_int_value(dns_mode_text, &dns_mode);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot parse LAN DHCP resolver mode (status=%d)",
			   status);
		return status;
	}

	if (!dhcp_lease_binary_selector_is_valid(dns_mode)) {
		errno = EINVAL;
		logmessage(LOGNAME,
			   "invalid LAN DHCP resolver mode (%d); refusing resolver publication",
			   dns_mode);
		return EINVAL;
	}

	errno = 0;
	lock = file_lock("resolv");
	if (lock < 0)
		return errno ? errno : EIO;

	fp = dnsmasq_file_open(resolv_temp);
	if (!fp) {
		status = errno ? errno : EIO;
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove LAN DHCP resolver stage '%s' after open failure (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		file_unlock(lock);
		errno = status;
		return status;
	}

	/* The previous resolver remains visible until the complete replacement
	 * has been flushed, closed and atomically renamed into place. */
	if (lan_dname && *lan_dname)
		fprintf(fp, "domain %s\n", lan_dname);

	if (dns_mode == 0) {
		if (is_valid_ipv4(dns1_ip)) {
			fprintf(fp, "nameserver %s\n", dns1_ip);
			dns_count++;
		}

		if (is_valid_ipv4(dns2_ip)) {
			fprintf(fp, "nameserver %s\n", dns2_ip);
			dns_count++;
		}
	} else {
		foreach(word, lan_dns ? lan_dns : "", next) {
			if (is_valid_ipv4(word)) {
				fprintf(fp, "nameserver %s\n", word);
				dns_count++;
			}
		}
	}

	if (!dns_count && is_valid_ipv4(lan_gateway))
		fprintf(fp, "nameserver %s\n", lan_gateway);

	if (dnsmasq_file_publish_with_rollback(
		    fp, resolv_temp, DNS_RESOLV_CONF, resolv_backup, 0644,
		    &recovery_status) != 0) {
		status = errno ? errno : EIO;
		if (recovery_status != 0)
			logmessage(LOGNAME,
				   "LAN DHCP resolver rollback/durability confirmation incomplete (status=%d; recovery link path='%s')",
				   recovery_status, resolv_backup);
	} else if (recovery_status != 0) {
		logmessage(LOGNAME,
			   "LAN DHCP resolver committed but rollback snapshot cleanup failed (status=%d); snapshot '%s' retained",
			   recovery_status, resolv_backup);
	}

	file_unlock(lock);
	if (status != 0)
		errno = status;
	return status;
}

struct lan_dhcp_bound_context {
	char *lan_ifname;
	char *lan_gateway;
	char *lan_domain;
	char *lan_dns;
};

static int
lan_dhcp_bound_delete_route(void *opaque, const char *gateway)
{
	struct lan_dhcp_bound_context *context =
		(struct lan_dhcp_bound_context *)opaque;

	return route_status_idempotent_delete(
		route_del(context->lan_ifname, 0, "0.0.0.0",
			  (char *)gateway, "0.0.0.0"));
}

static int
lan_dhcp_bound_clear_address(void *opaque)
{
	struct lan_dhcp_bound_context *context =
		(struct lan_dhcp_bound_context *)opaque;

	return ifconfig(context->lan_ifname, IFUP, "0.0.0.0", NULL);
}

static int
lan_dhcp_bound_set_address(void *opaque, const char *ipaddr,
			   const char *netmask)
{
	struct lan_dhcp_bound_context *context =
		(struct lan_dhcp_bound_context *)opaque;

	return ifconfig(context->lan_ifname, IFUP,
			(char *)ipaddr, (char *)netmask);
}

static int
lan_dhcp_bound_add_route(void *opaque, const char *gateway)
{
	struct lan_dhcp_bound_context *context =
		(struct lan_dhcp_bound_context *)opaque;

	return route_add(context->lan_ifname, 0, "0.0.0.0",
			 (char *)gateway, "0.0.0.0");
}

static int
lan_dhcp_bound_publish_resolver(void *opaque)
{
	struct lan_dhcp_bound_context *context =
		(struct lan_dhcp_bound_context *)opaque;

	return lan_publish_resolver_auto(context->lan_gateway,
					 context->lan_domain,
					 context->lan_dns);
}

static const struct dhcp_lease_bound_ops lan_dhcp_bound_ops = {
	lan_dhcp_bound_delete_route,
	lan_dhcp_bound_clear_address,
	lan_dhcp_bound_set_address,
	lan_dhcp_bound_add_route,
	lan_dhcp_bound_publish_resolver
};


static int
lan_dhcp_snapshot_begin(void *opaque)
{
	if (!opaque) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_begin((struct nvram_transaction *)opaque);
}


static int
lan_dhcp_snapshot_get(void *opaque, const char *name, char **value)
{
	if (!opaque) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_get((struct nvram_transaction *)opaque, name, value);
}


static int
lan_dhcp_snapshot_end(void *opaque)
{
	if (!opaque) {
		errno = EINVAL;
		return -1;
	}
	return nvram_transaction_end((struct nvram_transaction *)opaque);
}


struct lan_dhcp_deconfig_state {
	char lease_gateway[16];
	char static_address[16];
	char static_netmask[16];
	char static_gateway[16];
	char static_domain[256];
	char static_dns_mode[8];
	char static_dns1[16];
	char static_dns2[16];
	char previous_address[16];
	char previous_netmask[16];
	char previous_domain[256];
	char previous_dns[256];
	char previous_wins[256];
	char previous_lease[16];
};

static int
lan_dhcp_nvram_state_publish(const struct dhcp_lease_nvram_value *values,
			     size_t count, int *rollback_status);


static int
lan_dhcp_deconfig_snapshot(struct lan_dhcp_deconfig_state *snapshot)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[14];

	if (!snapshot) {
		errno = EINVAL;
		return EINVAL;
	}

	/* Capture both the static configuration and the last committed lease
	 * tuple under one NVRAM transaction. The old gateway is also the route
	 * identity needed to remove the previous DHCP default route. */
	fields[0].param_name = "lan_gateway_t";
	fields[0].destination = snapshot->lease_gateway;
	fields[0].destination_size = sizeof(snapshot->lease_gateway);
	fields[1].param_name = "lan_ipaddr";
	fields[1].destination = snapshot->static_address;
	fields[1].destination_size = sizeof(snapshot->static_address);
	fields[2].param_name = "lan_netmask";
	fields[2].destination = snapshot->static_netmask;
	fields[2].destination_size = sizeof(snapshot->static_netmask);
	fields[3].param_name = "lan_gateway";
	fields[3].destination = snapshot->static_gateway;
	fields[3].destination_size = sizeof(snapshot->static_gateway);
	fields[4].param_name = "lan_domain";
	fields[4].destination = snapshot->static_domain;
	fields[4].destination_size = sizeof(snapshot->static_domain);
	fields[5].param_name = "lan_ipaddr_t";
	fields[5].destination = snapshot->previous_address;
	fields[5].destination_size = sizeof(snapshot->previous_address);
	fields[6].param_name = "lan_netmask_t";
	fields[6].destination = snapshot->previous_netmask;
	fields[6].destination_size = sizeof(snapshot->previous_netmask);
	fields[7].param_name = "lan_domain_t";
	fields[7].destination = snapshot->previous_domain;
	fields[7].destination_size = sizeof(snapshot->previous_domain);
	fields[8].param_name = "lan_dns_t";
	fields[8].destination = snapshot->previous_dns;
	fields[8].destination_size = sizeof(snapshot->previous_dns);
	fields[9].param_name = "lan_wins_t";
	fields[9].destination = snapshot->previous_wins;
	fields[9].destination_size = sizeof(snapshot->previous_wins);
	fields[10].param_name = "lan_lease_t";
	fields[10].destination = snapshot->previous_lease;
	fields[10].destination_size = sizeof(snapshot->previous_lease);
	fields[11].param_name = "lan_dns_x";
	fields[11].destination = snapshot->static_dns_mode;
	fields[11].destination_size = sizeof(snapshot->static_dns_mode);
	fields[12].param_name = "lan_dns1";
	fields[12].destination = snapshot->static_dns1;
	fields[12].destination_size = sizeof(snapshot->static_dns1);
	fields[13].param_name = "lan_dns2";
	fields[13].destination = snapshot->static_dns2;
	fields[13].destination_size = sizeof(snapshot->static_dns2);

	return rc_wan_nvram_copy_snapshot(
		fields, 14, lan_dhcp_snapshot_begin, lan_dhcp_snapshot_get,
		lan_dhcp_snapshot_end, &transaction);
}

static int
lan_down_auto(char *lan_ifname)
{
	FILE *fp;
	struct lan_dhcp_deconfig_state snapshot;
	struct dhcp_lease_nvram_value metadata[LAN_DHCP_DECONFIG_METADATA_COUNT];
	char resolv_temp[] = "/etc/resolv.conf.XXXXXX";
	int snapshot_status, route_status = 0, ifconfig_status, resolver_status = 0;
	int static_gateway_status = 0;
	int lock, status, hosts_status = 0;
	int metadata_status, metadata_rollback_status = 0;
	int static_dns_mode, use_static_dns;

	/*
	 * Restore one coherent view of the DHCP gateway and static LAN settings,
	 * while also retaining every old temporary metadata value for rollback.
	 * NVRAM reads use a rotating cache, so the snapshot helper copies every
	 * field before the next read and invalidates the tuple on any read/unlock
	 * error.
	 */
	snapshot_status = lan_dhcp_deconfig_snapshot(&snapshot);
	if (snapshot_status != 0) {
		logmessage("DHCP LAN Client",
			   "cannot snapshot LAN deconfig settings on %s (status=%d); retaining lease state",
			   lan_ifname, snapshot_status);
		return snapshot_status;
	}

	/* Use the same strict NVRAM integer parser as lease-bound resolver
	 * publication. atoi() would accept malformed text such as "0;reboot"
	 * as static-DNS mode and overwrite the resolver during deconfig. */
	status = rc_wan_nvram_parse_int_value(snapshot.static_dns_mode,
					      &static_dns_mode);
	if (status != 0) {
		errno = status;
		logmessage("DHCP LAN Client",
			   "cannot parse LAN DNS mode during deconfig on %s (status=%d); retaining lease state",
			   lan_ifname, status);
		return status;
	}
	if (!dhcp_lease_binary_selector_is_valid(static_dns_mode)) {
		errno = EINVAL;
		logmessage("DHCP LAN Client",
			   "invalid LAN DNS mode during deconfig on %s (value=%d); retaining lease state",
			   lan_ifname, static_dns_mode);
		return EINVAL;
	}
	use_static_dns = (static_dns_mode == 0);

	/*
	 * Remove the recorded DHCP default route before replacing the interface
	 * address. A failed delete must keep the old lease fields intact so a
	 * later callback still knows which route needs reconciliation.
	 */
	if (is_valid_ipv4(snapshot.lease_gateway)) {
		route_status = route_status_idempotent_delete(
			route_del(lan_ifname, 0, "0.0.0.0",
				  snapshot.lease_gateway, "0.0.0.0"));
		status = dhcp_lease_deconfig_status(route_status, 0, 0, 0);
		if (status != 0) {
			logmessage("DHCP LAN Client",
				   "failed to remove previous gateway route on %s during deconfig (status=%d); retaining lease state",
				   lan_ifname, status);
			return status;
		}
	}

	ifconfig_status = ifconfig(lan_ifname, IFUP,
				   snapshot.static_address, snapshot.static_netmask);
	status = dhcp_lease_deconfig_status(route_status, ifconfig_status, 0, 0);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "failed to restore static LAN address on %s during deconfig (status=%d); retaining lease state",
			   lan_ifname, status);
		return status;
	}

	/*
	 * Restore the configured static default route after the lease route
	 * is removed and the static interface address is back. EEXIST is
	 * accepted because DECONFIG_PENDING can retry after later cleanup fails.
	 */
	if (is_valid_ipv4(snapshot.static_gateway)) {
		static_gateway_status = route_status_idempotent_add(
			route_add(lan_ifname, 0, "0.0.0.0",
				  snapshot.static_gateway, "0.0.0.0"));
		if (static_gateway_status != 0)
			logmessage("DHCP LAN Client",
				   "failed to restore static LAN gateway route on %s during deconfig (status=%d)",
				   lan_ifname, static_gateway_status);
	}

	/* Stage the configured static resolver tuple and atomically publish it
	 * under the same lock used by other resolver writers. */
	errno = 0;
	lock = file_lock("resolv");
	if (lock < 0) {
		resolver_status = errno ? errno : EIO;
	} else {
		fp = dnsmasq_file_open(resolv_temp);
		if (!fp) {
			resolver_status = errno ? errno : EIO;
			if (dnsmasq_file_discard(resolv_temp) != 0)
				logmessage("DHCP LAN Client",
					   "cannot remove LAN resolver stage '%s' after open failure (errno=%d)",
					   resolv_temp, errno ? errno : EIO);
		} else {
			int dns_count = 0;

			if (*snapshot.static_domain) {
				if (!dnsmasq_valid_server_domain(snapshot.static_domain)) {
					resolver_status = EINVAL;
				} else {
					errno = 0;
					if (fprintf(fp, "domain %s\n", snapshot.static_domain) < 0)
						resolver_status = errno ? errno : EIO;
				}
			}

			if (resolver_status == 0 && use_static_dns &&
			    is_valid_ipv4(snapshot.static_dns1)) {
				errno = 0;
				if (fprintf(fp, "nameserver %s\n", snapshot.static_dns1) < 0)
					resolver_status = errno ? errno : EIO;
				else
					dns_count++;
			}

			if (resolver_status == 0 && use_static_dns &&
			    is_valid_ipv4(snapshot.static_dns2)) {
				errno = 0;
				if (fprintf(fp, "nameserver %s\n", snapshot.static_dns2) < 0)
					resolver_status = errno ? errno : EIO;
				else
					dns_count++;
			}

			if (resolver_status == 0 &&
			    dhcp_lease_static_resolver_uses_gateway(
				use_static_dns, dns_count,
				is_valid_ipv4(snapshot.static_gateway))) {
				errno = 0;
				if (fprintf(fp, "nameserver %s\n",
					    snapshot.static_gateway) < 0)
					resolver_status = errno ? errno : EIO;
			}

			if (resolver_status != 0) {
				if (fclose(fp) != 0)
					logmessage("DHCP LAN Client",
						   "cannot close failed LAN resolver stage (status=%d)",
						   resolver_status);
				if (dnsmasq_file_discard(resolv_temp) != 0)
					logmessage("DHCP LAN Client",
						   "cannot remove failed LAN resolver stage '%s' (errno=%d)",
						   resolv_temp, errno ? errno : EIO);
			} else if (dnsmasq_file_publish(fp, resolv_temp,
							DNS_RESOLV_CONF, 0644) != 0) {
				resolver_status = errno ? errno : EIO;
				if (dnsmasq_file_discard(resolv_temp) != 0)
					logmessage("DHCP LAN Client",
						   "cannot remove failed LAN resolver stage '%s' after publish failure (errno=%d)",
						   resolv_temp, errno ? errno : EIO);
			}
		}
		file_unlock(lock);
	}
	status = dhcp_lease_deconfig_status(route_status, ifconfig_status,
					    static_gateway_status, resolver_status);

	/*
	 * A DHCP address may have been published into /etc/hostname and /etc/hosts.
	 * Once the old route is gone and the static interface address is restored,
	 * restore those files even if resolver cleanup needs another retry.
	 */
	if (dhcp_lease_deconfig_address_ready(route_status, ifconfig_status)) {
		hosts_status = create_hosts_lan(snapshot.static_address,
						snapshot.static_domain);
		if (hosts_status != 0) {
			logmessage("DHCP LAN Client",
				   "failed to restore static LAN hostname/hosts during deconfig (status=%d)",
				   hosts_status);
			if (status == 0)
				status = hosts_status;
		}
	}

	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "LAN DHCP deconfig incomplete (status=%d); retaining lease identity for retry",
			   status);
		return status;
	}

	/*
	 * Commit the static LAN identity and clear DHCP-only metadata as one
	 * compensatable seven-key transaction. If any setter or transaction close
	 * fails, restore the prior tuple and return an error so BusyBox retries
	 * DECONFIG_PENDING instead of accepting a partially cleared lease.
	 */
	lan_dhcp_deconfig_metadata_prepare(
		metadata, snapshot.static_address, snapshot.static_netmask,
		snapshot.static_gateway, snapshot.static_domain,
		snapshot.previous_address, snapshot.previous_netmask,
		snapshot.lease_gateway, snapshot.previous_domain,
		snapshot.previous_dns, snapshot.previous_wins,
		snapshot.previous_lease);
	metadata_status = lan_dhcp_nvram_state_publish(
		metadata, LAN_DHCP_DECONFIG_METADATA_COUNT,
		&metadata_rollback_status);
	if (metadata_status != 0) {
		logmessage("DHCP LAN Client",
			   "failed to publish static LAN DHCP metadata on %s (status=%d); retaining callback for retry",
			   lan_ifname, metadata_status);
		if (metadata_rollback_status != 0)
			logmessage("DHCP LAN Client",
				   "LAN DHCP deconfig metadata rollback incomplete on %s (status=%d)",
				   lan_ifname, metadata_rollback_status);
		return metadata_status;
	}

	/* Once the lease cleanup is committed, monitor pause is advisory. */
	(void)notify_pause_detect_internet();

#if defined(APP_SMBD)
	/* update SMB fastpath owner address */
	config_smb_fastpath(1);
#endif

	return 0;
}

struct lan_dhcp_runtime_snapshot {
	char static_address[16];
	char static_netmask[16];
	char static_domain[256];
	char dhcp_gateway[16];
	char ap_gateway[16];
	char previous_address[16];
	char previous_netmask[16];
	char previous_domain[256];
	char previous_gateway[16];
};

static int
lan_dhcp_runtime_snapshot_copy(struct lan_dhcp_runtime_snapshot *snapshot)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[9];

	if (!snapshot) {
		errno = EINVAL;
		return EINVAL;
	}

	fields[0].param_name = "lan_ipaddr";
	fields[0].destination = snapshot->static_address;
	fields[0].destination_size = sizeof(snapshot->static_address);
	fields[1].param_name = "lan_netmask";
	fields[1].destination = snapshot->static_netmask;
	fields[1].destination_size = sizeof(snapshot->static_netmask);
	fields[2].param_name = "lan_domain";
	fields[2].destination = snapshot->static_domain;
	fields[2].destination_size = sizeof(snapshot->static_domain);
	fields[3].param_name = "dhcp_gateway_x";
	fields[3].destination = snapshot->dhcp_gateway;
	fields[3].destination_size = sizeof(snapshot->dhcp_gateway);
	fields[4].param_name = "lan_gateway";
	fields[4].destination = snapshot->ap_gateway;
	fields[4].destination_size = sizeof(snapshot->ap_gateway);
	fields[5].param_name = "lan_ipaddr_t";
	fields[5].destination = snapshot->previous_address;
	fields[5].destination_size = sizeof(snapshot->previous_address);
	fields[6].param_name = "lan_netmask_t";
	fields[6].destination = snapshot->previous_netmask;
	fields[6].destination_size = sizeof(snapshot->previous_netmask);
	fields[7].param_name = "lan_domain_t";
	fields[7].destination = snapshot->previous_domain;
	fields[7].destination_size = sizeof(snapshot->previous_domain);
	fields[8].param_name = "lan_gateway_t";
	fields[8].destination = snapshot->previous_gateway;
	fields[8].destination_size = sizeof(snapshot->previous_gateway);

	return rc_wan_nvram_copy_snapshot(
		fields, 9, lan_dhcp_snapshot_begin, lan_dhcp_snapshot_get,
		lan_dhcp_snapshot_end, &transaction);
}


int
update_lan_status(int is_auto)
{
	struct lan_dhcp_runtime_snapshot snapshot;
	struct dhcp_lease_nvram_value values[LAN_DHCP_STATUS_METADATA_COUNT];
	const char *gateway;
	int is_ap_mode, dhcpd_enabled, ap_mode_status;
	int status, rollback_status = 0;

	if (is_auto)
		return 0;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage(LOGNAME,
			   "LAN status update aborted because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	dhcpd_enabled = (!is_ap_mode && is_dhcpd_enabled(0));

	status = lan_dhcp_runtime_snapshot_copy(&snapshot);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot snapshot LAN runtime metadata (status=%d)",
			   status);
		return status;
	}

	if (is_ap_mode)
		gateway = snapshot.ap_gateway;
	else if (dhcpd_enabled && snapshot.dhcp_gateway[0])
		gateway = snapshot.dhcp_gateway;
	else
		gateway = snapshot.static_address;

	lan_dhcp_status_metadata_prepare(
		values, snapshot.static_address, snapshot.static_netmask,
		snapshot.static_domain, gateway, snapshot.previous_address,
		snapshot.previous_netmask, snapshot.previous_domain,
		snapshot.previous_gateway);
	status = lan_dhcp_nvram_state_publish(
		values, LAN_DHCP_STATUS_METADATA_COUNT, &rollback_status);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot atomically publish LAN runtime metadata (status=%d)",
			   status);
		if (rollback_status != 0)
			logmessage(LOGNAME,
				   "LAN runtime metadata rollback incomplete (status=%d)",
				   rollback_status);
	}

	return status;
}

static int
udhcpc_lan_deconfig(char *lan_ifname)
{
	int status;

	status = lan_down_auto(lan_ifname);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "%s: deconfig incomplete on %s (status=%d); preserving previous lease identity",
			   udhcpc_lan_state, lan_ifname, status);
		return status;
	}

	logmessage("DHCP LAN Client", "%s: lease is lost", udhcpc_lan_state);
	return 0;
}


static int
lan_dhcp_publish_hosts_and_schedule_retry(const char *lease_ip,
					  const char *lease_domain)
{
	int hosts_status, saved_errno, retry_errno;
	int notification_status, is_ap_mode, ap_mode_status;

	errno = 0;
	hosts_status = create_hosts_lan((char *)lease_ip, (char *)lease_domain);
	if (hosts_status == 0)
		return 0;

	saved_errno = errno ? errno : EIO;
	logmessage("DHCP LAN Client",
		   "LAN lease applied but hostname files could not be published (status=%d)",
		   hosts_status);
	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		logmessage("DHCP LAN Client",
			   "hostname-file retry suppressed because AP/router mode cannot be verified (status=%d)",
			   ap_mode_status);
		errno = saved_errno;
		return hosts_status;
	}
	if (is_ap_mode) {
		/* Queue an immediate retry; a later AP DHCP RENEW is the durable
		 * fallback if the notification queue is unavailable. */
		notification_status = notify_rc_status(RCN_RESTART_DHCPD);
		if (notification_status != 0) {
			retry_errno = errno ? errno : EIO;
			logmessage("DHCP LAN Client",
				   "cannot queue AP hostname-file recovery (errno=%d); retrying",
				   retry_errno);
			errno = 0;
			notification_status = notify_rc_status(RCN_RESTART_DHCPD);
			if (notification_status != 0) {
				retry_errno = errno ? errno : EIO;
				logmessage("DHCP LAN Client",
					   "AP hostname-file recovery remains unqueued (errno=%d); next AP DHCP renewal will retry hostname-file publication",
					   retry_errno);
			}
		}
	}

	errno = saved_errno;
	return hosts_status;
}


static int
lan_dhcp_nvram_transaction_set(void *opaque, const char *name,
			       const char *value)
{
	struct nvram_transaction *transaction =
		(struct nvram_transaction *)opaque;
	int status;

	errno = 0;
	status = nvram_transaction_set(transaction, name, value, 1);
	if (status < 0)
		return errno ? errno : EIO;
	return status;
}

static int
lan_dhcp_nvram_restore_values(
	const struct dhcp_lease_nvram_value *values, size_t count,
	int *rollback_status)
{
	struct nvram_transaction transaction;
	int status, end_status;

	if (rollback_status)
		*rollback_status = 0;
	if (!values || count == 0 || count > 7) {
		if (rollback_status)
			*rollback_status = EINVAL;
		return EINVAL;
	}

	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0) {
		status = errno ? errno : EIO;
		if (rollback_status)
			*rollback_status = status;
		return status;
	}

	status = dhcp_lease_nvram_restore(values, count,
					  lan_dhcp_nvram_transaction_set,
					  &transaction, rollback_status);
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		if (rollback_status && *rollback_status == 0)
			*rollback_status = end_status;
	}
	return status;
}

static int
lan_dhcp_nvram_state_publish(const struct dhcp_lease_nvram_value *values,
			     size_t count, int *rollback_status)
{
	struct nvram_transaction transaction;
	int status, end_status, recovery_status, recovery_detail = 0;

	if (rollback_status)
		*rollback_status = 0;
	errno = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return errno ? errno : EIO;
	status = dhcp_lease_nvram_publish(values, count,
					  lan_dhcp_nvram_transaction_set,
					  &transaction, rollback_status);
	if (nvram_transaction_end(&transaction) != 0) {
		end_status = errno ? errno : EIO;
		if (status == 0)
			status = end_status;
		else if (rollback_status && *rollback_status == 0)
			*rollback_status = end_status;
	}

	/*
	 * The in-lock rollback above is the fast path. Re-apply every old tuple
	 * under a fresh lock after any error: a failed ioctl/close can report
	 * failure after the tuple was actually changed, and a failed rollback
	 * must not prevent later keys from being restored.
	 */
	if (status != 0) {
		recovery_status = lan_dhcp_nvram_restore_values(
			values, count, &recovery_detail);
		if (rollback_status)
			*rollback_status = recovery_status != 0 ?
				(recovery_detail != 0 ? recovery_detail :
				 recovery_status) : 0;
	}

	return status;
}

struct lan_dhcp_bound_nvram_state {
	char previous_address[16];
	char previous_netmask[16];
	char previous_gateway[16];
	char previous_dns[256];
	char previous_domain[256];
	char previous_wins[256];
	char previous_lease[16];
	char configured_netmask[16];
};

static int
lan_dhcp_bound_snapshot(struct lan_dhcp_bound_nvram_state *snapshot)
{
	struct nvram_transaction transaction;
	struct rc_wan_nvram_snapshot_field fields[8];

	if (!snapshot) {
		errno = EINVAL;
		return EINVAL;
	}

	/* Keep the rollback tuple and the static-netmask fallback from one
	 * generation. A failed read or transaction close invalidates all fields. */
	fields[0].param_name = "lan_ipaddr_t";
	fields[0].destination = snapshot->previous_address;
	fields[0].destination_size = sizeof(snapshot->previous_address);
	fields[1].param_name = "lan_netmask_t";
	fields[1].destination = snapshot->previous_netmask;
	fields[1].destination_size = sizeof(snapshot->previous_netmask);
	fields[2].param_name = "lan_gateway_t";
	fields[2].destination = snapshot->previous_gateway;
	fields[2].destination_size = sizeof(snapshot->previous_gateway);
	fields[3].param_name = "lan_dns_t";
	fields[3].destination = snapshot->previous_dns;
	fields[3].destination_size = sizeof(snapshot->previous_dns);
	fields[4].param_name = "lan_domain_t";
	fields[4].destination = snapshot->previous_domain;
	fields[4].destination_size = sizeof(snapshot->previous_domain);
	fields[5].param_name = "lan_wins_t";
	fields[5].destination = snapshot->previous_wins;
	fields[5].destination_size = sizeof(snapshot->previous_wins);
	fields[6].param_name = "lan_lease_t";
	fields[6].destination = snapshot->previous_lease;
	fields[6].destination_size = sizeof(snapshot->previous_lease);
	fields[7].param_name = "lan_netmask";
	fields[7].destination = snapshot->configured_netmask;
	fields[7].destination_size = sizeof(snapshot->configured_netmask);

	return rc_wan_nvram_copy_snapshot(
		fields, 8, lan_dhcp_snapshot_begin, lan_dhcp_snapshot_get,
		lan_dhcp_snapshot_end, &transaction);
}


static int
lan_dhcp_networkmap_retry_pending(void)
{
	int dir_fd, marker_status, saved_errno;

	dir_fd = rc_notify_queue_open_dir(DIR_RC_INCOMPLETE, 0);
	if (dir_fd < 0)
		return -(errno ? errno : EIO);

	errno = 0;
	marker_status = rc_notify_queue_marker_is_safe_at(
		dir_fd, RCN_RESTART_NETWORKMAP, 0);
	saved_errno = errno;
	if (close(dir_fd) != 0 && marker_status == 0)
		return -(errno ? errno : EIO);

	if (marker_status == 0)
		return 1;
	if (saved_errno == ENOENT)
		return 0;
	return -(saved_errno ? saved_errno : EIO);
}


static int
udhcpc_lan_bound(char *lan_ifname, int is_renew)
{
	char *value, *lease_ip, *lease_netmask, *lease_domain;
	char *lease_dns, *lease_wins;
	char gateway_buf[INET_ADDRSTRLEN], lease_text[16];
	char old_ipaddr[INET_ADDRSTRLEN], old_netmask[INET_ADDRSTRLEN];
	char old_gateway[INET_ADDRSTRLEN], old_dns[256], old_domain[256];
	char old_wins[256], old_lease[16];
	unsigned int parsed_lease;
	int is_changed, ip_changed, netmask_changed, gateway_changed;
	int address_changed, apply_address, clear_address, apply_gateway;
	int lease_dur = 0;
	int status, rollback_status = 0, hosts_status = 0;
	int watchdog_status, monitor_status, has_gateway;
	int nvram_rollback_status = 0, network_rollback_status = 0;
	int network_rollback_cleanup_status = 0, network_transaction_applied = 0;
	int refresh_hosts, networkmap_retry_pending;
	int networkmap_status, networkmap_retry_status;
	int is_ap_mode = 1, refresh_as_ap = 1;
	int ap_mode_status, refresh_mode_status;
	struct dhcp_lease_nvram_value lease_state[7];
	struct lan_dhcp_bound_context context;
	struct lan_dhcp_bound_nvram_state nvram_state;

	if (!lan_ifname || !*lan_ifname)
		return EINVAL;

	status = lan_dhcp_bound_snapshot(&nvram_state);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "cannot snapshot last-known-good LAN DHCP state on %s (status=%d); rejecting bound/renew",
			   lan_ifname, status);
		return status;
	}

	/* Do not publish any part of a lease until its required address data is
	 * valid. The AP-mode LAN address is supplied by an untrusted DHCP server. */
	value = getenv("ip");
	lease_ip = value ? trim_r(value) : "";
	value = getenv("subnet");
	if (value) {
		lease_netmask = trim_r(value);
	} else {
		lease_netmask = nvram_state.previous_netmask;
		if (!dhcp_lease_netmask_is_valid(lease_netmask))
			lease_netmask = nvram_state.configured_netmask;
	}

	if (!dhcp_lease_ipv4_is_valid(lease_ip) ||
	    !dhcp_lease_netmask_is_valid(lease_netmask)) {
		logmessage("DHCP LAN Client",
			"bound/renew (%s) rejected invalid DHCP address/netmask",
			lan_ifname);
		return EINVAL;
	}

	/* Normalize optional options on every event. An option withdrawn by the
	 * server must not leave the previous lease's values live in NVRAM. */
	value = getenv("router");
	status = dhcp_lease_router_option_parse(
		value ? trim_r(value) : NULL, gateway_buf, sizeof(gateway_buf),
		&has_gateway);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			"rejecting bound/renew with malformed DHCP router option on %s (status=%d)",
			lan_ifname, status);
		return status;
	}
	if (!has_gateway)
		gateway_buf[0] = '\0';

	value = getenv("dns");
	lease_dns = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_dns, 255, 16, 1)) {
		logmessage("DHCP LAN Client",
			"ignoring invalid DHCP DNS list on %s", lan_ifname);
		lease_dns = "";
	}

	value = getenv("wins");
	lease_wins = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_wins, 255, 16, 1)) {
		logmessage("DHCP LAN Client",
			"ignoring invalid DHCP WINS list on %s", lan_ifname);
		lease_wins = "";
	}

	value = getenv("domain");
	lease_domain = value ? trim_r(value) : "";
	if (*lease_domain && !dnsmasq_valid_server_domain(lease_domain)) {
		logmessage("DHCP LAN Client",
			"ignoring invalid DHCP domain on %s", lan_ifname);
		lease_domain = "";
	}

	value = getenv("lease");
	status = lan_dhcp_effective_lease_seconds(
		value ? trim_r(value) : NULL, &parsed_lease);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "rejecting invalid DHCP lease duration on %s (status=%d)",
			   lan_ifname, status);
		return status;
	}
	lease_dur = (int)parsed_lease;
	snprintf(lease_text, sizeof(lease_text), "%d", lease_dur);

	/* Snapshot the last-known-good lease before attempting any kernel or file
	 * changes. A failed bound callback must leave these values available for
	 * the next DHCP event, since BusyBox discards this callback's return code. */
	snprintf(old_ipaddr, sizeof(old_ipaddr), "%s",
		nvram_state.previous_address);
	snprintf(old_netmask, sizeof(old_netmask), "%s",
		nvram_state.previous_netmask);
	snprintf(old_gateway, sizeof(old_gateway), "%s",
		nvram_state.previous_gateway);
	snprintf(old_dns, sizeof(old_dns), "%s",
		nvram_state.previous_dns);
	snprintf(old_domain, sizeof(old_domain), "%s",
		nvram_state.previous_domain);
	snprintf(old_wins, sizeof(old_wins), "%s",
		nvram_state.previous_wins);
	snprintf(old_lease, sizeof(old_lease), "%s",
		nvram_state.previous_lease);

	ip_changed = strcmp(old_ipaddr, lease_ip) != 0;
	netmask_changed = strcmp(old_netmask, lease_netmask) != 0;
	gateway_changed = strcmp(old_gateway, gateway_buf) != 0;
	address_changed = ip_changed || netmask_changed;
	is_changed = address_changed || gateway_changed ||
		strcmp(old_dns, lease_dns) != 0 ||
		strcmp(old_domain, lease_domain) != 0;
	apply_address = address_changed || !is_renew;
	clear_address = ip_changed;
	apply_gateway = gateway_changed || !is_renew;
	lease_state[0].name = "lan_ipaddr_t";
	lease_state[0].value = lease_ip;
	lease_state[0].previous_value = old_ipaddr;
	lease_state[1].name = "lan_netmask_t";
	lease_state[1].value = lease_netmask;
	lease_state[1].previous_value = old_netmask;
	lease_state[2].name = "lan_gateway_t";
	lease_state[2].value = gateway_buf;
	lease_state[2].previous_value = old_gateway;
	lease_state[3].name = "lan_dns_t";
	lease_state[3].value = lease_dns;
	lease_state[3].previous_value = old_dns;
	lease_state[4].name = "lan_domain_t";
	lease_state[4].value = lease_domain;
	lease_state[4].previous_value = old_domain;
	lease_state[5].name = "lan_wins_t";
	lease_state[5].value = lease_wins;
	lease_state[5].previous_value = old_wins;
	lease_state[6].name = "lan_lease_t";
	lease_state[6].value = lease_text;
	lease_state[6].previous_value = old_lease;
	/*
	 * A failed AP/router mode read must not suppress the periodic AP hosts
	 * refresh fallback on an otherwise unchanged RENEW. Treat unknown mode
	 * conservatively as AP; host publication is ancillary to the committed
	 * lease and its failure is handled by the existing retry path.
	 */
	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	refresh_mode_status = lan_dhcp_hosts_refresh_ap_mode_resolve(
		ap_mode_status, is_ap_mode, &refresh_as_ap);
	if (refresh_mode_status != 0)
		logmessage("DHCP LAN Client",
			   "cannot verify AP/router mode before LAN DHCP host refresh; treating as AP (status=%d)",
			   refresh_mode_status);
	refresh_hosts = lan_dhcp_hosts_refresh_required(
		is_changed, is_renew, refresh_as_ap);

	context.lan_ifname = lan_ifname;
	context.lan_gateway = gateway_buf;
	context.lan_domain = lease_domain;
	context.lan_dns = lease_dns;

	if (is_changed || !is_renew) {
		status = dhcp_lease_bound_apply(
			old_ipaddr, old_netmask, old_gateway,
			lease_ip, lease_netmask, gateway_buf,
			apply_address, clear_address, apply_gateway,
			&lan_dhcp_bound_ops, &context, &rollback_status);
		if (status != 0) {
			logmessage("DHCP LAN Client",
				"bound/renew transaction failed on %s (status=%d); keeping previous lease",
				lan_ifname, status);
			if (rollback_status != 0)
				logmessage("DHCP LAN Client",
					"failed to roll back LAN DHCP bound transaction on %s (status=%d)",
					lan_ifname, rollback_status);
			return status;
		}
		network_transaction_applied = 1;
	}

	/* Publish the seven volatile lease tuples as one serialized operation.
	 * A failed setter restores the failing key and earlier keys under the
	 * NVRAM transaction lock. If publication fails, reverse the kernel and
	 * resolver transition so BusyBox cannot accept a split lease. */
	status = lan_dhcp_nvram_state_publish(
		lease_state, sizeof(lease_state) / sizeof(lease_state[0]),
		&nvram_rollback_status);
	if (status != 0) {
		logmessage("DHCP LAN Client",
			   "cannot publish LAN DHCP lease state on %s (status=%d)",
			   lan_ifname, status);
		if (nvram_rollback_status != 0)
			logmessage("DHCP LAN Client",
				   "LAN DHCP NVRAM restoration incomplete on %s (status=%d)",
				   lan_ifname, nvram_rollback_status);
		if (network_transaction_applied) {
			context.lan_gateway = old_gateway;
			context.lan_domain = old_domain;
			context.lan_dns = old_dns;
			network_rollback_status = dhcp_lease_bound_apply(
				lease_ip, lease_netmask, gateway_buf,
				old_ipaddr, old_netmask, old_gateway,
				apply_address, clear_address, apply_gateway,
				&lan_dhcp_bound_ops, &context,
				&network_rollback_cleanup_status);
			if (network_rollback_status != 0)
				logmessage("DHCP LAN Client",
					   "LAN DHCP kernel/resolver rollback failed on %s (status=%d)",
					   lan_ifname, network_rollback_status);
			if (network_rollback_cleanup_status != 0)
				logmessage("DHCP LAN Client",
					   "LAN DHCP kernel/resolver rollback cleanup incomplete on %s (status=%d)",
					   lan_ifname, network_rollback_cleanup_status);
		}
		return status;
	}

	if (refresh_hosts)
		hosts_status = lan_dhcp_publish_hosts_and_schedule_retry(
			lease_ip, lease_domain);

	if (is_changed || !is_renew) {
		watchdog_status = notify_watchdog_time();
		if (watchdog_status != 0)
			logmessage("DHCP LAN Client",
				   "watchdog time notification after lease update failed on %s (status=%d)",
				   lan_ifname, watchdog_status);

#if defined(APP_SMBD)
		/* update SMB fastpath owner address */
		config_smb_fastpath(1);
#endif

		/* di wakeup after 2 secs */
		monitor_status = notify_run_detect_internet(2);
		if (monitor_status != 0)
			logmessage("DHCP LAN Client",
				   "Internet monitor notification after lease update failed on %s (status=%d)",
				   lan_ifname, monitor_status);
		{
			int iappd_status = system("/usr/bin/iappd.sh restart");

			if (iappd_status != 0)
				logmessage("DHCP LAN Client",
					   "IAPP daemon restart after lease update failed on %s (wait status=%d)",
					   lan_ifname, iappd_status);
		}
		logmessage("DHCP LAN Client",
			"%s, IP: %s/%s, GW: %s, lease time: %d",
			udhcpc_lan_state, lease_ip, lease_netmask, gateway_buf, lease_dur);
	}

	/*
	 * A failed networkmap refresh is retained in PID 1's incomplete-action
	 * directory. Requeue it on every later BOUND/RENEW callback, including
	 * same-address renewals, until the daemon confirms a full-scan request.
	 */
	networkmap_retry_pending = lan_dhcp_networkmap_retry_pending();
	if (networkmap_retry_pending < 0)
		logmessage("DHCP LAN Client",
			   "cannot inspect pending networkmap refresh (status=%d)",
			   -networkmap_retry_pending);
	networkmap_retry_pending =
		lan_dhcp_networkmap_retry_required(networkmap_retry_pending);

	if (lan_dhcp_networkmap_request_required(
		address_changed, is_renew, networkmap_retry_pending)) {
		if (networkmap_retry_pending) {
			networkmap_retry_status = notify_rc_status(RCN_RESTART_NETWORKMAP);
			if (networkmap_retry_status != 0)
				logmessage("DHCP LAN Client",
					   "cannot requeue pending networkmap refresh (errno=%d)",
					   errno ? errno : EIO);
		} else {
			networkmap_status = restart_networkmap();
			if (networkmap_status != 0) {
				logmessage("DHCP LAN Client",
					   "networkmap refresh after lease update failed on %s (status=%d)",
					   lan_ifname, networkmap_status);
				networkmap_retry_status = notify_rc_status(RCN_RESTART_NETWORKMAP);
				if (networkmap_retry_status != 0)
					logmessage("DHCP LAN Client",
						   "cannot queue deferred networkmap refresh (errno=%d)",
						   errno ? errno : EIO);
			}
		}
	}

	return lan_dhcp_bound_callback_status(0, hosts_status);
}

static int 
udhcpc_lan_leasefail(char *lan_ifname)
{
	logmessage("DHCP LAN Client",
		   "no DHCP lease available on %s; udhcpc will retry acquisition",
		   lan_ifname);
	return 0;
}

static int 
udhcpc_lan_noack(char *lan_ifname)
{
	logmessage("DHCP LAN Client", "Received NAK for %s", lan_ifname);
	return 0;
}

int
udhcpc_lan_main(int argc, char **argv)
{
	int ret = 0;
	char *lan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	lan_ifname = safe_getenv("interface");
	if (!lan_ifname || !*lan_ifname)
		return EINVAL;
	snprintf(udhcpc_lan_state, sizeof(udhcpc_lan_state), "%s", argv[1]);

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_lan_deconfig(lan_ifname);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_lan_bound(lan_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_lan_bound(lan_ifname, 1);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_lan_leasefail(lan_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_lan_noack(lan_ifname);

	return ret;
}

int
start_udhcpc_lan(char *lan_ifname)
{
	char *lan_hostname = get_our_hostname();
	int hostname_status = lan_udhcpc_hostname_status(
		lan_hostname, lan_hostname ? 0 : (errno > 0 ? errno : EIO),
		lan_hostname && is_valid_hostname(lan_hostname));
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", lan_ifname,
		"-s", SCRIPT_UDHCPC_LAN,
		"-p", "/var/run/udhcpc_lan.pid",
		"-t4",
		"-T4",
		"-d", /* Background after run (new patch for udhcpc) */
		"-H", lan_hostname,
		NULL
	};

	if (hostname_status != 0) {
		logmessage(LOGNAME,
			   "refusing to start LAN DHCP client without a verified hostname (status=%d)",
			   hostname_status);
		errno = hostname_status;
		return hostname_status;
	}

	return udhcpc_start_and_verify(dhcp_argv, "/var/run/udhcpc_lan.pid",
				       lan_ifname, "DHCP LAN Client");
}

int
stop_udhcpc_lan(void)
{
	const char *pidfiles[] = { "/var/run/udhcpc_lan.pid", NULL };
	int signal_status;
	int wait_status;

	/*
	 * The LAN client is not started with -R. SIGTERM alone exits a BOUND
	 * client without running deconfig, so request release first. BusyBox
	 * keeps failed deconfig callbacks pending and retries them on SIGTERM.
	 */
	signal_status = process_pidfile_signal(pidfiles[0], "udhcpc", SIGUSR2);
	if (signal_status != 0) {
		if (errno != ENOENT && errno != ESRCH) {
			logmessage(LOGNAME, "refusing LAN DHCP stop: PID identity is unknown (errno=%d)",
				   errno);
			return -1;
		}
	} else {
		wait_status = service_wait_microseconds(300000U);
		if (wait_status != 0) {
			wait_status = errno > 0 ? errno : EIO;
			logmessage(LOGNAME, "refusing LAN DHCP stop: deconfig grace wait failed (status=%d)",
				   wait_status);
			errno = wait_status;
			return wait_status;
		}
	}

	/* Never SIGKILL udhcpc while its lease cleanup may be retrying. */
	return kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0);
}
