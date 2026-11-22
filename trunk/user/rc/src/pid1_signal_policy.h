#ifndef PID1_SIGNAL_POLICY_H
#define PID1_SIGNAL_POLICY_H

#include <errno.h>
#include <signal.h>

typedef int (*pid1_signal_set_empty_fn)(sigset_t *);
typedef int (*pid1_signal_set_add_fn)(sigset_t *, int);

static inline int
pid1_signal_empty_set(pid1_signal_set_empty_fn empty_set, sigset_t *set)
{
	if (!empty_set || !set)
		return EINVAL;

	errno = 0;
	if (empty_set(set) < 0)
		return errno > 0 ? errno : EIO;
	return 0;
}

static inline int
pid1_signal_add_set(pid1_signal_set_add_fn add_set, sigset_t *set, int sig)
{
	if (!add_set || !set)
		return EINVAL;

	errno = 0;
	if (add_set(set, sig) < 0)
		return errno > 0 ? errno : EIO;
	return 0;
}

typedef int (*pid1_signal_mask_fn)(int, const sigset_t *, sigset_t *);

static inline int
pid1_signal_update_mask(pid1_signal_mask_fn update, int how,
			const sigset_t *set, sigset_t *old_set)
{
	if (!update || !set)
		return EINVAL;

	errno = 0;
	if (update(how, set, old_set) < 0)
		return errno > 0 ? errno : EIO;
	return 0;
}

typedef int (*pid1_signal_action_fn)(int, const struct sigaction *,
					 struct sigaction *);

static inline int
pid1_signal_install_action(pid1_signal_action_fn install, int signal_number,
			   const struct sigaction *action)
{
	if (!install || !action)
		return EINVAL;

	errno = 0;
	if (install(signal_number, action, NULL) < 0)
		return errno > 0 ? errno : EIO;
	return 0;
}

static inline int
pid1_signal_status_merge(int first_status, int next_status)
{
	return first_status != 0 ? first_status : next_status;
}

typedef struct {
	volatile sig_atomic_t shutdown_pending;
	volatile sig_atomic_t shutdown_signal;
	volatile sig_atomic_t usr1_pending;
	volatile sig_atomic_t alarm_pending;
	volatile sig_atomic_t console_shell_pid;
} pid1_signal_state_t;

/* First shutdown request wins; discard lower-priority queued events. */
static inline void
pid1_signal_record_shutdown(pid1_signal_state_t *state, int sig)
{
	if (!state->shutdown_pending) {
		state->shutdown_signal = (sig_atomic_t)sig;
		state->shutdown_pending = 1;
	}
	state->usr1_pending = 0;
	state->alarm_pending = 0;
}

/*
 * The console is tracked by its direct-child PID. SIGCHLD stays blocked from
 * fork until publication; the reaper clears only the exact collected PID.
 */
static inline void
pid1_signal_record_console_shell(pid1_signal_state_t *state, sig_atomic_t pid)
{
	state->console_shell_pid = pid > 0 ? pid : 0;
}

static inline int
pid1_signal_record_child_reaped(pid1_signal_state_t *state, sig_atomic_t pid)
{
	if (pid <= 0 || state->console_shell_pid != pid)
		return 0;

	state->console_shell_pid = 0;
	return 1;
}

/* This routine is signal-context safe: it only reads/writes sig_atomic_t. */
static inline void
pid1_signal_record_event(pid1_signal_state_t *state, int sig)
{
	if (state->shutdown_pending)
		return;

	if (sig == SIGUSR1)
		state->usr1_pending = 1;
	else if (sig == SIGALRM)
		state->alarm_pending = 1;
}

#endif
