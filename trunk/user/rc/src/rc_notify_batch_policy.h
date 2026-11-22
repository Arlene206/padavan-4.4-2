#ifndef RC_NOTIFY_BATCH_POLICY_H
#define RC_NOTIFY_BATCH_POLICY_H

#define RC_NOTIFY_BATCH_LIMIT 10

#include <string.h>
#define RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG   (1U << 0)
#define RC_NOTIFY_HOTPLUG_RETRY_PRINTER_UNPLUG (1U << 1)
#define RC_NOTIFY_HOTPLUG_RETRY_MODEM_PLUG     (1U << 2)
#define RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG   (1U << 3)
#define RC_NOTIFY_HOTPLUG_RETRY_STORAGE_PLUG   (1U << 4)
static inline unsigned int
rc_notify_hotplug_retry_flag(const char *event_name)
{
	if (!event_name) return 0;
	if (!strcmp(event_name, "on_hotplug_usb_printer")) return RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG;
	if (!strcmp(event_name, "on_unplug_usb_printer")) return RC_NOTIFY_HOTPLUG_RETRY_PRINTER_UNPLUG;
	if (!strcmp(event_name, "on_hotplug_usb_modem")) return RC_NOTIFY_HOTPLUG_RETRY_MODEM_PLUG;
	if (!strcmp(event_name, "on_unplug_usb_modem")) return RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG;
	if (!strcmp(event_name, "on_hotplug_mass_storage")) return RC_NOTIFY_HOTPLUG_RETRY_STORAGE_PLUG;
	return 0;
}
static inline unsigned int
rc_notify_hotplug_retry_record(unsigned int pending, const char *event_name, int marker_status)
{
	unsigned int flag = rc_notify_hotplug_retry_flag(event_name);
	if (marker_status != 0) pending |= flag;
	return pending;
}
static inline unsigned int
rc_notify_hotplug_retry_complete(unsigned int pending, unsigned int queued, int signal_status)
{
	return (signal_status == 0) ? (pending & ~queued) : pending;
}

/* Requeue only after a full bounded batch when safe work remains. */
static inline int
rc_notify_batch_should_requeue(unsigned int entries_seen, int queue_pending,
			       int stop_handle)
{
	return !stop_handle && queue_pending &&
	       entries_seen >= RC_NOTIFY_BATCH_LIMIT;
}

/* Notification handlers retain the incomplete marker only for negative
 * action results. Normalize errno-style positive service failures as well. */
static inline int
rc_notify_merge_action_result(int current_result, int operation_status)
{
	return (current_result < 0 || operation_status != 0) ? -1 : 0;
}

#endif /* RC_NOTIFY_BATCH_POLICY_H */
