#ifndef WIRELESS_RADIO_POLICY_H
#define WIRELESS_RADIO_POLICY_H

#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * The iNIC is enabled unless its disable flag is exactly 1. A checked-read
 * failure must never be coerced to zero, which would silently power it up.
 * Initialize the output to disabled so every error is fail-closed.
 */
static inline int
wireless_inic_start_enabled(int read_status, int disabled_value,
			    int *inic_enabled)
{
	if (!inic_enabled)
		return EINVAL;
	*inic_enabled = 0;
	if (read_status != 0)
		return read_status;
	*inic_enabled = (disabled_value != 1);
	return 0;
}

/*
 * Parse the checked radio-enable NVRAM value. Missing and empty values keep
 * the historical disabled default; backend and malformed-value failures
 * must not become a false "already off" result.
 */
static inline int
wireless_radio_nvram_flag_status(int api_result, int saved_errno,
				 const char *value, int *enabled)
{
	char *end;
	long parsed;

	if (!enabled)
		return EINVAL;
	*enabled = 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !value)
		return EIO;
	if (!value[0])
		return 0;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (end == value) {
		while (*end && isspace((unsigned char)*end))
			++end;
		return *end ? EINVAL : 0;
	}
	if (errno == ERANGE)
		return EINVAL;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (*end)
		return EINVAL;

	*enabled = (parsed != 0);
	return 0;
}

/*
 * Read a numeric NVRAM setting without collapsing backend errors into zero.
 * Missing, empty and whitespace-only values preserve the caller's legacy
 * default. Non-numeric, partially numeric and out-of-range values fail closed.
 */
static inline int
wireless_radio_nvram_int_status(int api_result, int saved_errno,
                                const char *value, int default_value,
                                int *configured_value)
{
	const char *cursor;
	char *end;
	long parsed;

	if (!configured_value)
		return EINVAL;
	*configured_value = default_value;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !value)
		return EIO;

	cursor = value;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (!*cursor)
		return 0;

	errno = 0;
	parsed = strtol(cursor, &end, 10);
	if (end == cursor || errno == ERANGE)
		return EINVAL;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (*end || parsed < INT_MIN || parsed > INT_MAX)
		return EINVAL;

	*configured_value = (int)parsed;
	return 0;
}

/*
 * A radio operation is considered successful only when the driver command or
 * the enclosing restart transaction returns the conventional zero status.
 */
/*
 * The Ralink config generator has historically clamped invalid TxPower
 * settings to 100. Keep the live iNIC radio-toggle path on the same bounded
 * value so a stale/corrupt NVRAM value cannot send an out-of-range iwpriv
 * command or prevent radio recovery.
 */
static inline int
wireless_txpower_config_value(int configured_value)
{
	return (configured_value < 0 || configured_value > 100) ? 100 : configured_value;
}

static inline int
wireless_radio_status_success(int operation_status)
{
	return operation_status == 0;
}

typedef struct wireless_radio_state_ops {
	int (*get_power)(void *context, int *is_on);
	int (*set_power)(void *context, int is_on);
	int (*publish_state)(void *context, int is_on);
} wireless_radio_state_ops_t;

/*
 * Couple a runtime radio power change to publishing its MLME state. Snapshot
 * the prior state before touching hardware. On a power or publication error,
 * restore that exact state (not simply the inverse of the requested state)
 * and preserve the original error; rollback_status reports compensation
 * failure separately.
 */
static inline int
wireless_radio_state_transition(int is_on, const wireless_radio_state_ops_t *ops,
				void *context, int *rollback_status)
{
	int target_on = !!is_on;
	int previous_on = 0;
	int status;

	if (rollback_status)
		*rollback_status = 0;
	if (!ops || !ops->get_power || !ops->set_power || !ops->publish_state)
		return EINVAL;

	status = ops->get_power(context, &previous_on);
	if (status != 0)
		return status;
	previous_on = !!previous_on;

	status = ops->set_power(context, target_on);
	if (status != 0) {
		int cleanup_status = ops->set_power(context, previous_on);

		if (rollback_status)
			*rollback_status = cleanup_status;
		return status;
	}

	status = ops->publish_state(context, target_on);
	if (status != 0) {
		int cleanup_status = ops->set_power(context, previous_on);

		if (rollback_status)
			*rollback_status = cleanup_status;
	}
	return status;
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
/*
 * A partially started AP-client must be disabled and detached from LAN even
 * when the first cleanup step fails. Preserve the original startup failure,
 * but still attempt the independent bridge cleanup.
 */
typedef int (*wireless_apcli_cleanup_step)(void *context);

typedef struct wireless_apcli_cleanup_ops {
	wireless_apcli_cleanup_step bring_interface_down;
	wireless_apcli_cleanup_step detach_bridge;
} wireless_apcli_cleanup_ops_t;

static inline int
wireless_apcli_start_cleanup_run(int primary_status,
				 const wireless_apcli_cleanup_ops_t *ops,
				 void *context, int *interface_status,
				 int *bridge_status)
{
	int status;

	if (!ops || !ops->bring_interface_down || !ops->detach_bridge ||
	    !interface_status || !bridge_status)
		return EINVAL;

	*interface_status = ops->bring_interface_down(context);
	*bridge_status = ops->detach_bridge(context);
	status = wireless_radio_status_merge(primary_status, *interface_status);
	return wireless_radio_status_merge(status, *bridge_status);
}

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
 * A notification may report success only after its state snapshot succeeded
 * and its requested control operation did not fail. The legacy helper takes
 * a boolean change result, so it cannot distinguish "no change" from an
 * unchecked NVRAM read that was collapsed to OFF.
 */
static inline int
wireless_radio_notification_status_result(int state_status,
					  int operation_status,
					  int was_on, int requested_on)
{
	if (state_status != 0)
		return state_status < 0 ? state_status : -state_status;
	if (operation_status < 0)
		return operation_status;
	return wireless_radio_notification_result(
		was_on, requested_on, operation_status);
}

/*
 * RT3352 MII radio state crosses a switch-isolation boundary. Do not publish
 * state until the driver and switch both complete the requested transition.
 */
typedef struct wireless_inic_radio_ops {
	int (*set_power)(void *context, int is_on);
	int (*set_isolation)(void *context, int isolated);
	int (*prepare_on)(void *context);
	int (*publish)(void *context, int is_on);
} wireless_inic_radio_ops_t;

static inline int
wireless_inic_radio_transition(int is_on, const wireless_inic_radio_ops_t *ops,
			       void *context)
{
	int isolation_status;
	int power_status;
	int prepare_status;
	int publish_status;

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
		if (ops->prepare_on) {
			prepare_status = ops->prepare_on(context);
			if (prepare_status != 0) {
				/* Do not expose the radio when its configured power was rejected. */
				(void)ops->set_isolation(context, 1);
				(void)ops->set_power(context, 0);
				return prepare_status;
			}
		}
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

	publish_status = ops->publish(context, !!is_on);
	if (publish_status != 0) {
		if (is_on) {
			/* Do not leave the radio active when state publication failed. */
			(void)ops->set_isolation(context, 1);
			(void)ops->set_power(context, 0);
		}
		return publish_status;
	}
	return 0;
}

/*
 * Preserve the legacy four-interface default unless WDS is explicitly
 * enabled. Once enabled, an unreadable or invalid count must not be
 * interpreted as a valid partial WDS configuration.
 */
static inline int
wireless_radio_wds_config_resolve(int apply_status, int apply_value,
                                  int count_status, int count_value,
                                  int *wds_count)
{
	if (!wds_count)
		return EINVAL;
	*wds_count = 0;
	if (apply_status != 0)
		return apply_status;
	if (apply_value != 1) {
		*wds_count = 4;
		return 0;
	}
	if (count_status != 0)
		return count_status;
	if (count_value < 0 || count_value > 4)
		return EINVAL;
	*wds_count = count_value;
	return 0;
}

/*
 * AP-client startup requires a verified SSID and WISP flag. An unreadable
 * value must not be interpreted as an absent SSID or a non-WISP LAN port.
 */
static inline int
wireless_apcli_start_config_status(int ssid_status, int ssid_present,
				   int wisp_status, int wisp_enabled,
				   int *ssid_configured, int *is_wisp)
{
	if (!ssid_configured || !is_wisp)
		return EINVAL;
	*ssid_configured = 0;
	*is_wisp = 0;
	if (ssid_status != 0)
		return ssid_status;
	if (ssid_present != 0 && ssid_present != 1)
		return EINVAL;
	if (!ssid_present)
		return 0;
	if (wisp_status != 0)
		return wisp_status;
	*ssid_configured = 1;
	*is_wisp = !!wisp_enabled;
	return 0;
}

#endif /* WIRELESS_RADIO_POLICY_H */
