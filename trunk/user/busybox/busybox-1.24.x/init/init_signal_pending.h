#ifndef PADAVAN_BUSYBOX_INIT_SIGNAL_PENDING_H
#define PADAVAN_BUSYBOX_INIT_SIGNAL_PENDING_H

#include <signal.h>

#define INIT_SIGNAL_PENDING_HUP      ((sig_atomic_t)0x01)
#define INIT_SIGNAL_PENDING_INT      ((sig_atomic_t)0x02)
#define INIT_SIGNAL_PENDING_QUIT     ((sig_atomic_t)0x04)
#define INIT_SIGNAL_PENDING_SHUTDOWN ((sig_atomic_t)0x08)

/*
 * PID 1's delayed handlers publish independent events here. Their sigaction
 * mask blocks every signal while recording, so the bitwise OR and associated
 * shutdown signal cannot be interrupted by a second writer. The main loop
 * blocks the delayed signal set while taking and clearing a batch.
 */
struct init_signal_pending {
	volatile sig_atomic_t events;
	volatile sig_atomic_t shutdown_signal;
};

static inline sig_atomic_t init_signal_pending_flag(int signo)
{
	switch (signo) {
	case SIGHUP:
		return INIT_SIGNAL_PENDING_HUP;
	case SIGINT:
		return INIT_SIGNAL_PENDING_INT;
	case SIGQUIT:
		return INIT_SIGNAL_PENDING_QUIT;
	case SIGTERM:
	case SIGUSR1:
	case SIGUSR2:
#ifdef SIGPWR
	case SIGPWR:
#endif
		return INIT_SIGNAL_PENDING_SHUTDOWN;
	default:
		return 0;
	}
}

/* Return 1 when recorded, 0 for an unsupported signal, and -1 on bad input. */
static inline int init_signal_pending_record(struct init_signal_pending *pending,
					     int signo)
{
	sig_atomic_t flag;

	if (!pending)
		return -1;
	flag = init_signal_pending_flag(signo);
	if (!flag)
		return 0;
	if (flag == INIT_SIGNAL_PENDING_SHUTDOWN)
		pending->shutdown_signal = (sig_atomic_t)signo;
	pending->events |= flag;
	return 1;
}

/* Caller blocks all delayed signal inputs before taking and clearing state. */
static inline sig_atomic_t
init_signal_pending_take(struct init_signal_pending *pending,
			 int *shutdown_signal)
{
	sig_atomic_t events;

	if (!pending)
		return (sig_atomic_t)-1;
	events = pending->events;
	if (shutdown_signal)
		*shutdown_signal = (int)pending->shutdown_signal;
	pending->events = 0;
	pending->shutdown_signal = 0;
	return events;
}

/* Requeue a taken batch if the caller cannot restore its previous signal mask. */
static inline int
init_signal_pending_restore(struct init_signal_pending *pending,
			    sig_atomic_t events, int shutdown_signal)
{
	const sig_atomic_t valid_events = INIT_SIGNAL_PENDING_HUP
		| INIT_SIGNAL_PENDING_INT
		| INIT_SIGNAL_PENDING_QUIT
		| INIT_SIGNAL_PENDING_SHUTDOWN;

	if (!pending || events < 0 || (events & ~valid_events))
		return -1;
	if ((events & INIT_SIGNAL_PENDING_SHUTDOWN)
	 && !(pending->events & INIT_SIGNAL_PENDING_SHUTDOWN))
		pending->shutdown_signal = (sig_atomic_t)shutdown_signal;
	pending->events |= events;
	return 0;
}

#endif /* PADAVAN_BUSYBOX_INIT_SIGNAL_PENDING_H */
