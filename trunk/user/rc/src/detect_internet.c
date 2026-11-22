/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>

#include "rc.h"
#include "detect_internet_signal_policy.h"
#include "detect_internet_nvram_policy.h"
#include "detect_internet_host_policy.h"
#include "process_presence_start_policy.h"
#include "detect_internet_notification_policy.h"
#include <process_inventory.h>
#include <process_pidfile.h>
#include <gpioutils.h>

#define DI_MAX_HOSTS	6
#define DI_STATUS_INIT	2
#define DI_PID_FILE	"/var/run/detect_internet.pid"

static struct sockaddr_in    di_host_sa[DI_MAX_HOSTS];
static int                   di_host_ok[DI_MAX_HOSTS];
static int                   di_host_next;
static int                   di_host_total;
static int                   di_settings_error;
static int                   di_poll_mode;
static int                   di_status;
static int                   di_status_nvram_old = -1;
static int                   di_status_nvram_target = DI_STATUS_INIT;
static int                   di_status_nvram_error = 0;
static int                   di_is_ap_mode;
static long                  di_time_last_activity;
static long                  di_time_last_state;
static long                  di_time_diff_state;
static long                  di_time_fail_event;
static volatile sig_atomic_t di_pause_received = 0;
static struct itimerval      di_itv;
static volatile sig_atomic_t di_pending_events;
static volatile sig_atomic_t di_pending_control = DI_CONTROL_NONE;

static void
di_alarmtimer(unsigned long sec)
{
	di_itv.it_value.tv_sec  = sec;
	di_itv.it_value.tv_usec = 0;
	di_itv.it_interval = di_itv.it_value;
	setitimer(ITIMER_REAL, &di_itv, NULL);
}

static int
di_connect_to_host(const struct sockaddr_in *p_sa_dst, int timeout)
{
	int fd, flags, ret, fd_status;
	struct linger lgr;
	struct timeval to;
	fd_set rset;

	ret = -1;

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
		return ret;

	/*
	 * FD_SET() indexes a fixed-size fd_set. A valid socket descriptor can
	 * still exceed that bound when the process has many open descriptors.
	 */
	fd_status = detect_internet_select_fd_status(fd, FD_SETSIZE);
	if (fd_status != 0) {
		close(fd);
		errno = fd_status;
		return -1;
	}

	/* allow immediate reuse of the address */
	flags = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags));

	/* don't wait for data to be delivered on close(). */
	memset(&lgr, 0, sizeof(lgr));
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&lgr, sizeof(lgr));

	/* set non-blocking mode */
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		goto done_exit;

	ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
		goto done_exit;

	/* try connect to host */
	ret = connect(fd, (struct sockaddr *)p_sa_dst, sizeof(struct sockaddr_in));
	if (ret < 0 && errno != EINPROGRESS)
		goto done_exit;

	if (ret == 0)
		goto done_exit;

	/* now wait connection established */
	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	to.tv_sec = timeout;
	to.tv_usec = 0;

	ret = select(fd + 1, NULL, &rset, NULL, &to);
	if (ret <= 0) {
		if (ret == 0)
			errno = ETIMEDOUT;
		ret = -1;
		goto done_exit;
	}

	if (di_pause_received) {
		ret = -1;
		goto done_exit;
	}

	if (FD_ISSET(fd, &rset)) {
		socklen_t len = (socklen_t)sizeof(ret);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len) < 0) {
			ret = -1;
			goto done_exit;
		}
		
		/* oops - something wrong with connect */
		if (ret != 0) {
			errno = ret;
			ret = -1;
		}
	}

done_exit:

	close(fd);

	return ret;
}

static int
di_write_nvram_status(void *context, int status)
{
	(void)context;
	return nvram_set_int_temp("link_internet", status);
}


static int
di_publish_nvram_status(int status)
{
	int result;

	di_status_nvram_target = status;
	result = detect_internet_publish_status(
		status, &di_status_nvram_old, di_write_nvram_status, NULL);
	if (result != 0) {
		if (di_status_nvram_error != result)
			logmessage("detect_internet",
				   "cannot publish Internet connectivity state (status=%d)",
				   result);
		di_status_nvram_error = result;
	} else {
		if (di_status_nvram_error != 0)
			logmessage("detect_internet",
				   "Internet connectivity NVRAM publication recovered");
		di_status_nvram_error = 0;
	}
	return result;
}


static void
di_reset_state(void)
{
	int i, status;

	for (i = 0; i < DI_MAX_HOSTS; i++)
		di_host_ok[i] = DI_STATUS_INIT;

	di_host_next = 0;

	di_time_fail_event = 0;
	di_time_last_state = uptime();
	di_time_diff_state = 0;

	di_status = DI_STATUS_INIT;
	status = di_publish_nvram_status(DI_STATUS_INIT);
	if (status != 0 && di_pause_received)
		di_alarmtimer(1);
}

static int
detect_internet_nvram_get_checked(void *context, const char *param_name,
				  char **value)
{
	(void)context;
	return nvram_get_checked(param_name, value);
}

static int
di_load_settings(void)
{
	struct sockaddr_in host_snapshot[DI_MAX_HOSTS];
	int host_ok_snapshot[DI_MAX_HOSTS];
	int i, i_items, port, poll_mode, status, previous_error;
	char nvram_addr[16], nvram_port[16], address[16];
	in_addr_t address_value;

	memset(host_snapshot, 0, sizeof(host_snapshot));
	for (i = 0; i < DI_MAX_HOSTS; ++i)
		host_ok_snapshot[i] = DI_STATUS_INIT;

	i_items = 0;
	for (i = 0; i < DI_MAX_HOSTS; ++i) {
		snprintf(nvram_addr, sizeof(nvram_addr), "di_addr%d", i);
		snprintf(nvram_port, sizeof(nvram_port), "di_port%d", i);

		status = detect_internet_read_endpoint(
			detect_internet_nvram_get_checked, NULL,
			nvram_addr, nvram_port, address, sizeof(address), &port);
		if (status != 0)
			goto read_failed;

		address_value = inet_addr_safe(address);
		if (address_value == INADDR_ANY || address_value == INADDR_NONE)
			continue;

		host_snapshot[i_items].sin_family = AF_INET;
		host_snapshot[i_items].sin_addr.s_addr = address_value;
		host_snapshot[i_items].sin_port = htons((unsigned short)port);
		++i_items;
	}

	if (i_items < 1) {
		host_snapshot[0].sin_family = AF_INET;
		host_snapshot[0].sin_addr.s_addr = 0x08080808;
		host_snapshot[0].sin_port = htons(53);
		i_items = 1;
	}

	status = detect_internet_read_int_setting(
		detect_internet_nvram_get_checked, NULL,
		"di_poll_mode", 0, 1, 0, &poll_mode);
	if (status != 0)
		goto read_failed;

	/*
	 * Commit the complete candidate only after every NVRAM read succeeds.
	 * Failed refreshes keep the previous snapshot but suppress probes until
	 * a later one-second retry can publish a complete replacement.
	 */
	previous_error = di_settings_error;
	memcpy(di_host_sa, host_snapshot, sizeof(di_host_sa));
	memcpy(di_host_ok, host_ok_snapshot, sizeof(di_host_ok));
	di_host_total = i_items;
	di_host_next = (i_items > 1) ? rand_seed_by_time() % i_items : 0;
	di_poll_mode = poll_mode;
	di_is_ap_mode = get_ap_mode();
	di_settings_error = 0;
	if (previous_error != 0)
		logmessage("detect_internet",
			   "Internet probe settings recovered after NVRAM read failure");
	return 0;

read_failed:
	previous_error = di_settings_error;
	di_settings_error = status;
	if (previous_error != status)
		logmessage("detect_internet",
			   "cannot load Internet probe settings (status=%d); retrying",
			   status);
	return status;
}

static void
di_update_last_activity(void)
{
	long last_activity = di_time_last_activity;

	di_time_last_activity = uptime();

	/* boost poll timer once */
	if (di_poll_mode == 0) {
		if ((unsigned long)(di_time_last_activity - last_activity) > 60) {
			if (!di_pause_received && di_itv.it_value.tv_sec != 1)
				di_alarmtimer(1);
		}
	}
}

static void
di_on_timer(void)
{
	int i, link_error, link_internet, has_unchecked;
	int publish_status, settings_status;
	int runtime_status, timeout, period_done, period_fail;
	int front_led_mode = 0, lost_delay = 10;
	long now;
	unsigned long period_next;

	/* A paused monitor only retries its externally visible down state. */
	if (di_pause_received) {
		publish_status = di_publish_nvram_status(di_status_nvram_target);
		di_alarmtimer(publish_status != 0 ? 1 : 0);
		return;
	}

	/* Never probe with an incomplete/failed NVRAM endpoint snapshot. */
	if (di_settings_error != 0) {
		settings_status = di_load_settings();
		if (settings_status != 0) {
			di_alarmtimer(1);
			return;
		}
	}

	/* Retry a failed mirror even when connectivity itself did not change. */
	publish_status = di_publish_nvram_status(di_status);

	/* A paused/stopping monitor must not publish a late probe result. */
	if (di_pause_received)
		return;

	/* check last user activity */
	if (di_poll_mode == 0) {
		if ((unsigned long)(uptime() - di_time_last_activity) > 60) {
			/* relax probe cadence, but keep retrying a failed NVRAM mirror */
			if (publish_status != 0)
				di_alarmtimer(1);
			else if (!di_pause_received && di_itv.it_value.tv_sec != 60)
				di_alarmtimer(60);
			return;
		}
	}

	/*
	 * Read probe timeout/cadence before starting network work. A failed NVRAM
	 * read must not silently turn into default probe timing.
	 */
	runtime_status = detect_internet_read_int_setting(
		detect_internet_nvram_get_checked, NULL, "di_timeout",
		1, 10, 3, &timeout);
	if (runtime_status == 0)
		runtime_status = detect_internet_read_int_setting(
			detect_internet_nvram_get_checked, NULL, "di_time_done",
			15, 600, 55, &period_done);
	if (runtime_status == 0)
		runtime_status = detect_internet_read_int_setting(
			detect_internet_nvram_get_checked, NULL, "di_time_fail",
			3, 120, 5, &period_fail);
	if (runtime_status != 0) {
		logmessage("detect_internet",
			   "cannot read probe timeout/cadence settings (status=%d); retrying",
			   runtime_status);
		di_alarmtimer(1);
		return;
	}

	link_error = -1;

	i = di_host_next;

	if (di_is_ap_mode || has_wan_ip4(0))
		link_error = di_connect_to_host(&di_host_sa[i], timeout);

	/* SIGUSR2/SIGTERM may interrupt the bounded connect/select operation. */
	if (di_pause_received)
		return;

	di_host_ok[i] = (link_error == 0) ? 1 : 0;
	di_host_next = (di_host_next + 1) % di_host_total;

	if (link_error == 0)
		period_next = (unsigned long)period_done;
	else
		period_next = (unsigned long)period_fail;

	has_unchecked = 0;
	link_internet = DI_STATUS_INIT;
	for (i = 0; i < di_host_total; i++) {
		if (di_host_ok[i] == DI_STATUS_INIT)
			has_unchecked = 1;
		if (di_host_ok[i] == 1) {
			link_internet = 1;
			break;
		}
	}

	now = uptime();

	/* all hosts failed */
	if (link_internet != 1 && !has_unchecked)
		link_internet = 0;

	if (di_pause_received)
		link_internet = 0;

	if (link_internet == 1 && di_time_fail_event)
		di_time_fail_event = 0;

	if ((di_time_fail_event > 0) && (now >= di_time_fail_event)) {
		di_time_fail_event = 0;
		
		if (link_internet == 0 && di_poll_mode != 0 && !di_pause_received)
			notify_on_internet_state_changed(0, di_time_diff_state);
	}

	if (link_internet != DI_STATUS_INIT && di_status != link_internet) {
		runtime_status = detect_internet_read_int_setting(
			detect_internet_nvram_get_checked, NULL, "front_led_wan",
			INT_MIN, INT_MAX, 0, &front_led_mode);
		if (runtime_status == 0 && di_poll_mode != 0 && !link_internet)
			runtime_status = detect_internet_read_int_setting(
				detect_internet_nvram_get_checked, NULL, "di_lost_delay",
				0, 600, 10, &lost_delay);
		if (runtime_status != 0) {
			logmessage("detect_internet",
				   "cannot read Internet transition policy (status=%d); retrying",
				   runtime_status);
			di_alarmtimer(1);
			return;
		}
		di_status = link_internet;
		di_time_diff_state = now - di_time_last_state;
		publish_status = di_publish_nvram_status(link_internet);
		
		if (front_led_mode == 3) {
			LED_CONTROL(LED_WAN, (link_internet) ? LED_ON : LED_OFF);
		}
		if (di_poll_mode != 0 && !di_pause_received) {
			long fail_delay = (long)lost_delay;
			
			if (link_internet || fail_delay == 0) {
				notify_on_internet_state_changed(link_internet, di_time_diff_state);
			} else {
				di_time_fail_event = now + fail_delay;
				di_time_diff_state += fail_delay;
			}
		}
		
		di_time_last_state = now;
	}

	if (!di_pause_received) {
		unsigned long next_period = publish_status != 0 ? 1 : period_next;

		if (di_itv.it_value.tv_sec != next_period)
			di_alarmtimer(next_period);
	}
}

static void
di_on_sigusr1(void)
{
	int delay_time, settings_status;

	di_pause_received = 0;

	di_alarmtimer(0);
	di_reset_state();
	settings_status = di_load_settings();
	if (settings_status != 0) {
		di_alarmtimer(1);
		return;
	}

	settings_status = detect_internet_read_int_setting(
		detect_internet_nvram_get_checked, NULL, "di_notify_delay",
		INT_MIN, INT_MAX, 0, &delay_time);
	if (settings_status != 0) {
		logmessage("detect_internet",
			   "cannot read Internet resume delay (status=%d); retrying",
			   settings_status);
		di_alarmtimer(1);
		return;
	}
	if (delay_time < 1)
		delay_time = 1;

	di_alarmtimer(delay_time);
}

static void
di_on_sigusr2(void)
{
	int status;

	di_pause_received = 1;
	di_alarmtimer(0);
	status = di_publish_nvram_status(0);
	if (status != 0)
		di_alarmtimer(1);
}

static void
catch_sig_detect_internet(int sig)
{
	detect_internet_record_signal(&di_pending_events, &di_pending_control,
				       &di_pause_received, sig);
}

int
stop_detect_internet(void)
{
	char *services[] = { "detect_internet", NULL };
	const char *pidfiles[] = { DI_PID_FILE, NULL };

	return kill_services_with_pidfiles(services, pidfiles, "detect_internet", 0, 3, 1);
}

int
start_detect_internet(int autorun_time)
{
	char arun[16];
	char *di_argv[] = {
		"/sbin/detect_internet",
		NULL,		/* -aX */
		NULL
	};

	if (autorun_time > 0) {
		snprintf(arun, sizeof(arun), "-a%u", autorun_time);
		di_argv[1] = arun;
	}

	return _eval(di_argv, NULL, 0, NULL);
}

int
notify_run_detect_internet(int delay_time)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;
	int scan_error, delay_write_status, delay_write_errno;
	int operation_status, operation_errno, operation_error;

	/*
	 * A live monitor consumes this delay after SIGUSR1. Do not signal it
	 * with a stale delay when the temporary NVRAM write failed.
	 */
	errno = 0;
	delay_write_status = nvram_set_int_temp("di_notify_delay", delay_time);
	delay_write_errno = errno;
	if (delay_write_status != 0) {
		operation_error = detect_internet_operation_error(
			delay_write_status, delay_write_errno);
		logmessage("detect_internet",
			   "cannot store Internet monitor resume delay (status=%d, errno=%d)",
			   delay_write_status, operation_error);
		return operation_error;
	}

	presence = process_inventory_scan("detect_internet", &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	action = process_presence_start_action_for_state(presence);
	if (action == PROCESS_PRESENCE_START_NEW) {
		errno = 0;
		operation_status = start_detect_internet(delay_time);
		operation_errno = errno;
		operation_error = detect_internet_operation_error(
			operation_status, operation_errno);
		if (operation_error != 0)
			logmessage("detect_internet",
				   "monitor launch failed (status=%d, errno=%d)",
				   operation_status, operation_error);
	} else if (action == PROCESS_PRESENCE_START_NOTIFY) {
		errno = 0;
		operation_status = process_pidfile_signal(
			DI_PID_FILE, "detect_internet", SIGUSR1);
		operation_errno = errno;
		operation_error = detect_internet_operation_error(
			operation_status, operation_errno);
		if (operation_error != 0)
			logmessage("detect_internet",
				   "monitor resume signal failed (status=%d, errno=%d)",
				   operation_status, operation_error);
	} else {
		operation_error = detect_internet_operation_error(-1, scan_error);
		logmessage("detect_internet",
			   "skipping start/notification because process inventory is unknown (error=%d)",
			   operation_error);
	}
	return operation_error;
}

int
notify_pause_detect_internet(void)
{
	int operation_status, operation_errno, operation_error;

	errno = 0;
	operation_status = process_pidfile_signal(
		DI_PID_FILE, "detect_internet", SIGUSR2);
	operation_errno = errno;
	operation_error = detect_internet_pause_error(
		operation_status, operation_errno);
	if (operation_error != 0)
		logmessage("detect_internet",
			   "monitor pause signal failed (status=%d, errno=%d)",
			   operation_status, operation_error);
	return operation_error;
}

int
detect_internet_main(int argc, char *argv[])
{
	pid_t pid;
	int c, auto_run_time = 0;
	int settings_status;
	int saved_errno;
	sigset_t managed_signals, original_mask, wait_mask, run_mask;
	struct sigaction sa;

	// usage : detect_internet -a X
	if(argc) {
		while ((c = getopt(argc, argv, "a:")) != -1) {
			switch (c) {
			case 'a':
				auto_run_time = atoi(optarg);
				break;
			}
		}
	}

	/*
	 * Keep all monitor signals blocked while taking pending work. The
	 * handler only records sig_atomic_t flags; all NVRAM, socket, timer,
	 * process and script operations run later in normal process context.
	 */
	sigemptyset(&managed_signals);
	sigaddset(&managed_signals, SIGHUP);
	sigaddset(&managed_signals, SIGUSR1);
	sigaddset(&managed_signals, SIGUSR2);
	sigaddset(&managed_signals, SIGALRM);
	sigaddset(&managed_signals, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &managed_signals, &original_mask) < 0) {
		perror("sigprocmask");
		return errno;
	}

	wait_mask = original_mask;
	run_mask = original_mask;
	sigdelset(&wait_mask, SIGHUP);
	sigdelset(&wait_mask, SIGUSR1);
	sigdelset(&wait_mask, SIGUSR2);
	sigdelset(&wait_mask, SIGALRM);
	sigdelset(&wait_mask, SIGTERM);
	sigdelset(&run_mask, SIGHUP);
	sigdelset(&run_mask, SIGUSR1);
	sigdelset(&run_mask, SIGUSR2);
	sigdelset(&run_mask, SIGALRM);
	sigdelset(&run_mask, SIGTERM);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_detect_internet;
	sa.sa_mask = managed_signals;
	if (sigaction(SIGHUP, &sa, NULL) < 0 ||
	    sigaction(SIGUSR1, &sa, NULL) < 0 ||
	    sigaction(SIGUSR2, &sa, NULL) < 0 ||
	    sigaction(SIGALRM, &sa, NULL) < 0 ||
	    sigaction(SIGTERM, &sa, NULL) < 0)
		goto signal_setup_failed;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGPIPE, &sa, NULL) < 0)
		goto signal_setup_failed;

	if (daemon(0, 0) < 0) {
		saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
		errno = saved_errno;
		perror("daemon");
		return saved_errno;
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	/* Publish PID and start_time before accepting notifications. */
	if (process_pidfile_write(DI_PID_FILE, "detect_internet") < 0) {
		saved_errno = errno ? errno : EIO;
		(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
		errno = saved_errno;
		perror("detect_internet pid file");
		return saved_errno;
	}

	di_pending_events = 0;
	di_pending_control = DI_CONTROL_NONE;
	nvram_set_int_temp("di_notify_delay", 0);

	di_pause_received = (auto_run_time > 0) ? 0 : 1;
	di_time_last_activity = 0;

	di_reset_state();
	settings_status = di_load_settings();
	if (settings_status != 0)
		di_alarmtimer(1);
	else if (auto_run_time > 0)
		di_alarmtimer(auto_run_time);

	for (;;) {
		sig_atomic_t events, control;

		/* Managed signals are blocked here, so snapshot-and-clear is atomic. */
		events = di_pending_events;
		control = di_pending_control;
		di_pending_events = 0;
		di_pending_control = DI_CONTROL_NONE;

		if (!events && control == DI_CONTROL_NONE) {
			if (sigsuspend(&wait_mask) < 0 && errno != EINTR) {
				saved_errno = errno;
				di_alarmtimer(0);
				if (remove(DI_PID_FILE) != 0 && errno != ENOENT)
					logmessage("detect_internet", "cannot remove pid file (errno=%d)", errno);
				(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
				errno = saved_errno;
				perror("sigsuspend");
				return saved_errno;
			}
			continue;
		}

		/* Termination wins over any queued refresh or timer tick. */
		if (events & DI_PENDING_TERMINATE) {
			di_alarmtimer(0);
			if (remove(DI_PID_FILE) != 0 && errno != ENOENT)
				logmessage("detect_internet", "cannot remove pid file (errno=%d)", errno);
			if (sigprocmask(SIG_SETMASK, &original_mask, NULL) < 0)
				logmessage("detect_internet", "cannot restore signal mask during shutdown (errno=%d)", errno);
			return 0;
		}

		if (control == DI_CONTROL_PAUSE)
			di_on_sigusr2();
		else if (control == DI_CONTROL_RESUME)
			di_on_sigusr1();

		if (events & DI_PENDING_ACTIVITY)
			di_update_last_activity();

		/*
		 * Apply pause/resume while managed signals remain blocked. Only
		 * unblock for the bounded probe so SIGUSR2/SIGTERM can interrupt
		 * select without racing a newer control request against resume.
		 */
		if ((events & DI_PENDING_TIMER) &&
		    control != DI_CONTROL_RESUME &&
		    (!di_pause_received || di_status_nvram_error != 0)) {
			if (sigprocmask(SIG_SETMASK, &run_mask, NULL) < 0) {
				saved_errno = errno;
				di_alarmtimer(0);
				if (remove(DI_PID_FILE) != 0 && errno != ENOENT)
					logmessage("detect_internet", "cannot remove pid file (errno=%d)", errno);
				(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
				errno = saved_errno;
				perror("sigprocmask");
				return saved_errno;
			}
			di_on_timer();
		}

		if (sigprocmask(SIG_BLOCK, &managed_signals, NULL) < 0) {
			saved_errno = errno;
			di_alarmtimer(0);
			if (remove(DI_PID_FILE) != 0 && errno != ENOENT)
				logmessage("detect_internet", "cannot remove pid file (errno=%d)", errno);
			(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
			errno = saved_errno;
			perror("sigprocmask");
			return saved_errno;
		}
	}

signal_setup_failed:
	saved_errno = errno;
	(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
	errno = saved_errno;
	perror("sigaction");
	return saved_errno;
}

