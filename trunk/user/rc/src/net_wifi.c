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
#include <sys/types.h>
#include <net/if.h>
#include <time.h>

#include "rc.h"
#include "wireless_restart_policy.h"
#include "wireless_radio_policy.h"
#include "wan_nvram_status_policy.h"
#include "wan_ifname_policy.h"
#include "service_wait_policy.h"
#include "wireless_guest_isolation_policy.h"
#include "switch.h"
#include "gpio_pins.h"
#include <gpioutils.h>

static int
wif_control(const char *wifname, int is_up)
{
	int ret;
	logmessage(LOGNAME, "%s: ifname: %s, isup: %d", __func__, wifname, is_up);
	ret = ifconfig(wifname, is_up ? IFUP : 0, NULL, NULL);
#if defined (USE_MT7615_AP) || defined (USE_MT7915_AP) || defined (USE_MT76X2_AP)
	/* Register only an interface that was actually brought up. */
	if (ret == 0 && is_up && is_module_loaded("hw_nat")) {
		int hw_nat_status = doSystem("iwpriv %s set hw_nat_register=%d",
					     wifname, 1);

		if (hw_nat_status != 0)
			logmessage(LOGNAME,
				   "%s: hardware NAT registration failed for %s (status=%d)",
				   __func__, wifname, hw_nat_status);
		ret = wireless_radio_status_merge(ret, hw_nat_status);
	}
#endif
	return ret;
}

/*
 * Do not publish a scheduler transition when the interface command failed.
 * The caller must preserve its current bridge/isolation state for retry.
 */
static int
wif_control_success(const char *wifname, int is_up)
{
	int ret = wif_control(wifname, is_up);

	if (!wireless_radio_status_success(ret)) {
		logmessage(LOGNAME, "%s: failed to set interface %s %s (status=%d)",
			   __func__, wifname, is_up ? "up" : "down", ret);
		return 0;
	}

	return 1;
}


typedef struct {
	const char *ifname_ap;
	const char *bridge_ifname;
} wireless_guest_rollback_context_t;

static int
wireless_guest_interface_state_checked(const char *ifname, int *is_up)
{
	int interface_flags;
	int interface_error;

	if (!ifname || !is_up)
		return -EINVAL;

	errno = 0;
	interface_flags = get_interface_flags(ifname);
	interface_error = errno;
	return wireless_guest_isolation_link_state(interface_flags, interface_error,
						   ENODEV, IFF_UP, is_up);
}

static int
wireless_guest_rollback_bring_down(void *context)
{
	wireless_guest_rollback_context_t *rollback =
		(wireless_guest_rollback_context_t *)context;
	int is_up;
	int status;

	status = wireless_guest_interface_state_checked(rollback->ifname_ap,
							 &is_up);
	if (!wireless_guest_rollback_should_request_down(status, is_up))
		return 0;
	if (!wif_control_success(rollback->ifname_ap, 0))
		return status != 0 ? status : -EIO;
	/*
	 * A failed flags probe is UNKNOWN, not DOWN. Still request the interface
	 * down; the rollback's independent postcondition probe decides whether
	 * the fail-closed transition actually completed.
	 */
	return 0;
}

static int
wireless_guest_rollback_detach_bridge(void *context)
{
	wireless_guest_rollback_context_t *rollback =
		(wireless_guest_rollback_context_t *)context;

	return br_add_del_if(IFNAME_BR, rollback->bridge_ifname, 0);
}

static int
wireless_guest_rollback_verify_down(void *context)
{
	wireless_guest_rollback_context_t *rollback =
		(wireless_guest_rollback_context_t *)context;
	int is_up;
	int status;

	status = wireless_guest_interface_state_checked(rollback->ifname_ap,
							 &is_up);
	if (status != 0)
		return status;
	return is_up ? -EBUSY : 0;
}

static void
wireless_guest_ap_rollback(const char *ifname_ap, const char *bridge_ifname,
			   wireless_guest_isolation_status_t *status)
{
	static const wireless_guest_isolation_rollback_ops_t ops = {
		wireless_guest_rollback_bring_down,
		wireless_guest_rollback_detach_bridge,
		wireless_guest_rollback_verify_down
	};
	wireless_guest_rollback_context_t context;
	unsigned int failures_before;

	if (!status)
		return;
	context.ifname_ap = ifname_ap;
	context.bridge_ifname = bridge_ifname;
	failures_before = status->failure_count;
	wireless_guest_isolation_status_rollback(status, &ops, &context);
	if (status->failure_count > failures_before)
		logmessage(LOGNAME,
			   "guest AP fail-closed rollback for %s had %u cleanup failure(s)",
			   ifname_ap, status->failure_count - failures_before);
}

static const char *
wireless_guest_rt_bridge_ifname(int is_ap_mode)
{
#if defined(USE_RT3352_MII)
	if (!is_ap_mode)
		return IFNAME_INIC_GUEST_VLAN;
#else
	(void)is_ap_mode;
#endif
	return IFNAME_2G_GUEST;
}

static const char *
wireless_guest_rt_interface_ifname(void)
{
#if defined(USE_RT3352_MII)
	return IFNAME_INIC_GUEST;
#else
	return IFNAME_2G_GUEST;
#endif
}

int
wireless_guest_rt_guest_is_up(int *is_up)
{
	return wireless_guest_interface_state_checked(
		wireless_guest_rt_interface_ifname(), is_up);
}

int
wireless_guest_wl_guest_is_up(int *is_up)
{
#if BOARD_HAS_5G_RADIO
	return wireless_guest_interface_state_checked(IFNAME_5G_GUEST, is_up);
#else
	if (!is_up)
		return -EINVAL;
	*is_up = 0;
	return 0;
#endif
}

/*
 * init_bridge() starts AP radios directly rather than through restart_wifi_*().
 * If startup or final isolation fails, remove only guest interfaces that were
 * down before init_bridge() and are up now.
 */
int
wireless_guest_init_bridge_fail_closed(int rt_guest_was_up,
				       int wl_guest_was_up,
				       int rt_guest_start_attempted,
				       int wl_guest_start_attempted,
				       int startup_failed)
{
	wireless_guest_isolation_status_t status;
	int cleanup_status;
	int rollback_rt;
	int rollback_wl;

	rollback_rt = wireless_guest_startup_rollback_candidate(
		rt_guest_was_up, rt_guest_start_attempted);
	rollback_wl = wireless_guest_startup_rollback_candidate(
		wl_guest_was_up, wl_guest_start_attempted);
	if (!startup_failed || (!rollback_rt && !rollback_wl))
		return 0;

	wireless_guest_isolation_status_reset(&status);
#if BOARD_HAS_5G_RADIO
	if (rollback_wl)
		wireless_guest_ap_rollback(IFNAME_5G_GUEST, IFNAME_5G_GUEST, &status);
#else
	(void)rollback_wl;
#endif
	if (rollback_rt)
		wireless_guest_ap_rollback(wireless_guest_rt_interface_ifname(),
					   wireless_guest_rt_bridge_ifname(get_ap_mode()),
					   &status);

	cleanup_status = restart_guest_lan_isolation();
	wireless_guest_isolation_status_record(&status,
		"reconcile guest isolation after LAN bridge startup rollback",
		cleanup_status);
	if (status.first_status != 0)
		logmessage(LOGNAME,
			   "LAN bridge guest AP rollback incomplete: %s failed first (status=%d; failed_operations=%u)",
			   status.first_operation ? status.first_operation : "unknown",
			   status.first_status, status.failure_count);
	return wireless_guest_isolation_status_result(&status);
}

static int
wireless_guest_restart_fail_closed(int restart_5g, int restart_peer,
				   int isolation_status)
{
	wireless_guest_isolation_status_t status;
	int cleanup_status;

	wireless_guest_isolation_status_reset(&status);
	wireless_guest_isolation_status_record(&status,
		"guest isolation after wireless restart", isolation_status);
	if (restart_5g) {
#if BOARD_HAS_5G_RADIO
		wireless_guest_ap_rollback(IFNAME_5G_GUEST, IFNAME_5G_GUEST, &status);
#endif
#if defined(BOARD_MT7615_DBDC) || defined(BOARD_MT7915_DBDC)
		if (restart_peer)
			wireless_guest_ap_rollback(wireless_guest_rt_interface_ifname(),
				wireless_guest_rt_bridge_ifname(get_ap_mode()), &status);
#endif
	} else {
		wireless_guest_ap_rollback(IFNAME_2G_GUEST,
			wireless_guest_rt_bridge_ifname(get_ap_mode()), &status);
#if defined(BOARD_MT7615_DBDC) || defined(BOARD_MT7915_DBDC)
		if (restart_peer)
			wireless_guest_ap_rollback(IFNAME_5G_GUEST, IFNAME_5G_GUEST, &status);
#endif
	}
#if !defined(BOARD_MT7615_DBDC) && !defined(BOARD_MT7915_DBDC)
	(void)restart_peer;
#endif
	cleanup_status = restart_guest_lan_isolation();
	wireless_guest_isolation_status_record(&status,
		"reconcile guest isolation after wireless rollback", cleanup_status);
	if (status.first_status != 0)
		logmessage(LOGNAME,
			   "wireless restart guest isolation rollback incomplete: %s failed first (status=%d; failed_operations=%u)",
			   status.first_operation ? status.first_operation : "unknown",
			   status.first_status, status.failure_count);
	return wireless_guest_isolation_status_result(&status);
}

/*
 * Keep an interface in its current bridge while it is still up. Once the
 * driver accepted the down request, detach its LAN bridge port as part of
 * the same shutdown result so restart cannot inherit stale WDS/AP memberships.
 */
static int
wireless_stop_interface(const char *ifname, const char *bridge_ifname)
{
	int ret;

	if (!wif_control_success(ifname, 0))
		return 0;

	ret = br_add_del_if(IFNAME_BR, bridge_ifname, 0);
	if (ret != 0) {
		logmessage(LOGNAME, "%s: failed to detach %s from %s after shutdown (status=%d)",
			   __func__, bridge_ifname, IFNAME_BR, ret);
		return 0;
	}

	return 1;
}

static void
wireless_bridge_membership_record(int *all_started, const char *ifname, int is_add)
{
	int status = br_add_del_if(IFNAME_BR, ifname, is_add);

	if (status != 0)
		logmessage(LOGNAME, "%s: failed to %s wireless interface %s %s (status=%d)",
			   __func__, is_add ? "attach" : "detach", ifname, IFNAME_BR, status);

	*all_started = wireless_radio_startup_status(
		*all_started, wireless_radio_status_success(status));
}

void
mlme_state_wl(int is_on)
{
	nvram_set_int("mlme_radio_wl", is_on);
}

void
mlme_state_rt(int is_on)
{
	nvram_set_int("mlme_radio_rt", is_on);
}

static int
mlme_radio_wl_apply(int is_on)
{
#if BOARD_HAS_5G_RADIO
	const char *wifname = find_wlan_if_up(1);
	int ret;

	if (!wifname)
		return 0;

	ret = doSystem("iwpriv %s set %s=%d", wifname, "RadioOn", (is_on) ? 1 : 0);
	if (!wireless_radio_status_success(ret)) {
		logmessage(LOGNAME, "5GHz RadioOn=%d failed (status %d)", (is_on) ? 1 : 0, ret);
		return 0;
	}
#endif
	mlme_state_wl(is_on);
	LED_CONTROL(LED_SW5G, (is_on) ? LED_ON : LED_OFF);
	return 1;
}

#if defined(USE_RT3352_MII)
typedef struct wireless_inic_radio_context {
	const char *ifname;
} wireless_inic_radio_context_t;

static int
wireless_inic_radio_set_power(void *opaque, int is_on)
{
	wireless_inic_radio_context_t *context = (wireless_inic_radio_context_t *)opaque;
	int status = doSystem("iwpriv %s set %s=%d", context->ifname, "RadioOn", !!is_on);

	if (!wireless_radio_status_success(status))
		logmessage(LOGNAME, "2.4GHz iNIC RadioOn=%d failed (status=%d)", !!is_on, status);
	return status;
}

static int
wireless_inic_radio_set_isolation(void *opaque, int isolated)
{
	int status = phy_isolate_inic((unsigned int)!!isolated);

	(void)opaque;
	if (status != 0)
		logmessage(LOGNAME, "2.4GHz iNIC isolation=%d failed (status=%d)", !!isolated, status);
	return status;
}

static int
wireless_inic_radio_prepare_on(void *opaque)
{
	wireless_inic_radio_context_t *context = (wireless_inic_radio_context_t *)opaque;
	int i_val = wireless_txpower_config_value(nvram_wlan_get_int(0, "TxPower"));
	int status = doSystem("iwpriv %s set %s=%d", context->ifname, "TxPower", i_val);

	if (!wireless_radio_status_success(status)) {
		logmessage(LOGNAME, "2.4GHz iNIC TxPower=%d update failed (status=%d)", i_val, status);
		return status;
	}
	return 0;
}

static int
wireless_inic_radio_publish(void *opaque, int is_on)
{
	int status;

	(void)opaque;
	status = nvram_set_int("mlme_radio_rt", !!is_on);
	if (status != 0) {
		logmessage(LOGNAME,
			   "failed to publish 2.4GHz iNIC radio state %d (status=%d)",
			   !!is_on, status);
		return status;
	}
	LED_CONTROL(LED_SW2G, is_on ? LED_ON : LED_OFF);
	return 0;
}

static const wireless_inic_radio_ops_t wireless_inic_radio_ops = {
	wireless_inic_radio_set_power,
	wireless_inic_radio_set_isolation,
	wireless_inic_radio_prepare_on,
	wireless_inic_radio_publish
};
#endif

static int
mlme_radio_rt_apply(int is_on)
{
	const char *wifname = find_wlan_if_up(0);
	int ret;
#if defined(USE_RT3352_MII)
	wireless_inic_radio_context_t context;
#endif

	if (!wifname)
		return 0;

#if defined(USE_RT3352_MII)
	/*
	 * The iNIC switch port is a security boundary. Keep it isolated while
	 * changing radio power and publish MLME/LED state only after the switch
	 * confirms the requested isolation state.
	 */
	context.ifname = wifname;
	ret = wireless_inic_radio_transition(is_on, &wireless_inic_radio_ops, &context);
	if (ret != 0) {
		logmessage(LOGNAME, "2.4GHz iNIC radio transition incomplete (requested=%d, status=%d)",
			   !!is_on, ret);
		return 0;
	}
#else
	ret = doSystem("iwpriv %s set %s=%d", wifname, "RadioOn", (is_on) ? 1 : 0);
	if (!wireless_radio_status_success(ret)) {
		logmessage(LOGNAME, "2.4GHz RadioOn=%d failed (status %d)", (is_on) ? 1 : 0, ret);
		return 0;
	}

	mlme_state_rt(is_on);
	LED_CONTROL(LED_SW2G, (is_on) ? LED_ON : LED_OFF);
#endif
	return 1;
}

void
mlme_radio_wl(int is_on)
{
	(void)mlme_radio_wl_apply(is_on);
}

void
mlme_radio_rt(int is_on)
{
	(void)mlme_radio_rt_apply(is_on);
}

int
get_mlme_radio_wl(void)
{
	return nvram_get_int("mlme_radio_wl");
}

int
get_mlme_radio_rt(void)
{
	return nvram_get_int("mlme_radio_rt");
}

int
get_enabled_radio_wl(void)
{
	return nvram_wlan_get_int(1, "radio_x");
}

int
get_enabled_radio_rt(void)
{
	return nvram_wlan_get_int(0, "radio_x");
}

int
get_enabled_guest_wl(void)
{
	return nvram_wlan_get_int(1, "guest_enable");
}

int
get_enabled_guest_rt(void)
{
	return nvram_wlan_get_int(0, "guest_enable");
}

int
get_mode_radio_wl(void)
{
	return nvram_wlan_get_int(1, "mode_x");
}

int
get_mode_radio_rt(void)
{
	return nvram_wlan_get_int(0, "mode_x");
}

int
is_apcli_wisp_wl(void)
{
	return nvram_wlan_get_int(1, "sta_wisp");
}

int
is_apcli_wisp_rt(void)
{
	return nvram_wlan_get_int(0, "sta_wisp");
}

int
get_apcli_sta_auto(int is_aband)
{
	int i_sta_auto = 0;

#if defined(USE_RT3352_MII)
	// iNIC not support ApCliAutoConnect
	if (is_aband)
#endif
		i_sta_auto = nvram_wlan_get_int(is_aband, "sta_auto");

	return i_sta_auto;
}

char *
get_apcli_wisp_ifname(void)
{
	int i_mode_x;

#if !defined(USE_RT3352_MII)
	i_mode_x = get_mode_radio_rt();
	if (get_enabled_radio_rt() && (i_mode_x == 3 || i_mode_x == 4) && is_apcli_wisp_rt() &&
	   (strlen(nvram_wlan_get(0, "sta_ssid")) > 0))
		return IFNAME_2G_APCLI;
#endif
#if BOARD_HAS_5G_RADIO
	i_mode_x = get_mode_radio_wl();
	if (get_enabled_radio_wl() && (i_mode_x == 3 || i_mode_x == 4) && is_apcli_wisp_wl() &&
	   (strlen(nvram_wlan_get(1, "sta_ssid")) > 0))
		return IFNAME_5G_APCLI;
#endif
	return NULL;
}

static int
check_apcli_wan(int is_5g, int radio_on)
{
	int man_id, wisp_id;
	int status = 0, ap_mode_status, is_ap_mode, ifname_status;
	char man_ifname[16], man_ifname_nvram[16];
	char *wisp_ifname;

	ap_mode_status = get_ap_mode_status(&is_ap_mode);
	if (ap_mode_status != 0) {
		errno = ap_mode_status;
		logmessage(LOGNAME,
			   "cannot decide AP-client WAN recovery without verified router mode (status=%d)",
			   ap_mode_status);
		return ap_mode_status;
	}
	if (is_ap_mode)
		return 0;

	is_5g &= 1;

	/*
	 * Compare the saved MAN interface with the active AP-client WISP
	 * interface only after the relevant NVRAM state is read successfully.
	 * nvram_safe_get() can otherwise turn an I/O failure into an empty name
	 * and spuriously request a full WAN restart.
	 */
	ifname_status = get_wan_unit_value_copy_status(
		0, "ifname", man_ifname_nvram, sizeof(man_ifname_nvram));
	if (ifname_status != 0) {
		errno = ifname_status;
		logmessage(LOGNAME,
			   "cannot decide AP-client WAN recovery without the saved interface name (status=%d)",
			   ifname_status);
		return ifname_status;
	}
	if (man_ifname_nvram[0]) {
		ifname_status = wan_ifname_copy(
			man_ifname, sizeof(man_ifname), man_ifname_nvram);
		if (ifname_status != 0) {
			errno = ifname_status;
			logmessage(LOGNAME,
				   "cannot decide AP-client WAN recovery from an invalid interface name (status=%d)",
				   ifname_status);
			return ifname_status;
		}
	} else {
		man_ifname[0] = '\0';
	}

	man_id = -1;
	if (strcmp(man_ifname, IFNAME_2G_APCLI) == 0)
		man_id = 0;
#if BOARD_HAS_5G_RADIO
	else if (strcmp(man_ifname, IFNAME_5G_APCLI) == 0)
		man_id = 1;
#endif

	wisp_id = -1;
	wisp_ifname = get_apcli_wisp_ifname();
	if (wisp_ifname) {
		if (strcmp(wisp_ifname, IFNAME_2G_APCLI) == 0)
			wisp_id = 0;
#if BOARD_HAS_5G_RADIO
		else if (strcmp(wisp_ifname, IFNAME_5G_APCLI) == 0)
			wisp_id = 1;
#endif
	}

	if (man_id != wisp_id) {
		/* MAN interface changed, need full restart WAN. */
		status = full_restart_wan();
	} else if (radio_on && wisp_id == is_5g) {
		/* MAN interface still uses APCLI; restart WAN after the radio cycle. */
		status = try_wan_reconnect(1, 0);
	}

	if (status != 0)
		logmessage(LOGNAME,
			   "WAN recovery after %s AP-client radio restart failed (status=%d)",
			   is_5g ? "5GHz" : "2.4GHz", status);
	return status;
}

static inline int
wif_control_m2u(int is_aband, const char *wifname)
{
#if !defined(USE_IGMP_SNOOP)
	int i_m2u = nvram_wlan_get_int(is_aband, "IgmpSnEnable");

	return brport_set_m2u(wifname, i_m2u);
#else
	return 0;
#endif
}

#if defined(USE_RT3352_MII)
static void
update_inic_mii(void)
{
#if 0
	int i;
	const char *ifname_inic = IFNAME_INIC_MAIN;

	// below params always set in new iNIC_mii.obj
	doSystem("iwpriv %s set %s=%d", ifname_inic, "asiccheck", 1);

	// config RT3352 embedded switch for VLAN3 passthrough
	doSystem("iwpriv %s switch setVlanId=%d,%d", ifname_inic, 2, INIC_GUEST_VLAN_VID);

	// power down unused PHY of RT3352 embedded switch
	for(i = 0; i < 5; i++)
		doSystem("iwpriv %s switch setPortPowerDown=%d,%d", ifname_inic, i, 1);

	// add static IGMP entries (workaround for IGMP snooping bug in iNIC firmware)
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:7f:ff:fa"); // SSDP IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:fb"); // mDNS IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:09"); // RIP  IPv4
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:0c"); // SSDP IPv6
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:fb"); // mDNS IPv6
#endif
}

static int
start_inic_mii(int inic_enabled)
{
	const char *ifname_inic = IFNAME_INIC_MAIN;
	int operation_status, all_started = 1;

	if (inic_enabled) {
		/* release iNIC reset pin */
		cpu_gpio_set_pin(1, 1);

		/* Wait until the reset line has settled before enabling RGMII. */
		operation_status = service_wait_microseconds(50000U);
		if (operation_status != 0) {
			operation_status = errno > 0 ? errno : EIO;
			errno = operation_status;
			logmessage(LOGNAME, "iNIC reset settling wait failed (status=%d)",
				   operation_status);
			goto fail_start;
		}

		/* Enable the RGMII port before bringing rai0 up. */
		operation_status = phy_disable_inic(0);
		if (operation_status != 0) {
			logmessage(LOGNAME, "failed to enable iNIC RGMII port (status=%d)",
				   operation_status);
			goto fail_start;
		}

		/* start iNIC boot */
		if (!wif_control_success(ifname_inic, 1)) {
			operation_status = -EIO;
			logmessage(LOGNAME, "iNIC startup aborted: failed to bring %s up", ifname_inic);
			goto fail_start;
		}

		/* update some iNIC params */
		update_inic_mii();

		if (get_mlme_radio_rt()) {
			/* clear isolation iNIC port from all LAN ports */
			operation_status = phy_isolate_inic(0);
			if (operation_status != 0) {
				logmessage(LOGNAME, "failed to remove iNIC port isolation (status=%d)",
					   operation_status);
				goto fail_start;
			}
		} else {
			/* disable mlme radio */
			operation_status = doSystem("iwpriv %s set %s=%d",
						    ifname_inic, "RadioOn", 0);
			if (!wireless_radio_status_success(operation_status)) {
				logmessage(LOGNAME, "failed to disable iNIC MLME radio (status=%d)",
					   operation_status);
				operation_status = -EIO;
				goto fail_start;
			}
		}

		/* Add rai0 to bridge when 802.1X requires it. */
		wireless_bridge_membership_record(
			&all_started, ifname_inic,
			is_need_8021x(nvram_wlan_get(0, "auth_mode")));
		if (!all_started) {
			operation_status = -EIO;
			goto fail_start;
		}
		return 0;

fail_start:
		/*
		 * A failed hardware, radio or bridge transition must not leave a
		 * partially initialized iNIC active. Continue every safe cleanup
		 * operation even when interface shutdown or bridge detach fails.
		 */
		if (!wif_control_success(ifname_inic, 0))
			logmessage(LOGNAME, "best-effort iNIC interface shutdown failed");
		wireless_bridge_membership_record(&all_started, ifname_inic, 0);
		if (phy_isolate_inic(1) != 0)
			logmessage(LOGNAME, "failed to isolate iNIC after startup failure");
		if (phy_disable_inic(1) != 0)
			logmessage(LOGNAME, "failed to disable iNIC RGMII after startup failure");
		cpu_gpio_set_pin(1, 0);
		return -EIO;
	}

	/* Force-disable path: always complete every fail-safe action. */
	all_started = wireless_radio_startup_status(
		all_started, wif_control_success(ifname_inic, 0));
	wireless_bridge_membership_record(&all_started, ifname_inic, 0);

	operation_status = phy_isolate_inic(1);
	if (operation_status != 0)
		logmessage(LOGNAME, "failed to isolate disabled iNIC port (status=%d)",
			   operation_status);
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(operation_status));

	operation_status = phy_disable_inic(1);
	if (operation_status != 0)
		logmessage(LOGNAME, "failed to disable iNIC RGMII port (status=%d)",
			   operation_status);
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(operation_status));

	/* Raise iNIC reset pin regardless of preceding cleanup failures. */
	cpu_gpio_set_pin(1, 0);
	logmessage(LOGNAME, "iNIC module disabled! (NVRAM %s=1)", "inic_disable");

	return wireless_radio_startup_result(all_started);
}

int
check_inic_mii_rebooted(void)
{
	int rt_mode_x;
	int guest_is_up;
	int status;

	if (!get_mlme_radio_rt()) {
		status = doSystem("iwpriv %s set %s=%d", IFNAME_INIC_MAIN,
				  "RadioOn", 0);
		if (status != 0) {
			logmessage(LOGNAME,
				   "iNIC radio force-disable command failed (status=%d)",
				   status);
			return wireless_radio_restart_status(0, status);
		}
		return 0;
	}

	rt_mode_x = get_mode_radio_rt();
	if (rt_mode_x != 1 && rt_mode_x != 3) {
		/* Do not confuse an ioctl/socket failure with a down guest interface. */
		status = wireless_guest_interface_state_checked(IFNAME_INIC_GUEST,
							 &guest_is_up);
		if (status != 0) {
			logmessage(LOGNAME,
				   "iNIC guest AP state is unknown; recovery deferred until the next health check (status=%d)",
				   status);
			return status;
		}

		if (!guest_is_up && is_guest_allowed_rt()) {
			status = wif_control(IFNAME_INIC_GUEST, 1);
			if (!wireless_radio_status_success(status)) {
				logmessage(LOGNAME,
					   "iNIC guest AP recovery deferred until the next health check (status=%d)",
					   status);
				return wireless_radio_restart_status(0, status);
			}
		}
	}

	return 0;
}
#endif

void
update_vga_clamp_wl(int first_call)
{
#if BOARD_HAS_5G_RADIO
#if defined (USE_WID_5G) && (USE_WID_5G==7612)
	int i_val;
	const char *wifname;

	wifname = find_wlan_if_up(1);
	if (!wifname)
		return;

	i_val = nvram_wlan_get_int(1, "VgaClamp");
	if (i_val == 0 && first_call)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "VgaClamp", i_val);
#endif
#endif
}

void
update_vga_clamp_rt(int first_call)
{
#if defined (USE_WID_2G) && (USE_WID_2G==7602 || USE_WID_2G==7612)
	int i_val;
	const char *wifname;

	wifname = find_wlan_if_up(0);
	if (!wifname)
		return;

	i_val = nvram_wlan_get_int(0, "VgaClamp");
	if (i_val == 0 && first_call)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "VgaClamp", i_val);
#endif
}

int
stop_wifi_all_wl(void)
{
	int all_stopped = 1;
#if BOARD_HAS_5G_RADIO

	/* Attempt every interface even after one failure; the LED is a state report. */
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_APCLI, IFNAME_5G_APCLI));

	/* stop WDS (4 interfaces) */
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_WDS3, IFNAME_5G_WDS3));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_WDS2, IFNAME_5G_WDS2));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_WDS1, IFNAME_5G_WDS1));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_WDS0, IFNAME_5G_WDS0));

	/* stop AP (guest + main) */
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_GUEST, IFNAME_5G_GUEST));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_5G_MAIN, IFNAME_5G_MAIN));

	if (all_stopped)
		LED_CONTROL(LED_SW5G, LED_OFF);
	else
		logmessage(LOGNAME, "5GHz shutdown incomplete; retaining radio LED state");
#endif
	return wireless_radio_shutdown_result(all_stopped);
}

int
stop_wifi_all_rt(void)
{
	int all_stopped = 1;

#if defined(USE_RT3352_MII)
	if (stop_inicd() != 0) {
		logmessage(LOGNAME, "2.4GHz shutdown aborted: iNIC health monitor did not stop");
		return -1;
	}

	/* Isolate the iNIC path, then stop and detach each started bridge port. */
	if (phy_isolate_inic(1) != 0) {
		logmessage(LOGNAME, "failed to isolate iNIC during 2.4GHz shutdown");
		all_stopped = 0;
	}
	all_stopped = wireless_radio_shutdown_status(all_stopped,
		wireless_stop_interface(IFNAME_INIC_GUEST, IFNAME_INIC_GUEST_VLAN));
	all_stopped = wireless_radio_shutdown_status(all_stopped,
		wireless_stop_interface(IFNAME_INIC_MAIN, IFNAME_INIC_MAIN));
	if (phy_disable_inic(1) != 0) {
		logmessage(LOGNAME, "failed to disable iNIC RGMII during 2.4GHz shutdown");
		all_stopped = 0;
	}
#else
	// stop ApCli
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_APCLI, IFNAME_2G_APCLI));

	// stop WDS (4 interfaces)
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_WDS3, IFNAME_2G_WDS3));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_WDS2, IFNAME_2G_WDS2));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_WDS1, IFNAME_2G_WDS1));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_WDS0, IFNAME_2G_WDS0));

	// stop AP (guest + main)
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_GUEST, IFNAME_2G_GUEST));
	all_stopped = wireless_radio_shutdown_status(all_stopped, wireless_stop_interface(IFNAME_2G_MAIN, IFNAME_2G_MAIN));
#endif

	if (all_stopped)
		LED_CONTROL(LED_SW2G, LED_OFF);
	else
		logmessage(LOGNAME, "2.4GHz shutdown incomplete; retaining radio LED state");

	return wireless_radio_shutdown_result(all_stopped);
}

int 
start_wifi_ap_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	int all_started = 1;
	int i_mode_x = get_mode_radio_wl();

	// check WDS only, ApCli only or Radio disabled
	if (i_mode_x == 1 || i_mode_x == 3 || !radio_on)
	{
		wireless_bridge_membership_record(&all_started, IFNAME_5G_GUEST, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_5G_MAIN, 0);
	}

	mlme_state_wl(radio_on);

	// check Radio enabled and not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3)
	{
		if (!wif_control_success(IFNAME_5G_MAIN, 1))
			return -EIO;
		wireless_bridge_membership_record(&all_started, IFNAME_5G_MAIN, 1);
		all_started = wireless_radio_startup_status(all_started,
			wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_MAIN)));
		
		if (is_guest_allowed_wl())
		{
			if (!wif_control_success(IFNAME_5G_GUEST, 1))
				return -EIO;
			wireless_bridge_membership_record(&all_started, IFNAME_5G_GUEST, 1);
			all_started = wireless_radio_startup_status(all_started,
				wireless_radio_status_success(wif_control_m2u(1, IFNAME_5G_GUEST)));
		}
	}
#endif
#if BOARD_HAS_5G_RADIO
	return wireless_radio_startup_result(all_started);
#else
	return 0;
#endif
}

int 
start_wifi_ap_rt(int radio_on)
{
	int all_started = 1;
	int i_mode_x = get_mode_radio_rt();
#if defined(USE_RT3352_MII)
	int operation_status;
	char *inic_disable_value = NULL;
	int nvram_api_result, nvram_saved_errno, nvram_read_status;
	int inic_disabled = 0, inic_enabled = 0, policy_status;
	int is_ap_mode = get_ap_mode();

	/* Read the iNIC gate once, before changing any radio/bridge state. */
	errno = 0;
	nvram_api_result = nvram_get_checked("inic_disable", &inic_disable_value);
	nvram_saved_errno = errno;
	nvram_read_status = rc_wan_nvram_read_int_status(
		nvram_api_result, nvram_saved_errno, inic_disable_value,
		0, &inic_disabled);
	policy_status = wireless_inic_start_enabled(
		nvram_read_status, inic_disabled, &inic_enabled);
	if (policy_status != 0) {
		errno = policy_status;
		logmessage(LOGNAME,
			   "iNIC startup aborted: cannot read inic_disable (status=%d)",
			   policy_status);
		return -policy_status;
	}
#endif

	// check WDS only, ApCli only or Radio disabled
	if (i_mode_x == 1 || i_mode_x == 3 || !radio_on)
	{
#if defined(USE_RT3352_MII)
		if (!is_ap_mode)
			wireless_bridge_membership_record(&all_started, IFNAME_INIC_GUEST_VLAN, 0);
#else
		wireless_bridge_membership_record(&all_started, IFNAME_2G_GUEST, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_2G_MAIN, 0);
#endif
	}

	mlme_state_rt(radio_on);

#if defined(USE_RT3352_MII)
	// iNIC_mii driver always needed rai0 first before use other interfaces (boot firmware)
	operation_status = start_inic_mii(inic_enabled);
	if (operation_status < 0) {
		logmessage(LOGNAME, "2.4GHz iNIC startup failed (status=%d)",
			   operation_status);
		return operation_status;
	}

	// check Radio enabled and check not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3 &&
	    inic_enabled && is_guest_allowed_rt())
	{
		if (!wif_control_success(IFNAME_INIC_GUEST, 1))
			return -EIO;
		if (!is_ap_mode)
			wireless_bridge_membership_record(&all_started, IFNAME_INIC_GUEST_VLAN, 1);
	}

	// The radio restart is incomplete if its enabled health monitor cannot start.
	if (inic_enabled) {
		int inicd_start_status = start_inicd();

		if (inicd_start_status != 0)
			logmessage(LOGNAME, "failed to start iNIC health monitor (status=%d)",
				   inicd_start_status);
		all_started = wireless_radio_startup_status(
			all_started, wireless_radio_status_success(inicd_start_status));
	}
#else
	// check Radio enabled and check not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3)
	{
		if (!wif_control_success(IFNAME_2G_MAIN, 1))
			return -EIO;
		wireless_bridge_membership_record(&all_started, IFNAME_2G_MAIN, 1);
		all_started = wireless_radio_startup_status(all_started,
			wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_MAIN)));
		
		if (is_guest_allowed_rt())
		{
			if (!wif_control_success(IFNAME_2G_GUEST, 1))
				return -EIO;
			wireless_bridge_membership_record(&all_started, IFNAME_2G_GUEST, 1);
			all_started = wireless_radio_startup_status(all_started,
				wireless_radio_status_success(wif_control_m2u(0, IFNAME_2G_GUEST)));
		}
	}
#endif

	return wireless_radio_startup_result(all_started);
}

int
start_wifi_wds_wl(int radio_on)
{
	int all_started = 1;
#if BOARD_HAS_5G_RADIO
	int interface_started;
	int i_mode_x = get_mode_radio_wl();

	if (radio_on && (i_mode_x == 1 || i_mode_x == 2))
	{
		int i_wds_num = 4;
		
		if (nvram_wlan_get_int(1, "wdsapply_x") == 1)
			i_wds_num = nvram_wlan_get_int(1, "wdsnum_x");
		
		if (i_wds_num > 3) {
			interface_started = wif_control_success(IFNAME_5G_WDS3, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started)
				wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS3, 1);
		}
		
		if (i_wds_num > 2) {
			interface_started = wif_control_success(IFNAME_5G_WDS2, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started)
				wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS2, 1);
		}
		
		if (i_wds_num > 1) {
			interface_started = wif_control_success(IFNAME_5G_WDS1, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started)
				wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS1, 1);
		}
		
		interface_started = wif_control_success(IFNAME_5G_WDS0, 1);
		all_started = wireless_radio_startup_status(all_started, interface_started);
		if (interface_started)
			wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS0, 1);
	}
	else
	{
		wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS3, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS2, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS1, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_5G_WDS0, 0);
	}
#endif

	return wireless_radio_startup_result(all_started);
}

int
start_wifi_wds_rt(int radio_on)
{
	int all_started = 1;
	int interface_started;

	int i_mode_x = get_mode_radio_rt();

	if (radio_on && (i_mode_x == 1 || i_mode_x == 2))
	{
		int i_wds_num = 4;
		
		if (nvram_wlan_get_int(0, "wdsapply_x") == 1)
			i_wds_num = nvram_wlan_get_int(0, "wdsnum_x");
		
		if (i_wds_num > 3) {
			interface_started = wif_control_success(IFNAME_2G_WDS3, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started) {
#if !defined(USE_RT3352_MII)
				wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS3, 1);
#endif
			}
		}
		
		if (i_wds_num > 2) {
			interface_started = wif_control_success(IFNAME_2G_WDS2, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started) {
#if !defined(USE_RT3352_MII)
				wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS2, 1);
#endif
			}
		}
		
		if (i_wds_num > 1) {
			interface_started = wif_control_success(IFNAME_2G_WDS1, 1);
			all_started = wireless_radio_startup_status(all_started, interface_started);
			if (interface_started) {
#if !defined(USE_RT3352_MII)
				wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS1, 1);
#endif
			}
		}
		
		interface_started = wif_control_success(IFNAME_2G_WDS0, 1);
		all_started = wireless_radio_startup_status(all_started, interface_started);
		if (interface_started) {
#if !defined(USE_RT3352_MII)
			wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS0, 1);
#endif
		}
	}
#if !defined(USE_RT3352_MII)
	else
	{
		wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS3, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS2, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS1, 0);
		wireless_bridge_membership_record(&all_started, IFNAME_2G_WDS0, 0);
	}
#endif

	return wireless_radio_startup_result(all_started);
}

int
start_wifi_apcli_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	int all_started = 1;
	const char *ifname_apcli = IFNAME_5G_APCLI;
	int i_mode_x = get_mode_radio_wl();

	if (radio_on && (i_mode_x == 3 || i_mode_x == 4) && (strlen(nvram_wlan_get(1, "sta_ssid")) > 0))
	{
		if (!wif_control_success(ifname_apcli, 1))
			return -EIO;
		wireless_bridge_membership_record(&all_started, ifname_apcli, !is_apcli_wisp_wl() || get_ap_mode());
		if (nvram_wlan_get_int(1, "sta_auto"))
#if defined (USE_WID_5G) && (USE_WID_5G==7615 || USE_WID_5G==7915)
		{
			if (!wireless_radio_status_success(doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 3))) {
			logmessage(LOGNAME, "failed to configure APCLI auto-connect on %s", ifname_apcli);
			return -EIO;
		}
			logmessage(LOGNAME, "Set ApCliAutoConnect to 3");
		}
#else
		{
			if (!wireless_radio_status_success(doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1))) {
			logmessage(LOGNAME, "failed to configure APCLI auto-connect on %s", ifname_apcli);
			return -EIO;
		}
			logmessage(LOGNAME, "Set ApCliAutoConnect to 1");
		}
#endif
	}
	else
	{
		wireless_bridge_membership_record(&all_started, ifname_apcli, 0);
	}
#endif
#if BOARD_HAS_5G_RADIO
	return wireless_radio_startup_result(all_started);
#else
	return 0;
#endif
}

int
start_wifi_apcli_rt(int radio_on)
{
	int all_started = 1;
	const char *ifname_apcli = IFNAME_2G_APCLI;
	int i_mode_x = get_mode_radio_rt();

	if (radio_on && (i_mode_x == 3 || i_mode_x == 4) && (strlen(nvram_wlan_get(0, "sta_ssid")) > 0))
	{
		if (!wif_control_success(ifname_apcli, 1))
			return -EIO;
#if !defined(USE_RT3352_MII)
		wireless_bridge_membership_record(&all_started, ifname_apcli, !is_apcli_wisp_rt() || get_ap_mode());
		if (nvram_wlan_get_int(0, "sta_auto"))
#if defined (USE_WID_2G) && (USE_WID_2G==7615 || USE_WID_2G==7915)
		{
			if (!wireless_radio_status_success(doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 3))) {
			logmessage(LOGNAME, "failed to configure APCLI auto-connect on %s", ifname_apcli);
			return -EIO;
		}
			logmessage(LOGNAME, "Set ApCliAutoConnect to 3");
		}
#else
		{
			if (!wireless_radio_status_success(doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1))) {
			logmessage(LOGNAME, "failed to configure APCLI auto-connect on %s", ifname_apcli);
			return -EIO;
		}
			logmessage(LOGNAME, "Set ApCliAutoConnect to 1");
		}
#endif
#endif
	}
#if !defined(USE_RT3352_MII)
	else
	{
		wireless_bridge_membership_record(&all_started, ifname_apcli, 0);
	}
#endif

	return wireless_radio_startup_result(all_started);
}

int
reconnect_apcli(const char *ifname_apcli, int force)
{
	int is_aband, i_mode_x;
	int status = 0, wait_status;

	if (!ifname_apcli)
		return 0;

	if (strcmp(ifname_apcli, IFNAME_2G_APCLI) == 0)
		is_aband = 0;
#if BOARD_HAS_5G_RADIO
	else if (strcmp(ifname_apcli, IFNAME_5G_APCLI) == 0)
		is_aband = 1;
#endif
	else
		return 0;

	if (!is_interface_up(ifname_apcli))
		return 0;

	i_mode_x = nvram_wlan_get_int(is_aband, "mode_x");
	if (i_mode_x != 3 && i_mode_x != 4)
		return 0;

	if (get_apcli_sta_auto(is_aband)) {
		if (is_aband) {
#if defined (USE_WID_5G) && (USE_WID_5G==7615 || USE_WID_5G==7915)
			status = wireless_radio_restart_status(status,
				doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 3));
			logmessage(LOGNAME, "Set ApCliAutoConnect to 3");
#else
			status = wireless_radio_restart_status(status,
				doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1));
			logmessage(LOGNAME, "Set ApCliAutoConnect to 1");
#endif
		} else {
#if defined (USE_WID_2G) && (USE_WID_2G==7615 || USE_WID_2G==7915)
			status = wireless_radio_restart_status(status,
				doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 3));
			logmessage(LOGNAME, "Set ApCliAutoConnect to 3");
#else
			status = wireless_radio_restart_status(status,
				doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1));
			logmessage(LOGNAME, "Set ApCliAutoConnect to 1");
#endif
		}
	} else if (force) {
		/* Re-enable even if disable or settling wait fails. */
		status = wireless_radio_restart_status(status,
			doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliEnable", 0));
		wait_status = service_wait_microseconds(300000U);
		if (wait_status != 0) {
			wait_status = errno > 0 ? errno : EIO;
			errno = wait_status;
			logmessage(LOGNAME,
				   "AP-client reconnect settling wait failed on %s (status=%d)",
				   ifname_apcli, wait_status);
		}
		status = wireless_radio_restart_status(status, wait_status);
		status = wireless_radio_restart_status(status,
			doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliEnable", 1));
	}

	if (status != 0)
		logmessage(LOGNAME, "AP-client reconnect command failed on %s (status=%d)",
			   ifname_apcli, status);
	return status;
}

int
restart_wifi_wl(int radio_on, int need_reload_conf)
{
	int auth_start_status;
	int isolation_status = 0;
#if BOARD_HAS_5G_RADIO
	if (wireless_restart_preflight(need_reload_conf, gen_ralink_config_5g) < 0) {
		logmessage(LOGNAME, "Wireless restart aborted: failed to generate 5GHz configuration");
		return -1;
	}
	if (stop_wifi_all_wl() < 0) {
		logmessage(LOGNAME, "5GHz wireless restart aborted: previous interfaces did not all stop");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf) {
		if (stop_wifi_all_rt() < 0) {
			logmessage(LOGNAME, "dual-band wireless restart aborted: 2.4GHz interfaces did not all stop");
			return -1;
		}
	}
#endif
	if (stop_8021x_wl() != 0) {
		logmessage(LOGNAME, "5GHz wireless restart aborted: 802.1X daemon did not stop");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf && stop_8021x_rt() != 0) {
		logmessage(LOGNAME, "dual-band wireless restart aborted: 2.4GHz 802.1X daemon did not stop");
		return -1;
	}
#endif

	if (need_reload_conf)
		nvram_set_int_temp("reload_svc_wl", 1);

	if (start_wifi_ap_wl(radio_on) < 0 ||
	    start_wifi_wds_wl(radio_on) < 0 ||
	    start_wifi_apcli_wl(radio_on) < 0) {
		logmessage(LOGNAME, "5GHz wireless startup incomplete; rolling back interfaces");
		if (stop_wifi_all_wl() < 0)
			logmessage(LOGNAME, "5GHz wireless startup rollback was incomplete");
		return -1;
	}

	auth_start_status = start_8021x_wl();
	if (auth_start_status != 0) {
		logmessage(LOGNAME, "5GHz wireless restart aborted: 802.1X authenticator launch failed (status=%d)", auth_start_status);
		if (stop_8021x_wl() != 0)
			logmessage(LOGNAME, "5GHz 802.1X startup rollback could not stop authenticator");
		if (stop_wifi_all_wl() < 0)
			logmessage(LOGNAME, "5GHz wireless restart rollback was incomplete");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf) {
		int rt_radio_on = get_enabled_radio_rt();
		if (rt_radio_on)
			rt_radio_on = is_radio_allowed_rt();
		if (start_wifi_ap_rt(rt_radio_on) < 0 ||
		    start_wifi_wds_rt(rt_radio_on) < 0 ||
		    start_wifi_apcli_rt(rt_radio_on) < 0) {
			logmessage(LOGNAME, "DBDC 2.4GHz startup incomplete; rolling back both radios");
			if (stop_8021x_wl() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 5GHz 802.1X daemon");
			if (stop_wifi_all_rt() < 0)
				logmessage(LOGNAME, "DBDC 2.4GHz startup rollback was incomplete");
			if (stop_wifi_all_wl() < 0)
				logmessage(LOGNAME, "DBDC 5GHz startup rollback was incomplete");
			return -1;
		}

		auth_start_status = start_8021x_rt();
		if (auth_start_status != 0) {
			logmessage(LOGNAME, "DBDC startup aborted: 2.4GHz 802.1X authenticator launch failed (status=%d)", auth_start_status);
			if (stop_8021x_rt() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 2.4GHz 802.1X daemon");
			if (stop_8021x_wl() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 5GHz 802.1X daemon");
			if (stop_wifi_all_rt() < 0)
				logmessage(LOGNAME, "DBDC 2.4GHz startup rollback was incomplete");
			if (stop_wifi_all_wl() < 0)
				logmessage(LOGNAME, "DBDC 5GHz startup rollback was incomplete");
			return -1;
		}
	}
#endif
	isolation_status = restart_guest_lan_isolation();
	if (isolation_status != 0) {
		logmessage(LOGNAME, "5GHz wireless restart: guest LAN isolation failed (status=%d); rolling back restarted guest APs",
			   isolation_status);
#if defined(BOARD_MT7615_DBDC) || defined(BOARD_MT7915_DBDC)
		isolation_status = wireless_guest_restart_fail_closed(1, need_reload_conf, isolation_status);
#else
		isolation_status = wireless_guest_restart_fail_closed(1, 0, isolation_status);
#endif
	}

	{
		int wan_status = check_apcli_wan(1, radio_on);

		isolation_status = wireless_radio_restart_status(isolation_status, wan_status);
	}

	if (radio_on) {
		update_vga_clamp_wl(0);
		LED_CONTROL(LED_SW5G, LED_ON);
	}
	{
		int iappd_restart_status = system("/usr/bin/iappd.sh restart");

		if (iappd_restart_status != 0)
			logmessage(LOGNAME,
				   "Ralink IAPP restart after 5GHz wireless restart failed (wait status=%d)",
				   iappd_restart_status);
		isolation_status = wireless_radio_restart_status(
			isolation_status, iappd_restart_status);
	}
#endif
	return isolation_status;
}

int
restart_wifi_rt(int radio_on, int need_reload_conf)
{
	int auth_start_status;
	int isolation_status;

	if (wireless_restart_preflight(need_reload_conf, gen_ralink_config_2g) < 0) {
		logmessage(LOGNAME, "Wireless restart aborted: failed to generate 2.4GHz configuration");
		return -1;
	}
	if (stop_wifi_all_rt() < 0) {
		logmessage(LOGNAME, "2.4GHz wireless restart aborted: previous interfaces did not all stop");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf) {
		if (stop_wifi_all_wl() < 0) {
			logmessage(LOGNAME, "dual-band wireless restart aborted: 5GHz interfaces did not all stop");
			return -1;
		}
	}
#endif
	if (stop_8021x_rt() != 0) {
		logmessage(LOGNAME, "2.4GHz wireless restart aborted: 802.1X daemon did not stop");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf && stop_8021x_wl() != 0) {
		logmessage(LOGNAME, "dual-band wireless restart aborted: 5GHz 802.1X daemon did not stop");
		return -1;
	}
#endif

	if (need_reload_conf)
		nvram_set_int_temp("reload_svc_rt", 1);

	if (start_wifi_ap_rt(radio_on) < 0 ||
	    start_wifi_wds_rt(radio_on) < 0 ||
	    start_wifi_apcli_rt(radio_on) < 0) {
		logmessage(LOGNAME, "2.4GHz wireless startup incomplete; rolling back interfaces");
		if (stop_wifi_all_rt() < 0)
			logmessage(LOGNAME, "2.4GHz wireless startup rollback was incomplete");
		return -1;
	}

	auth_start_status = start_8021x_rt();
	if (auth_start_status != 0) {
		logmessage(LOGNAME, "2.4GHz wireless restart aborted: 802.1X authenticator launch failed (status=%d)", auth_start_status);
		if (stop_8021x_rt() != 0)
			logmessage(LOGNAME, "2.4GHz 802.1X startup rollback could not stop authenticator");
		if (stop_wifi_all_rt() < 0)
			logmessage(LOGNAME, "2.4GHz wireless restart rollback was incomplete");
		return -1;
	}
#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	if (need_reload_conf) {
		int wl_radio_on = get_enabled_radio_wl();
		if (wl_radio_on)
			wl_radio_on = is_radio_allowed_wl();
		if (start_wifi_ap_wl(wl_radio_on) < 0 ||
		    start_wifi_wds_wl(wl_radio_on) < 0 ||
		    start_wifi_apcli_wl(wl_radio_on) < 0) {
			logmessage(LOGNAME, "DBDC 5GHz startup incomplete; rolling back both radios");
			if (stop_8021x_rt() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 2.4GHz 802.1X daemon");
			if (stop_wifi_all_wl() < 0)
				logmessage(LOGNAME, "DBDC 5GHz startup rollback was incomplete");
			if (stop_wifi_all_rt() < 0)
				logmessage(LOGNAME, "DBDC 2.4GHz startup rollback was incomplete");
			return -1;
		}

		auth_start_status = start_8021x_wl();
		if (auth_start_status != 0) {
			logmessage(LOGNAME, "DBDC startup aborted: 5GHz 802.1X authenticator launch failed (status=%d)", auth_start_status);
			if (stop_8021x_wl() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 5GHz 802.1X daemon");
			if (stop_8021x_rt() != 0)
				logmessage(LOGNAME, "DBDC rollback could not stop the 2.4GHz 802.1X daemon");
			if (stop_wifi_all_wl() < 0)
				logmessage(LOGNAME, "DBDC 5GHz startup rollback was incomplete");
			if (stop_wifi_all_rt() < 0)
				logmessage(LOGNAME, "DBDC 2.4GHz startup rollback was incomplete");
			return -1;
		}
	}
#endif
	isolation_status = restart_guest_lan_isolation();
	if (isolation_status != 0) {
		logmessage(LOGNAME, "2.4GHz wireless restart: guest LAN isolation failed (status=%d); rolling back restarted guest APs",
			   isolation_status);
#if defined(BOARD_MT7615_DBDC) || defined(BOARD_MT7915_DBDC)
		isolation_status = wireless_guest_restart_fail_closed(0, need_reload_conf, isolation_status);
#else
		isolation_status = wireless_guest_restart_fail_closed(0, 0, isolation_status);
#endif
	}

	{
		int wan_status = check_apcli_wan(0, radio_on);

		isolation_status = wireless_radio_restart_status(isolation_status, wan_status);
	}

	if (radio_on) {
		update_vga_clamp_rt(0);
		LED_CONTROL(LED_SW2G, LED_ON);
	}
	{
		int iappd_restart_status = system("/usr/bin/iappd.sh restart");

		if (iappd_restart_status != 0)
			logmessage(LOGNAME,
				   "Ralink IAPP restart after 2.4GHz wireless restart failed (wait status=%d)",
				   iappd_restart_status);
		isolation_status = wireless_radio_restart_status(
			isolation_status, iappd_restart_status);
	}
	return isolation_status;
}

int is_need_8021x(char *auth_mode)
{
	if (!strcmp(auth_mode, "wpa") ||
	    !strcmp(auth_mode, "wpa2") ||
	    !strcmp(auth_mode, "radius"))
		return 1;

	return 0;
}

int
start_8021x_wl(void)
{
	int start_status = 0;
#if BOARD_HAS_5G_RADIO
	if (!get_enabled_radio_wl())
		return 0;

	if (is_need_8021x(nvram_wlan_get(1, "auth_mode"))) {
		start_status = eval("rt2860apd", "-i", IFNAME_5G_MAIN);
		if (start_status != 0)
			logmessage(LOGNAME, "802.1X 5GHz authenticator launch failed (status=%d)", start_status);
	}
#endif

	const char *wifname = find_wlan_if_up(1);

	if (!wifname)
		return start_status;

	int wl_KickStaRssiLow = nvram_get_int("wl_KickStaRssiLow");
	int wl_AssocReqRssiThres = nvram_get_int("wl_AssocReqRssiThres");
	doSystem("iwpriv %s set %s=%d", wifname, "KickStaRssiLow", wl_KickStaRssiLow);
	doSystem("iwpriv %s set %s=%d", wifname, "AssocReqRssiThres", wl_AssocReqRssiThres);
	return start_status;
}


int
start_8021x_rt(void)
{
	int start_status = 0;

#if !defined(USE_RT3352_MII)
	if (!get_enabled_radio_rt())
		return 0;
#endif
	if (is_need_8021x(nvram_wlan_get(0, "auth_mode"))) {
		start_status = eval("rtinicapd", "-i", IFNAME_2G_MAIN);
		if (start_status != 0)
			logmessage(LOGNAME, "802.1X 2.4GHz authenticator launch failed (status=%d)", start_status);
	}

	const char *wifname = find_wlan_if_up(0);

	if (!wifname)
		return start_status;

	int rt_KickStaRssiLow = nvram_get_int("rt_KickStaRssiLow");
	int rt_AssocReqRssiThres = nvram_get_int("rt_AssocReqRssiThres");
	doSystem("iwpriv %s set %s=%d", wifname, "KickStaRssiLow", rt_KickStaRssiLow);
	doSystem("iwpriv %s set %s=%d", wifname, "AssocReqRssiThres", rt_AssocReqRssiThres);
	return start_status;
}


int
stop_8021x_wl(void)
{
#if BOARD_HAS_5G_RADIO
	char* svcs[] = { "rt2860apd", NULL };
	return kill_services(svcs, 3, 1);
#else
	return 0;
#endif
}

int
stop_8021x_rt(void)
{
	char* svcs[] = { "rtinicapd", NULL };
	return kill_services(svcs, 3, 1);
}

int
stop_8021x_all(void)
{
	char* svcs[] = { "rt2860apd", "rtinicapd", NULL };
	return kill_services(svcs, 3, 1);
}

int 
is_radio_on_wl(void)
{
#if BOARD_HAS_5G_RADIO
#if defined(USE_IWPRIV_RADIO_5G)
	return (is_interface_up(IFNAME_5G_MAIN) && get_mlme_radio_wl());
#else
	return is_interface_up(IFNAME_5G_MAIN) ||
	       is_interface_up(IFNAME_5G_GUEST) ||
	       is_interface_up(IFNAME_5G_APCLI) ||
	       is_interface_up(IFNAME_5G_WDS0) ||
	       is_interface_up(IFNAME_5G_WDS1) ||
	       is_interface_up(IFNAME_5G_WDS2) ||
	       is_interface_up(IFNAME_5G_WDS3);
#endif
#else
	return 0;
#endif
}

int 
is_radio_on_rt(void)
{
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
	return (is_interface_up(IFNAME_2G_MAIN) && get_mlme_radio_rt());
#else
	return is_interface_up(IFNAME_2G_MAIN) ||
	       is_interface_up(IFNAME_2G_GUEST) ||
	       is_interface_up(IFNAME_2G_APCLI) ||
	       is_interface_up(IFNAME_2G_WDS0) ||
	       is_interface_up(IFNAME_2G_WDS1) ||
	       is_interface_up(IFNAME_2G_WDS2) ||
	       is_interface_up(IFNAME_2G_WDS3);
#endif
}

int 
is_radio_allowed_wl(void)
{
	return timecheck_wifi(1, "radio_date_x", "radio_time_x", "radio_time2_x");
}

int 
is_radio_allowed_rt(void)
{
	return timecheck_wifi(0, "radio_date_x", "radio_time_x", "radio_time2_x");
}

int 
is_guest_allowed_wl(void)
{
	if (get_enabled_guest_wl())
		return timecheck_wifi(1, "guest_date_x", "guest_time_x", "guest_time2_x");
	return 0;
}

int 
is_guest_allowed_rt(void)
{
	if (get_enabled_guest_rt())
		return timecheck_wifi(0, "guest_date_x", "guest_time_x", "guest_time2_x");
	return 0;
}

int 
control_radio_wl(int radio_on, int manual)
{
	int is_radio_changed = 0;

#if BOARD_HAS_5G_RADIO
	if (radio_on)
	{
		if (!is_radio_on_wl()) {
#if defined(USE_IWPRIV_RADIO_5G)
			is_radio_changed = mlme_radio_wl_apply(1);
#else
			is_radio_changed = wireless_radio_status_success(restart_wifi_wl(1, 0));
#endif
		}
	}
	else
	{
		if (is_radio_on_wl()) {
#if defined(USE_IWPRIV_RADIO_5G)
			is_radio_changed = mlme_radio_wl_apply(0);
#else
			is_radio_changed = wireless_radio_status_success(restart_wifi_wl(0, 0));
#endif
		}
	}

	if (is_radio_changed && !manual)
		logmessage("WiFi scheduler", "5GHz radio: %s", (radio_on) ? "ON" : "OFF");
#endif

	return is_radio_changed;
}

int 
control_radio_rt(int radio_on, int manual)
{
	int is_radio_changed = 0;

	if (radio_on)
	{
		if (!is_radio_on_rt()) {
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
			is_radio_changed = mlme_radio_rt_apply(1);
#else
			is_radio_changed = wireless_radio_status_success(restart_wifi_rt(1, 0));
#endif
		}
	}
	else
	{
		if (is_radio_on_rt()) {
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
			is_radio_changed = mlme_radio_rt_apply(0);
#else
			is_radio_changed = wireless_radio_status_success(restart_wifi_rt(0, 0));
#endif
		}
	}

	if (is_radio_changed && !manual)
		logmessage("WiFi scheduler", "2.4GHz radio: %s", (radio_on) ? "ON" : "OFF");

	return is_radio_changed;
}

static void
wireless_guest_isolation_record(wireless_guest_isolation_status_t *status,
				const char *operation, int operation_status);

int
control_guest_wl(int guest_on, int manual)
{
	int is_ap_changed = 0;
	wireless_guest_isolation_status_t control_status;
#if BOARD_HAS_5G_RADIO
	const char *ifname_ap = IFNAME_5G_GUEST;
	const char *bridge_ifname = IFNAME_5G_GUEST;
	int radio_on = get_enabled_radio_wl();
	int i_mode_x = get_mode_radio_wl();
	int operation_status;
	int isolation_status;
	int state_status;
	int main_is_up = 0;
	int guest_is_up = 0;
	int rollback_candidate = 0;
#endif

	wireless_guest_isolation_status_reset(&control_status);
#if BOARD_HAS_5G_RADIO
	if (guest_on && (i_mode_x == 1 || i_mode_x == 3 || !radio_on))
		return 0;

	if (guest_on) {
		state_status = wireless_guest_interface_state_checked(IFNAME_5G_MAIN,
							      &main_is_up);
		wireless_guest_isolation_status_record(&control_status,
			"inspect 5GHz main AP state before guest activation", state_status);
		if (state_status != 0)
			goto finish_wl;
		if (!main_is_up)
			return 0;
	}

	state_status = wireless_guest_interface_state_checked(ifname_ap, &guest_is_up);
	wireless_guest_isolation_status_record(&control_status,
		"inspect 5GHz guest AP state", state_status);
	if (state_status != 0 && guest_on)
		goto finish_wl;

	if (guest_on) {
		if (!guest_is_up) {
			/* Roll back only an interface that was down before this request. */
			rollback_candidate = 1;
			if (!wif_control_success(ifname_ap, 1)) {
				wireless_guest_isolation_status_record(&control_status,
					"bring up 5GHz guest AP", -EIO);
				goto finish_wl;
			}
			is_ap_changed = 1;
		}
		operation_status = br_add_del_if(IFNAME_BR, ifname_ap, 1);
		wireless_guest_isolation_record(&control_status,
			"attach 5GHz guest AP to LAN bridge", operation_status);
		operation_status = wif_control_m2u(1, ifname_ap);
		wireless_guest_isolation_record(&control_status,
			"configure 5GHz guest AP multicast-to-unicast", operation_status);
	} else {
		/* Unknown state still triggers best-effort down and bridge detach. */
		if (state_status != 0 || guest_is_up) {
			if (!wif_control_success(ifname_ap, 0)) {
				wireless_guest_isolation_status_record(&control_status,
					"bring down 5GHz guest AP", -EIO);
			} else if (guest_is_up) {
				is_ap_changed = 1;
			}
		}
		operation_status = br_add_del_if(IFNAME_BR, ifname_ap, 0);
		wireless_guest_isolation_record(&control_status,
			"detach 5GHz guest AP from LAN bridge", operation_status);
	}
finish_wl:
	isolation_status = restart_guest_lan_isolation();
	wireless_guest_isolation_record(&control_status,
		"reapply guest LAN isolation after 5GHz guest update", isolation_status);
	if (guest_on && rollback_candidate && control_status.first_status != 0) {
		wireless_guest_ap_rollback(ifname_ap, bridge_ifname, &control_status);
		isolation_status = restart_guest_lan_isolation();
		wireless_guest_isolation_record(&control_status,
			"reconcile guest LAN isolation after 5GHz AP rollback", isolation_status);
	}

	if (is_ap_changed && !manual &&
	    wireless_guest_isolation_status_result(&control_status) == 0)
		logmessage("WiFi scheduler", "5GHz guest AP: %s", guest_on ? "ON" : "OFF");
#endif

	if (control_status.first_status != 0)
		logmessage(LOGNAME,
			"5GHz guest AP control incomplete: %s failed first (status=%d; failed_operations=%u)",
			control_status.first_operation ? control_status.first_operation : "unknown",
			control_status.first_status, control_status.failure_count);
	return wireless_guest_isolation_control_result(is_ap_changed, &control_status);
}

int
control_guest_rt(int guest_on, int manual)
{
	int is_ap_changed = 0;
	wireless_guest_isolation_status_t control_status;
	int operation_status;
	int isolation_status;
	int state_status;
	int main_is_up = 0;
	int guest_is_up = 0;
	int rollback_candidate = 0;
	const char *ifname_ap = IFNAME_2G_GUEST;
	int radio_on = get_enabled_radio_rt();
	int i_mode_x = get_mode_radio_rt();
#if defined(USE_RT3352_MII)
	int is_ap_mode = get_ap_mode();
	const char *bridge_ifname = wireless_guest_rt_bridge_ifname(is_ap_mode);
#else
	const char *bridge_ifname = IFNAME_2G_GUEST;
#endif

	wireless_guest_isolation_status_reset(&control_status);
	if (guest_on && (i_mode_x == 1 || i_mode_x == 3 || !radio_on))
		return 0;
	if (guest_on) {
		state_status = wireless_guest_interface_state_checked(IFNAME_2G_MAIN,
							      &main_is_up);
		wireless_guest_isolation_status_record(&control_status,
			"inspect 2.4GHz main AP state before guest activation", state_status);
		if (state_status != 0)
			goto finish_rt;
		if (!main_is_up)
			return 0;
	}

	state_status = wireless_guest_interface_state_checked(ifname_ap, &guest_is_up);
	wireless_guest_isolation_status_record(&control_status,
		"inspect 2.4GHz guest AP state", state_status);
	if (state_status != 0 && guest_on)
		goto finish_rt;

	if (guest_on) {
		if (!guest_is_up) {
			/* Preserve a guest AP that was already up before this call. */
			rollback_candidate = 1;
			if (!wif_control_success(ifname_ap, 1)) {
				wireless_guest_isolation_status_record(&control_status,
					"bring up 2.4GHz guest AP", -EIO);
				goto finish_rt;
			}
			is_ap_changed = 1;
		}
#if defined(USE_RT3352_MII)
		if (!is_ap_mode) {
			operation_status = br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 1);
			wireless_guest_isolation_record(&control_status,
				"attach iNIC guest VLAN to LAN bridge", operation_status);
		}
#else
		operation_status = br_add_del_if(IFNAME_BR, ifname_ap, 1);
		wireless_guest_isolation_record(&control_status,
			"attach 2.4GHz guest AP to LAN bridge", operation_status);
		operation_status = wif_control_m2u(0, ifname_ap);
		wireless_guest_isolation_record(&control_status,
			"configure 2.4GHz guest AP multicast-to-unicast", operation_status);
#endif
	} else {
		/* Unknown state still triggers best-effort down and bridge detach. */
		if (state_status != 0 || guest_is_up) {
			if (!wif_control_success(ifname_ap, 0)) {
				wireless_guest_isolation_status_record(&control_status,
					"bring down 2.4GHz guest AP", -EIO);
			} else if (guest_is_up) {
				is_ap_changed = 1;
			}
		}
#if defined(USE_RT3352_MII)
		if (!is_ap_mode) {
			operation_status = br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 0);
			wireless_guest_isolation_record(&control_status,
				"detach iNIC guest VLAN from LAN bridge", operation_status);
		}
#else
		operation_status = br_add_del_if(IFNAME_BR, ifname_ap, 0);
		wireless_guest_isolation_record(&control_status,
			"detach 2.4GHz guest AP from LAN bridge", operation_status);
#endif
	}

finish_rt:
	isolation_status = restart_guest_lan_isolation();
	wireless_guest_isolation_record(&control_status,
		"reapply guest LAN isolation after 2.4GHz guest update", isolation_status);
	if (guest_on && rollback_candidate && control_status.first_status != 0) {
		wireless_guest_ap_rollback(ifname_ap, bridge_ifname, &control_status);
		isolation_status = restart_guest_lan_isolation();
		wireless_guest_isolation_record(&control_status,
			"reconcile guest LAN isolation after 2.4GHz AP rollback", isolation_status);
	}

	if (is_ap_changed && !manual &&
	    wireless_guest_isolation_status_result(&control_status) == 0)
		logmessage("WiFi scheduler", "2.4GHz guest AP: %s", guest_on ? "ON" : "OFF");

	if (control_status.first_status != 0)
		logmessage(LOGNAME,
			"2.4GHz guest AP control incomplete: %s failed first (status=%d; failed_operations=%u)",
			control_status.first_operation ? control_status.first_operation : "unknown",
			control_status.first_status, control_status.failure_count);
	return wireless_guest_isolation_control_result(is_ap_changed, &control_status);
}
static void
wireless_guest_isolation_record(wireless_guest_isolation_status_t *status,
				const char *operation, int operation_status)
{
	if (operation_status != 0)
		logmessage(LOGNAME, "guest LAN isolation operation '%s' failed (status=%d)",
			   operation, operation_status);
	wireless_guest_isolation_status_record(status, operation, operation_status);
}

static void
ebtables_filter_guest_ap(const char *wifname, int is_aband, int i_need_dhcp,
			 wireless_guest_isolation_status_t *status)
{
#if !BOARD_HAS_5G_RADIO
	(void)is_aband;
#endif
	if (i_need_dhcp) {
		wireless_guest_isolation_record(status, "add guest INPUT non-DHCP IPv4 drop",
			doSystem("ebtables -A %s -i %s -p IPv4 --ip-protocol ! %s -j %s",
				 "INPUT", wifname, "udp", "DROP"));
		wireless_guest_isolation_record(status, "add guest INPUT non-DHCP UDP drop",
			doSystem("ebtables -A %s -i %s -p IPv4 --ip-protocol %s --ip-destination-port ! %d -j %s",
				 "INPUT", wifname, "udp", 67, "DROP"));
	} else {
		wireless_guest_isolation_record(status, "add guest INPUT drop",
			doSystem("ebtables -A %s -i %s -j %s",
				 "INPUT", wifname, "DROP"));
	}
#if BOARD_HAS_5G_RADIO
	if (is_aband) {
#if defined(USE_RT3352_MII)
		wireless_guest_isolation_record(status, "add 5GHz-to-iNIC guest FORWARD drop",
			doSystem("ebtables -A %s -i %s -o %s -j %s",
				 "FORWARD", wifname, IFNAME_INIC_GUEST_VLAN, "DROP"));
#else
		wireless_guest_isolation_record(status, "add 5GHz-to-2.4GHz primary FORWARD drop",
			doSystem("ebtables -A %s -i %s -o %s -j %s",
				 "FORWARD", wifname, IFNAME_2G_MAIN, "DROP"));
		wireless_guest_isolation_record(status, "add 5GHz-to-2.4GHz guest FORWARD drop",
			doSystem("ebtables -A %s -i %s -o %s -j %s",
				 "FORWARD", wifname, IFNAME_2G_GUEST, "DROP"));
#endif
	} else {
		wireless_guest_isolation_record(status, "add 2.4GHz-to-5GHz primary FORWARD drop",
			doSystem("ebtables -A %s -i %s -o %s -j %s",
				 "FORWARD", wifname, IFNAME_5G_MAIN, "DROP"));
		wireless_guest_isolation_record(status, "add 2.4GHz-to-5GHz guest FORWARD drop",
			doSystem("ebtables -A %s -i %s -o %s -j %s",
				 "FORWARD", wifname, IFNAME_5G_GUEST, "DROP"));
	}
#endif
}

int
restart_guest_lan_isolation(void)
{
	int bp_isolate, is_need_ebtables = 0;
	int is_ap_mode = get_ap_mode();
	int cleanup_status = 0;
	int operation_status;
	int state_status;
	int ebtables_module_status, ebtables_module_error, ebtables_disable_action;
	unsigned int ebtables_failures_before = 0;
	int interface_is_up = 0;
	const char *rt_ifname_guest = IFNAME_2G_GUEST;
	wireless_guest_isolation_status_t status;
#if BOARD_HAS_5G_RADIO
	const char *wl_ifname_guest = IFNAME_5G_GUEST;
#endif

	wireless_guest_isolation_status_reset(&status);
#if BOARD_HAS_5G_RADIO
	bp_isolate = 0;
	state_status = wireless_guest_interface_state_checked(wl_ifname_guest,
							      &interface_is_up);
	wireless_guest_isolation_record(&status,
		"inspect 5GHz guest AP state for LAN isolation", state_status);
	if (nvram_wlan_get_int(1, "guest_lan_isolate")) {
		if (is_ap_mode)
			is_need_ebtables |= 0x10;
		else if (state_status == 0 && interface_is_up)
			bp_isolate = 1;
	}
	if (state_status == 0 && interface_is_up) {
		operation_status = brport_set_param_int(wl_ifname_guest, "isolate_mode", bp_isolate);
		wireless_guest_isolation_record(&status, "set 5GHz guest bridge isolate_mode",
					operation_status);
	}
#endif

	bp_isolate = 0;
	state_status = wireless_guest_interface_state_checked(rt_ifname_guest,
							      &interface_is_up);
	wireless_guest_isolation_record(&status,
		"inspect 2.4GHz guest AP state for LAN isolation", state_status);
	if (nvram_wlan_get_int(0, "guest_lan_isolate")) {
		if (is_ap_mode)
			is_need_ebtables |= 0x01;
		else if (state_status == 0 && interface_is_up)
			bp_isolate = 1;
	}
#if defined(USE_RT3352_MII)
	rt_ifname_guest = IFNAME_INIC_GUEST_VLAN;
	if (is_ap_mode)
		is_need_ebtables &= ~0x01;
#endif
	state_status = wireless_guest_interface_state_checked(rt_ifname_guest,
							      &interface_is_up);
	wireless_guest_isolation_record(&status,
		"inspect 2.4GHz/iNIC guest bridge port state", state_status);
	if (state_status == 0 && interface_is_up) {
		operation_status = brport_set_param_int(rt_ifname_guest, "isolate_mode", bp_isolate);
		wireless_guest_isolation_record(&status, "set 2.4GHz/iNIC guest bridge isolate_mode",
					operation_status);
	}

	if (is_ap_mode) {
		ebtables_failures_before = status.failure_count;
		if (is_need_ebtables) {
			int i_need_dhcp = is_dhcpd_enabled(1);

			/*
			 * module_smart_load() returns zero both when a module is already
			 * present and when loading fails. Verify the postcondition before
			 * flushing or publishing guest-isolation rules.
			 */
			(void)module_smart_load("ebtable_filter", NULL);
			ebtables_module_status = is_module_loaded_status("ebtable_filter");
			ebtables_module_error =
				wireless_guest_isolation_module_presence_error(ebtables_module_status);
			wireless_guest_isolation_record(&status, "load ebtables filter module",
							ebtables_module_error);
			if (ebtables_module_error != 0) {
				logmessage(LOGNAME,
					   "guest isolation rule setup aborted: ebtables filter module is not confirmed loaded (status=%d)",
					   ebtables_module_error);
			} else {
				operation_status = doSystem("ebtables %s", "-F");
				wireless_guest_isolation_record(&status, "flush ebtables rules before guest isolation", operation_status);
				operation_status = doSystem("ebtables %s", "-X");
				wireless_guest_isolation_record(&status, "delete ebtables user chains before guest isolation", operation_status);
#if BOARD_HAS_5G_RADIO
				if (is_need_ebtables & 0x10)
					ebtables_filter_guest_ap(wl_ifname_guest, 1, i_need_dhcp, &status);
#endif
				if (is_need_ebtables & 0x01)
					ebtables_filter_guest_ap(rt_ifname_guest, 0, i_need_dhcp, &status);
			}
		} else {
			ebtables_module_status = is_module_loaded_status("ebtables");
			ebtables_disable_action =
				wireless_guest_isolation_ebtables_disable_action(ebtables_module_status);
			if (ebtables_disable_action != WIRELESS_GUEST_EBTABLES_DISABLE_NONE) {
				if (ebtables_disable_action == WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY) {
					ebtables_module_error =
						wireless_guest_isolation_module_presence_error(ebtables_module_status);
					wireless_guest_isolation_record(&status,
						"inspect ebtables module state before cleanup", ebtables_module_error);
					logmessage(LOGNAME,
						   "ebtables module state is unknown (status=%d); attempting rule cleanup without unloading modules",
						   ebtables_module_error);
				}
				operation_status = doSystem("ebtables %s", "-F");
				wireless_guest_isolation_record(&status, "flush ebtables rules while disabling guest isolation", operation_status);
				cleanup_status = operation_status;
				operation_status = doSystem("ebtables %s", "-X");
				wireless_guest_isolation_record(&status, "delete ebtables user chains while disabling guest isolation", operation_status);
				cleanup_status = cleanup_status != 0 ? cleanup_status : operation_status;
				if (cleanup_status == 0) {
					if (ebtables_disable_action == WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_AND_UNLOAD) {
						operation_status = module_smart_unload("ebt_ip", 0);
						wireless_guest_isolation_record(&status, "unload ebt_ip after clearing guest isolation", operation_status);
						operation_status = module_smart_unload("ebtable_filter", 0);
						wireless_guest_isolation_record(&status, "unload ebtables filter after clearing guest isolation", operation_status);
						operation_status = module_smart_unload("ebtables", 0);
						wireless_guest_isolation_record(&status, "unload ebtables after clearing guest isolation", operation_status);
					} else {
						logmessage(LOGNAME,
							   "retaining ebtables modules because their loaded state is unknown");
					}
				} else {
					logmessage(LOGNAME,
						   "retaining ebtables modules because rule cleanup was incomplete (status=%d)",
						   cleanup_status);
				}
			}
		}
		if (wireless_guest_isolation_ebtables_rollback_required(
			    (unsigned int)is_need_ebtables, ebtables_failures_before,
			    status.failure_count)) {
			logmessage(LOGNAME,
				   "guest ebtables isolation setup failed; taking configured guest APs offline");
#if BOARD_HAS_5G_RADIO
			if (is_need_ebtables & 0x10)
				wireless_guest_ap_rollback(IFNAME_5G_GUEST, IFNAME_5G_GUEST, &status);
#endif
			if (is_need_ebtables & 0x01)
				wireless_guest_ap_rollback(wireless_guest_rt_interface_ifname(),
							   wireless_guest_rt_bridge_ifname(is_ap_mode),
							   &status);
		}
	}

	if (status.first_status != 0)
		logmessage(LOGNAME,
			"guest LAN isolation incomplete: %s failed first (status=%d; failed_operations=%u)",
			status.first_operation ? status.first_operation : "unknown",
			status.first_status, status.failure_count);
	return wireless_guest_isolation_status_result(&status);
}

int 
manual_toggle_radio_rt(int radio_on)
{
	if (!get_enabled_radio_rt())
		return 0;

	if (radio_on < 0) {
		radio_on = !is_radio_on_rt();
	} else {
		if (is_radio_on_rt() == radio_on)
			return 0;
	}

	notify_watchdog_wifi(0);

	logmessage(LOGNAME, "Perform manual toggle %s radio: %s", "2.4GHz", (radio_on) ? "ON" : "OFF");

	return control_radio_rt(radio_on, 1);
}

int 
manual_toggle_radio_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	if (!get_enabled_radio_wl())
		return 0;

	if (radio_on < 0) {
		radio_on = !is_radio_on_wl();
	} else {
		if (is_radio_on_wl() == radio_on)
			return 0;
	}

	notify_watchdog_wifi(1);

	logmessage(LOGNAME, "Perform manual toggle %s radio: %s", "5GHz", (radio_on) ? "ON" : "OFF");

	return control_radio_wl(radio_on, 1);
#else
	return 0;
#endif
}

static int
wait_wifi_watchdog_update(const char *radio_name)
{
	int status;

	status = service_wait_microseconds(300000U);
	if (status == 0)
		return 0;

	status = errno > 0 ? errno : EIO;
	errno = status;
	logmessage(LOGNAME,
		   "watchdog Wi-Fi notification settling wait failed for %s (status=%d)",
		   radio_name, status);
	return -1;
}

int 
manual_change_radio_rt(int radio_on)
{
	int wait_status;
	if (get_enabled_radio_rt() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(0);
		wait_status = wait_wifi_watchdog_update("2.4GHz radio");
		if (wait_status != 0)
			return wait_status;
	}

	nvram_wlan_set_int(0, "radio_x", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "2.4GHz", "radio");

	return control_radio_rt(radio_on, 1);
}

int 
manual_change_radio_wl(int radio_on)
{
	int wait_status;
#if BOARD_HAS_5G_RADIO
	if (get_enabled_radio_wl() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(1);
		wait_status = wait_wifi_watchdog_update("5GHz radio");
		if (wait_status != 0)
			return wait_status;
	}

	nvram_wlan_set_int(1, "radio_x", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "5GHz", "radio");

	return control_radio_wl(radio_on, 1);
#else
	return 0;
#endif
}

int 
manual_change_guest_rt(int radio_on)
{
	int wait_status;
	if (get_enabled_guest_rt() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(0);
		wait_status = wait_wifi_watchdog_update("2.4GHz guest AP");
		if (wait_status != 0)
			return wait_status;
	}

	nvram_wlan_set_int(0, "guest_enable", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "2.4GHz", "AP Guest");

	return control_guest_rt(radio_on, 1);
}

int
manual_change_guest_wl(int radio_on)
{
	int wait_status;
#if BOARD_HAS_5G_RADIO
	if (get_enabled_guest_wl() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(1);
		wait_status = wait_wifi_watchdog_update("5GHz guest AP");
		if (wait_status != 0)
			return wait_status;
	}

	nvram_wlan_set_int(1, "guest_enable", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "5GHz", "AP Guest");

	return control_guest_wl(radio_on, 1);
#else
	return 0;
#endif
}

int 
timecheck_wifi(int is_aband, const char *nv_date, const char *nv_time1, const char *nv_time2)
{
#define DOW_MASK_SUN (1 << 0)
#define DOW_MASK_MON (1 << 1)
#define DOW_MASK_TUE (1 << 2)
#define DOW_MASK_WED (1 << 3)
#define DOW_MASK_THU (1 << 4)
#define DOW_MASK_FRI (1 << 5)
#define DOW_MASK_SAT (1 << 6)

	time_t now;
	struct tm *tm;
	char *aDate, *aTime1, *aTime2;
	int i, current_min, current_dow, schedul_dow, iTime1B, iTime1E, iTime2B, iTime2E;

	aDate  = nvram_wlan_get(is_aband, nv_date);
	aTime1 = nvram_wlan_get(is_aband, nv_time1);
	aTime2 = nvram_wlan_get(is_aband, nv_time2);

	if (strlen(aDate) != 7 || strlen(aTime1) != 8 || strlen(aTime2) != 8)
		return 1;

	if (strcmp(aDate, "1111111")==0 &&
	    strcmp(aTime1, "00002359")==0 &&
	    strcmp(aTime2, "00002359")==0)
		return 1;

	// Mon..Fri time
	iTime1B = ((aTime1[0]-'0')*10 + (aTime1[1]-'0'))*60 + (aTime1[2]-'0')*10 + (aTime1[3]-'0');
	iTime1E = ((aTime1[4]-'0')*10 + (aTime1[5]-'0'))*60 + (aTime1[6]-'0')*10 + (aTime1[7]-'0');

	// Sat, Sun time
	iTime2B = ((aTime2[0]-'0')*10 + (aTime2[1]-'0'))*60 + (aTime2[2]-'0')*10 + (aTime2[3]-'0');
	iTime2E = ((aTime2[4]-'0')*10 + (aTime2[5]-'0'))*60 + (aTime2[6]-'0')*10 + (aTime2[7]-'0');

	time(&now);
	tm = localtime(&now);
	current_min = tm->tm_hour * 60 + tm->tm_min;
	current_dow = 1 << tm->tm_wday;

	schedul_dow = 0;
	for(i=0; i<7; i++){
		if (aDate[i] == '1')
			schedul_dow |= (1 << i);
	}

	/* Saturday */
	if (current_dow & DOW_MASK_SAT)
	{
		if (schedul_dow & DOW_MASK_SAT)
		{
			if (iTime2B < iTime2E)
			{
				if (current_min >= iTime2B && current_min <= iTime2E)
					return 1;
			}
			else
			{
				if (current_min >= iTime2B)
					return 1;
				
				/* Check Friday -> Saturday after midnight (special check after workweek) */
				if ((schedul_dow & DOW_MASK_FRI) && (iTime1B < iTime1E) && (current_min <= iTime2E))
					return 1;
			}
		}
		
		/* Check cross-night from Friday */
		if ((schedul_dow & DOW_MASK_FRI) && (iTime1B >= iTime1E) && (current_min <= iTime1E))
			return 1;
	}
	else /* Sunday */
	if (current_dow & DOW_MASK_SUN)
	{
		if (schedul_dow & DOW_MASK_SUN)
		{
			if (iTime2B < iTime2E)
			{
				if (current_min >= iTime2B && current_min <= iTime2E)
					return 1;
			}
			else
			{
				if (current_min >= iTime2B)
					return 1;
			}
		}
		
		/* Check cross-night from Saturday */
		if ((schedul_dow & DOW_MASK_SAT) && (iTime2B >= iTime2E) && (current_min <= iTime2E))
			return 1;
	}
	else /* Monday */
	if (current_dow & DOW_MASK_MON)
	{
		if (schedul_dow & DOW_MASK_MON)
		{
			if (iTime1B < iTime1E)
			{
				if (current_min >= iTime1B && current_min <= iTime1E)
					return 1;
			}
			else
			{
				if (current_min >= iTime1B)
					return 1;
				
				/* Check Sunday -> Monday after midnight (special check after weekend) */
				if ((schedul_dow & DOW_MASK_SUN) && (iTime2B < iTime2E) && (current_min <= iTime1E))
					return 1;
			}
		}
		
		/* Check cross-night from Sunday */
		if ((schedul_dow & DOW_MASK_SUN) && (iTime2B >= iTime2E) && (current_min <= iTime2E))
			return 1;
	}
	else /* Tuesday..Friday */
	{
		if (schedul_dow & current_dow)
		{
			if (iTime1B < iTime1E)
			{
				if (current_min >= iTime1B && current_min <= iTime1E)
					return 1;
			}
			else
			{
				if (current_min >= iTime1B)
					return 1;
			}
		}
		
		/* Check cross-night from previous day */
		if ((schedul_dow & (current_dow >> 1)) && (iTime1B >= iTime1E) && (current_min <= iTime1E))
			return 1;
	}

	return 0;
}

