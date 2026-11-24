#ifndef PADAVAN_NVRAM_IOCTL_CLOSE_POLICY_H
#define PADAVAN_NVRAM_IOCTL_CLOSE_POLICY_H

#include <errno.h>

/* Preserve the primary ioctl failure, but never report success when close()
 * fails after an otherwise successful NVRAM operation. */
static inline int
nvram_ioctl_close_status(int ioctl_status, int ioctl_errno,
                         int close_status, int close_errno,
                         int *result_errno)
{
	if (!result_errno) {
		errno = EINVAL;
		return -1;
	}
	if (ioctl_status < 0) {
		*result_errno = ioctl_errno ? ioctl_errno : EIO;
		return ioctl_status;
	}
	if (close_status < 0) {
		*result_errno = close_errno ? close_errno : EIO;
		return -1;
	}
	*result_errno = 0;
	return ioctl_status;
}

#endif /* PADAVAN_NVRAM_IOCTL_CLOSE_POLICY_H */
