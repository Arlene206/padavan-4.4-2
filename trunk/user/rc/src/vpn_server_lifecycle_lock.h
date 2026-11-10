#ifndef PADAVAN_VPN_SERVER_LIFECYCLE_LOCK_H
#define PADAVAN_VPN_SERVER_LIFECYCLE_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define VPN_SERVER_LIFECYCLE_LOCK_FILE "/tmp/.vpns.lifecycle.lock"
#define VPN_SERVER_LIFECYCLE_LOCK_WAIT_SECONDS 10

/* Keep one stable root-private inode; never unlink it while the router runs. */
static inline int
vpn_server_lifecycle_lock_acquire_path(const char *path, unsigned int wait_seconds)
{
	int fd, saved_errno;
	unsigned int attempt = 0;
	struct stat st;
	struct flock lock;

	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}
	fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC, S_IRUSR | S_IWUSR);
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
		if (errno != EINTR && errno != EACCES && errno != EAGAIN)
			goto failed;
		if (attempt++ >= wait_seconds) {
			if (errno == EACCES || errno == EAGAIN)
				errno = EBUSY;
			goto failed;
		}
		(void)sleep(1);
	}
failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static inline int
vpn_server_lifecycle_lock_acquire(void)
{
	return vpn_server_lifecycle_lock_acquire_path(
		VPN_SERVER_LIFECYCLE_LOCK_FILE, VPN_SERVER_LIFECYCLE_LOCK_WAIT_SECONDS);
}

static inline int
vpn_server_lifecycle_lock_release(int fd)
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

#endif /* PADAVAN_VPN_SERVER_LIFECYCLE_LOCK_H */
