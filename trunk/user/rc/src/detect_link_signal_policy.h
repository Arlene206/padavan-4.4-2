#ifndef DETECT_LINK_SIGNAL_POLICY_H
#define DETECT_LINK_SIGNAL_POLICY_H

#include <signal.h>

#define DL_PENDING_TIMER        ((sig_atomic_t)0x01)
#define DL_PENDING_LED_REFRESH  ((sig_atomic_t)0x02)
#define DL_PENDING_RESET        ((sig_atomic_t)0x04)
#define DL_PENDING_TERMINATE    ((sig_atomic_t)0x08)

/* Only record signal identity here; hardware, NVRAM and service work is deferred. */
static inline void
detect_link_record_signal(volatile sig_atomic_t *pending_events,
                          int signal_number)
{
	if (!pending_events)
		return;

	switch (signal_number) {
	case SIGALRM:
		*pending_events |= DL_PENDING_TIMER;
		break;
	case SIGHUP:
		*pending_events |= DL_PENDING_LED_REFRESH;
		break;
	case SIGUSR1:
		*pending_events |= DL_PENDING_RESET;
		break;
	case SIGTERM:
		*pending_events |= DL_PENDING_TERMINATE;
		break;
	default:
		break;
	}
}

/* The caller blocks all managed signals before taking and clearing the snapshot. */
static inline sig_atomic_t
detect_link_take_pending_events(volatile sig_atomic_t *pending_events)
{
	sig_atomic_t events;

	if (!pending_events)
		return 0;

	events = *pending_events;
	*pending_events = 0;
	return events;
}

#endif /* DETECT_LINK_SIGNAL_POLICY_H */
