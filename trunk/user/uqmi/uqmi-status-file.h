#ifndef UQMI_STATUS_FILE_H
#define UQMI_STATUS_FILE_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Publish small numeric QMI state files through a private staging file and
 * atomic rename. Never truncate the currently published client/packet handle
 * before the replacement has been written successfully.
 */
static inline int
uqmi_status_file_write(const char *path, unsigned long value)
{
	char staging[PATH_MAX];
	char contents[3 * sizeof(unsigned long) + 1];
	struct stat st;
	int staging_len, contents_len, fd = -1, saved_errno;
	size_t offset = 0;
	ssize_t written;

	if (!path || path[0] != '/') {
		errno = EINVAL;
		return -1;
	}

	staging_len = snprintf(staging, sizeof(staging), "%s.tmp.XXXXXX", path);
	if (staging_len < 0 || staging_len >= (int)sizeof(staging)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	contents_len = snprintf(contents, sizeof(contents), "%lu", value);
	if (contents_len < 0 || contents_len >= (int)sizeof(contents)) {
		errno = EOVERFLOW;
		return -1;
	}

	fd = mkstemp(staging);
	if (fd < 0)
		return -1;

	if (fchmod(fd, 0600) < 0 || fstat(fd, &st) < 0)
		goto fail;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    (st.st_mode & 07777) != 0600 || st.st_nlink != 1) {
		errno = EPERM;
		goto fail;
	}

	while (offset < (size_t)contents_len) {
		written = write(fd, contents + offset,
				(size_t)contents_len - offset);
		if (written < 0) {
			if (errno == EINTR)
				continue;
			goto fail;
		}
		if (written == 0) {
			errno = EIO;
			goto fail;
		}
		offset += (size_t)written;
	}

	if (fsync(fd) < 0)
		goto fail;
	if (close(fd) < 0) {
		fd = -1;
		goto fail;
	}
	fd = -1;

	if (rename(staging, path) < 0)
		goto fail;

	return 0;

fail:
	saved_errno = errno > 0 ? errno : EIO;
	if (fd >= 0)
		close(fd);
	unlink(staging);
	errno = saved_errno;
	return -1;
}

#endif /* UQMI_STATUS_FILE_H */
