/*
 * Shared safe-open policy for the DHCPv6 client DUID file.
 *
 * The file lives in /tmp, which is shared-writable. Never follow a symlink,
 * accept a non-regular or multiply-linked file, or overwrite an existing path.
 */
#ifndef DHCP6C_DUID_FILE_POLICY_H
#define DHCP6C_DUID_FILE_POLICY_H

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
dhcp6c_duid_open_existing(const char *path)
{
	struct stat st;
	int fd;
	int saved_errno;

	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}

	/* O_NONBLOCK prevents a hostile FIFO from hanging the DHCPv6 client. */
	fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
	if (fd < 0)
		return -1;

	if (fstat(fd, &st) != 0) {
		saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}

	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & (S_IWGRP | S_IWOTH)) != 0 ||
	    st.st_size < (off_t)sizeof(uint16_t) ||
	    st.st_size > (off_t)(sizeof(uint16_t) + 128)) {
		close(fd);
		errno = EPERM;
		return -1;
	}

	return fd;
}

static int
dhcp6c_duid_open_create(const char *path)
{
	if (!path || !*path) {
		errno = EINVAL;
		return -1;
	}

	/* O_EXCL makes a raced-in path fail rather than truncate its target. */
	return open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW,
		    S_IRUSR | S_IWUSR);
}

#endif
