#ifndef HTTPD_DAEMON_POLICY_H
#define HTTPD_DAEMON_POLICY_H

#include <errno.h>
#include "process_inventory.h"

#define HTTPD_START_INITIAL_WAIT_SECONDS 1
#define HTTPD_START_RETRY_WAIT_SECONDS 1
#define HTTPD_START_PROBE_ATTEMPTS 4

/* A successful _eval() only confirms that the launcher exited successfully;
 * the detached HTTPD child must also remain observable. */
static inline int
httpd_start_postcondition(int launcher_status,
			  enum process_presence process_presence)
{
	if (launcher_status != 0)
		return launcher_status;

	if (process_presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_presence == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	return EIO;
}

/* Only a complete scan that confirms absence authorizes configuration/start. */
static inline int
httpd_start_can_launch(enum process_presence process_presence)
{
	return process_presence == PROCESS_PRESENCE_ABSENT;
}

/* Preserve duplicate status separately from an unobservable process table. */
static inline int
httpd_start_precondition_status(enum process_presence process_presence)
{
	if (process_presence == PROCESS_PRESENCE_RUNNING)
		return EBUSY;
	if (process_presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return EIO;
}

/* Do not publish HTTPD as stopped while the old process may still be alive. */
static inline int
httpd_stop_policy_publish_stopped(int stop_status)
{
	return stop_status == 0;
}

/* Never start a replacement until the old HTTPD process is confirmed gone. */
static inline int
httpd_restart_can_start(int stop_status)
{
	return stop_status == 0;
}

#endif /* HTTPD_DAEMON_POLICY_H */
