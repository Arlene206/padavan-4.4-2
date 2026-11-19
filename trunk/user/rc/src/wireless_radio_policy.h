#ifndef WIRELESS_RADIO_POLICY_H
#define WIRELESS_RADIO_POLICY_H

#include <errno.h>

/*
 * A radio operation is considered successful only when the driver command or
 * the enclosing restart transaction returns the conventional zero status.
 */
static inline int
wireless_radio_status_success(int operation_status)
{
	return operation_status == 0;
}

/*
 * Preserve the first failed wireless operation when interface setup has more
 * than one required step (for example, ifconfig followed by HW NAT
 * registration). A later success must never hide an earlier failure.
 */
static inline int
wireless_radio_status_merge(int first_status, int next_status)
{
	return first_status != 0 ? first_status : next_status;
}

/*
 * Preserve a failed interface-down result while still attempting every
 * remaining interface during radio shutdown.
 */
static inline int
wireless_radio_shutdown_status(int all_stopped, int stop_succeeded)
{
	return all_stopped && stop_succeeded;
}

/* Convert the aggregate stop result to the RC restart convention. */
static inline int
wireless_radio_shutdown_result(int all_stopped)
{
	return all_stopped ? 0 : -EIO;
}

/*
 * Preserve an incomplete interface-up result while WDS startup continues
 * attempting the remaining configured interfaces.
 */
static inline int
wireless_radio_startup_status(int all_started, int start_succeeded)
{
	return all_started && start_succeeded;
}

/* Convert the aggregate startup result to the RC restart convention. */
static inline int
wireless_radio_startup_result(int all_started)
{
	return all_started ? 0 : -EIO;
}

/*
 * Merge the IAPP helper's wait status into a wireless restart result.
 * system() reports both fork errors (-1) and encoded child failures (>0);
 * normalize either to a negative RC error while preserving an earlier
 * guest-isolation failure.
 */
static inline int
wireless_radio_restart_status(int previous_status, int operation_status)
{
	if (previous_status != 0)
		return previous_status;
	return operation_status == 0 ? 0 : -EIO;
}

/*
 * control_radio_* retains its legacy boolean "state changed" result. A no-op
 * is successful only when the radio was already at the requested state.
 */
static inline int
wireless_radio_notification_result(int was_on, int requested_on,
				   int change_reported)
{
	if (!!was_on == !!requested_on)
		return 0;
	return change_reported == 1 ? 0 : -EIO;
}

#endif /* WIRELESS_RADIO_POLICY_H */
