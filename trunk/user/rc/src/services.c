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
#include "process_inventory.h"
#include "optional_daemon_policy.h"
#include "logger_start_policy.h"
#include "service_start_status.h"
#include "watchdog_cpu_restart_policy.h"

static int optional_daemon_start_status(const char *service_name, const char *process_name);
static int optional_daemon_stop_status(const char *service_name, const char *process_name, int command_status);
#include "sshd_daemon_policy.h"
#include "networkmap_daemon_policy.h"
#include "crond_daemon_policy.h"
#include "infosvr_daemon_policy.h"
#include "httpd_daemon_policy.h"

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

	return _eval(syslogd_argv, NULL, 0, NULL);
}

int
start_klogd(void)
{
	return eval("/sbin/klogd");
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
	int start_status;

	start_status = optional_daemon_start_status("infosvr", "infosvr");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	if (nvram_invmatch("adsc_enable", "1"))
		return 1;

	start_status = eval("/usr/sbin/infosvr", IFNAME_BR);
	if (start_status != 0)
		logmessage("infosvr", "infosvr launch failed (status=%d)",
			   start_status);
	return start_status;
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
	int launch_status, attempt, status, scan_error;
	enum process_presence presence;

	if (nvram_invmatch("crond_enable", "1"))
		return 1;

	if (nvram_match("crond_log", "0"))
		crond_argv[1] = "-d8";

	setenv_tz();

	/*
	 * BusyBox crond exits its launcher successfully after forking. The
	 * detached child still has to enter the spool directory, scan jobs
	 * and publish its pidfile, so _eval() alone is not a daemon postcondition.
	 */
	launch_status = _eval(crond_argv, NULL, 0, NULL);
	if (launch_status != 0) {
		logmessage(LOGNAME, "crond launcher failed: %d", launch_status);
		return crond_start_postcondition(launch_status, PROCESS_PRESENCE_ABSENT);
	}

	sleep(CROND_START_INITIAL_WAIT_SECONDS);
	status = -1;
	for (attempt = 0; attempt < CROND_START_PROBE_ATTEMPTS; attempt++) {
		presence = crond_process_presence(&scan_error);
		status = crond_start_postcondition(launch_status, presence);
		if (presence == PROCESS_PRESENCE_RUNNING)
			return 0;
		if (presence == PROCESS_PRESENCE_UNKNOWN) {
			logmessage(LOGNAME,
				   "cannot verify crond startup postcondition (scan_error=%d)",
				   scan_error ? scan_error : EIO);
			break;
		}

		if (attempt + 1 < CROND_START_PROBE_ATTEMPTS)
			sleep(CROND_START_RETRY_WAIT_SECONDS);
	}

	logmessage(LOGNAME, "crond daemon did not remain running after launch");
	return status;
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
	int scan_error;

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

	return eval("/usr/sbin/networkmap", (first_call) ? "-w" : "");
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
	int start_status;

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

	start_status = eval("telnetd");
	if (start_status != 0)
		logmessage(LOGNAME, "telnetd launcher failed (status=%d)", start_status);

	return start_status;
}

#if defined(APP_SSHD)
static enum process_presence
sshd_process_presence(int *scan_error)
{
	struct process_snapshot snapshot;
	enum process_presence presence;
	const char *process_name = NULL;

	if (check_if_file_exist("/usr/bin/dropbearmulti"))
		process_name = "dropbear";
	else if (check_if_file_exist("/usr/sbin/sshd"))
		process_name = "sshd";

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
	return eval("/usr/bin/sshd.sh", "stop");
}

int start_sshd(void)
{
	int sshd_mode = nvram_get_int("sshd_enable");
	enum process_presence state;
	int scan_error = 0;

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
		return eval("/usr/bin/sshd.sh", "start", "-s");
	return eval("/usr/bin/sshd.sh", "start");
}

int restart_sshd(void)
{
	enum process_presence before_state;
	enum process_presence stopped_state;
	enum process_presence after_state;
	int scan_error;
	int stop_status;
	int start_status;

	before_state = sshd_process_presence(NULL);
	stop_status = stop_sshd();
	if (stop_status != 0) {
		logmessage(LOGNAME, "SSH restart aborted because the previous daemon did not stop (status=%d)",
			   stop_status);
		return stop_status;
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

	start_status = start_sshd();
	after_state = sshd_process_presence(&scan_error);

	if (sshd_firewall_refresh_required(before_state, after_state) &&
	    nvram_match("sshd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();

	if (start_status != 0)
		return start_status;
	if (after_state == PROCESS_PRESENCE_UNKNOWN) {
		errno = scan_error > 0 ? scan_error : EIO;
		logmessage(LOGNAME, "SSH startup state is unknown (scan_error=%d)", scan_error);
		return -1;
	}
	return 0;
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

int start_scutclient(void)
{
	int mode = nvram_get_int("scutclient_enable");
	int start_status;

	if (mode != 1)
		return 0;

	start_status = optional_daemon_start_status("scutclient", "bin_scutclient");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/scutclient.sh", "start");
}

int restart_scutclient(void)
{
	int stop_status = stop_scutclient();

	if (stop_status != 0)
		return stop_status;
	return start_scutclient();
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

int start_mentohust(void)
{
	int mode = nvram_get_int("mentohust_enable");
	int start_status;

	if (mode != 1)
		return 0;

	start_status = optional_daemon_start_status("mentohust", "bin_mentohust");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/mentohust.sh", "start");
}

int restart_mentohust(void)
{
	int stop_status = stop_mentohust();

	if (stop_status != 0)
		return stop_status;
	return start_mentohust();
}
#endif

#if defined(APP_TTYD)
int stop_ttyd(void){
	return eval("/usr/bin/ttyd.sh","stop");
}

int
start_ttyd(void)
{
	int ttyd_mode = nvram_get_int("ttyd_enable");
	int start_status;

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

	start_status = eval("/usr/bin/ttyd.sh", "start");
	if (start_status != 0)
		logmessage(LOGNAME, "ttyd launcher failed (status=%d)", start_status);

	return start_status;
}

int restart_ttyd(void){
	int stop_status = stop_ttyd();

	if (stop_status != 0) {
		logmessage(LOGNAME, "ttyd restart aborted because stop failed");
		return stop_status;
	}
	return start_ttyd();
}
#endif

#if defined(APP_SHADOWSOCKS)
int stop_ss(void){
	return eval("/usr/bin/shadowsocks.sh","stop");
}

int start_ss(void){
	int ss_enable = nvram_get_int("ss_enable");
	if (ss_enable != 1)
		return 0;
	return eval("/usr/bin/shadowsocks.sh","start");
}

int restart_ss(void){
	int stop_status = stop_ss();

	if (stop_status != 0) {
		logmessage(LOGNAME, "ss restart aborted because stop failed");
		return stop_status;
	}
	return start_ss();
}

int stop_ss_tunnel(void){
	return eval("/usr/bin/ss-tunnel.sh","stop");
}

int start_ss_tunnel(void){
	int ss_tunnel_mode = nvram_get_int("ss-tunnel_enable");
	if (ss_tunnel_mode != 1)
		return 0;
	return eval("/usr/bin/ss-tunnel.sh","start");
}

int restart_ss_tunnel(void){
	int stop_status = stop_ss_tunnel();

	if (stop_status != 0) {
		logmessage(LOGNAME, "ss_tunnel restart aborted because stop failed");
		return stop_status;
	}
	return start_ss_tunnel();
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

int start_vlmcsd(void)
{
	int mode = nvram_get_int("vlmcsd_enable");
	int start_status;

	if (mode != 1)
		return 0;

	start_status = optional_daemon_start_status("vlmcsd", "vlmcsd");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/vlmcsd.sh", "start");
}

int restart_vlmcsd(void)
{
	int stop_status = stop_vlmcsd();

	if (stop_status != 0)
		return stop_status;
	return start_vlmcsd();
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

int start_iperf3(void)
{
	int mode = nvram_get_int("iperf3_enable");
	int start_status;

	if (mode != 1)
		return 0;

	start_status = optional_daemon_start_status("iperf3", "iperf3");
	if (start_status > 0)
		return 0;
	if (start_status < 0)
		return start_status;

	return eval("/usr/bin/iperf3.sh", "start");
}

int restart_iperf3(void)
{
	int stop_status = stop_iperf3();

	if (stop_status != 0)
		return stop_status;
	return start_iperf3();
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
	int adg_mode = nvram_get_int("adg_enable");
	if ( adg_mode == 1) {
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
	int zerotier_enable = nvram_get_int("zerotier_enable");
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
	int ddnsto_enable = nvram_get_int("ddnsto_enable");
	if ( ddnsto_enable == 1) {
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
	int sqm_mode = nvram_get_int("sqm_enable");
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
	int aldriver_enable = nvram_get_int("aliyundrive_enable");
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
	int wireguard_enable = nvram_get_int("wireguard_enable");
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
	int adbyby_mode = nvram_get_int("adbyby_enable");
	if ( adbyby_mode == 1) {
		start_status = eval("/usr/bin/adbyby.sh","start");
		if (start_status != 0)
			logmessage(LOGNAME, "Adbyby launcher failed (status=%d)", start_status);
	}
}

int restart_adbyby(void){
	/* One launcher owns stop/start atomically with cron and manual actions. */
	return eval("/usr/bin/adbyby.sh","restart");
}

void update_adb(void){
	eval("/usr/bin/adbyby.sh","updateadb");
}
#endif

#if defined(APP_SMARTDNS)
int stop_smartdns(void){
	return eval("/usr/bin/smartdns.sh","stop");
}

int start_smartdns(void){
	int smartdns_mode = nvram_get_int("sdns_enable");
	if (smartdns_mode != 1)
		return 0;
	return eval("/usr/bin/smartdns.sh","start");
}

int restart_smartdns(void){
	int stop_status = stop_smartdns();

	if (stop_status != 0) {
		logmessage(LOGNAME, "smartdns restart aborted because stop failed");
		return stop_status;
	}
	return start_smartdns();
}
#endif

#if defined(APP_ALIDDNS)
int stop_aliddns(void){
	return eval("/usr/bin/aliddns.sh","stop");
}

void start_aliddns(void){
	int start_status;
	int aliddns_mode = nvram_get_int("aliddns_enable");
	if ( aliddns_mode == 1) {
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

int
start_httpd(int restart_fw)
{
	int http_proto, argv_index, http_port, restart_fw_need;
	int launch_status, attempt, status, scan_error;
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
			nvram_set_int_temp("httpd_started", 1);
			logmessage(LOGNAME, "HTTPD is already running; refusing duplicate launch");
		} else {
			logmessage(LOGNAME,
				   "cannot verify HTTPD process state; refusing startup (scan_error=%d)",
				   scan_error ? scan_error : EIO);
		}
		return status;
	}

#if defined (SUPPORT_HTTPS)
	http_proto = nvram_get_int("http_proto");
#else
	http_proto = 0;
#endif
	if (http_proto == 0 || http_proto == 2) {

		http_port = nvram_get_int("http_lanport");
		if (http_port < 80 || http_port > 65535) {
			http_port = 80;
			nvram_set_int("http_lanport", http_port);
		}

		sprintf(http_port_s, "%d", http_port);

		httpd_argv[argv_index++] = "-p";
		httpd_argv[argv_index++] = http_port_s;

		restart_fw_need |= nvram_get_int("misc_http_x");
	}

#if defined (SUPPORT_HTTPS)
	if (http_proto == 1 || http_proto == 2) {
		int https_port = nvram_get_int("https_lport");
		if (https_port < 81 || https_port > 65535 || https_port == http_port) {
			https_port = 443;
			nvram_set_int("https_lport", https_port);
		}

		sprintf(https_port_s, "%d", https_port);

		httpd_argv[argv_index++] = "-s";
		httpd_argv[argv_index++] = https_port_s;

		restart_fw_need |= nvram_get_int("https_wopen");
	}
#endif

	/* Clear stale state only after the earlier complete scan confirmed ABSENT. */
	nvram_set_int_temp("httpd_started", 0);

	launch_status = _eval(httpd_argv, NULL, 0, NULL);
	if (launch_status != 0) {
		logmessage(LOGNAME, "HTTPD launcher failed: %d", launch_status);
		return httpd_start_postcondition(launch_status, PROCESS_PRESENCE_ABSENT);
	}

	/*
	 * HTTPD can daemonize after _eval()'s launcher exits. Publish service
	 * state and open firewall ports only after the child remains observable.
	 */
	sleep(HTTPD_START_INITIAL_WAIT_SECONDS);
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

		if (attempt + 1 < HTTPD_START_PROBE_ATTEMPTS)
			sleep(HTTPD_START_RETRY_WAIT_SECONDS);
	}

	if (status != 0) {
		logmessage(LOGNAME, "HTTPD daemon did not remain running after launch");
		return status;
	}

	nvram_set_int_temp("httpd_started", 1);

	if (restart_fw && restart_fw_need && nvram_match("fw_enable_x", "1"))
		restart_firewall();

	return 0;
}

int
stop_httpd(void)
{
	char* svcs[] = { "httpd", NULL };
	int stop_status = kill_services(svcs, 3, 1);

	if (!httpd_stop_policy_publish_stopped(stop_status)) {
		logmessage(LOGNAME, "HTTPD did not stop cleanly; retaining service state (status=%d)",
			   stop_status);
		return stop_status;
	}

	nvram_set_int_temp("httpd_started", 0);
	return 0;
}

int
restart_httpd(void)
{
	int stop_status = stop_httpd();

	if (!httpd_restart_can_start(stop_status)) {
		logmessage(LOGNAME, "refusing HTTPD restart: previous daemon did not stop (status=%d)",
			   stop_status);
		return stop_status;
	}

	return start_httpd(1);
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
	int start_status;

	if (nvram_invmatch("rstats_enable", "1"))
		return 1;

	start_status = eval("/sbin/rstats");
	if (start_status != 0)
		logmessage(LOGNAME, "rstats daemon launch failed (status=%d)", start_status);

	return start_status;
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
	int start_status;

	if (nvram_invmatch("lltd_enable", "1"))
		return 1;

	start_status = eval("/bin/lld2d", IFNAME_BR);
	if (start_status != 0)
		logmessage(LOGNAME, "LLTD daemon launch failed (status=%d)", start_status);

	return start_status;
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

	/* Attempt both daemons even if the first launch fails. */
	syslog_status = start_syslogd();

	if (showinfo)
	{
		// wait for logger daemon started
		usleep(300000);
		
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

	return rc_logger_start_status(syslog_status, klog_status);
}

int
stop_logger(void)
{
	char* svcs[] = { "klogd", "syslogd", NULL };
	return kill_services(svcs, 3, 1);
}

int
start_watchdog_cpu(void)
{
	if (nvram_get_int("watchdog_cpu") == 0)
		return 0;
	/*
	 * module_smart_load() returns zero for both "already loaded" and
	 * "load failed". Verify module state instead of treating its return as
	 * a boolean success flag.
	 */
	(void)module_smart_load("rt_timer_wdg", NULL);
	if (!is_module_loaded("rt_timer_wdg")) {
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
	return is_module_loaded(module_name);
}

static const rc_watchdog_cpu_restart_ops_t watchdog_cpu_restart_ops = {
	watchdog_cpu_restart_module_load,
	watchdog_cpu_restart_module_unload,
	watchdog_cpu_restart_module_is_loaded
};

int
restart_watchdog_cpu(void)
{
	int should_load = (nvram_get_int("watchdog_cpu") != 0);
	int status = rc_watchdog_cpu_restart_verified("rt_timer_wdg", should_load,
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

int
start_services_once(int is_ap_mode)
{
	int start_status = 0;

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
			br_set_stp(IFNAME_BR, 1);
			br_set_fd(IFNAME_BR, 15);
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
		int skipdb_start_status = doSystem("/usr/sbin/skipd -d /etc/storage/db");
		if (skipdb_start_status != 0)
			logmessage(LOGNAME, "SkipDBv2 boot startup failed (wait status=%d)",
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
#if defined(APP_VLMCSD)
	record_service_stop_status(&stop_status, "vlmcsd", stop_vlmcsd());
#endif
#if defined(APP_IPERF3)
	record_service_stop_status(&stop_status, "iperf3", stop_iperf3());
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

int
stop_misc(void)
{
	char* svcs[] = { "ntpd", "detect_wan", "watchdog", NULL };
	return kill_services(svcs, 3, 1);
}
