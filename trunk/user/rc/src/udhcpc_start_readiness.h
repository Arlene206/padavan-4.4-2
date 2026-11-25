#ifndef PADAVAN_UDHCPC_START_READINESS_H
#define PADAVAN_UDHCPC_START_READINESS_H

#include <errno.h>
#include <process_pidfile.h>
#include "process_presence_start_policy.h"
#include "service_wait_policy.h"

/*
 * BusyBox udhcpc -d and -b detach before publishing their PID/start_time
 * record. _eval() waits only for the pre-detach process, so a zero launcher
 * status is not proof that the DHCP client survived. Verify the exact child
 * identity with bounded retries before RC reports a successful start.
 */
static inline int
udhcpc_start_and_verify(char *const argv[], const char *pidfile,
			const char *ifname, const char *log_prefix)
{
	struct process_identity identity;
	enum process_presence presence;
	int launch_status, postcondition_status, wait_status;
	int attempt;

	if (!argv || !pidfile || !pidfile[0] || !ifname || !ifname[0] ||
	    !log_prefix || !log_prefix[0])
		return EINVAL;

	logmessage(log_prefix, "starting on %s ...", ifname);
	launch_status = _eval(argv, NULL, 0, NULL);
	if (launch_status != 0)
		return launch_status;

	wait_status = service_wait_seconds(
		PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (wait_status != 0) {
		wait_status = errno > 0 ? errno : EIO;
		logmessage(log_prefix,
			   "initial DHCP child readiness wait failed on %s (status=%d)",
			   ifname, wait_status);
		errno = wait_status;
		return wait_status;
	}

	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS;
	     ++attempt) {
		presence = process_pidfile_read_identity(pidfile, "udhcpc",
							 &identity);
		postcondition_status = process_presence_start_postcondition(
			launch_status, presence);
		if (postcondition_status == 0)
			return 0;
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			postcondition_status = errno > 0 ? errno : EIO;
			logmessage(log_prefix,
				   "cannot verify DHCP child identity on %s (status=%d)",
				   ifname, postcondition_status);
			errno = postcondition_status;
			return postcondition_status;
		}
		if (presence != PROCESS_PRESENCE_ABSENT) {
			logmessage(log_prefix,
				   "unexpected DHCP child process state on %s",
				   ifname);
			errno = EIO;
			return EIO;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			wait_status = service_wait_seconds(
				PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (wait_status != 0) {
				wait_status = errno > 0 ? errno : EIO;
				logmessage(log_prefix,
					   "DHCP child readiness retry wait failed on %s (status=%d)",
					   ifname, wait_status);
				errno = wait_status;
				return wait_status;
			}
		}
	}

	errno = ETIMEDOUT;
	logmessage(log_prefix,
		   "DHCP child did not publish a live PID identity on %s (status=%d)",
		   ifname, ETIMEDOUT);
	return ETIMEDOUT;
}

#endif /* PADAVAN_UDHCPC_START_READINESS_H */
