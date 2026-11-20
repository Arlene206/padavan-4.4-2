#include "wireless_radio_policy.h"

#include <stdio.h>

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
static void inic_radio_test_prepare_on(void *p)
{
	inic_radio_record((inic_radio_test_context_t *)p, INIC_EVENT_PREPARE_ON);
}
static void inic_radio_test_publish(void *p, int on)
{
	inic_radio_test_context_t *c = (inic_radio_test_context_t *)p;
	inic_radio_record(c, on ? INIC_EVENT_PUBLISH_ON : INIC_EVENT_PUBLISH_OFF);
	c->publish_count++;
	c->published_state = !!on;
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
	const int e[] = { INIC_EVENT_RADIO_ON, INIC_EVENT_PREPARE_ON,
		INIC_EVENT_UNISOLATE, INIC_EVENT_PUBLISH_ON };
	if (wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != 0) return 1;
	return !inic_radio_events_equal(&c, e, 4) || c.publish_count != 1 || c.published_state != 1;
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
	const int e[] = { INIC_EVENT_RADIO_ON, INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.radio_on_status = EIO;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EIO ||
	       !inic_radio_events_equal(&c, e, 3) || c.publish_count != 0;
}
static int test_inic_radio_unisolation_failure_rolls_back_and_preserves_cause(void)
{
	inic_radio_test_context_t c = { 0 };
	const int e[] = { INIC_EVENT_RADIO_ON, INIC_EVENT_PREPARE_ON,
		INIC_EVENT_UNISOLATE, INIC_EVENT_ISOLATE, INIC_EVENT_RADIO_OFF };
	c.unisolate_status = EACCES;
	return wireless_inic_radio_transition(1, &inic_radio_test_ops, &c) != EACCES ||
	       !inic_radio_events_equal(&c, e, 5) || c.publish_count != 0;
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
static int test_inic_radio_policy_rejects_incomplete_operations(void)
{
	return wireless_inic_radio_transition(1, NULL, NULL) != -EINVAL;
}

int
main(void)
{
	if (test_success_is_published() ||
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
	    test_inic_radio_policy_rejects_incomplete_operations()) {
		fprintf(stderr, "Wireless interface status policy: FAIL\n");
		return 1;
	}

	printf("Wireless radio and interface lifecycle policies: PASS\n");
	return 0;
}
