#ifndef HTTPD_DAEMON_POLICY_H
#define HTTPD_DAEMON_POLICY_H

#include <errno.h>

#define HTTPD_START_INITIAL_WAIT_SECONDS 1
#define HTTPD_START_RETRY_WAIT_SECONDS 1
#define HTTPD_START_PROBE_ATTEMPTS 4

/* A successful _eval() only confirms that the launcher exited successfully;
 * the detached HTTPD child must also remain observable. */
static inline int
httpd_start_postcondition(int launcher_status, int process_present)
{
	if (launcher_status != 0)
		return launcher_status;

	return process_present ? 0 : ESRCH;
}

/* Do not start another HTTPD if an earlier daemon is already present. */
static inline int
httpd_start_can_launch(int process_present)
{
	return !process_present;
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
