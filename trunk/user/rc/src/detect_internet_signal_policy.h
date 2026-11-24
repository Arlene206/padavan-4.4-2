#ifndef DETECT_INTERNET_SIGNAL_POLICY_H
#define DETECT_INTERNET_SIGNAL_POLICY_H

#include <signal.h>

#define DI_PENDING_TIMER      ((sig_atomic_t)0x01)
#define DI_PENDING_ACTIVITY   ((sig_atomic_t)0x02)
#define DI_PENDING_TERMINATE  ((sig_atomic_t)0x04)

enum {
	DI_CONTROL_NONE = 0,
	DI_CONTROL_RESUME,
	DI_CONTROL_PAUSE
};

/* Keep signal context limited to sig_atomic_t state updates. */
static inline void
detect_internet_record_signal(volatile sig_atomic_t *pending_events,
			      volatile sig_atomic_t *pending_control,
			      volatile sig_atomic_t *pause_received,
			      int signal_number)
{
	if (!pending_events || !pending_control || !pause_received)
		return;

	switch (signal_number) {
	case SIGALRM:
		*pending_events |= DI_PENDING_TIMER;
		break;
	case SIGHUP:
		*pending_events |= DI_PENDING_ACTIVITY;
		break;
	case SIGUSR1:
		*pending_control = DI_CONTROL_RESUME;
		break;
	case SIGUSR2:
		*pause_received = 1;
		*pending_control = DI_CONTROL_PAUSE;
		break;
	case SIGTERM:
		*pause_received = 1;
		*pending_events |= DI_PENDING_TERMINATE;
		break;
	default:
		break;
	}
}

#endif /* DETECT_INTERNET_SIGNAL_POLICY_H */
