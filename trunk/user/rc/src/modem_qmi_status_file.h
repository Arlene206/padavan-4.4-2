#ifndef RC_MODEM_QMI_STATUS_FILE_H
#define RC_MODEM_QMI_STATUS_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "modem_wan_start_policy.h"

#ifndef O_NOFOLLOW
#error "QMI status-file reads require O_NOFOLLOW support"
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

/*
 * uqmi publishes these root-owned /tmp status files atomically with mode 0600.
 * Do not trust a pre-created symlink, FIFO, special file, foreign inode, or
 * hard-linked file when deciding whether a WAN session is already active.
 */
static inline int
modem_qmi_status_file_metadata_status(const struct stat *st)
{
	if (!st)
		return EINVAL;
	if (!S_ISREG(st->st_mode) || st->st_uid != geteuid() ||
	    (st->st_mode & 07777) != 0600 || st->st_nlink != 1)
		return EPERM;
	return 0;
}

/*
 * Read a single bounded decimal status value. O_NONBLOCK prevents an attacker
 * from stalling PID-1 by pre-creating a FIFO at /tmp/qmi-handle or
 * /tmp/qmi-client-id; O_NOFOLLOW prevents redirection through a symlink.
 */
static inline int
modem_qmi_status_file_read(const char *path, int *handle)
{
	struct stat st;
	char buffer[64];
	char extra;
	size_t used = 0;
	ssize_t count;
	int fd, status = 0;

	if (!handle)
		return EINVAL;
	*handle = -1;
	if (!path || path[0] != '/')
		return EINVAL;

	fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0)
		return errno > 0 ? errno : EIO;

	if (fstat(fd, &st) < 0) {
		status = errno > 0 ? errno : EIO;
		goto done;
	}
	status = modem_qmi_status_file_metadata_status(&st);
	if (status != 0)
		goto done;

	for (;;) {
		if (used == sizeof(buffer) - 1) {
			do {
				count = read(fd, &extra, sizeof(extra));
			} while (count < 0 && errno == EINTR);
			if (count > 0)
				status = EOVERFLOW;
			else if (count < 0)
				status = errno > 0 ? errno : EIO;
			break;
		}
		count = read(fd, buffer + used, sizeof(buffer) - 1 - used);
		if (count < 0 && errno == EINTR)
			continue;
		if (count < 0) {
			status = errno > 0 ? errno : EIO;
			break;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}

	if (status == 0) {
		if (memchr(buffer, '\0', used) != NULL) {
			status = EINVAL;
		} else {
			buffer[used] = '\0';
			status = modem_qmi_parse_handle(buffer, handle);
		}
	}

done:
	if (close(fd) < 0 && status == 0)
		status = errno > 0 ? errno : EIO;
	if (status != 0)
		*handle = -1;
	return status;
}

#endif /* RC_MODEM_QMI_STATUS_FILE_H */
