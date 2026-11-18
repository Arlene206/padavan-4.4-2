#ifndef PADAVAN_MINIUPNPD_LIFECYCLE_LOCK_H
#define PADAVAN_MINIUPNPD_LIFECYCLE_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MINIUPNPD_LIFECYCLE_LOCK_FILE "/tmp/.miniupnpd.lifecycle.lock"
#define MINIUPNPD_LIFECYCLE_LOCK_WAIT_SECONDS 10

/*
 * sleep() returns the unslept remainder when a signal interrupts it. Keep
 * waiting for that remainder so signal traffic cannot consume the lock's
 * contention timeout early. The callback makes the interruption contract
 * deterministic in the host regression.
 */
typedef unsigned int (*miniupnpd_lifecycle_lock_sleep_fn)(
	unsigned int seconds, void *context);

static inline unsigned int
miniupnpd_lifecycle_lock_sleep_real(unsigned int seconds, void *context)
{
	(void)context;
	return sleep(seconds);
}

static inline int
miniupnpd_lifecycle_lock_wait(unsigned int seconds,
			      miniupnpd_lifecycle_lock_sleep_fn sleep_fn,
			      void *context)
{
	unsigned int remaining = seconds;

	if (!sleep_fn) {
		errno = EINVAL;
		return -1;
	}
	while (remaining > 0) {
		unsigned int next = sleep_fn(remaining, context);

		/* sleep() cannot return more than the requested remainder. */
		if (next > remaining) {
			errno = EIO;
			return -1;
		}
		remaining = next;
	}
	return 0;
}

/* Keep this inode stable: unlinking a locked file can split lock participants. */
static inline int
miniupnpd_lifecycle_lock_acquire_path(const char *path,
                                      unsigned int wait_seconds)
{
	int fd, saved_errno;
	unsigned int attempt = 0;
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
		/* EINTR is a transient syscall interruption, not lock contention. */
		if (errno == EINTR)
			continue;
		if (errno != EACCES && errno != EAGAIN)
			goto failed;
		if (attempt++ >= wait_seconds) {
			errno = EBUSY;
			goto failed;
		}
		if (miniupnpd_lifecycle_lock_wait(
			    1, miniupnpd_lifecycle_lock_sleep_real, NULL) != 0)
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
		if (errno == EINTR)
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
