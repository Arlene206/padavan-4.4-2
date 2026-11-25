#ifndef WSD_SELECT_FD_POLICY_H
#define WSD_SELECT_FD_POLICY_H

#include <errno.h>
#include <sys/select.h>

/*
 * FD_SET/FD_ISSET only support descriptors below FD_SETSIZE. Passing a
 * larger descriptor writes beyond fd_set on common libc targets.
 */
static inline int
wsd_select_fd_add(fd_set *set, int fd, int *maxfd)
{
	if (!set || !maxfd || fd < 0) {
		errno = EINVAL;
		return -1;
	}
	if (fd >= FD_SETSIZE) {
		errno = EMFILE;
		return -1;
	}

	FD_SET(fd, set);
	if (*maxfd < fd)
		*maxfd = fd;
	return 0;
}

#endif /* WSD_SELECT_FD_POLICY_H */
