#include "wireless_radio_policy.h"

#include <limits.h>
#include <stdio.h>

static int
test_wds_config_defaults_and_checked_count_validation(void)
{
	int count = -1;

	if (wireless_radio_wds_config_resolve(0, 0, EIO, 99, &count) != 0 || count != 4)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 2, ENODEV, 99, &count) != 0 || count != 4)
		return 1;
	if (wireless_radio_wds_config_resolve(EIO, 1, 0, 3, &count) != EIO || count != 0)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, ENODEV, 3, &count) != ENODEV || count != 0)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, 0, -1, &count) != EINVAL || count != 0)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, 0, 5, &count) != EINVAL || count != 0)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, 0, 0, &count) != 0 || count != 0)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, 0, 4, &count) != 0 || count != 4)
		return 1;
	if (wireless_radio_wds_config_resolve(0, 1, 0, 3, NULL) != EINVAL)
		return 1;
	return 0;
}

static int
test_inic_start_enable_gate_fails_closed(void)
{
	int enabled = 1;

	if (wireless_inic_start_enabled(EACCES, 0, &enabled) != EACCES ||
	    enabled != 0)
		return 1;
	if (wireless_inic_start_enabled(0, 0, &enabled) != 0 || enabled != 1)
		return 1;
	if (wireless_inic_start_enabled(0, 1, &enabled) != 0 || enabled != 0)
		return 1;
	if (wireless_inic_start_enabled(0, 2, &enabled) != 0 || enabled != 1)
		return 1;
	return wireless_inic_start_enabled(0, 0, NULL) != EINVAL;
}

enum wireless_radio_state_test_event {
	WIRELESS_TEST_READ_STATE = 1,
	WIRELESS_TEST_POWER_ON,
	WIRELESS_TEST_POWER_OFF,
	WIRELESS_TEST_PUBLISH_ON,
	WIRELESS_TEST_PUBLISH_OFF
};

typedef struct wireless_radio_state_test_context {
	int events[4];
	int event_count;
	int power_calls;
	int read_status;
	int initial_power;
	int power_status[2];
	int publish_status;
} wireless_radio_state_test_context_t;

static int
wireless_radio_test_get_power(void *opaque, int *is_on)
{
	wireless_radio_state_test_context_t *context =
		(wireless_radio_state_test_context_t *)opaque;

	if (!is_on || context->event_count >= 4)
		return EINVAL;
	context->events[context->event_count++] = WIRELESS_TEST_READ_STATE;
	if (context->read_status != 0)
		return context->read_status;
	*is_on = context->initial_power;
	return 0;
}

static int
wireless_radio_test_set_power(void *opaque, int is_on)
{
	wireless_radio_state_test_context_t *context =
		(wireless_radio_state_test_context_t *)opaque;
	int call = context->power_calls++;

	if (call >= 2 || context->event_count >= 4)
		return EOVERFLOW;
	context->events[context->event_count++] =
		is_on ? WIRELESS_TEST_POWER_ON : WIRELESS_TEST_POWER_OFF;
	return context->power_status[call];
}

static int
wireless_radio_test_publish_state(void *opaque, int is_on)
{
	wireless_radio_state_test_context_t *context =
		(wireless_radio_state_test_context_t *)opaque;

	if (context->event_count >= 4)
		return EOVERFLOW;
	context->events[context->event_count++] =
		is_on ? WIRELESS_TEST_PUBLISH_ON : WIRELESS_TEST_PUBLISH_OFF;
	return context->publish_status;
}

static const wireless_radio_state_ops_t wireless_radio_test_ops = {
	wireless_radio_test_get_power,
	wireless_radio_test_set_power,
	wireless_radio_test_publish_state
};

static int
test_radio_state_transition_reads_state_before_power(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, 0, 0, { 0, 0 }, 0
	};
	int rollback_status = -1;

	if (wireless_radio_state_transition(
		    1, &wireless_radio_test_ops, &context, &rollback_status) != 0)
		return 1;
	return rollback_status != 0 || context.event_count != 3 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.events[1] != WIRELESS_TEST_POWER_ON ||
	       context.events[2] != WIRELESS_TEST_PUBLISH_ON;
}

static int
test_radio_state_transition_restores_previous_power_after_publish_failure(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, 0, 1, { 0, 0 }, EIO
	};
	int rollback_status = -1;

	if (wireless_radio_state_transition(
		    0, &wireless_radio_test_ops, &context, &rollback_status) != EIO)
		return 1;
	return rollback_status != 0 || context.event_count != 4 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.events[1] != WIRELESS_TEST_POWER_OFF ||
	       context.events[2] != WIRELESS_TEST_PUBLISH_OFF ||
	       context.events[3] != WIRELESS_TEST_POWER_ON;
}

static int
test_radio_state_transition_restores_already_on_state_after_publish_failure(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, 0, 1, { 0, 0 }, EACCES
	};
	int rollback_status = -1;

	if (wireless_radio_state_transition(
		    1, &wireless_radio_test_ops, &context, &rollback_status) != EACCES)
		return 1;
	return rollback_status != 0 || context.event_count != 4 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.events[1] != WIRELESS_TEST_POWER_ON ||
	       context.events[2] != WIRELESS_TEST_PUBLISH_ON ||
	       context.events[3] != WIRELESS_TEST_POWER_ON;
}

static int
test_radio_state_transition_preserves_primary_and_rollback_errors(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, 0, 0, { 0, EBUSY }, EACCES
	};
	int rollback_status = 0;

	if (wireless_radio_state_transition(
		    1, &wireless_radio_test_ops, &context, &rollback_status) != EACCES)
		return 1;
	return rollback_status != EBUSY || context.event_count != 4 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.events[1] != WIRELESS_TEST_POWER_ON ||
	       context.events[2] != WIRELESS_TEST_PUBLISH_ON ||
	       context.events[3] != WIRELESS_TEST_POWER_OFF;
}

static int
test_radio_state_transition_restores_previous_power_after_command_failure(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, 0, 0, { ENODEV, 0 }, 0
	};
	int rollback_status = -1;

	if (wireless_radio_state_transition(
		    1, &wireless_radio_test_ops, &context, &rollback_status) != ENODEV)
		return 1;
	return rollback_status != 0 || context.event_count != 3 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.events[1] != WIRELESS_TEST_POWER_ON ||
	       context.events[2] != WIRELESS_TEST_POWER_OFF;
}

static int
test_radio_state_transition_read_failure_prevents_power_change(void)
{
	wireless_radio_state_test_context_t context = {
		{ 0 }, 0, 0, EACCES, 1, { 0, 0 }, 0
	};
	int rollback_status = -1;

	if (wireless_radio_state_transition(
		    0, &wireless_radio_test_ops, &context, &rollback_status) != EACCES)
		return 1;
	return rollback_status != 0 || context.event_count != 1 ||
	       context.events[0] != WIRELESS_TEST_READ_STATE ||
	       context.power_calls != 0;
}

static int
test_radio_state_transition_rejects_incomplete_operations(void)
{
	wireless_radio_state_ops_t incomplete = {
		wireless_radio_test_get_power,
		wireless_radio_test_set_power,
		NULL
	};
	wireless_radio_state_ops_t missing_reader = {
		NULL,
		wireless_radio_test_set_power,
		wireless_radio_test_publish_state
	};
	int rollback_status = 0;

	return wireless_radio_state_transition(
		       1, &incomplete, NULL, &rollback_status) != EINVAL ||
	       wireless_radio_state_transition(
		       1, &missing_reader, NULL, &rollback_status) != EINVAL ||
	       rollback_status != 0;
}

static int
test_success_is_published(void)
{
	return !wireless_radio_status_success(0);
}

static int
test_interface_failures_are_not_published(void)
{
	return wireless_radio_status_success(-1) ||
	       wireless_radio_status_success(1) ||
	       wireless_radio_status_success(256);
}

static int
test_interface_and_hw_nat_failures_preserve_first_error(void)
{
	return wireless_radio_status_merge(0, 0) != 0 ||
	       wireless_radio_status_merge(0, EIO) != EIO ||
	       wireless_radio_status_merge(0, 256) != 256 ||
	       wireless_radio_status_merge(-ENODEV, EIO) != -ENODEV ||
	       wireless_radio_status_merge(7, 0) != 7;
}

static int
test_shutdown_failure_is_sticky(void)
{
	int all_stopped = 1;

	all_stopped = wireless_radio_shutdown_status(all_stopped, 1);
	if (!all_stopped)
		return 1;

	all_stopped = wireless_radio_shutdown_status(all_stopped, 0);
	if (all_stopped)
		return 1;

	/* A later successful stop must not erase an earlier failure. */
	all_stopped = wireless_radio_shutdown_status(all_stopped, 1);
	return all_stopped;
}

static int
test_incomplete_shutdown_is_reported(void)
{
	return wireless_radio_shutdown_result(1) != 0 ||
	       wireless_radio_shutdown_result(0) >= 0;
}

static int
test_startup_failure_is_sticky(void)
{
	int all_started = 1;

	all_started = wireless_radio_startup_status(all_started, 1);
	if (!all_started)
		return 1;

	all_started = wireless_radio_startup_status(all_started, 0);
	if (all_started)
		return 1;

	/* A later successful WDS interface must not erase an earlier failure. */
	all_started = wireless_radio_startup_status(all_started, 1);
	return all_started;
}

static int
test_incomplete_startup_is_reported(void)
{
	return wireless_radio_startup_result(1) != 0 ||
	       wireless_radio_startup_result(0) >= 0;
}

static int
test_restart_failure_wait_status_is_normalized_and_sticky(void)
{
	return wireless_radio_restart_status(0, 0) != 0 ||
	       wireless_radio_restart_status(0, -1) >= 0 ||
	       wireless_radio_restart_status(0, 1) >= 0 ||
	       wireless_radio_restart_status(0, 256) >= 0 ||
	       wireless_radio_restart_status(-EACCES, 0) != -EACCES ||
	       wireless_radio_restart_status(-EACCES, 256) != -EACCES;
}

static int
test_apcli_autoconnect_failure_is_reported(void)
{
	int status = 0;

	status = wireless_radio_restart_status(status, 0);
	status = wireless_radio_restart_status(status, 256);
	if (status != -EIO)
		return 1;

	/* A later successful command must not erase the first failure. */
	status = wireless_radio_restart_status(status, 0);
	return status != -EIO;
}

static int
test_apcli_forced_reenable_failure_is_reported(void)
{
	int status = 0;

	status = wireless_radio_restart_status(status, 0);
	status = wireless_radio_restart_status(status, -1);
	return status != -EIO;
}

static int
test_apcli_wan_recovery_failure_is_reported(void)
{
	/* Both full WAN restart and AP-client reconnect failures are nonzero. */
	if (wireless_radio_restart_status(0, EIO) != -EIO ||
	    wireless_radio_restart_status(0, -ENETDOWN) != -EIO)
		return 1;

	/* A WAN failure must not replace an earlier guest-isolation failure. */
	return wireless_radio_restart_status(-EACCES, EIO) != -EACCES;
}

static int
test_inic_monitor_launch_failure_is_sticky(void)
{
	int all_started = 1;

	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(-EIO));
	if (all_started)
		return 1;

	/* A later successful interface must not erase the monitor failure. */
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(0));
	return all_started;
}

static int
test_inic_force_disable_interface_failure_is_sticky(void)
{
	int all_started = 1;

	/* A failed rai0 ifconfig-down must remain visible after safe teardown succeeds. */
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(-EIO));
	if (all_started)
		return 1;

	all_started = wireless_radio_startup_status(all_started, 1);
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(0));
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(0));
	return all_started;
}

static int
test_radio_notification_noop_is_success_only_when_already_at_target(void)
{
	return wireless_radio_notification_result(0, 0, 0) != 0 ||
	       wireless_radio_notification_result(1, 1, 0) != 0;
}

static int
test_radio_notification_successful_transitions(void)
{
	return wireless_radio_notification_result(0, 1, 1) != 0 ||
	       wireless_radio_notification_result(1, 0, 1) != 0;
}

static int
test_radio_notification_failed_transitions_remain_pending(void)
{
	return wireless_radio_notification_result(0, 1, 0) != -EIO ||
	       wireless_radio_notification_result(1, 0, 0) != -EIO ||
	       wireless_radio_notification_result(0, 1, 2) != -EIO;
}

enum inic_radio_test_event {
	INIC_EVENT_RADIO_ON = 1, INIC_EVENT_RADIO_OFF, INIC_EVENT_PREPARE_ON,
	INIC_EVENT_UNISOLATE, INIC_EVENT_ISOLATE, INIC_EVENT_PUBLISH_ON,
	INIC_EVENT_PUBLISH_OFF
};
typedef struct inic_radio_test_context {
	int radio_on_status, radio_off_status, isolate_status, unisolate_status;
	int prepare_status, publish_status;
	int event_count, events[8], publish_count, published_state;
} inic_radio_test_context_t;
static void inic_radio_record(inic_radio_test_context_t *c, int e)
{
	if (c->event_count < (int)(sizeof(c->events) / sizeof(c->events[0])))
		c->events[c->event_count++] = e;
}
static int inic_radio_test_set_power(void *p, int on)
{
	inic_radio_test_context_t *c = (inic_radio_test_context_t *)p;
	inic_radio_record(c, on ? INIC_EVENT_RADIO_ON : INIC_EVENT_RADIO_OFF);
	return on ? c->radio_on_status : c->radio_off_status;
}
static int inic_radio_test_set_isolation(void *p, int isolated)
{
	inic_radio_test_context_t *c = (inic_radio_test_context_t *)p;
	inic_radio_record(c, isolated ? INIC_EVENT_ISOLATE : INIC_EVENT_UNISOLATE);
	return isolated ? c->isolate_status : c->unisolate_status;
}
static int inic_radio_test_prepare_on(void *p)
{
	inic_radio_test_context_t *c = (inic_radio_test_context_t *)p;
	inic_radio_record(c, INIC_EVENT_PREPARE_ON);
	return c->prepare_status;
}
static int inic_radio_test_publish(void *p, int on)
{
	inic_radio_test_context_t *c = (inic_radio_test_context_t *)p;
	inic_radio_record(c, on ? INIC_EVENT_PUBLISH_ON : INIC_EVENT_PUBLISH_OFF);
	c->publish_count++;
	c->published_state = !!on;
	return c->publish_status;
}
static const wireless_inic_radio_ops_t inic_radio_test_ops = {
	inic_radio_test_set_power, inic_radio_test_set_isolation,
	inic_radio_test_prepare_on, inic_radio_test_publish
};
static int inic_radio_events_equal(const inic_radio_test_context_t *c,
				   const int *expected, int count)
{
	int i;
	if (c->event_count != count) return 0;
	for (i = 0; i < count; i++) if (c->events[i] != expected[i]) return 0;
	return 1;
}
static int test_inic_radio_enable_orders_power_before_unisolation_and_publication(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_ON,
		INIC_EVENT_PREPARE_ON, INIC_EVENT_UNISOLATE, INIC_EVENT_PUBLISH_ON };
	if (wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != 0) return 1;
	return !inic_radio_events_equal(&c, e, 5) || c.publish_count != 1 || c.published_state != 1;
}
static int test_inic_radio_disable_isolates_before_power_off_and_publication(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF, INIC_EVENT_PUBLISH_OFF };
	if (wireless_inic_radio_transition(0, &inic_radio_test_ops, &c) != 0) return 1;
	return !inic_radio_events_equal(&c, e, 3) || c.publish_count != 1 || c.published_state != 0;
}
static int test_inic_radio_power_on_failure_rolls_back_without_publishing(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_ON, INIC_EVENT_RADIO_OFF };
	c.radio_on_status = EIO;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EIO ||
	       !inic_radio_events_equal(&c, e, 3) || c.publish_count != 0;
}
static int test_inic_radio_unisolation_failure_rolls_back_and_preserves_cause(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_ON,
		INIC_EVENT_PREPARE_ON, INIC_EVENT_UNISOLATE,
		INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.unisolate_status = EACCES;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EACCES ||
	       !inic_radio_events_equal(&c, e, 6) || c.publish_count != 0;
}
static int test_inic_radio_isolation_failure_still_powers_off_without_publishing(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.isolate_status = EACCES;
	return wireless_inic_radio_transition(0, &inic_radio_test_ops, &c) != EACCES ||
	       !inic_radio_events_equal(&c, e, 2) || c.publish_count != 0;
}
static int test_inic_radio_power_off_failure_does_not_publish(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.radio_off_status = EBUSY;
	return wireless_inic_radio_transition(0, &inic_radio_test_ops, &c) != EBUSY ||
	       !inic_radio_events_equal(&c, e, 2) || c.publish_count != 0;
}
static int test_inic_radio_initial_isolation_failure_prevents_power_on(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.isolate_status = EACCES;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EACCES ||
	       !inic_radio_events_equal(&c, e, 2) || c.publish_count != 0;
}

static int test_inic_radio_txpower_failure_rolls_back_without_unisolation(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_ON,
		INIC_EVENT_PREPARE_ON, INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.prepare_status = EIO;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EIO ||
	       !inic_radio_events_equal(&c, e, 5) || c.publish_count != 0;
}

static int test_inic_radio_enable_publication_failure_rolls_back(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_ON,
		INIC_EVENT_PREPARE_ON, INIC_EVENT_UNISOLATE,
		INIC_EVENT_PUBLISH_ON, INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.publish_status = ENOSPC;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != ENOSPC ||
	       !inic_radio_events_equal(&c, e, 7) || c.publish_count != 1;
}

static int test_inic_radio_disable_publication_failure_is_reported(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF,
		INIC_EVENT_PUBLISH_OFF };
	c.publish_status = EIO;
	return wireless_inic_radio_transition(0, &inic_radio_test_ops, &c) != EIO ||
	       !inic_radio_events_equal(&c, e, 3) || c.publish_count != 1;
}

static int test_inic_radio_policy_rejects_incomplete_operations(void)
{
	return wireless_inic_radio_transition(1, NULL, NULL) != -EINVAL;
}

static int
test_txpower_nvram_read_status_fails_closed(void)
{
	int configured = -1;

	/* A missing/empty value retains the legacy TxPower default of zero. */
	if (wireless_radio_nvram_int_status(0, 0, NULL, 0, &configured) != 0 ||
	    configured != 0)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "", 17, &configured) != 0 ||
	    configured != 17)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "  \t ", 17, &configured) != 0 ||
	    configured != 17)
		return 1;

	if (wireless_radio_nvram_int_status(1, 0, " 73 \t", 0, &configured) != 0 ||
	    configured != 73)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "-4", 0, &configured) != 0 ||
	    configured != -4)
		return 1;

	/* Read errors, invalid API statuses and malformed values never become 0. */
	if (wireless_radio_nvram_int_status(-1, EACCES, "73", 0,
					    &configured) != EACCES || configured != 0)
		return 1;
	if (wireless_radio_nvram_int_status(-1, 0, "73", 0,
					    &configured) != EIO || configured != 0)
		return 1;
	if (wireless_radio_nvram_int_status(2, 0, "73", 0,
					    &configured) != EIO || configured != 0)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, NULL, 0,
					    &configured) != EIO || configured != 0)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "73dBm", 17,
					    &configured) != EINVAL || configured != 17)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "2147483648", 17,
					    &configured) != EINVAL || configured != 17)
		return 1;
	if (wireless_radio_nvram_int_status(1, 0, "-2147483649", 17,
					    &configured) != EINVAL || configured != 17)
		return 1;
	return wireless_radio_nvram_int_status(1, 0, "73", 0, NULL) != EINVAL;
}

static int
test_txpower_config_value_is_normalized(void)
{
	return wireless_txpower_config_value(INT_MIN) != 100 ||
	       wireless_txpower_config_value(-1) != 100 ||
	       wireless_txpower_config_value(0) != 0 ||
	       wireless_txpower_config_value(1) != 1 ||
	       wireless_txpower_config_value(100) != 100 ||
	       wireless_txpower_config_value(101) != 100 ||
	       wireless_txpower_config_value(INT_MAX) != 100;
}

static int
test_radio_nvram_flag_status_fails_closed(void)
{
	int enabled = 9;

	if (wireless_radio_nvram_flag_status(0, 0, NULL, &enabled) != 0 ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "", &enabled) != 0 ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "   ", &enabled) != 0 ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "0", &enabled) != 0 ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "1", &enabled) != 0 ||
	    enabled != 1)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, " 2 ", &enabled) != 0 ||
	    enabled != 1)
		return 1;
	if (wireless_radio_nvram_flag_status(-1, EACCES, "1", &enabled) != EACCES ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(-1, 0, "1", &enabled) != EIO ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(2, 0, "1", &enabled) != EIO ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "on", &enabled) != EINVAL ||
	    enabled != 0)
		return 1;
	if (wireless_radio_nvram_flag_status(1, 0, "1x", &enabled) != EINVAL ||
	    enabled != 0)
		return 1;
	return wireless_radio_nvram_flag_status(1, 0, "1", NULL) != EINVAL;
}

int
main(void)
{
	if (test_wds_config_defaults_and_checked_count_validation() ||
	    test_radio_state_transition_reads_state_before_power() ||
	    test_radio_state_transition_restores_previous_power_after_publish_failure() ||
	    test_radio_state_transition_restores_already_on_state_after_publish_failure() ||
	    test_radio_state_transition_preserves_primary_and_rollback_errors() ||
	    test_radio_state_transition_restores_previous_power_after_command_failure() ||
	    test_radio_state_transition_read_failure_prevents_power_change() ||
	    test_radio_state_transition_rejects_incomplete_operations() ||
	    test_radio_nvram_flag_status_fails_closed() ||
	    test_txpower_nvram_read_status_fails_closed() ||
	    test_inic_start_enable_gate_fails_closed() ||
	    test_txpower_config_value_is_normalized() ||
	    test_success_is_published() ||
	    test_interface_failures_are_not_published() ||
	    test_interface_and_hw_nat_failures_preserve_first_error() ||
	    test_shutdown_failure_is_sticky() ||
	    test_incomplete_shutdown_is_reported() ||
	    test_startup_failure_is_sticky() ||
	    test_incomplete_startup_is_reported() ||
	    test_restart_failure_wait_status_is_normalized_and_sticky() ||
	    test_apcli_autoconnect_failure_is_reported() ||
	    test_apcli_forced_reenable_failure_is_reported() ||
	    test_apcli_wan_recovery_failure_is_reported() ||
	    test_inic_monitor_launch_failure_is_sticky() ||
	    test_inic_force_disable_interface_failure_is_sticky() ||
	    test_radio_notification_noop_is_success_only_when_already_at_target() ||
	    test_radio_notification_successful_transitions() ||
	    test_radio_notification_failed_transitions_remain_pending() ||
	    test_inic_radio_enable_orders_power_before_unisolation_and_publication() ||
	    test_inic_radio_disable_isolates_before_power_off_and_publication() ||
	    test_inic_radio_power_on_failure_rolls_back_without_publishing() ||
	    test_inic_radio_unisolation_failure_rolls_back_and_preserves_cause() ||
	    test_inic_radio_isolation_failure_still_powers_off_without_publishing() ||
	    test_inic_radio_power_off_failure_does_not_publish() ||
	    test_inic_radio_initial_isolation_failure_prevents_power_on() ||
	    test_inic_radio_txpower_failure_rolls_back_without_unisolation() ||
	    test_inic_radio_enable_publication_failure_rolls_back() ||
	    test_inic_radio_disable_publication_failure_is_reported() ||
	    test_inic_radio_policy_rejects_incomplete_operations()) {
		fprintf(stderr, "Wireless interface status policy: FAIL\n");
		return 1;
	}

	printf("Wireless radio and interface lifecycle policies: PASS\n");
	return 0;
}
