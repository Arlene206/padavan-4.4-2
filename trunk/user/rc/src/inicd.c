#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "rc.h"
#include "inicd_signal_policy.h"

#define INICD_POLL_INTERVAL 15
#define INICD_PID_FILE "/var/run/inicd.pid"

static volatile sig_atomic_t pending_events;

static int
inicd_alarmtimer(unsigned long sec)
{
	struct itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	if (setitimer(ITIMER_REAL, &itv, NULL) < 0)
		return -errno;
	return 0;
}

static void
catch_sig_inicd(int sig)
{
	/*
	 * Defer all work: this handler can interrupt libc, NVRAM or driver
	 * operations, so it only records a sig_atomic_t event.
	 */
	inicd_record_signal(&pending_events, sig);
}

static void
inicd_cleanup(void)
{
	if (inicd_alarmtimer(0) < 0)
		logmessage("inicd", "Failed to disable health-check timer: %s", strerror(errno));
	if (remove(INICD_PID_FILE) < 0 && errno != ENOENT)
		logmessage("inicd", "Failed to remove PID file: %s", strerror(errno));
}

int
stop_inicd(void)
{
	return doSystem("killall %s %s", "-q", "inicd");
}

int
start_inicd(void)
{
	return eval("/sbin/inicd");
}

int
inicd_main(int argc, char *argv[])
{
	FILE *fp;
	struct sigaction sa;
	sigset_t managed_signals, previous_mask, wait_mask, pending_signals;
	sig_atomic_t events;
	int timer_status;
	int exit_status = EXIT_SUCCESS;

	(void)argc;
	(void)argv;

	/*
	 * Block managed signals before installing handlers. The main loop
	 * snapshots flags while blocked; sigsuspend() atomically unmasks while
	 * waiting, preventing a check-then-sleep lost wakeup.
	 */
	sigemptyset(&managed_signals);
	sigaddset(&managed_signals, SIGALRM);
	sigaddset(&managed_signals, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &managed_signals, &previous_mask) < 0) {
		int saved_errno = errno;
		logmessage("inicd", "Failed to block managed signals: %s", strerror(saved_errno));
		return saved_errno;
	}

	wait_mask = previous_mask;
	sigdelset(&wait_mask, SIGALRM);
	sigdelset(&wait_mask, SIGTERM);
	pending_events = 0;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_inicd;
	sa.sa_mask = managed_signals;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		int saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
		logmessage("inicd", "Failed to install SIGALRM handler: %s", strerror(saved_errno));
		return saved_errno;
	}
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		int saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
		logmessage("inicd", "Failed to install SIGTERM handler: %s", strerror(saved_errno));
		return saved_errno;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	if (daemon(0, 0) < 0) {
		int saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
		errno = saved_errno;
		perror("daemon");
		return saved_errno;
	}

	if ((fp = fopen(INICD_PID_FILE, "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	timer_status = inicd_alarmtimer(INICD_POLL_INTERVAL);
	if (timer_status < 0) {
		logmessage("inicd", "Failed to start health-check timer: %s", strerror(-timer_status));
		inicd_cleanup();
		(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
		return -timer_status;
	}

	/*
	 * Execute wireless/NVRAM/ioctl work in normal process context. SIGTERM
	 * takes precedence over a coincident poll.
	 */
	for (;;) {
		events = inicd_take_pending_events(&pending_events);
		if (sigpending(&pending_signals) < 0) {
			logmessage("inicd", "Failed to inspect pending signals: %s", strerror(errno));
			exit_status = EXIT_FAILURE;
			break;
		}
		/*
		 * The two standard signals are delivered one at a time. If SIGALRM
		 * woke sigsuspend() while SIGTERM remained kernel-pending, observe
		 * that stop request before dispatching the poll callback.
		 */
		if (sigismember(&pending_signals, SIGTERM) == 1)
			events |= INICD_EVENT_STOP;
		if (inicd_event_should_stop(events))
			break;

		if (events & INICD_EVENT_POLL)
			check_inic_mii_rebooted();

		if (events == 0 &&
		    sigsuspend(&wait_mask) < 0 && errno != EINTR) {
			logmessage("inicd", "Signal wait failed: %s", strerror(errno));
			exit_status = EXIT_FAILURE;
			break;
		}
	}

	inicd_cleanup();
	(void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
	return exit_status;
}
