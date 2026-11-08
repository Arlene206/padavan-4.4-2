#ifndef PADAVAN_VPN_SERVER_LEASE_LOCK_H
#define PADAVAN_VPN_SERVER_LEASE_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define VPN_SERVER_LEASE_LOCK_FILE "/tmp/.vpns.leases.lock"

/* Keep one stable root-private inode. Unlinking a locked file could split
 * concurrent participants across different inodes and defeat serialization. */
static int
vpn_server_lease_lock_acquire(const char *path)
{
	int fd, saved_errno;
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
	while (fcntl(fd, F_SETLKW, &lock) != 0) {
		if (errno == EINTR)
			continue;
		goto failed;
	}
	return fd;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static int
vpn_server_lease_lock_release(int fd)
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

#endif /* PADAVAN_VPN_SERVER_LEASE_LOCK_H */
