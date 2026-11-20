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

/*
 * RT3352 MII radio state crosses a switch-isolation boundary. Do not publish
 * state until the driver and switch both complete the requested transition.
 */
typedef struct wireless_inic_radio_ops {
	int (*set_power)(void *context, int is_on);
	int (*set_isolation)(void *context, int isolated);
	void (*prepare_on)(void *context);
	void (*publish)(void *context, int is_on);
} wireless_inic_radio_ops_t;

static inline int
wireless_inic_radio_transition(int is_on, const wireless_inic_radio_ops_t *ops,
			       void *context)
{
	int isolation_status;
	int power_status;

	if (!ops || !ops->set_power || !ops->set_isolation || !ops->publish)
		return -EINVAL;

	if (is_on) {
		/*
		 * Do not assume the switch retained its previous state across boot or
		 * an out-of-band change. Establish isolation before enabling the radio.
		 */
		isolation_status = ops->set_isolation(context, 1);
		if (isolation_status != 0) {
			(void)ops->set_power(context, 0);
			return isolation_status;
		}
		power_status = ops->set_power(context, 1);
		if (power_status != 0) {
			/* The failed command may have changed radio hardware partially. */
			(void)ops->set_power(context, 0);
			return power_status;
		}
		if (ops->prepare_on)
			ops->prepare_on(context);
		isolation_status = ops->set_isolation(context, 0);
		if (isolation_status != 0) {
			/* Best-effort fail-closed rollback; preserve the original cause. */
			(void)ops->set_isolation(context, 1);
			(void)ops->set_power(context, 0);
			return isolation_status;
		}
	} else {
		/* Still attempt radio-off when switch isolation itself fails. */
		isolation_status = ops->set_isolation(context, 1);
		power_status = ops->set_power(context, 0);
		if (isolation_status != 0)
			return isolation_status;
		if (power_status != 0)
			return power_status;
	}

	ops->publish(context, !!is_on);
	return 0;
}

#endif /* WIRELESS_RADIO_POLICY_H */
