#ifndef GPIO_BTN_DAEMON_POLICY_H
#define GPIO_BTN_DAEMON_POLICY_H

#include <errno.h>

#define GPIO_BTN_START_WAIT_ATTEMPTS 4

/* eval() waits for the launcher process; daemon() may exit that process while
 * its child fails immediately during netlink socket initialization. */
static inline int
gpio_btn_start_policy_status(int launch_status, int daemon_running)
{
	if (launch_status != 0)
		return launch_status;
	return daemon_running ? 0 : ESRCH;
}

#endif /* GPIO_BTN_DAEMON_POLICY_H */
