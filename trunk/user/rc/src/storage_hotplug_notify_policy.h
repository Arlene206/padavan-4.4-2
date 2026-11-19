#ifndef __STORAGE_HOTPLUG_NOTIFY_POLICY_H__
#define __STORAGE_HOTPLUG_NOTIFY_POLICY_H__

/* A successful hotplug operation is complete only after PID 1 accepted its event. */
static inline int
storage_hotplug_completion_status(int operation_status, int notification_status)
{
	return (operation_status == 0 && notification_status == 0) ? 0 : 1;
}

#endif
