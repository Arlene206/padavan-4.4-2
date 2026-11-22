#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "pid1_signal_policy.h"

static int fake_mask_how;
static int fake_mask_result;
static int fake_mask_errno;
static int fake_mask_saw_sigusr1;
static int fake_mask_old_written;

static int
fake_sigprocmask(int how, const sigset_t *set, sigset_t *old_set)
{
	fake_mask_how = how;
	fake_mask_saw_sigusr1 = sigismember(set, SIGUSR1);
	fake_mask_old_written = 0;
	if (old_set) {
		sigemptyset(old_set);
		sigaddset(old_set, SIGCHLD);
		fake_mask_old_written = 1;
	}
	if (fake_mask_result < 0) {
		errno = fake_mask_errno;
		return -1;
	}
	return 0;
}

static int
test_pid1_signal_mask_update_reports_failures(void)
{
	sigset_t set;
	sigset_t old_set;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	if (pid1_signal_update_mask(NULL, SIG_BLOCK, &set, NULL) != EINVAL ||
	    pid1_signal_update_mask(fake_sigprocmask, SIG_BLOCK, NULL, NULL) != EINVAL)
		return 1;

	fake_mask_result = 0;
	fake_mask_errno = EBUSY;
	fake_mask_how = -1;
	fake_mask_saw_sigusr1 = 0;
	fake_mask_old_written = 0;
	errno = EINTR;
	if (pid1_signal_update_mask(fake_sigprocmask, SIG_BLOCK, &set, &old_set) != 0 ||
	    fake_mask_how != SIG_BLOCK || fake_mask_saw_sigusr1 != 1 ||
	    !fake_mask_old_written || sigismember(&old_set, SIGCHLD) != 1 ||
	    errno != 0)
		return 1;

	fake_mask_result = -1;
	fake_mask_errno = EBUSY;
	if (pid1_signal_update_mask(fake_sigprocmask, SIG_SETMASK, &set, NULL) != EBUSY)
		return 1;

	fake_mask_errno = 0;
	if (pid1_signal_update_mask(fake_sigprocmask, SIG_UNBLOCK, &set, NULL) != EIO)
		return 1;

	return pid1_signal_status_merge(0, EBUSY) != EBUSY ||
	       pid1_signal_status_merge(EBUSY, 0) != EBUSY ||
	       pid1_signal_status_merge(EBUSY, EIO) != EBUSY;
}

static int fake_signal_number;
static int fake_signal_result;
static int fake_signal_errno;
static void (*fake_signal_handler)(int);

static int
fake_sigaction(int signal_number, const struct sigaction *action,
	       struct sigaction *old_action)
{
	(void)old_action;
	fake_signal_number = signal_number;
	fake_signal_handler = action ? action->sa_handler : NULL;
	if (fake_signal_result < 0) {
		errno = fake_signal_errno;
		return -1;
	}
	return 0;
}

static void
test_signal_handler(int signal_number)
{
	(void)signal_number;
}

static int
test_pid1_signal_installation_reports_failures(void)
{
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_handler = test_signal_handler;
	if (pid1_signal_install_action(NULL, SIGTERM, &action) != EINVAL ||
	    pid1_signal_install_action(fake_sigaction, SIGTERM, NULL) != EINVAL)
		return 1;

	fake_signal_result = 0;
	/* A stale caller errno must not be reported after successful install. */
	fake_signal_errno = EBUSY;
	errno = EINTR;
	fake_signal_number = 0;
	fake_signal_handler = NULL;
	if (pid1_signal_install_action(fake_sigaction, SIGTERM, &action) != 0 ||
	    fake_signal_number != SIGTERM ||
	    fake_signal_handler != test_signal_handler ||
	    errno != 0)
		return 1;

	fake_signal_result = -1;
	fake_signal_errno = EACCES;
	if (pid1_signal_install_action(fake_sigaction, SIGCHLD, &action) != EACCES)
		return 1;

	/* A failed libc call without errno still fails closed as EIO. */
	fake_signal_errno = 0;
	if (pid1_signal_install_action(fake_sigaction, SIGALRM, &action) != EIO)
		return 1;

	return pid1_signal_status_merge(0, EACCES) != EACCES ||
	       pid1_signal_status_merge(EACCES, 0) != EACCES ||
	       pid1_signal_status_merge(EACCES, EIO) != EACCES;
}


int
main(void)
{
	assert(test_pid1_signal_installation_reports_failures() == 0);
	assert(test_pid1_signal_mask_update_reports_failures() == 0);
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
