#ifndef PADAVAN_MINIUPNPD_LIFECYCLE_LOCK_H
#define PADAVAN_MINIUPNPD_LIFECYCLE_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "service_wait_policy.h"

#define MINIUPNPD_LIFECYCLE_LOCK_FILE "/tmp/.miniupnpd.lifecycle.lock"
#define MINIUPNPD_LIFECYCLE_LOCK_WAIT_SECONDS 10
#define MINIUPNPD_LIFECYCLE_LOCK_MAX_INTERRUPTS 8U

/* Bound repeated signal interruptions separately from ordinary lock contention. */
static inline int
miniupnpd_lifecycle_lock_retry_interrupted(unsigned int *interruptions)
{
	if (!interruptions) {
		errno = EINVAL;
		return -1;
	}
	if (*interruptions >= MINIUPNPD_LIFECYCLE_LOCK_MAX_INTERRUPTS) {
		errno = EINTR;
		return -1;
	}
	++*interruptions;
	return 0;
}

/* Keep this inode stable: unlinking a locked file can split lock participants. */
static inline int
miniupnpd_lifecycle_lock_acquire_path(const char *path,
                                      unsigned int wait_seconds)
{
	int fd, saved_errno;
	unsigned int attempt = 0;
	unsigned int interruptions = 0;
	struct stat st;
	struct flock lock;

	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}
	fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC,
	          S_IRUSR | S_IWUSR);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & 077) != 0) {
		errno = EPERM;
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	for (;;) {
		if (fcntl(fd, F_SETLK, &lock) == 0)
			return fd;
		/* Retry transient EINTR, but never let a signal storm spin forever. */
		if (errno == EINTR) {
			if (miniupnpd_lifecycle_lock_retry_interrupted(&interruptions) == 0)
				continue;
			goto failed;
		}
		interruptions = 0;
		if (errno != EACCES && errno != EAGAIN)
			goto failed;
		if (attempt++ >= wait_seconds) {
			errno = EBUSY;
			goto failed;
		}
		if (service_wait_seconds(1) != 0)
			goto failed;
	}
failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static inline int
miniupnpd_lifecycle_lock_acquire(void)
{
	return miniupnpd_lifecycle_lock_acquire_path(
		MINIUPNPD_LIFECYCLE_LOCK_FILE,
		MINIUPNPD_LIFECYCLE_LOCK_WAIT_SECONDS);
}

static inline int
miniupnpd_lifecycle_lock_release(int fd)
{
	int saved_errno = 0;
	unsigned int interruptions = 0;
	struct flock lock;

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	while (fcntl(fd, F_SETLK, &lock) != 0) {
		if (errno == EINTR &&
		    miniupnpd_lifecycle_lock_retry_interrupted(&interruptions) == 0)
			continue;
		saved_errno = errno ? errno : EIO;
		break;
	}
	if (close(fd) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_MINIUPNPD_LIFECYCLE_LOCK_H */
