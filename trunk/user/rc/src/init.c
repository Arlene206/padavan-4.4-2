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
#include "init_mdev_config_file.h"
#include "init_mdev_retry_policy.h"
#include "nvram_device_node_policy.h"
#include "tun_device_node_policy.h"

#define CONSOLE_TERMINAL	"vt100"
#define INIT_HOME_PATH		"/home/root"

static pid1_signal_state_t signal_state = { 0, 0, 0, 0 };
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
static void
reset_signals(void);

static void
handle_sig_fatal(int sig)
{
	printf("%s signal: %s\n", "fatal", strsignal(sig));

	if (sig == SIGQUIT) {
		shutdown_router(1);
		return;
	}

	reset_signals();

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
		if (pid > 0)
			continue;
		if (pid < 0 && errno == EINTR)
			continue;
		break;
	}
}

/* Set terminal settings to reasonable defaults */
static void
setup_term(void)
{
	struct termios tty;

	tcgetattr(STDIN_FILENO, &tty);

	/* set control chars */
	tty.c_cc[VINTR]  = 3;	/* C-c */
	tty.c_cc[VQUIT]  = 28;	/* C-\ */
	tty.c_cc[VERASE] = 127;	/* C-? */
	tty.c_cc[VKILL]  = 21;	/* C-u */
	tty.c_cc[VEOF]   = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP]  = 19;	/* C-s */
	tty.c_cc[VSUSP]  = 26;	/* C-z */

	/* use line discipline 0 */
	tty.c_line = 0;

	/* added CRTSCTS to fix Debian bug 528560 */
	tty.c_cflag &= CBAUD | CBAUDEX | CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS;
	tty.c_cflag |= CREAD | HUPCL | CLOCAL;

	/* input modes */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* output modes */
	tty.c_oflag = OPOST | ONLCR;

	/* local modes */
	tty.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

static int
init_console(void)
{
	int fd;
	int ret = 0;

	/* Clean up */
	ioctl(STDIN_FILENO, TIOCNOTTY, 0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	setsid();

	/* Reopen console */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		ret = errno;
		perror(_PATH_CONSOLE);
		if ((fd = open("/dev/null", O_RDWR)) < 0) {
			perror("/dev/null");
			return errno;
		}
	}

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	ioctl(STDIN_FILENO, TIOCSCTTY, 1);
	tcsetpgrp(STDIN_FILENO, getpgrp());

	setup_term();

	return ret;
}

static void
reset_signals(void)
{
	int i;
	sigset_t set;
	struct sigaction sa;

	/* Disable signal handlers */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < (_NSIG-1); i++) {
		if (i == SIGCHLD)
			continue;
		sigaction(i, &sa, NULL);
	}

	/* Unblock all signals */
	sigfillset(&set);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

static pid_t
run_shell(int timeout, int nowait)
{
	pid_t pid;
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

	switch ((pid = fork())) {
	case -1:
		perror("fork");
		return 0;
	case 0:
		/* Reset signal handlers set for parent process */
		reset_signals();

		/* Reopen console */
		init_console();

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
		if (nowait)
			return pid;
		else {
			waitpid(pid, NULL, 0);
			return 0;
		}
	}
}

static void
control_signal(int sig, int how)
{
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	sigprocmask(how, &ss, NULL);
}

static void
init_signals(void)
{
	int i;
	struct sigaction sa;

	/* Prevent nested lifecycle handlers from observing partial state. */
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < ARRAY_SIZE(signals_event); i++)
		sigaddset(&sa.sa_mask, signals_event[i]);
	for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++)
		sigaddset(&sa.sa_mask, signals_shutdown[i]);
	for (i = 0; i < ARRAY_SIZE(signals_fatal); i++)
		sigaddset(&sa.sa_mask, signals_fatal[i]);

	/* Synchronous faults retain the emergency path; shutdown requests defer. */
	sa.sa_handler = catch_sig_fatal;
	for (i = 0; i < ARRAY_SIZE(signals_fatal); i++)
		sigaction(signals_fatal[i], &sa, NULL);

	sa.sa_handler = catch_sig_shutdown;
	for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++)
		sigaction(signals_shutdown[i], &sa, NULL);

	/* SIGCHLD reaping performs waitpid only; no stdio in signal context. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_reap;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	/* Event handlers only set their corresponding sig_atomic_t flags. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_event;
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < ARRAY_SIZE(signals_event); i++)
		sigaddset(&sa.sa_mask, signals_event[i]);
	for (i = 0; i < ARRAY_SIZE(signals_event); i++)
		sigaction(signals_event[i], &sa, NULL);
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

static int
init_nodes(void)
{
	int nvram_result;

	nvram_result = init_nvram_node();
#if defined (USE_HW_NAT)
	mknod("/dev/hwnat0", S_IFCHR | 0666, makedev(220, 0));
#endif
#if defined (USE_MTK_AES)
	mknod("/dev/crypto", S_IFCHR | 0666, makedev(10, 60));
#endif
#if defined (APP_OPENVPN)
	{
		int tun_status = init_openvpn_tun_node();
		if (tun_status != 0)
			logmessage(LOGNAME,
				   "OpenVPN TUN device node initialization failed (status=%d)",
				   tun_status);
	}
#endif

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

	/* Legacy void helpers do not expose the status of their internal writes. */
	set_tcp_tweaks();

#if defined (USE_IPV6)
	status = control_if_ipv6_all(0);
	first_status = init_sysctl_record_status(first_status, "net.ipv6.conf.{default,all}.disable_ipv6", status);
#else
	set_libc_gai(1);
#endif
	return first_status;
}

/* Main loop */
void
init_main_loop(void)
{
	pid_t shell_pid = 0;
	int shutdown_handled = 0;
	int router_ready;
	int router_init_status = INIT_ROUTER_RESULT_FATAL;
	int nvram_node_result;
	int dev_init_status;
	int sysctl_status;
	int mdev_status;
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
	nvram_node_result = init_nodes();
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
	init_signals();

	/* Schedule retry only after SIGALRM handler installation. */
	if (init_mdev_retry_state_is_pending(&mdev_retry_state))
		alarm(INIT_MDEV_RETRY_DELAY_SECONDS);

	/* block SIGUSR1 during init_router() */
	control_signal(SIGUSR1, SIG_BLOCK);

	/* SIGUSR2 remains a reserved, currently no-op event signal. */
	control_signal(SIGUSR2, SIG_BLOCK);

	/* Required host prerequisites gate router init; service errors are degraded, not fatal. */
	if (dev_init_status == 0 && nvram_node_result == 0)
		router_init_status = init_router();
	router_init_status = init_router_result_merge_sysctl_status(
		router_init_status, sysctl_status);
	router_ready = (dev_init_status == 0 &&
		       nvram_node_result == 0 &&
		       init_router_status_is_ready(router_init_status));
	if (!router_ready)
		logmessage(LOGNAME, "Router startup failed; console recovery remains available");
	else if (init_router_status_is_degraded(router_init_status))
		logmessage(LOGNAME,
			   "Router startup reached ready state with degraded service status; PID 1 event handling remains enabled");

	/* unblock SIGUSR1 */
	control_signal(SIGUSR1, SIG_UNBLOCK);

	/*
	 * Block queued work signals while inspecting their flags. sigsuspend()
	 * atomically restores the prior mask while sleeping, so a signal cannot
	 * land between the flag check and the wait.
	 */
	sigemptyset(&wait_mask);
	sigaddset(&wait_mask, SIGUSR1);
	sigaddset(&wait_mask, SIGALRM);
	for (i = 0; i < ARRAY_SIZE(signals_shutdown); i++)
		sigaddset(&wait_mask, signals_shutdown[i]);
	sigprocmask(SIG_BLOCK, &wait_mask, &original_mask);

	/* Loop forever */
	for (;;) {
		/* Wait until an event or a deferred shutdown request is pending. */
		while (!signal_state.usr1_pending &&
		       !signal_state.alarm_pending &&
		       (!signal_state.shutdown_pending || shutdown_handled)) {
			if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0)) {
				/* Keep console setup from starving a signal that arrives now. */
				sigprocmask(SIG_SETMASK, &original_mask, NULL);
				shell_pid = run_shell(0, 1);
				sigprocmask(SIG_BLOCK, &wait_mask, NULL);
			} else {
				sigsuspend(&original_mask);
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
