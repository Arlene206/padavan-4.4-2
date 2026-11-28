#ifndef PADAVAN_NVRAM_TRANSACTION_LOCK_H
#define PADAVAN_NVRAM_TRANSACTION_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NVRAM_TRANSACTION_LOCK_FILE "/tmp/.nvram.api.lock"
#define NVRAM_TRANSACTION_LOCK_WAIT_SECONDS 15
#define NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS 8U
#define NVRAM_TRANSACTION_LOCK_MODE (S_IRUSR | S_IWUSR)

static inline int
nvram_transaction_lock_retry_interrupted(unsigned int *interruptions)
{
	if (!interruptions) {
		errno = EINVAL;
		return -1;
	}
	if (*interruptions >= NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS) {
		errno = EINTR;
		return -1;
	}
	++*interruptions;
	return 0;
}

typedef int (*nvram_transaction_lock_flock_fn)(int fd, int operation,
						       void *context);

static inline int
nvram_transaction_lock_flock_real(int fd, int operation, void *context)
{
	(void)context;
	return flock(fd, operation);
}

/* Bound interruption retries for lock, unlock and nonblocking acquisition. */
static inline int
nvram_transaction_lock_flock_with(int fd, int operation,
				  nvram_transaction_lock_flock_fn flock_fn,
				  void *context)
{
	unsigned int interruptions = 0;

	if (!flock_fn) {
		errno = EINVAL;
		return -1;
	}
	for (;;) {
		if (flock_fn(fd, operation, context) == 0)
			return 0;
		if (errno != EINTR)
			return -1;
		if (nvram_transaction_lock_retry_interrupted(&interruptions) != 0)
			return -1;
	}
}

/* Complete the requested interval even when signals interrupt nanosleep. */
static inline int
nvram_transaction_lock_wait_interval(unsigned int seconds)
{
	struct timespec requested, remaining;
	unsigned int interruptions = 0;

	requested.tv_sec = (time_t)seconds;
	requested.tv_nsec = 0;
	for (;;) {
		if (nanosleep(&requested, &remaining) == 0)
			return 0;
		if (errno != EINTR)
			return -1;
		if (nvram_transaction_lock_retry_interrupted(&interruptions) != 0)
			return -1;
		requested = remaining;
	}
}

/* The lock inode must remain stable for the entire boot. */
static inline int
nvram_transaction_lock_acquire_path(const char *path, int exclusive,
                                    unsigned int wait_seconds)
{
	int fd, saved_errno;
	unsigned int attempt = 0;
	struct stat st, tmp_stat;
	int operation = exclusive ? LOCK_EX : LOCK_SH;

	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}
	/* /tmp's sticky bit prevents other users replacing the root-owned inode. */
	if (lstat("/tmp", &tmp_stat) != 0)
		return -1;
	if (!S_ISDIR(tmp_stat.st_mode) || tmp_stat.st_uid != (uid_t)0 ||
	    (tmp_stat.st_mode & S_ISVTX) == 0) {
		errno = EPERM;
		return -1;
	}

	fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC,
		  NVRAM_TRANSACTION_LOCK_MODE);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & 077) != 0 ||
	    (st.st_mode & 07000) != 0) {
		errno = EPERM;
		goto failed;
	}
	if ((st.st_mode & 07777) != NVRAM_TRANSACTION_LOCK_MODE) {
		if (fchmod(fd, NVRAM_TRANSACTION_LOCK_MODE) != 0 ||
		    fstat(fd, &st) != 0)
			goto failed;
		if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
		    st.st_nlink != 1 ||
		    (st.st_mode & 07777) != NVRAM_TRANSACTION_LOCK_MODE) {
			errno = EPERM;
			goto failed;
		}
	}

	for (;;) {
		if (nvram_transaction_lock_flock_with(
			    fd, operation | LOCK_NB,
			    nvram_transaction_lock_flock_real, NULL) == 0)
			return fd;
		if (errno == EINTR)
			goto failed;
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			goto failed;
		if (attempt >= wait_seconds) {
			errno = EBUSY;
			goto failed;
		}
		++attempt;
		if (nvram_transaction_lock_wait_interval(1) != 0)
			goto failed;
	}

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static inline int
nvram_transaction_lock_acquire(int exclusive, unsigned int wait_seconds)
{
	return nvram_transaction_lock_acquire_path(
		NVRAM_TRANSACTION_LOCK_FILE, exclusive, wait_seconds);
}

static inline int
nvram_transaction_lock_release(int fd)
{
	int saved_errno = 0;

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	if (nvram_transaction_lock_flock_with(
		    fd, LOCK_UN, nvram_transaction_lock_flock_real, NULL) != 0)
		saved_errno = errno ? errno : EIO;
	if (close(fd) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_NVRAM_TRANSACTION_LOCK_H */
