#ifndef PADAVAN_CONFIG_FILE_SYNC_H
#define PADAVAN_CONFIG_FILE_SYNC_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Retry transient interruption without treating it as a durability success. */
static int
config_file_sync_fd(int fd)
{
	int status;

	do {
		errno = 0;
		status = fsync(fd);
	} while (status != 0 && errno == EINTR);

	return status;
}

/*
 * fsync(file) does not persist the directory entry installed by rename().
 * Sync the containing directory after publishing a generated configuration.
 */
static int
config_file_sync_parent(const char *target_path)
{
	char parent_path[PATH_MAX];
	const char *slash;
	size_t parent_length;
	int directory_fd;
	int status;
	int saved_errno;

	if (target_path == NULL || target_path[0] == '\0') {
		errno = EINVAL;
		return -1;
	}

	slash = strrchr(target_path, '/');
	if (slash == NULL) {
		parent_path[0] = '.';
		parent_path[1] = '\0';
	} else if (slash == target_path) {
		parent_path[0] = '/';
		parent_path[1] = '\0';
	} else {
		parent_length = (size_t)(slash - target_path);
		if (parent_length >= sizeof(parent_path)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		memcpy(parent_path, target_path, parent_length);
		parent_path[parent_length] = '\0';
	}

#if !defined(O_DIRECTORY) || !defined(O_NOFOLLOW)
	errno = ENOTSUP;
	return -1;
#else
	directory_fd = open(parent_path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
#endif
	if (directory_fd < 0)
		return -1;

	status = config_file_sync_fd(directory_fd);
	saved_errno = status != 0 ? (errno ? errno : EIO) : 0;
	if (close(directory_fd) != 0 && status == 0) {
		status = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (status != 0) {
		errno = saved_errno;
		return -1;
	}

	return 0;
}

#endif /* PADAVAN_CONFIG_FILE_SYNC_H */
