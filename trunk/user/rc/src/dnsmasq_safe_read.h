#ifndef PADAVAN_DNSMASQ_SAFE_READ_H
#define PADAVAN_DNSMASQ_SAFE_READ_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Open an untrusted runtime input without a lstat()/open() race. O_NOFOLLOW
 * prevents a final-component symlink swap and O_NONBLOCK makes a raced FIFO
 * harmless until fstat() rejects every non-regular file.
 */
static inline FILE *
dnsmasq_file_open_safe_read(const char *path)
{
	struct stat st;
	FILE *fp;
	int fd, saved_errno;

	if (!path || !path[0]) {
		errno = EINVAL;
		return NULL;
	}

	do {
		fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK, 0);
	} while (fd < 0 && errno == EINTR);
	if (fd < 0)
		return NULL;

	if (fstat(fd, &st) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return NULL;
	}
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & (S_IWGRP | S_IWOTH))) {
		saved_errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM :
			 (st.st_nlink != 1 ? EMLINK : EPERM));
		close(fd);
		errno = saved_errno;
		return NULL;
	}

	fp = fdopen(fd, "r");
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return NULL;
	}
	return fp;
}

#endif /* PADAVAN_DNSMASQ_SAFE_READ_H */
