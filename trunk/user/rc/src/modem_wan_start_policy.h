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

#endif
