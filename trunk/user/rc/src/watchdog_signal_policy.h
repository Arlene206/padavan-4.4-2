#ifndef WATCHDOG_SIGNAL_POLICY_H
#define WATCHDOG_SIGNAL_POLICY_H

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

#endif /* WATCHDOG_SIGNAL_POLICY_H */
