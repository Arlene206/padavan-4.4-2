#ifndef NETWORKMAP_SIGNAL_POLICY_H
#define NETWORKMAP_SIGNAL_POLICY_H

#include <errno.h>
#include <signal.h>

/*
 * Expand to direct volatile sig_atomic_t assignments inside signal handlers.
 * Do not replace this macro with a helper-function call: handlers must avoid
 * calling non-async-signal-safe functions.
 */
#define networkmap_signal_record(refresh_state, exit_state, signal_value) \
	do { \
		if ((signal_value) == SIGUSR1) { \
			*(refresh_state) = 1; \
		} else if ((signal_value) == SIGTERM) { \
			*(exit_state) = 1; \
		} \
	} while (0)

/*
 * Consume a refresh event without losing a SIGUSR1 delivered between the
 * snapshot and clear operations. Preserve the caller's previous signal mask.
 * This function is for normal process context only.
 */
static inline int
networkmap_signal_take_refresh(volatile sig_atomic_t *refresh_requested)
{
	sigset_t block_set, previous_set;
	sig_atomic_t requested;

	if (!refresh_requested) {
		errno = EINVAL;
		return -1;
	}

	if (sigemptyset(&block_set) < 0 || sigaddset(&block_set, SIGUSR1) < 0)
		return -1;

	if (sigprocmask(SIG_BLOCK, &block_set, &previous_set) < 0)
		return -1;

	requested = *refresh_requested;
	*refresh_requested = 0;

	if (sigprocmask(SIG_SETMASK, &previous_set, NULL) < 0) {
		/* Exit safely; don't silently lose the refresh event on failure. */
		*refresh_requested = 1;
		return -1;
	}

	return requested ? 1 : 0;
}

#endif
