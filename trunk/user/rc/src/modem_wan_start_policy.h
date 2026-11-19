#ifndef RC_MODEM_WAN_START_POLICY_H
#define RC_MODEM_WAN_START_POLICY_H

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

#endif
