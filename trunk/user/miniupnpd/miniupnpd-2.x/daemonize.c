/* $Id: daemonize.c,v 1.13 2012/03/05 20:36:15 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>

#include "daemonize.h"
#include "config.h"
#include "miniupnpd_pidfile_policy.h"

#ifndef USE_DAEMON

int
daemonize(void)
{
	int pid, i;

	switch(fork())
	{
	/* fork error */
	case -1:
		perror("fork()");
		exit(1);

	/* child process */
	case 0:
		/* obtain a new process group */
		if( (pid = setsid()) < 0)
		{
			perror("setsid()");
			exit(1);
		}

		/* close all descriptors */
		for (i=getdtablesize();i>=0;--i) close(i);

		i = open("/dev/null", O_RDWR); /* open stdin */
		dup(i); /* stdout */
		dup(i); /* stderr */

		umask(027);
		chdir("/"); /* chdir to /tmp ? */

		return pid;

	/* parent process */
	default:
		exit(0);
	}
}
#endif

int
writepidfile(const char * fname, int pid)
{
	char pidstring[16];
	int pidstringlen;
	int saved_errno;

	if (!fname || fname[0] == '\0' || pid <= 0) {
		errno = EINVAL;
		return -1;
	}

	pidstringlen = snprintf(pidstring, sizeof(pidstring), "%d\n", pid);
	if (pidstringlen <= 0 || (size_t)pidstringlen >= sizeof(pidstring)) {
		saved_errno = errno ? errno : EOVERFLOW;
		errno = saved_errno;
		syslog(LOG_ERR, "Unable to format pidfile %s: %m", fname);
		errno = saved_errno;
		return -1;
	}

	if (miniupnpd_pidfile_publish(fname, pidstring,
	                             (size_t)pidstringlen) != 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		syslog(LOG_ERR, "Unable to publish pidfile %s: %m", fname);
		errno = saved_errno;
		return -1;
	}

	return 0;
}

/* Keep this descriptor open until exit. The lock inode is separate from the
 * atomically replaced PID file, so rename cannot invalidate singleton ownership. */
static int miniupnpd_pidfile_instance_lock_fd = -1;

int
checkforrunning(const char * fname)
{
	char buffer[64];
	size_t length = 0;
	int pidfile = -1;
	int saved_errno;
	int lock_fd;
	struct stat st;
	pid_t pid;

	if (!fname || fname[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
	if (miniupnpd_pidfile_instance_lock_fd >= 0) {
		errno = EALREADY;
		return -1;
	}

	lock_fd = miniupnpd_pidfile_lock_acquire(fname);
	if (lock_fd < 0) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno == EBUSY)
			return -2;
		syslog(LOG_ERR, "Unable to acquire MiniUPnP PID lock for %s: %s",
		       fname, strerror(saved_errno));
		errno = saved_errno;
		return -1;
	}
	miniupnpd_pidfile_instance_lock_fd = lock_fd;

	pidfile = open(fname, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
	if (pidfile < 0) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno == ENOENT)
			return 0;
		syslog(LOG_ERR, "Unable to open MiniUPnP PID file %s: %s",
		       fname, strerror(saved_errno));
		errno = saved_errno;
		return -1;
	}
	if (fstat(pidfile, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}
	if (miniupnpd_pidfile_read_all_with(pidfile, buffer, sizeof(buffer),
	                                    &length, miniupnpd_pidfile_read_real,
	                                    NULL) != 0)
		goto failed;
	if (close(pidfile) != 0) {
		pidfile = -1;
		goto failed;
	}
	pidfile = -1;
	if (miniupnpd_pidfile_parse_pid(buffer, length, &pid) != 0)
		goto failed;
	/* A stale file can contain this newly reused PID; it cannot name another
	 * live process in the current PID namespace. */
	if (pid == getpid())
		return 0;
	if (kill(pid, 0) == 0)
		return -2;
	saved_errno = errno ? errno : EIO;
	if (saved_errno == ESRCH)
		return 0;
	errno = saved_errno;
	syslog(LOG_ERR, "Unable to verify process from MiniUPnP PID file %s: %s",
	       fname, strerror(saved_errno));
	return -1;

failed:
	saved_errno = errno ? errno : EIO;
	if (pidfile >= 0)
		(void)close(pidfile);
	syslog(LOG_ERR, "Invalid or unreadable MiniUPnP PID file %s: %s",
	       fname, strerror(saved_errno));
	errno = saved_errno;
	return -1;
}

