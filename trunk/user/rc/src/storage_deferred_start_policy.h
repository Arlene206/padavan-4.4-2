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

#endif /* PADAVAN_STORAGE_DEFERRED_START_POLICY_H */
