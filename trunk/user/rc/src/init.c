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
#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#define __USE_GNU
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysmacros.h>

#include "rc.h"
#include "init_router_result_policy.h"
#include "gpio_pins.h"
#include "pid1_signal_policy.h"
#include "pid1_console_policy.h"
#include "init_mdev_config_file.h"
#include "init_mdev_retry_policy.h"
#include "nvram_device_node_policy.h"
#include "tun_device_node_policy.h"
#include "boot_device_node_policy.h"

#define CONSOLE_TERMINAL	"vt100"
#define INIT_HOME_PATH		"/home/root"

static pid1_signal_state_t signal_state = { 0, 0, 0, 0, 0 };
static int noconsole = 0;

/* Synchronous faults cannot safely return to the faulting instruction. */
static const int signals_fatal[] = {
	SIGILL,
	SIGSEGV,
	SIGABRT,
	SIGFPE,
	SIGBUS,
	SIGSYS,
	SIGTRAP
};

/* Ordinary termination requests are handled later in the main loop. */
static const int signals_shutdown[] = {
	SIGPIPE,
	SIGPWR,
	SIGQUIT,
	SIGTERM
};

static const int signals_event[] = {
	SIGHUP,
	SIGUSR1,
	SIGUSR2,
	SIGALRM
};

static const char *const environment[] = {
	"PATH=" SYS_EXEC_PATH,
	"SHELL=" SYS_SHELL,
	"USER=" SYS_USER_ROOT,
	"HOME=" INIT_HOME_PATH,
	NULL
};

/* backward definition */
static int
reset_signals(void);

static void
handle_sig_fatal(int sig)
{
	int reset_status;

	printf("%s signal: %s\n", "fatal", strsignal(sig));

	if (sig == SIGQUIT) {
		shutdown_router(1);
		return;
	}

	reset_status = reset_signals();
	if (reset_status != 0)
		printf("failed to reset signal state before emergency shutdown (status=%d)\n",
		       reset_status);

	shutdown_router(0);

	kill(-1, SIGTERM);
	sleep(1);
	sync();

#ifdef MTD_FLASH_32M_REBOOT_BUG
	system("/bin/mtd_write -r unlock mtd1");
#else
	reboot(RB_AUTOBOOT);
#endif

	do {
		sleep(1);
	} while (1);
}

/*
 * Synchronous CPU faults cannot be deferred: returning would normally
 * re-enter the faulting instruction. Keep the legacy emergency shutdown
 * path for these signals; ordinary termination signals never use it.
 */
static void
catch_sig_fatal(int sig)
{
	pid1_signal_record_shutdown(&signal_state, sig);
	handle_sig_fatal(sig);
}

/* Async termination handlers only publish sig_atomic_t state. */
static void
catch_sig_shutdown(int sig)
{
	pid1_signal_record_shutdown(&signal_state, sig);
}

static void
catch_sig_event(int sig)
{
	pid1_signal_record_event(&signal_state, sig);
}

/* waitpid is async-signal-safe; stdio logging is deliberately omitted. */
static void
catch_sig_reap(int sig)
{
	pid_t pid;

	(void)sig;
	for (;;) {
		pid = waitpid(-1, NULL, WNOHANG);
		if (pid > 0) {
			pid1_signal_record_child_reaped(&signal_state, (sig_atomic_t)pid);
			continue;
		}
		if (pid < 0 && errno == EINTR)
			continue;
		break;
	}
}

/* Set terminal settings to reasonable defaults; reject partial setup. */
static int
setup_term(void)
{
	return pid1_console_configure(STDIN_FILENO, tcgetattr, tcsetattr);
}

static int
init_console(void)
{
	int fd;
	int ret = 0;
	int console_available = 1;
	int terminal_status;

	/* Clean up */
	ioctl(STDIN_FILENO, TIOCNOTTY, 0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	setsid();

	/* Reopen console */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		ret = errno ? errno : EIO;
		console_available = 0;
		perror(_PATH_CONSOLE);
		if ((fd = open("/dev/null", O_RDWR)) < 0) {
			perror("/dev/null");
			return errno ? errno : EIO;
		}
	}

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	ioctl(STDIN_FILENO, TIOCSCTTY, 1);
	tcsetpgrp(STDIN_FILENO, getpgrp());

	/* /dev/null is a deliberate fallback, not a terminal to configure. */
	if (console_available) {
		terminal_status = setup_term();
		if (terminal_status != 0) {
			logmessage(LOGNAME, "console termios setup failed (status=%d)", terminal_status);
			if (ret == 0)
				ret = terminal_status;
		}
	}

	return ret;
}

/* Reset signal handlers installed by PID 1 and release inherited masks. */
static int
reset_signals(void)
{
	int i;
	int first_status = 0;
	int status;
	sigset_t set;
	struct sigaction sa;

	/* Disable every handler installed by PID 1 except its SIGCHLD reaper. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	errno = 0;
	if (sigemptyset(&sa.sa_mask) < 0)
		return errno > 0 ? errno : EIO;

	for (i = 0; i < ARRAY_SIZE(signals_event); i++) {
		status = pid1_signal_install_action(sigaction, signals_event[i], &sa);
		first_status = pid1_signal_status_merge(first_status, status);
	}
	for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++) {
		status = pid1_signal_install_action(sigaction, signals_shutdown[i], &sa);
		first_status = pid1_signal_status_merge(first_status, status);
	}
	for (i = 0; i < ARRAY_SIZE(signals_fatal); i++) {
		status = pid1_signal_install_action(sigaction, signals_fatal[i], &sa);
		first_status = pid1_signal_status_merge(first_status, status);
	}

	/* A child or emergency path must not inherit PID 1's blocked signals. */
	errno = 0;
	if (sigfillset(&set) < 0)
		status = errno > 0 ? errno : EIO;
	else
		status = pid1_signal_update_mask(sigprocmask, SIG_UNBLOCK, &set, NULL);
	return pid1_signal_status_merge(first_status, status);
}

static pid_t
run_shell(int timeout, int nowait)
{
	pid_t pid;
	sigset_t child_mask, saved_mask;
	int child_mask_blocked = 0;
	int console_status;
	char tz[128];
	char *envp[] = {
		"TERM=" CONSOLE_TERMINAL,
		"TERMINFO=/usr/share/terminfo",
		"PATH=" SYS_EXEC_PATH,
		"SHELL=" SYS_SHELL,
		"USER=" SYS_USER_ROOT,
		"HOME=" INIT_HOME_PATH,
		tz,
		NULL
	};

	/* Wait for user input */
//	if (waitfor (STDIN_FILENO, timeout) <= 0)
//		return 0;

	/*
	 * Publish the child PID before allowing SIGCHLD to run, preventing a
	 * short-lived recovery shell from being reaped before it is tracked.
	 */
	if (nowait) {
		int mask_error;

		mask_error = pid1_signal_empty_set(sigemptyset, &child_mask);
		if (mask_error == 0)
			mask_error = pid1_signal_add_set(sigaddset, &child_mask, SIGCHLD);
		if (mask_error == 0)
			mask_error = pid1_signal_update_mask(
				sigprocmask, SIG_BLOCK, &child_mask, &saved_mask);
		if (mask_error != 0) {
			logmessage(LOGNAME,
				   "cannot prepare SIGCHLD mask before starting recovery shell (status=%d)",
				   mask_error);
			errno = mask_error;
			return -1;
		}
		child_mask_blocked = 1;
	}

	switch ((pid = fork())) {
	case -1: {
		int fork_error = errno ? errno : EIO;

		if (child_mask_blocked &&
		    sigprocmask(SIG_SETMASK, &saved_mask, NULL) < 0) {
			int mask_error = errno ? errno : EIO;

			logmessage(LOGNAME,
				   "cannot restore signal mask after recovery shell fork failure (status=%d)",
				   mask_error);
			if (sigprocmask(SIG_UNBLOCK, &child_mask, NULL) < 0)
				logmessage(LOGNAME,
					   "cannot unblock SIGCHLD after recovery shell fork failure (status=%d)",
					   errno ? errno : EIO);
		}
		errno = fork_error;
		perror("fork");
		return -1;
	}
	case 0:
		/* Do not enter console setup with the parent's signal state. */
		{
			int reset_status = reset_signals();

			if (reset_status != 0) {
				logmessage(LOGNAME,
					   "recovery shell aborted: cannot reset signal state (status=%d)",
					   reset_status);
				_exit(126);
			}
		}

		/* Do not execute an interactive shell without a usable console. */
		console_status = init_console();
		if (console_status != 0) {
			logmessage(LOGNAME,
				   "recovery shell aborted: console setup failed (status=%d)",
				   console_status);
			_exit(125);
		}

		/* Pass on TZ */
		time_zone_x_mapping();
		snprintf(tz, sizeof(tz), "TZ=%s", nvram_safe_get("time_zone_x"));

		/* Now run it.  The new program will take over this PID, 
		 * so nothing further in init.c should be run. */
		execve(SYS_SHELL, (char *[]) { SYS_SHELL, NULL }, envp);

		/* We're still here?  Some error happened. */
		perror(SYS_SHELL);
		exit(errno);
	default:
		if (nowait) {
			pid1_signal_record_console_shell(&signal_state, (sig_atomic_t)pid);
			if (child_mask_blocked &&
			    sigprocmask(SIG_SETMASK, &saved_mask, NULL) < 0) {
				int mask_error = errno ? errno : EIO;

				logmessage(LOGNAME,
					   "cannot restore SIGCHLD mask after recovery shell start (status=%d)",
					   mask_error);
				if (sigprocmask(SIG_UNBLOCK, &child_mask, NULL) < 0)
					logmessage(LOGNAME,
						   "cannot unblock SIGCHLD after recovery shell start (status=%d)",
						   errno ? errno : EIO);
			}
			return pid;
		}
		waitpid(pid, NULL, 0);
		return 0;
	}
}

static int
init_signal_mask_update(int how, const sigset_t *set, sigset_t *old_set,
			const char *operation)
{
	int status = pid1_signal_update_mask(sigprocmask, how, set, old_set);

	if (status != 0)
		logmessage(LOGNAME,
			   "PID 1 failed to %s (status=%d)",
			   operation, status);
	return status;
}

static int
init_signal_set_empty(sigset_t *set, const char *operation)
{
	int status = pid1_signal_empty_set(sigemptyset, set);

	if (status != 0)
		logmessage(LOGNAME, "PID 1 failed to %s (status=%d)",
			   operation, status);
	return status;
}

static int
init_signal_set_add(sigset_t *set, int signal_number, const char *operation)
{
	int status = pid1_signal_add_set(sigaddset, set, signal_number);

	if (status != 0)
		logmessage(LOGNAME,
			   "PID 1 failed to add signal %d while trying to %s (status=%d)",
			   signal_number, operation, status);
	return status;
}

static int
init_signal_mask_add_group(sigset_t *set, const int *signals, int count,
			   const char *operation)
{
	int i;
	int status;

	for (i = 0; i < count; i++) {
		status = init_signal_set_add(set, signals[i], operation);
		if (status != 0)
			return status;
	}
	return 0;
}

static int
control_signal(int sig, int how)
{
	sigset_t set;
	int status;

	status = init_signal_set_empty(&set, "initialize a control signal mask");
	if (status != 0)
		return status;
	status = init_signal_set_add(&set, sig, "initialize a control signal mask");
	if (status != 0)
		return status;

	return init_signal_mask_update(how, &set, NULL,
				       "update a control signal mask");
}

static int
init_signal_install_status(int first_status, int signal_number,
			   const struct sigaction *action)
{
	int status = pid1_signal_install_action(sigaction, signal_number, action);

	if (status != 0)
		logmessage(LOGNAME,
			   "failed to install PID 1 handler for signal %d (status=%d)",
			   signal_number, status);
	return pid1_signal_status_merge(first_status, status);
}

static int
init_signals(void)
{
	int i;
	int first_status = 0;
	int mask_status;
	struct sigaction sa;

	/* Prevent nested lifecycle handlers from observing partial state. */
	memset(&sa, 0, sizeof(sa));
	mask_status = init_signal_set_empty(&sa.sa_mask,
					    "initialize lifecycle signal mask");
	if (mask_status == 0)
		mask_status = init_signal_mask_add_group(
			&sa.sa_mask, signals_event, ARRAY_SIZE(signals_event),
			"initialize lifecycle signal mask");
	if (mask_status == 0)
		mask_status = init_signal_mask_add_group(
			&sa.sa_mask, signals_shutdown, ARRAY_SIZE(signals_shutdown),
			"initialize lifecycle signal mask");
	if (mask_status == 0)
		mask_status = init_signal_mask_add_group(
			&sa.sa_mask, signals_fatal, ARRAY_SIZE(signals_fatal),
			"initialize lifecycle signal mask");

	if (mask_status == 0) {
		/* Synchronous faults retain the emergency path; shutdown requests defer. */
		sa.sa_handler = catch_sig_fatal;
		for (i = 0; i < ARRAY_SIZE(signals_fatal); i++)
			first_status = init_signal_install_status(
				first_status, signals_fatal[i], &sa);

		sa.sa_handler = catch_sig_shutdown;
		for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++)
			first_status = init_signal_install_status(
				first_status, signals_shutdown[i], &sa);
	} else {
		first_status = pid1_signal_status_merge(first_status, mask_status);
	}

	/* SIGCHLD reaping performs waitpid only; no stdio in signal context. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_reap;
	mask_status = init_signal_set_empty(&sa.sa_mask,
					    "initialize SIGCHLD handler mask");
	if (mask_status == 0)
		first_status = init_signal_install_status(
			first_status, SIGCHLD, &sa);
	else
		first_status = pid1_signal_status_merge(first_status, mask_status);

	/* Event handlers only set their corresponding sig_atomic_t flags. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_event;
	mask_status = init_signal_set_empty(&sa.sa_mask,
					    "initialize event signal mask");
	if (mask_status == 0)
		mask_status = init_signal_mask_add_group(
			&sa.sa_mask, signals_event, ARRAY_SIZE(signals_event),
			"initialize event signal mask");
	if (mask_status == 0) {
		for (i = 0; i < ARRAY_SIZE(signals_event); i++)
			first_status = init_signal_install_status(
				first_status, signals_event[i], &sa);
	} else {
		first_status = pid1_signal_status_merge(first_status, mask_status);
	}

	return first_status;
}

static void
init_time(void)
{
	struct tm stm;
	time_t st;

	time(&st);
	localtime_r(&st, &stm);
#if defined (USE_RTC_HCTOSYS)
	/* check RTC is valid */
	if (stm.tm_year >= (SYS_START_YEAR - 1900))
		return;
#endif
	stm.tm_year = (SYS_START_YEAR - 1900);
	st = mktime(&stm);
	stime(&st);
}

static int
init_nvram_node(void)
{
	const dev_t nvram_device = makedev(NVRAM_DEVICE_NODE_MAJOR,
					   NVRAM_DEVICE_NODE_MINOR);
	struct stat st;
	int saved_errno;

	if (mknod(NVRAM_DEVICE_NODE_PATH,
		  S_IFCHR | NVRAM_DEVICE_NODE_MODE, nvram_device) != 0 &&
	    errno != EEXIST) {
		saved_errno = errno;
		logmessage(LOGNAME, "Failed to create NVRAM device node: %s",
			   strerror(saved_errno));
		return -saved_errno;
	}

	/* Never chmod a symlink, regular file or a different character device. */
	if (lstat(NVRAM_DEVICE_NODE_PATH, &st) != 0) {
		saved_errno = errno;
		logmessage(LOGNAME, "Failed to inspect NVRAM device node: %s",
			   strerror(saved_errno));
		return -saved_errno;
	}
	if (!S_ISCHR(st.st_mode) || st.st_rdev != nvram_device) {
		logmessage(LOGNAME,
			   "Refusing unsafe NVRAM device node (type/device mismatch)");
		return -EPERM;
	}

	if ((st.st_uid != (uid_t)0 || st.st_gid != (gid_t)0) &&
	    chown(NVRAM_DEVICE_NODE_PATH, 0, 0) != 0) {
		saved_errno = errno;
		logmessage(LOGNAME, "Failed to set NVRAM device ownership: %s",
			   strerror(saved_errno));
		return -saved_errno;
	}
	if ((st.st_mode & 07777) != NVRAM_DEVICE_NODE_MODE &&
	    chmod(NVRAM_DEVICE_NODE_PATH, NVRAM_DEVICE_NODE_MODE) != 0) {
		saved_errno = errno;
		logmessage(LOGNAME, "Failed to restrict NVRAM device permissions: %s",
			   strerror(saved_errno));
		return -saved_errno;
	}

	if (lstat(NVRAM_DEVICE_NODE_PATH, &st) != 0) {
		saved_errno = errno;
		logmessage(LOGNAME, "Failed to recheck NVRAM device node: %s",
			   strerror(saved_errno));
		return -saved_errno;
	}
	if (!nvram_device_node_is_secure(&st, nvram_device)) {
		logmessage(LOGNAME,
			   "NVRAM device node is not root:root 0600; router start blocked");
		return -EPERM;
	}

	return 0;
}

static int
init_openvpn_tun_node(void)
{
	struct stat st;
	const dev_t tun_device = makedev(TUN_DEVICE_NODE_MAJOR,
					 TUN_DEVICE_NODE_MINOR);
	int saved_errno;

	/* /dev/net must be a root-owned directory that untrusted users cannot alter. */
	if (lstat("/dev/net", &st) != 0) {
		if (errno != ENOENT)
			return -(errno ? errno : EIO);
		if (mkdir("/dev/net", 0755) != 0 && errno != EEXIST)
			return -(errno ? errno : EIO);
		if (lstat("/dev/net", &st) != 0)
			return -(errno ? errno : EIO);
	}
	if (!S_ISDIR(st.st_mode)) {
		/* Unlink a file or symlink itself; never follow it outside /dev. */
		if (unlink("/dev/net") != 0)
			return -(errno ? errno : EIO);
		if (mkdir("/dev/net", 0755) != 0)
			return -(errno ? errno : EIO);
		if (lstat("/dev/net", &st) != 0)
			return -(errno ? errno : EIO);
	}
	if (!S_ISDIR(st.st_mode))
		return -ENOTDIR;

	if (st.st_uid != (uid_t)0 || st.st_gid != (gid_t)0) {
		if (chown("/dev/net", 0, 0) != 0)
			return -(errno ? errno : EIO);
		if (lstat("/dev/net", &st) != 0)
			return -(errno ? errno : EIO);
	}
	/*
	 * OpenVPN's generated server config drops to nobody:nogroup. The path
	 * must therefore remain searchable by non-root users even after the
	 * daemon drops privileges or reopens the persistent TUN endpoint.
	 */
	if (!tun_device_directory_is_secure(&st)) {
		if (chmod("/dev/net", 0755) != 0)
			return -(errno ? errno : EIO);
	}
	if (lstat("/dev/net", &st) != 0)
		return -(errno ? errno : EIO);
	if (!tun_device_directory_is_secure(&st))
		return -EPERM;

	/* lstat is intentional: a symlink or ordinary file is not a TUN device. */
	if (lstat(TUN_DEVICE_NODE_PATH, &st) == 0) {
		if (tun_device_node_is_secure(&st, tun_device))
			return 0;
		if (S_ISDIR(st.st_mode)) {
			if (rmdir(TUN_DEVICE_NODE_PATH) != 0)
				return -(errno ? errno : EIO);
		} else if (unlink(TUN_DEVICE_NODE_PATH) != 0) {
			return -(errno ? errno : EIO);
		}
	} else if (errno != ENOENT) {
		return -(errno ? errno : EIO);
	}

	if (mknod(TUN_DEVICE_NODE_PATH, S_IFCHR | TUN_DEVICE_NODE_MODE,
		  tun_device) != 0) {
		saved_errno = errno ? errno : EIO;
		/* A concurrent root setup is acceptable only after full validation. */
		if (saved_errno == EEXIST &&
		    lstat(TUN_DEVICE_NODE_PATH, &st) == 0 &&
		    tun_device_node_is_secure(&st, tun_device))
			return 0;
		return -saved_errno;
	}
	if (chown(TUN_DEVICE_NODE_PATH, 0, 0) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(TUN_DEVICE_NODE_PATH);
		return -saved_errno;
	}
	if (chmod(TUN_DEVICE_NODE_PATH, TUN_DEVICE_NODE_MODE) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(TUN_DEVICE_NODE_PATH);
		return -saved_errno;
	}
	if (lstat(TUN_DEVICE_NODE_PATH, &st) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(TUN_DEVICE_NODE_PATH);
		return -saved_errno;
	}
	if (!tun_device_node_is_secure(&st, tun_device)) {
		unlink(TUN_DEVICE_NODE_PATH);
		return -EPERM;
	}

	return 0;
}

#if defined(USE_HW_NAT) || defined(USE_MTK_AES)
static int
init_optional_device_node(const char *path, dev_t device)
{
	struct stat st;
	int saved_errno;

	if (!path || path[0] != '/')
		return -EINVAL;

	if (mknod(path, S_IFCHR | RC_BOOT_DEVICE_NODE_MODE, device) != 0 &&
	    errno != EEXIST) {
		saved_errno = errno ? errno : EIO;
		return -saved_errno;
	}

	/* EEXIST is safe only when the existing path is the exact device. */
	if (lstat(path, &st) != 0) {
		saved_errno = errno ? errno : EIO;
		return -saved_errno;
	}
	if (!rc_boot_device_node_is_secure(&st, device))
		return -EPERM;

	return 0;
}
#endif

#if defined(USE_HW_NAT) || defined(USE_MTK_AES) || defined(APP_OPENVPN)
static int
init_nodes_record_optional_status(int first_status, const char *path,
				  int status)
{
	if (status != 0) {
		logmessage(LOGNAME,
			   "optional boot device %s is unavailable or unsafe (status=%d); dependent feature may be unavailable",
			   path, status);
		if (first_status == 0)
			first_status = status;
	}
	return first_status;
}
#endif

static int
init_nodes(int *optional_status)
{
	int nvram_result;
	int first_optional_status = 0;

	nvram_result = init_nvram_node();
#if defined (USE_HW_NAT)
	{
		int status = init_optional_device_node("/dev/hwnat0",
						      makedev(220, 0));
		first_optional_status = init_nodes_record_optional_status(
			first_optional_status, "/dev/hwnat0", status);
	}
#endif
#if defined (USE_MTK_AES)
	{
		int status = init_optional_device_node("/dev/crypto",
						      makedev(10, 60));
		first_optional_status = init_nodes_record_optional_status(
			first_optional_status, "/dev/crypto", status);
	}
#endif
#if defined (APP_OPENVPN)
	{
		int tun_status = init_openvpn_tun_node();
		first_optional_status = init_nodes_record_optional_status(
			first_optional_status, TUN_DEVICE_NODE_PATH, tun_status);
	}
#endif
	if (optional_status)
		*optional_status = first_optional_status;

	return nvram_result;
}


static int
init_mdev(void)
{
	FILE *fp = NULL;
	char staged_path[PATH_MAX];
	const char *mdev_conf = "/etc/mdev.conf";
	int ret;
	int hotplug_result;
	int hotplug_errno;

	ret = init_mdev_config_open(mdev_conf, staged_path,
				    sizeof(staged_path), &fp);
	if (ret != 0) {
		logmessage(LOGNAME, "Failed to stage mdev configuration: %s",
			   strerror(-ret));
		return ret;
	}

	fprintf(fp, "%s\n", "# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]");
#if defined (USE_MMC_SUPPORT)
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "mmcblk[0-9]",  "*", "mdev_mmc");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "mmcblk[0-9]p[0-9]", "*", "mdev_mmc");
#endif
#if defined (USE_BLK_DEV_SD)
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sd[a-z]",      "*", "mdev_sd");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sd[a-z][0-9]", "*", "mdev_sd");
#endif
#if defined (USE_USB_SUPPORT)
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "usb/lp[0-9]",  "*", "mdev_lp");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sg[0-9]",      "@", "mdev_sg");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sr[0-9]",      "@", "mdev_sr");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "weth[0-9]",    "*", "mdev_net");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "wwan[0-9]",   "*", "mdev_net");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "cdc-wdm[0-9]", "*", "mdev_wdm");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "ttyUSB[0-9]",  "*", "mdev_tty");
	fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "ttyACM[0-9]",  "*", "mdev_tty");
	fprintf(fp, "%s 0:0 %s\n", "video[0-9]", "0660");
#endif
	fprintf(fp, "%s 0:0 %s\n", "null", "0666");
	fprintf(fp, "%s 0:0 %s\n", "zero", "0666");
	fprintf(fp, "%s 0:0 %s\n", "full", "0666");
	fprintf(fp, "%s 0:0 %s\n", "random", "0666");
	fprintf(fp, "%s 0:0 %s\n", "urandom", "0444");

	ret = init_mdev_config_publish(&fp, staged_path, mdev_conf);
	if (ret != 0) {
		logmessage(LOGNAME, "Failed to publish mdev configuration (%s); staging path: %s",
			   strerror(-ret), staged_path);
		return ret;
	}

	errno = 0;
	hotplug_result = fput_string("/proc/sys/kernel/hotplug", "/sbin/mdev");
	if (hotplug_result != 0) {
		hotplug_errno = (hotplug_result > 0) ? hotplug_result :
				((errno > 0) ? errno : EIO);
		logmessage(LOGNAME, "Failed to activate mdev kernel hotplug handler: %s",
			   strerror(hotplug_errno));
		return -hotplug_errno;
	}

	return 0;
}
static int
init_sysctl_record_status(int first_status, const char *setting, int status)
{
	if (status != 0) {
		logmessage(LOGNAME,
			   "early boot sysctl initialization failed for %s (status=%d)",
			   setting, status);
		if (first_status == 0)
			first_status = status;
	}
	return first_status;
}

static int
init_sysctl(void)
{
	int first_status = 0;
	int status;

	status = fput_int("/proc/sys/kernel/panic", 1);
	first_status = init_sysctl_record_status(first_status, "/proc/sys/kernel/panic", status);

	status = fput_int("/proc/sys/net/core/rmem_max", KERNEL_NET_CORE_RMEM);
	first_status = init_sysctl_record_status(first_status, "/proc/sys/net/core/rmem_max", status);
	status = fput_int("/proc/sys/net/core/wmem_max", KERNEL_NET_CORE_WMEM);
	first_status = init_sysctl_record_status(first_status, "/proc/sys/net/core/wmem_max", status);

	status = set_interface_conf_int("ipv4", "all", "rp_filter", 0);
	first_status = init_sysctl_record_status(first_status, "net.ipv4.conf.all.rp_filter", status);

	status = fput_int("/proc/sys/vm/min_free_kbytes", KERNEL_MIN_FREE_KBYTES);
	first_status = init_sysctl_record_status(first_status, "/proc/sys/vm/min_free_kbytes", status);
	status = fput_int("/proc/sys/vm/overcommit_memory", 0);
	first_status = init_sysctl_record_status(first_status, "/proc/sys/vm/overcommit_memory", status);

	status = set_tcp_tweaks();
	first_status = init_sysctl_record_status(first_status, "TCP performance sysctls", status);

#if defined (USE_IPV6)
	status = control_if_ipv6_all(0);
	first_status = init_sysctl_record_status(first_status, "net.ipv6.conf.{default,all}.disable_ipv6", status);
#else
	status = set_libc_gai(1);
	first_status = init_sysctl_record_status(first_status, "/etc/gai.conf", status);
#endif
	return first_status;
}

/* Main loop */
void
init_main_loop(void)
{
	int shutdown_handled = 0;
	int router_ready;
	int router_init_status = INIT_ROUTER_RESULT_FATAL;
	int nvram_node_result;
	int optional_node_status = 0;
	int dev_init_status;
	int sysctl_status;
	int mdev_status;
	int signal_status;
	int mask_status;
	int wait_mask_build_status = 0;
	int wait_mask_ready = 0;
	int original_mask_ready = 0;
	init_mdev_retry_state_t mdev_retry_state = { 0 };
	int i;
	sigset_t wait_mask, original_mask;

	umask(0000);

	/* Basic initialization */
	init_time();
#if BOARD_RAM_SIZE > 32
	dev_init_status = system("dev_init.sh");
#else
	dev_init_status = system("dev_init.sh -l");
#endif
	if (dev_init_status != 0)
		logmessage(LOGNAME,
			   "required device/filesystem initialization failed (system status=%d); router startup disabled",
			   dev_init_status);
	nvram_node_result = init_nodes(&optional_node_status);
	if (nvram_node_result != 0)
		logmessage(LOGNAME, "NVRAM device is unavailable or unsafe; router startup remains in recovery mode");
	mdev_status = init_mdev();
	init_mdev_retry_state_begin(&mdev_retry_state, dev_init_status,
				    mdev_status);
	if (mdev_status != 0) {
		if (init_mdev_retry_state_is_pending(&mdev_retry_state))
			logmessage(LOGNAME,
				   "mdev initialization failed (status=%d); deferred retry scheduled",
				   mdev_status);
		else
			logmessage(LOGNAME,
				   "mdev initialization failed (status=%d); hotplug handling may be unavailable",
				   mdev_status);
	}
	sysctl_status = init_sysctl();
	/* Optional accelerator/TUN failures degrade boot, but NVRAM remains fatal. */
	if (sysctl_status == 0)
		sysctl_status = optional_node_status;

	/* Setup console */
	if (init_console())
		noconsole = 1;

	chdir("/");
	setsid();

	/* Setup shell environment */
	{
		const char *const *e;
		/* Make sure environs is set to something sane */
		for (e = environment; *e; e++)
			putenv((char *) *e);
	}

	/* Setup signal handlers */
	signal_status = init_signals();
	if (signal_status != 0)
		logmessage(LOGNAME,
			   "PID 1 signal setup incomplete (status=%d); router startup disabled",
			   signal_status);

	/* Schedule retry only after SIGALRM handler installation. */
	if (signal_status == 0 && init_mdev_retry_state_is_pending(&mdev_retry_state))
		alarm(INIT_MDEV_RETRY_DELAY_SECONDS);

	/* block SIGUSR1 during init_router(); mask failures gate router startup. */
	mask_status = control_signal(SIGUSR1, SIG_BLOCK);
	signal_status = pid1_signal_status_merge(signal_status, mask_status);

	/* SIGUSR2 remains a reserved, currently no-op event signal. */
	mask_status = control_signal(SIGUSR2, SIG_BLOCK);
	signal_status = pid1_signal_status_merge(signal_status, mask_status);

	/* Required host prerequisites gate router init; service errors are degraded, not fatal. */
	if (dev_init_status == 0 && nvram_node_result == 0 && signal_status == 0)
		router_init_status = init_router();
	router_init_status = init_router_result_merge_sysctl_status(
		router_init_status, sysctl_status);
	router_ready = (dev_init_status == 0 &&
		       nvram_node_result == 0 &&
		       signal_status == 0 &&
		       init_router_status_is_ready(router_init_status));
	if (!router_ready)
		logmessage(LOGNAME, "Router startup failed; console recovery remains available");
	else if (init_router_status_is_degraded(router_init_status))
		logmessage(LOGNAME,
			   "Router startup reached ready state with degraded service status; PID 1 event handling remains enabled");

	/* Unblock SIGUSR1 after startup; failed delivery disables notifications. */
	mask_status = control_signal(SIGUSR1, SIG_UNBLOCK);
	if (mask_status != 0) {
		signal_status = pid1_signal_status_merge(signal_status, mask_status);
		router_ready = 0;
		logmessage(LOGNAME,
			   "PID 1 cannot unblock SIGUSR1; router notifications disabled (status=%d)",
			   mask_status);
	}

	/*
	 * Build the queued-work mask before using it. A failed mask operation
	 * falls back to polling; never pass an uninitialized old mask to
	 * sigsuspend().
	 */
	errno = 0;
	if (sigemptyset(&wait_mask) < 0)
		wait_mask_build_status = errno > 0 ? errno : EIO;
	if (wait_mask_build_status == 0) {
		errno = 0;
		if (sigaddset(&wait_mask, SIGUSR1) < 0)
			wait_mask_build_status = errno > 0 ? errno : EIO;
		errno = 0;
		if (sigaddset(&wait_mask, SIGALRM) < 0 &&
		    wait_mask_build_status == 0)
			wait_mask_build_status = errno > 0 ? errno : EIO;
		for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++) {
			errno = 0;
			if (sigaddset(&wait_mask, signals_shutdown[i]) < 0 &&
			    wait_mask_build_status == 0)
				wait_mask_build_status = errno > 0 ? errno : EIO;
		}
	}
	if (wait_mask_build_status == 0) {
		mask_status = init_signal_mask_update(SIG_BLOCK, &wait_mask,
						     &original_mask,
						     "block queued work signals");
		if (mask_status == 0) {
			wait_mask_ready = 1;
			original_mask_ready = 1;
		}
	} else {
		logmessage(LOGNAME,
			   "PID 1 could not build queued-work signal mask; using recovery polling (status=%d)",
			   wait_mask_build_status);
	}

	/* Loop forever */
	for (;;) {
		/* Wait until an event or a deferred shutdown request is pending. */
		while (!signal_state.usr1_pending &&
		       !signal_state.alarm_pending &&
		       (!signal_state.shutdown_pending || shutdown_handled)) {
			if (!wait_mask_ready) {
				if (wait_mask_build_status != 0) {
					sleep(1);
					continue;
				}
				mask_status = init_signal_mask_update(
					SIG_BLOCK, &wait_mask,
					original_mask_ready ? NULL : &original_mask,
					"retry blocking queued work signals");
				if (mask_status != 0) {
					sleep(1);
					continue;
				}
				original_mask_ready = 1;
				wait_mask_ready = 1;
			}
			if (!noconsole && !signal_state.console_shell_pid) {
				/* Keep console setup from starving a signal that arrives now. */
				mask_status = init_signal_mask_update(
					SIG_SETMASK, &original_mask, NULL,
					"restore original mask for console shell");
				if (mask_status != 0) {
					wait_mask_ready = 0;
					sleep(1);
					continue;
				}
				wait_mask_ready = 0;
				if (run_shell(0, 1) < 0)
					sleep(1);
				mask_status = init_signal_mask_update(
					SIG_BLOCK, &wait_mask, NULL,
					"reblock queued work signals after console shell");
				if (mask_status == 0)
					wait_mask_ready = 1;
				else
					sleep(1);
			} else {
				errno = 0;
				if (sigsuspend(&original_mask) < 0 && errno != EINTR) {
					mask_status = errno > 0 ? errno : EIO;
					wait_mask_ready = 0;
					logmessage(LOGNAME,
						   "PID 1 sigsuspend failed; retrying mask setup (status=%d)",
						   mask_status);
					sleep(1);
				}
			}
		}

		/* Ordinary termination signals are handled outside signal context. */
		if (signal_state.shutdown_pending && !shutdown_handled) {
			shutdown_handled = 1;
			handle_sig_fatal((int)signal_state.shutdown_signal);
			continue;
		}

		/* Handle SIGUSR1 signal */
		if (signal_state.usr1_pending) {
			signal_state.usr1_pending = 0;
			if (router_ready)
				handle_notifications();
		}

		/* Handle SIGALRM signal */
		if (signal_state.alarm_pending) {
			signal_state.alarm_pending = 0;
			if (init_mdev_retry_state_is_pending(&mdev_retry_state)) {
				/*
				 * dev_init.sh already completed mdev -s. Retry only
				 * policy/handler publication; do not replay coldplug adds.
				 */
				mdev_status = init_mdev();
				if (init_mdev_retry_state_record_result(&mdev_retry_state,
								mdev_status)) {
					logmessage(LOGNAME,
						   "deferred mdev initialization failed (status=%d); retrying",
						   mdev_status);
					alarm(INIT_MDEV_RETRY_DELAY_SECONDS);
				} else {
					logmessage(LOGNAME,
						   "mdev kernel hotplug handler restored by deferred retry");
				}
			}
#if defined (USE_NAND_FLASH)
			if (router_ready && retry_rwfs_optware() != 0)
				alarm(5);
#endif
#if defined (USE_USB_SUPPORT) || defined (USE_STORAGE)
			if (router_ready)
				rc_notify_retry_deferred_hotplug_events();
			if (router_ready)
				on_deferred_hotplug_dev();
#endif
		}
	}
}

int
sys_exit(void)
{
	return kill(1, SIGTERM);
}

int
sys_stop(void)
{
	return kill(1, SIGQUIT);
}
