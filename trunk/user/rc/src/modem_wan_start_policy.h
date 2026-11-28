#ifndef RC_MODEM_WAN_START_POLICY_H
#define RC_MODEM_WAN_START_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * A QMI session is usable only after uqmi succeeded and published a complete,
 * nonnegative packet-data handle. File presence alone is not a session result.
 */
static inline int
modem_qmi_start_attempt_status(int command_status, int packet_data_handle)
{
	if (command_status != 0)
		return command_status > 0 ? command_status : EIO;
	return packet_data_handle >= 0 ? 0 : EIO;
}

/* The modem UI exposes only Auto and the nine explicit network profiles. */
static inline int
modem_qmi_network_mode_is_valid(int network_mode)
{
	return network_mode >= 0 && network_mode <= 9;
}

/* Normalize the wait status returned by the command runner. */
static inline int
modem_qmi_command_status(int command_status)
{
	if (command_status == 0)
		return 0;
	return command_status > 0 ? command_status : EIO;
}

/*
 * A successful QMI response without its persisted PDH, or a failed command
 * that nevertheless published a PDH, may represent an active untracked
 * session. Stop instead of creating a duplicate session on retry.
 */
static inline int
modem_qmi_start_attempt_is_ambiguous(int command_status,
				     int packet_data_handle, int handle_read_status)
{
	if (command_status == 0)
		return packet_data_handle < 0;
	return packet_data_handle >= 0 || handle_read_status != ENOENT;
}

/* Do not discard an existing session marker and start a second data call. */
static inline int
modem_qmi_existing_pdh_start_status(int packet_data_handle, int read_status)
{
	if (packet_data_handle >= 0)
		return EBUSY;
	if (read_status == ENOENT)
		return 0;
	return read_status > 0 ? read_status : EIO;
}

/* A persisted WDS CID is reusable only when it fits the QMI one-byte field. */
static inline int
modem_qmi_client_id_is_valid(int client_id)
{
	return client_id >= 0 && client_id <= UCHAR_MAX;
}

static inline int
modem_qmi_client_id_should_allocate(int client_id, int read_status)
{
	return client_id < 0 && read_status == ENOENT;
}

/*
 * A stop command must target the persisted WDS CID that owns the session.
 * Do not let uqmi silently allocate a fresh CID when its recovery marker is
 * missing: that CID cannot address an older packet-data handle.
 */
static inline int
modem_qmi_stop_preflight(int client_id, int client_id_status,
			 int packet_data_handle, int packet_data_status,
			 int *stop_required)
{
	if (!stop_required)
		return EINVAL;
	*stop_required = 0;

	if (client_id_status == 0) {
		if (!modem_qmi_client_id_is_valid(client_id))
			return EINVAL;
		*stop_required = 1;
		return 0;
	}
	if (client_id_status != ENOENT)
		return client_id_status > 0 ? client_id_status : EIO;
	if (client_id != -1)
		return EINVAL;

	/* A missing CID is harmless only when no PDH says a session may exist. */
	if (packet_data_handle >= 0)
		return EIO;
	if (packet_data_status == ENOENT)
		return 0;
	return packet_data_status > 0 ? packet_data_status : EIO;
}

/* A valid PDH from an ambiguous start may be the only recovery handle. */
/* Preserve non-missing PDH state while a start result is ambiguous. */
static inline int
modem_qmi_start_attempt_preserve_handle(int ambiguous_session,
					int packet_data_handle, int handle_read_status)
{
	return ambiguous_session &&
		(packet_data_handle >= 0 || handle_read_status != ENOENT);
}

/* Missing old status files are benign; other unlink failures block startup. */
static inline int
modem_qmi_handle_file_unlink_status(int unlink_status, int saved_errno)
{
	if (unlink_status == 0 || (unlink_status < 0 && saved_errno == ENOENT))
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

/* Do not release the WDS client ID while a stopped-session marker remains. */
static inline int
modem_qmi_client_id_release_allowed(int packet_data_handle_cleanup_status)
{
	return packet_data_handle_cleanup_status == 0;
}

/* Parse exactly one nonnegative decimal handle, allowing surrounding whitespace. */
static inline int
modem_qmi_parse_handle(const char *text, int *handle)
{
	char *end;
	long value;

	if (!text || !handle)
		return EINVAL;
	*handle = -1;
	errno = 0;
	value = strtol(text, &end, 10);
	if (end == text || errno == ERANGE || value < 0 || value > INT_MAX)
		return EINVAL;
	while (*end && isspace((unsigned char)*end))
		end++;
	if (*end)
		return EINVAL;
	*handle = (int)value;
	return 0;
}

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
/*
 * qmi_wwan cannot establish a data session without its matching WDM control
 * node. Treat a missing per-device node as a hard startup error, not as an
 * optional driver with no userspace control operation.
 */
static inline int
modem_ndis_qmi_control_node_status(int node_found)
{
	return node_found ? 0 : ENODEV;
}

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

/* Normalize a direct Ethernet-link probe without confusing UNKNOWN for DOWN. */
static inline int
modem_wan_link_probe_error(int link_result, int saved_errno)
{
	if (link_result == 0 || link_result == 1)
		return 0;
	if (link_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	return EIO;
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
 * Reconnect after a physical WAN link transition only when modem priority,
 * device readiness and the checked modem-WAN identity agree with link state.
 * has_link retains the legacy truthy/falsy interpretation.
 */
static inline int
modem_wan_link_change_reconnect_enabled(int modem_priority, int modem_devnum,
					int is_modem_wan, int has_link,
					int *reconnect)
{
	if (!reconnect)
		return EINVAL;
	*reconnect = 0;
	if (modem_devnum < 0 ||
	    (is_modem_wan != 0 && is_modem_wan != 1))
		return EINVAL;

	*reconnect = modem_priority == 2 && modem_devnum > 0 &&
		     (is_modem_wan == (has_link != 0));
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

/*
 * Preserve the legacy NVRAM match/invmatch gate for a newly enumerated modem.
 * Missing keys never match, including the inverted modem_prio/modem_type
 * checks. Backend failures are handled by the caller as retryable events.
 */
static inline int
modem_wan_hotplug_notification_enabled(int priority_present,
				       const char *priority,
				       int rule_present, const char *rule,
				       int type_present, const char *type,
				       int is_ndis, int *notify)
{
	if (!notify)
		return EINVAL;
	*notify = 0;
	if ((is_ndis != 0 && is_ndis != 1) ||
	    (priority_present != 0 && priority_present != 1) ||
	    (rule_present != 0 && rule_present != 1) ||
	    (type_present != 0 && type_present != 1))
		return EINVAL;
	if (!priority_present || !rule_present || !type_present)
		return 0;
	if (!priority || !rule || !type)
		return EIO;
	*notify = strcmp(priority, "0") != 0 &&
		  strcmp(rule, "1") == 0 &&
		  (is_ndis ? strcmp(type, "3") == 0 : strcmp(type, "3") != 0);
	return 0;
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

/*
 * A physical modem unplug must notify WAN even when its identity snapshot is
 * unreadable; skipping the event would leave stale WAN ownership behind.
 */
static inline int
modem_wan_unplug_notification_required(int identity_status, int is_modem)
{
	return identity_status != 0 || is_modem != 0;
}

/* Do not auto-select a modem unless the current WAN identity is known. */
static inline int
modem_wan_auto_start_allowed(int identity_status, int is_modem)
{
	return identity_status == 0 && is_modem == 0;
}

/* Never reload/unload modem modules against an unknown WAN identity. */
static inline int
modem_wan_restart_allowed(int identity_status)
{
	return identity_status == 0;
}

/*
 * Complete a retry backoff even when sleep(3) is interrupted. A successful
 * sleep returns zero; an interrupted sleep returns the unslept remainder.
 */
static inline void
modem_qmi_retry_wait_full_delay(unsigned int seconds,
				unsigned int (*sleep_fn)(unsigned int))
{
	while (seconds != 0)
		seconds = sleep_fn(seconds);
}

#endif
