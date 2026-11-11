#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "networkmap_signal_policy.h"

static volatile sig_atomic_t refresh_requested;
static volatile sig_atomic_t daemon_exit;

static int
check(int condition, const char *message)
{
	if (!condition) {
		fprintf(stderr, "FAIL: %s\n", message);
		return 1;
	}
	return 0;
}

static void
test_signal_handler(int signal_number)
{
	networkmap_signal_record(&refresh_requested, &daemon_exit, signal_number);
}

int
main(void)
{
	struct sigaction action;
	sigset_t block_set, previous_set;
	int failures = 0;

	refresh_requested = 0;
	daemon_exit = 0;

	memset(&action, 0, sizeof(action));
	sigemptyset(&action.sa_mask);
	action.sa_handler = test_signal_handler;
	if (sigaction(SIGUSR1, &action, NULL) < 0) {
		perror("sigaction SIGUSR1");
		return 1;
	}
	if (sigaction(SIGTERM, &action, NULL) < 0) {
		perror("sigaction SIGTERM");
		return 1;
	}

	if (raise(SIGUSR1) != 0)
		return 1;
	failures += check(refresh_requested == 1 && daemon_exit == 0,
			  "SIGUSR1 requests refresh only");
	failures += check(networkmap_signal_take_refresh(&refresh_requested) == 1 &&
			  refresh_requested == 0,
			  "refresh request is consumed exactly once");
	failures += check(networkmap_signal_take_refresh(&refresh_requested) == 0,
			  "empty refresh state stays empty");

	networkmap_signal_record(&refresh_requested, &daemon_exit, SIGUSR1);
	networkmap_signal_record(&refresh_requested, &daemon_exit, SIGUSR1);
	failures += check(refresh_requested == 1,
			  "repeated refresh requests coalesce");

	if (raise(SIGTERM) != 0)
		return 1;
	failures += check(refresh_requested == 1 && daemon_exit == 1,
			  "SIGTERM requests shutdown without clearing refresh");

	networkmap_signal_record(&refresh_requested, &daemon_exit, SIGINT);
	failures += check(refresh_requested == 1 && daemon_exit == 1,
			  "unmanaged signals are ignored");

	/* A SIGUSR1 queued during the snapshot remains pending until unblocked. */
	refresh_requested = 0;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &block_set, &previous_set) < 0) {
		perror("sigprocmask block test");
		return 1;
	}
	if (kill(getpid(), SIGUSR1) < 0) {
		perror("kill SIGUSR1");
		return 1;
	}
	failures += check(networkmap_signal_take_refresh(&refresh_requested) == 0,
			  "snapshot preserves the caller's already-blocked mask");
	if (sigprocmask(SIG_SETMASK, &previous_set, NULL) < 0) {
		perror("sigprocmask restore test");
		return 1;
	}
	failures += check(refresh_requested == 1,
			  "pending SIGUSR1 is delivered after original mask restoration");
	failures += check(networkmap_signal_take_refresh(&refresh_requested) == 1,
			  "pending refresh is consumed on the next main-loop pass");

	failures += check(networkmap_signal_take_refresh(NULL) == -1,
			  "NULL refresh state fails explicitly");

	if (failures)
		return 1;

	puts("networkmap signal deferral policy: PASS");
	return 0;
}
