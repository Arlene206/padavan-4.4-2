#ifndef PADAVAN_MINIUPNPD_DAEMON_POLICY_H
#define PADAVAN_MINIUPNPD_DAEMON_POLICY_H

#include <errno.h>

#define MINIUPNPD_START_WAIT_ATTEMPTS 4

/* eval() observes the daemonizing parent, not necessarily its child. */
static int
miniupnpd_start_policy_status(int launch_status, int daemon_running)
{
	if (launch_status != 0)
		return launch_status;
	return daemon_running ? 0 : ESRCH;
}

#endif /* PADAVAN_MINIUPNPD_DAEMON_POLICY_H */
