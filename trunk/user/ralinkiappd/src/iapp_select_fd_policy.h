#ifndef IAPP_SELECT_FD_POLICY_H
#define IAPP_SELECT_FD_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/select.h>

/* FD_SET is only defined for descriptors in [0, FD_SETSIZE). */
static inline int
iapp_select_fd_add(fd_set *set, int fd, int *max_fd)
{
	if (set == NULL || max_fd == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	if (fd >= FD_SETSIZE) {
		errno = EINVAL;
		return -1;
	}

	FD_SET(fd, set);
	if (fd > *max_fd)
		*max_fd = fd;
	return 0;
}

#endif /* IAPP_SELECT_FD_POLICY_H */
