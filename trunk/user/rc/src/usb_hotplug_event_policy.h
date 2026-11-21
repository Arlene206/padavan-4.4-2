#ifndef PADAVAN_USB_HOTPLUG_EVENT_POLICY_H
#define PADAVAN_USB_HOTPLUG_EVENT_POLICY_H

#include <errno.h>

/*
 * Evaluate an unplug/plug marker pair only when both reads succeeded. On
 * error, leave both outputs clear so callers cannot acknowledge one side
 * while the ordering of the other side is unknown.
 */
static inline int
usb_hotplug_event_pair_status(int unplug_read_status, int unplug_value,
			      int plug_read_status, int plug_value,
			      int *unplug_pending, int *plug_pending)
{
	if (!unplug_pending || !plug_pending)
		return EINVAL;
	*unplug_pending = 0;
	*plug_pending = 0;
	if (unplug_read_status != 0)
		return unplug_read_status > 0 ? unplug_read_status : EIO;
	if (plug_read_status != 0)
		return plug_read_status > 0 ? plug_read_status : EIO;
	*unplug_pending = unplug_value == 1;
	*plug_pending = plug_value == 1;
	return 0;
}

#endif /* PADAVAN_USB_HOTPLUG_EVENT_POLICY_H */
