#ifndef PADAVAN_FTP_CONFIG_FILE_H
#define PADAVAN_FTP_CONFIG_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * Build generated FTP configuration beside its destination. Keeping the
 * temporary file in the same directory makes the final rename atomic.
 */
static int ftp_config_file_sync_fd(int fd);
static int ftp_config_file_sync_parent(const char *target_path);

static int
ftp_config_file_open(const char *target_path, char *staged_path,
                     size_t staged_path_size, FILE **stream)
{
	int fd;
	int path_len;
	int saved_errno;

	if (!target_path || !*target_path || !staged_path ||
	    staged_path_size == 0 || !stream) {
		errno = EINVAL;
		return -1;
	}
	*stream = NULL;
	staged_path[0] = '\0';

	path_len = snprintf(staged_path, staged_path_size, "%s.XXXXXX",
			    target_path);
	if (path_len < 0) {
		staged_path[0] = '\0';
		errno = EINVAL;
		return -1;
	}
	if ((size_t)path_len >= staged_path_size) {
		staged_path[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(staged_path);
	if (fd < 0) {
		saved_errno = errno;
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (fchmod(fd, 0644) != 0)
		goto failed;

	*stream = fdopen(fd, "w");
	if (!*stream)
		goto failed;

	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	close(fd);
	unlink(staged_path);
	staged_path[0] = '\0';
	errno = saved_errno;
	return -1;
}

/*
 * Do not replace a working configuration until all buffered output and the
 * close operation have succeeded. On every failure, discard the staged file.
 */
static int
ftp_config_file_publish(FILE **stream, char *staged_path,
                        const char *target_path)
{
	FILE *fp;
	int saved_errno;

	if (!stream || !*stream || !staged_path || !*staged_path ||
	    !target_path || !*target_path) {
		errno = EINVAL;
		return -1;
	}

	fp = *stream;
	*stream = NULL;

	errno = 0;
	if (ferror(fp) || fflush(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		fclose(fp);
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	if (ftp_config_file_sync_fd(fileno(fp)) != 0) {
		saved_errno = errno ? errno : EIO;
		fclose(fp);
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	if (fclose(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	if (rename(staged_path, target_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	staged_path[0] = '\0';

	/*
	 * rename() already made the complete candidate visible. If the parent
	 * sync fails, report failure so callers keep the daemon stopped; do not
	 * claim that the new configuration will survive a power loss.
	 */
	if (ftp_config_file_sync_parent(target_path) != 0)
		return -1;

	return 0;
}

/* Retry interrupted durability barriers without weakening the commit point. */
static int
ftp_config_file_sync_fd(int fd)
{
	int status;

	do {
		status = fsync(fd);
	} while (status != 0 && errno == EINTR);

	return status;
}

/* Persist the directory entry installed by rename(), not just file contents. */
static int
ftp_config_file_sync_parent(const char *target_path)
{
	char parent_path[PATH_MAX];
	const char *slash;
	size_t parent_length;
	int directory_fd;
	int status, saved_errno;

	if (!target_path || !*target_path) {
		errno = EINVAL;
		return -1;
	}

	slash = strrchr(target_path, '/');
	if (!slash) {
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

	directory_fd = open(parent_path, O_RDONLY | O_DIRECTORY);
	if (directory_fd < 0)
		return -1;

	status = ftp_config_file_sync_fd(directory_fd);
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

#endif /* PADAVAN_FTP_CONFIG_FILE_H */
