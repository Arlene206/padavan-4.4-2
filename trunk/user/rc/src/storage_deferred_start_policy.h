#ifndef PADAVAN_STORAGE_DEFERRED_START_POLICY_H
#define PADAVAN_STORAGE_DEFERRED_START_POLICY_H

#include <errno.h>

static inline int
storage_deferred_start_dispatch_status(int unplug_read_status,
				       int unplug_pending,
				       int hotplug_read_status,
				       int hotplug_pending,
				       int optware_read_status,
				       int optware_pending,
				       int worker_result_pending,
				       int *should_start)
{
	if (!should_start)
		return EINVAL;
	*should_start = 0;
	if (unplug_read_status != 0)
		return unplug_read_status > 0 ? unplug_read_status : EIO;
	if (hotplug_read_status != 0)
		return hotplug_read_status > 0 ? hotplug_read_status : EIO;
	if (optware_read_status != 0)
		return optware_read_status > 0 ? optware_read_status : EIO;
	*should_start = !unplug_pending &&
		(hotplug_pending || optware_pending || worker_result_pending);
	return 0;
}

/*
 * Retain successful cleanup completion while its NVRAM marker clear is retried.
 * Do not repeat the eject operation unless the cleanup itself failed.
 */
static inline int
storage_deferred_event_action_should_run(int event_pending,
					 int action_ack_pending)
{
	return event_pending && !action_ack_pending;
}

static inline int
storage_deferred_event_ack_status(int event_pending, int action_status,
				  int marker_status, int *action_ack_pending)
{
	if (!action_ack_pending)
		return EINVAL;
	if (!event_pending) {
		*action_ack_pending = 0;
		return 0;
	}
	if (action_status != 0) {
		*action_ack_pending = 0;
		return action_status > 0 ? action_status : EIO;
	}
	if (marker_status == 0) {
		*action_ack_pending = 0;
		return 0;
	}
	*action_ack_pending = 1;
	return marker_status > 0 ? marker_status : EIO;
}

#endif /* PADAVAN_STORAGE_DEFERRED_START_POLICY_H */
