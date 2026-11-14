#ifndef GPIO_BTN_DAEMON_POLICY_H
#define GPIO_BTN_DAEMON_POLICY_H

#include <errno.h>

#include "process_inventory.h"

#define GPIO_BTN_START_WAIT_ATTEMPTS 4

/*
 * eval() waits for the launcher process, while daemon() detaches the child.
 * Only a complete process inventory may prove that the daemon is running or
 * absent. UNKNOWN must never be converted into a successful start.
 */
static inline int
gpio_btn_start_policy_status(int launch_status,
			     enum process_presence daemon_presence)
{
	if (launch_status != 0)
		return launch_status;

	if (daemon_presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (daemon_presence == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	return EIO;
}

#endif /* GPIO_BTN_DAEMON_POLICY_H */
