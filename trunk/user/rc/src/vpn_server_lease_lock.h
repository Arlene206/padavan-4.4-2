#ifndef PADAVAN_VPN_SERVER_LEASE_LOCK_H
#define PADAVAN_VPN_SERVER_LEASE_LOCK_H

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(APP_OPENVPN)

#define VPN_SERVER_LEASE_LOCK_FILE "/tmp/.vpns.leases.shared.lock"
#define VPN_SERVER_LEASE_LOCK_MODE (S_IRUSR | S_IWUSR | S_IWGRP)

/* OpenVPN drops to SYS_GROUP_NOGROUP before invoking client hooks. Keep one
 * stable root-owned lock inode writable only by that group and root. /tmp's
 * sticky bit prevents the dropped-privilege process from replacing it. */
static inline int
vpn_server_lease_lock_acquire_path(const char *path, uid_t owner_uid,
                                   gid_t group_gid, int prepare)
{
	int fd, saved_errno;
	struct stat st, tmp_stat;
	struct flock lock;

	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}
	if (prepare && geteuid() != owner_uid) {
		errno = EPERM;
		return -1;
	}
	/* A root-owned sticky /tmp is the trust boundary for the shared inode. */
	if (lstat("/tmp", &tmp_stat) != 0)
		return -1;
	if (!S_ISDIR(tmp_stat.st_mode) || tmp_stat.st_uid != (uid_t)0 ||
	    (tmp_stat.st_mode & S_ISVTX) == 0) {
		errno = EPERM;
		return -1;
	}
	fd = open(path, O_WRONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC |
		  (prepare ? O_CREAT : 0), S_IRUSR | S_IWUSR);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != owner_uid ||
	    st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}

	/* Only root prepares shared permissions. The unprivileged callback can
	 * open an existing inode but can never create or replace it. */
	if (prepare) {
		if (st.st_gid != group_gid &&
		    fchown(fd, owner_uid, group_gid) != 0)
			goto failed;
		if (fchmod(fd, VPN_SERVER_LEASE_LOCK_MODE) != 0)
			goto failed;
		if (fstat(fd, &st) != 0)
			goto failed;
	}
	if (!S_ISREG(st.st_mode) || st.st_uid != owner_uid ||
	    st.st_gid != group_gid || st.st_nlink != 1 ||
	    (st.st_mode & 07777) != VPN_SERVER_LEASE_LOCK_MODE) {
		errno = EPERM;
		goto failed;
	}

	memset(&lock, 0, sizeof(lock));
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

static inline int
vpn_server_lease_lock_acquire(const char *path)
{
	struct group *group_entry;

	if (!SYS_GROUP_NOGROUP[0]) {
		errno = EINVAL;
		return -1;
	}
	group_entry = getgrnam(SYS_GROUP_NOGROUP);
	if (!group_entry) {
		errno = ENOENT;
		return -1;
	}
	return vpn_server_lease_lock_acquire_path(path, (uid_t)0,
						  group_entry->gr_gid,
						  geteuid() == (uid_t)0);
}

static inline int
vpn_server_lease_lock_release(int fd)
{
	int saved_errno = 0;
	struct flock lock;

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	memset(&lock, 0, sizeof(lock));
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

#else /* !APP_OPENVPN */

/* PPP-only builds retain the original root-private lock and do not depend on
 * an OpenVPN nobody group account. */
#define VPN_SERVER_LEASE_LOCK_FILE "/tmp/.vpns.leases.lock"

static inline int
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
	memset(&lock, 0, sizeof(lock));
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

static inline int
vpn_server_lease_lock_release(int fd)
{
	int saved_errno = 0;
	struct flock lock;

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	memset(&lock, 0, sizeof(lock));
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

#endif /* APP_OPENVPN */

#endif /* PADAVAN_VPN_SERVER_LEASE_LOCK_H */
