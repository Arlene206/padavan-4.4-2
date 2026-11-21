#ifndef RC_MODEM_WAN_START_POLICY_H
#define RC_MODEM_WAN_START_POLICY_H

#include <errno.h>

/* Do not migrate the active WAN interface while miniupnpd is still bound. */
static inline int
modem_wan_start_migration_succeeded(int migration_status)
{
	return migration_status == 0;
}

/*
 * Some NDIS drivers establish data sessions through a required user-space
 * control command; others have no user-space control path. Do not treat an
 * optional no-op as a failure, but never discard a required command failure.
 */
static inline int
modem_ndis_control_failed(int control_required, int control_status)
{
	return control_required && control_status != 0;
}

/* Remove a credential-bearing RAS peer file when its pppd launch fails. */
static inline int
modem_wan_ras_peer_cleanup_required(int launch_status)
{
	return launch_status != 0;
}

/*
 * USB removal races network-device teardown. ENODEV from a fresh flags probe
 * means the interface has already disappeared; an extant or uninspectable
 * interface must not hide a failed ifconfig-down operation.
 */
static inline int
modem_wan_unplug_interface_down_result(int ifconfig_status,
				       int interface_flags, int interface_error,
				       int absent_error)
{
	if (ifconfig_status == 0)
		return 0;
	if (interface_flags >= 0)
		return ifconfig_status;
	if (interface_error == absent_error)
		return 0;
	return interface_error > 0 ? interface_error : EIO;
}

/* Preserve the first failed transition; DHCP status matters only afterward. */
static inline int
modem_wan_start_status(int migration_status, int dhcp_status)
{
	return migration_status != 0 ? migration_status : dhcp_status;
}

/*
 * Select the modem only from a complete configuration snapshot. A failed
 * priority read or uncertain Ethernet link must never be interpreted as
 * permission to replace the currently selected WAN transport.
 */
static inline int
modem_wan_select_device_status(int modem_devnum, int priority_status,
			       int modem_priority, int is_apcli_wisp,
			       int link_probe_status, int has_ethernet_link,
			       int *selected_devnum)
{
	if (!selected_devnum)
		return EINVAL;
	if (modem_devnum < 0)
		return EINVAL;

	*selected_devnum = modem_devnum;
	if (modem_devnum == 0) {
		*selected_devnum = 0;
		return 0;
	}
	if (priority_status != 0)
		return priority_status;
	if (modem_priority < 1) {
		*selected_devnum = 0;
		return 0;
	}
	if (modem_priority == 2) {
		if (is_apcli_wisp) {
			*selected_devnum = 0;
			return 0;
		}
		if (link_probe_status != 0)
			return link_probe_status;
		if (has_ethernet_link != 0 && has_ethernet_link != 1)
			return EIO;
		if (has_ethernet_link)
			*selected_devnum = 0;
	}

	return 0;
}

/*
 * Preserve a queued replug while an unplug transition is still running.
 * A failed transition is retried by restoring its matching hotplug event.
 */
#define MODEM_WAN_HOTPLUG_RETRY_UNPLUG 0x01U
#define MODEM_WAN_HOTPLUG_RETRY_PLUG   0x02U

static inline unsigned int
modem_wan_hotplug_retry_mask(int unplug_pending, int plug_pending,
			     int unplug_status, int plug_status)
{
	unsigned int retry_mask = 0;

	if (unplug_pending && unplug_status != 0)
		retry_mask |= MODEM_WAN_HOTPLUG_RETRY_UNPLUG;
	if (plug_pending && (unplug_pending || plug_status != 0))
		retry_mask |= MODEM_WAN_HOTPLUG_RETRY_PLUG;

	return retry_mask;
}

/*
 * Hotplug metadata is not enough to complete a modem transition unless PID 1
 * accepted the corresponding secure notification queue markers.
 */
static inline int
modem_wan_hotplug_notification_result(int existing_status, int queue_status)
{
	return existing_status != 0 ? existing_status : (queue_status == 0 ? 0 : 1);
}



typedef int (*modem_wan_nvram_set_value_fn)(void *context,
					    const char *param_name,
					    const char *value);

/*
 * Clear the active interface before its protocol label. Always attempt both
 * writes and return the first cleanup error so stale identity is not treated
 * as successfully removed.
 */
static inline int
modem_wan_clear_ndis_identity(modem_wan_nvram_set_value_fn set_value,
			      void *context)
{
	int status, first_status = 0;

	if (!set_value)
		return EINVAL;

	status = set_value(context, "ifname_t", "");
	if (status != 0)
		first_status = status;
	status = set_value(context, "proto_t", "");
	if (first_status == 0 && status != 0)
		first_status = status;

	return first_status;
}

/*
 * Publish the NDIS protocol label and active interface before bringing the
 * device up. If either NVRAM write fails, clear both fields in reverse order;
 * preserve the publication error and report the first cleanup error separately.
 */
static inline int
modem_wan_publish_ndis_identity(modem_wan_nvram_set_value_fn set_value,
				void *context, const char *ifname,
				int *rollback_status)
{
	int status, cleanup_status;

	if (rollback_status)
		*rollback_status = 0;
	if (!set_value || !ifname || !ifname[0])
		return EINVAL;

	status = set_value(context, "proto_t", "NDIS Modem");
	if (status == 0)
		status = set_value(context, "ifname_t", ifname);
	if (status == 0)
		return 0;

	/* A failed setter may have partially changed backend state. Clear both. */
	cleanup_status = modem_wan_clear_ndis_identity(set_value, context);
	if (rollback_status)
		*rollback_status = cleanup_status;
	return status;
}

/*
 * Decide whether a deferred USB unplug needs WAN reconnection only from a
 * successful modem_rule read. An absent NVRAM key is passed as zero, matching
 * the historical no-modem default; backend errors must keep the event queued.
 */
static inline int
modem_wan_unplug_event_status(int read_status, int modem_rule,
			      int *unplug_required)
{
	if (!unplug_required)
		return EINVAL;
	*unplug_required = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	*unplug_required = modem_rule > 0;
	return 0;
}

#endif
