#ifndef WATCHDOG_SIGNAL_POLICY_H
#define WATCHDOG_SIGNAL_POLICY_H

#include <errno.h>
#include <signal.h>

#define WD_PENDING_TIMER         ((sig_atomic_t)0x01)
#define WD_PENDING_TIME_REFRESH  ((sig_atomic_t)0x02)
#define WD_PENDING_WIFI_STATE    ((sig_atomic_t)0x04)
#define WD_PENDING_TERMINATE     ((sig_atomic_t)0x08)

/* Signal context records intent only; callers dispatch work in normal context. */
static inline void
watchdog_record_signal(volatile sig_atomic_t *pending_events,
                       int signal_number)
{
	if (!pending_events)
		return;

	switch (signal_number) {
	case SIGALRM:
		*pending_events |= WD_PENDING_TIMER;
		break;
	case SIGHUP:
		*pending_events |= WD_PENDING_TIME_REFRESH;
		break;
	case SIGUSR1:
		*pending_events |= WD_PENDING_WIFI_STATE;
		break;
	case SIGTERM:
		*pending_events |= WD_PENDING_TERMINATE;
		break;
	default:
		break;
	}
}

/* The caller blocks all managed signals before taking and clearing the snapshot. */
static inline sig_atomic_t
watchdog_take_pending_events(volatile sig_atomic_t *pending_events)
{
	sig_atomic_t events;

	if (!pending_events)
		return 0;

	events = *pending_events;
	*pending_events = 0;
	return events;
}

/*
 * A termination signal may arrive after a batch snapshot was taken while
 * signals are unblocked for dispatch. Do not start later work from that batch
 * once shutdown has been queued.
 */
static inline int
watchdog_should_dispatch_pending_event(
	const volatile sig_atomic_t *pending_events,
	sig_atomic_t batch_events, sig_atomic_t event)
{
	if (!pending_events || !event)
		return 0;
	return ((*pending_events & WD_PENDING_TERMINATE) == 0 &&
		(batch_events & event) != 0);
}

/*
 * Watchdog schedule state represents a notification accepted by PID 1, not
 * merely a target observed by the scheduler. Preserve the prior state on
 * queue errors so the caller retries the target on its next timer tick.
 */
static inline int
watchdog_notification_state_commit(int previous_state, int requested_state,
                                    int queue_status)
{
	return queue_status == 0 ? requested_state : previous_state;
}

/*
 * SIGUSR1 selects its target through wd_notify_id. Never send it when that
 * band selector failed to persist, or the watchdog may refresh the other band.
 */
static inline int
watchdog_wifi_notification_should_signal(int state_write_status)
{
	return state_write_status == 0;
}

/*
 * Resolve the band selector only from a successful, recognized NVRAM value.
 * A failed read or stale/uninitialized identifier must never dispatch a
 * refresh against an arbitrary radio.
 */
static inline int
watchdog_wifi_notification_band_status(int read_status, int notify_id,
                                        int wifi2_id, int wifi5_id,
                                        int *is_5ghz)
{
	if (!is_5ghz)
		return EINVAL;
	*is_5ghz = -1;
	if (read_status != 0)
		return read_status;
	if (notify_id == wifi2_id) {
		*is_5ghz = 0;
		return 0;
	}
	if (notify_id == wifi5_id) {
		*is_5ghz = 1;
		return 0;
	}
	return EINVAL;
}

#endif /* WATCHDOG_SIGNAL_POLICY_H */
