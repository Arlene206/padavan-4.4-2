#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "pid1_signal_policy.h"

static int fake_empty_result;
static int fake_empty_errno;
static int fake_empty_calls;
static int fake_add_result;
static int fake_add_errno;
static int fake_add_calls;
static int fake_add_signal;

static int
fake_signal_empty(sigset_t *set)
{
	fake_empty_calls++;
	if (fake_empty_result < 0) {
		errno = fake_empty_errno;
		return -1;
	}
	return sigemptyset(set);
}

static int
fake_signal_add(sigset_t *set, int signal_number)
{
	fake_add_calls++;
	fake_add_signal = signal_number;
	if (fake_add_result < 0) {
		errno = fake_add_errno;
		return -1;
	}
	return sigaddset(set, signal_number);
}

static int
test_pid1_signal_set_initialization_reports_failures(void)
{
	sigset_t set;

	if (pid1_signal_empty_set(NULL, &set) != EINVAL ||
	    pid1_signal_empty_set(fake_signal_empty, NULL) != EINVAL ||
	    pid1_signal_add_set(NULL, &set, SIGUSR1) != EINVAL ||
	    pid1_signal_add_set(fake_signal_add, NULL, SIGUSR1) != EINVAL)
		return 1;

	fake_empty_result = 0;
	fake_empty_errno = EBUSY;
	fake_empty_calls = 0;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	errno = EINTR;
	if (pid1_signal_empty_set(fake_signal_empty, &set) != 0 ||
	    fake_empty_calls != 1 || sigismember(&set, SIGUSR1) != 0 ||
	    errno != 0)
		return 1;

	fake_empty_result = -1;
	fake_empty_errno = EACCES;
	if (pid1_signal_empty_set(fake_signal_empty, &set) != EACCES)
		return 1;
	fake_empty_errno = 0;
	if (pid1_signal_empty_set(fake_signal_empty, &set) != EIO)
		return 1;

	fake_add_result = 0;
	fake_add_errno = EBUSY;
	fake_add_calls = 0;
	errno = EINTR;
	if (pid1_signal_add_set(fake_signal_add, &set, SIGUSR2) != 0 ||
	    fake_add_calls != 1 || fake_add_signal != SIGUSR2 ||
	    sigismember(&set, SIGUSR2) != 1 || errno != 0)
		return 1;

	/* SIGCHLD must be part of PID 1's blocked wait mask before sigsuspend. */
	fake_add_calls = 0;
	if (pid1_signal_add_set(fake_signal_add, &set, SIGCHLD) != 0 ||
	    fake_add_calls != 1 || fake_add_signal != SIGCHLD ||
	    sigismember(&set, SIGCHLD) != 1)
		return 1;

	fake_add_result = -1;
	fake_add_errno = ERANGE;
	if (pid1_signal_add_set(fake_signal_add, &set, SIGTERM) != ERANGE)
		return 1;
	fake_add_errno = 0;
	if (pid1_signal_add_set(fake_signal_add, &set, SIGTERM) != EIO)
		return 1;

	return 0;
}

static int fake_mask_how;
static int fake_mask_result;
static int fake_mask_errno;
static int fake_mask_saw_sigusr1;
static int fake_mask_saw_sigalrm;
static int fake_mask_old_written;

static int
fake_sigprocmask(int how, const sigset_t *set, sigset_t *old_set)
{
	fake_mask_how = how;
	fake_mask_saw_sigusr1 = sigismember(set, SIGUSR1);
	fake_mask_saw_sigalrm = sigismember(set, SIGALRM);
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

	/* The old-mask output is optional for transitions that do not need it. */
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	errno = EINTR;
	if (pid1_signal_update_mask(fake_sigprocmask, SIG_UNBLOCK, &set, NULL) != 0 ||
	    fake_mask_how != SIG_UNBLOCK || fake_mask_saw_sigalrm != 1 ||
	    fake_mask_old_written || errno != 0)
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


struct fake_waitpid_step {
	pid_t result;
	int error_number;
};

static struct fake_waitpid_step fake_waitpid_steps[4];
static int fake_waitpid_step_count;
static int fake_waitpid_step_index;

static pid_t
fake_pid1_waitpid(pid_t pid, int *status, int options)
{
	struct fake_waitpid_step step;

	assert(pid == -1);
	assert(status == NULL);
	assert(options == WNOHANG);
	assert(fake_waitpid_step_index < fake_waitpid_step_count);
	step = fake_waitpid_steps[fake_waitpid_step_index++];
	errno = step.error_number;
	return step.result;
}

static void
test_pid1_signal_reaper_preserves_errno(void)
{
	pid1_signal_state_t state = { 0, 0, 0, 0, 0, 0 };

	pid1_signal_record_console_shell(&state, (sig_atomic_t)4321);
	fake_waitpid_steps[0].result = (pid_t)9876;
	fake_waitpid_steps[0].error_number = ENOTTY;
	fake_waitpid_steps[1].result = (pid_t)-1;
	fake_waitpid_steps[1].error_number = EINTR;
	fake_waitpid_steps[2].result = (pid_t)4321;
	fake_waitpid_steps[2].error_number = EBUSY;
	fake_waitpid_steps[3].result = (pid_t)-1;
	fake_waitpid_steps[3].error_number = ECHILD;
	fake_waitpid_step_count = 4;
	fake_waitpid_step_index = 0;
	errno = EACCES;

	pid1_signal_reap_available_children(&state, fake_pid1_waitpid);

	assert(fake_waitpid_step_index == 4);
	assert(state.console_shell_pid == 0);
	assert(errno == EACCES);
}

static void
test_pid1_signal_shutdown_quiescence(void)
{
	assert(pid1_signal_shutdown_quiesce_required(0, 0) == 0);
	assert(pid1_signal_shutdown_quiesce_required(0, 1) == 0);
	assert(pid1_signal_shutdown_quiesce_required(1, 0) == 0);
	assert(pid1_signal_shutdown_quiesce_required(1, 1) == 1);
}

int
main(void)
{
	test_pid1_signal_shutdown_quiescence();
	assert(test_pid1_signal_set_initialization_reports_failures() == 0);
	assert(test_pid1_signal_installation_reports_failures() == 0);
	assert(test_pid1_signal_mask_update_reports_failures() == 0);
	test_pid1_signal_reaper_preserves_errno();
	pid1_signal_state_t state = { 0, 0, 0, 0, 0, 0 };

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
	assert(pid1_signal_take_console_shell_reaped(&state) == 0);

	/* Reaping the exact child publishes one completion for main context. */
	assert(pid1_signal_record_child_reaped(&state, (sig_atomic_t)1234) == 1);
	assert(state.console_shell_pid == 0);
	assert(state.console_shell_reaped == 1);
	assert(pid1_signal_take_console_shell_reaped(&state) == 1);
	assert(state.console_shell_reaped == 0);
	assert(pid1_signal_take_console_shell_reaped(&state) == 0);

	/* A later recovery shell can be tracked without stale state. */
	pid1_signal_record_console_shell(&state, (sig_atomic_t)2345);
	assert(state.console_shell_pid == (sig_atomic_t)2345);
	assert(pid1_signal_record_child_reaped(&state, (sig_atomic_t)2345) == 1);
	assert(state.console_shell_pid == 0);
	assert(pid1_signal_take_console_shell_reaped(&state) == 1);

	return 0;
}
