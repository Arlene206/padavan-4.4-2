#ifndef PADAVAN_FTP_DAEMON_POLICY_H
#define PADAVAN_FTP_DAEMON_POLICY_H

#include "process_inventory.h"

/* A failed detached launch is considered active unless absence is proven. */
static int
ftp_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
}

#endif /* PADAVAN_FTP_DAEMON_POLICY_H */
