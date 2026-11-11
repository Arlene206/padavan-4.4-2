#ifndef INICD_SIGNAL_POLICY_H
#define INICD_SIGNAL_POLICY_H

#include <signal.h>

#define INICD_EVENT_POLL 0x01
#define INICD_EVENT_STOP 0x02

static inline sig_atomic_t
inicd_event_for_signal(int signum)
{
	if (signum == SIGALRM)
		return INICD_EVENT_POLL;
	if (signum == SIGTERM)
		return INICD_EVENT_STOP;
	return 0;
}

static inline void
inicd_record_signal(volatile sig_atomic_t *pending_events, int signum)
{
	sig_atomic_t event;

	if (!pending_events)
		return;
	event = inicd_event_for_signal(signum);
	if (event)
		*pending_events |= event;
}

static inline sig_atomic_t
inicd_take_pending_events(volatile sig_atomic_t *pending_events)
{
	sig_atomic_t events;

	if (!pending_events)
		return 0;
	events = *pending_events;
	*pending_events = 0;
	return events;
}

static inline int
inicd_event_should_stop(sig_atomic_t events)
{
	return (events & INICD_EVENT_STOP) != 0;
}

#endif
