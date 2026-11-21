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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rc.h"
#include "nvram_linux.h"
#include "process_inventory.h"
#include "optional_daemon_policy.h"
#include "process_presence_start_policy.h"
#include "detached_daemon_start_policy.h"
#include "logger_start_policy.h"
#include "service_start_status.h"
#include "watchdog_cpu_restart_policy.h"
#include "wan_nvram_status_policy.h"
#include "firewall_nvram_match_policy.h"
#include "service_wait_policy.h"

static int optional_daemon_start_status(const char *service_name, const char *process_name);
static int service_enabled_mode_status(const char *service_name, const char *nvram_key, int *enabled);
static int stop_httpd_locked(void);
static int optional_daemon_stop_status(const char *service_name, const char *process_name, int command_status);
#include "sshd_daemon_policy.h"
#include "networkmap_daemon_policy.h"
#include "crond_daemon_policy.h"
#include "infosvr_daemon_policy.h"
#include "httpd_daemon_policy.h"
#include "httpd_lifecycle_lock.h"


static int
service_start_probe_wait(const char *service_name, unsigned int seconds)
{
	int status;

	if (service_wait_seconds(seconds) == 0)
		return 0;
	status = errno > 0 ? errno : EIO;
	logmessage(LOGNAME, "%s startup probe wait failed (errno=%d)",
		   service_name, status);
	return service_wait_failure_status(status);
}

static int
service_nvram_match_status(const char *param_name, const char *expected,
			   int missing_match, int *matches)
{
	char *text = NULL;
	int api_result, saved_errno;

	if (!param_name || !param_name[0] || !expected || !matches)
		return EINVAL;

	errno = 0;
	api_result = nvram_get_checked(param_name, &text);
	saved_errno = errno;
	return firewall_nvram_match_status(api_result, saved_errno, text,
				   expected, missing_match, matches);
}

static void
record_service_stop_status(int *first_status, const char *service_name, int status)
{
	if (status == 0)
		return;
	logmessage(LOGNAME, "%s stop failed (status=%d)", service_name, status);
	if (*first_status == 0)
		*first_status = status;
}

static enum process_presence
optional_daemon_presence(const char *process_name, int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	int saved_error;

	presence = process_inventory_scan(process_name, &snapshot);
	saved_error = snapshot.error > 0 ? snapshot.error : EIO;
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (presence == PROCESS_PRESENCE_UNKNOWN)
		errno = saved_error;
	return presence;
}

static int
optional_daemon_start_status(const char *service_name, const char *process_name)
{
	enum process_presence presence;
	int scan_error = 0;

	presence = optional_daemon_presence(process_name, &scan_error);
	if (optional_daemon_start_allowed(presence))
		return 0;
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 1;

	errno = scan_error > 0 ? scan_error : EIO;
	logmessage(LOGNAME, "%s start aborted: process inventory unknown (errno=%d)",
		   service_name, errno);
	return -1;
}

static int
optional_daemon_stop_status(const char *service_name, const char *process_name,
			    int command_status)
{
	enum process_presence presence;
	int scan_error = 0;
	int result;

	presence = optional_daemon_presence(process_name, &scan_error);
	result = optional_daemon_stop_complete(command_status, presence);
	if (result != 0) {
		if (command_status == 0) {
			if (presence == PROCESS_PRESENCE_UNKNOWN)
				errno = scan_error > 0 ? scan_error : EIO;
			else if (presence == PROCESS_PRESENCE_RUNNING)
				errno = EBUSY;
			else
				errno = EIO;
		}
		logmessage(LOGNAME, "%s stop incomplete (command=%d, process_state=%d, scan_error=%d)",
			   service_name, command_status, presence, scan_error);
	}
	return result;
}

static int
service_enabled_mode_status(const char *service_name, const char *nvram_key,
			    int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, configured_mode = 0;
	int read_status, status;

	if (!service_name || !service_name[0] ||
	    !nvram_key || !nvram_key[0] || !enabled)
		return EINVAL;

	errno = 0;
	api_result = nvram_get_checked(nvram_key, &value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_int_status(
		api_result, saved_errno, value, 0, &configured_mode);
	status = rc_service_enabled_mode_decide(
		read_status, configured_mode, enabled);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "%s enable-setting read failed; preserving existing daemon state (status=%d)",
			   service_name, status);
	}
	return status;
}


static enum process_presence
optional_daemon_rollback_scan(void *context, int *scan_error)
{
	if (!context) {
		if (scan_error)
			*scan_error = EINVAL;
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	return optional_daemon_presence((const char *)context, scan_error);
}

static int
optional_daemon_rollback_stop(void *context)
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

static int
optional_daemon_rollback_failed_start(const char *service_name,
				      int start_status, int start_errno)
{
	int stop_status = 0;
	int scan_error = 0;
	int saved_errno;
	int result;
	enum process_presence final_state = PROCESS_PRESENCE_UNKNOWN;

	result = detached_daemon_start_rollback_with_ops(
		start_status, start_errno, optional_daemon_rollback_stop,
		optional_daemon_rollback_scan, (void *)service_name,
		&stop_status, &final_state, &scan_error);
	saved_errno = errno;
	if (final_state == PROCESS_PRESENCE_ABSENT) {
		logmessage(LOGNAME,
			   "%s startup rollback confirmed process absent (stop_status=%d)",
			   service_name, stop_status);
	} else {
		logmessage(LOGNAME,
			   "%s startup rollback incomplete (primary_status=%d, stop_status=%d, process_state=%d, scan_error=%d)",
			   service_name, start_status, stop_status, final_state, scan_error);
	}
	errno = saved_errno;
	return result;
}

void
stop_syslogd(void)
{
	char* svcs[] = { "syslogd", NULL };
	kill_services(svcs, 3, 1);
}

void
stop_klogd(void)
{
	char* svcs[] = { "klogd", NULL };
	kill_services(svcs, 3, 1);
}

int
start_syslogd(void)
{
	char *log_ipaddr, log_rot[8], host_dst[32];
	char *syslogd_argv[] = {
		"/sbin/syslogd",
		log_rot,			/* max size before rotation */
		"-b0",				/* purge on rotate */
		"-S",				/* smaller output */
		"-D",				/* drop duplicates */
		"-O", "/tmp/syslog.log",	/* syslog file */
		NULL,				/* -L */
		NULL, NULL,			/* -R host:port */
		NULL
	};
	int start_status, attempt, scan_error, start_errno;
	enum process_presence process_state;

	start_status = optional_daemon_start_status("syslogd", "syslogd");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	snprintf(log_rot, sizeof(log_rot), "-s%d", LOG_ROTATE_SIZE_MAX);

	log_ipaddr = nvram_safe_get("log_ipaddr");
	if (is_valid_ipv4(log_ipaddr)) {
		int log_port = nvram_safe_get_int("log_port", 514, 1, 65535);
		snprintf(host_dst, sizeof(host_dst), "%s:%d", log_ipaddr, log_port);
		syslogd_argv[7] = "-L";		/* local & remote */
		syslogd_argv[8] = "-R";
		syslogd_argv[9] = host_dst;
	}

	setenv_tz();

	/*
	 * BusyBox syslogd daemonizes unless -n is supplied. _eval() waits for
	 * that short-lived launcher, not for the detached logging child.
	 */
	errno = 0;
	start_status = _eval(syslogd_argv, NULL, 0, NULL);
	if (start_status != 0) {
		logmessage(LOGNAME, "syslogd launcher failed (status=%d)", start_status);
		start_errno = errno;
		return optional_daemon_rollback_failed_start("syslogd", start_status, start_errno);
	}

	start_status = service_start_probe_wait("syslogd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0) {
		start_errno = errno;
		return optional_daemon_rollback_failed_start("syslogd", start_status, start_errno);
	}
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("syslogd", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify syslogd startup (scan_error=%d)",
				   errno);
			start_errno = errno;
			return optional_daemon_rollback_failed_start("syslogd", -1, start_errno);
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("syslogd", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0) {
				start_errno = errno;
				return optional_daemon_rollback_failed_start("syslogd", start_status, start_errno);
			}
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "syslogd did not remain running after launch");
	return optional_daemon_rollback_failed_start("syslogd", -1, ETIMEDOUT);
}

int
start_klogd(void)
{
	int start_status, attempt, scan_error, start_errno;
	enum process_presence process_state;

	start_status = optional_daemon_start_status("klogd", "klogd");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	/*
	 * BusyBox klogd daemonizes by default before opening the kernel log.
	 * Verify the detached reader, not only the short-lived launcher.
	 */
	errno = 0;
	start_status = eval("/sbin/klogd");
	if (start_status != 0) {
		logmessage(LOGNAME, "klogd launcher failed (status=%d)", start_status);
		start_errno = errno;
		return optional_daemon_rollback_failed_start("klogd", start_status, start_errno);
	}

	start_status = service_start_probe_wait("klogd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0) {
		start_errno = errno;
		return optional_daemon_rollback_failed_start("klogd", start_status, start_errno);
	}
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("klogd", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify klogd startup (scan_error=%d)",
				   errno);
			start_errno = errno;
			return optional_daemon_rollback_failed_start("klogd", -1, start_errno);
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("klogd", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0) {
				start_errno = errno;
				return optional_daemon_rollback_failed_start("klogd", start_status, start_errno);
			}
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "klogd did not remain running after launch");
	return optional_daemon_rollback_failed_start("klogd", -1, ETIMEDOUT);
}

int
stop_infosvr(void)
{
	char* svcs[] = { "infosvr", NULL };
	return kill_services(svcs, 3, 1);
}

int
start_infosvr(void)
{
	int start_status, attempt, scan_error, start_errno;
	enum process_presence process_state;

	if (nvram_invmatch("adsc_enable", "1"))
		return 0;

	start_status = optional_daemon_start_status("infosvr", "infosvr");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	/*
	 * infosvr calls daemon() before entering its packet loop. eval() only
	 * observes the daemonizing parent, so confirm the detached child remains
	 * visible before reporting a successful service start to PID 1.
	 */
	errno = 0;
	start_status = eval("/usr/sbin/infosvr", IFNAME_BR);
	if (start_status != 0) {
		logmessage("infosvr", "infosvr launch failed (status=%d)",
			   start_status);
		if (start_status > 0)
			errno = EIO;
		start_errno = errno;
		return optional_daemon_rollback_failed_start("infosvr", -1, start_errno);
	}

	start_status = service_start_probe_wait("infosvr", INFOSVR_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0) {
		start_errno = errno;
		return optional_daemon_rollback_failed_start("infosvr", start_status, start_errno);
	}
	for (attempt = 0; attempt < INFOSVR_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("infosvr", &scan_error);
		if (infosvr_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage("infosvr", "cannot verify daemon startup (scan_error=%d)",
				   errno);
			start_errno = errno;
			return optional_daemon_rollback_failed_start("infosvr", -1, start_errno);
		}
		if (attempt + 1 < INFOSVR_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("infosvr", INFOSVR_START_RETRY_WAIT_SECONDS);
			if (start_status != 0) {
				start_errno = errno;
				return optional_daemon_rollback_failed_start("infosvr", start_status, start_errno);
			}
		}
	}

	errno = ETIMEDOUT;
	logmessage("infosvr", "daemon did not remain running after launch");
	return optional_daemon_rollback_failed_start("infosvr", -1, ETIMEDOUT);
}

int
restart_infosvr(void)
{
	struct process_snapshot snapshot;
	enum process_presence process_state;
	int stop_status = stop_infosvr();
	int scan_error;

	process_state = process_inventory_scan("infosvr", &snapshot);
	scan_error = snapshot.error;
	if (!infosvr_restart_can_start(stop_status, process_state)) {
		process_snapshot_release(&snapshot);
		logmessage(LOGNAME, "refusing infosvr restart: previous daemon absence is unconfirmed (status=%d, state=%d, scan_error=%d)",
			   stop_status, process_state, scan_error);
		if (stop_status != 0)
			return stop_status;

		/* PID 1 treats negative action_result as a failed notification. */
		if (process_state == PROCESS_PRESENCE_UNKNOWN)
			errno = scan_error > 0 ? scan_error : EIO;
		else
			errno = EBUSY;
		return -1;
	}
	process_snapshot_release(&snapshot);

	return start_infosvr();
}

int
stop_crond(void)
{
	char* svcs[] = { "crond", NULL };
	return kill_services(svcs, 3, 1);
}

static enum process_presence
crond_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;

	presence = process_inventory_scan("crond", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return presence;
}

int
start_crond(void)
{
	char *crond_argv[] = {
		"/usr/sbin/crond",
		NULL,			/* -d8 */
		NULL
	};
	int launch_status, attempt, status, scan_error, preflight_status, launch_errno;
	enum process_presence presence;

	if (nvram_invmatch("crond_enable", "1"))
		return 0;

	/* Do not spawn a second scheduler when start is requested again. */
	preflight_status = optional_daemon_start_status("crond", "crond");
	if (preflight_status > 0) {
		logmessage(LOGNAME, "crond start skipped: scheduler is already running");
		return 0;
	}
	if (preflight_status < 0)
		return preflight_status;

	if (nvram_match("crond_log", "0"))
		crond_argv[1] = "-d8";

	setenv_tz();

	/*
	 * BusyBox crond exits its launcher successfully after forking. The
	 * detached child still has to enter the spool directory, scan jobs
	 * and publish its pidfile, so _eval() alone is not a daemon postcondition.
	 */
	errno = 0;
	launch_status = _eval(crond_argv, NULL, 0, NULL);
	if (launch_status != 0) {
		logmessage(LOGNAME, "crond launcher failed: %d", launch_status);
		launch_errno = errno;
		return optional_daemon_rollback_failed_start("crond", -1, launch_errno);
	}

	status = service_start_probe_wait("crond", CROND_START_INITIAL_WAIT_SECONDS);
	if (status != 0) {
		launch_errno = errno;
		return optional_daemon_rollback_failed_start("crond", status, launch_errno);
	}
	status = -1;
	for (attempt = 0; attempt < CROND_START_PROBE_ATTEMPTS; attempt++) {
		presence = crond_process_presence(&scan_error);
		status = crond_start_postcondition(launch_status, presence);
		if (presence == PROCESS_PRESENCE_RUNNING)
			return 0;
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME,
				   "cannot verify crond startup postcondition (scan_error=%d)",
				   scan_error ? scan_error : EIO);
			return optional_daemon_rollback_failed_start("crond", status, errno);
		}

		if (attempt + 1 < CROND_START_PROBE_ATTEMPTS) {
			status = service_start_probe_wait("crond", CROND_START_RETRY_WAIT_SECONDS);
			if (status != 0) {
				launch_errno = errno;
				return optional_daemon_rollback_failed_start("crond", status, launch_errno);
			}
		}
	}

	logmessage(LOGNAME, "crond daemon did not remain running after launch");
	errno = ETIMEDOUT;
	return optional_daemon_rollback_failed_start("crond", -1, ETIMEDOUT);
}

int
restart_crond(void)
{
	int stop_status = stop_crond();

	if (!crond_restart_can_start(stop_status)) {
		logmessage(LOGNAME, "refusing crond restart: previous daemon did not stop");
		return stop_status;
	}

	return start_crond();
}

static enum process_presence
networkmap_validate_identity(void *context,
			     const struct process_identity *identity)
{
	(void)context;
	return process_identity_validate("networkmap", identity);
}

static int
networkmap_send_signal(void *context, pid_t pid, int sig)
{
	(void)context;
	return kill(pid, sig);
}

int
start_networkmap(int first_call)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	int scan_error, launch_status, attempt, launch_errno;

	state = process_inventory_scan("networkmap", &snapshot);
	if (state == PROCESS_PRESENCE_RUNNING) {
		process_snapshot_release(&snapshot);
		return 0;
	}
	if (!networkmap_start_allowed(state)) {
		scan_error = snapshot.error > 0 ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = scan_error;
		logmessage(LOGNAME, "networkmap start aborted: process inventory unknown (errno=%d)",
			   scan_error);
		return -1;
	}
	process_snapshot_release(&snapshot);

	/*
	 * networkmap calls daemon(0, 0) before opening its pidfile and entering
	 * the scan loop. eval() observes the parent; verify the detached child.
	 */
	errno = 0;
	launch_status = eval("/usr/sbin/networkmap", (first_call) ? "-w" : "");
	if (launch_status != 0) {
		logmessage(LOGNAME, "networkmap launch failed (status=%d)", launch_status);
		launch_errno = errno;
		return optional_daemon_rollback_failed_start("networkmap", launch_status, launch_errno);
	}

	launch_status = service_start_probe_wait("networkmap", NETWORKMAP_START_INITIAL_WAIT_SECONDS);
	if (launch_status != 0) {
		launch_errno = errno;
		return optional_daemon_rollback_failed_start("networkmap", launch_status, launch_errno);
	}
	for (attempt = 0; attempt < NETWORKMAP_START_PROBE_ATTEMPTS; attempt++) {
		state = process_inventory_scan("networkmap", &snapshot);
		if (networkmap_start_postcondition(launch_status, state) == 0) {
			process_snapshot_release(&snapshot);
			return 0;
		}
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			scan_error = snapshot.error > 0 ? snapshot.error : EIO;
			process_snapshot_release(&snapshot);
			errno = scan_error;
			logmessage(LOGNAME, "networkmap startup inventory unknown (errno=%d)",
				   scan_error);
			launch_errno = scan_error;
			return optional_daemon_rollback_failed_start("networkmap", -1, launch_errno);
		}
		process_snapshot_release(&snapshot);
		if (attempt + 1 < NETWORKMAP_START_PROBE_ATTEMPTS) {
			launch_status = service_start_probe_wait("networkmap", NETWORKMAP_START_RETRY_WAIT_SECONDS);
			if (launch_status != 0) {
				launch_errno = errno;
				return optional_daemon_rollback_failed_start("networkmap", launch_status, launch_errno);
			}
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "networkmap daemon did not remain running after launch");
	return optional_daemon_rollback_failed_start("networkmap", -1, ETIMEDOUT);
}

int
stop_networkmap(void)
{
	char* svcs[] = { "networkmap", NULL };
	return kill_services(svcs, 3, 1);
}

int
restart_networkmap(void)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	enum networkmap_daemon_action action;
	int scan_error;
	int result;
	int saved_errno;

	state = process_inventory_scan("networkmap", &snapshot);
	action = networkmap_action_for_presence(state);
	if (action == NETWORKMAP_DAEMON_ABORT) {
		scan_error = snapshot.error > 0 ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = scan_error;
		logmessage(LOGNAME, "networkmap restart aborted: process inventory unknown (errno=%d)",
			   scan_error);
		return -1;
	}
	if (action == NETWORKMAP_DAEMON_START) {
		process_snapshot_release(&snapshot);
		return start_networkmap(0);
	}

	result = networkmap_signal_snapshot(&snapshot, SIGUSR1,
					    networkmap_validate_identity,
					    networkmap_send_signal, NULL);
	saved_errno = errno;
	process_snapshot_release(&snapshot);
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		logmessage(LOGNAME, "networkmap refresh signal failed (errno=%d)", errno);
		return -1;
	}

	return 0;
}

int
stop_telnetd(void)
{
	char* svcs[] = { "telnetd", NULL };
	return optional_daemon_stop_status("telnetd", "telnetd", kill_services(svcs, 3, 1));
}

int
run_telnetd(void)
{
	int stop_status = stop_telnetd();

	if (stop_status != 0) {
		logmessage(LOGNAME, "telnetd manual restart aborted because stop failed (status=%d)",
			   stop_status);
		return stop_status;
	}

	return start_telnetd();
}

int
start_telnetd(void)
{
	int start_status, attempt, scan_error;
	enum process_presence process_state;

	if (!nvram_match("telnetd", "1"))
		return 0;

	start_status = optional_daemon_start_status("telnetd", "telnetd");
	if (start_status > 0)
		return 0;
	if (start_status < 0) {
		logmessage(LOGNAME, "telnetd start aborted: process inventory unavailable (status=%d)",
			   start_status);
		return start_status;
	}

	/*
	 * BusyBox telnetd daemonizes before binding/listening. eval() only observes
	 * the parent, so a bind or post-fork setup failure must not be reported as
	 * a successful remote-terminal start.
	 */
	start_status = eval("telnetd");
	if (start_status != 0) {
		logmessage(LOGNAME, "telnetd launcher failed (status=%d)", start_status);
		return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
	}

	start_status = service_start_probe_wait("telnetd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0)
		return start_status;
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("telnetd", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify telnetd startup (scan_error=%d)",
				   errno);
			return -1;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("telnetd", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0)
				return start_status;
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "telnetd daemon did not remain running after launch");
	return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
}

#if defined(APP_SSHD)
static enum process_presence
sshd_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	const char *process_name = sshd_process_name_for_binaries(
		check_if_file_exist("/usr/bin/dropbearmulti"),
		check_if_file_exist("/usr/sbin/sshd"));

	if (!process_name) {
		if (scan_error)
			*scan_error = 0;
		return PROCESS_PRESENCE_ABSENT;
	}

	presence = process_inventory_scan(process_name, &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	if (presence != PROCESS_PRESENCE_ABSENT &&
	    presence != PROCESS_PRESENCE_RUNNING) {
		presence = PROCESS_PRESENCE_UNKNOWN;
		if (scan_error && *scan_error <= 0)
			*scan_error = EIO;
	}
	return presence;
}

int is_sshd_run(void)
{
	return sshd_process_presence(NULL) == PROCESS_PRESENCE_RUNNING;
}

int stop_sshd(void)
{
	char *service_names[2];
	const char *process_name = sshd_process_name_for_binaries(
		check_if_file_exist("/usr/bin/dropbearmulti"),
		check_if_file_exist("/usr/sbin/sshd"));

	if (!process_name)
		return 0;

	service_names[0] = (char *)process_name;
	service_names[1] = NULL;
	/*
	 * kill_services() snapshots PID/start-time identities, validates them
	 * before TERM/KILL, and returns success only after a fresh scan confirms
	 * the selected daemon is absent.
	 */
	return kill_services(service_names, 3, 1);
}

static int
sshd_enabled_mode_read(int *sshd_mode)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!sshd_mode)
		return EINVAL;
	errno = 0;
	api_result = nvram_get_checked("sshd_enable", &value);
	saved_errno = errno;
	status = sshd_nvram_mode_status(api_result, saved_errno,
					value, sshd_mode);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read SSH enable configuration; preserving current daemon state (errno=%d)",
			   status);
		errno = status;
		return status;
	}
	return 0;
}

static int
sshd_start_failure_recover(int failure_status, const char *phase)
{
	int failure_errno = errno > 0 ? errno : EIO;
	int stop_status;
	int scan_error = 0;
	enum process_presence after_stop;

	/*
	 * The launcher may have forked the daemon before reporting an error, and
	 * a failed wait/procfs scan does not prove the listener is absent.
	 */
	stop_status = stop_sshd();
	after_stop = sshd_process_presence(&scan_error);
	if (sshd_failed_start_preserve_running_state(after_stop)) {
		logmessage(LOGNAME,
			   "SSH daemon may remain active after %s startup failure (stop_status=%d, state=%d, scan_error=%d)",
			   phase, stop_status, after_stop, scan_error);
	} else if (stop_status != 0) {
		logmessage(LOGNAME,
			   "SSH daemon confirmed absent after %s startup failure but stop reported status=%d",
			   phase, stop_status);
	}

	errno = failure_errno;
	return failure_status;
}

static int
start_sshd_with_mode(int sshd_mode)
{
	enum process_presence state;
	int scan_error = 0;
	int start_status;
	int attempt;

	if (sshd_mode != 1 && sshd_mode != 2)
		return 0;

	state = sshd_process_presence(&scan_error);
	if (state == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (!sshd_start_allowed(state)) {
		errno = scan_error > 0 ? scan_error : EIO;
		logmessage(LOGNAME, "SSH start aborted: process inventory unknown (errno=%d)",
			   errno);
		return -1;
	}

	if (sshd_mode == 2)
		start_status = eval("/usr/bin/sshd.sh", "start", "-s");
	else
		start_status = eval("/usr/bin/sshd.sh", "start");
	if (start_status != 0) {
		logmessage(LOGNAME, "SSH launcher failed (status=%d)", start_status);
		return sshd_start_failure_recover(start_status, "launcher");
	}

	/*
	 * OpenSSH and Dropbear detach from the RC launcher. A successful eval()
	 * only proves that the wrapper returned; require the child to remain
	 * visible before reporting a successful start or restart.
	 */
	start_status = service_start_probe_wait("SSH", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0)
		return sshd_start_failure_recover(start_status, "initial startup");
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		state = sshd_process_presence(&scan_error);
		if (sshd_start_postcondition(start_status, state) == 0)
			return 0;
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify SSH startup (scan_error=%d)",
				   errno);
			return sshd_start_failure_recover(-1, "process inventory");
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("SSH", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0)
				return sshd_start_failure_recover(start_status, "retry");
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "SSH daemon did not remain running after launch");
	return sshd_start_postcondition(start_status, PROCESS_PRESENCE_ABSENT);
}

int start_sshd(void)
{
	int sshd_mode, config_status;

	config_status = sshd_enabled_mode_read(&sshd_mode);
	if (config_status != 0)
		return -1;
	return start_sshd_with_mode(sshd_mode);
}

int restart_sshd(void)
{
	enum process_presence before_state;
	enum process_presence stopped_state;
	enum process_presence after_state;
	int scan_error;
	int stop_status;
	int start_status;
	int firewall_status = 0;
	int firewall_errno = 0;
	int firewall_refresh_needed = 0;
	int sshd_public_access = 0;
	int firewall_enabled = 0;
	int firewall_config_status = 0;
	int firewall_stop_status;
	int firewall_stop_scan_error = 0;
	enum process_presence firewall_stop_state;
	int sshd_mode, config_status;

	config_status = sshd_enabled_mode_read(&sshd_mode);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "SSH restart aborted before stop because enable configuration is unreadable (errno=%d)",
			   config_status);
		errno = config_status;
		return -1;
	}

	before_state = sshd_process_presence(&scan_error);
	if (!sshd_restart_before_state_allows_stop(before_state)) {
		errno = scan_error > 0 ? scan_error : EIO;
		logmessage(LOGNAME, "SSH restart aborted: initial process inventory unknown (scan_error=%d)",
			   errno);
		return -1;
	}

	stop_status = 0;
	if (sshd_restart_requires_stop(before_state)) {
		stop_status = stop_sshd();
		if (stop_status != 0) {
			logmessage(LOGNAME, "SSH restart aborted because the previous daemon did not stop (status=%d)",
				   stop_status);
			return stop_status;
		}
	}

	stopped_state = sshd_process_presence(&scan_error);
	if (!sshd_restart_can_start(stop_status, stopped_state)) {
		if (stopped_state == PROCESS_PRESENCE_UNKNOWN)
			errno = scan_error > 0 ? scan_error : EIO;
		else
			errno = EBUSY;
		logmessage(LOGNAME, "SSH restart aborted: previous daemon absence is unconfirmed (state=%d, scan_error=%d)",
			   stopped_state, scan_error);
		return -1;
	}

	start_status = start_sshd_with_mode(sshd_mode);
	after_state = sshd_process_presence(&scan_error);

	firewall_refresh_needed =
		sshd_firewall_refresh_required(before_state, after_state);
	if (firewall_refresh_needed) {
		firewall_config_status = service_nvram_match_status(
			"sshd_wopen", "1", 0, &sshd_public_access);
		if (firewall_config_status == 0 && sshd_public_access)
			firewall_config_status = service_nvram_match_status(
				"fw_enable_x", "1", 0, &firewall_enabled);

		if (firewall_config_status != 0) {
			firewall_status = firewall_config_status;
			firewall_errno = firewall_config_status;
			logmessage(LOGNAME,
				   "cannot read SSH public-firewall configuration; reconciling listener fail-closed (status=%d)",
				   firewall_config_status);
		} else if (sshd_public_access && firewall_enabled) {
			errno = 0;
			firewall_status = restart_firewall();
			if (firewall_status != 0)
				firewall_errno = errno > 0 ? errno :
					(firewall_status > 0 ? firewall_status : EIO);
		}
	}

	/*
	 * A failed public-access refresh cannot leave a newly active listener
	 * exposed under an unverified ruleset. Stop and verify the postcondition.
	 */
	if (sshd_firewall_failure_requires_stop(firewall_status, after_state)) {
		firewall_stop_status = stop_sshd();
		firewall_stop_state = sshd_process_presence(&firewall_stop_scan_error);
		if (firewall_stop_state == PROCESS_PRESENCE_ABSENT) {
			after_state = PROCESS_PRESENCE_ABSENT;
			logmessage(LOGNAME,
				   "SSH listener confirmed stopped after firewall refresh failure (stop_status=%d)",
				   firewall_stop_status);
		} else {
			logmessage(LOGNAME,
				   "SSH listener may remain active after firewall refresh failure (stop_status=%d, state=%d, scan_error=%d)",
				   firewall_stop_status, firewall_stop_state,
				   firewall_stop_scan_error);
		}
	}
	if (firewall_status != 0)
		errno = firewall_errno;

	if (start_status != 0)
		return start_status;
	if (after_state == PROCESS_PRESENCE_UNKNOWN) {
		errno = scan_error > 0 ? scan_error : EIO;
		logmessage(LOGNAME, "SSH startup state is unknown (scan_error=%d)", scan_error);
		return -1;
	}
	return firewall_status;
}
#endif

#if defined(APP_SCUT)
int is_scutclient_run(void)
{
	return optional_daemon_presence("bin_scutclient", NULL);
}

int stop_scutclient(void)
{
	return optional_daemon_stop_status("scutclient", "bin_scutclient",
					   eval("/usr/bin/scutclient.sh", "stop"));
}

static int
start_scutclient_with_mode(int enabled)
{
	int start_status;

	if (!enabled)
		return 0;

	start_status = optional_daemon_start_status("scutclient", "bin_scutclient");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/scutclient.sh", "start");
}

int start_scutclient(void)
{
	int enabled = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"SCUT client", "scutclient_enable", &enabled);
	if (config_status != 0)
		return config_status;
	return start_scutclient_with_mode(enabled);
}

int restart_scutclient(void)
{
	int enabled = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"SCUT client", "scutclient_enable", &enabled);
	if (config_status != 0)
		return config_status;

	stop_status = stop_scutclient();
	if (stop_status != 0)
		return stop_status;
	return start_scutclient_with_mode(enabled);
}
#endif


#if defined(APP_MENTOHUST)
int is_mentohust_run(void)
{
	return optional_daemon_presence("bin_mentohust", NULL);
}

int stop_mentohust(void)
{
	return optional_daemon_stop_status("mentohust", "bin_mentohust",
					   eval("/usr/bin/mentohust.sh", "stop"));
}

static int
start_mentohust_with_mode(int enabled)
{
	int start_status;

	if (!enabled)
		return 0;

	start_status = optional_daemon_start_status("mentohust", "bin_mentohust");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/mentohust.sh", "start");
}

int start_mentohust(void)
{
	int enabled = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"MentoHUST", "mentohust_enable", &enabled);
	if (config_status != 0)
		return config_status;
	return start_mentohust_with_mode(enabled);
}

int restart_mentohust(void)
{
	int enabled = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"MentoHUST", "mentohust_enable", &enabled);
	if (config_status != 0)
		return config_status;

	stop_status = stop_mentohust();
	if (stop_status != 0)
		return stop_status;
	return start_mentohust_with_mode(enabled);
}
#endif

#if defined(APP_TTYD)
int stop_ttyd(void){
	return eval("/usr/bin/ttyd.sh","stop");
}

static int
start_ttyd_with_mode(int ttyd_mode)
{
	int start_status, attempt, scan_error;
	enum process_presence process_state;

	if (ttyd_mode != 1)
		return 0;

	start_status = optional_daemon_start_status("ttyd", "ttyd");
	if (start_status > 0)
		return 0;
	if (start_status < 0) {
		logmessage(LOGNAME, "ttyd start aborted: process inventory unavailable (status=%d)",
			   start_status);
		return start_status;
	}

	/*
	 * ttyd.sh backgrounds the daemon with start-stop-daemon -b. A successful
	 * wrapper exit only confirms that launch was requested; require the
	 * detached ttyd child to remain visible before reporting startup success.
	 */
	start_status = eval("/usr/bin/ttyd.sh", "start");
	if (start_status != 0) {
		logmessage(LOGNAME, "ttyd launcher failed (status=%d)", start_status);
		return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
	}

	start_status = service_start_probe_wait("ttyd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0)
		return start_status;
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("ttyd", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify ttyd startup (scan_error=%d)",
				   errno);
			return -1;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("ttyd", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0)
				return start_status;
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "ttyd daemon did not remain running after launch");
	return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
}

int start_ttyd(void)
{
	int ttyd_mode = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"ttyd", "ttyd_enable", &ttyd_mode);
	if (config_status != 0)
		return config_status;
	return start_ttyd_with_mode(ttyd_mode);
}

int restart_ttyd(void)
{
	int ttyd_mode = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"ttyd", "ttyd_enable", &ttyd_mode);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "ttyd restart aborted because configuration is unavailable (status=%d)",
			   config_status);
		return config_status;
	}

	stop_status = stop_ttyd();
	if (stop_status != 0) {
		logmessage(LOGNAME, "ttyd restart aborted because stop failed");
		return stop_status;
	}
	return start_ttyd_with_mode(ttyd_mode);
}
#endif

#if defined(APP_SHADOWSOCKS)
int stop_ss(void){
	return eval("/usr/bin/shadowsocks.sh","stop");
}

static int
start_ss_with_mode(int ss_enable)
{
	if (ss_enable != 1)
		return 0;
	return eval("/usr/bin/shadowsocks.sh", "start");
}

int start_ss(void)
{
	int ss_enable = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"Shadowsocks", "ss_enable", &ss_enable);
	if (config_status != 0)
		return config_status;
	return start_ss_with_mode(ss_enable);
}

int restart_ss(void)
{
	int ss_enable = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"Shadowsocks", "ss_enable", &ss_enable);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "Shadowsocks restart aborted because configuration is unavailable (status=%d)",
			   config_status);
		return config_status;
	}

	stop_status = stop_ss();
	if (stop_status != 0) {
		logmessage(LOGNAME, "ss restart aborted because stop failed");
		return stop_status;
	}
	return start_ss_with_mode(ss_enable);
}

int stop_ss_tunnel(void){
	return eval("/usr/bin/ss-tunnel.sh","stop");
}

static int
start_ss_tunnel_with_mode(int ss_tunnel_mode)
{
	if (ss_tunnel_mode != 1)
		return 0;
	return eval("/usr/bin/ss-tunnel.sh", "start");
}

int start_ss_tunnel(void)
{
	int ss_tunnel_mode = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"Shadowsocks tunnel", "ss-tunnel_enable", &ss_tunnel_mode);
	if (config_status != 0)
		return config_status;
	return start_ss_tunnel_with_mode(ss_tunnel_mode);
}

int restart_ss_tunnel(void)
{
	int ss_tunnel_mode = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"Shadowsocks tunnel", "ss-tunnel_enable", &ss_tunnel_mode);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "Shadowsocks tunnel restart aborted because configuration is unavailable (status=%d)",
			   config_status);
		return config_status;
	}

	stop_status = stop_ss_tunnel();
	if (stop_status != 0) {
		logmessage(LOGNAME, "ss_tunnel restart aborted because stop failed");
		return stop_status;
	}
	return start_ss_tunnel_with_mode(ss_tunnel_mode);
}

void update_chnroute(void){
	eval("/bin/sh","-c","/usr/bin/update_chnroute.sh force &");
}

void update_gfwlist(void){
	eval("/bin/sh","-c","/usr/bin/update_gfwlist.sh force &");
}
void update_dlink(void){
	eval("/bin/sh","-c","/usr/bin/update_dlink.sh start &");
}

void reset_dlink(void){
	eval("/bin/sh","-c","/usr/bin/update_dlink.sh reset &");
}
#endif

#if defined(APP_VLMCSD)
int is_vlmcsd_run(void)
{
	return optional_daemon_presence("vlmcsd", NULL);
}

int stop_vlmcsd(void)
{
	return optional_daemon_stop_status("vlmcsd", "vlmcsd",
					   eval("/usr/bin/vlmcsd.sh", "stop"));
}

static int
start_vlmcsd_with_mode(int enabled)
{
	int start_status;

	if (enabled != 1)
		return 0;
	start_status = optional_daemon_start_status("vlmcsd", "vlmcsd");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;
	return eval("/usr/bin/vlmcsd.sh", "start");
}

int start_vlmcsd(void)
{
	int enabled = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"vlmcsd", "vlmcsd_enable", &enabled);
	if (config_status != 0)
		return config_status;
	return start_vlmcsd_with_mode(enabled);
}

int restart_vlmcsd(void)
{
	int enabled = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"vlmcsd", "vlmcsd_enable", &enabled);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "vlmcsd restart aborted because configuration is unavailable (status=%d)",
			   config_status);
		return config_status;
	}

	stop_status = stop_vlmcsd();
	if (stop_status != 0)
		return stop_status;
	return start_vlmcsd_with_mode(enabled);
}
#endif

#if defined(APP_IPERF3)
int is_iperf3_run(void)
{
	return optional_daemon_presence("iperf3", NULL);
}

int stop_iperf3(void)
{
	return optional_daemon_stop_status("iperf3", "iperf3",
					   eval("/usr/bin/iperf3.sh", "stop"));
}

static int
start_iperf3_with_mode(int enabled)
{
	int start_status;

	if (enabled != 1)
		return 0;
	start_status = optional_daemon_start_status("iperf3", "iperf3");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;
	return eval("/usr/bin/iperf3.sh", "start");
}

int start_iperf3(void)
{
	int enabled = 0;
	int config_status;

	config_status = service_enabled_mode_status(
		"iperf3", "iperf3_enable", &enabled);
	if (config_status != 0)
		return config_status;
	return start_iperf3_with_mode(enabled);
}

int restart_iperf3(void)
{
	int enabled = 0;
	int config_status, stop_status;

	config_status = service_enabled_mode_status(
		"iperf3", "iperf3_enable", &enabled);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "iperf3 restart aborted because configuration is unavailable (status=%d)",
			   config_status);
		return config_status;
	}

	stop_status = stop_iperf3();
	if (stop_status != 0)
		return stop_status;
	return start_iperf3_with_mode(enabled);
}
#endif
#if defined(APP_DNSFORWARDER)
int stop_dnsforwarder(void){
	return eval("/usr/bin/dns-forwarder.sh","stop");
}

int start_dnsforwarder(void){
	return eval("/usr/bin/dns-forwarder.sh","start");
}

int restart_dnsforwarder(void){
	/* Stop + conditional start run as one serialized launcher transaction. */
	return eval("/usr/bin/dns-forwarder.sh","restart");
}
#endif

#if defined(APP_ADGUARDHOME)
int stop_adguardhome(void){
	return eval("/usr/bin/adguardhome.sh","stop");
}

void start_adguardhome(void){
	int start_status;
	int adg_mode;
	int config_status = service_enabled_mode_status(
		"AdGuardHome", "adg_enable", &adg_mode);

	if (config_status != 0)
		return;
	if (adg_mode == 1) {
		start_status = eval("/usr/bin/adguardhome.sh","start");
		if (start_status != 0)
			logmessage(LOGNAME, "AdGuardHome launcher failed (status=%d)", start_status);
	}
}

int restart_adguardhome(void){
	/* Keep stop + conditional start inside one launcher lifecycle lock. */
	return eval("/usr/bin/adguardhome.sh","restart");
}

#endif

#if defined(APP_ZEROTIER)
int stop_zerotier(void){
	return eval("/usr/bin/zerotier.sh","stop");
}

int start_zerotier(void){
	int zerotier_enable;
	int config_status = service_enabled_mode_status(
		"ZeroTier", "zerotier_enable", &zerotier_enable);

	if (config_status != 0)
		return config_status;
	if (zerotier_enable != 1)
		return 0;
	return eval("/usr/bin/zerotier.sh","start");
}

int restart_zerotier(void){
	int stop_status = stop_zerotier();

	if (stop_status != 0) {
		logmessage(LOGNAME, "ZeroTier restart aborted because stop failed");
		return stop_status;
	}
	return start_zerotier();
}
#endif

#if defined(APP_DDNSTO)
int stop_ddnsto(void){
	return eval("/usr/bin/ddnsto.sh","stop");
}

void start_ddnsto(void){
	int start_status;
	int ddnsto_enable;
	int config_status = service_enabled_mode_status(
		"DDNS.to", "ddnsto_enable", &ddnsto_enable);

	if (config_status != 0)
		return;
	if (ddnsto_enable == 1) {
		start_status = eval("/usr/bin/ddnsto.sh","start");
		if (start_status != 0)
			logmessage(LOGNAME, "DDNS.to launcher failed (status=%d)", start_status);
	}
}

int restart_ddnsto(void){
	/* Keep stop + conditional start inside one DDNSTO lifecycle lock. */
	return eval("/usr/bin/ddnsto.sh","restart");
}
#endif

#if defined(APP_SQM)
int stop_sqm(void){
	return eval("/usr/lib/sqm/run.sh","stop");
}

int start_sqm(void){
	int sqm_mode;
	int config_status = service_enabled_mode_status(
		"SQM", "sqm_enable", &sqm_mode);

	if (config_status != 0)
		return config_status;
	if (sqm_mode != 1)
		return 0;
	return eval("/usr/lib/sqm/run.sh");
}

int restart_sqm(void){
	int stop_status = stop_sqm();

	if (stop_status != 0) {
		logmessage(LOGNAME, "sqm restart aborted because stop failed");
		return stop_status;
	}
	return start_sqm();
}
#endif

#if defined(APP_ALDRIVER)
int stop_aldriver(void){
	return eval("/usr/bin/aliyundrive-webdav.sh","stop");
}

int start_aldriver(void){
	int aldriver_enable;
	int config_status = service_enabled_mode_status(
		"AliyunDrive WebDAV", "aliyundrive_enable", &aldriver_enable);

	if (config_status != 0)
		return config_status;
	if (aldriver_enable != 1)
		return 0;
	return eval("/usr/bin/aliyundrive-webdav.sh","start");
}

int restart_aldriver(void){
	int stop_status = stop_aldriver();

	if (stop_status != 0) {
		logmessage(LOGNAME, "aldriver restart aborted because stop failed");
		return stop_status;
	}
	return start_aldriver();
}
#endif

#if defined(APP_WIREGUARD)
int stop_wireguard(void){
	return eval("/usr/bin/wireguard.sh","stop");
}

int start_wireguard(void){
	int wireguard_enable;
	int config_status = service_enabled_mode_status(
		"WireGuard", "wireguard_enable", &wireguard_enable);

	if (config_status != 0)
		return config_status;
	if (wireguard_enable != 1)
		return 0;
	return eval("/usr/bin/wireguard.sh","start");
}

int restart_wireguard(void){
	int stop_status = stop_wireguard();

	if (stop_status != 0) {
		logmessage(LOGNAME, "wireguard restart aborted because stop failed");
		return stop_status;
	}
	return start_wireguard();
}
#endif

#if defined(APP_ADBYBY)
int stop_adbyby(void){
	return eval("/usr/bin/adbyby.sh","stop");
}

void start_adbyby(void){
	int start_status;
	int adbyby_mode;
	int config_status = service_enabled_mode_status(
		"Adbyby", "adbyby_enable", &adbyby_mode);

	if (config_status != 0)
		return;
	if (adbyby_mode == 1) {
		start_status = eval("/usr/bin/adbyby.sh","start");
		if (start_status != 0)
			logmessage(LOGNAME, "Adbyby launcher failed (status=%d)", start_status);
	}
}

int restart_adbyby(void){
	/* One launcher owns stop/start atomically with cron and manual actions. */
	return eval("/usr/bin/adbyby.sh","restart");
}

int update_adb(void){
	return eval("/usr/bin/adbyby.sh","updateadb");
}
#endif

#if defined(APP_SMARTDNS)
int stop_smartdns(void){
	return eval("/usr/bin/smartdns.sh","stop");
}

int start_smartdns(void){
	int smartdns_mode;
	int config_status = service_enabled_mode_status(
		"SmartDNS", "sdns_enable", &smartdns_mode);

	if (config_status != 0)
		return config_status;
	if (smartdns_mode != 1)
		return 0;
	return eval("/usr/bin/smartdns.sh","start");
}

int restart_smartdns(void){
	/* The shell lifecycle lock must cover the complete restart transaction. */
	return eval("/usr/bin/smartdns.sh","restart");
}
#endif

#if defined(APP_ALIDDNS)
int stop_aliddns(void){
	return eval("/usr/bin/aliddns.sh","stop");
}

void start_aliddns(void){
	int start_status;
	int aliddns_mode;
	int config_status = service_enabled_mode_status(
		"AliDDNS", "aliddns_enable", &aliddns_mode);

	if (config_status != 0)
		return;
	if (aliddns_mode == 1) {
		start_status = eval("/usr/bin/aliddns.sh","start");
		if (start_status != 0)
			logmessage(LOGNAME, "AliDDNS launcher failed (status=%d)", start_status);
	}
}

int restart_aliddns(void){
	return eval("/usr/bin/aliddns.sh","restart");
}
#endif

#if defined(APP_FRP)
int stop_frp(void){
	int status = eval("/usr/bin/frp.sh","stop");

	if (status != 0)
		logmessage(LOGNAME, "FRP stop failed: %d", status);
	return status;
}

void start_frp(void){
	int status = eval("/usr/bin/frp.sh","start");

	if (status != 0)
		logmessage(LOGNAME, "FRP start failed: %d", status);
}

int restart_frp(void){
	int status = eval("/usr/bin/frp.sh","restart");

	if (status != 0)
		logmessage(LOGNAME, "FRP restart failed: %d", status);
	return status;
}
#endif

static int
httpd_nvram_set_int_status(const char *param_name, int value, int temporary)
{
	int api_result, saved_errno;

	if (!param_name || !param_name[0] ||
	    (temporary != 0 && temporary != 1))
		return EINVAL;

	errno = 0;
	if (temporary)
		api_result = nvram_set_int_temp(param_name, value);
	else
		api_result = nvram_set_int(param_name, value);
	saved_errno = errno;
	return httpd_nvram_write_status(api_result, saved_errno);
}

static int
httpd_nvram_read_int_status(const char *param_name, int missing_value, int *value)
{
	char *text = NULL;
	int api_result, saved_errno;

	if (!param_name || !param_name[0] || !value)
		return EINVAL;

	errno = 0;
	api_result = nvram_get_checked(param_name, &text);
	saved_errno = errno;
	return rc_wan_nvram_read_int_status(
		api_result, saved_errno, text, missing_value, value);
}

static int
httpd_nvram_read_int_adapter(void *context, const char *param_name,
			    int missing_value, int *value)
{
	(void)context;
	return httpd_nvram_read_int_status(param_name, missing_value, value);
}



static int
httpd_started_state_publish_status(int started)
{
	return httpd_nvram_set_int_status("httpd_started", started, 1);
}


static enum process_presence
httpd_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;

	presence = process_inventory_scan("httpd", &snapshot);
	if (scan_error)
		*scan_error = snapshot.error;
	process_snapshot_release(&snapshot);
	return presence;
}

static int
httpd_start_wait_failure_recover(int wait_status, const char *phase)
{
	int wait_errno = errno > 0 ? errno : EIO;
	int rollback_status;
	int rollback_scan_error = 0;
	int state_status;
	enum process_presence rollback_presence;

	/*
	 * The listener may already be active even though a startup probe wait
	 * failed. Never leave a potentially live listener without published state.
	 */
	rollback_status = stop_httpd_locked();
	rollback_presence = httpd_process_presence(&rollback_scan_error);
	if (httpd_failed_start_preserve_running_state(rollback_presence)) {
		state_status = httpd_started_state_publish_status(1);
		if (state_status != 0)
			logmessage(LOGNAME,
				   "cannot publish HTTPD possibly-running state after %s wait failure (status=%d)",
				   phase, state_status);
		logmessage(LOGNAME,
			   "HTTPD may remain active after %s wait failure (stop_status=%d, state=%d, scan_error=%d)",
			   phase, rollback_status, rollback_presence,
			   rollback_scan_error);
	} else if (rollback_status != 0) {
		logmessage(LOGNAME,
			   "HTTPD listener is absent after %s wait failure but rollback reported status=%d",
			   phase, rollback_status);
	}

	errno = wait_errno;
	return wait_status;
}

static int
start_httpd_locked(int restart_fw)
{
	struct httpd_start_config start_config;
	int http_proto, argv_index, http_port, restart_fw_need, firewall_enabled = 0;
	int launch_status, attempt, status, scan_error, firewall_status = 0;
	int firewall_errno = 0;
	int firewall_stop_status;
	int firewall_stop_scan_error = 0;
	enum process_presence firewall_stop_state;
	enum process_presence presence;
	char http_port_s[8];
#if defined (SUPPORT_HTTPS)
	char https_port_s[8];
#endif
	char *httpd_argv[] = {
		"/usr/sbin/httpd",
		NULL, NULL,
		NULL, NULL,
		NULL,
	};

	argv_index = 1;
	http_port = 0;
	restart_fw_need = 0;

	/*
	 * Configuration normalization and stale-state publication are writes.
	 * First require an authoritative process scan: RUNNING is a duplicate,
	 * UNKNOWN is an error, and only ABSENT permits the startup transaction.
	 */
	presence = httpd_process_presence(&scan_error);
	if (!httpd_start_can_launch(presence)) {
		status = httpd_start_precondition_status(presence);
		if (presence == PROCESS_PRESENCE_RUNNING) {
			status = httpd_started_state_publish_status(1);
			if (status != 0) {
				logmessage(LOGNAME,
					   "cannot publish HTTPD running state (status=%d)",
					   status);
				return status;
			}
			logmessage(LOGNAME, "HTTPD is already running; refusing duplicate launch");
		} else {
			logmessage(LOGNAME,
				   "cannot verify HTTPD process state; refusing startup (scan_error=%d)",
				   scan_error ? scan_error : EIO);
		}
		return status;
	}

#if defined (SUPPORT_HTTPS)
	status = httpd_start_config_read(httpd_nvram_read_int_adapter, NULL,
					 1, &start_config);
#else
	status = httpd_start_config_read(httpd_nvram_read_int_adapter, NULL,
					 0, &start_config);
#endif
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot read HTTPD startup settings; refusing listener launch (status=%d)",
			   status);
		return status;
	}
	http_proto = start_config.http_proto;
	http_port = start_config.http_port;
	restart_fw_need = httpd_start_config_firewall_required(&start_config);
	if (restart_fw && restart_fw_need) {
		status = service_nvram_match_status("fw_enable_x", "1", 0,
						 &firewall_enabled);
		if (status != 0) {
			logmessage(LOGNAME,
				   "cannot read firewall enable state; refusing HTTPD listener launch (status=%d)",
				   status);
			return status;
		}
	}

	if (http_proto == 0 || http_proto == 2) {

		if (http_port < 80 || http_port > 65535) {
			http_port = 80;
			status = httpd_nvram_set_int_status("http_lanport", http_port, 0);
			if (status != 0) {
				logmessage(LOGNAME,
					   "cannot persist normalized HTTP port (status=%d)",
					   status);
				return status;
			}
		}

		sprintf(http_port_s, "%d", http_port);

		httpd_argv[argv_index++] = "-p";
		httpd_argv[argv_index++] = http_port_s;

	}

#if defined (SUPPORT_HTTPS)
	if (http_proto == 1 || http_proto == 2) {
		int https_port = start_config.https_port;
		if (https_port < 81 || https_port > 65535 || https_port == http_port) {
			https_port = 443;
			status = httpd_nvram_set_int_status("https_lport", https_port, 0);
			if (status != 0) {
				logmessage(LOGNAME,
					   "cannot persist normalized HTTPS port (status=%d)",
					   status);
				return status;
			}
		}

		sprintf(https_port_s, "%d", https_port);

		httpd_argv[argv_index++] = "-s";
		httpd_argv[argv_index++] = https_port_s;

	}
#endif

	/* Clear stale state only after the earlier complete scan confirmed ABSENT. */
	status = httpd_started_state_publish_status(0);
	if (status != 0) {
		logmessage(LOGNAME,
			   "cannot clear stale HTTPD state before launch (status=%d)",
			   status);
		return status;
	}

	launch_status = _eval(httpd_argv, NULL, 0, NULL);
	if (launch_status != 0) {
		logmessage(LOGNAME, "HTTPD launcher failed: %d", launch_status);
		return httpd_start_postcondition(launch_status, PROCESS_PRESENCE_ABSENT);
	}

	/*
	 * HTTPD can daemonize after _eval()'s launcher exits. Publish service
	 * state and open firewall ports only after the child remains observable.
	 */
	status = service_start_probe_wait("HTTPD", HTTPD_START_INITIAL_WAIT_SECONDS);
	if (status != 0)
		return httpd_start_wait_failure_recover(status, "initial startup");
	status = -1;
	for (attempt = 0; attempt < HTTPD_START_PROBE_ATTEMPTS; attempt++) {
		presence = httpd_process_presence(&scan_error);
		status = httpd_start_postcondition(launch_status, presence);
		if (presence == PROCESS_PRESENCE_RUNNING)
			break;
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			logmessage(LOGNAME,
				   "cannot verify HTTPD startup postcondition (scan_error=%d)",
				   scan_error ? scan_error : EIO);
			break;
		}

		if (attempt + 1 < HTTPD_START_PROBE_ATTEMPTS) {
			status = service_start_probe_wait("HTTPD", HTTPD_START_RETRY_WAIT_SECONDS);
			if (status != 0)
				return httpd_start_wait_failure_recover(status, "retry");
		}
	}

	if (status != 0) {
		logmessage(LOGNAME, "HTTPD daemon did not remain running after launch");
		return status;
	}

	status = httpd_started_state_publish_status(1);
	if (status != 0) {
		int rollback_status;

		rollback_status = stop_httpd_locked();
		logmessage(LOGNAME,
			   "cannot publish HTTPD running state; attempted listener rollback (publish_status=%d, stop_status=%d)",
			   status, rollback_status);
		errno = status;
		return status;
	}

	if (httpd_firewall_refresh_required(restart_fw, restart_fw_need, firewall_enabled)) {
		errno = 0;
		firewall_status = restart_firewall();
		if (firewall_status != 0) {
			firewall_errno = errno > 0 ? errno :
				(firewall_status > 0 ? firewall_status : EIO);
			if (httpd_firewall_failure_requires_stop(firewall_status, presence)) {
				firewall_stop_status = stop_httpd_locked();
				firewall_stop_state = httpd_process_presence(&firewall_stop_scan_error);
				if (firewall_stop_state == PROCESS_PRESENCE_ABSENT) {
					logmessage(LOGNAME,
						   "HTTPD listener confirmed stopped after firewall refresh failure (stop_status=%d)",
						   firewall_stop_status);
				} else {
					status = httpd_started_state_publish_status(1);
					if (status != 0)
						logmessage(LOGNAME,
							   "cannot publish HTTPD possibly-running state after firewall failure (status=%d)",
							   status);
					logmessage(LOGNAME,
						   "HTTPD listener may remain active after firewall refresh failure (stop_status=%d, state=%d, scan_error=%d)",
						   firewall_stop_status, firewall_stop_state,
						   firewall_stop_scan_error);
				}
			}
			errno = firewall_errno;
			return firewall_status;
		}
	}

	return 0;
}

static int
httpd_lifecycle_lock_acquire_status(const char *operation, int *lock_fd)
{
	*lock_fd = httpd_lifecycle_lock_acquire();
	if (*lock_fd >= 0)
		return 0;
	{
		int lock_error = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot acquire HTTPD %s lifecycle lock (status=%d)", operation, lock_error);
		return lock_error;
	}
}
static int
httpd_lifecycle_lock_finish(int lock_fd, int operation_status, const char *operation)
{
	if (httpd_lifecycle_lock_release(lock_fd) != 0) {
		int release_error = errno ? errno : EIO;
		logmessage(LOGNAME, "HTTPD %s lifecycle lock release failed (status=%d)", operation, release_error);
		if (operation_status == 0)
			operation_status = release_error;
	}
	return operation_status;
}
int
start_httpd(int restart_fw)
{
	int lock_fd, lock_status, status;
	lock_status = httpd_lifecycle_lock_acquire_status("start", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = start_httpd_locked(restart_fw);
	return httpd_lifecycle_lock_finish(lock_fd, status, "start");
}
int
stop_httpd(void)
{
	int lock_fd, lock_status, status;
	lock_status = httpd_lifecycle_lock_acquire_status("stop", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	status = stop_httpd_locked();
	return httpd_lifecycle_lock_finish(lock_fd, status, "stop");
}
static int
stop_httpd_locked(void)
{
	char* svcs[] = { "httpd", NULL };
	int stop_status = kill_services(svcs, 3, 1);
	int publish_status;

	if (!httpd_stop_policy_publish_stopped(stop_status)) {
		logmessage(LOGNAME, "HTTPD did not stop cleanly; retaining service state (status=%d)",
			   stop_status);
		return stop_status;
	}

	publish_status = httpd_started_state_publish_status(0);
	if (publish_status != 0) {
		logmessage(LOGNAME,
			   "HTTPD stopped but failed to publish stopped state (status=%d)",
			   publish_status);
		return publish_status;
	}
	return 0;
}

int
restart_httpd(void)
{
	int lock_fd, lock_status, stop_status, status;
	lock_status = httpd_lifecycle_lock_acquire_status("restart", &lock_fd);
	if (lock_status != 0)
		return lock_status;
	stop_status = stop_httpd_locked();
	if (!httpd_restart_can_start(stop_status)) {
		logmessage(LOGNAME, "refusing HTTPD restart: previous daemon did not stop (status=%d)",
			   stop_status);
		status = stop_status;
	} else {
		status = start_httpd_locked(1);
	}
	return httpd_lifecycle_lock_finish(lock_fd, status, "restart");
}

int
stop_rstats(void)
{
	char* svcs[] = { "rstats", NULL };
	return kill_services(svcs, 3, 1);
}

int
start_rstats(void)
{
	int start_status, attempt, scan_error;
	enum process_presence process_state;

	if (nvram_invmatch("rstats_enable", "1"))
		return 0;

	start_status = optional_daemon_start_status("rstats", "rstats");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	/*
	 * rstats daemonizes before allocating and publishing its PID file.
	 * eval() only observes the parent; verify that the child completes this
	 * startup work and remains present before reporting success.
	 */
	start_status = eval("/sbin/rstats");
	if (start_status != 0) {
		logmessage(LOGNAME, "rstats daemon launch failed (status=%d)", start_status);
		return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
	}

	start_status = service_start_probe_wait("rstats", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0)
		return start_status;
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("rstats", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify rstats startup (scan_error=%d)",
				   errno);
			return -1;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("rstats", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0)
				return start_status;
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "rstats daemon did not remain running after launch");
	return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
}

int
restart_rstats(void)
{
	int stop_status = stop_rstats();
	int start_status;

	if (stop_status != 0)
		return stop_status;
	start_status = start_rstats();
	if (start_status != 0 && nvram_match("rstats_enable", "1"))
		return start_status;
	return 0;
}

int
start_lltd(void)
{
	int start_status, attempt, scan_error;
	enum process_presence process_state;

	if (nvram_invmatch("lltd_enable", "1"))
		return 0;

	start_status = optional_daemon_start_status("lld2d", "lld2d");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	/*
	 * lld2d double-forks before publishing its per-interface PID file and
	 * entering the packet event loop. eval() observes only the exiting parent.
	 */
	start_status = eval("/bin/lld2d", IFNAME_BR);
	if (start_status != 0) {
		logmessage(LOGNAME, "LLTD daemon launch failed (status=%d)", start_status);
		return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
	}

	start_status = service_start_probe_wait("LLTD", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (start_status != 0)
		return start_status;
	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("lld2d", &scan_error);
		if (process_presence_start_postcondition(start_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			errno = scan_error > 0 ? scan_error : EIO;
			logmessage(LOGNAME, "cannot verify LLTD startup (scan_error=%d)",
				   errno);
			return -1;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			start_status = service_start_probe_wait("LLTD", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (start_status != 0)
				return start_status;
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "LLTD daemon did not remain running after launch");
	return process_presence_start_postcondition(start_status,
					     PROCESS_PRESENCE_ABSENT);
}

int
stop_lltd(void)
{
	char* svcs[] = { "lld2d", NULL };
	return kill_services(svcs, 2, 1);
}

int
restart_lltd(void)
{
	int stop_status = stop_lltd();
	int start_status;

	if (stop_status != 0)
		return stop_status;
	start_status = start_lltd();
	if (start_status != 0 && nvram_match("lltd_enable", "1"))
		return start_status;
	return 0;
}

int
start_logger(int showinfo)
{
	int syslog_status;
	int klog_status;
	int settle_status = 0;

	/* Attempt both daemons even if the first launch fails. */
	syslog_status = start_syslogd();

	if (showinfo)
	{
		/* Preserve the logger settling interval even when a signal arrives. */
		settle_status = service_wait_microseconds(300000U);
		if (settle_status != 0) {
			settle_status = errno > 0 ? errno : EIO;
			logmessage(LOGNAME, "logger settling wait failed (status=%d)", settle_status);
		}
		
#if 0
#if defined (VENDOR_ASUS)
		logmessage(LOGNAME, "bootloader version: %s", nvram_safe_get("blver"));
#endif
#endif
		logmessage(LOGNAME, "firmware version: %s", nvram_safe_get("firmver_sub"));
	}

	klog_status = start_klogd();
	if (syslog_status != 0)
		logmessage(LOGNAME, "syslogd startup failed (status=%d)", syslog_status);
	if (klog_status != 0)
		logmessage(LOGNAME, "klogd startup failed (status=%d)", klog_status);

	return rc_logger_start_status_with_wait(syslog_status, settle_status, klog_status);
}

int
stop_logger(void)
{
	char* svcs[] = { "klogd", "syslogd", NULL };
	return kill_services(svcs, 3, 1);
}

static int
watchdog_cpu_config_enabled(int *should_load)
{
	char *value = NULL;
	int api_result, saved_errno, configured_value = 0;
	int read_status, status;

	if (!should_load) {
		errno = EINVAL;
		return EINVAL;
	}
	errno = 0;
	api_result = nvram_get_checked("watchdog_cpu", &value);
	saved_errno = errno;
	read_status = rc_wan_nvram_read_int_status(
		api_result, saved_errno, value, 0, &configured_value);
	status = rc_watchdog_cpu_restart_config_status(
		read_status, configured_value, should_load);
	if (status != 0)
		errno = status;
	return status;
}

int
start_watchdog_cpu(void)
{
	int module_status, should_load, config_status;

	config_status = watchdog_cpu_config_enabled(&should_load);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "cannot read CPU watchdog configuration; preserving current module state (status=%d)",
			   config_status);
		return config_status;
	}
	if (!should_load)
		return 0;

	/*
	 * module_smart_load() returns zero for both "already loaded" and
	 * "load failed". Verify the tri-state module probe after the operation.
	 */
	(void)module_smart_load("rt_timer_wdg", NULL);
	module_status = is_module_loaded_status("rt_timer_wdg");
	if (module_status < 0) {
		int probe_error = -module_status;
		errno = probe_error;
		logmessage(LOGNAME, "CPU watchdog module state probe failed (errno=%d)",
			   probe_error);
		return probe_error;
	}
	if (module_status != 1) {
		logmessage(LOGNAME, "CPU watchdog module startup failed");
		return EIO;
	}
	return 0;
}

static int
watchdog_cpu_restart_module_load(const char *module_name, const char *module_param)
{
	return module_smart_load(module_name, module_param);
}

static int
watchdog_cpu_restart_module_unload(const char *module_name, int recurse_unload)
{
	return module_smart_unload(module_name, recurse_unload);
}

static int
watchdog_cpu_restart_module_is_loaded(const char *module_name)
{
	return is_module_loaded_status(module_name);
}

static const rc_watchdog_cpu_restart_ops_t watchdog_cpu_restart_ops = {
	watchdog_cpu_restart_module_load,
	watchdog_cpu_restart_module_unload,
	watchdog_cpu_restart_module_is_loaded
};

int
restart_watchdog_cpu(void)
{
	int should_load, config_status, status;

	config_status = watchdog_cpu_config_enabled(&should_load);
	if (config_status != 0) {
		logmessage(LOGNAME,
			   "cannot read CPU watchdog configuration; refusing module state change (status=%d)",
			   config_status);
		return config_status;
	}
	status = rc_watchdog_cpu_restart_verified("rt_timer_wdg", should_load,
					 &watchdog_cpu_restart_ops);
	if (status != 0) {
		if (should_load)
			logmessage(LOGNAME, "CPU watchdog module enable failed or module is unavailable (status=%d)",
				   status);
		else
			logmessage(LOGNAME, "CPU watchdog module disable failed; module remains loaded (status=%d)",
				   status);
	}
	return status;
}

#if defined(APP_SKIPDBV2)
/*
 * skipd daemonizes before returning from doSystem(). Confirm the detached
 * database process survived the fork/session setup before reporting startup.
 */
static int
start_skipdbv2(void)
{
	int preflight_status, launch_status, attempt, scan_error;
	enum process_presence process_state;

	preflight_status = optional_daemon_start_status("SkipDBv2", "skipd");
	if (preflight_status > 0)
		return 0;
	if (preflight_status < 0)
		return preflight_status;

	launch_status = doSystem("/usr/sbin/skipd -d /etc/storage/db");
	if (launch_status != 0) {
		logmessage(LOGNAME, "SkipDBv2 launcher failed (status=%d)", launch_status);
		return launch_status;
	}

	launch_status = service_start_probe_wait(
		"SkipDBv2", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);
	if (launch_status != 0)
		return launch_status;

	for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++) {
		process_state = optional_daemon_presence("skipd", &scan_error);
		if (process_presence_start_postcondition(launch_status, process_state) == 0)
			return 0;
		if (process_state == PROCESS_PRESENCE_UNKNOWN) {
			scan_error = scan_error > 0 ? scan_error : EIO;
			errno = scan_error;
			logmessage(LOGNAME,
				   "SkipDBv2 startup inventory unknown (errno=%d)", scan_error);
			return -1;
		}
		if (attempt + 1 < PROCESS_PRESENCE_START_PROBE_ATTEMPTS) {
			launch_status = service_start_probe_wait(
				"SkipDBv2", PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS);
			if (launch_status != 0)
				return launch_status;
		}
	}

	errno = ETIMEDOUT;
	logmessage(LOGNAME, "SkipDBv2 daemon did not remain running after launch");
	return -1;
}
#endif

int
start_services_once(int is_ap_mode)
{
	int start_status = 0;
	int bridge_stp_status, bridge_forward_delay_status;

	{
		int wl_auth_start_status = start_8021x_wl();

		if (wl_auth_start_status != 0) {
			logmessage(LOGNAME,
				   "5GHz boot startup aborted: 802.1X authenticator launch failed (status=%d)",
				   wl_auth_start_status);
			if (stop_8021x_wl() != 0)
				logmessage(LOGNAME, "5GHz boot rollback could not stop 802.1X authenticator");
			if (stop_wifi_all_wl() < 0)
				logmessage(LOGNAME, "5GHz boot rollback could not stop wireless interfaces");
		}
		start_status = rc_first_service_start_status(start_status, wl_auth_start_status);
	}
	{
		int rt_auth_start_status = start_8021x_rt();

		if (rt_auth_start_status != 0) {
			logmessage(LOGNAME,
				   "2.4GHz boot startup aborted: 802.1X authenticator launch failed (status=%d)",
				   rt_auth_start_status);
			if (stop_8021x_rt() != 0)
				logmessage(LOGNAME, "2.4GHz boot rollback could not stop 802.1X authenticator");
			if (stop_wifi_all_rt() < 0)
				logmessage(LOGNAME, "2.4GHz boot rollback could not stop wireless interfaces");
		}
		start_status = rc_first_service_start_status(start_status, rt_auth_start_status);
	}
	start_status = rc_first_service_start_status(start_status, start_httpd(0));
	start_status = rc_first_service_start_status(start_status, start_telnetd());
#if defined(APP_SSHD)
	start_status = rc_first_service_start_status(start_status, start_sshd());
#endif
	{
		int vpn_start_status = start_vpn_server();
		if (vpn_start_status != 0)
			logmessage(LOGNAME, "VPN server boot startup failed (status=%d)",
				   vpn_start_status);
		start_status = rc_first_service_start_status(start_status, vpn_start_status);
	}
	start_status = rc_first_service_start_status(start_status, start_watchdog());
	start_status = rc_first_service_start_status(start_status, start_gpio_btn());
	if (nvram_match("adsc_enable", "1"))
		start_status = rc_first_service_start_status(start_status, start_infosvr());

	if (!is_ap_mode) {
		if (!is_upnp_run())
			start_status = rc_first_service_start_status(start_status, start_upnp());

		if (!nvram_match("lan_stp", "0")) {
			bridge_stp_status = br_set_stp(IFNAME_BR, 1);
			if (bridge_stp_status != 0)
				logmessage(LOGNAME, "LAN bridge STP enable failed (status=%d)",
					   bridge_stp_status);
			start_status = rc_first_service_start_status(
				start_status, bridge_stp_status);

			bridge_forward_delay_status = br_set_fd(IFNAME_BR, 15);
			if (bridge_forward_delay_status != 0)
				logmessage(LOGNAME, "LAN bridge forward-delay configuration failed (status=%d)",
					   bridge_forward_delay_status);
			start_status = rc_first_service_start_status(
				start_status, bridge_forward_delay_status);
		}
	} else {
		start_status = rc_first_service_start_status(start_status,
							    start_udpxy(IFNAME_BR));
#if defined(APP_XUPNPD)
		start_status = rc_first_service_start_status(start_status,
							    start_xupnpd(IFNAME_BR));
#endif
	}
#if defined(APP_SKIPDBV2)
	{
		int skipdb_start_status = start_skipdbv2();
		if (skipdb_start_status != 0)
			logmessage(LOGNAME, "SkipDBv2 boot startup failed (status=%d)",
				   skipdb_start_status);
		start_status = rc_first_service_start_status(start_status, skipdb_start_status);
	}
#endif
#if defined(APP_SCUT)
	{
		int scut_start_status = start_scutclient();
		if (scut_start_status != 0)
			logmessage(LOGNAME, "SCUT client boot startup failed (status=%d)",
				   scut_start_status);
		start_status = rc_first_service_start_status(start_status, scut_start_status);
	}
#endif
#if defined(APP_DNSFORWARDER)
	{
		int dnsforwarder_start_status = start_dnsforwarder();
		if (dnsforwarder_start_status != 0)
			logmessage(LOGNAME, "DNS Forwarder boot startup failed (status=%d)",
				   dnsforwarder_start_status);
		start_status = rc_first_service_start_status(start_status, dnsforwarder_start_status);
	}
#endif
#if defined(APP_TTYD)
	{
		int ttyd_start_status = start_ttyd();
		if (ttyd_start_status != 0)
			logmessage(LOGNAME, "ttyd boot startup failed (status=%d)", ttyd_start_status);
		start_status = rc_first_service_start_status(start_status, ttyd_start_status);
	}
#endif
#if defined(APP_VLMCSD)
	{
		int vlmcsd_start_status = start_vlmcsd();
		if (vlmcsd_start_status != 0)
			logmessage(LOGNAME, "vlmcsd boot startup failed (status=%d)",
				   vlmcsd_start_status);
		start_status = rc_first_service_start_status(start_status, vlmcsd_start_status);
	}
#endif
#if defined(APP_IPERF3)
	{
		int iperf3_start_status = start_iperf3();
		if (iperf3_start_status != 0)
			logmessage(LOGNAME, "iperf3 boot startup failed (status=%d)",
				   iperf3_start_status);
		start_status = rc_first_service_start_status(start_status, iperf3_start_status);
	}
#endif
	if (nvram_match("lltd_enable", "1"))
		start_status = rc_first_service_start_status(start_status, start_lltd());
	start_status = rc_first_service_start_status(start_status, start_watchdog_cpu());
	if (nvram_match("crond_enable", "1"))
		start_status = rc_first_service_start_status(start_status, start_crond());
	{
		int networkmap_start_status = start_networkmap(1);
		if (networkmap_start_status != 0)
			logmessage(LOGNAME,
				   "networkmap boot startup failed (status=%d)",
				   networkmap_start_status);
		start_status = rc_first_service_start_status(start_status, networkmap_start_status);
	}
	if (nvram_match("rstats_enable", "1"))
		start_status = rc_first_service_start_status(start_status, start_rstats());
#if defined(APP_MENTOHUST)
	{
		int mentohust_start_status = start_mentohust();
		if (mentohust_start_status != 0)
			logmessage(LOGNAME, "MentoHUST boot startup failed (status=%d)",
				   mentohust_start_status);
		start_status = rc_first_service_start_status(start_status, mentohust_start_status);
	}
#endif
	{
		int iappd_restart_status = system("/usr/bin/iappd.sh restart");
		if (iappd_restart_status != 0)
			logmessage(LOGNAME, "Ralink IAPP restart command failed (wait status=%d)",
				   iappd_restart_status);
		start_status = rc_first_service_start_status(start_status, iappd_restart_status);
	}
	{
		int tproxy_module_status = system("modprobe xt_TPROXY");
		if (tproxy_module_status != 0)
			logmessage(LOGNAME, "xt_TPROXY module load command failed (wait status=%d)",
				   tproxy_module_status);
		start_status = rc_first_service_start_status(start_status, tproxy_module_status);
	}
	{
		int iappd_test_status = system("/usr/bin/iappd.sh test");
		if (iappd_test_status != 0)
			logmessage(LOGNAME, "Ralink IAPP post-start test failed (wait status=%d)",
				   iappd_test_status);
		start_status = rc_first_service_start_status(start_status, iappd_test_status);
	}
	return start_status;
}

int
stop_services(int stopall)
{
	int stop_status = 0;

	if (stopall) {
		record_service_stop_status(&stop_status, "telnetd", stop_telnetd());
#if defined (APP_SSHD)
		record_service_stop_status(&stop_status, "sshd", stop_sshd());
#endif
		record_service_stop_status(&stop_status, "httpd", stop_httpd());
		record_service_stop_status(&stop_status, "VPN server", stop_vpn_server());
	}
#if defined (USE_USB_SUPPORT)
	record_service_stop_status(&stop_status, "p910nd", stop_p910nd());
#if defined (SRV_LPRD)
	record_service_stop_status(&stop_status, "lpd", stop_lpd());
#endif
#if defined (SRV_U2EC)
	record_service_stop_status(&stop_status, "u2ec", stop_u2ec());
#endif
#endif
#if defined(APP_SCUT)
	record_service_stop_status(&stop_status, "scutclient", stop_scutclient());
#endif
#if defined(APP_MENTOHUST)
	record_service_stop_status(&stop_status, "mentohust", stop_mentohust());
#endif
	record_service_stop_status(&stop_status, "Ralink IAPP",
				    system("/usr/bin/iappd.sh stop"));
#if defined(APP_VLMCSD)
	record_service_stop_status(&stop_status, "vlmcsd", stop_vlmcsd());
#endif
#if defined(APP_IPERF3)
	record_service_stop_status(&stop_status, "iperf3", stop_iperf3());
#endif
#if defined(APP_DNSFORWARDER)
	record_service_stop_status(&stop_status, "DNS Forwarder", stop_dnsforwarder());
#endif
#if defined(APP_ADGUARDHOME)
	record_service_stop_status(&stop_status, "AdGuardHome", stop_adguardhome());
#endif
#if defined(APP_SHADOWSOCKS)
	record_service_stop_status(&stop_status, "Shadowsocks", stop_ss());
	record_service_stop_status(&stop_status, "Shadowsocks tunnel", stop_ss_tunnel());
#endif
#if defined(APP_ADBYBY)
	record_service_stop_status(&stop_status, "Adbyby", stop_adbyby());
#endif
#if defined(APP_DDNSTO)
	record_service_stop_status(&stop_status, "DDNSTO", stop_ddnsto());
#endif
#if defined(APP_SQM)
	record_service_stop_status(&stop_status, "SQM", stop_sqm());
#endif
#if defined(APP_ALDRIVER)
	record_service_stop_status(&stop_status, "AliyunDrive WebDAV", stop_aldriver());
#endif
#if defined(APP_WIREGUARD)
	record_service_stop_status(&stop_status, "WireGuard", stop_wireguard());
#endif
#if defined(APP_ZEROTIER)
	record_service_stop_status(&stop_status, "ZeroTier", stop_zerotier());
#endif
#if defined(APP_SMARTDNS)
	record_service_stop_status(&stop_status, "SmartDNS", stop_smartdns());
#endif
#if defined(APP_ALIDDNS)
	record_service_stop_status(&stop_status, "AliDDNS", stop_aliddns());
#endif
#if defined(APP_TTYD)
	record_service_stop_status(&stop_status, "ttyd", stop_ttyd());
#endif
#if defined(APP_FRP)
	record_service_stop_status(&stop_status, "FRP", stop_frp());
#endif
	record_service_stop_status(&stop_status, "networkmap", stop_networkmap());
	record_service_stop_status(&stop_status, "lld2d", stop_lltd());
	record_service_stop_status(&stop_status, "detect_internet", stop_detect_internet());
	record_service_stop_status(&stop_status, "rstats", stop_rstats());
	record_service_stop_status(&stop_status, "infosvr", stop_infosvr());
	record_service_stop_status(&stop_status, "crond", stop_crond());
	record_service_stop_status(&stop_status, "IPTV proxies", stop_igmpproxy(NULL));
	/* Remaining optional package stop hooks still expose legacy void APIs. */
	return stop_status;
}

int
stop_services_lan_wan(void)
{
	int stop_status = 0;
	record_service_stop_status(&stop_status, "DNS/DHCP", stop_dns_dhcpd());
	record_service_stop_status(&stop_status, "UPnP", stop_upnp());
	record_service_stop_status(&stop_status, "detect_link", stop_detect_link());
#if defined (APP_SMBD) || defined (APP_NMBD)
	record_service_stop_status(&stop_status, "nmbd", stop_nmbd());
#endif
	return stop_status;
}

#if defined(APP_SKIPDBV2)
static int
stop_skipdbv2(void)
{
	char *svcs[] = { "skipd", NULL };

	/*
	 * SIGTERM exits skipd's event loop and commits/closes the database.
	 * Never SIGKILL this daemon: an unclean exit can lose persistent state.
	 */
	return kill_services(svcs, 30, 0);
}
#endif

int
stop_misc(void)
{
	char *svcs[] = { "ntpd", "detect_wan", "watchdog", "gpio_btn", NULL };
	int stop_status = kill_services(svcs, 4, 1);

#if defined(APP_SKIPDBV2)
	/* stop_misc is part of reboot, power-off and firmware-update teardown. */
	record_service_stop_status(&stop_status, "SkipDBv2", stop_skipdbv2());
#endif
	return stop_status;
}
