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
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <linux/limits.h>

#include "rc.h"
#include <process_inventory.h>
#include <process_pidfile.h>
#include "process_presence_start_policy.h"
#include "detached_daemon_start_policy.h"
#include "watchdog_signal_policy.h"
#include "watchdog_ntp_policy.h"
#include "service_wait_policy.h"

#define WD_NORMAL_PERIOD	10		/* 10s */

#define WD_NOTIFY_ID_WIFI2	1
#define WD_NOTIFY_ID_WIFI5	2

#define WD_PID_FILE		"/var/run/watchdog.pid"

enum
{
	RADIO5_ACTIVE = 0,
	GUEST5_ACTIVE,
	RADIO2_ACTIVE,
	GUEST2_ACTIVE,
	ACTIVEITEMS
};

static int svcStatus[ACTIVEITEMS] = {-1, -1, -1, -1};

static int ntpc_timer = -1;
static int ntpc_server_idx = 0;
static int ntpc_tries = 0;
static int ntp_ready_publish_error = 0;

static int httpd_missing = 0;
static int dnsmasq_missing = 0;

static struct itimerval wd_itv;

/* Signal handlers only record events; the watchdog loop owns all work. */
static volatile sig_atomic_t wd_pending_events = 0;

static int
wd_alarmtimer(unsigned long sec, unsigned long usec)
{
	wd_itv.it_value.tv_sec  = sec;
	wd_itv.it_value.tv_usec = usec;
	wd_itv.it_interval = wd_itv.it_value;
	return setitimer(ITIMER_REAL, &wd_itv, NULL);
}

#ifdef HTTPD_CHECK
#define DETECT_HTTPD_FILE "/tmp/httpd_check_result"
static int
httpd_check_v2()
{
	FILE *fp = NULL;
	int i, httpd_live, http_port;
	char line[80], *login_timestamp;
	long now;
	static int check_count_down = 3;
	static int httpd_timer = 0;

	/* skip 30 seconds after start watchdog */
	if (check_count_down)
	{
		check_count_down--;
		return 1;
	}

	/* check every 30 seconds */
	httpd_timer = (httpd_timer + 1) % 3;
	if (httpd_timer)
		return 1;

	/* check last http login */
	login_timestamp = nvram_safe_get("login_timestamp");
	if (strlen(login_timestamp) < 1)
		return 1;

	now = uptime();
	if (((unsigned long)(now - strtoul(login_timestamp, NULL, 10)) < 60))
		return 1;

#if defined (SUPPORT_HTTPS)
	/* check HTTPS only */
	if (nvram_get_int("http_proto") == 1)
		return 1;
#endif
	remove(DETECT_HTTPD_FILE);

	http_port = nvram_get_int("http_lanport");

	/* httpd will not count 127.0.0.1 */
	/* The probe is detached; bound its network wait instead of killing every wget. */
	doSystem("wget -q -T 1 http://127.0.0.1:%d/httpd_check.htm -O %s &", http_port, DETECT_HTTPD_FILE);

	httpd_live = 0;
	for (i=0; i < 3; i++)
	{
		if ((fp = fopen(DETECT_HTTPD_FILE, "r")) != NULL)
		{
			if ( fgets(line, sizeof(line), fp) != NULL )
			{
				if (strstr(line, "ASUSTeK"))
				{
					httpd_live = 1;
				}
			}
			
			fclose(fp);
		}
		
		if (httpd_live)
			break;
		
		/* check port changed */
		if (nvram_get_int("http_lanport") != http_port)
			return 1;
		
		sleep(1);
	}

	if (!httpd_live)
	{
		dbg("httpd is so dead!!!\n");
		
		return 0;
	}

	return 1;
}
#endif

static int
refresh_ntp(void)
{
	int result = 0;
	int stop_status;
	char *svcs[] = { "ntpd", NULL };
	char *ntp_addr[2], *ntp_server;

	stop_status = kill_services(svcs, 3, 1);
	if (!watchdog_ntp_refresh_should_launch(stop_status)) {
		logmessage("NTP Client",
			   "Previous ntpd instance did not stop (status %d); refusing overlapping synchronization",
			   stop_status);
		return stop_status;
	}

	ntp_addr[0] = nvram_safe_get("ntp_server0");
	ntp_addr[1] = nvram_safe_get("ntp_server1");

	if (strlen(ntp_addr[0]) < 3)
		ntp_addr[0] = ntp_addr[1];
	else if (strlen(ntp_addr[1]) < 3)
		ntp_addr[1] = ntp_addr[0];

	if (strlen(ntp_addr[0]) < 3) {
		ntp_addr[0] = "pool.ntp.org";
		ntp_addr[1] = ntp_addr[0];
	}

	ntp_server = ntp_addr[ntpc_server_idx];
	ntpc_server_idx = (ntpc_server_idx + 1) % 2;

	result = eval("/usr/sbin/ntpd", "-qt", "-S", NTPC_DONE_SCRIPT, "-p", ntp_server);

	logmessage("NTP Client", "Synchronizing time to %s.", ntp_server);
	return result;
}

int
is_ntpc_updated(void)
{
	return (nvram_get_int("ntpc_counter") > 0) ? 1 : 0;
}

static void
ntpc_handler(void)
{
	int ntp_period = nvram_get_int("ntp_period");

	if (ntp_period < 1)
		return;

	if (ntp_period > 336)
		ntp_period = 336; // max two weeks

	ntp_period = ntp_period * 360;

	// update ntp every period time
	ntpc_timer = (ntpc_timer + 1) % ntp_period;
	if (ntpc_timer == 0) {
		setenv_tz();
		refresh_ntp();
	} else if (!is_ntpc_updated()) {
		int ntp_skip = 3;	// update every 30s
		
		ntpc_tries++;
		if (ntpc_tries > 60)
			ntp_skip = 30;	// update every 5m
		else if (ntpc_tries > 9)
			ntp_skip = 6;	// update every 60s
		
		if (!(ntpc_tries % ntp_skip))
			refresh_ntp();
	}
}

static void
ntpc_ready_handler(void)
{
	int ntpc_counter, ntp_ready, status;

	ntpc_counter = nvram_get_int("ntpc_counter");
	ntp_ready = nvram_get_int("ntp_ready");
	if (!watchdog_ntp_ready_retry_required(ntpc_counter, ntp_ready))
		return;

	status = nvram_set_int("ntp_ready", 1);
	if (status != 0) {
		if (ntp_ready_publish_error != status)
			logmessage("NTP Client",
				   "cannot persist NTP readiness (status=%d)", status);
	} else if (ntp_ready_publish_error != 0) {
		logmessage("NTP Client", "NTP readiness persistence recovered");
	}
	ntp_ready_publish_error = status;
}


static void
inet_handler(int is_ap_mode)
{
	if (!is_ap_mode)
	{
		long i_deferred_wanup = nvram_get_int("deferred_wanup_t");
		if (i_deferred_wanup > 0 && uptime() >= i_deferred_wanup)
		{
			notify_rc("deferred_wan_connect");
			
			return;
		}
		
		if (has_wan_ip4(0) && has_wan_gw4())
		{
			/* sync time to ntp server if necessary */
			ntpc_handler();
		}
	}
	else
	{
		if (has_lan_ip4() && has_lan_gw4())
			ntpc_handler();
	}
}

int timecheck_reboot(char *activeSchedule)
{
	int active, current_time, current_date, Time2Active, Date2Active;
	int hour, minute, i;
	time_t now;
	struct tm *tm;

	if (!activeSchedule || strlen(activeSchedule) != 11)
		return 0;

	for (i = 0; i < 7; i++) {
		if (activeSchedule[i] != '0' && activeSchedule[i] != '1')
			return 0;
	}
	for (i = 7; i < 11; i++) {
		if (activeSchedule[i] < '0' || activeSchedule[i] > '9')
			return 0;
	}

	hour = (activeSchedule[7] - '0') * 10 + activeSchedule[8] - '0';
	minute = (activeSchedule[9] - '0') * 10 + activeSchedule[10] - '0';
	if (hour > 23 || minute > 59)
		return 0;

	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	time(&now);
	tm = localtime(&now);
	if (!tm)
		return 0;
	current_time = tm->tm_hour * 60 + tm->tm_min;
	current_date = 1 << (6 - tm->tm_wday);
	active = 0;
	Time2Active = hour * 60 + minute;
	Date2Active = 0;

	for (i = 0; i <= 6; i++)
		Date2Active |= (activeSchedule[i] - '0') << (6 - i);

	if ((current_time == Time2Active) && (Date2Active & current_date))
		active = 1;

	//dbG("[watchdog] current_time=%d, ActiveTime=%d, current_date=%d, ActiveDate=%d, active=%d\n",
	//	current_time, Time2Active, current_date, Date2Active, active);

	return active;
}

/* Check for time-dependent service */
static int 
svc_timecheck(void)
{
	int activeNow;

#if BOARD_HAS_5G_RADIO
	if (get_enabled_radio_wl())
	{
		/* Initialize */
		if (nvram_match("reload_svc_wl", "1"))
		{
			nvram_set_int_temp("reload_svc_wl", 0);
			svcStatus[RADIO5_ACTIVE] = -1;
			svcStatus[GUEST5_ACTIVE] = -1;
		}
		
		activeNow = is_radio_allowed_wl();
		if (activeNow != svcStatus[RADIO5_ACTIVE])
		{
			svcStatus[RADIO5_ACTIVE] = activeNow;
			
			if (activeNow)
				notify_rc("control_wifi_radio_wl_on");
			else
				notify_rc("control_wifi_radio_wl_off");
		}
		
		if (svcStatus[RADIO5_ACTIVE] > 0)
		{
			activeNow = is_guest_allowed_wl();
			if (activeNow != svcStatus[GUEST5_ACTIVE])
			{
				svcStatus[GUEST5_ACTIVE] = activeNow;
				
				if (activeNow)
					notify_rc("control_wifi_guest_wl_on");
				else
					notify_rc("control_wifi_guest_wl_off");
			}
		}
		else
		{
			if (svcStatus[GUEST5_ACTIVE] >= 0)
				svcStatus[GUEST5_ACTIVE] = -1;
		}
	}
#endif

	if (get_enabled_radio_rt())
	{
		/* Initialize */
		if (nvram_match("reload_svc_rt", "1"))
		{
			nvram_set_int_temp("reload_svc_rt", 0);
			svcStatus[RADIO2_ACTIVE] = -1;
			svcStatus[GUEST2_ACTIVE] = -1;
		}
		
		activeNow = is_radio_allowed_rt();
		if (activeNow != svcStatus[RADIO2_ACTIVE])
		{
			svcStatus[RADIO2_ACTIVE] = activeNow;
			
			if (activeNow)
				notify_rc("control_wifi_radio_rt_on");
			else
				notify_rc("control_wifi_radio_rt_off");
		}
		
		if (svcStatus[RADIO2_ACTIVE] > 0)
		{
			activeNow = is_guest_allowed_rt();
			if (activeNow != svcStatus[GUEST2_ACTIVE])
			{
				svcStatus[GUEST2_ACTIVE] = activeNow;
				
				if (activeNow)
					notify_rc("control_wifi_guest_rt_on");
				else
					notify_rc("control_wifi_guest_rt_off");
			}
		}
		else
		{
			if (svcStatus[GUEST2_ACTIVE] >= 0)
				svcStatus[GUEST2_ACTIVE] = -1;
		}
	}
	
	char reboot_schedule[PATH_MAX];
	if (nvram_match("reboot_schedule_enable", "1"))
	{
		if (nvram_match("ntp_ready", "1"))
		{
			snprintf(reboot_schedule, sizeof(reboot_schedule), "%s", nvram_safe_get("reboot_schedule"));
			if (strlen(reboot_schedule) == 11)
			{
				if (timecheck_reboot(reboot_schedule))
				{
					logmessage("reboot scheduler", "[%s] The system is going down for reboot\n", __FUNCTION__);
	                sys_exit();
				}
			}
		}
		//else
		//	logmessage("reboot scheduler", "[%s] NTP sync error\n", __FUNCTION__);
	}

	char ss_schedule[PATH_MAX];
	if (nvram_match("ss_schedule_enable", "1"))
	{
		if (nvram_match("ntp_ready", "1"))
		{
			snprintf(ss_schedule, sizeof(ss_schedule), "%s", nvram_safe_get("ss_schedule"));
			if (strlen(ss_schedule) == 11)
			{
				if (timecheck_reboot(ss_schedule))
				{
					//logmessage("reboot scheduler", "[%s] The system is going down for reboot\n", __FUNCTION__);
		            doSystem("/usr/bin/update_dlink.sh %s", "update");
					if (!(wd_pending_events & WD_PENDING_TERMINATE))
						sleep(70);
				}
			}
		}
	}
	return 0;
}

static void
update_svc_status_wifi2()
{
	nvram_set_int_temp("reload_svc_rt", 0);
	svcStatus[RADIO2_ACTIVE] = is_radio_allowed_rt();

	if (svcStatus[RADIO2_ACTIVE] > 0)
		svcStatus[GUEST2_ACTIVE] = is_guest_allowed_rt();
	else
		svcStatus[GUEST2_ACTIVE] = -1;
}

static void
update_svc_status_wifi5()
{
#if BOARD_HAS_5G_RADIO
	nvram_set_int_temp("reload_svc_wl", 0);
	svcStatus[RADIO5_ACTIVE] = is_radio_allowed_wl();

	if (svcStatus[RADIO5_ACTIVE] > 0)
		svcStatus[GUEST5_ACTIVE] = is_guest_allowed_wl();
	else
		svcStatus[GUEST5_ACTIVE] = -1;
#endif
}

/* Sometimes, httpd becomes inaccessible, try to re-run it */
static void httpd_process_check(void)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;
	int scan_error, restart_status;
	int httpd_is_run;

	presence = process_inventory_scan("httpd", &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	action = process_presence_start_action_for_state(presence);
	if (action == PROCESS_PRESENCE_START_ABORT) {
		logmessage("watchdog",
			   "skipping HTTPD recovery because process inventory is unknown (error=%d)",
			   scan_error ? scan_error : EIO);
		return;
	}
	httpd_is_run = (action == PROCESS_PRESENCE_START_NOTIFY);

	if (!httpd_is_run)
		httpd_missing++;
	else
		httpd_missing = 0;

	if (httpd_missing == 1)
		return;

	if ((!httpd_is_run
#ifdef HTTPD_CHECK
	    || !httpd_check_v2()
#endif
	    ) && nvram_match("httpd_started", "1"))
	{
		printf("## restart httpd ##\n");
		httpd_missing = 0;
#ifdef HTTPD_CHECK
		/* Discard the previous probe result before the serialized restart transaction. */
		remove(DETECT_HTTPD_FILE);
#endif
		restart_status = restart_httpd();
		if (restart_status != 0)
			logmessage("watchdog", "HTTPD recovery restart failed (status=%d)", restart_status);
	}
}

/* Sometimes, dnsmasq crashed, try to re-run it */
static void
dnsmasq_process_check(void)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;
	int scan_error, start_status;

	presence = process_inventory_scan("dnsmasq", &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	action = process_presence_start_action_for_state(presence);
	if (action == PROCESS_PRESENCE_START_ABORT) {
		logmessage("watchdog",
			   "skipping DNSMasq recovery because process inventory is unknown (error=%d)",
			   scan_error ? scan_error : EIO);
		return;
	}
	if (action == PROCESS_PRESENCE_START_NEW)
		dnsmasq_missing++;
	else
		dnsmasq_missing = 0;

	if (dnsmasq_missing > 1) {
		dnsmasq_missing = 0;
		logmessage("watchdog", "dnsmasq is missing, start again!");
		start_status = start_dns_dhcpd(0);
		if (start_status != 0)
			logmessage("watchdog",
				   "DNSMasq recovery start failed (status=%d)",
				   start_status);
	}
}

int
ntpc_updated_main(int argc, char *argv[])
{
	char *offset;
	int ntpc_counter, counter_status, ready_status;

	if (argc < 2)
		return -1;

	if (strcmp(argv[1], "step") != 0)
		return 0;

	offset = getenv("offset");
	if (!watchdog_ntp_step_has_offset(offset))
		return 0;

	ntpc_counter = nvram_get_int("ntpc_counter");
	counter_status = nvram_set_int_temp("ntpc_counter", ntpc_counter + 1);
	if (counter_status != 0)
		logmessage("NTP Client",
			   "cannot record NTP step counter (status=%d)",
			   counter_status);

#if defined (USE_RTC_HCTOSYS)
	/* update current system time to RTC chip */
	system("hwclock -w");
#endif
	logmessage("NTP Client", "System time changed, offset: %ss", offset);
	sleep(5);
	ready_status = nvram_set_int("ntp_ready", 1);
	if (ready_status != 0)
		logmessage("NTP Client",
			   "cannot persist NTP readiness (status=%d)",
			   ready_status);

	return ready_status;
}

int
ntpc_syncnow_main(int argc, char *argv[])
{
	return refresh_ntp();
}

static void
watchdog_on_sighup(void)
{
	setenv_tz();

	if (!is_ntpc_updated()) {
		ntpc_tries = 0;
		ntpc_timer = -1; // want call now
	}
}

static void
watchdog_on_sigusr1(void)
{
	int wd_notify_id = nvram_get_int("wd_notify_id");
	if (wd_notify_id == WD_NOTIFY_ID_WIFI2) {
		update_svc_status_wifi2();
	} else if (wd_notify_id == WD_NOTIFY_ID_WIFI5) {
		update_svc_status_wifi5();
	}
}

static void
watchdog_on_timer(void)
{
	int is_ap_mode;

	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;
	is_ap_mode = get_ap_mode();

	/* Retry readiness persistence even while WAN connectivity is down. */
	ntpc_ready_handler();

	/* check for time-dependent services */
	svc_timecheck();
	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;

	/* http server check */
	httpd_process_check();
	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;

	/* DNS/DHCP server check */
	if (!is_ap_mode)
		dnsmasq_process_check();
	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;

	inet_handler(is_ap_mode);
	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;

	/* update kernel timezone daylight */
	setkernel_tz();
	if (wd_pending_events & WD_PENDING_TERMINATE)
		return;

	storage_save_time(10);
}

static void
catch_sig_watchdog(int sig)
{
	watchdog_record_signal(&wd_pending_events, sig);
}

static int
watchdog_start_probe_wait(unsigned int seconds)
{
	int status;

	if (service_wait_seconds(seconds) == 0)
		return 0;
	status = errno > 0 ? errno : EIO;
	errno = status;
	logmessage("watchdog", "watchdog startup probe wait failed (status=%d)",
		   status);
	return status;
}

static int
watchdog_start_rollback_stop(void *context)
{
	char *service_names[2];

	if (!context) {
		errno = EINVAL;
		return EINVAL;
	}
	service_names[0] = (char *)context;
	service_names[1] = NULL;
	return kill_services(service_names, 3, 1);
}

static enum process_presence
watchdog_start_rollback_scan(void *context, int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;

	if (!context) {
		if (scan_error)
			*scan_error = EINVAL;
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	presence = process_inventory_scan((const char *)context, &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return presence;
}

static int
watchdog_rollback_failed_start(int start_status, int start_errno,
			       const char *operation)
{
	int stop_status = 0;
	int scan_error = 0;
	int saved_errno;
	int result;
	enum process_presence final_state = PROCESS_PRESENCE_UNKNOWN;

	if (start_status == 0)
		start_status = -1;
	if (start_errno <= 0)
		start_errno = start_status > 0 ? start_status : EIO;
	result = detached_daemon_start_rollback_with_ops(
		start_status, start_errno, watchdog_start_rollback_stop,
		watchdog_start_rollback_scan, (void *)"watchdog",
		&stop_status, &final_state, &scan_error);
	saved_errno = errno;
	if (final_state == PROCESS_PRESENCE_ABSENT && stop_status == 0) {
		logmessage("watchdog",
			   "startup rollback after %s confirmed watchdog absent",
			   operation);
	} else {
		logmessage("watchdog",
			   "startup rollback after %s incomplete (primary_status=%d, stop_status=%d, process_state=%d, scan_error=%d)",
			   operation, start_status, stop_status, final_state, scan_error);
	}
	errno = saved_errno;
	return result;
}
int
start_watchdog(void)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	enum process_presence_start_action action;
	int scan_error, launch_status, launch_errno, wait_status, wait_errno, attempt;

	presence = process_inventory_scan("watchdog", &snapshot);
	scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	action = process_presence_start_action_for_state(presence);
	if (action == PROCESS_PRESENCE_START_ABORT) {
		errno = scan_error ? scan_error : EIO;
		logmessage("watchdog",
			   "refusing watchdog start because process inventory is unknown (error=%d)",
			   errno);
		return -1;
	}
	if (action == PROCESS_PRESENCE_START_NOTIFY)
		return 0;
	if (action == PROCESS_PRESENCE_START_NEW) {
		/*
		 * watchdog_main() calls daemon() before publishing its PID file and
		 * arming the timer. eval() only waits for the exiting launcher; it
		 * cannot prove the detached child survived initialization.
		 */
		errno = 0;
		launch_status = eval("/sbin/watchdog");
		if (launch_status != 0) {
			launch_errno = errno > 0 ? errno :
				(launch_status > 0 ? launch_status : EIO);
			logmessage("watchdog", "watchdog launcher failed (status=%d)",
				   launch_status);
			return watchdog_rollback_failed_start(
				launch_status, launch_errno, "launcher");
		}

		wait_status = watchdog_start_probe_wait(
			PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
		if (wait_status != 0) {
			wait_errno = errno > 0 ? errno : wait_status;
			return watchdog_rollback_failed_start(
				wait_status, wait_errno, "initial readiness wait");
		}
		for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
			presence = process_inventory_scan("watchdog", &snapshot);
			if (process_presence_start_postcondition(launch_status, presence) == 0) {
				process_snapshot_release(&snapshot);
				return 0;
			}
			if (presence == PROCESS_PRESENCE_UNKNOWN) {
				scan_error = snapshot.error > 0 ? snapshot.error : EIO;
				process_snapshot_release(&snapshot);
				logmessage("watchdog",
					   "cannot verify detached watchdog startup (errno=%d)",
					   scan_error);
				return watchdog_rollback_failed_start(
					-1, scan_error, "process inventory");
			}
			if (presence != PROCESS_PRESENCE_ABSENT) {
				process_snapshot_release(&snapshot);
				logmessage("watchdog",
					   "invalid process-presence state while verifying watchdog startup");
				return watchdog_rollback_failed_start(
					-1, EIO, "invalid process inventory state");
			}
			process_snapshot_release(&snapshot);
			if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
				wait_status = watchdog_start_probe_wait(
					PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
				if (wait_status != 0) {
					wait_errno = errno > 0 ? errno : wait_status;
					return watchdog_rollback_failed_start(
						wait_status, wait_errno, "retry readiness wait");
				}
			}
		}

		logmessage("watchdog", "watchdog daemon did not remain running after launch");
		return watchdog_rollback_failed_start(
			-1, ETIMEDOUT, "readiness timeout");
	}

	/* Only a confirmed ABSENT inventory may authorize a new daemon. */
	errno = EIO;
	logmessage("watchdog",
		   "refusing watchdog start because process-presence policy is invalid");
	return -1;
}
int
notify_watchdog_time(void)
{
	int signal_status, signal_errno;
	int start_status = 0, start_errno = 0;
	int operation_error;

	errno = 0;
	signal_status = process_pidfile_signal(WD_PID_FILE, "watchdog", SIGHUP);
	signal_errno = errno;
	if (signal_status != 0 &&
	    (signal_errno == ENOENT || signal_errno == ESRCH)) {
		errno = 0;
		start_status = start_watchdog();
		start_errno = errno;
		if (start_status != 0)
			logmessage("watchdog", "cannot recover time-refresh notification by starting daemon (status=%d, errno=%d)",
				   start_status, start_errno ? start_errno : EIO);
	} else if (signal_status != 0) {
		logmessage("watchdog", "cannot signal time refresh (status=%d, errno=%d)",
			   signal_status, signal_errno ? signal_errno : EIO);
	}

	operation_error = watchdog_time_notification_error(
		signal_status, signal_errno, start_status, start_errno);
	return operation_error;
}

void
notify_watchdog_wifi(int is_5ghz)
{
	int wd_notify_id = (is_5ghz) ? WD_NOTIFY_ID_WIFI5 : WD_NOTIFY_ID_WIFI2;
	int saved_errno;

	nvram_set_int_temp("wd_notify_id", wd_notify_id);
	if (process_pidfile_signal(WD_PID_FILE, "watchdog", SIGUSR1) < 0) {
		saved_errno = errno;
		if (saved_errno != ENOENT && saved_errno != ESRCH)
			logmessage("watchdog", "cannot signal Wi-Fi update (errno=%d)", saved_errno);
	}
}

int
watchdog_main(int argc, char *argv[])
{
	int saved_errno = 0;
	sig_atomic_t events;
	sigset_t managed_signals, original_mask, dispatch_mask;
	struct sigaction sa;

	(void)argc;
	(void)argv;

	sigemptyset(&managed_signals);
	sigaddset(&managed_signals, SIGALRM);
	sigaddset(&managed_signals, SIGHUP);
	sigaddset(&managed_signals, SIGUSR1);
	sigaddset(&managed_signals, SIGTERM);

	/* Block managed signals before installing handlers or publishing state. */
	if (sigprocmask(SIG_BLOCK, &managed_signals, &original_mask) < 0) {
		saved_errno = errno;
		errno = saved_errno;
		perror("sigprocmask");
		return saved_errno;
	}

	wd_pending_events = 0;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_watchdog;
	sa.sa_mask = managed_signals;
	if (sigaction(SIGHUP, &sa, NULL) < 0 ||
	    sigaction(SIGUSR1, &sa, NULL) < 0 ||
	    sigaction(SIGTERM, &sa, NULL) < 0 ||
	    sigaction(SIGALRM, &sa, NULL) < 0) {
		saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
		errno = saved_errno;
		perror("sigaction");
		return saved_errno;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
		errno = saved_errno;
		perror("sigaction");
		return saved_errno;
	}

	/* Preserve unrelated inherited mask bits while making watchdog signals live. */
	dispatch_mask = original_mask;
	sigdelset(&dispatch_mask, SIGALRM);
	sigdelset(&dispatch_mask, SIGHUP);
	sigdelset(&dispatch_mask, SIGUSR1);
	sigdelset(&dispatch_mask, SIGTERM);

	if (daemon(0, 0) < 0) {
		saved_errno = errno;
		(void)sigprocmask(SIG_SETMASK, &original_mask, NULL);
		errno = saved_errno;
		perror("daemon");
		return saved_errno;
	}

	/* never invoke oom killer */
	oom_score_adjust(getpid(), OOM_SCORE_ADJ_MIN);

	/* Publish PID + start_time atomically; do not run without identity state. */
	if (process_pidfile_write(WD_PID_FILE, "watchdog") < 0) {
		saved_errno = errno;
		goto watchdog_cleanup;
	}

	nvram_set_int_temp("wd_notify_id", 0);

	/* set timer */
	if (wd_alarmtimer(WD_NORMAL_PERIOD, 0) < 0) {
		saved_errno = errno;
		goto watchdog_cleanup;
	}

	/*
	 * Signals wake this loop but never perform watchdog work themselves.
	 * The blocked snapshot plus sigsuspend closes the check/wait race.
	 */
	while (1) {
		events = watchdog_take_pending_events(&wd_pending_events);
		if (events & WD_PENDING_TERMINATE)
			break;

		if (events == 0) {
			if (sigsuspend(&dispatch_mask) < 0 && errno != EINTR) {
				saved_errno = errno;
				break;
			}
			continue;
		}

		/* Work runs in ordinary context; handlers may only queue more events. */
		if (sigprocmask(SIG_SETMASK, &dispatch_mask, NULL) < 0) {
			saved_errno = errno;
			break;
		}

		/* Apply control updates before a timer poll in the same batch. */
		if (events & WD_PENDING_TIME_REFRESH)
			watchdog_on_sighup();
		if (events & WD_PENDING_WIFI_STATE)
			watchdog_on_sigusr1();
		if (events & WD_PENDING_TIMER)
			watchdog_on_timer();

		if (sigprocmask(SIG_BLOCK, &managed_signals, NULL) < 0) {
			saved_errno = errno;
			break;
		}
	}

watchdog_cleanup:
	(void)wd_alarmtimer(0, 0);
	/*
	 * Keep the last identity record. Unlinking by pathname can delete a
	 * replacement watchdog's freshly published record after a fast restart;
	 * stale PID/start_time pairs are rejected by process_pidfile_signal().
	 */
	if (sigprocmask(SIG_SETMASK, &original_mask, NULL) < 0 && saved_errno == 0)
		saved_errno = errno;

	if (saved_errno != 0) {
		errno = saved_errno;
		perror("watchdog");
		return saved_errno;
	}

	return 0;
}