#ifndef PADAVAN_FTP_DAEMON_POLICY_H
#define PADAVAN_FTP_DAEMON_POLICY_H

#include <errno.h>

#include "process_inventory.h"
#include "storage_service_stop_policy.h"

/* A failed detached launch is considered active unless absence is proven. */
static int
ftp_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
}

/* Reconcile only after procfs positively confirms the final daemon absence.
 * Preserve kill_services() errors separately: a partial stop failure can
 * still leave stale FTP/UPnP rules after the process has exited. */
static inline int
ftp_stop_should_reconcile_firewall(int final_presence)
{
	return final_presence == PROCESS_PRESENCE_ABSENT;
}

/* A confirmed absent daemon requires rule reconciliation even if it was
 * already absent before stop: stale NAT/UPnP state can outlive a crash. */
static inline int
ftp_firewall_reconcile_needed(int before, int after)
{
	return storage_service_process_presence_changed(before, after) ||
	       ftp_stop_should_reconcile_firewall(after);
}

/* Never reconcile FTP firewall/UPnP state while the daemon may still
 * exist. A pre-stop observation cannot override RUNNING or UNKNOWN after
 * the stop; only complete final procfs absence authorizes cleanup. */
static inline int
ftp_storage_stop_needs_firewall_refresh(int final_presence)
{
	return ftp_stop_should_reconcile_firewall(final_presence);
}

/* A successful kill command is insufficient unless final procfs state is
 * ABSENT. Preserve any detailed service_lifecycle_stop() failure verbatim. */
static inline int
ftp_stop_status_result(int command_status, int final_presence)
{
	if (command_status != 0)
		return command_status;
	return final_presence == PROCESS_PRESENCE_ABSENT ? 0 : -1;
}

/* Do not turn an NVRAM backend error into an implicit disabled value. */
static inline int
ftp_enable_value_status(int read_status, int value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	*enabled = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value != 0 && value != 1)
		return EINVAL;
	*enabled = value;
	return 0;
}

#endif /* PADAVAN_FTP_DAEMON_POLICY_H */
