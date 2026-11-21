#ifndef PADAVAN_NVRAM_TRANSACTION_LOCK_H
#define PADAVAN_NVRAM_TRANSACTION_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NVRAM_TRANSACTION_LOCK_FILE "/tmp/.nvram.api.lock"
#define NVRAM_TRANSACTION_LOCK_WAIT_SECONDS 15
#define NVRAM_TRANSACTION_LOCK_MODE (S_IRUSR | S_IWUSR)

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
		if (flock(fd, operation | LOCK_NB) == 0)
			return fd;
		if (errno == EINTR)
			continue;
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			goto failed;
		if (attempt >= wait_seconds) {
			errno = EBUSY;
			goto failed;
		}
		++attempt;
		(void)sleep(1);
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
	while (flock(fd, LOCK_UN) != 0) {
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

#endif /* PADAVAN_NVRAM_TRANSACTION_LOCK_H */
