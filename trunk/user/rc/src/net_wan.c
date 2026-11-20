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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <signal.h>

#include "rc.h"
#include "network_sysctl.h"
#include "network_sysctl_policy.h"
#include "service_start_status.h"
#include "wan_mtu_setup_policy.h"
#include "wan_route_flush_policy.h"
#include "apcli_wait_policy.h"
#include "service_wait_policy.h"
#include <process_inventory.h>
#include <process_pidfile.h>
#include "process_presence_start_policy.h"
#include "ppp_options_file.h"
#include "ppp_callback_link_policy.h"
#include "vpn_server_lifecycle_lock.h"
#include "route_status_policy.h"
#include "wan_nvram_status_policy.h"
#include "detect_internet_notification_policy.h"
#include "dnsmasq_file.h"
#include "dnsmasq_lifecycle_lock.h"
#include "dnsmasq_domain_policy.h"
#include "wan_resolver_refresh_policy.h"
#include "wan_teardown_policy.h"
#include "dhcp_lease_policy.h"
#include "dhcp_wan_interface_policy.h"
#include "wan_ifname_policy.h"
#include "dhcp_wan_route_transition_policy.h"
#include "wan_viptv_static_policy.h"
#include "bridge_membership_policy.h"
#include "switch.h"
#include "switch_status_policy.h"
#include <gpioutils.h>

char*
get_wan_unit_value(int unit, const char* param_name)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	return nvram_safe_get(wanN_param);
}

int
get_wan_unit_value_int(int unit, const char* param_name)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	return nvram_get_int(wanN_param);
}

static int
set_wan_unit_value_status(int unit, const char *param_name, const char *value)
{
	char wanN_param[64];
	int path_len, api_result, saved_errno;

	path_len = snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit,
			    param_name);
	if (path_len < 0)
		return errno ? errno : EIO;
	if ((size_t)path_len >= sizeof(wanN_param))
		return ENAMETOOLONG;

	errno = 0;
	api_result = nvram_set_temp(wanN_param, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
set_wan_unit_param_status(int unit, const char *param_name)
{
	char wan_param[64];
	int path_len;

	if (!param_name || !param_name[0])
		return EINVAL;

	path_len = snprintf(wan_param, sizeof(wan_param), "wan_%s", param_name);
	if (path_len < 0)
		return errno ? errno : EIO;
	if ((size_t)path_len >= sizeof(wan_param))
		return ENAMETOOLONG;

	return set_wan_unit_value_status(unit, param_name,
					 nvram_safe_get(wan_param));
}

static int
set_wan_unit_param_status_callback(void *context, const char *param_name)
{
	int *unit = (int *)context;

	if (!unit)
		return EINVAL;
	return set_wan_unit_param_status(*unit, param_name);
}

void
set_wan_unit_value(int unit, const char* param_name, const char* value)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	nvram_set_temp(wanN_param, value);
}

void
set_wan_unit_value_int(int unit, const char* param_name, int value)
{
	char wanN_param[64];

	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);
	nvram_set_int_temp(wanN_param, value);
}

void
set_wan_unit_param(int unit, const char* param_name)
{
	char wan_param[64], wanN_param[64];

	snprintf(wan_param, sizeof(wan_param), "wan_%s", param_name);
	snprintf(wanN_param, sizeof(wanN_param), "wan%d_%s", unit, param_name);

	nvram_set_temp(wanN_param, nvram_safe_get(wan_param));
}


static int
set_wan_unit_value_status_callback(void *context, const char *param_name,
				   const char *value)
{
	int *unit = (int *)context;

	if (!unit)
		return EINVAL;
	return set_wan_unit_value_status(*unit, param_name, value);
}

static void
control_wan_led_isp_state(int is_wan_up, int is_modem_unit)
{
	int front_led_wan = nvram_get_int("front_led_wan");

	if (front_led_wan == 2) {
		int has_link = 0;
		if (is_wan_up) {
			has_link = 1;
			if (!get_wan_wisp_active(&has_link) && !is_modem_unit)
				has_link = get_wan_ether_link_cached();
		}
		LED_CONTROL(LED_WAN, (is_wan_up && has_link) ? LED_ON : LED_OFF);
	} else if (front_led_wan == 3) {
		if (!is_wan_up)
			LED_CONTROL(LED_WAN, LED_OFF);
	}
}

static void
clear_wan_state(void)
{
	set_wan_unit_value_int(0, "err", 0);
	set_wan_unit_value_int(0, "uptime", 0);
	set_wan_unit_value_int(0, "dltime", 0);
	set_wan_unit_value_int(0, "bytes_rx", 0);
	set_wan_unit_value_int(0, "bytes_tx", 0);
	nvram_set_int_temp("l2tp_wan_t", 0);
	nvram_set_temp("vpnc_dns_t", "");
	nvram_set_temp("vpnc_dom_t", "");
}

static int
set_nvram_value_status(const char *name, const char *value)
{
	int api_result, saved_errno;

	if (!name || !name[0] || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_set_temp(name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
set_nvram_persistent_value_status(const char *name, const char *value)
{
	int api_result, saved_errno;

	if (!name || !name[0] || !value)
		return EINVAL;
	errno = 0;
	api_result = nvram_set(name, value);
	saved_errno = errno;
	return rc_wan_nvram_write_status(api_result, saved_errno);
}

static int
set_nvram_named_value_status(void *context, const char *name)
{
	const char *value = (const char *)context;

	if (!value)
		return EINVAL;
	return set_nvram_value_status(name, value);
}


static int
set_nvram_snapshot_value_status(void *context, const char *param_name,
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
reset_man_vars_status(void)
{
	static const char *const empty_params[] = {
		"wanx_ipaddr", "wanx_netmask", "wanx_gateway", "wanx_dns",
		"wanx_lease", "wanx_domain", "wanx_mtu", "wanx_routes",
		"wanx_routes_ms", "wanx_routes_rfc",
		"manv_ipaddr", "manv_netmask", "manv_gateway", "manv_mtu",
		"manv_routes", "manv_routes_ms", "manv_routes_rfc"
	};
	static const char *const zero_params[] = { "wanx_err", "manv_err" };
	int status;

	status = rc_wan_nvram_set_required_params(
		empty_params, (unsigned int)(sizeof(empty_params) / sizeof(empty_params[0])),
		set_nvram_named_value_status, "");
	if (status != 0)
		return status;

	return rc_wan_nvram_set_required_params(
		zero_params, (unsigned int)(sizeof(zero_params) / sizeof(zero_params[0])),
		set_nvram_named_value_status, "0");
}

void
reset_man_vars(void)
{
	int status = reset_man_vars_status();

	if (status != 0)
		logmessage(LOGNAME,
			   "MAN/virtual IPTV route snapshot clear incomplete (status=%d)",
			   status);
}

static int wan_vars_reset_status;

static int
reset_wan_vars_status(void)
{
	int unit, wan_proto, proto_status;
	char mac_buf[24], *man_addr, *man_mask, *man_gate, *man_mtu;
	const char *wan_hwaddr;
	const char *reset_failure_context = NULL;
	static const char *const dhcp_ppp_params[] = {
		"dnsenable_x", "vci", "hostname"
	};
	static const char *const auth_params[] = {
		"auth_mode", "auth_user", "auth_pass"
	};
	static const char *const static_ipoe_params[] = {
		"ipaddr", "netmask", "gateway", "mtu"
	};
	static const char *const ppp_credential_params[] = {
		"pppoe_username", "pppoe_passwd"
	};
	static const char *const ppp_option_params[] = {
		"pppoe_txonly_x", "pppoe_service", "pppoe_ac", "pppoe_man",
		"pppoe_mtu", "pppoe_mru", "pptp_mtu", "pptp_mru",
		"l2tp_mtu", "l2tp_mru", "ppp_peer", "ppp_auth", "ppp_mppe",
		"ppp_lcp", "ppp_alcp", "ppp_pppd"
	};
	static const struct rc_wan_nvram_value runtime_clear_values[] = {
		{ "dns", "" }, { "wins", "" }, { "lease", "" },
		{ "domain", "" }, { "routes", "" }, { "routes_ms", "" },
		{ "routes_rfc", "" }, { "err", "0" }
	};
	static const struct rc_wan_nvram_value nonstatic_address_clear_values[] = {
		{ "ipaddr", "0.0.0.0" }, { "netmask", "0.0.0.0" },
		{ "gateway", "0.0.0.0" }, { "mtu", "" }
	};


	wan_vars_reset_status = 0;
	unit = 0;

	clear_wan_state();
	proto_status = reset_man_vars_status();
	if (proto_status != 0) {
		reset_failure_context = "MAN/virtual IPTV route snapshot";
		goto reset_failed;
	}

	/*
	 * get_wan_proto() reads wanN_proto, so a failed copy from wan_proto
	 * must not leave a previous protocol selected for this WAN restart.
	 */
	proto_status = set_wan_unit_value_status(
		unit, "proto", nvram_safe_get("wan_proto"));
	if (proto_status != 0) {
		wan_vars_reset_status = proto_status;
		errno = proto_status;
		logmessage(LOGNAME,
			   "cannot stage configured WAN protocol before reset (status=%d)",
			   proto_status);
		return proto_status;
	}
	proto_status = rc_wan_nvram_set_required_params(
		dhcp_ppp_params,
		(unsigned int)(sizeof(dhcp_ppp_params) / sizeof(dhcp_ppp_params[0])),
		set_wan_unit_param_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN DHCP/DNS client options";
		goto reset_failed;
	}

	proto_status = rc_wan_nvram_set_required_params(
		auth_params, (unsigned int)(sizeof(auth_params) / sizeof(auth_params[0])),
		set_wan_unit_param_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN authentication mode and credentials";
		goto reset_failed;
	}


	proto_status = rc_wan_nvram_set_required_values(
		runtime_clear_values,
		(unsigned int)(sizeof(runtime_clear_values) / sizeof(runtime_clear_values[0])),
		set_wan_unit_value_status_callback, &unit);
	if (proto_status != 0) {
		reset_failure_context = "WAN runtime DNS and route snapshot";
		goto reset_failed;
	}


	wan_proto = get_wan_proto(unit);

	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
		proto_status = rc_wan_nvram_set_required_params(
			static_ipoe_params,
			(unsigned int)(sizeof(static_ipoe_params) /
				       sizeof(static_ipoe_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "static IPoE address settings";
			goto reset_failed;
		}
	} else {

		proto_status = rc_wan_nvram_set_required_values(
			nonstatic_address_clear_values,
			(unsigned int)(sizeof(nonstatic_address_clear_values) /
				       sizeof(nonstatic_address_clear_values[0])),
			set_wan_unit_value_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "non-static WAN address snapshot";
			goto reset_failed;
		}

	}

	man_addr = "0.0.0.0";
	man_mask = "0.0.0.0";
	man_gate = "0.0.0.0";
	man_mtu  = "";

	if (nvram_match("x_DHCPClient", "0")) {
		if (wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP ||
		   (wan_proto == IPV4_WAN_PROTO_PPPOE && nvram_match("wan_pppoe_man", "1"))) {
			man_addr = nvram_safe_get("wan_ipaddr");
			man_mask = nvram_safe_get("wan_netmask");
			man_gate = nvram_safe_get("wan_gateway");
			man_mtu  = nvram_safe_get("wan_mtu");
		}
	}

	proto_status = set_wan_unit_value_status(unit, "man_ipaddr", man_addr);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_netmask", man_mask);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_gateway", man_gate);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "man_mtu", man_mtu);
	if (proto_status != 0) {
		reset_failure_context = "manual WAN endpoint settings";
		goto reset_failed;
	}

	if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
	    wan_proto == IPV4_WAN_PROTO_PPTP ||
	    wan_proto == IPV4_WAN_PROTO_L2TP) {
		proto_status = set_nvram_value_status("wanx_ipaddr", man_addr);
		if (proto_status == 0 && is_valid_ipv4(man_addr) &&
		    is_valid_ipv4(man_mask))
			proto_status = set_nvram_value_status("wanx_netmask", man_mask);
		if (proto_status == 0 && is_valid_ipv4(man_gate))
			proto_status = set_nvram_value_status("wanx_gateway", man_gate);
		if (proto_status != 0) {
			reset_failure_context = "global MAN route endpoint snapshot";
			goto reset_failed;
		}
		
		proto_status = rc_wan_nvram_set_required_params(
			ppp_credential_params,
			(unsigned int)(sizeof(ppp_credential_params) /
				       sizeof(ppp_credential_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "PPP authentication credentials";
			goto reset_failed;
		}

		if (wan_proto == IPV4_WAN_PROTO_PPPOE)
			proto_status = set_wan_unit_param_status(unit, "pppoe_idletime");
		else
			proto_status = set_wan_unit_value_status(
				unit, "pppoe_idletime", "0");
		if (proto_status != 0) {
			reset_failure_context = "PPP idle-time setting";
			goto reset_failed;
		}

		proto_status = rc_wan_nvram_set_required_params(
			ppp_option_params,
			(unsigned int)(sizeof(ppp_option_params) /
				       sizeof(ppp_option_params[0])),
			set_wan_unit_param_status_callback, &unit);
		if (proto_status != 0) {
			reset_failure_context = "PPP peer and negotiation options";
			goto reset_failed;
		}
	}

	mac_buf[0] = 0;
	mac_conv("wan_hwaddr_x", -1, mac_buf);
	wan_hwaddr = (strlen(mac_buf) == 17) ?
		mac_buf : nvram_safe_get("il1macaddr");
	proto_status = set_nvram_persistent_value_status("wan_hwaddr", wan_hwaddr);
	if (proto_status == 0)
		proto_status = set_wan_unit_value_status(unit, "hwaddr", wan_hwaddr);
	if (proto_status != 0) {
		reset_failure_context = "WAN interface hardware address";
		goto reset_failed;
	}

#if defined (USE_IPV6)
	reset_wan6_vars();
#endif
	return 0;

reset_failed:
	wan_vars_reset_status = proto_status;
	errno = proto_status;
	logmessage(LOGNAME,
		   "cannot stage %s before WAN activation (status=%d)",
		   reset_failure_context ? reset_failure_context : "required WAN settings",
		   proto_status);
	return proto_status;
}

void
reset_wan_vars(void)
{
	(void)reset_wan_vars_status();
}

int
get_vlan_vid_wan(void)
{
	int vlan_vid;
	int is_vlan_filter;

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter)
		vlan_vid = nvram_get_int("vlan_vid_cpu");
	else
		vlan_vid = 0;

	if (!is_vlan_vid_valid(vlan_vid)) {
		vlan_vid = phy_vlan_pvid_wan_get();
		if (vlan_vid < 0)
			return vlan_vid;
		if (!is_vlan_vid_valid(vlan_vid))
			return -EINVAL;
	}

	return vlan_vid;
}

int
get_wan_bridge_mode(void)
{
	int bridge_mode = nvram_get_int("wan_stb_x");

	if (bridge_mode < 0 || bridge_mode > 7)
		bridge_mode = SWAPI_WAN_BRIDGE_DISABLE;

	return bridge_mode;
}

int
get_wan_bridge_iso_mode(int bridge_mode)
{
	int bwan_isolation = nvram_get_int("wan_stb_iso");

	if (bwan_isolation < 0 || bwan_isolation > 2 || bridge_mode == SWAPI_WAN_BRIDGE_DISABLE)
		bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;

	return bwan_isolation;
}

struct soft_bridge_wan_ownership {
	char bridge_name[16];
	char wan_ifname[24];
	char bwan_ifname[24];
	int bridge_created;
	int bridge_existed;
	int bridge_was_up;
	int bwan_vlan_created;
	int bwan_vlan_existed;
	int bwan_vlan_was_up;
	int wan_port_attach_attempted;
	int wan_port_added;
	int wan_port_detached_safe;
	int bwan_port_attach_attempted;
	int bwan_port_added;
	int bwan_port_detached_safe;
};

struct soft_bridge_other_entry {
	char port_ifname[24];
	int port_vlan_created;
	int port_vlan_existed;
	int port_vlan_was_up;
	struct soft_bridge_wan_ownership bridge;
};

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)

#define BRPREFIX_WAN	"brwan"

static int
config_soft_bridge_lan(void)
{
	char lan_ifname[24];
	const char *wired_ifname;
	int igmp_snoop = nvram_get_int("ether_igmp");
	int wired_m2u = nvram_get_int("ether_m2u");
	int ret, saved_errno, cleanup_ret;
	int bridge_enabled, vlan_flags, vlan_exists, vlan_was_up;
	int vlan_created = 0;
	int physical_detached = 0, vlan_attached = 0;
	int vlan_detached = 0, physical_attached = 0;
	int vlan_attach_attempted = 0;
	int vlan_attach_ambiguous = 0, physical_attach_ambiguous = 0;
	int vlan_state_safe = 1, physical_state_safe = 1;
	int vlan_detached_safe = 0;

	snprintf(lan_ifname, sizeof(lan_ifname), "%s.%d", IFNAME_MAC, 1);
	bridge_enabled = get_wan_bridge_mode() != SWAPI_WAN_BRIDGE_DISABLE;

	/*
	 * Snapshot the VLAN before changing it.  The tracked creator publishes
	 * ownership only after its full setup succeeds; a reused VLAN must retain
	 * its original administrative state when the bridge transaction rolls
	 * back.
	 */
	vlan_flags = get_interface_flags(lan_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO)
		return -1;
	vlan_exists = vlan_flags >= 0;
	vlan_was_up = vlan_exists && ((vlan_flags & IFF_UP) != 0);

	if (bridge_enabled) {
		ret = create_vlan_iface_tracked(IFNAME_MAC, 1, -1, -1,
						nvram_safe_get("lan_hwaddr"), 1,
						&vlan_created);
		if (ret != 0)
			goto rollback;
#if defined (USE_IPV6)
		control_if_ipv6(lan_ifname, 0);
#endif
		ret = br_add_del_if_tracked(IFNAME_BR, IFNAME_MAC, 0,
					    &physical_detached);
		if (ret != 0)
			goto rollback;
		vlan_attach_attempted = 1;
		ret = br_add_del_if_tracked(IFNAME_BR, lan_ifname, 1,
					    &vlan_attached);
		if (ret != 0) {
			vlan_attach_ambiguous = 1;
			goto rollback;
		}
		wired_ifname = lan_ifname;
	} else {
		if (vlan_exists) {
			ret = br_add_del_if_tracked(IFNAME_BR, lan_ifname, 0,
						    &vlan_detached);
			if (ret != 0)
				goto rollback;
		}
		ret = br_add_del_if_tracked(IFNAME_BR, IFNAME_MAC, 1,
					    &physical_attached);
		if (ret != 0) {
			physical_attach_ambiguous = 1;
			goto rollback;
		}
		wired_ifname = IFNAME_MAC;
	}

	ret = brport_set_m2u(wired_ifname,
			     (igmp_snoop && wired_m2u == 1) ? 1 : 0);
	if (ret != 0)
		goto rollback;

	/* Delete the old VLAN only after the replacement port is fully ready. */
	if (!bridge_enabled && vlan_exists) {
		ret = remove_vlan_iface(lan_ifname);
		if (ret != 0)
			goto rollback;
	}

	return 0;

rollback:
	saved_errno = errno;
	if (bridge_enabled) {
		/*
		 * Undo only a VLAN membership created by this call.  A successful
		 * idempotent add (changed_out == 0) belongs to the pre-existing
		 * topology and must not be detached or deleted here.
		 */
		if (vlan_attached) {
			cleanup_ret = br_add_del_if(IFNAME_BR, lan_ifname, 0);
			if (cleanup_ret == 0)
				vlan_detached_safe = 1;
			else {
				vlan_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not detach %s (status=%d)",
					   lan_ifname, cleanup_ret);
			}
		} else if (!vlan_attach_attempted) {
			vlan_detached_safe = 1;
		}

		if (vlan_attach_ambiguous)
			vlan_state_safe = 0;

		/* Restore the physical port only after the new VLAN port is safe. */
		if (physical_detached && vlan_state_safe) {
			cleanup_ret = br_add_del_if(IFNAME_BR, IFNAME_MAC, 1);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore %s (status=%d)",
					   IFNAME_MAC, cleanup_ret);
		}

		if (bridge_rollback_can_remove_created_vlan(vlan_created,
							    vlan_detached_safe)) {
			cleanup_ret = remove_vlan_iface(lan_ifname);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not remove new VLAN %s (status=%d)",
					   lan_ifname, cleanup_ret);
		} else if (bridge_rollback_can_restore_interface(vlan_exists,
								 vlan_state_safe)) {
			cleanup_ret = vlan_was_up ?
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(lan_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore VLAN %s state (status=%d)",
					   lan_ifname, cleanup_ret);
		}
	} else {
		/*
		 * On the reverse transition, remove only the physical membership
		 * introduced here before restoring the detached VLAN membership.
		 */
		if (physical_attached) {
			cleanup_ret = br_add_del_if(IFNAME_BR, IFNAME_MAC, 0);
			if (cleanup_ret != 0) {
				physical_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not detach %s (status=%d)",
					   IFNAME_MAC, cleanup_ret);
			}
		}
		if (physical_attach_ambiguous)
			physical_state_safe = 0;

		if (vlan_detached && physical_state_safe) {
			cleanup_ret = br_add_del_if(IFNAME_BR, lan_ifname, 1);
			if (cleanup_ret != 0) {
				vlan_state_safe = 0;
				logmessage(LOGNAME, "LAN bridge rollback could not restore %s membership (status=%d)",
					   lan_ifname, cleanup_ret);
			}
		}
		if (vlan_exists && physical_state_safe && vlan_state_safe) {
			cleanup_ret = vlan_was_up ?
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(lan_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "LAN bridge rollback could not restore VLAN %s state (status=%d)",
					   lan_ifname, cleanup_ret);
		}
	}

	logmessage(LOGNAME, "LAN bridge transition failed (status=%d); rollback attempted",
		   ret);
	errno = saved_errno;
	return ret;
}

static int
disassembly_bridges_wan(void)
{
	int i, ret, status = 0;
	char br_ifname[16];

	/* disassembly all wan bridges brwan5..brwan0 */
	for (i = 5; i >= 0; i--) {
		snprintf(br_ifname, sizeof(br_ifname), "%s%d", BRPREFIX_WAN, i);
		if (!is_interface_exist(br_ifname))
			continue;

		ret = ifconfig(br_ifname, 0, NULL, NULL);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0)
			logmessage(LOGNAME, "WAN bridge %s shutdown failed (status=%d)",
				   br_ifname, ret);

		ret = br_del_all_ifs(br_ifname, 1, 1);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s port teardown incomplete (status=%d)",
				   br_ifname, ret);
			continue;
		}

		ret = br_add_del_bridge(br_ifname, 0);
		status = wan_teardown_status_update(status, ret);
		if (ret != 0)
			logmessage(LOGNAME, "WAN bridge %s removal failed (status=%d)",
				   br_ifname, ret);
	}

	return status;
}



static int
config_soft_bridge_wan_internal(int br_id, int vid, int prio, const char *wan_ifname,
				const char *wan_hwaddr,
				struct soft_bridge_wan_ownership *ownership_out)
{
	struct soft_bridge_wan_ownership local_ownership;
	struct soft_bridge_wan_ownership *ownership =
		ownership_out ? ownership_out : &local_ownership;
	char br_ifname[16], bwan_ifname[24];
	int ret, saved_errno, cleanup_ret, bridge_empty;
	int bridge_flags, bridge_created = 0, bridge_was_up = 0;
	int bridge_remove_safe = 1;
	int vlan_flags = -1, vlan_exists = 0, vlan_was_up = 0;
	int vlan_created = 0, vlan_remove_safe = 1, vlan_restore_safe = 1;
	int wan_attach_attempted = 0, wan_port_added = 0;
	int bwan_attach_attempted = 0, bwan_port_added = 0;

	memset(ownership, 0, sizeof(*ownership));
	ownership->wan_port_detached_safe = 1;
	ownership->bwan_port_detached_safe = 1;
	bwan_ifname[0] = '\0';
	snprintf(br_ifname, sizeof(br_ifname), "%s%d", BRPREFIX_WAN, br_id);
	snprintf(ownership->bridge_name, sizeof(ownership->bridge_name), "%s", br_ifname);
	snprintf(ownership->wan_ifname, sizeof(ownership->wan_ifname), "%s", wan_ifname);
	bridge_flags = get_interface_flags(br_ifname);
	if (bridge_flags < 0) {
		if (errno != ENODEV && errno != ENXIO)
			return -1;
		ret = br_add_del_bridge(br_ifname, 1);
		if (ret != 0)
			return ret;
		bridge_created = 1;
		ownership->bridge_created = 1;
	} else {
		ownership->bridge_existed = 1;
		bridge_was_up = (bridge_flags & IFF_UP) != 0;
		ownership->bridge_was_up = bridge_was_up;
		ret = ifconfig(br_ifname, 0, NULL, NULL);
		if (ret != 0)
			goto rollback;
	}

	if (bridge_created) {
		ret = br_set_stp(br_ifname, 0);
		if (ret != 0)
			goto rollback;
		ret = br_set_fd(br_ifname, 2);
		if (ret != 0)
			goto rollback;
		ret = br_set_param_int(br_ifname, "multicast_snooping", 0);
		if (ret != 0)
			goto rollback;
	}
	ret = set_interface_hwaddr(br_ifname, wan_hwaddr);
	if (ret != 0)
		goto rollback;
	ret = set_interface_mtu(br_ifname, 1500);
	if (ret != 0)
		goto rollback;

	snprintf(bwan_ifname, sizeof(bwan_ifname), "%s.%d", IFNAME_MAC, vid);
	vlan_flags = get_interface_flags(bwan_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO) {
		ret = -1;
		goto rollback;
	}
	vlan_exists = vlan_flags >= 0;
	vlan_was_up = vlan_exists && ((vlan_flags & IFF_UP) != 0);
	ownership->bwan_vlan_existed = vlan_exists;
	ownership->bwan_vlan_was_up = vlan_was_up;
	snprintf(ownership->bwan_ifname, sizeof(ownership->bwan_ifname), "%s", bwan_ifname);
	ret = create_vlan_iface_tracked(IFNAME_MAC, vid, prio, -1,
					wan_hwaddr, 1, &vlan_created);
	ownership->bwan_vlan_created = vlan_created;
	if (ret != 0)
		goto rollback;

#if defined (USE_IPV6)
	control_if_ipv6(wan_ifname, 0);
	control_if_ipv6(bwan_ifname, 0);
#endif
	wan_attach_attempted = 1;
	ownership->wan_port_attach_attempted = 1;
	ownership->wan_port_detached_safe = 0;
	ret = br_add_del_if_tracked(br_ifname, wan_ifname, 1, &wan_port_added);
	ownership->wan_port_added = wan_port_added;
	if (ret != 0)
		goto rollback;
	bwan_attach_attempted = 1;
	ownership->bwan_port_attach_attempted = 1;
	ownership->bwan_port_detached_safe = 0;
	ret = br_add_del_if_tracked(br_ifname, bwan_ifname, 1, &bwan_port_added);
	ownership->bwan_port_added = bwan_port_added;
	if (ret != 0)
		goto rollback;
	if (br_id < 2) {
		if (nvram_get_int("wan_stb_iso") == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
			ret = brport_set_param_int(wan_ifname, "isolate_mode", 1);
			if (ret != 0)
				goto rollback;
			ret = brport_set_param_int(bwan_ifname, "isolate_mode", 1);
			if (ret != 0)
				goto rollback;
		}
	} else {
#if defined (USE_IPV6)
		control_if_ipv6(br_ifname, 0);
#endif
	}
	ret = ifconfig(br_ifname, IFUP, "0.0.0.0", NULL);
	if (ret != 0)
		goto rollback;
	return 0;

rollback:
	saved_errno = errno;
	/* Detach only memberships actually added by this invocation. */
	if (bwan_port_added) {
		cleanup_ret = br_add_del_if(br_ifname, bwan_ifname, 0);
		if (cleanup_ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s rollback could not detach %s (status=%d)",
				   br_ifname, bwan_ifname, cleanup_ret);
			vlan_remove_safe = 0;
			vlan_restore_safe = 0;
			bridge_remove_safe = 0;
		} else {
			ownership->bwan_port_detached_safe = 1;
		}
	} else if (bwan_attach_attempted) {
		ownership->bwan_port_detached_safe = 0;
	}
	if (wan_port_added) {
		cleanup_ret = br_add_del_if(br_ifname, wan_ifname, 0);
		if (cleanup_ret != 0) {
			logmessage(LOGNAME, "WAN bridge %s rollback could not detach %s (status=%d)",
				   br_ifname, wan_ifname, cleanup_ret);
			bridge_remove_safe = 0;
		} else {
			ownership->wan_port_detached_safe = 1;
		}
	} else if (wan_attach_attempted) {
		ownership->wan_port_detached_safe = 0;
	}
	/*
	 * A successful idempotent result or a raced attach is not ours to undo.
	 * Preserve the VLAN/bridge when ownership of an attempted attach is unclear.
	 */
	if (bwan_attach_attempted && !bwan_port_added) {
		vlan_remove_safe = 0;
		if (bridge_created)
			bridge_remove_safe = 0;
	}
	if (wan_attach_attempted && !wan_port_added && bridge_created)
		bridge_remove_safe = 0;
	bridge_remove_safe = bridge_remove_safe &&
		ownership->wan_port_detached_safe &&
		ownership->bwan_port_detached_safe;

	if (vlan_created) {
		if (vlan_remove_safe) {
			cleanup_ret = remove_vlan_iface(bwan_ifname);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not remove VLAN %s (status=%d)",
					   br_ifname, bwan_ifname, cleanup_ret);
		} else {
			logmessage(LOGNAME, "WAN bridge %s rollback preserved VLAN %s because attach ownership is uncertain",
				   br_ifname, bwan_ifname);
		}
	} else if (vlan_exists && vlan_restore_safe) {
		cleanup_ret = vlan_was_up ?
			ifconfig(bwan_ifname, IFUP, "0.0.0.0", NULL) :
			ifconfig(bwan_ifname, 0, NULL, NULL);
		if (cleanup_ret != 0)
			logmessage(LOGNAME, "WAN bridge %s rollback could not restore VLAN %s state (status=%d)",
				   br_ifname, bwan_ifname, cleanup_ret);
	}

	if (bridge_created) {
		bridge_empty = br_bridge_is_empty(br_ifname);

		if (bridge_remove_safe &&
		    bridge_batch_can_remove_created_bridge(bridge_created, bridge_empty)) {
			cleanup_ret = ifconfig(br_ifname, 0, NULL, NULL);
			if (cleanup_ret == 0)
				cleanup_ret = br_add_del_bridge(br_ifname, 0);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not remove new bridge (status=%d)",
					   br_ifname, cleanup_ret);
		} else {
			cleanup_ret = ifconfig(br_ifname, 0, NULL, NULL);
			if (cleanup_ret != 0)
				logmessage(LOGNAME, "WAN bridge %s rollback could not keep incomplete bridge down (status=%d)",
					   br_ifname, cleanup_ret);
			logmessage(LOGNAME, "WAN bridge %s retained because rollback could not prove ownership of all ports",
				   br_ifname);
		}
	} else if (bridge_was_up) {
		cleanup_ret = ifconfig(br_ifname, IFUP, "0.0.0.0", NULL);
		if (cleanup_ret != 0)
			logmessage(LOGNAME, "WAN bridge %s rollback could not restore its previous UP state (status=%d)",
				   br_ifname, cleanup_ret);
	}

	logmessage(LOGNAME, "WAN bridge %s setup failed (status=%d); rollback attempted",
		   br_ifname, ret);
	errno = saved_errno;
	return ret;
}

static int
config_soft_bridge_wan(int br_id, int vid, int prio, const char *wan_ifname,
		       const char *wan_hwaddr)
{
	return config_soft_bridge_wan_internal(br_id, vid, prio, wan_ifname,
					       wan_hwaddr, NULL);
}
static int
rollback_soft_bridge_other_entry(struct soft_bridge_other_entry *entry)
{
	struct soft_bridge_wan_ownership *bridge = &entry->bridge;
	int ret, bridge_flags, bridge_empty, bridge_empty_errno, status = 0;
	int wan_detached_safe, bwan_detached_safe;

	wan_detached_safe = bridge_batch_membership_detached_safe(
		bridge->wan_port_attach_attempted, bridge->wan_port_added,
		bridge->wan_port_detached_safe);
	bwan_detached_safe = bridge_batch_membership_detached_safe(
		bridge->bwan_port_attach_attempted, bridge->bwan_port_added,
		bridge->bwan_port_detached_safe);

	/* Reverse attachment order: WAN-side VLAN port was attached last. */
	if (bridge->bwan_port_added && !bwan_detached_safe) {
		ret = br_add_del_if(bridge->bridge_name, bridge->bwan_ifname, 0);
		bwan_detached_safe = (ret == 0);
		if (bwan_detached_safe)
			bridge->bwan_port_detached_safe = 1;
		if (ret != 0) {
			status = route_status_accumulate(status, ret);
			logmessage(LOGNAME, "LAN bridge batch rollback could not detach %s (status=%d)",
				   bridge->bwan_ifname, ret);
		}
	}
	if (bridge->wan_port_added && !wan_detached_safe) {
		ret = br_add_del_if(bridge->bridge_name, bridge->wan_ifname, 0);
		wan_detached_safe = (ret == 0);
		if (wan_detached_safe)
			bridge->wan_port_detached_safe = 1;
		if (ret != 0) {
			status = route_status_accumulate(status, ret);
			logmessage(LOGNAME, "LAN bridge batch rollback could not detach %s (status=%d)",
				   bridge->wan_ifname, ret);
		}
	}

	/* Restore/remove VLANs only after their bridge membership is known safe. */
	if (bridge->bwan_vlan_created) {
		if (bwan_detached_safe) {
			ret = remove_vlan_iface(bridge->bwan_ifname);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not remove WAN-side VLAN %s (status=%d)",
					   bridge->bwan_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s because membership ownership is uncertain",
				   bridge->bwan_ifname);
		}
	} else if (bridge->bwan_vlan_existed) {
		if (bwan_detached_safe) {
			ret = bridge->bwan_vlan_was_up ?
				ifconfig(bridge->bwan_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(bridge->bwan_ifname, 0, NULL, NULL);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not restore VLAN %s state (status=%d)",
					   bridge->bwan_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s state because membership ownership is uncertain",
				   bridge->bwan_ifname);
		}
	}

	if (entry->port_vlan_created) {
		if (wan_detached_safe) {
			ret = remove_vlan_iface(entry->port_ifname);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not remove VLAN %s (status=%d)",
					   entry->port_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s because membership ownership is uncertain",
				   entry->port_ifname);
		}
	} else if (entry->port_vlan_existed) {
		if (wan_detached_safe) {
			ret = entry->port_vlan_was_up ?
				ifconfig(entry->port_ifname, IFUP, "0.0.0.0", NULL) :
				ifconfig(entry->port_ifname, 0, NULL, NULL);
			if (ret != 0) {
				status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback could not restore VLAN %s state (status=%d)",
					   entry->port_ifname, ret);
			}
		} else {
			status = route_status_accumulate(status, EIO);
			logmessage(LOGNAME, "LAN bridge batch rollback preserved VLAN %s state because membership ownership is uncertain",
				   entry->port_ifname);
		}
	}

	if (bridge->bridge_name[0]) {
		bridge_flags = get_interface_flags(bridge->bridge_name);
		if (bridge->bridge_created) {
			if (bridge_flags >= 0) {
				bridge_empty = br_bridge_is_empty(bridge->bridge_name);
				bridge_empty_errno = bridge_empty < 0 ? (errno ? errno : EIO) : 0;
				if (bridge_batch_can_remove_created_bridge(1, bridge_empty) &&
				    wan_detached_safe && bwan_detached_safe) {
					ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
					if (ret == 0)
						ret = br_add_del_bridge(bridge->bridge_name, 0);
					if (ret != 0) {
						status = route_status_accumulate(status, ret);
						logmessage(LOGNAME, "LAN bridge batch rollback could not remove bridge %s (status=%d)",
							   bridge->bridge_name, ret);
					}
				} else {
					ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
					if (ret != 0)
						status = route_status_accumulate(status, ret);
					if (bridge_empty < 0)
						status = route_status_accumulate(status, bridge_empty_errno);
					else
						status = route_status_accumulate(status, EBUSY);
					logmessage(LOGNAME, "LAN bridge batch rollback preserved bridge %s because it is non-empty or its port inventory is unknown",
						   bridge->bridge_name);
				}
			} else if (errno != ENODEV && errno != ENXIO) {
				status = route_status_accumulate(status, errno ? errno : EIO);
				logmessage(LOGNAME, "LAN bridge batch rollback could not inspect bridge %s",
					   bridge->bridge_name);
			}
		} else if (bridge->bridge_existed) {
			if (wan_detached_safe && bwan_detached_safe) {
				ret = bridge->bridge_was_up ?
					ifconfig(bridge->bridge_name, IFUP, "0.0.0.0", NULL) :
					ifconfig(bridge->bridge_name, 0, NULL, NULL);
				if (ret != 0) {
					status = route_status_accumulate(status, ret);
					logmessage(LOGNAME, "LAN bridge batch rollback could not restore bridge %s state (status=%d)",
						   bridge->bridge_name, ret);
				}
			} else {
				ret = ifconfig(bridge->bridge_name, 0, NULL, NULL);
				status = route_status_accumulate(status, EIO);
				if (ret != 0)
					status = route_status_accumulate(status, ret);
				logmessage(LOGNAME, "LAN bridge batch rollback kept bridge %s down because membership ownership is uncertain",
					   bridge->bridge_name);
			}
		}
	}
	return status;
}

static int
config_soft_bridges_other(int vinet_vid, int viptv_vid, int pvid_wan, const char *wan_hwaddr, int bridge_mode)
{
	struct soft_bridge_other_entry entries[4];
	struct soft_bridge_other_entry *entry;
	int i, br_id, port_vid[4], port_pri[4] = {0};
	int ret, saved_errno, port_flags, entry_count = 0;
	char port_ifname[24];

	for (i = 0; i < 4; i++)
		port_vid[i] = -1;
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN1:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		port_vid[0] = nvram_get_int("vlan_vid_lan1");
		port_pri[0] = nvram_get_int("vlan_pri_lan1") & 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		port_vid[1] = nvram_get_int("vlan_vid_lan2");
		port_pri[1] = nvram_get_int("vlan_pri_lan2") & 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN3:
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		port_vid[2] = nvram_get_int("vlan_vid_lan3");
		port_pri[2] = nvram_get_int("vlan_pri_lan3") & 0x07;
		break;
	}
	switch (bridge_mode) {
	case SWAPI_WAN_BRIDGE_LAN4:
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		port_vid[3] = nvram_get_int("vlan_vid_lan4");
		port_pri[3] = nvram_get_int("vlan_pri_lan4") & 0x07;
		break;
	}
	for (i = 0; i < 4; i++) {
		if (port_vid[i] < 0)
			continue;
		if (!is_vlan_vid_valid(port_vid[i]))
			port_vid[i] = pvid_wan;
		if (port_vid[i] == vinet_vid || port_vid[i] == viptv_vid)
			port_vid[i] = 0;
	}
	br_id = 2;
	for (i = 0; i < 4; i++) {
		if (port_vid[i] < 2)
			continue;
		entry = &entries[entry_count];
		memset(entry, 0, sizeof(*entry));
		snprintf(port_ifname, sizeof(port_ifname), "%s.%d", IFNAME_MAC2, port_vid[i]);
		snprintf(entry->port_ifname, sizeof(entry->port_ifname), "%s", port_ifname);
		port_flags = get_interface_flags(port_ifname);
		if (port_flags < 0 && errno != ENODEV && errno != ENXIO) {
			ret = -1;
			goto rollback;
		}
		entry->port_vlan_existed = port_flags >= 0;
		entry->port_vlan_was_up = entry->port_vlan_existed && (port_flags & IFF_UP);
		entry_count++;

		ret = create_vlan_iface_tracked(IFNAME_MAC2, port_vid[i], port_pri[i],
						1500, wan_hwaddr, 1,
						&entry->port_vlan_created);
		if (ret != 0) {
			logmessage(LOGNAME, "LAN bridge VLAN creation failed for %s (status=%d)",
				   port_ifname, ret);
			goto rollback;
		}
		ret = config_soft_bridge_wan_internal(br_id, port_vid[i], port_pri[i],
						      port_ifname, wan_hwaddr,
						      &entry->bridge);
		if (ret != 0) {
			logmessage(LOGNAME, "LAN bridge setup failed for %s (status=%d)",
				   port_ifname, ret);
			goto rollback;
		}
		br_id++;
	}
	return 0;

rollback:
	saved_errno = errno;
	while (entry_count > 0) {
		int rollback_status = rollback_soft_bridge_other_entry(&entries[--entry_count]);
		if (rollback_status != 0)
			logmessage(LOGNAME, "additional WAN bridge batch rollback incomplete (status=%d)",
				   rollback_status);
	}
	errno = saved_errno;
	return ret;
}
#endif

static int
rollback_viptv_vlan_setup(struct soft_bridge_other_entry *entry)
{
	int status = 0, ret = 0;
	const char *recovery_ifname = NULL;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	int bridge_flags;

	status = rollback_soft_bridge_other_entry(entry);
	if (entry->bridge.bridge_name[0]) {
		bridge_flags = get_interface_flags(entry->bridge.bridge_name);
		if (entry->bridge.bridge_existed || bridge_flags >= 0 ||
		    (bridge_flags < 0 && errno != ENODEV && errno != ENXIO))
			recovery_ifname = entry->bridge.bridge_name;
		else if (status != 0 || entry->port_vlan_existed)
			recovery_ifname = entry->port_ifname;
	} else if (status != 0 || entry->port_vlan_existed) {
		recovery_ifname = entry->port_ifname;
	}
#else
	/* Boards without GMAC2 bridge support still need VLAN-only rollback. */
	if (entry->port_vlan_created) {
		ret = remove_vlan_iface(entry->port_ifname);
	} else if (entry->port_vlan_existed) {
		ret = entry->port_vlan_was_up ?
			ifconfig(entry->port_ifname, IFUP, "0.0.0.0", NULL) :
			ifconfig(entry->port_ifname, 0, NULL, NULL);
	}
	if (ret != 0) {
		status = route_status_accumulate(status, ret);
		logmessage(LOGNAME, "IPTV VLAN-only rollback incomplete for %s (status=%d)",
			   entry->port_ifname, ret);
	}
	if (status != 0 || entry->port_vlan_existed)
		recovery_ifname = entry->port_ifname;
#endif

	if (recovery_ifname)
		nvram_set_temp("viptv_ifname", recovery_ifname);
	else
		nvram_set_temp("viptv_ifname", "");
	return status;
}

static int
config_vinet_wan(void)
{
	int vlan_vid[2], vlan_pri, pvid_wan;
	int is_vlan_filter, is_vlan_ifname;
	int need_pvid, pvid_result = 0, ret;
	char vinet_ifname[24];
	const char *vinet_iflast, *wan_hwaddr;
#if defined (USE_SINGLE_MAC)
	const char *ifname_wan_cpu = IFNAME_MAC;
#else
	const char *ifname_wan_cpu = IFNAME_MAC2;
#endif
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	int bridge_mode = get_wan_bridge_mode();
#endif

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter) {
		vlan_vid[0] = nvram_get_int("vlan_vid_cpu");
		vlan_vid[1] = nvram_get_int("vlan_vid_iptv");
		vlan_pri = nvram_get_int("vlan_pri_cpu") & 0x07;
	} else {
		vlan_vid[0] = 0;
		vlan_vid[1] = 0;
		vlan_pri = 0;
	}

	need_pvid = (!is_vlan_vid_valid(vlan_vid[0]) ||
		     !is_vlan_vid_valid(vlan_vid[1]));
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE && is_vlan_filter)
		need_pvid = 1;
#endif
	if (need_pvid) {
		pvid_result = phy_vlan_pvid_wan_get();
		if (pvid_result < 0 || !is_vlan_vid_valid(pvid_result)) {
			ret = (pvid_result < 0) ? pvid_result : -EINVAL;
			logmessage(LOGNAME, "cannot configure WAN VLANs: switch PVID read failed (%d)", pvid_result);
			return ret;
		}
	}
	pvid_wan = pvid_result;
	{
		int configured_vid = vlan_vid[0];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_result, is_vlan_vid_valid(pvid_result),
						     &vlan_vid[0]);
		if (ret != 0)
			return ret;
		if (!is_vlan_vid_valid(configured_vid))
			vlan_pri = 0;
	}
	{
		int configured_vid = vlan_vid[1];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_result, is_vlan_vid_valid(pvid_result),
						     &vlan_vid[1]);
		if (ret != 0)
			return ret;
	}

	/* update VLAN VID for raeth (HW_VLAN_TX) */
	hw_vlan_tx_map(6, vlan_vid[0]);
	hw_vlan_tx_map(7, vlan_vid[1]);

	wan_hwaddr = nvram_safe_get("wan_hwaddr");

	is_vlan_ifname = 1;
	snprintf(vinet_ifname, sizeof(vinet_ifname), "%s.%d", ifname_wan_cpu, vlan_vid[0]);
#if !defined (USE_SINGLE_MAC)
#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	/* case1: CPU Internet tagged: n, CPU IPTV tagged: n */
	/* case2: CPU Internet tagged: y, CPU IPTV tagged: y (common VID) */
	if (vlan_vid[0] == vlan_vid[1])
#else
	/* case: no VLAN filter */
	if (!is_vlan_filter)
#endif
	{
		is_vlan_ifname = 0;
		snprintf(vinet_ifname, sizeof(vinet_ifname), "%s", IFNAME_MAC2);
	}

	/* always prepare eth3 interface */
	ret = ifconfig(IFNAME_MAC2, 0, NULL, NULL);
	if (ret != 0)
		return ret;
	ret = set_interface_hwaddr(IFNAME_MAC2, wan_hwaddr);
	if (ret != 0)
		return ret;
	ret = set_interface_mtu(IFNAME_MAC2, 1500);
	if (ret != 0)
		return ret;
	ret = ifconfig(IFNAME_MAC2, IFUP, "0.0.0.0", NULL);
	if (ret != 0)
		return ret;
#endif

	/* remove previous VLAN interface */
	vinet_iflast = get_wan_unit_value(0, "viflast");
	if (*vinet_iflast && strcmp(vinet_iflast, vinet_ifname)) {
		ret = remove_vlan_iface(vinet_iflast);
		if (ret != 0) {
			logmessage(LOGNAME, "previous WAN VLAN removal failed for %s (status=%d)",
				   vinet_iflast, ret);
			return ret;
		}
	}

	/* create VLAN interface for INET */
	if (is_vlan_ifname) {
#if defined (USE_IPV6)
		if (get_ipv6_type() != IPV6_DISABLED) {
			ret = control_if_ipv6(ifname_wan_cpu, 0);
			if (ret != 0) {
				logmessage(LOGNAME, "failed to disable IPv6 on WAN VLAN parent %s (status=%d)",
					   ifname_wan_cpu, ret);
				return ret;
			}
		}
#endif
		ret = create_vlan_iface(ifname_wan_cpu, vlan_vid[0], vlan_pri, 1500, wan_hwaddr, 1);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN VLAN creation failed for %s (status=%d)",
				   vinet_ifname, ret);
			return ret;
		}
	}

#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE && is_vlan_filter) {
		ret = config_soft_bridges_other(vlan_vid[0], vlan_vid[1], pvid_wan, wan_hwaddr, bridge_mode);
		if (ret != 0)
			return ret;
	}
	if (bridge_mode != SWAPI_WAN_BRIDGE_DISABLE) {
		ret = config_soft_bridge_wan(0, vlan_vid[0], vlan_pri, vinet_ifname, wan_hwaddr);
		if (ret != 0) {
			logmessage(LOGNAME, "WAN bridge brwan0 setup failed (status=%d)", ret);
			return ret;
		}
		snprintf(vinet_ifname, sizeof(vinet_ifname), "%s%d", BRPREFIX_WAN, 0);
	}
#endif

	if (is_vlan_ifname) {
		char vlan_iflast[24];

		snprintf(vlan_iflast, sizeof(vlan_iflast), "%s.%d", ifname_wan_cpu, vlan_vid[0]);
		set_wan_unit_value(0, "viflast", vlan_iflast);
	} else {
		set_wan_unit_value(0, "viflast", "");
	}

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED) {
		ret = control_if_ipv6(vinet_ifname, 1);
		if (ret != 0) {
			logmessage(LOGNAME, "failed to enable IPv6 on WAN interface %s (status=%d)",
				   vinet_ifname, ret);
			return ret;
		}
	}
#endif
	set_wan_unit_value(0, "ifname", vinet_ifname);
	return 0;
}

static int
config_apcli_wisp(void)
{
	int unit = 0;
	char *man_ifname;
	char *wisp_ifname = get_apcli_wisp_ifname();

	if (!wisp_ifname)
		return 0;

	man_ifname = get_man_ifname(unit);
	if (strlen(man_ifname) > 0 && !is_man_wisp(man_ifname)) {
		int address_status, down_status;

		address_status = ifconfig(man_ifname, IFUP, "0.0.0.0", NULL);
		if (address_status != 0)
			logmessage(LOGNAME, "failed to clear previous WAN address on %s (status=%d)",
				   man_ifname, address_status);
#if defined (USE_SINGLE_MAC)
		down_status = ifconfig(man_ifname, 0, "0.0.0.0", NULL);
		if (down_status != 0) {
			logmessage(LOGNAME, "failed to shut down previous WAN interface %s (status=%d)",
				   man_ifname, down_status);
			down_status = ifconfig(man_ifname, 0, NULL, NULL);
			if (down_status != 0) {
				logmessage(LOGNAME, "unable to keep previous WAN interface %s down; refusing WISP migration (status=%d)",
					   man_ifname, down_status);
				return down_status;
			}
		}
#else
		if (address_status != 0) {
			down_status = ifconfig(man_ifname, 0, NULL, NULL);
			if (down_status != 0) {
				logmessage(LOGNAME, "unable to clear or shut down previous WAN interface %s; refusing WISP migration (status=%d)",
					   man_ifname, down_status);
				return down_status;
			}
		}
#endif
	}

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED) {
		int ipv6_status = control_if_ipv6(wisp_ifname, 1);
		if (ipv6_status != 0) {
			logmessage(LOGNAME, "failed to enable IPv6 on AP-client WAN interface %s (status=%d)",
				   wisp_ifname, ipv6_status);
			return ipv6_status;
		}
	}
#endif
	set_wan_unit_value(0, "ifname", wisp_ifname);
	return 0;
}

static int
wait_apcli_connected(const char *ifname, int wait_sec)
{
	int i;

	for (i = 0; i <= (wait_sec * 2); i++) {
		if (service_wait_microseconds(500000U) != 0)
			return -1;
		if (get_apcli_connected(ifname))
			return 1;
	}

	return 0;
}


struct wan_viptv_static_start_context {
	char *ifname;
	char *ipaddr;
	char *netmask;
	char *gateway;
};

static int
wan_viptv_static_configure_address(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	return ifconfig(context->ifname, IFUP, context->ipaddr, context->netmask);
}

static void
wan_viptv_static_publish_address(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	nvram_set_temp("manv_ipaddr", context->ipaddr);
	nvram_set_temp("manv_netmask", context->netmask);
}

static void
wan_viptv_static_publish_gateway(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	nvram_set_temp("manv_gateway",
		       is_valid_ipv4(context->gateway) ? context->gateway : "");
}

static int
wan_viptv_static_add_default_route(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	return route_add(context->ifname, 10, "0.0.0.0",
			 context->gateway, "0.0.0.0");
}

static int
wan_viptv_static_start_multicast(void *opaque)
{
	struct wan_viptv_static_start_context *context =
		(struct wan_viptv_static_start_context *)opaque;

	return start_igmpproxy(context->ifname);
}

static const struct wan_viptv_static_ops wan_viptv_static_start_ops = {
	wan_viptv_static_configure_address,
	wan_viptv_static_publish_address,
	wan_viptv_static_publish_gateway,
	wan_viptv_static_add_default_route,
	wan_viptv_static_start_multicast
};
static int
launch_viptv_wan(void)
{
	int vlan_vid[2], vlan_pri, pvid_wan = 0;
	int need_pvid, ret;
	int is_vlan_filter, viptv_mode;
	int vlan_flags, saved_errno, rollback_status;
	char viptv_ifname[24], viptv_addr[16], viptv_mask[16], viptv_gate[16];
	const char *viptv_iflast, *vinet_iflast, *wan_hwaddr;
	const char *viptv_addr_setting, *viptv_mask_setting, *viptv_gate_setting;
	struct soft_bridge_other_entry rollback_entry;
#if defined (USE_SINGLE_MAC)
	const char *ifname_wan_cpu = IFNAME_MAC;
#else
	const char *ifname_wan_cpu = IFNAME_MAC2;
#endif

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter) {
		vlan_vid[0] = nvram_get_int("vlan_vid_cpu");
		vlan_vid[1] = nvram_get_int("vlan_vid_iptv");
		vlan_pri = nvram_get_int("vlan_pri_iptv") & 0x07;
	} else {
		vlan_vid[0] = 0;
		vlan_vid[1] = 0;
		vlan_pri = 0;
	}
	need_pvid = (!is_vlan_vid_valid(vlan_vid[0]) ||
		     !is_vlan_vid_valid(vlan_vid[1]));
	if (need_pvid) {
		pvid_wan = phy_vlan_pvid_wan_get();
		if (pvid_wan < 0 || !is_vlan_vid_valid(pvid_wan)) {
			ret = (pvid_wan < 0) ? pvid_wan : -EINVAL;
			logmessage(LOGNAME, "cannot configure IPTV VLANs: switch PVID read failed (%d)", pvid_wan);
			return ret;
		}
	}
	{
		int configured_vid = vlan_vid[0];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_wan, is_vlan_vid_valid(pvid_wan),
						     &vlan_vid[0]);
		if (ret != 0)
			return ret;
	}
	{
		int configured_vid = vlan_vid[1];
		ret = switch_status_resolve_vlan_vid(configured_vid,
						     is_vlan_vid_valid(configured_vid),
						     pvid_wan, is_vlan_vid_valid(pvid_wan),
						     &vlan_vid[1]);
		if (ret != 0)
			return ret;
		if (!is_vlan_vid_valid(configured_vid))
			vlan_pri = 0;
	}
	vinet_iflast = get_wan_unit_value(0, "viflast");
	viptv_iflast = nvram_safe_get("viptv_ifname");

	if (vlan_vid[1] == vlan_vid[0]) {
		/* case1: CPU Internet tagged: n, CPU IPTV tagged: n */
		/* case2: CPU Internet tagged: y, CPU IPTV tagged: y (common VID) */
		if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast)) {
			ret = remove_vlan_iface(viptv_iflast);
			if (ret != 0)
				return ret;
		}
		viptv_ifname[0] = 0;
		nvram_set_temp("viptv_ifname", viptv_ifname);
		return 0;
	}

	/* case3: CPU Internet tagged: y, CPU IPTV tagged: y (different VID) */
	/* case4: CPU Internet tagged: n, CPU IPTV tagged: y (different VID) */
	/* case5: CPU Internet tagged: y, CPU IPTV tagged: n */
	viptv_mode = nvram_get_int("viptv_mode");
	viptv_addr_setting = nvram_safe_get("viptv_ipaddr");
	viptv_mask_setting = nvram_safe_get("viptv_netmask");
	viptv_gate[0] = 0;
	if (viptv_mode == 2) {
		/* Snapshot NVRAM-backed strings before any temporary value is changed. */
		if (!is_valid_ipv4(viptv_addr_setting) ||
		    !is_valid_ipv4(viptv_mask_setting) ||
		    strlen(viptv_addr_setting) >= sizeof(viptv_addr) ||
		    strlen(viptv_mask_setting) >= sizeof(viptv_mask)) {
			logmessage(LOGNAME, "rejecting invalid static IPTV address or netmask");
			return EINVAL;
		}
		strcpy(viptv_addr, viptv_addr_setting);
		strcpy(viptv_mask, viptv_mask_setting);

		viptv_gate_setting = nvram_safe_get("viptv_gateway");
		if (is_valid_ipv4(viptv_gate_setting)) {
			if (strlen(viptv_gate_setting) >= sizeof(viptv_gate)) {
				logmessage(LOGNAME, "rejecting oversized static IPTV gateway");
				return EINVAL;
			}
			strcpy(viptv_gate, viptv_gate_setting);
		}
	}

	/* VLAN interface for IPTV */
	snprintf(viptv_ifname, sizeof(viptv_ifname), "%s.%d", ifname_wan_cpu, vlan_vid[1]);

	wan_hwaddr = nvram_safe_get("wan_hwaddr");
	memset(&rollback_entry, 0, sizeof(rollback_entry));
	snprintf(rollback_entry.port_ifname, sizeof(rollback_entry.port_ifname),
		 "%s", viptv_ifname);
	vlan_flags = get_interface_flags(viptv_ifname);
	if (vlan_flags < 0 && errno != ENODEV && errno != ENXIO)
		return -1;
	rollback_entry.port_vlan_existed = vlan_flags >= 0;
	rollback_entry.port_vlan_was_up =
		rollback_entry.port_vlan_existed && ((vlan_flags & IFF_UP) != 0);


	/* Remove a no-longer-used VLAN and its stale publication before replacement. */
	if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast) &&
	    strcmp(viptv_iflast, viptv_ifname)) {
		ret = remove_vlan_iface(viptv_iflast);
		if (ret != 0)
			return ret;
		if (strchr(viptv_iflast, '.') != NULL)
			nvram_set_temp("viptv_ifname", "");
	}

	ret = create_vlan_iface_tracked(ifname_wan_cpu, vlan_vid[1], vlan_pri,
					1500, wan_hwaddr, 1,
					&rollback_entry.port_vlan_created);
	if (ret != 0) {
		saved_errno = errno;
		rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
		if (rollback_status != 0)
			logmessage(LOGNAME, "IPTV VLAN creation rollback incomplete (status=%d)",
				   rollback_status);
		errno = saved_errno;
		return ret;
	}
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	if (get_wan_bridge_mode() != SWAPI_WAN_BRIDGE_DISABLE) {
		ret = config_soft_bridge_wan_internal(1, vlan_vid[1], vlan_pri,
						      viptv_ifname, wan_hwaddr,
						      &rollback_entry.bridge);
		if (ret != 0) {
			saved_errno = errno;
			rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
			if (rollback_status != 0)
				logmessage(LOGNAME, "IPTV VLAN/bridge rollback incomplete (status=%d)",
					   rollback_status);
			errno = saved_errno;
			return ret;
		}
		snprintf(viptv_ifname, sizeof(viptv_ifname), "%s%d", BRPREFIX_WAN, 1);
	}
#endif
	ret = set_interface_conf_int("ipv4", viptv_ifname, "rp_filter", 0);
	if (ret != 0) {
		saved_errno = errno;
		rollback_status = rollback_viptv_vlan_setup(&rollback_entry);
		if (rollback_status != 0)
			logmessage(LOGNAME, "IPTV interface setup rollback incomplete (status=%d)",
				   rollback_status);
		errno = saved_errno;
		return ret;
	}
	nvram_set_temp("viptv_ifname", viptv_ifname);

	if (viptv_mode == 2) {
		struct wan_viptv_static_start_context context;
		context.ifname = viptv_ifname;
		context.ipaddr = viptv_addr;
		context.netmask = viptv_mask;
		context.gateway = viptv_gate;
		ret = wan_viptv_static_start(is_valid_ipv4(viptv_gate),
					     &wan_viptv_static_start_ops, &context);
		if (ret != 0)
			logmessage(LOGNAME, "static IPTV startup incomplete on %s (error=%d)",
				   viptv_ifname, ret);
		else
			logmessage(LOGNAME, "%s %s (%s)", "IPTV MAN", "up", viptv_ifname);
		return ret;
	} else if (viptv_mode == 1) {
		ret = start_zcip_viptv(viptv_ifname);
	} else {
		ret = start_udhcpc_viptv(viptv_ifname);
	}
	return ret;
}

static int man_up_internal(char *, int, int, int *);

static int
launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)
{
	int launch_status = 0;
	int man_mtu = get_wan_unit_value_int(unit, "man_mtu");
	char *man_addr = get_wan_unit_value(unit, "man_ipaddr");
	char *man_mask = get_wan_unit_value(unit, "man_netmask");

	if (!is_valid_ipv4(man_addr))
		man_addr = "0.0.0.0";

	if (!is_valid_ipv4(man_addr) || !is_valid_ipv4(man_mask))
		man_mask = NULL;

	/* Do not launch a physical WAN client until the AP-client link is ready. */
	if (is_man_wisp(man_ifname)) {
		launch_status = rc_wan_apcli_wait_status(
			wait_apcli_connected(man_ifname, 3));
		if (launch_status != 0) {
			logmessage(LOGNAME,
				   "AP-client WAN interface %s did not associate before physical WAN setup (status=%d)",
				   man_ifname, launch_status);
			return launch_status;
		}
	}

	if (use_zcip || !is_valid_ipv4(man_addr)) {
		/* PPPoE connection not needed WAN physical address first, skip wait DHCP lease */
		/* PPTP and L2TP needed WAN physical first for create VPN tunnel, wait DHCP lease */
		if (!use_zcip) {
			launch_status = start_udhcpc_wan(man_ifname, unit, wait_dhcpc);
			
			/* add delay 2s after eth3 up: gethostbyname delay issue (L2TP/PPTP) */
			if (wait_dhcpc && launch_status == 0 &&
			    service_wait_seconds(2) != 0) {
				launch_status = errno > 0 ? errno : EIO;
				logmessage(LOGNAME, "WAN DHCP settling wait failed on %s (status=%d)",
					   man_ifname, launch_status);
			}
		} else
			launch_status = start_zcip_wan(man_ifname);
	} else {
		int man_err = 0, gateway_route_status = 0, route_status, address_status, mtu_status;
		char *lan_addr = nvram_safe_get("lan_ipaddr");
		char *lan_mask = nvram_safe_get("lan_netmask");
		
		if (!dhcp_lease_ipv4_is_valid(man_addr) ||
		    !dhcp_lease_netmask_is_valid(man_mask)) {
			man_err = 1;
			launch_status = EINVAL;
			logmessage(LOGNAME, "invalid static WAN IPv4 address or netmask on %s",
				   man_ifname);
		} else if (is_same_subnet2(man_addr, lan_addr, man_mask, lan_mask)) {
			man_err = 1;
			launch_status = EINVAL;
			
			logmessage(LOGNAME, "%s (%s) ERROR! Static IP [%s/%s] in LAN subnet [%s/%s]!",
				"MAN", man_ifname, man_addr, man_mask, lan_addr, lan_mask);
		} else {
			if (man_mtu >= 1300 && man_mtu < 1500) {
				mtu_status = set_interface_mtu(man_ifname, man_mtu);
				if (mtu_status != 0) {
					launch_status = route_status_accumulate(
						launch_status, mtu_status);
					logmessage(LOGNAME, "MAN interface %s MTU setup incomplete (status=%d)",
						   man_ifname, mtu_status);
				}
			}
			address_status = ifconfig(man_ifname, IFUP, man_addr, man_mask);
			if (address_status != 0) {
				man_err = 1;
				launch_status = route_status_accumulate(launch_status, address_status);
				logmessage(LOGNAME, "failed to configure static WAN address on %s (status=%d)",
					   man_ifname, address_status);
			} else {
				route_status = man_up_internal(man_ifname, unit, 1, &gateway_route_status);
				if (route_status != 0) {
					man_err = 1;
					logmessage(LOGNAME, "MAN route setup incomplete on %s (error=%d)",
						   man_ifname, route_status);
				}
				launch_status = route_status_accumulate(launch_status, route_status);
				if (gateway_route_status != 0) {
					man_err = 1;
					launch_status = route_status_accumulate(launch_status, gateway_route_status);
				}
			}
		}
		
		nvram_set_int_temp("wanx_err", man_err);
	}

#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 0 && !is_wan_ipv6_if_ppp()) {
		int ipv6_status = wan6_up(man_ifname, unit);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN physical IPv6 setup incomplete on %s (status=%d)",
				   man_ifname, ipv6_status);
		launch_status = route_status_accumulate(launch_status, ipv6_status);
	}
#endif

	return launch_status;
}

static int
wait_ppp_up(char *ppp_ifname, int unit)
{
	int timeout, sockfd, route_status;
	int interface_status, failure_status, socket_status;
	struct ifreq ifr;
	char *ppp_gate;

	/* Wait for pppX to be created */
	timeout = 5;
	while ((interface_status = ifconfig(ppp_ifname, IFUP, NULL, NULL)) != 0) {
		/* Six failed probes (five one-second waits) mean the interface never appeared. */
		if (timeout-- <= 0) {
			failure_status = (interface_status < 0 && errno != 0) ?
				errno : interface_status;
			logmessage(LOGNAME, "PPP interface %s did not appear before startup timeout (status=%d)",
				   ppp_ifname, failure_status);
			return failure_status;
		}
		if (service_wait_seconds(1) != 0) {
			failure_status = errno > 0 ? errno : EIO;
			logmessage(LOGNAME, "PPP interface %s startup wait failed (status=%d)",
				   ppp_ifname, failure_status);
			return failure_status;
		}
	}

	/* Retrieve IP info */
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		socket_status = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot inspect demand PPP interface %s (errno=%d)",
			   ppp_ifname, socket_status);
		return socket_status;
	}

	/* Store temporary IP address */
	ppp_gate = "";
	strncpy(ifr.ifr_name, ppp_ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0)
	{
		set_wan_unit_value(unit, "ipaddr", inet_ntoa(sin_addr(&ifr.ifr_addr)));
		set_wan_unit_value(unit, "netmask", "255.255.255.255");
		
		/* Store temporary P-t-P address */
		if (ioctl(sockfd, SIOCGIFDSTADDR, &ifr) == 0)
		{
			ppp_gate = inet_ntoa(sin_addr(&ifr.ifr_dstaddr));
			set_wan_unit_value(unit, "gateway", ppp_gate);
		}
	}
	close(sockfd);

	/* Preset routes so that traffic can be sent to proper pppx even before the link is brought up. */
	route_status = preset_wan_ppp_routes(ppp_ifname, unit);
	if (!route_status_demand_ready(route_status)) {
		logmessage(LOGNAME, "PPP demand route preinstallation incomplete on %s (status=%d)",
			   ppp_ifname, route_status);
		/*
		 * A failed batch can still have installed some routes. Do not blindly
		 * delete the configured set: EEXIST-normalized entries may predate this
		 * attempt. Demand pppd remains persistent; a later ip-up callback retries
		 * wan_up() route setup after start_wan() restores physical WAN publication.
		 */
		return route_status ? route_status : EIO;
	}

	/* Stimulate link up */
	if (is_valid_ipv4(ppp_gate))
		eval("ping", "-c1", ppp_gate);

	return 0;
}

static int
restore_wan_after_ppp_start_failure(int unit, char *wan_ifname)
{
	int upnp_status, firewall_status, nvram_status;

	/* Restore the physical path while demand PPP is unavailable. A later
	 * successful ip-up callback may switch the active WAN back to ppp0. */
	upnp_status = check_upnp_wanif_changed(wan_ifname);
	if (upnp_status != 0) {
		logmessage(LOGNAME, "cannot restore WAN after PPP start failure: miniupnpd did not stop (status=%d)",
			   upnp_status);
		return upnp_status;
	}
	nvram_status = set_wan_unit_value_status(unit, "ifname_t", wan_ifname);
	if (nvram_status != 0) {
		errno = nvram_status;
		logmessage(LOGNAME, "cannot publish physical WAN interface %s after PPP start failure (status=%d)",
			   wan_ifname, nvram_status);
		return nvram_status;
	}
	firewall_status = start_firewall_ex();
	if (firewall_status != 0)
		logmessage(LOGNAME, "cannot restore WAN firewall after PPP start failure (status=%d)",
			   firewall_status);
	return firewall_status;
}


static int
restore_wan_after_firewall_failure(int unit, char *wan_ifname)
{
	int restore_status, down_status;

	restore_status = restore_wan_after_ppp_start_failure(unit, wan_ifname);
	if (restore_status == 0)
		return 0;

	/*
	 * A stale or partial firewall is not sufficient to expose the WAN. If
	 * rebuilding the physical-WAN policy also fails, take that interface down.
	 */
	down_status = ifconfig(wan_ifname, 0, NULL, NULL);
	if (down_status != 0)
		logmessage(LOGNAME, "failed to keep physical WAN interface %s down after firewall recovery failure (status=%d)",
			   wan_ifname, down_status);
	return rc_first_service_start_status(restore_status, down_status);
}

static const char *const wan_callback_link_paths[] = {
 "/tmp/ppp/ip-up", "/tmp/ppp/ip-down",
 SCRIPT_UDHCPC_WAN, SCRIPT_UDHCPC_VIPTV, SCRIPT_ZCIP_WAN,
 SCRIPT_ZCIP_VIPTV, SCRIPT_WPACLI_WAN,
#if defined (USE_IPV6)
 "/tmp/ppp/ipv6-up", "/tmp/ppp/ipv6-down", SCRIPT_DHCP6C_WAN,
#endif
};
#define WAN_CALLBACK_LINK_COUNT (sizeof(wan_callback_link_paths)/sizeof(wan_callback_link_paths[0]))
static int
create_cb_links(void)
{
 unsigned char created[WAN_CALLBACK_LINK_COUNT];size_t i;int saved_errno;
 memset(created,0,sizeof(created));
 if(ppp_callback_link_directory_ready("/tmp")!=0||
    ppp_options_directory_ready("/tmp/ppp")!=0||
    ppp_options_directory_ready(PPP_PEERS_DIR)!=0){
  saved_errno=errno?errno:EPERM;
  logmessage(LOGNAME,"refusing WAN startup: unsafe shared tmp/PPP callback/peer directory (errno=%d)",saved_errno);
  errno=saved_errno;return -1;
 }
 for(i=0;i<WAN_CALLBACK_LINK_COUNT;++i){
  if(ppp_callback_link_ensure("/sbin/rc",wan_callback_link_paths[i],&created[i])!=0){
   saved_errno=errno?errno:EIO;
   logmessage(LOGNAME,"refusing WAN startup: cannot publish trusted callback link %s (errno=%d)",wan_callback_link_paths[i],saved_errno);
   while(i>0){
    int rollback_errno;--i;
    if(created[i]&&ppp_callback_link_remove("/sbin/rc",wan_callback_link_paths[i])!=0){
     rollback_errno=errno?errno:EIO;
     logmessage(LOGNAME,"cannot roll back callback link %s after startup abort (errno=%d)",wan_callback_link_paths[i],rollback_errno);
    }
   }
   errno=saved_errno;return -1;
  }
 }
 return 0;
}
static int
remove_cb_links(void)
{
 size_t i;int first_error=0;
 for(i=0;i<WAN_CALLBACK_LINK_COUNT;++i){
  if(ppp_callback_link_remove("/sbin/rc",wan_callback_link_paths[i])!=0){
   int remove_errno=errno?errno:EIO;
   if(first_error==0)first_error=remove_errno;
   logmessage(LOGNAME,"cannot safely remove WAN callback link %s (errno=%d)",wan_callback_link_paths[i],remove_errno);
  }
 }
 if(first_error){errno=first_error;return first_error;}
 return 0;
}

int
start_wan(void)
{
	int unit, wan_proto, wan_mtu;
	int vlan_result, mtu_status, interface_status, interface_cleanup_status;
	int cleanup_status;
	int callback_status;
	int static_address_clear_status;
	int wan_start_status = 0, wan_up_status, sysctl_status;
	int tcp_syncookies_enabled = 0;
	int tcp_syncookies_status = 0;
	char *wan_ifname;

	if (wan_vars_reset_status != 0) {
		wan_start_status = wan_vars_reset_status;
		logmessage(LOGNAME,
			   "aborting WAN startup because configured WAN protocol staging failed (status=%d)",
			   wan_start_status);
		return wan_start_status;
	}

	/* Apply explicit SYN-cookie protection before any WAN setup side effects. */
	tcp_syncookies_enabled = nvram_get_int("fw_syn_cook") ? 1 : 0;
	sysctl_status = set_tcp_syncookies(tcp_syncookies_enabled, &tcp_syncookies_status);
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN TCP/ICMP security sysctl configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);
	if (!rc_network_sysctl_tcp_syncookies_allows_wan(
		tcp_syncookies_enabled, tcp_syncookies_status)) {
		logmessage(LOGNAME,
			   "aborting WAN startup: configured TCP SYN cookies could not be applied (status=%d)",
			   tcp_syncookies_status);
		return rc_first_service_start_status(wan_start_status, tcp_syncookies_status);
	}

	/* Secure PPP callback/peer storage before publishing callback links. */
	if (create_cb_links() != 0) {
		callback_status = errno ? errno : EIO;
		wan_start_status = rc_first_service_start_status(
			wan_start_status, callback_status);
		logmessage(LOGNAME, "aborting WAN startup: PPP callback/peer directory setup failed (errno=%d)",
			   callback_status);
		return wan_start_status;
	}

	vlan_result = config_vinet_wan();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "aborting WAN startup: WAN VLAN configuration failed (%d)", vlan_result);
		cleanup_status = remove_cb_links();
		if (cleanup_status != 0)
			logmessage(LOGNAME, "WAN startup callback cleanup incomplete (status=%d)", cleanup_status);
		return wan_start_status;
	}

	/*
	 * Resolve the old physical WAN interface before starting an IPTV
	 * DHCP/ZeroConf client. On migration failure, leave no client running
	 * against an interface whose address/administrative state is unknown.
	 */
	vlan_result = config_apcli_wisp();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "aborting WAN startup: AP-client WISP migration failed (status=%d)",
			   vlan_result);
		cleanup_status = remove_cb_links();
		if (cleanup_status != 0)
			logmessage(LOGNAME, "WAN startup callback cleanup incomplete (status=%d)", cleanup_status);
		return wan_start_status;
	}

	vlan_result = launch_viptv_wan();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "IPTV VLAN or service setup incomplete (error=%d)", vlan_result);
	}

	vlan_result = reload_nat_modules();
	if (vlan_result != 0) {
		wan_start_status = rc_first_service_start_status(
			wan_start_status, vlan_result);
		logmessage(LOGNAME, "NAT helper or acceleration setup incomplete (status=%d); continuing WAN startup",
			   vlan_result);
	}

	refresh_resolvconf_nonfatal("WAN startup", 1);

	sysctl_status = set_nf_conntrack();
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN conntrack limit configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);


	sysctl_status = set_igmp_mld_version();
	if (sysctl_status != 0)
		logmessage(LOGNAME, "WAN IGMP/MLD version sysctl configuration failed (status=%d)",
			   sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);

	/* DI wakeup after 60 secs; retain launch/inventory/signal errors. */
	sysctl_status = notify_run_detect_internet(60);
	if (sysctl_status != 0)
		logmessage(LOGNAME, "Internet monitor startup incomplete (status=%d)", sysctl_status);
	wan_start_status = rc_first_service_start_status(wan_start_status, sysctl_status);

	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < 1; unit++)
	{
		int static_address_cleanup_failed = 0;

		wan_proto = get_wan_proto(unit);
		if (wan_proto < 0)
			continue;
		
		/* make sure the connection exists and is enabled */
		wan_ifname = get_man_ifname(unit);
		if (strlen(wan_ifname) < 1)
			continue;
		
		/* Read the configured MTU before applying the physical default. */
		wan_mtu = get_wan_unit_value_int(unit, "mtu");

		/*
		 * Static IPoE applies its requested MTU later, immediately before
		 * publishing the address. Do not let a redundant 1500-byte request
		 * mask a successful configured MTU or report a false startup error.
		 */
		if (wan_default_mtu_should_be_set(
			wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, wan_mtu)) {
			mtu_status = set_interface_mtu(wan_ifname, 1500);
			if (mtu_status != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, mtu_status);
				logmessage(LOGNAME, "WAN interface %s default MTU setup incomplete (status=%d)",
					   wan_ifname, mtu_status);
			}
		}
		interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		if (interface_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, interface_status);
			logmessage(LOGNAME, "aborting WAN startup for unit %d: failed to bring up physical interface %s (status=%d)",
				   unit, wan_ifname, interface_status);
			interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
			if (interface_cleanup_status != 0)
				logmessage(LOGNAME, "failed to keep physical WAN interface %s down after startup failure (status=%d)",
					   wan_ifname, interface_cleanup_status);
			continue;
		}
		
		/* Perform AP-client reconnect and retain driver command failures. */
		vlan_result = reconnect_apcli(wan_ifname, 0);
		if (vlan_result != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, vlan_result);
			logmessage(LOGNAME, "WAN AP-client reconnect incomplete on %s (status=%d)",
				   wan_ifname, vlan_result);
		}
		
#if defined (USE_USB_SUPPORT)
		if (get_usb_modem_wan(unit))
		{
			if (nvram_get_int("modem_type") == 3)
			{
				vlan_result = launch_wan_usbnet(unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0)
					logmessage(LOGNAME, "USB modem WAN startup incomplete (status=%d)",
						   vlan_result);
			}
			else
			{
				char *ppp_ifname = IFNAME_RAS;
				
				vlan_result = check_upnp_wanif_changed(ppp_ifname);
				if (vlan_result != 0) {
					logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
						   unit, vlan_result);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				set_wan_unit_value(unit, "proto_t", "RAS Modem");
				vlan_result = set_wan_unit_value_status(unit, "ifname_t", ppp_ifname);
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME, "aborting USB modem RAS startup for unit %d: cannot publish active WAN interface %s (status=%d)",
						   unit, ppp_ifname, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				
				if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
				    wan_proto == IPV4_WAN_PROTO_PPTP ||
				    wan_proto == IPV4_WAN_PROTO_L2TP)
				{
					int i_pppoe, i_pppoe_man, wanx_status;
					
					i_pppoe = (wan_proto == IPV4_WAN_PROTO_PPPOE) ? 1 : 0;
					i_pppoe_man = get_wan_unit_value_int(unit, "pppoe_man");
					wanx_status = 0;
					if (!i_pppoe || i_pppoe_man == 1)
						wanx_status = launch_wanx(wan_ifname, unit, 0, 0);
					else if (i_pppoe && i_pppoe_man == 2)
						wanx_status = launch_wanx(wan_ifname, unit, 0, 1);
										if (wanx_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, wanx_status);
						logmessage(LOGNAME, "WAN physical interface setup incomplete on %s (status=%d)",
							wan_ifname, wanx_status);
					}
				}
				
				/* Rebuild required firewall policy before exposing the RAS client. */
				vlan_result = start_firewall_ex();
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0)
					logmessage(LOGNAME, "USB modem RAS firewall startup incomplete (status=%d)",
						   vlan_result);
				if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
					logmessage(LOGNAME, "aborting USB modem RAS startup for unit %d because the firewall rebuild failed (status=%d)",
						   unit, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					continue;
				}
				
				vlan_result = launch_wan_modem_ras(unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				if (vlan_result != 0) {
					logmessage(LOGNAME, "USB modem RAS startup incomplete (status=%d)",
						   vlan_result);
					/*
					 * RAS launch failure leaves no usable PPP client. Restore the
					 * physical WAN binding and firewall before returning to callers.
					 * If recovery itself fails, the helper keeps the interface down.
					 */
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					if (vlan_result != 0)
						logmessage(LOGNAME,
							   "WAN recovery after USB modem RAS launch failure incomplete (status=%d)",
							   vlan_result);
				}
			}
		}
		else
#endif
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP)
		{
			char *proto_desc, *ppp_ifname = IFNAME_PPP;
			int i_pppoe, i_pppoe_man, i_demand, wanx_status;
			
			if (wan_proto == IPV4_WAN_PROTO_PPPOE)
				proto_desc = "PPPoE";
			else if (wan_proto == IPV4_WAN_PROTO_PPTP)
				proto_desc = "PPTP";
			else
				proto_desc = "L2TP";
			
			vlan_result = check_upnp_wanif_changed(ppp_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
					   unit, vlan_result);
				continue;
			}
			set_wan_unit_value(unit, "proto_t", proto_desc);
			vlan_result = set_wan_unit_value_status(unit, "ifname_t", ppp_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d: cannot publish active WAN interface %s (status=%d)",
					   unit, ppp_ifname, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			i_pppoe = (wan_proto == IPV4_WAN_PROTO_PPPOE) ? 1 : 0;
			i_pppoe_man = get_wan_unit_value_int(unit, "pppoe_man");
			
			wanx_status = 0;
			if (!i_pppoe || i_pppoe_man == 1)
				wanx_status = launch_wanx(wan_ifname, unit, !i_pppoe, 0);
			else if (i_pppoe && i_pppoe_man == 2)
				wanx_status = launch_wanx(wan_ifname, unit, 0, 1);
			if (wanx_status != 0) {
				/* PPPoE can start without a physical IPv4 lease. Keep pppd best-effort,
				 * but report failure of the explicitly configured physical WAN path. */
				wan_start_status = rc_first_service_start_status(
					wan_start_status, wanx_status);
				logmessage(LOGNAME, "WAN physical interface setup incomplete on %s (status=%d)",
					   wan_ifname, wanx_status);
			}
			if (!i_pppoe && wanx_status != 0) {
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d: physical WAN setup failed (status=%d)",
					   unit, wanx_status);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
				continue;
			}
			
			/* Rebuild required firewall policy before launching the PPP client. */
			vlan_result = start_firewall_ex();
			wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
			if (vlan_result != 0)
				logmessage(LOGNAME, "WAN PPP firewall startup incomplete (status=%d)", vlan_result);
			if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d because the firewall rebuild failed (status=%d)",
					   unit, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			/* update demand option */
			i_demand = get_wan_unit_value_int(unit, "pppoe_idletime");
			if (!i_pppoe || i_demand < 0)
				i_demand = 0;
			
			if (i_demand)
				i_demand = 1;
			
			set_wan_unit_value_int(unit, "pppoe_demand", i_demand);
			
			/* Launch the PPP client only after the options file is finalized. */
			vlan_result = launch_wan_pppd(unit, wan_proto);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN PPP startup for unit %d after launch failure (status=%d)",
					   unit, vlan_result);
				/* Do not leave a phantom ppp0 interface in firewall/UPnP state. */
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
				continue;
			}

			/* Pretend that the WAN interface is up */
			if (i_demand) {
				vlan_result = wait_ppp_up(ppp_ifname, unit);
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME, "aborting WAN PPP startup for unit %d because the demand interface or route setup is incomplete (status=%d)",
						   unit, vlan_result);
					vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
					wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
					continue;
				}
			}
		}
		else if (wan_proto == IPV4_WAN_PROTO_IPOE_DHCP ||
			 wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)
		{
			int wan_auth_mode, wan_err = 0, wan_upnp_status, auth_status, dhcp_status;
			int firewall_cleanup_status;
			
			/* Configure DHCP connection. */
			vlan_result = check_upnp_wanif_changed(wan_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting WAN startup for unit %d: miniupnpd did not stop before interface migration (status=%d)",
					   unit, vlan_result);
				continue;
			}
			set_wan_unit_value(unit, "proto_t", "IPoE");
			vlan_result = set_wan_unit_value_status(unit, "ifname_t", wan_ifname);
			if (vlan_result != 0) {
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				logmessage(LOGNAME, "aborting IPoE WAN startup for unit %d: cannot publish active WAN interface %s (status=%d)",
					   unit, wan_ifname, vlan_result);
				vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, vlan_result);
				continue;
			}
			
			/* Do not publish static/DHCP WAN state until WISP association is ready. */
			if (is_man_wisp(wan_ifname)) {
				vlan_result = rc_wan_apcli_wait_status(
					wait_apcli_connected(wan_ifname, 3));
				if (vlan_result != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, vlan_result);
					logmessage(LOGNAME,
						   "aborting IPoE WAN activation because AP-client interface %s did not associate (status=%d)",
						   wan_ifname, vlan_result);
					set_wan_unit_value_int(unit, "err", 1);
					continue;
				}
			}
			
			/* Assign static IP address to i/f */
			if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
				int mtu_status, address_status, clear_status;
				char *wan_addr = get_wan_unit_value(unit, "ipaddr");
				char *wan_mask = get_wan_unit_value(unit, "netmask");
				char *lan_addr = nvram_safe_get("lan_ipaddr");
				char *lan_mask = nvram_safe_get("lan_netmask");
				
				if (!dhcp_lease_ipv4_is_valid(wan_addr) ||
				    !dhcp_lease_netmask_is_valid(wan_mask)) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, EINVAL);
					logmessage(LOGNAME, "invalid static WAN IPv4 address or netmask on %s",
						   wan_ifname);
				} else if (is_same_subnet2(wan_addr, lan_addr, wan_mask, lan_mask)) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, EADDRINUSE);
					logmessage(LOGNAME, "%s (%s) ERROR! Static IP [%s/%s] in LAN subnet [%s/%s]!",
						"WAN", wan_ifname, wan_addr, wan_mask, lan_addr, lan_mask);
				} else {
					if (wan_mtu >= 1300 && wan_mtu < 1500) {
						mtu_status = set_interface_mtu(wan_ifname, wan_mtu);
						if (mtu_status != 0) {
							wan_err = 1;
							wan_start_status = rc_first_service_start_status(
								wan_start_status, mtu_status);
							logmessage(LOGNAME, "static WAN interface %s MTU setup failed (status=%d); skipping address and route setup",
								   wan_ifname, mtu_status);
						}
					}
					
					/* Do not publish an address or route using a requested MTU we failed to apply. */
					if (!wan_err) {
						address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);
						if (address_status != 0) {
							wan_err = 1;
							wan_start_status = rc_first_service_start_status(
								wan_start_status, address_status);
							logmessage(LOGNAME, "failed to configure static WAN address on %s (status=%d)",
								   wan_ifname, address_status);
							clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
							if (rc_wan_static_ipoe_clear_failure_requires_ifdown(
								wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, clear_status)) {
								static_address_cleanup_failed = 1;
								wan_start_status = rc_first_service_start_status(
									wan_start_status, clear_status);
								logmessage(LOGNAME, "failed to clear partially configured static WAN address on %s (status=%d); forcing interface down",
									   wan_ifname, clear_status);
								interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
								if (interface_cleanup_status != 0) {
									wan_start_status = rc_first_service_start_status(
										wan_start_status, interface_cleanup_status);
									logmessage(LOGNAME, "failed to take static WAN interface %s down after partial address cleanup failure (status=%d)",
										   wan_ifname, interface_cleanup_status);
								}
							}
						}
					}
				}
				
				set_wan_unit_value_int(unit, "err", wan_err);
			}
			
			/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */
			vlan_result = start_firewall_ex();
			wan_start_status = rc_first_service_start_status(wan_start_status, vlan_result);
			if (vlan_result != 0)
				logmessage(LOGNAME, "WAN IPoE firewall startup incomplete (status=%d)", vlan_result);
			if (!rc_wan_firewall_start_allows_protocol(vlan_result)) {
				wan_err = 1;
				logmessage(LOGNAME, "aborting WAN IPoE protocol startup on %s because the firewall rebuild failed (status=%d)",
					   wan_ifname, vlan_result);
				if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
					firewall_cleanup_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
					if (firewall_cleanup_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, firewall_cleanup_status);
						logmessage(LOGNAME, "failed to clear static WAN address after firewall failure on %s (status=%d); forcing interface down",
							   wan_ifname, firewall_cleanup_status);
						interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
						if (interface_cleanup_status != 0) {
							wan_start_status = rc_first_service_start_status(
								wan_start_status, interface_cleanup_status);
							logmessage(LOGNAME, "failed to take static WAN interface %s down after firewall failure (status=%d)",
								   wan_ifname, interface_cleanup_status);
						}
					}
				}
				set_wan_unit_value_int(unit, "err", 1);
				continue;
			}
			
			if (static_address_cleanup_failed) {
				logmessage(LOGNAME, "skipping IPoE protocol activation after static address cleanup failure on %s",
					   wan_ifname);
				continue;
			}

			/* Start eapol authenticator */
			wan_auth_mode = get_wan_unit_value_int(unit, "auth_mode");
			if (wan_auth_mode > 1) {
				auth_status = start_auth_eapol(wan_ifname, unit, wan_auth_mode - 2);
				if (auth_status != 0) {
					wan_start_status = rc_first_service_start_status(
						wan_start_status, auth_status);
					logmessage(LOGNAME, "WAN EAPOL authentication startup failed on %s (status=%d); skipping address/lease setup",
						   wan_ifname, auth_status);
					set_wan_unit_value_int(unit, "err", 1);
					static_address_clear_status = 0;
					if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)
						static_address_clear_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
					if (rc_wan_static_ipoe_clear_failure_requires_ifdown(
						wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, static_address_clear_status)) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, static_address_clear_status);
						logmessage(LOGNAME, "failed to clear static WAN address after EAPOL startup failure on %s (status=%d); forcing interface down",
							   wan_ifname, static_address_clear_status);
						interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
						if (interface_cleanup_status != 0) {
							wan_start_status = rc_first_service_start_status(
								wan_start_status, interface_cleanup_status);
							logmessage(LOGNAME, "failed to take static WAN interface %s down after EAPOL address cleanup failure (status=%d)",
								   wan_ifname, interface_cleanup_status);
						}
					}
					continue;
				}
			}
			
			/* We are done configuration */
			if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
				if (!wan_err) {
					wan_up_status = wan_up(wan_ifname, unit, 1);
					wan_start_status = rc_first_service_start_status(
						wan_start_status, wan_up_status);
					if (wan_up_status != 0)
						logmessage(LOGNAME, "static WAN route/service setup incomplete on %s (status=%d)",
							   wan_ifname, wan_up_status);
					/* Update UPnP forwards after WAN setup; do not hide reload failure. */
					wan_upnp_status = update_upnp();
					if (wan_upnp_status != 0) {
						wan_start_status = rc_first_service_start_status(
							wan_start_status, wan_upnp_status);
						logmessage(LOGNAME, "UPnP update incomplete after WAN startup (status=%d)", wan_upnp_status);
					}
				}
			} else {
				dhcp_status = start_udhcpc_wan(wan_ifname, unit, 0);
				if (dhcp_status != 0) {
					wan_err = 1;
					wan_start_status = rc_first_service_start_status(
						wan_start_status, dhcp_status);
					logmessage(LOGNAME, "WAN DHCP client launch failed on %s (status=%d)",
						   wan_ifname, dhcp_status);
				}
				set_wan_unit_value_int(unit, "err", wan_err);
			}
#if defined (USE_IPV6)
			if (is_wan_ipv6_type_sit() == 0) {
				int ipv6_status = wan6_up(wan_ifname, unit);
				wan_start_status = rc_first_service_start_status(
					wan_start_status, ipv6_status);
				if (ipv6_status != 0)
					logmessage(LOGNAME, "WAN IPv6 setup incomplete on %s (status=%d)",
						   wan_ifname, ipv6_status);
			}
#endif
		}
	}

	{
		int passthrough_status = set_passthrough_pppoe(1);
		if (passthrough_status != 0) {
			wan_start_status = rc_first_service_start_status(
				wan_start_status, passthrough_status);
			logmessage(LOGNAME, "PPPoE passthrough startup incomplete (status=%d)",
				   passthrough_status);
		}
	}

	return wan_start_status;
}

static int
stop_wan_ppp_locked(void)
{
	char* svcs_ppp[] = {
		"ip-up", "ip-down",
#if defined (USE_IPV6)
		"ipv6-up", "ipv6-down",
#endif
		"l2tpd", "xl2tpd", "pppd", NULL
	};
	int pause_status, stop_status = 0, resume_status;

	/* A running monitor must be paused before fencing or stopping PPP. */
	pause_status = notify_pause_detect_internet();
	if (pause_status != 0) {
		logmessage(LOGNAME,
			   "refusing to stop WAN PPP because Internet monitor pause failed (status=%d)",
			   pause_status);
		return pause_status;
	}
	stop_status = invalidate_wan_ppp_callback_session();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing to stop WAN PPP because callback-session fencing failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	nvram_set_int_temp("deferred_wanup_t", 0);
	stop_status = stop_vpn_client_under_lifecycle_lock();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN PPP teardown because VPN client stop was incomplete (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	stop_status = kill_services(svcs_ppp, 6, 1);
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing to clear WAN PPP state while PPP services remain active (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	clear_wan_state();
	control_wan_led_isp_state(0, 0);
	return 0;

abort_resume_detect:
	resume_status = notify_run_detect_internet(2);
	if (resume_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor resume failed after aborted WAN PPP teardown (status=%d)",
			   resume_status);
	return wan_teardown_abort_status(stop_status, resume_status);
}

static int
wan_teardown_ifdown_interface(void *context, const char *ifname)
{
	int ret;

	(void)context;
	ret = ifconfig(ifname, 0, "0.0.0.0", NULL);
	if (ret == ENODEV)
		return 0; /* The interface disappeared and is already unavailable. */
	if (ret != 0)
		logmessage(LOGNAME, "failed to bring WAN interface %s down (status=%d)", ifname, ret);

	return ret;
}

static const struct wan_teardown_interface_ops wan_teardown_ifdown_ops = {
	wan_teardown_ifdown_interface
};

static int
stop_wan_locked(void)
{
	int unit, wan_proto, ifdown_status, stop_status = -1;
	int callback_cleanup_status;
	int pause_status, eapol_stop_status, kabinet_stop_status, resume_status;
#if defined (USE_IPV6)
	int ipv6_status = 0;
#endif
	char *man_ifname;
	char *usbnet_ifname;
	char* svcs_ppp[] = {
		"ip-up", "ip-down",
#if defined (USE_IPV6)
		"ipv6-up", "ipv6-down",
#endif
		"l2tpd", "xl2tpd", "pppd", NULL
	};
	char* svcs_wan[] = {
		"ntpd", "inadyn", "igmpproxy", "xupnpd", "udpxy", "msd_lite",
		"zcip", "pppoe-relay", "detect_wan", NULL
	};
	const char *udhcpc_pidfiles[] = {
		"/var/run/udhcpc0.pid", "/var/run/udhcpc_viptv.pid", NULL
	};

	unit = 0;
	pause_status = notify_pause_detect_internet();
	if (pause_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because Internet monitor pause failed (status=%d)",
			   pause_status);
		return pause_status;
	}
	stop_status = invalidate_wan_ppp_callback_session();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN teardown because PPP callback-session fencing failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	nvram_set_int_temp("deferred_wanup_t", 0);
	wan_proto = get_wan_proto(unit);
	man_ifname = get_man_ifname(unit);
	usbnet_ifname = NULL;
	stop_status = stop_vpn_client_under_lifecycle_lock();
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN PPP teardown because VPN client stop was incomplete (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}

	stop_status = kill_services(svcs_ppp, 6, 1);
	if (stop_status != 0) {
		logmessage(LOGNAME,
			   "refusing WAN interface teardown while PPP services remain active (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() != 1) {
		ipv6_status = wan6_down(man_ifname, unit);
		if (ipv6_status != 0) {
			logmessage(LOGNAME, "WAN IPv6 teardown incomplete on %s (status=%d)",
				   man_ifname, ipv6_status);
			logmessage(LOGNAME, "refusing WAN interface teardown while DHCPv6 callback fencing is incomplete");
			stop_status = ipv6_status;
			goto abort_resume_detect;
		}
	}
#endif
	/* Release only WAN/VIPTV DHCP clients; never signal the LAN DHCP client. */
	if (process_pidfile_signal(udhcpc_pidfiles[0], "udhcpc", SIGUSR2) != 0 &&
	    errno != ENOENT && errno != ESRCH) {
		logmessage(LOGNAME, "refusing WAN teardown: WAN DHCP PID identity is unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (process_pidfile_signal(udhcpc_pidfiles[1], "udhcpc", SIGUSR2) != 0 &&
	    errno != ENOENT && errno != ESRCH) {
		logmessage(LOGNAME, "refusing WAN teardown: VIPTV DHCP PID identity is unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (service_wait_microseconds(300000U) != 0) {
		stop_status = errno > 0 ? errno : EIO;
		logmessage(LOGNAME, "refusing WAN teardown: DHCP deconfig grace wait failed (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	/*
	 * udhcpc owns retries for a failed deconfig callback (default -A is 20s).
	 * Never SIGKILL it: wait once for its callback retry and abort teardown
	 * on timeout. Stop unrelated WAN daemons separately so they retain the
	 * normal bounded TERM/KILL policy.
	 */
	if (kill_services_with_pidfiles(NULL, udhcpc_pidfiles, "udhcpc", 0, 30, 0) != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown while DHCP deconfig is active or unknown");
		stop_status = -1;
		goto abort_resume_detect;
	}
	if (kill_services(svcs_wan, 3, 1) != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown while WAN services remain active");
		stop_status = -1;
		goto abort_resume_detect;
	}

	eapol_stop_status = stop_auth_eapol();
	if (eapol_stop_status != 0)
		logmessage(LOGNAME, "WAN teardown could not stop EAPOL authentication (status=%d)", eapol_stop_status);
	kabinet_stop_status = stop_auth_kabinet();
	if (kabinet_stop_status != 0)
		logmessage(LOGNAME, "WAN teardown could not stop kabinet authentication (status=%d)", kabinet_stop_status);
	if (eapol_stop_status != 0 || kabinet_stop_status != 0) {
		stop_status = eapol_stop_status != 0 ? eapol_stop_status : kabinet_stop_status;
		goto abort_resume_detect;
	}
	stop_status = set_passthrough_pppoe(0);
	if (stop_status != 0) {
		logmessage(LOGNAME, "refusing WAN interface teardown because PPPoE relay did not stop (status=%d)",
			   stop_status);
		goto abort_resume_detect;
	}
	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC &&
	    strcmp(man_ifname, get_wan_unit_value(unit, "ifname_t")) == 0) {
		stop_status = wan_down(man_ifname, unit, 1);
		if (stop_status != 0)
			logmessage(LOGNAME, "static WAN route teardown incomplete on %s (status=%d)",
				   man_ifname, stop_status);
	}
#if defined (USE_USB_SUPPORT)
	if (get_usb_modem_wan(unit) && nvram_get_int("modem_type") == 3)
		usbnet_ifname = get_wan_unit_value(unit, "ifname_t");
	{
		int modem_stop_status = stop_wan_usbnet();
		stop_status = rc_first_service_start_status(stop_status, modem_stop_status);
	}
#endif
	ifdown_status = wan_teardown_ifdown_interfaces(man_ifname, IFNAME_WAN,
						       usbnet_ifname,
						       &wan_teardown_ifdown_ops, NULL);
	callback_cleanup_status = remove_cb_links();
	if (callback_cleanup_status != 0)
		logmessage(LOGNAME, "WAN callback link cleanup incomplete (status=%d)", callback_cleanup_status);
	if (stop_status != 0 || ifdown_status != 0 || callback_cleanup_status != 0) {
		logmessage(LOGNAME,
			   "WAN interface shutdown incomplete (modem=%d interface=%d callbacks=%d); skipping final WAN state clear",
			   stop_status, ifdown_status, callback_cleanup_status);
		if (stop_status == 0)
			stop_status = ifdown_status != 0 ? ifdown_status : callback_cleanup_status;
		goto abort_resume_detect;
	}
#if defined (USE_IPV6)
	if (ipv6_status != 0) {
		logmessage(LOGNAME, "WAN IPv6 teardown incomplete; skipping final WAN state clear");
		stop_status = ipv6_status;
		goto abort_resume_detect;
	}
#endif
	clear_wan_state();
	control_wan_led_isp_state(0, 0);
	return 0;

abort_resume_detect:
	/* Every failure exits before a confirmed complete teardown; restore the
	 * monitor so it can observe or recover the still-live/partial WAN. */
	resume_status = notify_run_detect_internet(2);
	if (resume_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor resume failed after aborted WAN teardown (status=%d)",
			   resume_status);
	return wan_teardown_abort_status(stop_status, resume_status);
}

static int
run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)
{
	int lock_fd, result, saved_errno;

	lock_fd = vpn_server_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		logmessage(LOGNAME, "refusing %s: cannot lock VPN server lifecycle (errno=%d)",
			   name, errno);
		return -1;
	}
	result = operation();
	saved_errno = errno;
	if (vpn_server_lifecycle_lock_release(lock_fd) != 0) {
		int release_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release VPN server lifecycle lock after %s (errno=%d)",
			   name, release_errno);
		if (result == 0) {
			result = -1;
			saved_errno = release_errno;
		}
	}
	if (result != 0 && saved_errno)
		errno = saved_errno;
	return result;
}

static int
stop_wan_ppp(void)
{
	return run_wan_stop_under_vpn_lifecycle_lock(stop_wan_ppp_locked, "WAN PPP stop");
}

int
stop_wan(void)
{
	return run_wan_stop_under_vpn_lifecycle_lock(stop_wan_locked, "WAN stop");
}

static int
get_wan_unit(char *wan_ifname)
{
	int unit;

	for (unit = 0; unit < 1; unit++) {
		if (strcmp(wan_ifname, get_wan_unit_value(unit, "ifname_t")) == 0)
			return unit;
		if (strcmp(wan_ifname, get_man_ifname(unit)) == 0)
			return unit;
	}

	/* fallback to unit 0 */
	return 0;
}

static int
is_ifunit_man(char *wan_ifname, int unit)
{
	int wan_proto = get_wan_proto(unit);

	if (!is_usbnet_interface(wan_ifname) &&
	    (wan_proto == IPV4_WAN_PROTO_PPPOE ||
	     wan_proto == IPV4_WAN_PROTO_PPTP ||
	     wan_proto == IPV4_WAN_PROTO_L2TP))
		return 1;

	return 0;
}

static int
is_ifunit_modem(char *wan_ifname, int unit)
{
	char *ifname_temp;

	if (!get_usb_modem_wan(unit))
		return 0;

	if (ppp_ifindex(wan_ifname) >= RAS_PPP_UNIT)
		return 1;

	ifname_temp = get_wan_unit_value(unit, "ifname_t");
	if (is_usbnet_interface(ifname_temp) && strcmp(wan_ifname, ifname_temp) == 0)
		return 2;

	return 0;
}

static int
add_man_gateway_routes(char *man_ifname, int unit, int metric, int *gateway_route_status)
{
	int is_man_dns_auto, is_modem_active, wan_proto;
	int status = 0, route_status;
	char *next, *man_gate, *man_addr, *man_mask, *man_dns;
	char word[100];

	if (gateway_route_status)
		*gateway_route_status = 0;

	man_gate = nvram_safe_get("wanx_gateway");
	if (!is_valid_ipv4(man_gate))
		return 0;

	wan_proto = get_wan_proto(unit);
	is_modem_active = get_usb_modem_wan(unit);
	is_man_dns_auto = (get_wan_unit_value_int(unit, "dnsenable_x") > 0) ? 1 : 0;

	man_addr = nvram_safe_get("wanx_ipaddr");
	man_mask = nvram_safe_get("wanx_netmask");
	man_dns  = nvram_safe_get("wanx_dns");

	/*
	 * Do not blindly roll back successful additions after a later failure:
	 * the kernel route operation cannot distinguish a route created here
	 * from an identical route already owned by another path. Continue all
	 * attempts, report the first error, and let matching teardown remove the
	 * scoped route set.
	 */
	/* if the gateway is out of the subnet */
	if (!is_same_subnet(man_gate, man_addr, man_mask)) {
		route_status = route_add(man_ifname, metric, man_gate, NULL, "255.255.255.255");
		status = route_status_accumulate(status, route_status);
		if (gateway_route_status)
			*gateway_route_status = route_status_accumulate(*gateway_route_status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add MAN gateway host route %s on %s (error=%d)",
				   man_gate, man_ifname, route_status);
	}

	/* default route via default gateway */
	route_status = route_add(man_ifname, metric, "0.0.0.0", man_gate, "0.0.0.0");
	status = route_status_accumulate(status, route_status);
	if (gateway_route_status)
		*gateway_route_status = route_status_accumulate(*gateway_route_status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add MAN default route via %s on %s (error=%d)",
			   man_gate, man_ifname, route_status);

	/* ... and to dns servers as well for demand ppp to work */
	if (is_man_dns_auto && !is_modem_active && wan_proto != IPV4_WAN_PROTO_PPPOE) {
		foreach(word, man_dns, next) {
			if (is_valid_ipv4(word) && inet_addr(word) != inet_addr(man_gate) &&
			    !is_same_subnet(word, man_addr, man_mask)) {
				route_status = route_add(man_ifname, metric, word, man_gate, "255.255.255.255");
				status = route_status_accumulate(status, route_status);
				if (route_status != 0)
					logmessage(LOGNAME, "failed to add MAN DNS route %s via %s on %s (error=%d)",
						   word, man_gate, man_ifname, route_status);
			}
		}
	}

	return status;
}

static int
queue_wan_firewall_restart(const char *context)
{
	int status = notify_rc_status("restart_firewall_wan");

	if (status != 0)
		logmessage(LOGNAME,
			   "could not queue WAN firewall refresh after %s (errno=%d)",
			   context, errno ? errno : EIO);
	return status;
}

static int
man_up_internal(char *man_ifname, int unit, int is_static, int *gateway_route_status)
{
	int status = 0, route_status;

	if (gateway_route_status)
		*gateway_route_status = 0;

	logmessage(LOGNAME, "%s %s (%s)", "MAN", "up", man_ifname);

	/* setup static wan routes via physical device */
	route_status = add_static_man_routes(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add static MAN routes on %s (error=%d)",
			   man_ifname, route_status);

	if (!is_static) {
		/* and routes supplied via DHCP */
		route_status = add_dhcp_routes_by_prefix("wanx_", man_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add DHCP MAN routes on %s (error=%d)",
				   man_ifname, route_status);
	}

	/* and default route (metric 2) */
	route_status = add_man_gateway_routes(man_ifname, unit, 2, gateway_route_status);
	status = route_status_accumulate(status, route_status);

	if (!is_static) {
		/* A resolver refresh error must not tear down this live link. */
		refresh_resolvconf_nonfatal("MAN up", 0);

		/* re-start firewall */
		route_status = queue_wan_firewall_restart("MAN up");
		status = route_status_accumulate(status, route_status);
	}

	/* Start multicast services and preserve their status for DHCP callers. */
	route_status = start_igmpproxy(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "MAN multicast startup incomplete on %s (error=%d)",
			   man_ifname, route_status);

	return status;
}

void
man_up(char *man_ifname, int unit, int is_static)
{
	int status = man_up_internal(man_ifname, unit, is_static, NULL);

	if (status != 0)
		logmessage(LOGNAME, "MAN route setup incomplete on %s (error=%d)",
			   man_ifname, status);
}

static int
man_down_internal(char *man_ifname, int unit, int clear_dhcp_address)
{
	int wan_proto, is_modem_active, is_man_dns_auto;
	int status = 0, route_status;
	char *next, *man_gate, *man_addr, *man_mask, *man_dns;
	char word[100];

	static const struct rc_wan_nvram_value clear_address_values[] = {
		{ "wanx_ipaddr", "0.0.0.0" }
	};
	static const struct rc_wan_nvram_value clear_runtime_values[] = {
		{ "wanx_gateway", "" }, { "wanx_dns", "" },
		{ "wanx_domain", "" }, { "wanx_routes", "" },
		{ "wanx_routes_ms", "" }, { "wanx_routes_rfc", "" }
	};


	logmessage(LOGNAME, "%s %s (%s)", "MAN", "down", man_ifname);
	/*
	 * DHCP loss is already committed, so a monitor-pause failure must not
	 * prevent route/interface cleanup. Retain it as the callback's first
	 * error while continuing the idempotent teardown sequence.
	 */
	route_status = notify_pause_detect_internet();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor pause incomplete during MAN teardown (status=%d)",
			   route_status);
	man_gate = nvram_safe_get("wanx_gateway");
	man_addr = nvram_safe_get("wanx_ipaddr");
	man_mask = nvram_safe_get("wanx_netmask");
	man_dns = nvram_safe_get("wanx_dns");
	wan_proto = get_wan_proto(unit);
	is_modem_active = get_usb_modem_wan(unit);
	is_man_dns_auto = (get_wan_unit_value_int(unit, "dnsenable_x") > 0) ? 1 : 0;

	if (dhcp_lease_ipv4_is_valid(man_gate)) {
		if (dhcp_lease_ipv4_is_valid(man_addr) && dhcp_lease_netmask_is_valid(man_mask) &&
		    !is_same_subnet(man_gate, man_addr, man_mask)) {
			route_status = route_del(man_ifname, 2, man_gate, NULL, "255.255.255.255");
			if (!dhcp_lease_route_delete_is_ok(route_status)) {
				status = route_status_accumulate(status, route_status);
				logmessage(LOGNAME, "failed to delete MAN gateway host route %s on %s (error=%d)", man_gate, man_ifname, route_status);
			}
		}
		route_status = route_del(man_ifname, 2, "0.0.0.0", man_gate, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_status)) {
			status = route_status_accumulate(status, route_status);
			logmessage(LOGNAME, "failed to delete MAN default route via %s on %s (error=%d)", man_gate, man_ifname, route_status);
		}
		if (is_man_dns_auto && !is_modem_active && wan_proto != IPV4_WAN_PROTO_PPPOE) {
			foreach(word, man_dns, next) {
				if (is_valid_ipv4(word) && inet_addr(word) != inet_addr(man_gate) &&
				    dhcp_lease_ipv4_is_valid(man_addr) && dhcp_lease_netmask_is_valid(man_mask) &&
				    !is_same_subnet(word, man_addr, man_mask)) {
					route_status = route_del(man_ifname, 2, word, man_gate, "255.255.255.255");
					if (!dhcp_lease_route_delete_is_ok(route_status)) {
						status = route_status_accumulate(status, route_status);
						logmessage(LOGNAME, "failed to delete MAN DNS route %s via %s on %s (error=%d)", word, man_gate, man_ifname, route_status);
					}
				}
			}
		}
	}

	route_status = del_dhcp_routes_by_prefix("wanx_", man_ifname, 0);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete DHCP MAN routes on %s (error=%d)", man_ifname, route_status);
	route_status = del_static_man_routes(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete static MAN routes on %s (error=%d)", man_ifname, route_status);

	if (status == 0) {
		if (clear_dhcp_address) {
			route_status = rc_wan_nvram_set_all_values(
				clear_address_values,
				(unsigned int)(sizeof(clear_address_values) /
					       sizeof(clear_address_values[0])),
				set_nvram_snapshot_value_status, NULL);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME,
					   "failed to clear MAN DHCP address snapshot (status=%d)",
					   route_status);
		}
		/* Attempt every independent clear even if the address write failed. */
		route_status = rc_wan_nvram_set_all_values(
			clear_runtime_values,
			(unsigned int)(sizeof(clear_runtime_values) /
				       sizeof(clear_runtime_values[0])),
			set_nvram_snapshot_value_status, NULL);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME,
				   "MAN runtime DNS/route snapshot clear incomplete (status=%d)",
				   route_status);
	} else {
		logmessage(LOGNAME, "MAN route teardown incomplete on %s (error=%d); preserving lease and route inventory", man_ifname, status);
	}

	refresh_resolvconf_nonfatal("MAN down", 0);
	route_status = queue_wan_firewall_restart("MAN down");
	status = route_status_accumulate(status, route_status);
	route_status = stop_igmpproxy(man_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "MAN multicast teardown incomplete on %s (error=%d)",
			   man_ifname, route_status);
	return status;
}

void
man_down(char *man_ifname, int unit)
{
	int status = man_down_internal(man_ifname, unit, 0);

	if (status != 0)
		logmessage(LOGNAME, "MAN route teardown incomplete on %s (error=%d)",
			   man_ifname, status);
}

static int
wan_up_internal(char *wan_ifname, int unit, int is_static)
{
	char wan_cnt[32], *wan_addr, *wan_mask, *wan_gate;
	const char *script_postw = SCRIPT_POST_WAN;
	int wan_proto, modem_unit_id;
	int ppp_index, gateway_valid, gateway_off_subnet;
	int status = 0, route_status;
#if defined (USE_IPV6)
	int ipv6_status;
#endif

	logmessage(LOGNAME, "%s %s (%s)", "WAN", "up", wan_ifname);

	wan_proto = get_wan_proto(unit);
	modem_unit_id = is_ifunit_modem(wan_ifname, unit);

	snprintf(wan_cnt, sizeof(wan_cnt), "%ld", uptime());
	set_wan_unit_value(unit, "uptime", wan_cnt);

	if (modem_unit_id || wan_proto != IPV4_WAN_PROTO_IPOE_STATIC) {
		snprintf(wan_cnt, sizeof(wan_cnt), "%llu", get_ifstats_bytes_rx(wan_ifname));
		set_wan_unit_value(unit, "bytes_rx", wan_cnt);
		
		snprintf(wan_cnt, sizeof(wan_cnt), "%llu", get_ifstats_bytes_tx(wan_ifname));
		set_wan_unit_value(unit, "bytes_tx", wan_cnt);
	}

	wan_addr = get_wan_unit_value(unit, "ipaddr");
	wan_mask = get_wan_unit_value(unit, "netmask");
	wan_gate = get_wan_unit_value(unit, "gateway");

	gateway_valid = is_valid_ipv4(wan_gate);
	if (!gateway_valid)
		wan_gate = NULL;
	ppp_index = ppp_ifindex(wan_ifname);
	gateway_off_subnet = gateway_valid && ppp_index < 0 &&
		!is_same_subnet(wan_gate, wan_addr, wan_mask);

	/* Set the same off-subnet gateway host route that WAN teardown removes. */
	if (route_status_wan_gateway_host_route_required(
		    gateway_valid, ppp_index, gateway_off_subnet)) {
		route_status = route_add(wan_ifname, 1, wan_gate, NULL, "255.255.255.255");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add WAN gateway host route %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
	}

	/* default route via default gateway */
	if (wan_gate) {
		route_status = route_add(wan_ifname, 1, "0.0.0.0", wan_gate, "0.0.0.0");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add WAN default route via %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
	}

#if 0
	/* hack: avoid routing cycles, when both peer and server has the same IP (for PPTP or L2TP) */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_PPTP || wan_proto == IPV4_WAN_PROTO_L2TP)) {
		/* delete gateway route as it's no longer needed */
		if (wan_gate)
			route_del(wan_ifname, 0, wan_gate, "0.0.0.0", "255.255.255.255");
	}
#endif

	/* Install interface dependent static routes. Keep attempting later setup
	 * steps after an individual route failure and preserve the first status. */
	route_status = add_static_wan_routes(wan_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to add static WAN routes on %s (error=%d)",
			   wan_ifname, route_status);

	/* Publish the global MAN gateway before static routes use it as a fallback. */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		route_status = set_nvram_snapshot_value_status(
			NULL, "wanx_gateway", (wan_gate) ? wan_gate : "0.0.0.0");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0) {
			logmessage(LOGNAME,
				   "failed to publish MAN gateway snapshot; skipping dependent static MAN routes (status=%d)",
				   route_status);
		} else {
			route_status = add_static_man_routes(wan_ifname);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "failed to add static MAN routes on %s (error=%d)",
					   wan_ifname, route_status);
		}
	}

	/* Add dynamic routes supplied via DHCP (for NDIS or IPoE Auto) */
	if ((modem_unit_id == 2) || (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		char prefix[16];
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		route_status = add_dhcp_routes_by_prefix(prefix, wan_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to add DHCP WAN routes on %s (error=%d)",
				   wan_ifname, route_status);
	}

#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 1) {
		ipv6_status = wan6_up(wan_ifname, unit);
		status = route_status_accumulate(status, ipv6_status);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN IPv6 tunnel setup incomplete on %s (status=%d)",
				   wan_ifname, ipv6_status);
	}
#endif

	/* Resolver refresh failure is non-fatal to the established WAN link. */
	refresh_resolvconf_nonfatal("WAN up", 0);

	if (!is_static) {
		/* re-start firewall */
		route_status = queue_wan_firewall_restart("WAN up");
		status = route_status_accumulate(status, route_status);
	}

	/* Start kabinet authenticator (for IPoE) */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		if (get_wan_unit_value_int(unit, "auth_mode") == 1) {
			route_status = start_auth_kabinet();
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "WAN kabinet authentication startup incomplete (status=%d)", route_status);
		}
	}

	/* Start multicast router (for NDIS or IPoE) and retain its status. */
	if (ppp_ifindex(wan_ifname) < 0) {
		route_status = start_igmpproxy(wan_ifname);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN multicast startup incomplete on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Notify the DDNS client only when configured, and preserve any failure. */
	route_status = notify_ddns_update();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN DDNS client update incomplete on %s (status=%d)",
			   wan_ifname, route_status);

	/* Notify watchdog for NTP update and retain signalling/recovery errors. */
	route_status = notify_watchdog_time();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN watchdog time notification incomplete (status=%d)", route_status);

	/* Queue the deferred VPN-client startup and preserve notification errors. */
	route_status = notify_rc_status("start_vpn_client");
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN VPN client start notification incomplete (status=%d)",
			   route_status);

	/* Start the gateway ARP checker only after a complete ABSENT scan. */
	if (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP &&
	    nvram_match("gw_arp_ping", "1")) {
		struct process_snapshot snapshot;
		enum process_presence presence;
		enum process_presence_start_action action;
		int scan_error;

		presence = process_inventory_scan("detect_wan", &snapshot);
		scan_error = snapshot.error;
		process_snapshot_release(&snapshot);
		action = process_presence_start_action_for_state(presence);
		route_status = 0;
		if (action == PROCESS_PRESENCE_START_NEW)
			route_status = eval("detect_wan");
		route_status = process_presence_start_status_for_action(
			action, scan_error, route_status);
		if (route_status != 0) {
			status = route_status_accumulate(status, route_status);
			if (action == PROCESS_PRESENCE_START_ABORT)
				logmessage(LOGNAME,
					   "skipping gateway ARP checker start because process inventory is unknown (status=%d)",
					   route_status);
			else
				logmessage(LOGNAME, "WAN gateway ARP checker startup incomplete (status=%d)",
					   route_status);
		}
	}

	if (wan_gate)
		control_wan_led_isp_state(1, modem_unit_id);

	/* DI wakeup after 2 secs; retain launch/inventory/signal errors. */
	route_status = notify_run_detect_internet(2);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "WAN Internet monitor notification incomplete (status=%d)", route_status);

	/* Call the optional post-WAN hook and preserve its exit status. */
	if (check_if_file_exist(script_postw)) {
		route_status = eval(script_postw, "up", wan_ifname, wan_addr);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN post-up script incomplete on %s (status=%d)",
				   wan_ifname, route_status);
	}
#if defined(APP_ZEROTIER)
	if (nvram_match("zerotier_enable", "1")) {
		route_status = start_zerotier();
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN ZeroTier startup incomplete (status=%d)",
				   route_status);
	}
#endif
	if (nvram_match("pppoemwan_enable", "1")) {
		route_status = eval("/usr/bin/detect.sh");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN PPPoE-MWAN detection incomplete (status=%d)",
				   route_status);
	}
	return status;
}

int
wan_up(char *wan_ifname, int unit, int is_static)
{
	int status = wan_up_internal(wan_ifname, unit, is_static);

	if (status != 0)
		logmessage(LOGNAME, "WAN route setup incomplete on %s (error=%d)",
			   wan_ifname, status);
	return status;
}

static int
wan_down_internal(char *wan_ifname, int unit, int is_static)
{
	char *wan_addr, *wan_gate, *wan_route_addr, *wan_mask;
	const char *script_postw = SCRIPT_POST_WAN;
	int wan_proto, modem_unit_id;
	int ppp_index, gateway_valid, gateway_off_subnet;
	int status = 0, route_status, interface_cleanup_status;
#if defined (USE_IPV6)
	int ipv6_status;
#endif

	logmessage(LOGNAME, "%s %s (%s)", "WAN", "down", wan_ifname);
	/*
	 * The DHCP/PPP down callback is already in flight. Complete teardown even
	 * when pause fails, but do not discard the notification error.
	 */
	route_status = notify_pause_detect_internet();
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME,
			   "Internet monitor pause incomplete during WAN teardown (status=%d)",
			   route_status);

	/* deferred stop static VPN client (prevent rebuild resolv.conf) */
	nvram_set_temp("vpnc_dns_t", "");
	if (!is_static) {
		route_status = notify_rc_status("stop_vpn_client");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME,
				   "WAN VPN client stop notification incomplete (status=%d)",
				   route_status);
	}

#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 1) {
		ipv6_status = wan6_down(wan_ifname, unit);
		status = route_status_accumulate(status, ipv6_status);
		if (ipv6_status != 0)
			logmessage(LOGNAME, "WAN IPv6 teardown incomplete on %s (status=%d)",
				   wan_ifname, ipv6_status);
	}
#endif

	wan_proto = get_wan_proto(unit);
	modem_unit_id = is_ifunit_modem(wan_ifname, unit);

	/* Remove only the lease-derived routes that this WAN path installed. */
	if (modem_unit_id == 2 ||
	    (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		char prefix[16];
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		route_status = del_dhcp_routes_by_prefix(prefix, wan_ifname, 0);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "failed to delete WAN DHCP routes on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Stop multicast router (for NDIS or IPoE) and retain its status. */
	if (ppp_ifindex(wan_ifname) < 0) {
		route_status = stop_igmpproxy(wan_ifname);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN multicast teardown incomplete on %s (error=%d)",
				   wan_ifname, route_status);
	}

	/* Stop kabinet authenticator (for IPoE) */
	if (!modem_unit_id && (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)) {
		if (get_wan_unit_value_int(unit, "auth_mode") == 1) {
			route_status = stop_auth_kabinet();
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME, "WAN kabinet authentication teardown incomplete (status=%d)", route_status);
		}
	}

	/* Remove the default route and the matching off-subnet gateway /32 route. */
	wan_gate = get_wan_unit_value(unit, "gateway");
	gateway_valid = is_valid_ipv4(wan_gate);
	if (gateway_valid) {
		route_status = route_del(wan_ifname, 1, "0.0.0.0", wan_gate, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_status)) {
			status = route_status_accumulate(status, route_status);
			logmessage(LOGNAME, "failed to delete WAN default route via %s on %s (error=%d)",
				   wan_gate, wan_ifname, route_status);
		}

		wan_route_addr = get_wan_unit_value(unit, "ipaddr");
		wan_mask = get_wan_unit_value(unit, "netmask");
		ppp_index = ppp_ifindex(wan_ifname);
		gateway_off_subnet = ppp_index < 0 &&
			!is_same_subnet(wan_gate, wan_route_addr, wan_mask);
		if (route_status_wan_gateway_host_route_required(
			    gateway_valid, ppp_index, gateway_off_subnet)) {
			route_status = route_del(wan_ifname, 1, wan_gate, NULL,
						 "255.255.255.255");
			route_status = route_status_idempotent_delete(route_status);
			status = route_status_accumulate(status, route_status);
			if (route_status != 0)
				logmessage(LOGNAME,
					   "failed to delete WAN gateway host route %s on %s (error=%d)",
					   wan_gate, wan_ifname, route_status);
		}
	}

	/* Remove interface dependent static routes */
	route_status = del_static_wan_routes(wan_ifname);
	status = route_status_accumulate(status, route_status);
	if (route_status != 0)
		logmessage(LOGNAME, "failed to delete static WAN routes on %s (error=%d)",
			   wan_ifname, route_status);

	/* leave as is if no dns servers left for demand to work */
	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || wan_proto == IPV4_WAN_PROTO_IPOE_DHCP ||
	    strlen(nvram_safe_get("wanx_dns")) > 6)
		set_wan_unit_value(unit, "dns", "");

	/* Resolver refresh failure must not block WAN teardown/cleanup. */
	refresh_resolvconf_nonfatal("WAN down", 0);

	if (!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_STATIC) {
		route_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		status = route_status_accumulate(status, route_status);
		if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, route_status)) {
			logmessage(LOGNAME, "failed to clear static WAN address on %s (error=%d); forcing interface down",
				   wan_ifname, route_status);
			interface_cleanup_status = ifconfig(wan_ifname, 0, NULL, NULL);
			status = route_status_accumulate(status, interface_cleanup_status);
			if (interface_cleanup_status != 0)
				logmessage(LOGNAME, "failed to take static WAN interface %s down after address cleanup failure (status=%d)",
					   wan_ifname, interface_cleanup_status);
		}
	}

	/* flush conntrack table (only old WAN IP records) */
	wan_addr = get_wan_unit_value(unit, "ipaddr");
	if (is_valid_ipv4(wan_addr)) {
		route_status = flush_conntrack_table(wan_addr);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN conntrack cleanup failed on %s (status=%d)",
				   wan_ifname, route_status);
	} else
		wan_addr = "0.0.0.0";

	control_wan_led_isp_state(0, 0);

	set_wan_unit_value_int(unit, "uptime", 0);
	set_wan_unit_value_int(unit, "dltime", 0);
	set_wan_unit_value_int(unit, "bytes_rx", 0);
	set_wan_unit_value_int(unit, "bytes_tx", 0);

	/* Call the optional post-WAN hook and preserve its exit status. */
	if (check_if_file_exist(script_postw)) {
		route_status = eval(script_postw, "down", wan_ifname, wan_addr);
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN post-down script incomplete on %s (status=%d)",
				   wan_ifname, route_status);
	}
	if (nvram_match("pppoemwan_enable", "1")) {
		route_status = eval("/usr/bin/detect.sh");
		status = route_status_accumulate(status, route_status);
		if (route_status != 0)
			logmessage(LOGNAME, "WAN PPPoE-MWAN reconfiguration incomplete (status=%d)",
				   route_status);
	}

	return status;
}

int
wan_down(char *wan_ifname, int unit, int is_static)
{
	int status = wan_down_internal(wan_ifname, unit, is_static);

	if (status != 0)
		logmessage(LOGNAME, "WAN route teardown incomplete on %s (error=%d)",
			   wan_ifname, status);
	return status;
}

static int
merge_l2tp_server_start_status(int wan_status)
{
	int xl2tp_status;

	if (!nvram_match("l2tp_srv_t", "1"))
		return wan_status;

	/* safe_start_xl2tpd() returns 1 when the shared daemon is already running. */
	xl2tp_status = safe_start_xl2tpd();
	if (wan_status == 0 && xl2tp_status < 0) {
		logmessage(LOGNAME, "L2TP server start failed after WAN start (status=%d)", xl2tp_status);
		return xl2tp_status;
	}
	return wan_status;
}

int
full_restart_wan(void)
{
	int status;
	int reset_status;
	int static_lan_route_status;
	int switch_vlan_status;
	int route_flush_status;
	char *route_ifnames[2];

	if (get_ap_mode())
		return 0;
	status = stop_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because services did not stop (status=%d)",
			   status);
		return status;
	}
	route_ifnames[0] = get_man_ifname(0);
	route_ifnames[1] = IFNAME_BR;
	route_flush_status = rc_wan_route_flush_and_cache_apply(
		route_ifnames, 2, clear_if_route4, flush_route_caches);
	if (route_flush_status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because interface routes or the global route cache could not be flushed (status=%d)",
			   route_flush_status);
		return route_flush_status;
	}
	status = phy_clear_mac_table();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because switch MAC table could not be cleared (status=%d)",
			   status);
		return status;
	}
	update_router_mode();
	reset_status = reset_wan_vars_status();
	if (reset_status != 0) {
		status = rc_wan_restore_lan_routes_after_failed_reset(
			reset_status, IFNAME_BR, add_static_lan_routes,
			&static_lan_route_status);
		if (static_lan_route_status != 0)
			logmessage(LOGNAME,
				   "static LAN route restoration also failed after WAN protocol staging error (status=%d)",
				   static_lan_route_status);
		logmessage(LOGNAME,
			   "aborting full WAN restart because configured WAN protocol staging failed (status=%d)",
			   reset_status);
		return status;
	}
	status = add_static_lan_routes(IFNAME_BR);
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because static LAN routes could not be restored (status=%d)",
			   status);
		return status;
	}
	select_usb_modem_to_wan();
	notify_reset_detect_link();
	switch_vlan_status = switch_config_vlan(0);
	if (switch_vlan_status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because switch VLAN reconfiguration failed (status=%d)",
			   switch_vlan_status);
		return switch_vlan_status;
	}
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	status = disassembly_bridges_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because WAN bridge teardown failed (status=%d)",
			   status);
		return status;
	}
	status = config_soft_bridge_lan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting full WAN restart because LAN bridge VLAN setup failed (status=%d)",
			   status);
		return status;
	}
#endif
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
try_wan_reconnect(int try_use_modem, long pause_in_seconds)
{
	int status;
	if (get_ap_mode())
		return 0;
	status = stop_wan();
	if (status != 0) {
		logmessage(LOGNAME, "aborting WAN reconnect because services did not stop (status=%d)", status);
		return status;
	}
	status = reset_wan_vars_status();
	if (status != 0) {
		logmessage(LOGNAME,
			   "aborting WAN reconnect because configured WAN protocol staging failed (status=%d)",
			   status);
		return status;
	}
	if (pause_in_seconds > 0) {
		long deferred_up_time = uptime() + pause_in_seconds;
		if (deferred_up_time == 0)
			deferred_up_time++;
		nvram_set_int_temp("deferred_wanup_t", deferred_up_time);
		status = start_watchdog();
		if (status != 0) {
			/*
			 * Keep the deferred intent published so a later watchdog recovery
			 * can still perform the requested WAN reconnect.
			 */
			logmessage(LOGNAME,
				   "cannot schedule deferred WAN reconnect because watchdog startup failed (status=%d)",
				   status);
			return status;
		}
		logmessage(LOGNAME, "WAN up delay: %lds.", pause_in_seconds);
		return 0;
	}
	if (try_use_modem)
		select_usb_modem_to_wan();
	notify_reset_detect_link();
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
deferred_wan_connect(void)
{
	int status;
	if (check_if_file_exist(SCRIPT_UDHCPC_WAN))
		return 0;
	nvram_set_int_temp("deferred_wanup_t", 0);
	select_usb_modem_to_wan();
	notify_reset_detect_link();
	status = start_wan();
	return merge_l2tp_server_start_status(status);
}

int
manual_wan_disconnect(void)
{
	int wan_proto, status;
	logmessage(LOGNAME, "Perform WAN %s %s", "manual", "disconnect");
	if (get_ap_mode())
		return 0;
	wan_proto = get_wan_proto(0);
	if (get_usb_modem_wan(0)) {
		if (nvram_get_int("modem_type") == 3)
			status = stop_wan();
		else
			status = stop_wan_ppp();
	} else if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		   wan_proto == IPV4_WAN_PROTO_PPTP ||
		   wan_proto == IPV4_WAN_PROTO_L2TP) {
		status = stop_wan_ppp();
	} else {
		status = stop_wan();
	}

	if (status != 0)
		logmessage(LOGNAME, "manual WAN disconnect failed (status=%d)", status);
	return status;
}

int
manual_wan_reconnect(void)
{
	logmessage(LOGNAME, "Perform WAN %s %s", "manual", "reconnect");
	return try_wan_reconnect(1, 0);
}

int
manual_wisp_reassoc(void)
{
	char *wisp_ifname = get_apcli_wisp_ifname();
	int reconnect_status;

	if (!wisp_ifname)
		return 0;

	logmessage(LOGNAME, "Perform WISP %s %s", "manual", "reassoc");

	reconnect_status = reconnect_apcli(wisp_ifname, 1);
	if (reconnect_status != 0)
		logmessage(LOGNAME, "manual WISP reassociation failed on %s (status=%d)",
			   wisp_ifname, reconnect_status);
	return reconnect_status;
}

int
auto_wan_reconnect(void)
{
	logmessage(LOGNAME, "Perform WAN %s %s", "auto", "reconnect");
	return try_wan_reconnect(1, 0);
}

int
auto_wan_reconnect_pause(void)
{
	logmessage(LOGNAME, "Perform WAN %s %s", "auto", "reconnect");
	return try_wan_reconnect(1, nvram_get_int("di_recon_pause"));
}

void
notify_on_wan_ether_link_restored(void)
{
	int unit = 0;
	int wan_proto = get_wan_proto(unit);

	if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC || nvram_match("x_DHCPClient", "0"))
		return;

	if (is_usbnet_interface(get_wan_unit_value(unit, "ifname_t")))
		return;

	if (get_wan_wisp_active(NULL))
		return;

	if (renew_udhcpc_wan(unit) == 0) {
		logmessage(LOGNAME, "force WAN DHCP client renew...");
	}
}

static void
queue_internet_lost_action(const char *event_name, const char *action_name)
{
	int status, saved_errno;

	errno = 0;
	status = notify_rc_status(event_name);
	saved_errno = detect_internet_notification_error(status, errno);
	if (saved_errno == 0)
		return;

	logmessage(LOGNAME,
		   "cannot enqueue %s after Internet loss (status=%d, errno=%d)",
		   action_name, status, saved_errno);
	errno = saved_errno;
}


void
notify_on_internet_state_changed(int has_internet, long elapsed)
{
	const char *script_inet = SCRIPT_INTERNET_STATE;
	int hook_status;

	if (!has_internet && !get_ap_mode()) {
		int fail_action = nvram_safe_get_int("di_lost_action", 0, 0, 3);
		switch (fail_action)
		{
		case 1:
			logmessage(LOGNAME, "Perform router auto-reboot on %s event", "Internet lost");
			queue_internet_lost_action(RCN_RESTART_REBOOT, "router auto-reboot");
			return;
		case 2:
			queue_internet_lost_action("auto_wan_reconnect_pause",
					   "automatic WAN reconnect pause");
			break;
#if defined (USE_USB_SUPPORT)
		case 3:
			if (get_modem_devnum()) {
				nvram_set_int("modem_prio", (get_usb_modem_wan(0)) ? 0 : 1);
				queue_internet_lost_action("auto_wan_reconnect",
						   "automatic WAN reconnect");
			}
			break;
#endif
		}
	}

	if (check_if_file_exist(script_inet)) {
		hook_status = doSystem("%s %d %ld", script_inet, has_internet, elapsed);
		if (hook_status != 0)
			logmessage(LOGNAME,
				   "internet state hook failed (state=%d, elapsed=%ld, wait status=%d)",
				   has_internet, elapsed, hook_status);
	}
}

int
update_resolvconf(int is_first_run, int do_not_notify)
{
	FILE *fp;
	char word[512], *next, *wan_dns;
	const char *google_dns = "8.8.8.8";
	char resolv_temp[] = "/etc/resolv.conf.XXXXXX";
	int i, i_pdns, i_total_dns = 0;
	int lock, dns_static, resolv_changed = 0;
	int dns_restart_status;
	int dns_servers_ready = 1, dns_servers_error = 0, saved_errno, validation_errno;

	if (!is_first_run && fill_dnsmasq_servers() != 0) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_is_safe_regular(DNS_SERVERS_FILE) != 0) {
			validation_errno = errno ? errno : EPERM;
			dns_servers_ready = 0;
			dns_servers_error = validation_errno;
			logmessage(LOGNAME, "DNSMasq reload blocked: unsafe DNS server snapshot (errno=%d)", validation_errno);
		} else {
			logmessage(LOGNAME, "DNS server snapshot refresh failed; keeping last-known-good file (errno=%d)", saved_errno);
		}
	}

	lock = file_lock("resolv");
	if (lock < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire resolver publication lock; refusing update (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	i_pdns = nvram_get_int("vpnc_pdns");
	dns_static = get_wan_dns_static();

	/* Never truncate /etc/resolv.conf or use a predictable temporary path. */
	fp = dnsmasq_file_open(resolv_temp);
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove resolver staging file '%s' after open failure (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		logmessage(LOGNAME, "cannot stage resolver configuration (errno=%d)", saved_errno);
		file_unlock(lock);
		errno = saved_errno;
		return saved_errno;
	}

	/* dnsmasq will resolve localhost DNS queries */
	fprintf(fp, "nameserver %s\n", "127.0.0.1");

	/* DNS servers for static VPN client */
	if (!is_first_run && i_pdns > 0) {
		wan_dns = nvram_safe_get("vpnc_dns_t");
		if (strlen(wan_dns) > 6) {
			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word)) {
					fprintf(fp, "nameserver %s\n", word);
					i_total_dns++;
				}
			}
		}
	}

	/* DNS servers for WAN/MAN */
	if (i_pdns != 2 || i_total_dns < 1) {
		if (dns_static) {
			char dns_name_x[16];

			if (is_first_run)
				resolv_changed = 1;

			for (i = 1; i <= 3; i++) {
				sprintf(dns_name_x, "wan_dns%d_x", i);
				wan_dns = nvram_safe_get(dns_name_x);
				if (is_valid_ipv4(wan_dns)) {
					fprintf(fp, "nameserver %s\n", wan_dns);
					i_total_dns++;
				}
			}

		} else if (!is_first_run) {
			wan_dns = get_wan_unit_value(0, "dns");
			if (strlen(wan_dns) < 7)
				wan_dns = nvram_safe_get("wanx_dns");

			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word)) {
					fprintf(fp, "nameserver %s\n", word);
					i_total_dns++;
				}
			}
		}
	}

	if (i_total_dns < 1)
		fprintf(fp, "nameserver %s\n", google_dns);

#if defined (USE_IPV6)
	/* DNSv6 servers */
	wan_dns = get_wan_unit_value(0, "dns6");
	foreach(word, wan_dns, next) {
		if (strlen(word) > 0) {
			char dns6s[INET6_ADDRSTRLEN] = {0};
			if (ipv6_compact(word, dns6s, 0) == 0) {
				fprintf(fp, "nameserver %s\n", dns6s);
				if (is_first_run)
					resolv_changed = 1;
			}
		}
	}
#endif

	/* Catch buffered generation failures before comparing or publishing. */
	if (fflush(fp) != 0 || ferror(fp)) {
		saved_errno = errno ? errno : EIO;
		if (fclose(fp) != 0)
			logmessage(LOGNAME, "error closing failed resolver staging file (errno=%d)", errno ? errno : EIO);
		if (dnsmasq_file_discard(resolv_temp) != 0)
			logmessage(LOGNAME, "cannot remove failed resolver staging file '%s' (errno=%d)",
				   resolv_temp, errno ? errno : EIO);
		file_unlock(lock);
		logmessage(LOGNAME, "cannot write resolver configuration (errno=%d)", saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* Unsafe/missing destinations are treated as changed and atomically replaced.
	 * Equal files are retained to avoid needless dnsmasq reloads. */
	if (dnsmasq_file_contents_equal(DNS_RESOLV_CONF, resolv_temp)) {
		if (fclose(fp) != 0) {
			saved_errno = errno ? errno : EIO;
			if (dnsmasq_file_discard(resolv_temp) != 0)
				logmessage(LOGNAME, "cannot remove unchanged resolver staging file '%s' (errno=%d)",
					   resolv_temp, errno ? errno : EIO);
			file_unlock(lock);
			logmessage(LOGNAME, "cannot close unchanged resolver staging file (errno=%d)", saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		if (dnsmasq_file_discard(resolv_temp) != 0) {
			saved_errno = errno ? errno : EIO;
			file_unlock(lock);
			logmessage(LOGNAME, "cannot remove unchanged resolver staging file '%s' (errno=%d)",
				   resolv_temp, saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
	} else {
		if (dnsmasq_file_publish(fp, resolv_temp, DNS_RESOLV_CONF, 0644) != 0) {
			saved_errno = errno ? errno : EIO;
			if (dnsmasq_file_discard(resolv_temp) != 0)
				logmessage(LOGNAME, "cannot remove failed resolver staging file '%s' (errno=%d)",
					   resolv_temp, errno ? errno : EIO);
			file_unlock(lock);
			logmessage(LOGNAME, "cannot publish resolver configuration (errno=%d)", saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		resolv_changed = 1;
	}

	file_unlock(lock);

/* Do not report success when an unsafe upstream snapshot prevented the
 * DNSMasq cache refresh. The resolver file is published, but the daemon
 * must not be signalled until its server input is safe. */
	saved_errno = wan_resolver_refresh_policy_blocked_status(dns_servers_ready,
								 dns_servers_error);
	if (saved_errno != 0) {
		logmessage(LOGNAME,
			   "resolver file published but DNSMasq server snapshot is unsafe; cache reload was blocked (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* force notify dnsmasq for modem */
	if (!resolv_changed && get_usb_modem_wan(0))
		resolv_changed = 1;

	/* Notify DNSMasq only on a complete process snapshot. A failed legacy
	 * pids() scan is indistinguishable from ABSENT and can leave stale caches. */
	if (resolv_changed && !do_not_notify && dns_servers_ready) {
		int dnsmasq_scan_error = 0;
		int dnsmasq_status = dnsmasq_process_status(&dnsmasq_scan_error);

		if (dnsmasq_status < 0) {
			saved_errno = dnsmasq_scan_error ? dnsmasq_scan_error : EIO;
			logmessage(LOGNAME,
				   "resolver file published but DNSMasq process state is unknown; cache reload not attempted (scan_error=%d)",
				   saved_errno);
			errno = saved_errno;
			return saved_errno;
		}
		if (dnsmasq_status > 0) {
			dns_restart_status = restart_dns();
			if (dns_restart_status != 0) {
				logmessage(LOGNAME,
					   "resolver file published but DNSMasq cache reload failed (status=%d)",
					   dns_restart_status);
				return dns_restart_status;
			}
		}
	}

	return 0;
}

static void
refresh_resolvconf_nonfatal_internal(const char *reason, int is_first_run,
				     int do_not_notify)
{
	int refresh_status;

	refresh_status = update_resolvconf(is_first_run, do_not_notify);
	if (wan_resolver_refresh_policy_action(refresh_status) ==
	    WAN_RESOLVER_REFRESH_KEEP_WAN) {
		logmessage(LOGNAME,
			   "resolver/DNS refresh did not complete after %s (status=%d); keeping WAN state",
			   (reason && *reason) ? reason : "WAN event", refresh_status);
	}
}

void
refresh_resolvconf_nonfatal(const char *reason, int is_first_run)
{
	refresh_resolvconf_nonfatal_internal(reason, is_first_run, 0);
}

void
refresh_resolvconf_nonfatal_no_notify(const char *reason, int is_first_run)
{
	refresh_resolvconf_nonfatal_internal(reason, is_first_run, 1);
}

int
update_hosts_router(const char *lan_ipaddr)
{
	char hostname_backup[] = "/etc/hostname.rollback.XXXXXX";
	char hosts_backup[] = "/etc/hosts.rollback.XXXXXX";
	FILE *hostname_fp = NULL, *hosts_fp = NULL;
	char hostname_temp[] = "/etc/hostname.XXXXXX";
	char hosts_temp[] = "/etc/hosts.XXXXXX";
	char *lan_hname, *lan_dname;
	int saved_errno = 0;
	int hosts_lock = -1;

	lan_hname = get_our_hostname();
	lan_dname = nvram_safe_get("lan_domain");
	if (!dhcp_lease_ipv4_is_valid(lan_ipaddr) || !lan_dname ||
	    (*lan_dname && !dnsmasq_valid_server_domain(lan_dname)) || !lan_hname) {
		errno = EINVAL;
		logmessage(LOGNAME, "refusing unsafe router hostname input");
		return EINVAL;
	}

	/* Stage both resolver-visible files before replacing either destination. */
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

	/* Never ingest a stale or attacker-planted shared-/tmp link as hosts data. */
	if (dnsmasq_file_is_safe_regular("/tmp/hosts.static") == 0)
		load_user_config(hosts_fp, "/tmp", "hosts.static", NULL);
	else if (errno != ENOENT)
		logmessage(LOGNAME, "skipping unsafe static DHCP host snapshot (errno=%d)", errno);

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED) {
		fprintf(hosts_fp, "%s %s %s\n", "::1", "localhost.localdomain", "localhost");
		{
			char addr6s[INET6_ADDRSTRLEN];
			char *lan_addr6_host = get_lan_addr6_host(addr6s);
			if (lan_addr6_host) {
				if (strlen(lan_dname) > 0)
					fprintf(hosts_fp, "%s %s.%s %s\n", lan_addr6_host, lan_hname, lan_dname, lan_hname);
				else
					fprintf(hosts_fp, "%s %s\n", lan_addr6_host, lan_hname);
				fprintf(hosts_fp, "%s %s\n", lan_addr6_host, "my.router");
			}
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

	/* The pair helper already tried once; retry snapshots while their buffers
	 * remain in scope and warn without undoing the completed publication. */
	if (dnsmasq_file_backup_discard(hostname_backup) != 0)
		logmessage(LOGNAME, "cannot remove router hostname rollback snapshot '%s' (errno=%d)",
			   hostname_backup, errno ? errno : EIO);
	if (dnsmasq_file_backup_discard(hosts_backup) != 0)
		logmessage(LOGNAME, "cannot remove router hosts rollback snapshot '%s' (errno=%d)",
			   hosts_backup, errno ? errno : EIO);

	/* Update kernel names only after both corresponding files were published. */
	sethostname(lan_hname, strlen(lan_hname));
	setdomainname(lan_dname, strlen(lan_dname));
	if (dnsmasq_lifecycle_lock_release(hosts_lock) != 0) {
		saved_errno = errno ? errno : EIO;
		hosts_lock = -1;
		goto failed;
	}
	hosts_lock = -1;
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
		logmessage(LOGNAME, "cannot remove router hostname stage '%s' (errno=%d)",
			   hostname_temp, errno ? errno : EIO);
	if (dnsmasq_file_discard(hosts_temp) != 0)
		logmessage(LOGNAME, "cannot remove router hosts stage '%s' (errno=%d)",
			   hosts_temp, errno ? errno : EIO);
	if (hostname_backup[0])
		logmessage(LOGNAME, "retaining router hostname rollback snapshot '%s' for recovery", hostname_backup);
	if (hosts_backup[0])
		logmessage(LOGNAME, "retaining router hosts rollback snapshot '%s' for recovery", hosts_backup);
	logmessage(LOGNAME, "cannot stage/publish router hostname files (errno=%d)", saved_errno);
	errno = saved_errno;
	return saved_errno;
}

static int
control_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms,
                    char *ifname, int metric, int is_add)
{
	char *routes, *tmp;
	const char *classless_routes;
	struct in_addr mask;
	char *ipaddr, *gateway, *bits_str, *bits_end;
	long parsed_bits;
	int bits;
	int status = 0, route_status;
	char netmask[] = "255.255.255.255";

	/* Keep cleanup working even after the dynamic-route option is disabled. */
	if (is_add && !nvram_match("dr_enable_x", "1"))
		return 0;
	if (!ifname || !*ifname)
		return EINVAL;
	if (!rt)
		rt = "";
	if (!rt_rfc)
		rt_rfc = "";
	if (!rt_ms)
		rt_ms = "";

	/* Legacy destination/gateway pairs. */
	routes = NULL;
	if (*rt) {
		routes = strdup(rt);
		if (!routes) {
			status = route_status_accumulate(status, ENOMEM);
			logmessage(LOGNAME, "failed to copy legacy DHCP routes on %s (errno=%d)",
				   ifname, ENOMEM);
		} else {
			for (tmp = routes; tmp && *tmp; ) {
				ipaddr = strsep(&tmp, "/");
				gateway = strsep(&tmp, " ");
				if (gateway && *gateway &&
				    dhcp_lease_ipv4_is_valid(ipaddr) &&
				    dhcp_lease_ipv4_is_valid(gateway)) {
					route_status = is_add ?
						route_add(ifname, metric, ipaddr, gateway, netmask) :
						route_del(ifname, metric, ipaddr, gateway, netmask);
					if ((is_add && !dhcp_lease_route_add_is_ok(route_status)) ||
					    (!is_add && !dhcp_lease_route_delete_is_ok(route_status))) {
						status = route_status_accumulate(status, route_status);
						logmessage(LOGNAME, "failed to %s DHCP route %s via %s on %s (error=%d)",
							   is_add ? "add" : "delete", ipaddr, gateway,
							   ifname, route_status);
					}
				}
			}
			free(routes);
		}
	}
	/* RFC3442 or Microsoft classless static routes. */
	classless_routes = *rt_rfc ? rt_rfc : rt_ms;
	if (*classless_routes) {
		routes = strdup(classless_routes);
		if (!routes) {
			status = route_status_accumulate(status, ENOMEM);
			logmessage(LOGNAME, "failed to copy classless DHCP routes on %s (errno=%d)",
				   ifname, ENOMEM);
		} else {
			for (tmp = routes; tmp && *tmp; ) {
				ipaddr = strsep(&tmp, "/");
				if (!ipaddr || !tmp || !*tmp)
					break;
				bits_str = strsep(&tmp, " ");
				if (!bits_str || !*bits_str || !tmp || !*tmp)
					break;
				errno = 0;
				parsed_bits = strtol(bits_str, &bits_end, 10);
				gateway = strsep(&tmp, " ");
				if (errno == ERANGE || bits_end == bits_str || *bits_end != '\0' ||
				    parsed_bits < 0 || parsed_bits > 32 ||
				    !gateway || !*gateway ||
				    !dhcp_lease_route_destination_is_valid(ipaddr, (unsigned int)parsed_bits) ||
				    !dhcp_lease_ipv4_is_valid(gateway))
					continue;
				bits = (int)parsed_bits;
				mask.s_addr = (bits == 0) ? 0 : htonl(0xffffffffU << (32 - bits));
				strcpy(netmask, inet_ntoa(mask));
				route_status = is_add ?
					route_add(ifname, metric, ipaddr, gateway, netmask) :
					route_del(ifname, metric, ipaddr, gateway, netmask);
				if ((is_add && !dhcp_lease_route_add_is_ok(route_status)) ||
				    (!is_add && !dhcp_lease_route_delete_is_ok(route_status))) {
					status = route_status_accumulate(status, route_status);
					logmessage(LOGNAME, "failed to %s DHCP classless route %s/%d via %s on %s (error=%d)",
						   is_add ? "add" : "delete", ipaddr, bits, gateway,
						   ifname, route_status);
				}
			}
			free(routes);
		}
	}
	return status;
}

int
add_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms, char *ifname, int metric)
{
	return control_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric, 1);
}

int
add_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric)
{
	char buf[32];
	char *rt, *rt_rfc, *rt_ms;

	if (!prefix || !*prefix)
		return EINVAL;
	rt     = nvram_safe_get(strcat_r(prefix, "routes", buf));
	rt_rfc = nvram_safe_get(strcat_r(prefix, "routes_rfc", buf));
	rt_ms  = nvram_safe_get(strcat_r(prefix, "routes_ms", buf));
	return add_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric);
}

int
del_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric)
{
	char buf[32];
	char *rt, *rt_rfc, *rt_ms;

	if (!prefix || !*prefix)
		return EINVAL;
	rt     = nvram_safe_get(strcat_r(prefix, "routes", buf));
	rt_rfc = nvram_safe_get(strcat_r(prefix, "routes_rfc", buf));
	rt_ms  = nvram_safe_get(strcat_r(prefix, "routes_ms", buf));
	return control_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric, 0);
}

int
add_static_wan_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 1);
}

int
del_static_wan_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 0);
}

int
add_static_man_routes(char *man_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, man_ifname, 1);
}

int
del_static_man_routes(char *man_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, man_ifname, 0);
}

void
select_usb_modem_to_wan(void)
{
	int modem_devnum = 0;

#if defined (USE_USB_SUPPORT)
	/* check modem enabled and ready */
	modem_devnum = get_modem_devnum();
	if (modem_devnum) {
		int modem_prio = nvram_get_int("modem_prio");
		if (modem_prio < 1) {
			modem_devnum = 0;
		} else if (modem_prio == 2) {
			if (!get_apcli_wisp_ifname()) {
				int has_link = get_wan_ether_link_direct(0);
				if (has_link < 0)
					has_link = 0;
				
				if (has_link)
					modem_devnum = 0;
			} else
				modem_devnum = 0;
		}
	}
#endif
	set_usb_modem_dev_wan(0, modem_devnum);
}

int
get_wan_ether_link_direct(int is_ap_mode)
{
	int ret = 0, wan_src_phy = SWAPI_PORT_WAN;
	unsigned int phy_link = 0;

	if (!is_ap_mode)
		wan_src_phy = nvram_get_int("wan_src_phy");

	if (wan_src_phy < 0)
		wan_src_phy = SWAPI_PORT_WAN;

	ret = phy_status_port_link(wan_src_phy, &phy_link);
	if (ret != 0)
		return -1;

	return (phy_link) ? 1 : 0;
}

int
get_wan_dns_static(void)
{
	int unit = 0;

	if (get_usb_modem_wan(unit))
		return nvram_match("modem_dnsa", "0"); // dynamic or static dns for ppp2/weth0

	if (get_wan_proto(unit) == IPV4_WAN_PROTO_IPOE_STATIC)
		return 1;

	if (get_wan_unit_value_int(unit, "dnsenable_x") < 1)
		return 1;

	return 0;
}

int
get_wan_wisp_active(int *p_has_link)
{
	int unit = 0;
	char *man_ifname;

	man_ifname = get_man_ifname(unit);
	if (!is_man_wisp(man_ifname))
		return 0;

	if (p_has_link)
		*p_has_link = get_apcli_connected(man_ifname);

	return 1;
}

int
get_wan_ifname(char wan_ifname[16])
{
	int unit = 0;
	int status;
	char *ifname = get_man_ifname(unit);
	char *ifname_temp = get_wan_unit_value(unit, "ifname_t");

	if (get_usb_modem_wan(unit)){
		if (nvram_get_int("modem_type") == 3) {
			if (is_usbnet_interface(ifname_temp))
				ifname = ifname_temp;
		} else {
			if (ppp_ifindex(ifname_temp) >= RAS_PPP_UNIT)
				ifname = ifname_temp;
			else
				ifname = IFNAME_RAS;
		}
	} else {
		int wan_proto = get_wan_proto(unit);
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP) {
			if (ppp_ifindex(ifname_temp) >= WAN_PPP_UNIT)
				ifname = ifname_temp;
			else
				ifname = IFNAME_PPP;
		}
	}

	status = wan_ifname_copy(wan_ifname, 16, ifname);
	if (status != 0) {
		errno = status;
		return status;
	}
	return 0;
}

in_addr_t
get_wan_ip4(int only_broadband_wan)
{
	int unit = 0;
	char *ifname, *ifname_temp;

	if (get_ap_mode())
		return INADDR_ANY;

	ifname = get_man_ifname(unit);
	ifname_temp = get_wan_unit_value(unit, "ifname_t");

	if (!only_broadband_wan && get_usb_modem_wan(unit)) {
		ifname = ifname_temp;
	} else {
		int wan_proto = get_wan_proto(unit);
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP) {
			if (ppp_ifindex(ifname_temp) >= 0)
				ifname = ifname_temp;
			else
				ifname = IFNAME_PPP;
		}
	}

	return get_interface_addr4(ifname);
}

int
has_wan_ip4(int only_broadband_wan)
{
	if (get_wan_ip4(only_broadband_wan) != INADDR_ANY)
		return 1;

	return 0;
}

int
has_wan_gw4(void)
{
	int unit = 0;

	if (is_valid_ipv4(get_wan_unit_value(unit, "gateway")))
		return 1;

	return 0;
}

static int
udhcpc_deconfig(char *wan_ifname, int is_zcip)
{
	int unit, wan_proto, is_man;
	int interface_status = 0, reset_man_address = 0;
	int conntrack_status = 0;
	int cleanup_status;
	char log_prefix[32], *udhcpc_state;

	unit = get_wan_unit(wan_ifname);
	wan_proto = get_wan_proto(unit);
	is_man = is_ifunit_man(wan_ifname, unit);

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", (is_zcip) ? "ZeroConf" : "DHCP", (is_man) ? "MAN" : "WAN");
	udhcpc_state = "deconfig";

	if (is_man && (wan_proto == IPV4_WAN_PROTO_PPTP ||
	               wan_proto == IPV4_WAN_PROTO_L2TP))
	{
		/* fix hang-up issue */
//		logmessage(log_prefix, "skipping resetting IP address to 0.0.0.0");
		;
	}
	else
	{
		interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		if (interface_status != 0)
			logmessage(log_prefix, "failed to clear address on %s (error=%d); continuing lease cleanup",
				   wan_ifname, interface_status);

		if (is_man) {
			char *man_addr = nvram_safe_get("wanx_ipaddr");

			/* flush conntrack table (only old WAN IP records) */
			if (is_valid_ipv4(man_addr)) {
				conntrack_status = flush_conntrack_table(man_addr);
				if (conntrack_status != 0)
					logmessage(log_prefix, "MAN conntrack cleanup failed (status=%d)",
						   conntrack_status);
			}

			/* man_down_internal() needs the old address to remove its host route. */
			reset_man_address = (interface_status == 0);
		}
	}

	logmessage(log_prefix, "%s (%s): lease is lost",
		udhcpc_state, wan_ifname);

	if (is_man)
		cleanup_status = man_down_internal(wan_ifname, unit, reset_man_address);
	else
		cleanup_status = wan_down_internal(wan_ifname, unit, 0);

	if (cleanup_status != 0)
		logmessage(log_prefix, "route teardown incomplete on %s (error=%d)",
			   wan_ifname, cleanup_status);

	return route_status_accumulate(
		route_status_accumulate(interface_status, conntrack_status),
		cleanup_status);
}

struct wan_dhcp_interface_context {
	char *ifname;
};

static int
wan_dhcp_interface_clear(void *opaque)
{
	struct wan_dhcp_interface_context *context =
		(struct wan_dhcp_interface_context *)opaque;

	return ifconfig(context->ifname, IFUP, "0.0.0.0", NULL);
}

static int
wan_dhcp_interface_set(void *opaque, const char *ipaddr,
		       const char *netmask)
{
	struct wan_dhcp_interface_context *context =
		(struct wan_dhcp_interface_context *)opaque;

	return ifconfig(context->ifname, IFUP, (char *)ipaddr, (char *)netmask);
}

static const struct dhcp_wan_interface_ops wan_dhcp_interface_ops = {
	wan_dhcp_interface_clear,
	wan_dhcp_interface_set
};


struct wan_dhcp_route_transition_context {
	struct wan_dhcp_interface_context *interface_context;
	const char *prefix;
	const char *old_gateway;
	const char *old_ip;
	const char *old_mask;
	int route_metric;
	int restore_host_route;
	int remove_dhcp_routes;
};

static int
wan_dhcp_delete_old_host_route(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return route_del(context->interface_context->ifname,
			 context->route_metric, (char *)context->old_gateway,
			 NULL, "255.255.255.255");
}

static int
wan_dhcp_delete_old_default_route(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return route_del(context->interface_context->ifname,
			 context->route_metric, "0.0.0.0",
			 (char *)context->old_gateway, "0.0.0.0");
}

static int
wan_dhcp_delete_old_dhcp_routes(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	return del_dhcp_routes_by_prefix((char *)context->prefix,
					 context->interface_context->ifname, 0);
}

static int
wan_dhcp_restore_old_address(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;

	if (dhcp_lease_ipv4_is_valid(context->old_ip) &&
	    dhcp_lease_netmask_is_valid(context->old_mask))
		return wan_dhcp_interface_set(context->interface_context,
					      context->old_ip, context->old_mask);
	return wan_dhcp_interface_clear(context->interface_context);
}

static int
wan_dhcp_restore_old_routes(void *opaque)
{
	struct wan_dhcp_route_transition_context *context =
		(struct wan_dhcp_route_transition_context *)opaque;
	char *ifname = context->interface_context->ifname;
	int status = 0, route_status;

	if (dhcp_lease_ipv4_is_valid(context->old_gateway)) {
		if (context->restore_host_route &&
		    dhcp_lease_ipv4_is_valid(context->old_ip) &&
		    dhcp_lease_netmask_is_valid(context->old_mask) &&
		    !is_same_subnet((char *)context->old_gateway,
				    (char *)context->old_ip,
				    (char *)context->old_mask)) {
			route_status = route_add(ifname, context->route_metric,
						  (char *)context->old_gateway,
						  NULL, "255.255.255.255");
			if (!dhcp_lease_route_add_is_ok(route_status))
				status = route_status_accumulate(status, route_status);
		}
		route_status = route_add(ifname, context->route_metric,
					 "0.0.0.0", (char *)context->old_gateway,
					 "0.0.0.0");
		if (!dhcp_lease_route_add_is_ok(route_status))
			status = route_status_accumulate(status, route_status);
	}
	if (context->remove_dhcp_routes) {
		route_status = add_dhcp_routes_by_prefix((char *)context->prefix,
							 ifname, 0);
		status = route_status_accumulate(status, route_status);
	}
	return status;
}

static const struct dhcp_wan_route_transition_ops wan_dhcp_route_transition_ops = {
	wan_dhcp_delete_old_host_route,
	wan_dhcp_delete_old_default_route,
	wan_dhcp_delete_old_dhcp_routes,
	wan_dhcp_restore_old_address,
	wan_dhcp_restore_old_routes
};

static int
udhcpc_bound(char *wan_ifname, int is_renew_mode)
{
	char *value, *lan_ip, *lan_nm, *wan_ip, *wan_nm, *wan_gw;
	char tmp[100], prefix[16], route[32], log_prefix[32], *udhcpc_state;
	char gateway_buf[INET_ADDRSTRLEN], old_gateway[INET_ADDRSTRLEN];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char *lease_ip, *lease_netmask, *lease_dns, *lease_wins, *lease_domain;
	char *route_value;
	unsigned int parsed_value;
	int unit, is_man, is_changed, i_err, has_gateway, lease_dur, dhcp_mtu;
	int wan_proto, modem_unit_id, route_result, route_metric;
	int interface_status, rollback_status, route_setup_status, mtu_status;
	int remove_old_host_route, remove_old_dhcp_routes;
	int route_cleanup_status, route_rollback_status;
	struct wan_dhcp_interface_context interface_context;
	struct wan_dhcp_route_transition_context route_context;

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);
	route_setup_status = 0;
	mtu_status = 0;

	if (is_man) {
		strcpy(prefix, "wanx_");
	} else {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		snprintf(tmp, sizeof(tmp), "%ld", uptime());
		set_wan_unit_value(unit, "dltime", tmp);
	}

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	udhcpc_state = (is_renew_mode) ? "renew" : "bound";

	/* Reject a malformed lease before it can reach NVRAM or ifconfig. */
	value = getenv("ip");
	lease_ip = value ? trim_r(value) : "";
	value = getenv("subnet");
	if (value)
		lease_netmask = trim_r(value);
	else
		lease_netmask = nvram_safe_get(strcat_r(prefix, "netmask", tmp));
	if (!dhcp_lease_ipv4_is_valid(lease_ip) ||
	    !dhcp_lease_netmask_is_valid(lease_netmask)) {
		logmessage(log_prefix, "%s (%s) rejected invalid DHCP address/netmask",
			udhcpc_state, wan_ifname);
		return EINVAL;
	}

	/* Snapshot old values before replacing the lease. */
	snprintf(old_gateway, sizeof(old_gateway), "%s",
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
	snprintf(old_ip, sizeof(old_ip), "%s",
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
	snprintf(old_mask, sizeof(old_mask), "%s",
		nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	wan_proto = get_wan_proto(unit);
	modem_unit_id = is_ifunit_modem(wan_ifname, unit);
	route_metric = is_man ? 2 : 1;

	/*
	 * Configure the candidate address before removing the previous lease's
	 * routes or publishing new NVRAM state. A failed ioctl must leave the
	 * last-known-good lease and its routes available for the next callback.
	 */
	is_changed = strcmp(old_ip, lease_ip) != 0;
	i_err = 0;
	wan_ip = lease_ip;
	wan_nm = lease_netmask;
	lan_ip = nvram_safe_get("lan_ipaddr");
	lan_nm = nvram_safe_get("lan_netmask");
	interface_context.ifname = wan_ifname;

	if (is_same_subnet2(wan_ip, lan_ip, wan_nm, lan_nm)) {
		if (!is_renew_mode) {
			interface_status = wan_dhcp_interface_clear(&interface_context);
			if (interface_status != 0) {
				logmessage(log_prefix,
					   "failed to clear conflicting DHCP address on %s (status=%d)",
					   wan_ifname, interface_status);
				return interface_status;
			}
		}
		i_err = 1;
		logmessage(log_prefix,
			   "%s (%s). ERROR! Obtained IP [%s/%s] in LAN subnet [%s/%s]. Need change LAN subnet!",
			   udhcpc_state, wan_ifname, wan_ip, wan_nm, lan_ip, lan_nm);
	} else {
		interface_status = dhcp_wan_interface_apply(
			old_ip, old_mask, lease_ip, lease_netmask, is_changed,
			&wan_dhcp_interface_ops, &interface_context, &rollback_status);
		if (interface_status != 0) {
			logmessage(log_prefix,
				   "failed to apply DHCP address on %s (status=%d); preserving previous lease and routes",
				   wan_ifname, interface_status);
			if (rollback_status != 0)
				logmessage(log_prefix,
					   "failed to restore previous DHCP address on %s (status=%d)",
					   wan_ifname, rollback_status);
			return interface_status;
		}
	}

	/* The old route inventory is still in NVRAM here. Do not publish a
	 * replacement lease if deleting that inventory fails: restore the old
	 * interface address and every route, then let udhcpc retry the callback. */
	remove_old_host_route = dhcp_lease_ipv4_is_valid(old_gateway) &&
		dhcp_lease_ipv4_is_valid(old_ip) &&
		dhcp_lease_netmask_is_valid(old_mask) &&
		!is_same_subnet(old_gateway, old_ip, old_mask);
	remove_old_dhcp_routes = is_man || modem_unit_id == 2 ||
		(!modem_unit_id && wan_proto == IPV4_WAN_PROTO_IPOE_DHCP);
	route_context.interface_context = &interface_context;
	route_context.prefix = prefix;
	route_context.old_gateway = old_gateway;
	route_context.old_ip = old_ip;
	route_context.old_mask = old_mask;
	route_context.route_metric = route_metric;
	route_context.restore_host_route = remove_old_host_route;
	route_context.remove_dhcp_routes = remove_old_dhcp_routes;
	route_cleanup_status = dhcp_wan_route_transition_apply(
		remove_old_host_route, dhcp_lease_ipv4_is_valid(old_gateway),
		remove_old_dhcp_routes, &wan_dhcp_route_transition_ops,
		&route_context, &route_rollback_status);
	if (route_cleanup_status != 0) {
		logmessage(log_prefix,
			   "failed to remove previous DHCP route inventory on %s (status=%d); preserving old lease",
			   wan_ifname, route_cleanup_status);
		if (route_rollback_status != 0)
			logmessage(log_prefix,
				   "failed to restore previous DHCP route inventory on %s (status=%d)",
				   wan_ifname, route_rollback_status);
		return route_cleanup_status;
	}

	lease_dur = 0;
	has_gateway = 0;
	dhcp_mtu = 0;
	gateway_buf[0] = '\0';

	nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), lease_ip);
	nvram_set_temp(strcat_r(prefix, "netmask", tmp), lease_netmask);

	value = getenv("router");
	if (value) {
		route_result = dhcp_lease_ipv4_list_first(trim_r(value),
			gateway_buf, sizeof(gateway_buf));
		if (route_result > 0)
			has_gateway = 1;
		else if (route_result < 0)
			logmessage(log_prefix, "ignoring invalid DHCP router list on %s", wan_ifname);
	}

	value = getenv("dns");
	lease_dns = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_dns, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP DNS list on %s", wan_ifname);
		lease_dns = "";
	}
	nvram_set_temp(strcat_r(prefix, "dns", tmp), lease_dns);

	value = getenv("wins");
	lease_wins = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_wins, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP WINS list on %s", wan_ifname);
		lease_wins = "";
	}
	nvram_set_temp(strcat_r(prefix, "wins", tmp), lease_wins);

	value = getenv("mtu");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 576U, 1500U, &parsed_value)) {
			dhcp_mtu = 0;
			logmessage(log_prefix, "ignoring invalid DHCP MTU on %s", wan_ifname);
		} else {
			dhcp_mtu = (int)parsed_value;
		}
		nvram_set_int_temp(strcat_r(prefix, "mtu", tmp), dhcp_mtu);
	}

	route_value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP route option on %s", wan_ifname);
		route_value = "";
	}
	nvram_set_temp(strcat_r(prefix, "routes", tmp), route_value);

	route_value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP Microsoft route option on %s", wan_ifname);
		route_value = "";
	}
	nvram_set_temp(strcat_r(prefix, "routes_ms", tmp), route_value);

	route_value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(route_value)) {
		logmessage(log_prefix, "ignoring invalid DHCP classless route option on %s", wan_ifname);
		route_value = "";
	}
	nvram_set_temp(strcat_r(prefix, "routes_rfc", tmp), route_value);

	value = getenv("domain");
	lease_domain = value ? trim_r(value) : "";
	if (*lease_domain && !dnsmasq_valid_server_domain(lease_domain)) {
		logmessage(log_prefix, "ignoring invalid DHCP domain on %s", wan_ifname);
		lease_domain = "";
	}
	nvram_set_temp(strcat_r(prefix, "domain", tmp), lease_domain);

	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value)) {
			lease_dur = 0;
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", wan_ifname);
		} else {
			lease_dur = (int)parsed_value;
		}
		nvram_set_int_temp(strcat_r(prefix, "lease", tmp), lease_dur);
	}

#if defined (USE_IPV6)
	value = getenv("ip6rd");
	if (value)
		store_ip6rd_from_dhcp(value, prefix);
#endif

	if (!has_gateway) {
		foreach(route, nvram_safe_get(strcat_r(prefix, "routes_rfc", tmp)), value) {
			if (has_gateway) {
				snprintf(gateway_buf, sizeof(gateway_buf), "%s", route);
				break;
			} else
				has_gateway = !strcmp(route, "0.0.0.0/0");
		}
	}
	nvram_set_temp(strcat_r(prefix, "gateway", tmp), gateway_buf);

	if (!i_err) {
		if (wan_dhcp_mtu_is_supported(dhcp_mtu)) {
			mtu_status = set_interface_mtu(wan_ifname, dhcp_mtu);
			if (mtu_status != 0)
				logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)",
					   udhcpc_state, wan_ifname, mtu_status);
		}

		wan_gw = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
		logmessage(log_prefix,
			"%s (%s), IP: %s, GW: %s, lease time: %d",
			udhcpc_state, wan_ifname, wan_ip, wan_gw, lease_dur);

		if (is_man)
			route_setup_status = man_up_internal(wan_ifname, unit, 0, NULL);
		else
			route_setup_status = wan_up_internal(wan_ifname, unit, 0);
		if (route_setup_status != 0)
			logmessage(log_prefix,
				   "%s (%s) accepted but route setup is incomplete (error=%d)",
				   udhcpc_state, wan_ifname, route_setup_status);
	}

	nvram_set_int_temp(strcat_r(prefix, "route_err", tmp), route_setup_status);
	nvram_set_int_temp(strcat_r(prefix, "err", tmp), i_err);

	return route_setup_status;
}

static int
udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)
{
	char *value, *lan_ip, *lan_nm, *ip, *nm, *gw, *rt, *rt_ms, *rt_rfc;
	char log_prefix[32], *udhcpc_state;
	char gateway_buf[INET_ADDRSTRLEN], old_gateway[INET_ADDRSTRLEN];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	unsigned int parsed_value;
	int lease_dur, is_changed, i_err, dhcp_mtu, mtu_seen, route_result;
	int interface_status, rollback_status, route_setup_status, mtu_status;
	int route_cleanup_status, route_rollback_status, multicast_status = 0;
	struct wan_dhcp_interface_context interface_context;
	struct wan_dhcp_route_transition_context route_context;

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	udhcpc_state = (is_renew_mode) ? "renew" : "bound";

	value = getenv("ip");
	ip = value ? trim_r(value) : "";
	value = getenv("subnet");
	nm = value ? trim_r(value) : "255.255.0.0";
	if (!dhcp_lease_ipv4_is_valid(ip) || !dhcp_lease_netmask_is_valid(nm)) {
		logmessage(log_prefix, "%s (%s) rejected invalid DHCP address/netmask",
			udhcpc_state, man_ifname);
		return EINVAL;
	}

	/* Keep the previous lease identity until its route set is removed. */
	snprintf(old_gateway, sizeof(old_gateway), "%s", nvram_safe_get("manv_gateway"));
	snprintf(old_ip, sizeof(old_ip), "%s", nvram_safe_get("manv_ipaddr"));
	snprintf(old_mask, sizeof(old_mask), "%s", nvram_safe_get("manv_netmask"));
	interface_context.ifname = man_ifname;
	is_changed = strcmp(old_ip, ip) != 0;

	gateway_buf[0] = '\0';
	value = getenv("router");
	if (value) {
		route_result = dhcp_lease_ipv4_list_first(trim_r(value),
			gateway_buf, sizeof(gateway_buf));
		if (route_result < 0)
			logmessage(log_prefix, "ignoring invalid DHCP router list on %s", man_ifname);
	}
	gw = gateway_buf;

	value = getenv("routes");
	rt = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV route option on %s", man_ifname);
		rt = "";
	}
	value = getenv("msstaticroutes");
	rt_ms = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt_ms)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV Microsoft route option on %s", man_ifname);
		rt_ms = "";
	}
	value = getenv("staticroutes");
	rt_rfc = value ? trim_r(value) : "";
	if (!dhcp_lease_route_text_is_safe(rt_rfc)) {
		logmessage(log_prefix, "ignoring invalid DHCP IPTV classless route option on %s", man_ifname);
		rt_rfc = "";
	}

	lease_dur = 0;
	dhcp_mtu = 0;
	mtu_seen = 0;
	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value))
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", man_ifname);
		else
			lease_dur = (int)parsed_value;
	}
	value = getenv("mtu");
	if (value) {
		mtu_seen = 1;
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 576U, 1500U, &parsed_value)) {
			dhcp_mtu = 0;
			logmessage(log_prefix, "ignoring invalid DHCP MTU on %s", man_ifname);
		} else {
			dhcp_mtu = (int)parsed_value;
		}
	}

	lan_ip = nvram_safe_get("lan_ipaddr");
	lan_nm = nvram_safe_get("lan_netmask");
	i_err = 0;
	if (is_same_subnet2(ip, lan_ip, nm, lan_nm)) {
		if (!is_renew_mode) {
			interface_status = wan_dhcp_interface_clear(&interface_context);
			if (interface_status != 0) {
				logmessage(log_prefix,
					"failed to clear conflicting IPTV DHCP address on %s (error=%d)",
					man_ifname, interface_status);
				return interface_status;
			}
		}

		i_err = 1;
		logmessage(log_prefix,
			"%s (%s). ERROR! Obtained IP [%s/%s] in LAN subnet [%s/%s]. Need change LAN subnet!",
			udhcpc_state, man_ifname, ip, nm, lan_ip, lan_nm);
	} else {
		/*
		 * Do not publish a lease or discard old routes until the new
		 * interface address is confirmed. Restore the last-known-good
		 * address if either address operation fails.
		 */
		interface_status = dhcp_wan_interface_apply(
			old_ip, old_mask, ip, nm, is_changed,
			&wan_dhcp_interface_ops, &interface_context, &rollback_status);
		if (interface_status != 0) {
			logmessage(log_prefix,
				"failed to apply IPTV DHCP address on %s (error=%d); preserving previous lease and routes",
				man_ifname, interface_status);
			if (rollback_status != 0)
				logmessage(log_prefix,
					"failed to restore previous IPTV DHCP address on %s (error=%d)",
					man_ifname, rollback_status);
			return interface_status;
		}
	}

	/*
	 * Keep the old IPTV lease as a recoverable unit. If any old route cannot
	 * be removed, restore both its address and route inventory before retry.
	 */
	route_context.interface_context = &interface_context;
	route_context.prefix = "manv_";
	route_context.old_gateway = old_gateway;
	route_context.old_ip = old_ip;
	route_context.old_mask = old_mask;
	route_context.route_metric = 10;
	route_context.restore_host_route = 0;
	route_context.remove_dhcp_routes = 1;
	route_cleanup_status = dhcp_wan_route_transition_apply(
		0, dhcp_lease_ipv4_is_valid(old_gateway), 1,
		&wan_dhcp_route_transition_ops, &route_context,
		&route_rollback_status);
	if (route_cleanup_status != 0) {
		logmessage(log_prefix,
			"previous IPTV route teardown incomplete on %s (error=%d); preserving previous lease and route inventory",
			man_ifname, route_cleanup_status);
		if (route_rollback_status != 0)
			logmessage(log_prefix,
				"failed to restore previous IPTV route inventory on %s (error=%d)",
				man_ifname, route_rollback_status);
		return route_cleanup_status;
	}

	nvram_set_temp("manv_ipaddr", ip);
	nvram_set_temp("manv_netmask", nm);
	nvram_set_temp("manv_gateway", gw);
	nvram_set_temp("manv_routes", rt);
	nvram_set_temp("manv_routes_ms", rt_ms);
	nvram_set_temp("manv_routes_rfc", rt_rfc);
	if (mtu_seen)
		nvram_set_int_temp("manv_mtu", dhcp_mtu);

	route_setup_status = 0;
	mtu_status = 0;
	if (!i_err) {
		if (wan_dhcp_mtu_is_supported(dhcp_mtu)) {
			mtu_status = set_interface_mtu(man_ifname, dhcp_mtu);
			if (mtu_status != 0)
				logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)",
					   udhcpc_state, man_ifname, mtu_status);
		}

		logmessage(log_prefix,
			"%s (%s), IP: %s, GW: %s, lease time: %d",
			udhcpc_state, man_ifname, ip, gw, lease_dur);

		if (*rt || *rt_rfc || *rt_ms) {
			route_result = add_dhcp_routes(rt, rt_rfc, rt_ms, man_ifname, 0);
			route_setup_status = route_status_accumulate(route_setup_status, route_result);
		}

		/* Default route via IPTV gateway (metric 10). */
		if (dhcp_lease_ipv4_is_valid(gw)) {
			route_result = route_add(man_ifname, 10, "0.0.0.0", gw, "0.0.0.0");
			route_setup_status = route_status_accumulate(route_setup_status, route_result);
			if (route_result != 0)
				logmessage(log_prefix, "failed to add IPTV default route via %s (error=%d)",
					gw, route_result);
		}

		if (route_setup_status != 0)
			logmessage(log_prefix,
				"%s (%s) accepted but route setup is incomplete (error=%d)",
				udhcpc_state, man_ifname, route_setup_status);
		multicast_status = start_igmpproxy(man_ifname);
		if (multicast_status != 0)
			logmessage(log_prefix,
				"%s (%s) multicast startup is incomplete (error=%d)",
				udhcpc_state, man_ifname, multicast_status);
	}

	/* Keep route retry state independent from the IPTV proxy lifecycle. */
	nvram_set_int_temp("manv_route_err", route_setup_status);
	nvram_set_int_temp("manv_err", i_err);
	return route_status_accumulate(route_setup_status, multicast_status);
}

static int
zcip_bound(char *man_ifname)
{
	char *value, *ip;
	char tmp[100], prefix[16];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char *zeroconf_mask = "255.255.0.0";
	int unit, interface_status, rollback_status, route_setup_status;
	struct wan_dhcp_interface_context interface_context;

	value = getenv("ip");
	if (!value) {
		logmessage(LOGNAME, "ZeroConf received no address on %s", man_ifname);
		return EINVAL;
	}
	ip = trim_r(value);
	if (!dhcp_lease_ipv4_is_valid(ip)) {
		logmessage(LOGNAME, "ZeroConf rejected invalid address on %s", man_ifname);
		return EINVAL;
	}
	snprintf(old_ip, sizeof(old_ip), "%s", nvram_safe_get("wanx_ipaddr"));
	snprintf(old_mask, sizeof(old_mask), "%s", nvram_safe_get("wanx_netmask"));
	interface_context.ifname = man_ifname;
	interface_status = dhcp_wan_interface_apply(old_ip, old_mask, ip, zeroconf_mask, 1,
		&wan_dhcp_interface_ops, &interface_context, &rollback_status);
	if (interface_status != 0) {
		logmessage(LOGNAME, "failed to apply ZeroConf address on %s (error=%d)", man_ifname, interface_status);
		if (rollback_status != 0)
			logmessage(LOGNAME, "failed to restore previous ZeroConf WAN address on %s (error=%d)", man_ifname, rollback_status);
		return interface_status;
	}

	strcpy(prefix, "wanx_");
	nvram_set_temp(strcat_r(prefix, "netmask", tmp), zeroconf_mask);
	nvram_set_temp(strcat_r(prefix, "gateway", tmp), "");
	nvram_set_temp(strcat_r(prefix, "dns", tmp), "");
	nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), ip);

	unit = get_wan_unit(man_ifname);
	route_setup_status = man_up_internal(man_ifname, unit, 0, NULL);
	if (route_setup_status != 0)
		logmessage(LOGNAME, "ZeroConf address on %s is active but MAN route setup is incomplete (error=%d)", man_ifname, route_setup_status);
	return route_setup_status;
}

static int
zcip_viptv_bound(char *man_ifname)
{
	char *value, *ip;
	char tmp[100], prefix[16];
	char old_ip[INET_ADDRSTRLEN], old_mask[INET_ADDRSTRLEN];
	char *zeroconf_mask = "255.255.0.0";
	int interface_status, rollback_status;
	struct wan_dhcp_interface_context interface_context;

	value = getenv("ip");
	if (!value) {
		logmessage(LOGNAME, "ZeroConf IPTV received no address on %s", man_ifname);
		return EINVAL;
	}
	ip = trim_r(value);
	if (!dhcp_lease_ipv4_is_valid(ip)) {
		logmessage(LOGNAME, "ZeroConf IPTV rejected invalid address on %s", man_ifname);
		return EINVAL;
	}
	snprintf(old_ip, sizeof(old_ip), "%s", nvram_safe_get("manv_ipaddr"));
	snprintf(old_mask, sizeof(old_mask), "%s", nvram_safe_get("manv_netmask"));
	interface_context.ifname = man_ifname;
	interface_status = dhcp_wan_interface_apply(old_ip, old_mask, ip, zeroconf_mask, 1,
		&wan_dhcp_interface_ops, &interface_context, &rollback_status);
	if (interface_status != 0) {
		logmessage(LOGNAME, "failed to apply ZeroConf IPTV address on %s (error=%d)", man_ifname, interface_status);
		if (rollback_status != 0)
			logmessage(LOGNAME, "failed to restore previous ZeroConf IPTV address on %s (error=%d)", man_ifname, rollback_status);
		return interface_status;
	}

	strcpy(prefix, "manv_");
	nvram_set_temp(strcat_r(prefix, "netmask", tmp), zeroconf_mask);
	nvram_set_temp(strcat_r(prefix, "gateway", tmp), "");
	nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), ip);
	return start_igmpproxy(man_ifname);
}

static int
udhcpc_renew(char *wan_ifname)
{
	char *param, *value, *lease_dns, *lease_wins, *lease_domain;
	char tmp[100], prefix[16], log_prefix[32], gateway_buf[INET_ADDRSTRLEN];
	unsigned int parsed_value;
	int unit, is_man, is_changed;

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);

	if (is_man) {
		strcpy(prefix, "wanx_");
	} else {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		snprintf(tmp, sizeof(tmp), "%ld", uptime());
		set_wan_unit_value(unit, "dltime", tmp);
	}

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");

	if (dhcp_wan_route_setup_retry_required(
		nvram_get_int(strcat_r(prefix, "route_err", tmp)))) {
		logmessage(log_prefix, "retrying incomplete DHCP route setup on %s", wan_ifname);
		return udhcpc_bound(wan_ifname, 1);
	}

	value = getenv("subnet");
	if (!value || nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return udhcpc_bound(wan_ifname, 1);

	value = getenv("router");
	if (dhcp_lease_router_is_changed(
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
		value ? trim_r(value) : NULL,
		gateway_buf, sizeof(gateway_buf)))
		return udhcpc_bound(wan_ifname, 1);

	value = getenv("ip");
	if (!value || nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), trim_r(value)))
		return udhcpc_bound(wan_ifname, 1);

	/* Changed route options must re-run WAN setup after removing the old routes. */
	value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch(strcat_r(prefix, "routes", tmp), value))
		return udhcpc_bound(wan_ifname, 1);

	value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch(strcat_r(prefix, "routes_ms", tmp), value))
		return udhcpc_bound(wan_ifname, 1);

	value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch(strcat_r(prefix, "routes_rfc", tmp), value))
		return udhcpc_bound(wan_ifname, 1);

	value = getenv("mtu");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 576U, 1500U, &parsed_value))
			parsed_value = 0;
		if (nvram_get_int(strcat_r(prefix, "mtu", tmp)) != (int)parsed_value)
			return udhcpc_bound(wan_ifname, 1);
	}

	is_changed = 0;
	value = getenv("dns");
	lease_dns = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_dns, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP DNS list on %s", wan_ifname);
		lease_dns = "";
	}
	param = strcat_r(prefix, "dns", tmp);
	if (nvram_invmatch(param, lease_dns)) {
		nvram_set_temp(param, lease_dns);
		is_changed = 1;
	}

	value = getenv("domain");
	lease_domain = value ? trim_r(value) : "";
	if (*lease_domain && !dnsmasq_valid_server_domain(lease_domain)) {
		logmessage(log_prefix, "ignoring invalid DHCP domain on %s", wan_ifname);
		lease_domain = "";
	}
	param = strcat_r(prefix, "domain", tmp);
	if (nvram_invmatch(param, lease_domain)) {
		nvram_set_temp(param, lease_domain);
		/* Refresh dnsmasq domain-specific server routing too. */
		is_changed = 1;
	}

	value = getenv("wins");
	lease_wins = value ? trim_r(value) : "";
	if (!dhcp_lease_ipv4_list_is_valid(lease_wins, 255, 16, 1)) {
		logmessage(log_prefix, "ignoring invalid DHCP WINS list on %s", wan_ifname);
		lease_wins = "";
	}
	nvram_set_temp(strcat_r(prefix, "wins", tmp), lease_wins);

	value = getenv("lease");
	if (value) {
		value = trim_r(value);
		if (dhcp_lease_decimal_is_valid(value, 0U, 2147483647U, &parsed_value))
			nvram_set_int_temp(strcat_r(prefix, "lease", tmp), (int)parsed_value);
		else
			logmessage(log_prefix, "ignoring invalid DHCP lease duration on %s", wan_ifname);
	}

	if (is_changed) {
		logmessage(log_prefix, "renew (%s), DNS/domain configuration changed", wan_ifname);
		refresh_resolvconf_nonfatal("WAN DHCP renewal", 0);
	}

	return 0;
}

static int
udhcpc_viptv_renew(char *man_ifname)
{
	char *value;
	char tmp[100], prefix[16], gateway_buf[INET_ADDRSTRLEN];
	unsigned int parsed_value;

	strcpy(prefix, "manv_");

	if (dhcp_wan_route_setup_retry_required(
		nvram_get_int(strcat_r(prefix, "route_err", tmp))))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("subnet");
	if (value && nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("router");
	if (dhcp_lease_router_is_changed(nvram_safe_get("manv_gateway"),
		value ? trim_r(value) : NULL, gateway_buf, sizeof(gateway_buf)))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("ip");
	if (value && nvram_invmatch("manv_ipaddr", trim_r(value)))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("routes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch("manv_routes", value))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("msstaticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch("manv_routes_ms", value))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = safe_getenv("staticroutes");
	if (!dhcp_lease_route_text_is_safe(value))
		value = "";
	if (nvram_invmatch("manv_routes_rfc", value))
		return udhcpc_viptv_bound(man_ifname, 1);

	value = getenv("mtu");
	if (value) {
		value = trim_r(value);
		if (!dhcp_lease_decimal_is_valid(value, 576U, 1500U, &parsed_value))
			parsed_value = 0;
		if (nvram_get_int("manv_mtu") != (int)parsed_value)
			return udhcpc_viptv_bound(man_ifname, 1);
	}

	return 0;
}

static int
udhcpc_leasefail(char *wan_ifname)
{
	/* DHCP failed for INET */
	return 0;
}

static int
udhcpc_viptv_leasefail(char *man_ifname)
{
	/* DHCP failed for IPTV */
	return 0;
}

static int
udhcpc_noack(char *wan_ifname)
{
	int unit, is_man;
	char log_prefix[32];

	unit = get_wan_unit(wan_ifname);
	is_man = is_ifunit_man(wan_ifname, unit);

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	logmessage(log_prefix, "Received NAK for %s", wan_ifname);
	return 0;
}

static int
udhcpc_viptv_noack(char *man_ifname)
{
	char log_prefix[32];

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	logmessage(log_prefix, "Received NAK for %s", man_ifname);
	return 0;
}

int
udhcpc_main(int argc, char **argv)
{
	int ret = 0;
	char *wan_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(wan_ifname))
		return EINVAL;

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(wan_ifname, 0);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_bound(wan_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_renew(wan_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_leasefail(wan_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_noack(wan_ifname);

	return ret;
}

static int
manv_route_deconfig(char *man_ifname)
{
	char old_gateway[INET_ADDRSTRLEN];
	int route_status = 0, route_result;

	snprintf(old_gateway, sizeof(old_gateway), "%s", nvram_safe_get("manv_gateway"));
	route_result = del_dhcp_routes_by_prefix("manv_", man_ifname, 0);
	route_status = route_status_accumulate(route_status, route_result);
	if (route_result != 0)
		logmessage(LOGNAME, "failed to delete IPTV DHCP routes on %s (error=%d)", man_ifname, route_result);
	if (dhcp_lease_ipv4_is_valid(old_gateway)) {
		route_result = route_del(man_ifname, 10, "0.0.0.0", old_gateway, "0.0.0.0");
		if (!dhcp_lease_route_delete_is_ok(route_result)) {
			route_status = route_status_accumulate(route_status, route_result);
			logmessage(LOGNAME, "failed to delete IPTV default route via %s (error=%d)", old_gateway, route_result);
		}
	}
	if (route_status == 0) {
		nvram_set_temp("manv_gateway", "");
		nvram_set_temp("manv_routes", "");
		nvram_set_temp("manv_routes_ms", "");
		nvram_set_temp("manv_routes_rfc", "");
	} else {
		logmessage(LOGNAME, "IPTV DHCP route teardown incomplete on %s (error=%d); preserving route inventory", man_ifname, route_status);
	}
	return route_status;
}

struct udhcpc_viptv_deconfig_context {
	char *ifname;
};

static int
udhcpc_viptv_deconfig_clear_address(void *opaque)
{
	struct udhcpc_viptv_deconfig_context *context =
		(struct udhcpc_viptv_deconfig_context *)opaque;

	return ifconfig(context->ifname, IFUP, "0.0.0.0", NULL);
}

static int
udhcpc_viptv_deconfig_delete_routes(void *opaque)
{
	struct udhcpc_viptv_deconfig_context *context =
		(struct udhcpc_viptv_deconfig_context *)opaque;

	return manv_route_deconfig(context->ifname);
}

static void
udhcpc_viptv_deconfig_publish_empty_address(void *opaque)
{
	(void)opaque;
	nvram_set_temp("manv_ipaddr", "0.0.0.0");
}

static int
udhcpc_viptv_deconfig(char *man_ifname)
{
	struct udhcpc_viptv_deconfig_context context;
	static const struct dhcp_lease_viptv_deconfig_ops ops = {
		udhcpc_viptv_deconfig_clear_address,
		udhcpc_viptv_deconfig_delete_routes,
		udhcpc_viptv_deconfig_publish_empty_address
	};
	int status, multicast_status;

	context.ifname = man_ifname;
	status = dhcp_lease_viptv_deconfig_apply(&ops, &context);
	if (status != 0)
		logmessage(LOGNAME,
			   "IPTV DHCP deconfig incomplete on %s (status=%d); retaining lease metadata for retry",
			   man_ifname, status);

	/* Always stop multicast service, even when address or route cleanup failed. */
	multicast_status = stop_igmpproxy(man_ifname);
	if (multicast_status != 0)
		logmessage(LOGNAME,
			   "IPTV DHCP multicast teardown incomplete on %s (status=%d)",
			   man_ifname, multicast_status);
	return route_status_accumulate(status, multicast_status);
}

int
udhcpc_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;
	umask(0000);
	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_viptv_deconfig(man_ifname);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_viptv_bound(man_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_viptv_renew(man_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_viptv_leasefail(man_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_viptv_noack(man_ifname);
	return ret;
}

int
zcip_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(man_ifname, 1);
	else if (!strcmp(argv[1], "config"))
		ret = zcip_bound(man_ifname);

	return ret;
}

static int
zcip_viptv_deconfig(char *man_ifname)
{
	int interface_status, route_status, multicast_status;

	interface_status = ifconfig(man_ifname, IFUP, "0.0.0.0", NULL);
	if (interface_status != 0)
		logmessage(LOGNAME, "failed to clear ZeroConf IPTV address on %s (error=%d)", man_ifname, interface_status);
	route_status = manv_route_deconfig(man_ifname);
	if (interface_status == 0 && route_status == 0) {
		nvram_set_temp("manv_ipaddr", "0.0.0.0");
	} else {
		logmessage(LOGNAME, "ZeroConf IPTV deconfig incomplete on %s (interface=%d route=%d); preserving address identity", man_ifname, interface_status, route_status);
	}
	multicast_status = stop_igmpproxy(man_ifname);
	if (multicast_status != 0)
		logmessage(LOGNAME, "ZeroConf IPTV multicast teardown incomplete on %s (error=%d)",
			   man_ifname, multicast_status);
	return route_status_accumulate(
		route_status_accumulate(interface_status, route_status),
		multicast_status);
}

int
zcip_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname = safe_getenv("interface");

	if (argc < 2 || !argv[1] || !dhcp_wan_interface_name_is_valid(man_ifname))
		return EINVAL;
	umask(0000);
	if (!strcmp(argv[1], "deconfig"))
		ret = zcip_viptv_deconfig(man_ifname);
	else if (!strcmp(argv[1], "config"))
		ret = zcip_viptv_bound(man_ifname);
	return ret;
}

int start_udhcpc_wan(char *wan_ifname, int unit, int wait_lease)
{
	int index, is_man;
	char log_prefix[32], pidfile[32] = {0};
	char *wan_hostname, *wan_vci;
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", wan_ifname,
		"-s", SCRIPT_UDHCPC_WAN,
		"-p", pidfile,
		"-t4",
		"-T4",
		NULL,
		NULL, NULL,	/* -x hostname:hostname */
		NULL, NULL,	/* -V vendorclass	*/
		NULL,		/* -O mtu		*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
#if defined (USE_IPV6)
		NULL,		/* -O 6rd		*/
		NULL,		/* -O comcast6rd	*/
#endif
		NULL
	};

	index = 9;		/* first NULL index	*/

	snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);

	if (is_man_wisp(wan_ifname) || is_usbnet_interface(wan_ifname))
		dhcp_argv[7] = "-t6";

	if (wait_lease)
		dhcp_argv[index++] = "-b";	/* Background if lease is not obtained (timeout 4*4 sec) */
	else
		dhcp_argv[index++] = "-d";	/* Background after run (new patch for udhcpc) */

	wan_hostname = get_wan_unit_value(unit, "hostname");
	if (strlen(wan_hostname) > 0) {
		dhcp_argv[index++] = "-H";
		dhcp_argv[index++] = sanity_hostname(wan_hostname);
	}

	wan_vci = get_wan_unit_value(unit, "vci");
	if (strlen(wan_vci) > 0) {
		dhcp_argv[index++] = "-V";
		dhcp_argv[index++] = wan_vci;
	}

	dhcp_argv[index++] = "-O26";	/* "mtu" */

	if (nvram_match("dr_enable_x", "1")) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
	}

#if defined (USE_IPV6)
	if (get_ipv6_type() == IPV6_6RD) {
		dhcp_argv[index++] = "-O212";	/* "6rd" */
		dhcp_argv[index++] = "-O150";	/* "comcast6rd" */
	}
#endif

	is_man = is_ifunit_man(wan_ifname, unit);
	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", (is_man) ? "MAN" : "WAN");
	logmessage(log_prefix, "starting on %s ...", wan_ifname);

	return _eval(dhcp_argv, NULL, 0, NULL);
}

int start_udhcpc_viptv(char *man_ifname)
{
	int index;
	char log_prefix[32], pidfile[32] = {0};
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", man_ifname,
		"-s", SCRIPT_UDHCPC_VIPTV,
		"-p", pidfile,
		"-t4",
		"-T4",
		"-d",		/* Background after run (new patch for udhcpc) */
		NULL,		/* -O mtu		*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
		NULL
	};

	index = 10;		/* first NULL index	*/

	snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc_viptv.pid");

	dhcp_argv[index++] = "-O26";	/* "mtu" */

	if (nvram_match("dr_enable_x", "1")) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
	}

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "DHCP", "IPTV");
	logmessage(log_prefix, "starting on %s ...", man_ifname);

	return _eval(dhcp_argv, NULL, 0, NULL);
}

int start_zcip_wan(char *wan_ifname)
{
	return eval("/sbin/zcip", wan_ifname, SCRIPT_ZCIP_WAN);
}

int start_zcip_viptv(char *man_ifname)
{
	char log_prefix[32];

	snprintf(log_prefix, sizeof(log_prefix), "%s %s Client", "ZeroConf", "IPTV");
	logmessage(log_prefix, "starting on %s ...", man_ifname);

	return eval("/sbin/zcip", "-q", man_ifname, SCRIPT_ZCIP_VIPTV);
}

static int
stop_udhcpc_pidfile(const char *path)
{
	const char *pidfiles[2];

	pidfiles[0] = path;
	pidfiles[1] = NULL;
	/* Let BusyBox complete its pending-deconfig retry; never SIGKILL it. */
	return kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0);
}

int renew_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return process_pidfile_signal(pidfile, "udhcpc", SIGUSR1);
}

int release_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return process_pidfile_signal(pidfile, "udhcpc", SIGUSR2);
}

int stop_udhcpc_wan(int unit)
{
	char pidfile[32];
	int length;

	length = snprintf(pidfile, sizeof(pidfile), "/var/run/udhcpc%d.pid", unit);
	if (length < 0 || length >= (int)sizeof(pidfile)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return stop_udhcpc_pidfile(pidfile);
}

int stop_udhcpc_viptv(void)
{
	return stop_udhcpc_pidfile("/var/run/udhcpc_viptv.pid");
}
