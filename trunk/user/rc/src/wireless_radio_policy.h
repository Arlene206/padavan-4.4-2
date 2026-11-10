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

#endif /* WIRELESS_RADIO_POLICY_H */
