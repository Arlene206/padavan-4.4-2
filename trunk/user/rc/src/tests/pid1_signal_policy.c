#include <assert.h>
#include <signal.h>

#include "pid1_signal_policy.h"

int
main(void)
{
	pid1_signal_state_t state = { 0, 0, 0, 0, 0 };

	pid1_signal_record_event(&state, SIGUSR1);
	assert(state.usr1_pending == 1);
	assert(state.alarm_pending == 0);

	pid1_signal_record_event(&state, SIGALRM);
	assert(state.usr1_pending == 1);
	assert(state.alarm_pending == 1);

	pid1_signal_record_shutdown(&state, SIGTERM);
	assert(state.shutdown_pending == 1);
	assert(state.shutdown_signal == SIGTERM);
	assert(state.usr1_pending == 0);
	assert(state.alarm_pending == 0);

	/* Repeated shutdown requests preserve the first selected action. */
	pid1_signal_record_shutdown(&state, SIGQUIT);
	assert(state.shutdown_signal == SIGTERM);

	/* Events arriving after shutdown cannot restart deferred work. */
	pid1_signal_record_event(&state, SIGUSR1);
	pid1_signal_record_event(&state, SIGALRM);
	assert(state.usr1_pending == 0);
	assert(state.alarm_pending == 0);

	/* Reaping another daemon must not clear the tracked console shell. */
	pid1_signal_record_console_shell(&state, (sig_atomic_t)1234);
	assert(state.console_shell_pid == (sig_atomic_t)1234);
	assert(pid1_signal_record_child_reaped(&state, (sig_atomic_t)5678) == 0);
	assert(state.console_shell_pid == (sig_atomic_t)1234);

	/* Reaping the exact child clears the old PID and permits a relaunch. */
	assert(pid1_signal_record_child_reaped(&state, (sig_atomic_t)1234) == 1);
	assert(state.console_shell_pid == 0);

	/* A later recovery shell can be tracked without stale state. */
	pid1_signal_record_console_shell(&state, (sig_atomic_t)2345);
	assert(state.console_shell_pid == (sig_atomic_t)2345);
	assert(pid1_signal_record_child_reaped(&state, (sig_atomic_t)2345) == 1);
	assert(state.console_shell_pid == 0);

	return 0;
}
